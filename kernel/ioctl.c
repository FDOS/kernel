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
static BYTE *RcsId =
    "$Id$";
#endif

/*
 * WARNING:  this code is non-portable (8086 specific).
 */

/*  TE 10/29/01

	although device drivers have only 20 pushes available for them,
	MS NET plays by its own rules

	at least TE's network card driver DM9PCI (some 10$ NE2000 clone) does:
	with SP=8DC before calling down to execrh, and SP=8CC when 
	callf [interrupt], 	DM9PCI touches DOSDS:792, 
	14 bytes into error stack :-(((
	
	so some optimizations were made.		
	this uses the fact, that only CharReq device buffer is ever used.
	fortunately, this saves some code as well :-)

*/

COUNT DosDevIOctl(lregs * r)
{
  static UBYTE cmds [] = {
	0, 0,
	/* 0x02 */ C_IOCTLIN,
	/* 0x03 */ C_IOCTLOUT,
	/* 0x04 */ C_IOCTLIN,
	/* 0x05 */ C_IOCTLOUT,
	/* 0x06 */ C_ISTAT,
	/* 0x07 */ C_OSTAT,
	/* 0x08 */ C_REMMEDIA,
	0, 0, 0,
	/* 0x0c */ C_GENIOCTL,
	/* 0x0d */ C_GENIOCTL,
	/* 0x0e */ C_GETLDEV,
	/* 0x0f */ C_SETLDEV,
	/* 0x10 */ C_IOCTLQRY,
	/* 0x11 */ C_IOCTLQRY,
  };
  static UWORD required_attr [] = {
	0, 0,
	/* 0x02 */ ATTR_IOCTL,
	/* 0x03 */ ATTR_IOCTL,
	/* 0x04 */ ATTR_IOCTL,
	/* 0x05 */ ATTR_IOCTL,
	0, 0,
	/* 0x08 */ ATTR_EXCALLS,
	0, 0, 0,
	/* 0x0c */ ATTR_GENIOCTL,
	/* 0x0d */ ATTR_GENIOCTL,
	/* 0x0e */ ATTR_GENIOCTL,
	/* 0x0f */ ATTR_GENIOCTL,
	/* 0x10 */ ATTR_QRYIOCTL,
	/* 0x11 */ ATTR_QRYIOCTL,
  };

  sft FAR *s;
  struct dhdr FAR *dev;
  struct dpb FAR *dpbp;
  unsigned attr, flags;
  UBYTE cmd, unit;

  switch (r->AL)
  {
    default: /* 0x12+ */
      return DE_INVLDFUNC;

    case 0x0b:
      if (r->DX)		/* skip, it's a special case		*/
        NetRetry = r->DX;
      NetDelay = r->CX;
      return SUCCESS;

    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x06:
    case 0x07:
    case 0x0a:
    case 0x0c:
    case 0x10:
      /* Test that the handle is valid and                    */
      /* get the SFT block that contains the SFT              */
      if ((s = get_sft(r->BX)) == (sft FAR *)-1)
        return DE_INVLDHNDL;
      flags = s->sft_flags;
      dev = s->sft_dev;
      attr = dev->dh_attr;
      break;

    case 0x0d:
      /* NOTE: CX checked before check if get_dpb()->dpb_device->dh_attr
         contains ATTR_GENIOCTL bit set
      */
      if ((r->CX & ~0x4021) == 0x084A)
      {			/* 084A/484A, 084B/484B, 086A/486A, 086B/486B */
        r->AX = 0;	/* (lock/unlock logical/physical volume) */
        return SUCCESS;	/* simulate success for MS-DOS 7+ SCANDISK etc. --LG */
      }

    case 0x04:
    case 0x05:
    case 0x08:
    case 0x09:
    case 0x0e:
    case 0x0f:
    case 0x11:
/*
   Line below previously returned the deviceheader at r->bl. But,
   DOS numbers its drives starting at 1, not 0. A=1, B=2, and so
   on. Changed this line so it is now zero-based. --SRM
 */
/* changed to use default drive if drive=0. --JPP */
/* Fixed it. --JT */

#define NDN_HACK
#ifdef NDN_HACK
/* NDN feeds the actual ASCII drive letter to this function */
    unit = (r->BL & 0x1f) - 1;
#else
    unit = r->BL - 1;
#endif
    if (unit == 0xff)
      unit = default_drive;

    if ((dpbp = get_dpb(unit)) == NULL)
    {
      if (r->AL != 0x09)
        return DE_INVLDDRV;
      attr = ATTR_REMOTE;
    }
    else
    {
      dev = dpbp->dpb_device;
      attr = dev->dh_attr;
    }
  } /* switch */

  /* required_attr[] may be zero and in this case attr ignored */
  if (~attr & required_attr [r->AL])
    return DE_INVLDFUNC;

  /* commonly used, shouldn't harm to do front up */
  CharReqHdr.r_command = cmd = cmds [r->AL];
  if (cmd == C_GENIOCTL || cmd == C_IOCTLQRY)
  {
    CharReqHdr.r_cat = r->CH;            /* category (major) code */
    CharReqHdr.r_fun = r->CL;            /* function (minor) code */
    CharReqHdr.r_si = r->SI;             /* contents of SI and DI */
    CharReqHdr.r_di = r->DI;
    CharReqHdr.r_io = MK_FP(r->DS, r->DX);    /* parameter block */
  }
  else
  {
    CharReqHdr.r_count = r->CX;
    CharReqHdr.r_trans = MK_FP(r->DS, r->DX);
  }
  CharReqHdr.r_length = sizeof(request);
  CharReqHdr.r_unit = dpbp->dpb_subunit;
  CharReqHdr.r_status = 0;

  switch (r->AL)
  {
    case 0x00:
          /* Get the flags from the SFT                           */
          r->DX = flags;
          if (flags & SFT_FDEVICE)
            r->DH = attr >> 8;
          /* Undocumented result, Ax = Dx seen using Pcwatch */
          r->AX = r->DX;
          break;

    case 0x01:
          /* can't set the status of a file.                      */
          if (!(flags & SFT_FDEVICE))
            return DE_INVLDFUNC;
          /* RBIL says this is only for DOS < 6, but MSDOS 7.10   */
          /* returns this as well... and some buggy program relies*/
          /* on it :(                                             */
          if (r->DH != 0)
            return DE_INVLDDATA;

          /* Set it to what we got in the DL register from the    */
          /* user.                                                */
          s->sft_flags_lo = SFT_FDEVICE | r->DL;
          /* Undocumented: AL should get the old value            */
          r->AL = (UBYTE)flags;
          break;

    case 0x0a:
          r->DX = flags;
          r->AX = 0; /* ??? RBIL doesn't says that AX changed --avb */
          break;

    case 0x06:
          if (!(flags & SFT_FDEVICE))
          {
            r->AL = s->sft_posit >= s->sft_size ? (UBYTE)0 : (UBYTE)0xFF;
            break;
          }
          /* fall through */

    case 0x07:
          if (!(flags & SFT_FDEVICE))
          {
            r->AL = 0;
            break;
          }
          /* fall through */

    case 0x02:
    case 0x03:
    case 0x0c:
    case 0x10:
          if (!(flags & SFT_FDEVICE))
            return DE_INVLDFUNC;
          CharReqHdr.r_unit = 0; /* ??? not used for devices --avb */
          goto execrequest;

    case 0x09:
        {
          const struct cds FAR *cdsp = get_cds(unit);
          if (cdsp == NULL)
            return DE_INVLDDRV;
          if (cdsp->cdsFlags & CDSSUBST)
            attr |= ATTR_SUBST;
          r->DX = attr;
          r->AX = S_DONE | S_BUSY; /* ??? S_* values only for driver interface;
                                      RBIL doesn't says that AX changed --avb */
          break;
        }

    case 0x04:
    case 0x05:
    case 0x08:
    case 0x0d:
    case 0x11:
    execrequest:
          execrh(&CharReqHdr, dev);
          if (CharReqHdr.r_status & S_ERROR)
          {
            CritErrCode = (CharReqHdr.r_status & S_MASK) + 0x13;
            return DE_DEVICE;
          }

          if (r->AL <= 0x05)		/* 0x02, 0x03, 0x04, 0x05 */
            r->AX = CharReqHdr.r_count;
          else if (r->AL <= 0x07)	/* 0x06, 0x07 */
            r->AX = (CharReqHdr.r_status & S_BUSY) ? 0000 : 0x00ff;
          else if (r->AL == 0x08)	/* 0x08 */
            r->AX = (CharReqHdr.r_status / S_BUSY) & 1u;
          else				/* 0x0c, 0x0d, 0x10, 0x11 */
            r->AX = CharReqHdr.r_status;
          break;

    case 0x0e:
    default: /* 0x0f */
          execrh(&CharReqHdr, dev);
          if (CharReqHdr.r_status & S_ERROR)
          {
            CritErrCode = (CharReqHdr.r_status & S_MASK) + 0x13;
            return DE_ACCESS;
          }
          r->AL = CharReqHdr.r_unit;
  } /* switch */
  return SUCCESS;
}
