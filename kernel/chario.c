/****************************************************************/
/*                                                              */
/*                          chario.c                            */
/*                           DOS-C                              */
/*                                                              */
/*    Character device functions and device driver interface    */
/*                                                              */
/*                      Copyright (c) 1994                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/*                                                              */
/****************************************************************/

#include "portab.h"

#ifdef VERSION_STRINGS
static BYTE *charioRcsId =
    "$Id$";
#endif

#include "globals.h"

STATIC int CharRequest(struct dhdr FAR *dev)
{
  CharReqHdr.r_unit = 0;
  CharReqHdr.r_status = 0;
  CharReqHdr.r_length = sizeof(request);
  execrh(&CharReqHdr, dev);
  if (CharReqHdr.r_status & S_ERROR)
  {
  charloop:
    switch (char_error(&CharReqHdr, dev))
    {
      case ABORT:
      case FAIL:
        return DE_INVLDACC;
      case CONTINUE:
        CharReqHdr.r_count = 0;
        return 0;
      case RETRY:
        return 1;
      default:
        goto charloop;
    }
  }
  return 0;
}

long BinaryCharIO(struct dhdr FAR * dev, size_t n, void FAR * bp, unsigned command)
{
  int err = SUCCESS;
  do
  {
    CharReqHdr.r_command = command;
    CharReqHdr.r_count = n;
    CharReqHdr.r_trans = bp;
  } while ((err = CharRequest(dev)) == 1);
  return err == SUCCESS ? CharReqHdr.r_count : err;
}

/* STATE FUNCTIONS */

STATIC struct dhdr FAR *idx_to_dev(int sft_idx)
{
  sft FAR *s = idx_to_sft(sft_idx);
  if (FP_OFF(s) == (size_t)-1 || (s->sft_mode & SFT_MWRITE) ||
      !(s->sft_flags & SFT_FDEVICE))
    return syscon;
  else
    return s->sft_dev;
}

/* if sft_idx is invalid, then we just monitor syscon */
STATIC BOOL Busy(int sft_idx)
{
  sft FAR *s = idx_to_sft(sft_idx);

  if (s->sft_flags & SFT_FDEVICE)
  {
    struct dhdr FAR *dev = idx_to_dev(sft_idx);
    do {
      CharReqHdr.r_command = C_ISTAT;
    } while(CharRequest(dev) == 1);
    if (CharReqHdr.r_status & S_BUSY)
      return TRUE;
    else
      return FALSE;
  }
  else
    return s->sft_posit >= s->sft_size;
}

BOOL StdinBusy(void)
{
  return Busy(get_sft_idx(STDIN));
}

STATIC void Do_DosIdle_loop(int sft_idx)
{
  /* the idle loop is only safe if we're using the character stack */  
  if (user_r->AH < 0xd)
    while (Busy(sft_idx))
      DosIdle_int();
}

/* get character from the console - this is how DOS gets
   CTL_C/CTL_S/CTL_P when outputting */
STATIC int ndread(int sft_idx)
{
  struct dhdr FAR *dev = idx_to_dev(sft_idx);
  do {
    CharReqHdr.r_command = C_NDREAD;
  } while(CharRequest(dev) == 1);
  if (CharReqHdr.r_status & S_BUSY)
    return -1;
  return CharReqHdr.r_ndbyte;
}

STATIC int con_get_char(int sft_idx)
{
  unsigned char c;
  BinaryCharIO(idx_to_dev(sft_idx), 1, &c, C_INPUT);
  if (c == CTL_C)
    handle_break(sft_idx);
  return c;
}

STATIC void con_hold(int sft_idx)
{
  int c;
  if (control_break())
    handle_break(-1);
  c = ndread(sft_idx);
  if (c == CTL_S)
  {
    con_get_char(sft_idx);
    Do_DosIdle_loop(sft_idx);
    /* just wait */
    c = con_get_char(sft_idx);
  }
  if (c == CTL_C)
  {
    con_get_char(sft_idx);
    handle_break(sft_idx);
  }
}

BOOL con_break(void)
{
  if (ndread(-1) == CTL_C)
  {
    con_get_char(-1);
    return TRUE;
  }
  else
    return FALSE;
}

/* OUTPUT FUNCTIONS */

#ifdef __WATCOMC__
void int29(char c);
#pragma aux int29 = "int 29h" parm[al] modify exact [bx]
#endif

/* writes a character in raw mode; use int29 if possible
   for speed */
