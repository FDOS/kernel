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

STATIC int CharRequest(struct dhdr FAR **pdev, unsigned command)
{
  struct dhdr FAR *dev = *pdev;
  CharReqHdr.r_command = command;
  CharReqHdr.r_unit = 0;
  CharReqHdr.r_status = 0;
  CharReqHdr.r_length = sizeof(request);
  execrh(&CharReqHdr, dev);
  if (CharReqHdr.r_status & S_ERROR)
  {
    for (;;) {
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
      }
    }
  }
  return SUCCESS;
}

long BinaryCharIO(struct dhdr FAR **pdev, size_t n, void FAR * bp,
                  unsigned command)
{
  int err;
  do
  {
    CharReqHdr.r_count = n;
    CharReqHdr.r_trans = bp;
    err = CharRequest(pdev, command);
  } while (err == 1);
  return err == SUCCESS ? (long)CharReqHdr.r_count : err;
}

STATIC int CharIO(struct dhdr FAR **pdev, unsigned char ch, unsigned command)
{
  int err = (int)BinaryCharIO(pdev, 1, &ch, command);
  if (err == 0)
    return 256;
  if (err < 0)
    return err;
  return ch;
}

/* STATE FUNCTIONS */

void CharCmd(struct dhdr FAR **pdev, unsigned command)
{
  while (CharRequest(pdev, command) == 1);
}

STATIC int Busy(struct dhdr FAR **pdev)
{
  CharCmd(pdev, C_ISTAT);
  return CharReqHdr.r_status & S_BUSY;
}

void con_flush(struct dhdr FAR **pdev)
{
  CharCmd(pdev, C_IFLUSH);
}

/* if the sft is invalid, then we just monitor syscon */
struct dhdr FAR *sft_to_dev(sft FAR *s)
{
  if (FP_OFF(s) == (size_t) -1)
    return syscon;
  if (s->sft_flags & SFT_FDEVICE)
    return s->sft_dev;
  return NULL;
}

int StdinBusy(void)
{
  sft FAR *s = get_sft(STDIN);
  struct dhdr FAR *dev = sft_to_dev(s);

  if (dev)
    return Busy(&dev);

  return s->sft_posit >= s->sft_size;
}

STATIC void Do_DosIdle_loop(struct dhdr FAR **pdev)
{
  /* the idle loop is only safe if we're using the character stack */
  if (user_r->AH < 0xd)
    while (Busy(pdev) && Busy(&syscon))
      DosIdle_int();
}

/* get character from the console - this is how DOS gets
   CTL_C/CTL_S/CTL_P when outputting */
STATIC int ndread(struct dhdr FAR **pdev)
{
  CharCmd(pdev, C_NDREAD);
  if (CharReqHdr.r_status & S_BUSY)
    return -1;
  return CharReqHdr.r_ndbyte;
}

STATIC void con_skip_char(struct dhdr FAR **pdev)
{
  if (CharIO(pdev, 0, C_INPUT) == CTL_C)
    handle_break(pdev);
}

STATIC void con_hold(struct dhdr FAR **pdev)
{
  int c = check_handle_break();
  if (*pdev != syscon)
    c = ndread(pdev);
  if (c == CTL_S || c == CTL_C)
  {
    con_skip_char(pdev);
    Do_DosIdle_loop(pdev);
    /* just wait */
    check_handle_break();
    con_skip_char(pdev);
  }
}

int con_break(void)
{
  int c = ndread(&syscon);
  if (c == CTL_C)
    con_skip_char(&syscon);
  return c;
}

/* OUTPUT FUNCTIONS */

#ifdef __WATCOMC__
void fast_put_char(char c);
#pragma aux fast_put_char = "int 29h" parm[al] modify exact [bx]
#else

/* writes a character in raw mode using int29 for speed */
STATIC void fast_put_char(unsigned char chr)
{
#if defined(__TURBOC__)
    _AL = chr;
    __int__(0x29);
#elif defined(I86)
    asm
    {
      mov al, byte ptr chr;
      int 0x29;
    }
#endif
}
#endif

void update_scr_pos(unsigned char c, unsigned char count)
{
  unsigned char scrpos = scr_pos;

  if (c == CR)
    scrpos = 0;
  else if (c == BS) {
    if (scrpos > 0)
      scrpos--;
  } else if (c != LF && c != BELL) {
    scrpos += count;
  }
  scr_pos = scrpos;
}

/* writes a character in cooked mode; maybe with printer echo;
   handles TAB expansion */
