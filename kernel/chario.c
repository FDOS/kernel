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

/*      Return a pointer to the first driver in the chain that
 *      matches the attributes.
 *      not necessary because we have the syscon pointer.
 */
#if 0
struct dhdr FAR *finddev(UWORD attr_mask)
{
  struct dhdr far *dh;

  for (dh = nul_dev.dh_next; FP_OFF(dh) != 0xFFFF; dh = dh->dh_next)
  {
    if (dh->dh_attr & attr_mask)
      return dh;
  }

  /* return dev/null if no matching driver found */
  return &nul_dev;
}
#endif

UCOUNT BinaryCharIO(struct dhdr FAR * dev, UCOUNT n, void FAR * bp, unsigned command, COUNT *err)
{
  *err = SUCCESS;
    
  FOREVER
  {
    CharReqHdr.r_length = sizeof(request);
    CharReqHdr.r_command = command;
    CharReqHdr.r_count = n;
    CharReqHdr.r_trans = bp;
    CharReqHdr.r_status = 0;
    execrh(&CharReqHdr, dev);
    if (CharReqHdr.r_status & S_ERROR)
    {
    charloop:
      switch (char_error(&CharReqHdr, dev))
      {
        case ABORT:
        case FAIL:
          *err = DE_INVLDACC;
          return 0;
        case CONTINUE:
          break;
        case RETRY:
          continue;
        default:
          goto charloop;
      }
    }
    break;
  }
  return CharReqHdr.r_count;
}

VOID _cso(COUNT c)
{
  if (syscon->dh_attr & ATTR_FASTCON)
  {
#if defined(__TURBOC__)
    _AL = c;
    __int__(0x29);
#elif defined(I86)
    asm
    {
      mov al, byte ptr c;
      int 0x29;
    }
#endif
    return;
  }
  BinaryCharIO(syscon, 1, &c, C_OUTPUT, &UnusedRetVal);
}

VOID cso(COUNT c)
{
  /* Test for hold char */
  con_hold();

  if (PrinterEcho)
    DosWrite(STDPRN, 1, (BYTE FAR *) & c, & UnusedRetVal);

  switch (c)
  {
    case CR:
      scr_pos = 0;
      break;
    case LF:
    case BELL:
      break;
    case BS:
      if (scr_pos > 0)
        scr_pos--;
      break;
    case HT:
      do
        _cso(' ');
      while ((++scr_pos) & 7);
      break;
    default:
      scr_pos++;
  }
  if (c != HT)
    _cso(c);
}

VOID sto(COUNT c)
{
  DosWrite(STDOUT, 1, (BYTE FAR *) & c, & UnusedRetVal);
}

unsigned mod_cso(register unsigned c)
{
  if (c < ' ' && c != HT)
  {
    cso('^');
    cso(c + '@');
  }
  else
    cso(c);
  return c;
}

VOID destr_bs(void)
{
  cso(BS);
  cso(' ');
  cso(BS);
}

VOID Do_DosIdle_loop(void)
{
  FOREVER
  {
    if (!StdinBusy())
      return;
    else
    {
      DosIdle_int();
      continue;
    }
  }
}

COUNT ndread(void)
{
  CharReqHdr.r_unit = 0;
  CharReqHdr.r_status = 0;
  CharReqHdr.r_command = C_NDREAD;
  CharReqHdr.r_length = sizeof(request);
  execrh((request FAR *) & CharReqHdr, syscon);
  if (CharReqHdr.r_status & S_BUSY)
    return -1;
  return CharReqHdr.r_ndbyte;
}

COUNT con_read(void)
{
  BYTE c;

  BinaryCharIO(syscon, 1, &c, C_INPUT, &UnusedRetVal);
  if (c == CTL_C)
    handle_break();
  return c;
}

VOID con_hold(void)
{
  UBYTE c = ndread();
  if (c == CTL_S)
  {
    con_read();
    Do_DosIdle_loop();
    /* just wait */
    c = con_read();
  }
  if (c == CTL_C)
  {
    con_read();
    handle_break();
  }
}

UCOUNT _sti(BOOL check_break)
{
  UBYTE c;
  /*
   * XXX: If there's a read error, this will just keep retrying the read until
   * the error disappears. Maybe it should do something else instead. -- ror4
   */
  Do_DosIdle_loop();
  if (check_break)
    con_hold();
  while (BinaryRead(STDIN, &c, & UnusedRetVal) != 1)
    ;
  return c;
}

BOOL con_break(void)
{
  if (ndread() == CTL_C)
  {
    con_read();
    return TRUE;
  }
  else
    return FALSE;
}

BOOL StdinBusy(void)
{
  sft FAR *s;

  if ((s = get_sft(STDIN)) == (sft FAR *) - 1)
    return FALSE;               /* XXX */
  if (s->sft_count == 0 || (s->sft_mode & SFT_MWRITE))
    return FALSE;               /* XXX */
  if (s->sft_flags & SFT_FDEVICE)
  {
    CharReqHdr.r_unit = 0;
    CharReqHdr.r_status = 0;
    CharReqHdr.r_command = C_ISTAT;
    CharReqHdr.r_length = sizeof(request);
    execrh((request FAR *) & CharReqHdr, s->sft_dev);
    if (CharReqHdr.r_status & S_BUSY)
      return TRUE;
    else
      return FALSE;
  }
  else
    return FALSE;               /* XXX */
}