STATIC int raw_put_char(int sft_idx, int c)
{
  struct dhdr FAR *dev = idx_to_dev(sft_idx);
  unsigned char chr = (unsigned char)c;
  
  if (PrinterEcho)
    DosWrite(STDPRN, 1, &chr);

  if (dev->dh_attr & ATTR_FASTCON)
  {
#if defined(__TURBOC__)
    _AL = chr;
    __int__(0x29);
#elif defined(__WATCOMC__)
    int29(chr);
#elif defined(I86)
    asm
    {
      mov al, byte ptr chr;
      int 0x29;
    }
#endif
    return 0;
  }
  c = (int)BinaryCharIO(dev, 1, &chr, C_OUTPUT);
  if (c < 0)
    return c;
  else
    return chr;
}

/* writes a character in cooked mode; maybe with printer echo;
   handles TAB expansion */
STATIC int cooked_put_char(int sft_idx, int c)
{
  int err = 0;
  unsigned char scrpos = scr_pos;
  
  /* Test for hold char */
  con_hold(sft_idx);

  switch (c)
  {
    case CR:
      scrpos = 0;
      break;
    case LF:
    case BELL:
      break;
    case BS:
      if (scrpos > 0)
        scrpos--;
      break;
    case HT:
      do
        err = raw_put_char(sft_idx, ' ');
      while (err >= 0 && ((++scrpos) & 7));
      break;
    default:
      scrpos++;
  }
  scr_pos = scrpos;
  if (c != HT)
    err = raw_put_char(sft_idx, c);
  return err;
}

long cooked_write(int sft_idx, size_t n, char FAR *bp)
{
  size_t xfer;
  int err = SUCCESS;
        
  for (xfer = 0; err >= SUCCESS && xfer < n && *bp != CTL_Z; bp++, xfer++)
    err = cooked_put_char(sft_idx, *bp);
  return err < SUCCESS ? err : xfer;
}

/* writes character for disk file or device */
void write_char(int c, int sft_idx)
{
  unsigned char ch = (unsigned char)c;
  DosRWSft(sft_idx, 1, &ch, XFR_FORCE_WRITE);
}

void write_char_stdout(int c)
{
  write_char(c, get_sft_idx(STDOUT));
}

#define iscntrl(c) ((unsigned char)(c) < ' ')

/* this is for handling things like ^C, mostly used in echoed input */
STATIC int echo_char(int c, int sft_idx)
{
  if (iscntrl(c) && c != HT && c != LF && c != CR)
  {
    write_char('^', sft_idx);
    write_char(c + '@', sft_idx);
  }
  else
    write_char(c, sft_idx);
  return c;
}

int echo_char_stdin(int c)
{
  return echo_char(c, get_sft_idx(STDIN));
}

STATIC void destr_bs(int sft_idx)
{
  write_char(BS, sft_idx);
  write_char(' ', sft_idx);
  write_char(BS, sft_idx);
}

/* READ FUNCTIONS */

STATIC int raw_get_char(int sft_idx, BOOL check_break)
{
  unsigned char c;
  int err;
  
  Do_DosIdle_loop(sft_idx);
  if (check_break)
    con_hold(sft_idx);
  
  err = (int)BinaryCharIO(idx_to_dev(sft_idx), 1, &c, C_INPUT);
  return err < 0 ? err : c;
}

long cooked_read(int sft_idx, size_t n, char FAR *bp)
{
  unsigned xfer = 0;
  int c;
  while(n--)
  {
    c = raw_get_char(sft_idx, TRUE);
    if (c < 0)
      return c;
    *bp++ = c;
    xfer++;
    if (bp[-1] == CTL_Z)
      break;
  }
  return xfer;
}

unsigned char read_char(int sft_idx, BOOL check_break)
{
  unsigned char c;
  sft FAR *s = idx_to_sft(sft_idx);

  if ((FP_OFF(s) == (size_t) -1) || (s->sft_flags & SFT_FDEVICE))
    return raw_get_char(sft_idx, check_break);

  DosRWSft(sft_idx, 1, &c, XFR_READ);
  return c;
}

STATIC unsigned char read_char_check_break(int sft_idx)
{
  return read_char(sft_idx, TRUE);
}

unsigned char read_char_stdin(BOOL check_break)
{
  return read_char(get_sft_idx(STDIN), check_break);
}

void KbdFlush(int sft_idx)
{
  struct dhdr FAR *dev = idx_to_dev(sft_idx);

  do {
    CharReqHdr.r_unit = 0;
    CharReqHdr.r_status = 0;
    CharReqHdr.r_command = C_IFLUSH;
    CharReqHdr.r_length = sizeof(request);
  } while (CharRequest(dev) == 1);
}

