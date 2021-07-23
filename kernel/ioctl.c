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
    "$Id: ioctl.c 1427 2009-06-09 12:23:14Z bartoldeman $";
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

/* this is a file scope static because with Turbo C 2.01 "static const" does
 * not work correctly inside the function */
STATIC const UBYTE cmd [] = {
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

int DosDevIOctl(lregs * r)
{
  struct dhdr FAR *dev;

  if (r->AL > 0x11)
    return DE_INVLDFUNC;

  switch (r->AL)
  {
    case 0x0b:
      /* skip, it's a special case.                           */
      NetDelay = r->CX;
      if (r->DX)
        NetRetry = r->DX;
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
    {
      sft FAR *s;
      unsigned flags;

      /* Test that the handle is valid and                    */
      /* get the SFT block that contains the SFT              */
      if ((s = get_sft(r->BX)) == (sft FAR *) - 1)
        return DE_INVLDHNDL;

      flags = s->sft_flags;

      switch (r->AL)
      {
        case 0x00:
          /* Get the flags from the SFT                           */
          r->AX = flags & 0xff;
          if (flags & SFT_FDEVICE)
            r->AX |= (s->sft_dev->dh_attr & 0xff00);
          /* else: files/networks return 0 in AH/DH */
          /* Undocumented result, Ax = Dx seen using Pcwatch */
          r->DX = r->AX;
          return SUCCESS;

        case 0x01:
          /* sft_flags is a file, return an error because you     */
          /* can't set the status of a file.                      */
          if (!(flags & SFT_FDEVICE))
            return DE_INVLDFUNC;
          /* RBIL says this is only for DOS < 6, but MSDOS 7.10   */
          /* returns this as well... and some buggy program relies*/
          /* on it :(                                             */
          if (r->DH != 0)
            return DE_INVLDDATA;

          /* Undocumented: AL should get the old value            */
          r->AL = s->sft_flags_lo;
          /* Set it to what we got in the DL register from the    */
          /* user.                                                */
          s->sft_flags_lo = SFT_FDEVICE | r->DL;
          return SUCCESS;

        case 0x0a:
          r->DX = flags;
          r->AX = 0;
          return SUCCESS;
      }
      if (!(flags & SFT_FDEVICE))
      {
        if (r->AL == 0x06)
          r->AL = s->sft_posit >= s->sft_size ? 0 : 0xFF;
        else if (r->AL == 0x07)
          r->AL = 0;
        else
          return DE_INVLDFUNC;
        return SUCCESS;
      }
      dev = s->sft_dev;
      CharReqHdr.r_unit = 0;
      break;
    }

    default: /* block IOCTL: 4, 5, 8, 9, d, e, f, 11 */
    {
      struct dpb FAR *dpbp;
      unsigned attr;
/*
   This line previously returned the deviceheader at r->bl. But,
   DOS numbers its drives starting at 1, not 0. A=1, B=2, and so
   on. Changed this line so it is now zero-based.

   -SRM
 */
/* JPP - changed to use default drive if drive=0 */
/* JT Fixed it */

      /* NDN feeds the actual ASCII drive letter to this function */
      dpbp = get_dpb((r->BL & 0x1f) == 0 ? default_drive : (r->BL & 0x1f) - 1);
      if (dpbp)
      {
        CharReqHdr.r_unit = dpbp->dpb_subunit;
        dev = dpbp->dpb_device;
        attr = dev->dh_attr;
      }
      else
      {
        if (r->AL != 9)
          return DE_INVLDDRV;
        dev = NULL;
        attr = ATTR_REMOTE;
      }

      switch (r->AL)
      {
        case 0x09:
        {
          /* note from get_dpb()                            */
          /* that if cdsp == NULL then dev must be NULL too */
          struct cds FAR *cdsp = get_cds1(r->BL & 0x1f);
          if (cdsp == NULL)
            return DE_INVLDDRV;
          if (cdsp->cdsFlags & CDSSUBST)
            attr |= ATTR_SUBST;
          r->AX = S_DONE | S_BUSY;
          r->DX = attr;
          return SUCCESS;
        }
        case 0x0d:
          if ((r->CX & ~(0x486B-0x084A)) == 0x084A)
          {             /* 084A/484A, 084B/484B, 086A/486A, 086B/486B */
            r->AX = 0;  /* (lock/unlock logical/physical volume) */
            /* simulate success for MS-DOS 7+ SCANDISK etc. --LG */
            return SUCCESS;
          }
          /* fall through */
        default: /* 0x04, 0x05, 0x08, 0x0e, 0x0f, 0x11 */
          break;
      }
      break;
    }
  }

  {
    unsigned testattr = ATTR_QRYIOCTL;
    if (r->AL<=0x0f)
      testattr = ATTR_GENIOCTL;
    if (r->AL<=0x08)
      testattr = ATTR_EXCALLS;
    if (r->AL<=0x07)
      testattr = 0xffff;
    if (r->AL<=0x05)
      testattr = ATTR_IOCTL;

    if (!(dev->dh_attr & testattr))
      return DE_INVLDFUNC;
  }

  CharReqHdr.r_command = cmd[r->AL];
  if (r->AL == 0x0C || r->AL == 0x0D || r->AL >= 0x10) /* generic or query */
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
  CharReqHdr.r_status = 0;

  execrh(&CharReqHdr, dev);

  if (CharReqHdr.r_status & S_ERROR)
  {
    CritErrCode = (CharReqHdr.r_status & S_MASK) + 0x13;
    return DE_ACCESS;
  }

  if (r->AL <= 0x05)                       /* 0x02, 0x03, 0x04, 0x05 */
    r->AX = CharReqHdr.r_count;
  else if (r->AL <= 0x07)                  /* 0x06, 0x07 */
    r->AX = (CharReqHdr.r_status & S_BUSY) ? 0000 : 0x00ff;
  else if (r->AL == 0x08)                  /* 0x08 */
    r->AX = (CharReqHdr.r_status & S_BUSY) ? 1 : 0;
  else if (r->AL == 0x0e || r->AL == 0x0f) /* 0x0e, 0x0f */
    r->AL = CharReqHdr.r_unit;
  else                                     /* 0x0c, 0x0d, 0x10, 0x11 */
    r->AX = CharReqHdr.r_status;
  return SUCCESS;
}