VOID KbdFlush(void)
{
  CharReqHdr.r_unit = 0;
  CharReqHdr.r_status = 0;
  CharReqHdr.r_command = C_IFLUSH;
  CharReqHdr.r_length = sizeof(request);
  execrh((request FAR *) & CharReqHdr, syscon);
}

/* reads a line */
void sti_0a(keyboard FAR * kp)
{
  REG UWORD c, cu_pos = scr_pos;
  unsigned count = 0, stored_pos = 0, size = kp->kb_size, stored_size = kp->kb_count;
  BOOL insert = FALSE;

  if (size == 0)
    return;

  /* the stored line is invalid unless it ends with a CR */
  if (kp->kb_buf[stored_size] != CR)
  {
    stored_size = 0;
  }
      
  while ((c = _sti(TRUE)) != CR)
  {
    switch (c)
    {
      case CTL_C:
        handle_break();
      case CTL_F:
        continue;

#ifndef NOSPCL
      case SPCL:
        switch (c = _sti(TRUE))
        {
          case LEFT:
            goto backspace;

          case RIGHT:
          case F1:
            if (stored_pos < stored_size && count < size - 1)
              local_buffer[count++] = mod_cso(kp->kb_buf[stored_pos++]);
            break;
            
          case F2:
            c = _sti(TRUE);
            /* insert up to character c */
        insert_to_c:            
            while (stored_pos < stored_size && count < size - 1)
            {
              char c2 = kp->kb_buf[stored_pos];
              if (c2 == c)
                break;
              stored_pos++;
              local_buffer[count++] = mod_cso(c2);
            }
            break;
            
          case F3:
            c = (unsigned)-1;
            goto insert_to_c;

          case F4:
            c = _sti(TRUE); 
            /* delete up to character c */
            while (stored_pos < stored_size && c != kp->kb_buf[stored_pos])
              stored_pos++;
            break;
            
          case F5:
            fmemcpy(kp->kb_buf, local_buffer, count);
            stored_size = count;
            cso('@');
            goto start_new_line;

          case F6:
            c = CTL_Z;
            goto default_case;

          case INS:
            insert = !insert;
            break;
            
          case DEL:
            stored_pos++;
            break;
        }
        break;
#endif

      case CTL_BS:
      case BS:
      backspace:
        if (count > 0)
        {
          unsigned new_pos;
          c = local_buffer[--count];
          if (c == HT)
          {
            unsigned i;
            new_pos = cu_pos;
            for (i = 0; i < count; i++)
            {
              if (local_buffer[i] == HT)
                new_pos = (new_pos + 8) & ~7;
              else if (local_buffer[i] < ' ')
                new_pos += 2;
              else
                new_pos++;
            }
            do
              destr_bs();
            while (scr_pos > new_pos);
          }
          else
          {
            if (c < ' ')
              destr_bs();
            destr_bs();
          }
        }
        if (stored_pos > 0 && !insert)
          stored_pos--;
        break;

      case LF:
        cso(CR);
        cso(LF);
        break;

      case ESC:
        cso('\\');
    start_new_line:
        cso(CR);
        cso(LF);
        for (c = 0; c < cu_pos; c++)
          cso(' ');
        count = 0;
        stored_pos = 0;
        insert = FALSE;
        break;

        /* fall through */
      default:
    default_case:
        if (count < size - 1)
          local_buffer[count++] = mod_cso(c);
        else
          cso(BELL);
        if (stored_pos < stored_size && !insert)
          stored_pos++;
        break;
    }
  }
  local_buffer[count] = CR;
  cso(CR);
  fmemcpy(kp->kb_buf, local_buffer, count + 1);
  /* if local_buffer overflows into the CON default buffer we
     must invalidate it */
  if (count > LINEBUFSIZECON)
    kb_buf.kb_size = 0;
  kp->kb_count = count;
}

unsigned sti(unsigned n, char FAR * bp)
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
    sti_0a(&kb_buf);
    bufend = &kb_buf.kb_buf[kb_buf.kb_count + 2];
    bufend[-1] = LF;
    cso(LF);
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

/*
 * Log: chario.c,v - for newer logs do "cvs log chario.c"
 *
 * Revision 1.7  2000/03/09 06:07:10  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.6  1999/09/23 04:40:45  jprice
 * *** empty log message ***
 *
 * Revision 1.4  1999/08/25 03:18:07  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.3  1999/04/16 12:21:21  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.2  1999/04/04 18:51:42  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:45  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/04 03:18:37  jprice
 * Formating.  Added comments.
 *
 * Revision 1.3  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.9   06 Dec 1998  8:43:36   patv
 * changes in character I/O because of new I/O subsystem.
 *
 *    Rev 1.8   11 Jan 1998  2:06:08   patv
 * Added functionality to ioctl.
 *
 *    Rev 1.7   08 Jan 1998 21:36:40   patv
 * Changed automatic requestic packets to static to save stack space.
 *
 *    Rev 1.6   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   30 Dec 1997  4:00:20   patv
 * Modified to support SDA
 *
 *    Rev 1.4   16 Jan 1997 12:46:36   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:15:12   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:42   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:05:44   patv
 * Initial revision.
 *
 */
