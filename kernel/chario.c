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
static BYTE *charioRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.4  2000/05/26 19:25:19  jimtabor
 * Read History file for Change info
 *
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
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

#include "globals.h"

static BYTE *con_name = "CON";

#ifdef PROTO
VOID kbfill(keyboard FAR *, UCOUNT, BOOL, UWORD *);
struct dhdr FAR *finddev(UWORD attr_mask);

#else
VOID kbfill();
struct dhdr FAR *finddev();
#endif

/*      Return a pointer to the first driver in the chain that
 *      matches the attributes.
 */

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

VOID cso(COUNT c)
{
   BYTE buf = c;
   struct dhdr FAR *lpDevice;

   CharReqHdr.r_length = sizeof(request);
   CharReqHdr.r_command = C_OUTPUT;
   CharReqHdr.r_count = 1;
   CharReqHdr.r_trans = (BYTE FAR *) (&buf);
   CharReqHdr.r_status = 0;
   execrh((request FAR *) & CharReqHdr,
   lpDevice = (struct dhdr FAR *)finddev(ATTR_CONOUT));
   if (CharReqHdr.r_status & S_ERROR)
   char_error(&CharReqHdr, lpDevice);
}


VOID sto(COUNT c)
{
  static COUNT scratch;         /* make this static to save stack space */

  DosWrite(STDOUT, 1, (BYTE FAR *) & c, (COUNT FAR *) &scratch);
}

VOID mod_sto(REG UCOUNT c)
{
  if (c < ' ' && c != HT)
  {
    sto('^');
    sto(c + '@');
  }
  else
    sto(c);
}

VOID destr_bs(void)
{
  sto(BS);
  sto(' ');
  sto(BS);
}

VOID Do_DosIdle_loop(void)
{
  FOREVER
  {
    if (StdinBusy())
      return;
    else
    {
      DosIdle_int();
      continue;
    }
  }
}

UCOUNT _sti(void)
{
  static COUNT scratch;
  UBYTE c;
  /*
   * XXX: If there's a read error, this will just keep retrying the read until
   * the error disappears. Maybe it should do something else instead. -- ror4
   */
  while (GenericRead(STDIN, 1, (BYTE FAR *) & c, (COUNT FAR *) & scratch, TRUE)
         != 1) ;
  return c;
}

BOOL con_break(void)
{
  CharReqHdr.r_unit = 0;
  CharReqHdr.r_status = 0;
  CharReqHdr.r_command = C_NDREAD;
  CharReqHdr.r_length = sizeof(request);
  execrh((request FAR *) & CharReqHdr, (struct dhdr FAR *)finddev(ATTR_CONIN));
  if (CharReqHdr.r_status & S_BUSY)
    return FALSE;
  if (CharReqHdr.r_ndbyte == CTL_C)
  {
    _sti();
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
  execrh((request FAR *) & CharReqHdr, (struct dhdr FAR *)finddev(ATTR_CONIN));
}

static VOID kbfill(keyboard FAR * kp, UCOUNT c, BOOL ctlf, UWORD * vp)
{
  if (kp->kb_count > kp->kb_size)
  {
    sto(BELL);
    return;
  }
  kp->kb_buf[kp->kb_count++] = c;
  if (!ctlf)
  {
    mod_sto(c);
    *vp += 2;
  }
  else
  {
    sto(c);
    if (c != HT)
      ++ * vp;
    else
      *vp = (*vp + 8) & -8;
  }
}

VOID sti(keyboard FAR * kp)
{
  REG UWORD c,
    cu_pos = scr_pos;
  UWORD
      virt_pos = scr_pos;
  WORD init_count = kp->kb_count;
#ifndef NOSPCL
  static BYTE local_buffer[LINESIZE];
#endif

  if (kp->kb_size == 0)
    return;
  if (kp->kb_size <= kp->kb_count || kp->kb_buf[kp->kb_count] != CR)
    kp->kb_count = 0;
  FOREVER
  {

    Do_DosIdle_loop();

    switch (c = _sti())
    {
      case CTL_C:
        handle_break();
      case CTL_F:
        continue;

#ifndef NOSPCL
      case SPCL:
        switch (c = _sti())
        {
          case LEFT:
            goto backspace;

          case F3:
            {
              REG COUNT i;

              for (i = kp->kb_count; local_buffer[i] != '\0'; i++)
              {
                c = local_buffer[kp->kb_count];
                if (c == '\r' || c == '\n')
                  break;
                kbfill(kp, c, FALSE, &virt_pos);
              }
              break;
            }

          case RIGHT:
            c = local_buffer[kp->kb_count];
            if (c == '\r' || c == '\n')
              break;
            kbfill(kp, c, FALSE, &virt_pos);
            break;
        }
        break;
#endif

      case CTL_BS:
      case BS:
      backspace:
        if (kp->kb_count > 0)
        {
          if (kp->kb_buf[kp->kb_count - 1] >= ' ')
          {
            destr_bs();
            --virt_pos;
          }
          else if ((kp->kb_buf[kp->kb_count - 1] < ' ')
                   && (kp->kb_buf[kp->kb_count - 1] != HT))
          {
            destr_bs();
            destr_bs();
            virt_pos -= 2;
          }
          else if (kp->kb_buf[kp->kb_count - 1] == HT)
          {
            do
            {
              destr_bs();
              --virt_pos;
            }
            while ((virt_pos > cu_pos) && (virt_pos & 7));
          }
          --kp->kb_count;
        }
        break;

      case CR:
        kbfill(kp, CR, TRUE, &virt_pos);
        kbfill(kp, LF, TRUE, &virt_pos);
#ifndef NOSPCL
        fbcopy((BYTE FAR *) kp->kb_buf,
               (BYTE FAR *) local_buffer, (COUNT) kp->kb_count);
        local_buffer[kp->kb_count] = '\0';
#endif
        return;

      case LF:
        sto(CR);
        sto(LF);
        break;

      case ESC:
        sto('\\');
        sto(CR);
        sto(LF);
        for (c = 0; c < cu_pos; c++)
          sto(' ');
        kp->kb_count = init_count;
        break;

      default:
        kbfill(kp, c, FALSE, &virt_pos);
        break;
    }
  }
}

VOID FAR init_call_sti(keyboard FAR * kp)
{
  sti(kp);
}