STATIC int cooked_write_char(struct dhdr FAR **pdev,
                      unsigned char c,
                      unsigned char *fast_counter)
{
  unsigned char count = 1;

  if (c == HT) {
    count = 8 - (scr_pos & 7);
    c = ' ';
  }
  update_scr_pos(c, count);

  do {

    /* if not fast then < 0x80; always check
       otherwise check every 32 characters */
    if (*fast_counter <= 0x80)
      /* Test for hold char and ctl_c */
      con_hold(pdev);
    *fast_counter += 1;
    *fast_counter &= 0x9f;

    if (PrinterEcho)
      DosWrite(STDPRN, 1, &c);
    if (*fast_counter & 0x80)
    {
      fast_put_char(c);
    }
    else
    {
      int err = CharIO(pdev, c, C_OUTPUT);
      if (err < 0)
        return err;
    }
  } while (--count != 0);
  return SUCCESS;
}

long cooked_write(struct dhdr FAR **pdev, size_t n, char FAR *bp)
{
  size_t xfer = 0;
  unsigned char fast_counter;

  /* bit 7 means fastcon; low 5 bits count number of characters */
  fast_counter = ((*pdev)->dh_attr & ATTR_FASTCON) << 3;

  for (xfer = 0; xfer < n; xfer++)
  {
    int err;
    unsigned char c = *bp++;

    if (c == CTL_Z)
      break;

    err = cooked_write_char(pdev, c, &fast_counter);
    if (err < 0)
      return err;
  }
  return xfer;
}

/* writes character for disk file or device */
void write_char(int c, int sft_idx)
{
  unsigned char ch = (unsigned char)c;
  DosRWSft(sft_idx, 1, &ch, XFR_FORCE_WRITE);
}

void write_char_stdout(int c)
{
  unsigned char count = 1;
  unsigned flags = get_sft(STDOUT)->sft_flags & (SFT_FDEVICE | SFT_FBINARY);

  /* ah=2, ah=9 should expand tabs even for raw devices and disk files */
  if (flags != SFT_FDEVICE)
  {
    if (c == HT) {
      count = 8 - (scr_pos & 7);
      c = ' ';
    }
    /* for raw devices already updated in dosfns.c */
    if (!(flags & SFT_FDEVICE))
      update_scr_pos(c, count);
  }

  do {
    write_char(c, get_sft_idx(STDOUT));
  } while (--count != 0);
}

#define iscntrl(c) ((unsigned char)(c) < ' ')

/* this is for handling things like ^C, mostly used in echoed input */
STATIC int echo_char(int c, int sft_idx)
{
  int out = c;
  if (iscntrl(c) && c != HT && c != LF && c != CR)
  {
    write_char('^', sft_idx);
    out += '@';
  }
  write_char(out, sft_idx);
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

STATIC int raw_get_char(struct dhdr FAR **pdev, BOOL check_break)
{
  Do_DosIdle_loop(pdev);
  if (check_break)
  {
    con_hold(pdev);
    Do_DosIdle_loop(pdev);
  }
  return CharIO(pdev, 0, C_INPUT);
}

long cooked_read(struct dhdr FAR **pdev, size_t n, char FAR *bp)
{
  unsigned xfer = 0;
  int c;
  while(n--)
  {
    c = raw_get_char(pdev, TRUE);
    if (c < 0)
      return c;
    if (c == 256)
      break;
    *bp++ = c;
    xfer++;
    if ((unsigned char)c == CTL_Z)
      break;
  }
  return xfer;
}

unsigned char read_char(int sft_idx, BOOL check_break)
{
  unsigned char c;
  struct dhdr FAR *dev = sft_to_dev(idx_to_sft(sft_idx));

  if (dev)
    return (unsigned char)raw_get_char(&dev, check_break);

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
  size_t chars_left;
    
  if (inputptr == NULL)
  {
    /* can we reuse kb_buf or was it overwritten? */
    if (kb_buf.kb_size != LINEBUFSIZECON)
    {
      kb_buf.kb_count = 0;
      kb_buf.kb_size = LINEBUFSIZECON;
    }
    read_line(sft_idx, sft_idx, &kb_buf);
    kb_buf.kb_buf[kb_buf.kb_count + 1] = echo_char(LF, sft_idx);
    inputptr = kb_buf.kb_buf;
    if (*inputptr == CTL_Z)
    {
      inputptr = NULL;
      return 0;
    }
  }

  chars_left = &kb_buf.kb_buf[kb_buf.kb_count + 2] - inputptr;
  if (n > chars_left)
    n = chars_left;

  fmemcpy(bp, inputptr, n);
  inputptr += n;
  if (n == chars_left)
    inputptr = NULL;
  return n;
}

