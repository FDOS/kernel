/****************************************************************/
/*                                                              */
/*                          ioctl.c                             */
/*                                                              */
/*                    DOS-C ioctl system call                   */
/*                                                              */
/*                    Copyright (c) 1995,1998                   */
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
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.5  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.4  2000/05/17 19:15:12  jimtabor
 * Cleanup, add and fix source.
 *
 * Revision 1.3  2000/05/11 04:26:26  jimtabor
 * Added code for DOS FN 69 & 6C
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.4  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.3  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:09  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.7   06 Dec 1998  8:48:22   patv
 * Expanded due to new I/O subsystem.
 *
 *    Rev 1.6   11 Jan 1998  2:06:22   patv
 * Added functionality to ioctl.
 *
 *    Rev 1.5   04 Jan 1998 23:15:18   patv
 * Changed Log for strip utility
 *
 *    Rev 1.4   16 Jan 1997 12:46:54   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:03:30   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   19 Feb 1996  3:21:34   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.1   01 Sep 1995 17:54:16   patv
 * First GPL release.
 *
 *    Rev 1.0   02 Jul 1995  8:32:04   patv
 * Initial revision.
 */

#ifdef PROTO
sft FAR *get_sft(COUNT);
#else
sft FAR *get_sft();
#endif

/*
 * WARNING:  this code is non-portable (8086 specific).
 */