/* reads a line (buffered, called by int21/ah=0ah, 3fh) */
void read_line(int sft_in, int sft_out, keyboard FAR * kp)
{
  unsigned c;
  unsigned cu_pos = scr_pos;
  unsigned count = 0, stored_pos = 0;
  unsigned size = kp->kb_size, stored_size = kp->kb_count; 
  BOOL insert = FALSE;

  if (size == 0)
    return;

  /* the stored line is invalid unless it ends with a CR */
  if (kp->kb_buf[stored_size] != CR)
    stored_size = 0;
      
  do
  {
    unsigned new_pos = stored_size;
    
    c = read_char_check_break(sft_in);
    if (c == 0)
      c = (unsigned)read_char_check_break(sft_in) << 8;
    switch (c)
    {
      case CTL_F:
        continue;

      case RIGHT:
      case F1:
        if (stored_pos < stored_size && count < size - 1)
          local_buffer[count++] = echo_char(kp->kb_buf[stored_pos++], sft_out);
        break;
            
      case F2:
      case F4:
        /* insert/delete up to character c */
        {
          unsigned char c2 = read_char_check_break(sft_in);
          new_pos = stored_pos;
          if (c2 == 0)
          {
            read_char_check_break(sft_in);
          }
          else
          {
            char FAR *sp = fmemchr(&kp->kb_buf[stored_pos],
                                   c2, stored_size - stored_pos);
            if (sp != NULL)
                new_pos = (FP_OFF(sp) - FP_OFF(&kp->kb_buf[stored_pos])) + 1;
          }
        }
        /* fall through */
      case F3:
        if (c != F4) /* not delete */
        {
          while (stored_pos < new_pos && count < size - 1)
              local_buffer[count++] = echo_char(kp->kb_buf[stored_pos++], sft_out);
        }
        stored_pos = new_pos;
        break;
        
      case F5:
        fmemcpy(kp->kb_buf, local_buffer, count);
        stored_size = count;
        write_char('@', sft_out);
        goto start_new_line;

      case INS:
        insert = !insert;
        break;
            
      case DEL:
        stored_pos++;
        break;

      case LEFT:
      case CTL_BS:
      case BS:
        if (count > 0)
        {
          unsigned new_pos;
          char c2 = local_buffer[--count];
          if (c2 == HT)
          {
            unsigned i;
            new_pos = cu_pos;
            for (i = 0; i < count; i++)
            {
              if (local_buffer[i] == HT)
                new_pos = (new_pos + 8) & ~7;
              else if (iscntrl(local_buffer[i]))
                new_pos += 2;
              else
                new_pos++;
            }
            do
              destr_bs(sft_out);
            while (scr_pos > new_pos);
          }
          else
          {
            if (iscntrl(c2))
              destr_bs(sft_out);
            destr_bs(sft_out);
          }
        }
        if (stored_pos > 0)
          stored_pos--;
        break;

      case ESC:
        write_char('\\', sft_out);
    start_new_line:
        write_char(CR, sft_out);
        write_char(LF, sft_out);
        for (count = 0; count < cu_pos; count++)
          write_char(' ', sft_out);
        count = 0;
        stored_pos = 0;
        insert = FALSE;
        break;

      case F6:
        c = CTL_Z;
        /* fall through */

      default:
        if (count < size - 1 || c == CR)
          local_buffer[count++] = echo_char(c, sft_out);
        else
          write_char(BELL, sft_out);
        if (stored_pos < stored_size && !insert)
          stored_pos++;
        break;
    }
  } while (c != CR);
  fmemcpy(kp->kb_buf, local_buffer, count);
  /* if local_buffer overflows into the CON default buffer we
     must invalidate it */
  if (count > LINEBUFSIZECON)
    kb_buf.kb_size = 0;
  /* kb_count does not include the final CR */
  kp->kb_count = count - 1;
}

/* called by handle func READ (int21/ah=3f) */
size_t read_line_handle(int sft_idx, size_t n, char FAR * bp)
{
  char *bufend = &kb_buf.kb_buf[kb_buf.kb_count + 2];
    
  if (inputptr == NULL)
  {
    /* can we reuse kb_buf or was it overwritten? */
    if (kb_buf.kb_size != LINEBUFSIZECON)
    {
      kb_buf.kb_count = 0;
      kb_buf.kb_size = LINEBUFSIZECON;
    }
    read_line(sft_idx, sft_idx, &kb_buf);
    bufend = &kb_buf.kb_buf[kb_buf.kb_count + 2];
    bufend[-1] = echo_char(LF, sft_idx);
    inputptr = kb_buf.kb_buf;
    if (*inputptr == CTL_Z)
    {
      inputptr = NULL;
      return 0;
    }   
  }

  if (inputptr > bufend - n)
    n = bufend - inputptr;

  fmemcpy(bp, inputptr, n);
  inputptr += n;
  if (inputptr == bufend)
    inputptr = NULL;
  return n;
}