COUNT DosDevIOctl(iregs FAR * r, COUNT FAR * err)
{
  sft FAR *s;
  struct dpb FAR *dpbp;
  struct cds FAR *cdsp;
  BYTE FAR *pBuffer = MK_FP(r->DS, r->DX);
  COUNT nMode;

  /* Test that the handle is valid                                */
  switch (r->AL)
  {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x06:
    case 0x07:
    case 0x0a:
    case 0x0c:

      /* Get the SFT block that contains the SFT              */
      if ((s = get_sft(r->BX)) == (sft FAR *) - 1)
      {
        *err = DE_INVLDHNDL;
        return 0;
      }
      break;

    case 0x04:
    case 0x05:
    case 0x08:
    case 0x09:
    case 0x0d:
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x11:

/*
   This line previously returned the deviceheader at r->bl. But,
   DOS numbers its drives starting at 1, not 0. A=1, B=2, and so
   on. Changed this line so it is now zero-based.

   -SRM
 */
/* JPP - changed to use default drive if drive=0 */
/* JT Fixed it */

      r->BL = ( r->BL == 0 ? default_drive : r->BL - 1);


      if (r->BL > lastdrive)
      {
        *err = DE_INVLDDRV;
        return 0;
      }
      else
      {
        cdsp = &CDSp->cds_table[r->BL];
        dpbp = cdsp->cdsDpb;
      }
      break;

    case 0x0b:
      /* skip, it's a special case.                           */

      NetDelay = r->CX;
      if (!r->DX)
        NetRetry = r->DX;
      break;

    default:
      *err = DE_INVLDFUNC;
      return 0;
  }

  switch (r->AL)
  {
    case 0x00:
      /* Get the flags from the SFT                           */
      r->DX = r->AX = s->sft_flags;

/*      r->DX = r->AX = s->sft_dev->dh_attr;*/

      break;

    case 0x01:
      /* sft_flags is a file, return an error because you     */
      /* can't set the status of a file.                      */
      if (!(s->sft_flags & SFT_FDEVICE))
      {
        *err = DE_INVLDFUNC;
        return 0;
      }

      /* Set it to what we got in the DL register from the    */
      /* user.                                                */
      r->AL = s->sft_flags_lo = SFT_FDEVICE | r->DL;
      break;

    case 0x0c:
      nMode = C_GENIOCTL;
      goto IoCharCommon;
    case 0x02:
      nMode = C_IOCTLIN;
      goto IoCharCommon;
    case 0x10:
      nMode = C_IOCTLQRY;
      goto IoCharCommon;
    case 0x03:
      nMode = C_IOCTLOUT;
    IoCharCommon:
      if ((s->sft_flags & SFT_FDEVICE)
            || ((r->AL == 0x02 ) && (s->sft_dev->dh_attr & SFT_FIOCTL))
            || ((r->AL == 0x03 ) && (s->sft_dev->dh_attr & SFT_FIOCTL))
            || ((r->AL == 0x10) && (s->sft_dev->dh_attr & ATTR_QRYIOCTL))
            || ((r->AL == 0x0c) && (s->sft_dev->dh_attr & ATTR_GENIOCTL)))
      {
          CharReqHdr.r_unit = 0;
          CharReqHdr.r_length = sizeof(request);
          CharReqHdr.r_command = nMode;
          CharReqHdr.r_count = r->CX;
          CharReqHdr.r_trans = pBuffer;
          CharReqHdr.r_status = 0;
          execrh((request FAR *) & CharReqHdr, s->sft_dev);

          if (CharReqHdr.r_status & S_ERROR)
            return char_error(&CharReqHdr, s->sft_dev);
          if (r->AL == 0x07)
          {
            r->AL =
                CharReqHdr.r_status & S_BUSY ?
                00 : 0xff;
          }
          break;
      }
      *err = DE_INVLDFUNC;
      return 0;

    case 0x0d:
      nMode = C_GENIOCTL;
      goto IoBlockCommon;
    case 0x04:
      nMode = C_IOCTLIN;
      goto IoBlockCommon;
    case 0x11:
      nMode = C_IOCTLQRY;
      goto IoBlockCommon;
    case 0x05:
      nMode = C_IOCTLOUT;
    IoBlockCommon:
      if ( ((r->AL == 0x04 ) && !(dpbp->dpb_device->dh_attr & ATTR_IOCTL))
            || ((r->AL == 0x05 ) && !(dpbp->dpb_device->dh_attr & ATTR_IOCTL))
            || ((r->AL == 0x11) && !(dpbp->dpb_device->dh_attr & ATTR_QRYIOCTL))
            || ((r->AL == 0x0d) && !(dpbp->dpb_device->dh_attr & ATTR_GENIOCTL)))
      {
        *err = DE_INVLDFUNC;
        return 0;
      }

      CharReqHdr.r_unit = r->BL;
      CharReqHdr.r_length = sizeof(request);
      CharReqHdr.r_command = nMode;
      CharReqHdr.r_count = r->CX;
      CharReqHdr.r_trans = pBuffer;
      CharReqHdr.r_status = 0;
      execrh((request FAR *) & CharReqHdr,
             dpbp->dpb_device);
      if (r->AL == 0x08)
      {
        if (CharReqHdr.r_status & S_ERROR)
        {
          *err = DE_DEVICE;
          return 0;
        }
        r->AX = (CharReqHdr.r_status & S_BUSY) ? 1 : 0;
      }
      else
      {
        if (CharReqHdr.r_status & S_ERROR)
        {
          *err = DE_DEVICE;
          return 0;
        }
      }
      break;

    case 0x06:
      if (s->sft_flags & SFT_FDEVICE)
      {
        r->AL = s->sft_flags & SFT_FEOF ? 0 : 0xFF;
      }
      else
        r->AL = s->sft_posit >= s->sft_size ? 0xFF : 0;
      break;

    case 0x07:
      if (s->sft_flags & SFT_FDEVICE)
      {
        goto IoCharCommon;
      }
      r->AL = 0;
      break;

    case 0x08:
      if (dpbp->dpb_device->dh_attr & ATTR_EXCALLS)
      {
        nMode = C_REMMEDIA;
        goto IoBlockCommon;
      }
      *err = DE_INVLDFUNC;
      return 0;

    case 0x09:
      if(cdsp->cdsFlags & CDSNETWDRV)
        r->DX = ATTR_REMOTE;
      else
        r->DX = dpbp->dpb_device->dh_attr;
      break;

    case 0x0a:
      r->DX = s->sft_flags & SFT_FSHARED;
      break;

    case 0x0e:
      nMode = C_GETLDEV;
      goto IoLogCommon;
    case 0x0f:
      nMode = C_SETLDEV;
    IoLogCommon:
      if ((dpbp->dpb_device->dh_attr & ATTR_GENIOCTL))
      {
        if (r->BL == 0)
          r->BL = default_drive;

        CharReqHdr.r_unit = r->BL;
        CharReqHdr.r_length = sizeof(request);
        CharReqHdr.r_command = nMode;
        CharReqHdr.r_count = r->CX;
        CharReqHdr.r_trans = pBuffer;
        CharReqHdr.r_status = 0;
        execrh((request FAR *) & CharReqHdr,
               dpbp->dpb_device);

        if (CharReqHdr.r_status & S_ERROR)
          *err = DE_ACCESS;
        else
          *err = SUCCESS;
        return 0;
      }
      *err = DE_INVLDFUNC;
      return 0;

    default:
      *err = DE_INVLDFUNC;
      return 0;
  }
  *err = SUCCESS;
  return 0;
}

