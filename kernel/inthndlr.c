/****************************************************************/
/*                                                              */
/*                          inthndlr.c                          */
/*                                                              */
/*    Interrupt Handler and Function dispatcher for Kernel      */
/*                                                              */
/*                      Copyright (c) 1995                      */
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
#define MAIN

#include "portab.h"
#include "globals.h"
#include "nls.h"
#include "win.h"
#include "debug.h"

#ifdef VERSION_STRINGS
BYTE *RcsId =
    "$Id: inthndlr.c 1709 2012-02-08 02:13:49Z perditionc $";
#endif

#ifdef TSC
STATIC VOID StartTrace(VOID);
STATIC bTraceNext = FALSE;
#endif

#if 0                           /* Very suspicious, passing structure by value??
                                   Deactivated -- 2000/06/16 ska */
/* Special entry for far call into the kernel                           */
#pragma argsused
VOID FAR int21_entry(iregs UserRegs)
{
  int21_handler(UserRegs);
}
#endif

/* Structures needed for int 25 / int 26 */
struct HugeSectorBlock {
  ULONG blkno;
  WORD nblks;
  BYTE FAR *buf;
};

/* Normal entry.  This minimizes user stack usage by avoiding local     */
/* variables needed for the rest of the handler.                        */
/* this here works on the users stack !! and only very few functions
   are allowed                                                          */

/* TODO: really, really make sure that this function works properly     */
/* even when SS != DGROUP (some changes to the compiler (!) may be      */
/* necessary).  Currently, between Turbo C++, gcc-ia16, & Open Watcom,  */
/* it seems only Watcom has explicit support for calling near-data      */
/* functions with SS != DGROUP.  The code for this function happens to  */
/* to compile under gcc-ia16 to correctly-behaving code _for now_.  But */
/* it will be better to be able to guarantee this.   -- tkchia 20191207 */

/* Update: I added experimental SS != DGROUP support, and a __seg_ss    */
/* address space qualifier, to gcc-ia16 around January 2020.  This      */
/* function now uses these if available.  In particular, the iregs *irp */
/* structure will always be on the stack, so (as far as the calling     */
/* convention permits) the function can treat the irp pointer as a      */
/* 16-bit pointer relative to SS.                    -- tkchia 20200417 */
#if defined __GNUC__ && defined  __IA16_FEATURE_ATTRIBUTE_NO_ASSUME_SS_DATA
__attribute__((no_assume_ss_data))
VOID ASMCFUNC int21_syscall(iregs __seg_ss * irp, ...)
#else
VOID ASMCFUNC int21_syscall(iregs FAR * irp)
#endif
{
  Int21AX = irp->AX;

  switch (irp->AH)
  {
    /* Set Interrupt Vector - now implemented in entry.asm          */
#if 0
    case 0x25:
      setvec(irp->AL, (intvec)MK_FP(irp->DS, irp->DX));
      break;
#endif

      /* DosVars - get/set dos variables                              */
    case 0x33:
      switch (irp->AL)
      {
          /* Set Ctrl-C flag; returns dl = break_ena              */
        case 0x01:
          break_ena = irp->DL & 1;
          /* fall through so DL only low bit (as in MS-DOS) */

          /* Get Ctrl-C flag                                      */
        case 0x00:
          irp->DL = break_ena;
          break;

        case 0x02:             /* andrew schulman: get/set extended control break  */
          {
            UBYTE tmp = break_ena;
            break_ena = irp->DL & 1;
            irp->DL = tmp;
          }
          break;

          /* Get Boot Drive                                       */
        case 0x05:
          irp->DL = BootDrive;
          break;

          /* Get (real) DOS-C version                             */
        case 0x06:
          irp->BL = os_major;
          irp->BH = os_minor;
          irp->DL = 0;                  /* lower 3 bits revision #, remaining should be 0 */
          irp->DH = version_flags;      /* bit3:runs in ROM,bit 4: runs in HMA */
          break;

     /* case 0x03: */          /* DOS 7 does not set AL */
     /* case 0x07: */          /* neither here */

        default:               /* set AL=0xFF as error, NOT carry */
          irp->AL = 0xff;
          break;

        /* the remaining are FreeDOS extensions */

                        /* return CPU family */
        case 0xfa:
          irp->AL = CPULevel;
          break;

#if 0           /* unknown if used / usage */
                case 0xfb:
#endif

#if 1      /* duplicates DOS 4 int 2F/122Fh, but used by CALLVER */
           /* set FreeDOS returned version for int 21.30 from BX */
        case 0xfc:
          os_setver_major = irp->BL;
          os_setver_minor = irp->BH;
          break;
#endif

          /* Toggle DOS-C rdwrblock trace dump                    */
#ifdef DEBUG
        case 0xfd:
          bDumpRdWrParms = !bDumpRdWrParms;
          break;
#endif

          /* Toggle DOS-C syscall trace dump                      */
#ifdef DEBUG
        case 0xfe:
          bDumpRegs = !bDumpRegs;
          break;
#endif

          /* Get DOS-C release string pointer                     */
        case 0xff:
#if !defined __GNUC__ || defined  __IA16_FEATURE_ATTRIBUTE_NO_ASSUME_SS_DATA
          irp->DX = FP_SEG(os_release);
#else  /* TODO: remove this hacky SS != DGROUP workaround  --tkchia 20191207 */
          asm volatile("movw %%ds, %0" : "=g" (irp->DX));
#endif
          irp->AX = FP_OFF(os_release);
      }
      break;

    /* Get Interrupt Vector - now implemented in entry.asm            */
#if 0
    case 0x35:
    {
      intvec p = getvec(irp->AL);
      irp->ES = FP_SEG(p);
      irp->BX = FP_OFF(p);
      break;
    }
#endif

      /* Set PSP                                                      */
    case 0x50:
      cu_psp = irp->BX;
      break;

      /* Get PSP                                                      */
    case 0x51:
      /* UNDOCUMENTED: return current psp                             */
    case 0x62:
      irp->BX = cu_psp;

      /* Normal DOS function - DO NOT ARRIVE HERE          */
 /* default: */
  }
}

#ifdef WITHFAT32
      /* DOS 7.0+ FAT32 extended functions */
int int21_fat32(lregs *r)
{
  COUNT rc;

  switch (r->AL)
  {
    /* Get extended drive parameter block */
    case 0x02:
    {
      struct dpb FAR *dpb;
      struct xdpbdata FAR *xddp;

      if (r->CX < sizeof(struct xdpbdata))
        return DE_INVLDBUF;

      dpb = GetDriveDPB(r->DL, &rc);
      if (rc != SUCCESS)
        return rc;

      /* hazard: no error checking! */
      flush_buffers(dpb->dpb_unit);
      dpb->dpb_flags = M_CHANGED;       /* force reread of drive BPB/DPB */

      if (media_check(dpb) < 0)
        return DE_INVLDDRV;

      xddp = MK_FP(r->ES, r->DI);

      fmemcpy(&xddp->xdd_dpb, dpb, sizeof(struct dpb));
      xddp->xdd_dpbsize = sizeof(struct dpb);

      /* if it doesn't look like an extended DPB, fill in those fields */
      if (!ISFAT32(dpb) && dpb->dpb_xsize != dpb->dpb_size)
      {
        xddp->xdd_dpb.dpb_nfreeclst_un.dpb_nfreeclst_st.dpb_nfreeclst_hi =
          (dpb->dpb_nfreeclst == 0xFFFF ? 0xFFFF : 0);
        dpb16to32(&xddp->xdd_dpb);
        xddp->xdd_dpb.dpb_xfatsize = dpb->dpb_fatsize;
        xddp->xdd_dpb.dpb_xcluster = (dpb->dpb_cluster == 0xFFFF ?
                       0xFFFFFFFFuL : dpb->dpb_cluster);
      }
      break;
    }
    /* Get extended free drive space */
    case 0x03:
    {
      struct xfreespace FAR *xfsp = MK_FP(r->ES, r->DI);

      if (r->CX < sizeof(struct xfreespace))
        return DE_INVLDBUF;

      rc = DosGetExtFree(MK_FP(r->DS, r->DX), xfsp);
      if (rc != SUCCESS)
        return rc;
      break;
    }
    /* Set DPB to use for formatting */
    case 0x04:
    {
      struct xdpbforformat FAR *xdffp = MK_FP(r->ES, r->DI);
      struct dpb FAR *dpb;
      if (r->CX < sizeof(struct xdpbforformat))
      {
        return DE_INVLDBUF;
      }
      dpb = GetDriveDPB(r->DL, &rc);
      if (rc != SUCCESS)
        return rc;

      xdffp->xdff_datasize = sizeof(struct xdpbforformat);
      xdffp->xdff_version.actual = 0;

      switch ((UWORD) xdffp->xdff_function)
      {
        case 0x00:
        {
          ULONG nfreeclst = xdffp->xdff_f.setdpbcounts.nfreeclst;
          ULONG cluster = xdffp->xdff_f.setdpbcounts.cluster;
          if (ISFAT32(dpb))
          {
            if ((dpb->dpb_xfsinfosec == 0xffff
                 && (nfreeclst != 0 || cluster != 0))
                || nfreeclst == 1 || nfreeclst > dpb->dpb_xsize
                || cluster == 1 || cluster > dpb->dpb_xsize)
            {
              return DE_INVLDPARM;
            }
            dpb->dpb_xnfreeclst = nfreeclst;
            dpb->dpb_xcluster = cluster;
            write_fsinfo(dpb);
          }
          else
          {
            if ((unsigned)nfreeclst == 1 || (unsigned)nfreeclst > dpb->dpb_size ||
                (unsigned)cluster == 1 || (unsigned)cluster > dpb->dpb_size)
            {
              return DE_INVLDPARM;
            }
            dpb->dpb_nfreeclst = (UWORD)nfreeclst;
            dpb->dpb_cluster = (UWORD)cluster;
          }
          break;
        }
        case 0x01:
        {
          ddt *pddt = getddt(r->DL);
          fmemcpy(&pddt->ddt_bpb, xdffp->xdff_f.rebuilddpb.bpbp,
                  sizeof(bpb));
        }
        case 0x02:
        {
        rebuild_dpb:
            /* hazard: no error checking! */
          flush_buffers(dpb->dpb_unit);
          dpb->dpb_flags = M_CHANGED;

          if (media_check(dpb) < 0)
            return DE_INVLDDRV;
          break;
        }
        case 0x03:
        case 0x04:
        {
          ULONG value;
          if (!ISFAT32(dpb))
            return DE_INVLDPARM;

          value = xdffp->xdff_f.setget.new;
          if ((UWORD) xdffp->xdff_function == 0x03)
          {
            /* FAT mirroring */
            if (value != 0xFFFFFFFFUL && (value & ~(0xf | 0x80)))
                return DE_INVLDPARM;
              xdffp->xdff_f.setget.old = dpb->dpb_xflags;
          }
          else
          {
            /* root cluster */
            if (value != 0xFFFFFFFFUL && (value < 2 || value > dpb->dpb_xsize))
              return DE_INVLDPARM;
            xdffp->xdff_f.setget.old = dpb->dpb_xrootclst;
          }
          if (value != 0xFFFFFFFFUL)
          {
            bpb FAR *bpbp;
            struct buffer FAR *bp = getblock(1, dpb->dpb_unit);
            bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT);
            bp->b_flag |= BFR_VALID | BFR_DIRTY;
            bpbp = (bpb FAR *) & bp->b_buffer[BT_BPB];
            if ((UWORD) xdffp->xdff_function == 0x03)
              bpbp->bpb_xflags = (UWORD)value;
            else
              bpbp->bpb_xrootclst = value;
          }
          goto rebuild_dpb;
        }
        default:
          return DE_INVLDFUNC;
      }

      break;
    }
    /* Extended absolute disk read/write */
    /* TODO(vlp) consider using of the 13-14th bits of SI */
    case 0x05:
    {
      struct HugeSectorBlock FAR *SectorBlock =
        (struct HugeSectorBlock FAR *)MK_FP(r->DS, r->BX);
      UBYTE mode;
      /* bit 0 of SI is 0 read / 1 write, bits 13/14 indicate a type:  */
      /* 0 any, 1 fat, 2 dir, 3 file. Type is mostly for "write hints" */

      if (r->CX != 0xffff || (r->SI & ~0x6001))
      {
        return DE_INVLDPARM;
      }

      if (r->DL > lastdrive || r->DL == 0)
        return -0x207;

      if ((r->SI & 1) == 0) /* while uncommon, reads CAN have type hints */
        mode = DSKREADINT25;
      else
        mode = DSKWRITEINT26;

      r->AX =
        dskxfer(r->DL - 1, SectorBlock->blkno, SectorBlock->buf,
                SectorBlock->nblks, mode);

      if (mode == DSKWRITEINT26)
        if (r->AX == 0)
          setinvld(r->DL - 1);

      if (r->AX > 0)
        return -0x20c;
      break;
    }
  default:
    return DE_INVLDFUNC;
  }
  return SUCCESS;
}
#endif

VOID ASMCFUNC int21_service(iregs FAR * r)
{
  COUNT rc;
  long lrc;
  lregs lr; /* 8 local registers (ax, bx, cx, dx, si, di, ds, es) */
  psp FAR *psp = MK_FP(cu_psp, 0);

#define FP_DS_DX (MK_FP(lr.DS, lr.DX))
#define FP_ES_DI (MK_FP(lr.ES, lr.DI))

#define CLEAR_CARRY_FLAG()  r->FLAGS &= ~FLG_CARRY
#define SET_CARRY_FLAG()    r->FLAGS |= FLG_CARRY

  psp->ps_stack = (BYTE FAR *) r;

  fmemcpy(&lr, r, sizeof(lregs) - 4);
  lr.DS = r->DS;
  lr.ES = r->ES;

dispatch:

#ifdef DEBUG
  if (bDumpRegs)
  {
    fmemcpy(&error_regs, user_r, sizeof(iregs));
    printf("System call (21h): %02x\n", user_r->AX);
    dump_regs = TRUE;
    dump();
  }
#endif

  if ((lr.AH >= 0x38 && lr.AH <= 0x4F) || (lr.AH >= 0x56 && lr.AH <= 0x5c) ||
      (lr.AH >= 0x5e && lr.AH <= 0x60) || (lr.AH >= 0x65 && lr.AH <= 0x6a) ||
      lr.AH == 0x6c)
  {
    CLEAR_CARRY_FLAG();
    if (lr.AH != 0x59)
      CritErrCode = SUCCESS;
  }
  /* Clear carry by default for these functions */

  /*
     what happened:
     Application does FindFirst("I:\*.*");
     this fails, and causes Int24
     this sets ErrorMode, and calls Int24
     Application decides NOT to return to DOS,
     but instead pop the stack and return to itself
     (this is legal; see RBIL/INT 24 description

     *) errormode NEVER gets set back to 0 until exit()

     I have NO idea how real DOS handles this;
     the appended patch cures the worst symptoms
  */
  if (/*ErrorMode && */lr.AH > 0x0c && lr.AH != 0x30 && lr.AH != 0x59)
  {
    /*if (ErrorMode)*/ ErrorMode = 0;
  }
  /* Check for Ctrl-Break */
  if (break_ena || (lr.AH >= 1 && lr.AH <= 5) || (lr.AH >= 8 && lr.AH <= 0x0b))
    check_handle_break(&syscon);

  /* The dispatch handler                                         */
  switch (lr.AH)
  {
      /* int 21h common error handler                                 */
    case 0x64:
        goto error_invalid;

      /* case 0x00:   --> Simulate a DOS-4C-00 */

      /* Read Keyboard with Echo                      */
    case 0x01:
  DOS_01:
      lr.AL = read_char_stdin(TRUE);
      write_char_stdout(lr.AL);
      break;

      /* Display Character                                            */
    case 0x02:
  DOS_02:
      lr.AL = lr.DL;
      write_char_stdout(lr.AL);
      break;

      /* Auxiliary Input                                                      */
    case 0x03:
    {
      int sft_idx = get_sft_idx(STDAUX);
      lr.AL = read_char(sft_idx, sft_idx, TRUE);
      break;
    }

      /* Auxiliary Output                                                     */
    case 0x04:
      write_char(lr.DL, get_sft_idx(STDAUX));
      break;
      /* Print Character                                                      */
    case 0x05:
      write_char(lr.DL, get_sft_idx(STDPRN));
      break;

      /* Direct Console I/O                                            */
    case 0x06:
  DOS_06:
      if (lr.DL != 0xff)
        goto DOS_02;

      lr.AL = 0x00;
      r->FLAGS |= FLG_ZERO;
      if (StdinBusy()) {
        DosIdle_int();
        break;
      }

      r->FLAGS &= ~FLG_ZERO;
      /* fall through */

      /* Direct Console Input                                         */
    case 0x07:
  DOS_07:
      lr.AL = read_char_stdin(FALSE);
      break;

      /* Read Keyboard Without Echo                                   */
    case 0x08:
  DOS_08:
      lr.AL = read_char_stdin(TRUE);
      break;

      /* Display String                                               */
    case 0x09:
      {
        unsigned char c;
        unsigned char FAR *bp = FP_DS_DX;

        while ((c = *bp++) != '$')
          write_char_stdout(c);

        lr.AL = c;
      }
      break;

      /* Buffered Keyboard Input                                      */
    case 0x0a:
  DOS_0A:
      read_line(get_sft_idx(STDIN), get_sft_idx(STDOUT), FP_DS_DX);
      break;

      /* Check Stdin Status                                           */
    case 0x0b:
      lr.AL = 0xFF;
      if (StdinBusy())
        lr.AL = 0x00;
      break;

      /* Flush Buffer, Read Keyboard                                 */
    case 0x0c:
    {
      struct dhdr FAR *dev = sft_to_dev(get_sft(STDIN));
      if (dev)
        con_flush(&dev);
      switch (lr.AL)
      {
      case 0x01: goto DOS_01;
      case 0x06: goto DOS_06;
      case 0x07: goto DOS_07;
      case 0x08: goto DOS_08;
      case 0x0a: goto DOS_0A;
      }
      lr.AL = 0x00;
      break;
    }

      /* Reset Drive                                                  */
    case 0x0d:
      flush();
      break;

      /* Set Default Drive                                            */
    case 0x0e:
      lr.AL = DosSelectDrv(lr.DL);
      break;

    case 0x0f:
      lr.AL = FcbOpen(FP_DS_DX, O_FCB | O_LEGACY | O_OPEN | O_RDWR);
      break;

    case 0x10:
      lr.AL = FcbClose(FP_DS_DX);
      break;

    case 0x11:
      lr.AL = FcbFindFirstNext(FP_DS_DX, TRUE);
      break;

    case 0x12:
      lr.AL = FcbFindFirstNext(FP_DS_DX, FALSE);
      break;

    case 0x13:
      lr.AL = FcbDelete(FP_DS_DX);
      break;

    case 0x14:
      /* FCB read */
      lr.AL = FcbReadWrite(FP_DS_DX, 1, XFR_READ);
      break;

    case 0x15:
      /* FCB write */
      lr.AL = FcbReadWrite(FP_DS_DX, 1, XFR_WRITE);
      break;

    case 0x16:
      lr.AL = FcbOpen(FP_DS_DX, O_FCB | O_LEGACY | O_CREAT | O_TRUNC | O_RDWR);
      break;

    case 0x17:
      lr.AL = FcbRename(FP_DS_DX);
      break;

    default:
#ifdef DEBUG
      printf("Unsupported INT21 AH = 0x%x, AL = 0x%x.\n", lr.AH, lr.AL);
#endif
      /* Fall through. */

      /* CP/M compatibility functions                                 */
    case 0x18:
    case 0x1d:
    case 0x1e:
    case 0x20:
#ifndef TSC
    case 0x61:
#endif
    case 0x6b:
      lr.AL = 0;
      break;

      /* Get Default Drive                                            */
    case 0x19:
      lr.AL = default_drive;
      break;

      /* Set DTA                                                      */
    case 0x1a:
      dta = FP_DS_DX;
      break;

      /* Get Default Drive Data                                       */
    case 0x1b:
      lr.DL = 0;
      /* fall through */

      /* Get Drive Data                                               */
    case 0x1c:
      {
        BYTE FAR *p;

        if ((p = FatGetDrvData(lr.DL, &lr.AL, &lr.CX, &lr.DX)) != NULL)
        {
          lr.DS = FP_SEG(p);
          lr.BX = FP_OFF(p);
        }
        else
          lr.AL = 0xff;  /* return 0xff on invalid drive */
      }
      break;

      /* Get default DPB                                              */
      /* case 0x1f: see case 0x32 */

      /* Random read using FCB: fields not updated
         (XFR_RANDOM should not be used here) */
    case 0x21:
      lr.AL = FcbRandomIO(FP_DS_DX, XFR_READ);
      break;

      /* Random write using FCB */
    case 0x22:
      lr.AL = FcbRandomIO(FP_DS_DX, XFR_WRITE);
      break;

      /* Get file size in records using FCB */
    case 0x23:
      lr.AL = FcbGetFileSize(FP_DS_DX);
      break;

      /* Set random record field in FCB */
    case 0x24:
      FcbSetRandom(FP_DS_DX);
      break;

      /* Set Interrupt Vector                                         */
      /* case 0x25: handled above (re-entrant)                        */

      /* Dos Create New Psp                                           */
    case 0x26:
      new_psp(lr.DX, r->CS);
      break;

      /* Read random record(s) using FCB */
    case 0x27:
      lr.AL = FcbRandomBlockIO(FP_DS_DX, &lr.CX, XFR_READ | XFR_FCB_RANDOM);
      break;

      /* Write random record(s) using FCB */
    case 0x28:
      lr.AL = FcbRandomBlockIO(FP_DS_DX, &lr.CX, XFR_WRITE | XFR_FCB_RANDOM);
      break;

      /* Parse File Name                                              */
    case 0x29:
      lr.SI = FcbParseFname(&lr.AL, MK_FP(lr.DS, lr.SI), FP_ES_DI);
      break;

      /* Get Date                                                     */
    case 0x2a:
      lr.AL = DosGetDate((struct dosdate *)&lr.CX);
      break;

      /* Set Date                                                     */
    case 0x2b:
      lr.AL = DosSetDate ((struct dosdate*)&lr.CX) == SUCCESS ? 0 : 0xFF;
      break;

      /* Get Time                                                     */
    case 0x2c:
      DosGetTime((struct dostime *)&lr.CL);
      break;

      /* Set Time                                                     */
    case 0x2d:
      lr.AL = DosSetTime ((struct dostime*)&lr.CX) == SUCCESS ? 0 : 0xFF;
      break;

      /* Set verify flag                                              */
    case 0x2e:
      verify_ena = lr.AL & 1;
      break;

      /* Get DTA                                                      */
    case 0x2f:
      lr.BX = FP_OFF(dta);
      lr.ES = FP_SEG(dta);
      break;

      /* Get (editable) DOS Version                                   */
    case 0x30:
    {
      if (lr.AL == 1) /* from RBIL, if AL=1 then return version_flags */
          lr.BH = version_flags;
      else
          lr.BH = OEM_ID;
      lr.AX = psp->ps_retdosver;
      lr.BL = REVISION_SEQ;
      lr.CX = 0; /* do not set this to a serial number!
                    32RTM won't like non-zero values   */

      if (ReturnAnyDosVersionExpected)
      {
        /* TE for testing purpose only and NOT
           to be documented:
           return programs, who ask for version == XX.YY
           exactly this XX.YY.
           this makes most MS programs more happy.
         */
        UBYTE FAR *retp = MK_FP(r->cs, r->ip);

        if (retp[0] == 0x3d &&  /* cmp ax, xxyy */
            (retp[3] == 0x75 || retp[3] == 0x74))       /* je/jne error    */
        {
          lr.AL = retp[1];
          lr.AH = retp[2];
        }
        else if (retp[0] == 0x86 &&     /* xchg al,ah   */
                 retp[1] == 0xc4 && retp[2] == 0x3d &&  /* cmp ax, xxyy */
                 (retp[5] == 0x75 || retp[5] == 0x74))  /* je/jne error    */
        {
          lr.AL = retp[4];
          lr.AH = retp[3];
        }

      }
    }

    break;

      /* Keep Program (Terminate and stay resident) */
    case 0x31:
      DosMemChange(cu_psp, lr.DX < 6 ? 6 : lr.DX, 0);
      return_code = lr.AL | 0x300;
      tsr = TRUE;
      return_user();
      break;

      /* Get default BPB */
    case 0x1f:
      /* Get DPB                                                      */
    case 0x32:
      /* r->DL is NOT changed by MS 6.22 */
      /* INT21/32 is documented to reread the DPB */
      {
        int drv = (lr.DL == 0 || lr.AH == 0x1f) ? default_drive : lr.DL - 1;
        struct dpb FAR *dpb = get_dpb(drv);

        if (dpb == NULL)
        {
          CritErrCode = -DE_INVLDDRV;
          lr.AL = 0xFF;
          break;
        }
        /* hazard: no error checking! */
        flush_buffers(dpb->dpb_unit);
        dpb->dpb_flags = M_CHANGED;     /* force flush and reread of drive BPB/DPB */

#ifdef WITHFAT32
        if (media_check(dpb) < 0 || ISFAT32(dpb))
#else
        if (media_check(dpb) < 0)
#endif
        {
          lr.AL = 0xff;
          CritErrCode = -DE_INVLDDRV;
          break;
        }
        lr.DS = FP_SEG(dpb);
        lr.BX = FP_OFF(dpb);
        lr.AL = 0;
      }

      break;
/*
    case 0x33:
    see int21_syscall
*/
      /* Get InDOS flag                                               */
    case 0x34:
      lr.BX = FP_OFF(&InDOS);
      lr.ES = FP_SEG(&InDOS);
      break;

      /* Get Interrupt Vector                                         */
      /* case 0x35: handled above (reentrant)                         */

      /* Dos Get Disk Free Space                                      */
    case 0x36:
      lr.AX = DosGetFree(lr.DL, &lr.BX, &lr.CX, &lr.DX);
      break;

      /* Undocumented Get/Set Switchar                                */
    case 0x37:
      switch (lr.AL)
      {
          /* Get switch character */
        case 0x00:
          lr.DL = switchar;
          lr.AL = 0x00;
          break;

          /* Set switch character */
        case 0x01:
          switchar = lr.DL;
          lr.AL = 0x00;
          break;

        default:
          goto error_invalid;
      }
      break;

      /* Get/Set Country Info                                         */
    case 0x38:
      {
        UWORD cntry = lr.AL;

        if (cntry == 0xff)
          cntry = lr.BX;

        if (0xffff == lr.DX)
        {
          /* Set Country Code */
          rc = DosSetCountry(cntry);
        }
        else
        {
          if (cntry == 0)
            cntry--;
          /* Get Country Information */
          rc = DosGetCountryInformation(cntry, FP_DS_DX);
          if (rc >= SUCCESS)
          {
            if (cntry == (UWORD) - 1)
              cntry = nlsInfo.actPkg->cntry;
            lr.AX = lr.BX = cntry;
          }
        }
        goto short_check;
      }

      /* Dos Create Directory                                         */
    case 0x39:
      /* Dos Remove Directory                                         */
    case 0x3a:
      rc = DosMkRmdir(FP_DS_DX, lr.AH);
      goto short_check;

      /* Dos Change Directory                                         */
    case 0x3b:
      rc = DosChangeDir(FP_DS_DX);
      goto short_check;

      /* Dos Create File                                              */
    case 0x3c:
      lrc = DosOpen(FP_DS_DX, O_LEGACY | O_RDWR | O_CREAT | O_TRUNC, lr.CL);
      goto long_check;

      /* Dos Open                                                     */
    case 0x3d:
      lrc = DosOpen(FP_DS_DX, O_LEGACY | O_OPEN | lr.AL, 0);
      goto long_check;

      /* Dos Close                                                    */
    case 0x3e:
      rc = DosClose(lr.BX);
      goto short_check;

      /* Dos Read                                                     */
    case 0x3f:
      lrc = DosRead(lr.BX, lr.CX, FP_DS_DX);
      goto long_check;

      /* Dos Write                                                    */
    case 0x40:
      lrc = DosWrite(lr.BX, lr.CX, FP_DS_DX);
      goto long_check;

      /* Dos Delete File                                              */
    case 0x41:
      rc = DosDelete((BYTE FAR *) FP_DS_DX, D_ALL);
      goto short_check;

      /* Dos Seek                                                     */
    case 0x42:
      if (lr.AL > 2)
        goto error_invalid;
      lrc = DosSeek(lr.BX, (LONG)MK_ULONG(lr.CX, lr.DX), lr.AL, &rc);
      if (rc == SUCCESS)
      {
        lr.DX = (UWORD)(lrc >> 16);
        lr.AX = (UWORD) lrc;
      }
      goto short_check;

      /* Get/Set File Attributes                                      */
    case 0x43:
      switch (lr.AL)
      {
        case 0x00:
          rc = DosGetFattr((BYTE FAR *) FP_DS_DX);
          if (rc >= SUCCESS)
            lr.CX = rc;
          break;

        case 0x01:
          rc = DosSetFattr((BYTE FAR *) FP_DS_DX, lr.CX);
          lr.AX = lr.CX;
          break;

#if 0
        case 0x02:
            /* get compressed size -> compression not support returns size in clusters */
            /* rc = DosGetClusterCnt((BYTE FAR *) FP_DS_DX); */
          goto error_invalid;
#endif

        case 0xff: /* DOS 7.20 (w98) extended name (128 char length) functions */
        {
          switch(lr.CL)
          {
                /* Dos Create Directory                                         */
                case 0x39:
                /* Dos Remove Directory                                         */
                case 0x3a:
                rc = DosMkRmdir(FP_DS_DX, lr.CL);
                goto short_check;

                /* Dos rename file */
                case 0x56:
                rc = DosRename(FP_DS_DX, FP_ES_DI);
                goto short_check;

              /* fall through to goto error_invaid */
          }
        }

        default:
          goto error_invalid;
      }
      goto short_check;

      /* Device I/O Control                                           */
    case 0x44:
      rc = DosDevIOctl(&lr);      /* can set critical error code! */

      if (rc < SUCCESS)
      {
        lr.AX = -rc;
        if (rc != DE_DEVICE && rc != DE_ACCESS)
          CritErrCode = lr.AX;
        goto error_carry;
      }
      break;

      /* Duplicate File Handle                                        */
    case 0x45:
      lrc = DosDup(lr.BX);
      goto long_check;

      /* Force Duplicate File Handle                                  */
    case 0x46:
      rc = DosForceDup(lr.BX, lr.CX);
      goto short_check;

      /* Get Current Directory                                        */
    case 0x47:
      rc = DosGetCuDir(lr.DL, MK_FP(lr.DS, lr.SI));
      lr.AX = 0x0100;         /*jpp: from interrupt list */
      goto short_check;

      /* Allocate memory */
    case 0x48:
      if ((rc = DosMemAlloc(lr.BX, mem_access_mode, &lr.AX, &lr.BX)) < 0)
      {
        DosMemLargest(&lr.BX);
        if (DosMemCheck() != SUCCESS)
          panic("MCB chain corrupted");
        goto error_exit;
      }
      lr.AX++;   /* DosMemAlloc() returns seg of MCB rather than data */
      break;

      /* Free memory */
    case 0x49:
      if ((rc = DosMemFree(lr.ES - 1)) < SUCCESS)
      {
        if (DosMemCheck() != SUCCESS)
          panic("MCB chain corrupted");
        goto error_exit;
      }
      break;

      /* Set memory block size */
    case 0x4a:
        if (DosMemCheck() != SUCCESS)
          panic("before 4a: MCB chain corrupted");

      if ((rc = DosMemChange(lr.ES, lr.BX, &lr.BX)) < 0)
      {
#if 0
        if (cu_psp == lr.ES)
        {
          psp->ps_size = lr.BX + cu_psp;
        }
#endif
        if (DosMemCheck() != SUCCESS)
          panic("after 4a: MCB chain corrupted");
        goto error_exit;
      }
      lr.AX = lr.ES; /* Undocumented MS-DOS behaviour expected by BRUN45! */
      break;

      /* Load and Execute Program */
    case 0x4b:
      break_flg = FALSE;

      rc = DosExec(lr.AL, MK_FP(lr.ES, lr.BX), FP_DS_DX);
      goto short_check;

      /* Terminate Program                                            */
    case 0x00:
      lr.AX = 0x4c00;

      /* End Program                                                  */
    case 0x4c:
      tsr = FALSE;
      rc = 0;
      if (ErrorMode)
      {
        ErrorMode = FALSE;
        rc = 0x200;
      }
      else if (break_flg)
      {
        break_flg = FALSE;
        rc = 0x100;
      }
      return_code = lr.AL | rc;
      if (DosMemCheck() != SUCCESS)
        panic("MCB chain corrupted");
#ifdef TSC
      StartTrace();
#endif
      return_user();
      break;

      /* Get Child-program Return Value                               */
    case 0x4d:
      lr.AX = return_code;
      /* needs to be cleared (RBIL) */
      return_code = 0;
      break;

      /* Dos Find First                                               */
    case 0x4e:
      /* dta for this call is set on entry.  This     */
      /* needs to be changed for new versions.        */
      rc = DosFindFirst(lr.CX, FP_DS_DX);
      lr.AX = 0;
      goto short_check;

      /* Dos Find Next                                                */
    case 0x4f:
      /* dta for this call is set on entry.  This     */
      /* needs to be changed for new versions.        */
      rc = DosFindNext();
      lr.AX = 0;
      goto short_check;
/*
    case 0x50:
    case 0x51:
    see int21_syscall
*/
      /* ************UNDOCUMENTED************************************* */
      /* Get List of Lists                                            */
    case 0x52:
      lr.BX = FP_OFF(&DPBp);
      lr.ES = FP_SEG(&DPBp);
      break;

    case 0x53:
      /*  DOS 2+ internal - TRANSLATE BIOS PARAMETER BLOCK TO DRIVE PARAM BLOCK */
      bpb_to_dpb((bpb FAR *) MK_FP(lr.DS, lr.SI),
                 (struct dpb FAR *)MK_FP(lr.ES, r->BP)
#ifdef WITHFAT32
                 , (lr.CX == 0x4558 && lr.DX == 0x4152)
#endif
          );
      break;

      /* Get verify state                                             */
    case 0x54:
      lr.AL = verify_ena;
      break;

      /* ************UNDOCUMENTED************************************* */
      /* Dos Create New Psp & set p_size                              */
    case 0x55:
      child_psp(lr.DX, cu_psp, lr.SI);
      /* copy command line from the parent (required for some device loaders) */
      fmemcpy(MK_FP(lr.DX, 0x80), MK_FP(cu_psp, 0x80), 128);
      cu_psp = lr.DX;
      break;

      /* Dos Rename                                                   */
    case 0x56:
      rc = DosRename(FP_DS_DX, FP_ES_DI);
      goto short_check;

      /* Get/Set File Date and Time                                   */
    case 0x57:
      switch (lr.AL)
      {
        case 0x00:
          rc = DosGetFtime((COUNT) lr.BX,       /* Handle               */
                           &lr.DX,        /* FileDate             */
                           &lr.CX);       /* FileTime             */
          break;

        case 0x01:
          rc = DosSetFtime((COUNT) lr.BX,       /* Handle               */
                           (ddate) lr.DX,       /* FileDate             */
                           (dtime) lr.CX);      /* FileTime             */
          break;

        default:
          goto error_invalid;
      }
      goto short_check;

      /* Get/Set Allocation Strategy                                  */
    case 0x58:
      switch (lr.AL)
      {
        case 0x00:
          lr.AL = mem_access_mode;
          lr.AH = 0;
          break;

        case 0x01:
          if (lr.BL > LAST_FIT_U ||                 /* 0x82       */
              (lr.BL & FIT_MASK) > LAST_FIT)        /* 0x3f, 0x02 */
            goto error_invalid;
          mem_access_mode = lr.BL;
          break;

        case 0x02:
          lr.AL = uppermem_link;
          break;

        case 0x03:
          if (uppermem_root != 0xffff)    /* always error if not exists */
          {
            DosUmbLink(lr.BX);
            break;
          }
          /* else fall through */

        default:
          goto error_invalid;
#ifdef DEBUG
        case 0xff:
          show_chain();
          break;
#endif
      }
      break;

      /* Get Extended Error */
    case 0x59:
      lr.AX = CritErrCode;
      lr.CH = CritErrLocus;
      lr.BH = CritErrClass;
      lr.BL = CritErrAction;
      lr.DI = FP_OFF(CritErrDev);
      lr.ES = FP_SEG(CritErrDev);
      break;

      /* Create Temporary File */
    case 0x5a:
      lrc = DosMkTmp(FP_DS_DX, lr.CX);
      goto long_check;

      /* Create New File */
    case 0x5b:
      lrc = DosOpen(FP_DS_DX, O_LEGACY | O_RDWR | O_CREAT, lr.CX);
      goto long_check;

/* /// Added for SHARE.  - Ron Cemer */
      /* Lock/unlock file access */
    case 0x5c:
      rc = DosLockUnlock
           (lr.BX, ((unsigned long)lr.CX << 16) | lr.DX,
                   ((unsigned long)lr.SI << 16) | lr.DI, lr.AL != 0);
      if (rc != SUCCESS)
        goto error_exit;
      break;
/* /// End of additions for SHARE.  - Ron Cemer */

      /* UNDOCUMENTED: server, share.exe and sda function             */
    case 0x5d:
      switch (lr.AL)
      {
          /* Remote Server Call */
        case 0x00:
          fmemcpy(&lr, FP_DS_DX, sizeof(lr));
          goto dispatch;

        case 0x06:
          lr.DS = FP_SEG(internal_data);
          lr.SI = FP_OFF(internal_data);
          lr.CX = swap_indos - internal_data;
          lr.DX = swap_always - internal_data;
          CLEAR_CARRY_FLAG();
          break;

        case 0x07:
        case 0x08:
        case 0x09:
          rc = remote_printredir(lr.DX, Int21AX);
          CLEAR_CARRY_FLAG();
          if (rc != SUCCESS)
            goto error_exit;
          break;

          /* Set Extended Error */
        case 0x0a:
          {
            lregs far *er      = FP_DS_DX;
            CritErrCode        = er->AX;
            CritErrDev         = MK_FP(er->ES, er->DI);
            CritErrLocus       = er->CH;
            CritErrClass       = er->BH;
            CritErrAction      = er->BL;
            CLEAR_CARRY_FLAG();
            break;
          }

        default:
          CritErrCode = SUCCESS;
          goto error_invalid;
      }
      break;

    case 0x5e:
      switch (lr.AL)
      {
        case 0x00:
          lr.CX = get_machine_name(FP_DS_DX);
          break;

        case 0x01:
          set_machine_name(FP_DS_DX, lr.CX);
          break;

        default:
          rc = (int)network_redirector_mx(REM_PRINTSET, &lr, (void *)Int21AX);
          goto short_check;
      }
      break;

    case 0x5f:
      if (lr.AL == 7 || lr.AL == 8)
      {
        if (lr.DL < lastdrive)
        {
          struct cds FAR *cdsp = CDSp + lr.DL;
          if (FP_OFF(cdsp->cdsDpb))     /* letter of physical drive?    */
          {
            cdsp->cdsFlags &= ~CDSPHYSDRV;
            if (lr.AL == 7)
              cdsp->cdsFlags |= CDSPHYSDRV;
            break;
          }
        }
        rc = DE_INVLDDRV;
        goto error_exit;
      }
      else
      {
        rc = (int)network_redirector_mx(REM_DOREDIRECT, &lr, (void *)Int21AX);
        /* the remote function manipulates *r directly !,
           so we should not copy lr to r here            */
        if (rc != SUCCESS)
        {
          CritErrCode = -rc;      /* Maybe set */
          SET_CARRY_FLAG();
        }
        r->AX = -rc;            /* okay because we use real_exit */
        goto real_exit;
      }

    case 0x60:                 /* TRUENAME */
      rc = DosTruename(MK_FP(lr.DS, lr.SI), adjust_far(FP_ES_DI));
      lr.AX = rc;
      goto short_check;

#ifdef TSC
      /* UNDOCUMENTED: no-op                                          */
      /*                                                              */
      /* DOS-C: tsc support                                           */
    case 0x61:
#ifdef DEBUG
      switch (lr.AL)
      {
        case 0x01:
          bTraceNext = TRUE;
          break;

        case 0x02:
          bDumpRegs = FALSE;
          break;
      }
#endif
      lr.AL = 0x00;
      break;
#endif

      /* UNDOCUMENTED: return current psp
         case 0x62: is in int21_syscall
         lr.BX = cu_psp;
         break;
       */

      /* UNDOCUMENTED: Double byte and korean tables                  */
    case 0x63:
      {
        VOID FAR *p;
#if 0
        /* not really supported, but will pass.                 */
        lr.AL = 0x00;           /*jpp: according to interrupt list */
        /*Bart: fails for PQDI and WATCOM utilities:
           use the above again */
#endif
        switch (lr.AL)
        {
          case 0:
            p = DosGetDBCS();
            lr.DS = FP_SEG(p);
            lr.SI = FP_OFF(p) + 2;
            break;
          case 1: /* set Korean Hangul input method to DL 0/1 */
            lr.AL = 0xff;       /* flag error (AL would be 0 if okay) */
            break;
          case 2: /* get Korean Hangul input method setting to DL */
            lr.AL = 0xff;       /* flag error, do not set DL */
            break;
          default:      /* is this the proper way to handle invalid AL? */
            rc = -1;
            goto error_exit;
        }
        break;
      }
/*
    case 0x64:
      see above (invalid)
*/

      /* Extended country info                                        */
    case 0x65:
      switch (lr.AL)
      {
        case 0x20:             /* upcase single character */
          lr.DL = DosUpChar(lr.DL);
          break;
        case 0x21:             /* upcase memory area */
          DosUpMem(FP_DS_DX, lr.CX);
          break;
        case 0x22:             /* upcase ASCIZ */
          DosUpString(FP_DS_DX);
          break;
        case 0xA0:             /* upcase single character of filenames */
          lr.DL = DosUpFChar(lr.DL);
          break;
        case 0xA1:             /* upcase memory area of filenames */
          DosUpFMem(FP_DS_DX, lr.CX);
          break;
        case 0xA2:             /* upcase ASCIZ of filenames */
          DosUpFString(FP_DS_DX);
          break;
        case 0x23:             /* check Yes/No response */
          lr.AX = DosYesNo(lr.DL);
          break;
        default:
#ifdef NLS_DEBUG
          if ((rc = DosGetData(lr.AL, lr.BX, lr.DX, lr.CX, FP_ES_DI)) < 0)
          {
            printf("DosGetData() := %d\n", rc);
            goto error_exit;
          }
          printf("DosGetData() returned successfully\n");
          break;
#else
          rc = DosGetData(lr.AL, lr.BX, lr.DX, lr.CX, FP_ES_DI);
          goto short_check;
#endif
      }
      break;

      /* Code Page functions */
    case 0x66:
        switch (lr.AL)
        {
          case 1:
            rc = DosGetCodepage(&lr.BX, &lr.DX);
            break;
          case 2:
            rc = DosSetCodepage(lr.BX, lr.DX);
            break;

          default:
            goto error_invalid;
        }
        if (rc != SUCCESS)
          goto error_exit;
        break;

      /* Set Max file handle count */
    case 0x67:
      rc = SetJFTSize(lr.BX);
      goto short_check;

      /* Flush file buffer -- COMMIT FILE.  */
    case 0x68:
    case 0x6a:
      rc = DosCommit(lr.BX);
      goto short_check;

      /* Get/Set Serial Number */
    case 0x69:
      rc = (lr.BL == 0 ? default_drive : lr.BL - 1);
      if (lr.AL < 2)
      {
        if (get_cds(rc) == NULL)
        {
          rc = DE_INVLDDRV;
          goto error_exit;
        }
        if (get_dpb(rc) != NULL)
        {
          UWORD saveCX = lr.CX;
          lr.CX = lr.AL == 0 ? 0x0866 : 0x0846;
          lr.AL = 0x0d;
          rc = DosDevIOctl(&lr);
          lr.CX = saveCX;
          goto short_check;
        }
      }
      goto error_invalid;
/*
    case 0x6a: see case 0x68
    case 0x6b: dummy func: return AL=0
*/
      /* Extended Open-Creat, not fully functional. (bits 4,5,6 of BH) */
    case 0x6c:
      /* high nibble must be <= 1, low nibble must be <= 2 */
      if ((lr.DL & 0xef) > 0x2)
        goto error_invalid;
      lrc = DosOpen(MK_FP(lr.DS, lr.SI),
                    (lr.BX & 0x70ff) | ((lr.DL & 3) << 8) |
                    ((lr.DL & 0x10) << 6), lr.CL);
      if (lrc >= SUCCESS)
        /* action */
        lr.CX = (UWORD)(lrc >> 16);
      goto long_check;

      /* case 0x6d and above not implemented : see default; return AL=0 */

#ifdef WITHFAT32
    /* LFN API */
    case 0x71:
      switch (lr.AL)
      {
#ifdef WITHLFNAPI
        /* Win95 LFN - reset drive */
        case 0x0d:
        /* Win95 LFN - make directory */
        case 0x39:
        /* Win95 LFN - remove directory */
        case 0x3a:
        /* Win95 LFN - change directory */
        case 0x3b:
        /* Win95 LFN - delete file */
        case 0x41:
        /* Win95 LFN - extended get/set file attributes */
        case 0x43:
        /* Win95 LFN - get current directory */
        case 0x47:
        /* Win95 LFN - find first file */
        case 0x4e:
        /* Win95 LFN - find next file */
        case 0xa2:  /* internal - same as 0x4f */
        case 0x4f: {
          goto unsupp;
        }
        /* Win95 LFN - rename file */
        case 0x56:
        /* Win95 LFN - canonicalize file name/path */
        case 0x60: {
          switch (lr.CL)
          {
            /* truename - canonicalize path, accepts short/long/or combination as input and may return combined short/long name */
            case 0x00: {
            }
            /* get canonical short (8.3) name or path, truename that accepts long name and returns short name  */
            case 0x01: {
            }
            /* get canonical long name or path, truename that accepts short name and returns long name */
            case 0x02: {
            }
            default:
              goto unsupp;
          }
        }
        /* Win95 LFN - create or open file */
        case 0xa9:  /* for real-mode servers only, AX is _global_ file handle on return */
        case 0x6c: {
          goto unsupp;
        }
        /* Win95 LFN - get volume information */
        case 0xa0:
        /* Win95 LFN - find file close */
        case 0xa1:  {
lfn_findclose:
          goto unsupp;
        }
#if 0
        /* Win95 LFN - internal use ??? */
        case 0xa3:
        case 0xa4:
        case 0xa5:
          goto unsupp;
#endif
#endif
        /* EDR-DOS LFN - Long LSEEK - SET CURRENT 64-bit FILE POSITION */
        case 0x42:
        /* Win95 LFN - get file info by handle */
        case 0xa6: {
          /* only passed to redirector supported for now */
          iregs saved_r;
          sft FAR *s;
          unsigned char idx;

          if (lr.BX >= psp->ps_maxfiles)
          {
            rc = DE_INVLDHNDL;
            goto error_exit;
          }
          idx = psp->ps_filetab[lr.BX];
          s = idx_to_sft(idx);
          if (s == (sft FAR *)-1)
          {
            rc = DE_INVLDHNDL;
            goto error_exit;
          }
          if (!(s->sft_flags & SFT_FSHARED)) {
            if (lr.AL != 0x42) {
              goto unsupp;    /* unsupported on local fs yet */
            }
          }
          /* call to redirector */
          fmemcpy(&saved_r, r, sizeof(saved_r));
          r->ES = FP_SEG(s);
          r->DI = FP_OFF(s);
          r->flags |= FLG_CARRY;
          r->AH = 0x11;
          call_intr(0x2f, r);
          if (!(r->flags & FLG_CARRY)) {
            r->ES = saved_r.ES;
            r->DI = saved_r.DI;
            goto real_exit;
          }
          /* carry still set - unhandled */
          fmemcpy(r, &saved_r, sizeof(saved_r));
          goto unsupp;
        }
#ifdef WITHLFNAPI
        /* Win95 LFN - Win95 64 UTC file time to/from DOS date and time (local timezone) */
        case 0xa7: {
          /* Note: valid input range limited to Jan 1, 1980 to Dec 31, 2107 */
          switch (lr.BL)
          {
            /* from Win95 UTC to DOS date/time */
            case 0x00: {
            }
            /* from DOS date/time to Win95 UTC */
            case 0x01: {
            }
            default:
              goto unsupp;
          }
        }
        /* Win95 LFN - generate short filename */
        case 0xa8:
        /* Win95 LFN - subst */
        case 0xaa: {
          goto unsupp;
        }
#endif
        default:
          goto unsupp;
      }
#ifdef WITHLFNAPI
    /* Win95 beta LFN - find close */
    case 0x72: goto lfn_findclose;
#endif

      /* DOS 7.0+ FAT32 extended functions */
    case 0x73:
      CLEAR_CARRY_FLAG();
      CritErrCode = SUCCESS;
      rc = int21_fat32(&lr);
      goto short_check;
#endif

#ifdef WITHLFNAPI
      /* FreeDOS LFN helper API functions */
    case 0x74:
      {
        switch (lr.AL)
        {
          /* Allocate LFN inode */
          case 0x01:
            rc = lfn_allocate_inode();
            break;
          /* Free LFN inode */
          case 0x02:
            rc = lfn_free_inode(lr.BX);
            break;
          /* Setup LFN inode */
          case 0x03:
            rc = lfn_setup_inode(lr.BX, MK_ULONG(lr.CX, lr.DX), MK_ULONG(lr.SI,lr.DI));
            break;
          /* Create LFN entries */
          case 0x04:
            rc = lfn_create_entries(lr.BX, (lfn_inode_ptr)FP_DS_DX);
            break;
          /* Read next LFN */
          case 0x05:
            rc = lfn_dir_read(lr.BX, (lfn_inode_ptr)FP_DS_DX);
            break;
          /* Write SFN pointed by LFN inode */
          case 0x06:
            rc = lfn_dir_write(lr.BX);
            break;
          default:
            goto error_invalid;
        }
        lr.AX = rc;
        CLEAR_CARRY_FLAG();
        goto short_check;
      }
#endif
  }
  goto exit_dispatch;
#ifdef WITHFAT32
unsupp:
  {
    lr.AL = 00;
    goto error_carry;
  }
#endif
long_check:
  if (lrc >= SUCCESS)
  {
    lr.AX = (UWORD)lrc;
    goto exit_dispatch;
  }
  rc = (int)lrc;
short_check:
  if (rc < SUCCESS)
    goto error_exit;
  goto exit_dispatch;
error_invalid:
  rc = DE_INVLDFUNC;
error_exit:
  lr.AX = -rc;
  if (CritErrCode == SUCCESS)
    CritErrCode = lr.AX;      /* Maybe set */
error_carry:
  SET_CARRY_FLAG();
exit_dispatch:
  fmemcpy(r, &lr, sizeof(lregs) - 4); /* copy lr -> r but exclude flags */
  r->DS = lr.DS;
  r->ES = lr.ES;
real_exit:;

#ifdef DEBUG
  if (bDumpRegs)
  {
    fmemcpy(&error_regs, user_r, sizeof(iregs));
    dump_regs = TRUE;
    dump();
  }
#endif
}

#if 0
        /* No kernel INT-23 handler required no longer -- 1999/04/15 ska */
/* ctrl-Break handler */
#pragma argsused
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp,
                             int sp, int bx, int dx, int cx, int ax,
                             int ip, int cs, int flags)
{
  tsr = FALSE;
  return_mode = 1;
  return_code = -1;
  mod_sto(CTL_C);
  DosMemCheck();
#ifdef TSC
  StartTrace();
#endif
  return_user();
}
#endif

struct int25regs {
  UWORD es, ds;
  UWORD di, si, bp, sp;
  UWORD bx, dx, cx, ax;
  UWORD flags, ip, cs;
};

/*
    this function is called from an assembler wrapper function
*/
VOID ASMCFUNC int2526_handler(WORD mode, struct int25regs FAR * r)
{
  ULONG blkno;
  UWORD nblks;
  BYTE FAR *buf;
  UBYTE drv;

  if (mode == 0x26)
    mode = DSKWRITEINT26;
  else
    mode = DSKREADINT25;

  drv = r->ax & 0x7f; /* according to RBIL, some programs may try with */
                      /* high bit of AL set, so mask it together with AH */
                      /* otherwise we might access a non-existing unit */

  if (drv >= lastdrive)
  {
    r->ax = 0x201;
    SET_CARRY_FLAG();
    return;
  }

#ifdef WITHFAT32
  {
    struct dpb FAR *dpbp = get_dpb(drv);
    if (dpbp != NULL && ISFAT32(dpbp))
    {
      r->ax = 0x207;
      SET_CARRY_FLAG();
      return;
    }
  }
#endif

  nblks = r->cx;
  blkno = r->dx;

  buf = MK_FP(r->ds, r->bx);

  if (nblks == 0xFFFF)
  {
    /*struct HugeSectorBlock FAR *lb = MK_FP(r->ds, r->bx); */
    blkno = ((struct HugeSectorBlock FAR *)buf)->blkno;
    nblks = ((struct HugeSectorBlock FAR *)buf)->nblks;
    buf = ((struct HugeSectorBlock FAR *)buf)->buf;
  }

  InDOS++;

  r->ax = dskxfer(drv, blkno, buf, nblks, mode);

  CLEAR_CARRY_FLAG();
  if (r->ax != 0)
  {
    SET_CARRY_FLAG();
    if (mode == DSKWRITEINT26)
      setinvld(drv);
  }
  --InDOS;
}

/*
VOID int25_handler(struct int25regs FAR * r) { int2526_handler(DSKREAD,r); }
VOID int26_handler(struct int25regs FAR * r) { int2526_handler(DSKWRITE,r); }
*/

#ifdef TSC
STATIC VOID StartTrace(VOID)
{
  if (bTraceNext)
  {
#ifdef DEBUG
    bDumpRegs = TRUE;
#endif
    bTraceNext = FALSE;
  }
#ifdef DEBUG
  else
    bDumpRegs = FALSE;
#endif
}
#endif

/* this function is called from an assembler wrapper function
   and serves the internal dos calls - int2f/12xx and int2f/4a01,4a02.
*/
struct int2f12regs {
#ifdef I386
#ifdef __WATCOMC__
  /* UWORD gs, fs;  ** GS/FS are protected through SI/DI */
#else
  UWORD high_edx,
#ifdef _MSC_VER
        high_ecx,
#else /* __BORLANDC__ */
        high_ebx,
#endif
        high_eax;
#endif
#endif
  UWORD es, ds;
  UWORD di, si, bp;
  xreg b, d, c, a;
  UWORD ip, cs, flags;
  UWORD callerARG1;             /* used if called from INT2F/12 */
};

extern intvec FAR ASM BIOSInt13;
extern intvec FAR ASM UserInt13;
extern intvec FAR ASM BIOSInt19;


/* WARNING: modifications in `r' are used outside of int2F_12_handler()
 * On input r.AX==0x12xx, 0x4A01 or 0x4A02
 * also handle Windows' DOS notification hooks, r.AH==0x16 and r.AH==0x13
 * along with DRIVER.SYS/DRIVPARAM r.AH=0x08 calls
 */
VOID ASMCFUNC int2F_12_handler(struct int2f12regs FAR *pr)
{
  COUNT rc;
  long lrc;
  UWORD requestedsize;

#define r (*pr)

  if (r.AH == 0x4a)
  {
    size_t size = 0, offs = 0xffff, realoffs;	/* defaults for no alloc */

    r.ES = offs;
    if (FP_SEG(firstAvailableBuf) == offs) /* HMA present? */
    {
      realoffs = FP_OFF(firstAvailableBuf);
      if (realoffs <= 0xFFF0) {		/* if any free (not yet exthausted) */
        offs = realoffs;
        offs += 15;
        offs &= ~ 15;			/* annoying: buffers may be unaligned */
        size = - offs;				/* bx + di = 10000h */
        if (r.AL == 0x02)			/* allocate HMA space */
        {
          requestedsize = (r.BX + 15) & ~ 15;	/* align to paragraph */
          if (requestedsize < r.BX ||
              requestedsize > size) { /* overflow or OOM */
            size = 0;
            offs = 0xffff;			/* requested more than we have */
          } else {
            size = (UWORD)requestedsize;	/* return rounded size */
            AllocateHMASpace(realoffs, offs + size - 1); /* ! realoffs */
            if ( ((UDWORD)offs + (UDWORD)size) == 0x10000UL ) {
              firstAvailableBuf = MK_FP(0xFFFF, 0xFFFF); /* exhausted */
            } else {
              firstAvailableBuf += size; /* advance free pointer */
            }
          }
        }
      }
    }
    r.DI = offs;
    r.BX = size;
    return;
  }
  else if (r.AH == 0x13) /* set disk interrupt (13h) handler */
  {
    /* set new values for int13h calls, and must return old values */
    register intvec tmp = UserInt13;
    UserInt13 = MK_FP(r.ds, r.DX);            /* int13h handler to use */
    r.ds = FP_SEG(tmp);  r.DX = FP_OFF(tmp);
    tmp = BIOSInt13;
    BIOSInt13 = MK_FP(r.es, r.BX);            /* int13h handler to restore on reboot */
    r.es = FP_SEG(tmp);  r.BX = FP_OFF(tmp);
    return;
  }
  else if (r.AH == 0x16) /* Window/Multitasking hooks */
  {
#ifdef WIN31SUPPORT  /* See "DOS Internals" or RBIL under DOSMGR for details */
    switch (r.AL)
    {
      /* default: unhandled requests pass through unchanged */
      case 0x0:          /* is Windows active */
      case 0x0A:         /* identify Windows version */
      {
        /* return AX unchanged if Windows not active */
        break;
      } /* 0x0, 0x0A */
      case 0x03:          /* Windows Get Instance Data */
      {
        /* This should only be called if AX=1607h/BX=15h is not supported. */
        /* The data returned here corresponds directly with text entries that
           can also be in INSTANCE.386 [which in theory means Windows could
           be updated to support FD kernel without responding to these?].
         */
        DebugPrintf(("get instance data\n"));
        break;
      } /* 0x03 */
      case 0x05:          /* Windows Startup Broadcast */
      {
        /* After receiving this call we activate compatibility changes
           as DOS 5 does, though can wait until 0x07 subfunc 0x01
         */
        /* on entry:
           DX flags, bit 0 is set(=1) for standard mode
           DI Windows version#, major# in high byte
           CX 0, set on exit to nonzero to fail load request
           DS:SI is 0000:0000, for enhanced mode, at most 1 program can
                 set to memory manager calling point to disable V86
           ES:BX is 0000:0000, set to startup structure
          */
        /* r.CX = 0x0; ** redundant and could be set nonzero by another hooked program */
        r.es = FP_SEG(&winStartupInfo);
        r.BX = FP_OFF(&winStartupInfo);
        winStartupInfo.winver = r.di;  /* match what caller says it is */
#if defined __GNUC__
        winseg1 = FP_SEG(&winStartupInfo);
        winseg2 = FP_SEG(&DATASTART);
        winseg3 = FP_OFF(&markEndInstanceData);
#endif
        winInstanced = 1; /* internal flag marking Windows is active */
        DebugPrintf(("Win startup\n"));
        break;
      } /* 0x05 */
      case 0x06:          /* Windows Exit Broadcast */
      {
        /* can do nothing or can remove any changes made
           specifically for Windows, must preserve DS.
           Note: If Windows fatally exits then may not be called.
         */
        winInstanced = 0; /* internal flag marking Windows is NOT active */
        DebugPrintf(("Win exit\n"));
        break;
      } /* 0x06 */
      case 0x07:          /* DOSMGR Virtual Device API */
      {
        DebugPrintf(("Vxd:DOSMGR:%x:%x:%x:%x\n",r.AX,r.BX,r.CX,r.DX));
        if (r.BX == 0x15) /* VxD id of "DOSMGR" */
        {
          switch (r.CX)
          {
            /* default: unhandled requests pass through unchanged */
            case 0x00:    /* query if supported */
            {
              r.CX = winInstanced; /* should always be nonzero if Win is active */
              r.DX = FP_SEG(&nul_dev); /* data segment / segment of DOS drivers */
              r.es = FP_SEG(&winPatchTable); /* es:bx points to table of offsets */
              r.BX = FP_OFF(&winPatchTable);
              break;
            }
            case 0x01:    /* enable Win support, ie patch DOS */
            {
              /* DOS 5+ return with bitflags unchanged, Windows critical section
                 needs are handled without need to patch.  If this
                 function does not return successfully windows will
                 attempt to do the patching itself (very bad idea).
                 On entry BX is bitflags describing support requested,
                 and on return DX is set to which we can support.
                 Note: any we report as unhandled Windows will attempt
                 to patch kernel to handle, probably not a good idea.
                   0001h: enable critical section signals (int 2Ah functions
                          80h/81h) to allow re-entering DOS while InDOS.
                   0002h: allow nonzero local machine ID, ie different VMs
                          report different values.
                          FIXME: does this mean we need to set this or does Windows?
                   0004h: split up binary reads to increase int 2Ah function 84h scheduling
                          / turn Int 21h function 3Fh on STDIN into polling loop
                   0008h: notify Windows of halting due to internal stack errors
                   0010h: notify Windows of logical drive map change ("Insert disk X:")
               */
              r.BX = r.DX;    /* sure we support everything asked for, ;-) */
              r.DX = 0xA2AB;  /* on succes DX:AX set to A2AB:B97Ch */
              r.AX = 0xB97C;
              /* FIXME: do we need to do anything special for FD kernel? */
              break;
            }
            case 0x02:    /* disable Win support, ie remove patches */
            {
              /* Note: if we do anything special in 'patch DOS', undo it here.
                 This is only called when Windows exits, can be ignored.
               */
              r.CX = 0;   /* for compatibility with MS-DOS 5/6 */
              break;
            }
            case 0x03:    /* get internal structure sizes */
            {
              if (r.CX & 0x01) /* size of Current Directory Structure in bytes */
              {
                r.DX = 0xA2AB;   /* on succes DS:AX set to A2AB:B97Ch */
                r.AX = 0xB97C;
                r.CX = sizeof(struct cds);
              }
              else
                r.CX = 0;      /* unknown or unsupported structure requested */
              break;
            }
            case 0x04:    /* Get Instancing Exemptions */
            {
              /* On exit BX is bit flags denoting data that is instanced
                 so Windows need not instance it.  DOS 5&6 fail with DX=CX=0.
                   0001h: Current Directory Structure
                   0002h: System File Table and device status of STDOUT
                   0004h: device driver chain
                   0008h: Swappable Data Area
               */
              r.DX = 0xA2AB;   /* on succes DX:AX set to A2AB:B97Ch */
              r.AX = 0xB97C;
              r.BX = 0; /* a zero here tells Windows to instance everything */
              break;
            }
            case 0x05:    /* get device driver size */
            {
              /* On entry ES:DI points to possible device driver
                 if is not one return with AX=BX=CX=DX=0
                 else return BX:CX size in bytes allocated to driver
                             and DX:AX set to A2AB:B97Ch */
              mcb FAR *smcb = MK_PTR(mcb, (r.ES-1), 0); /* para before is possibly submcb segment */
              /* drivers always start a seg:0 (DI==0), so if not then either
                 not device driver or duplicate (ie device driver file loaded
                 is of multi-driver variety; multiple device drivers in same file,
                 whose memory was allocated as a single chunk)
                 Drivers don't really have a MCB, instead the DOS MCB is broken
                 up into submcbs, which will have a type of 'D' (or 'E')
                 So we check that this is primary segment, a device driver, and owner.
              */
              if (!r.DI && (smcb->m_type == 'D') && (smcb->m_psp == r.ES))
              {
                ULONG size = smcb->m_size * 16ul;
                r.BX = hiword(size);
                r.CX = loword(size);
                r.DX = 0xA2AB;   /* on succes DX:AX set to A2AB:B97Ch */
                r.AX = 0xB97C;
                break;
              }
              r.DX = 0;   /* we aren't one so return unsupported */
              r.AX = 0;
              r.BX = 0;
              r.CX = 0;
              break;
            }
          }
        }
        DebugPrintf(("Vxd:DOSMGR:%x:%x:%x:%x\n",r.AX,r.BX,r.CX,r.DX));
        break;
      } /* 0x07 */
      case 0x08:          /* Windows Init Complete Broadcast */
      {
        DebugPrintf(("Init complete\n"));
        break;
      } /* 0x08 */
      case 0x09:          /* Windows Begin Exit Broadcast */
      {
        DebugPrintf(("Exit initiated\n"));
        break;
      } /* 0x09 */
      case 0x0B:          /* Win TSR Identify */
      {
        DebugPrintf(("TSR identify request.\n"));
        break;
      } /* 0x0B */
      case 0x80:          /* Win Release Time-slice */
      {
        /* This function is generally only called in idle loops */
        DosIdle_hlt();
        r.AX = 0;
        /* DebugPrintf(("Release Time Slice\n")); */
        break;
      } /* 0x80 */
      case 0x81:          /* Win3 Begin Critical Section */
      {
        DebugPrintf(("Begin CritSect\n"));
        break;
      } /* 0x81 */
      case 0x82:          /* Win3 End Critical Section */
      {
        DebugPrintf(("End CritSect\n"));
        break;
      } /* 0x82 */
      case 0x8F:          /* Win4 Close Awareness */
      {
        if (r.DH != 0x01) /* query close */
          r.AX = 0x0;
        /* else r.AX = 0x168F;  don't close -- continue execution */
        break;
      } /* 0x8F */
      default:
        DebugPrintf(("Win call (int 2Fh/16h): %04x %04x %04x %04x\n", r.AX, r.BX, r.CX, r.DX));
        break;
    }
#endif
    return;
  }
  else if (r.AH == 0x46) /* MS Windows WinOLDAP switching */
  {
#ifdef WIN31SUPPORT  /* See "DOS Internals" under DOSMGR or RBIL for details */
    if (r.AL == 0x01)      /* save MCB */
    {
      /* To prevent corruption when dos=umb where Windows 3.0 standard
         writes a sentinel at 9FFEh, DOS 5 will save the MCB marking
         end of conventional memory (ie MCB following caller's PSP memory
         block [which I assume occupies all of conventional memory] into
         the DOS data segment.
         Note: presumably Win3.1 uses the WinPatchTable.OffLastMCBSeg
         when DOS ver > 5 to do this itself.
       */
      /* FIXME: Implement this! */
    }
    else if (r.AL == 0x02) /* restore MCB */
    {
      /* Copy the MCB we previously saved back. */
      /* FIXME: Implement this! */
    }
#endif
    return;
  }
  else if (r.AH == 0x08) /* DRIVER.SYS / DRIVPARAM */
  {
    switch (r.AL)
    {
    case 0x00:                 /* installation check */
      r.AL = 0xff;             /* installed, 0=not installed */
      break;

    case 0x01:                 /* add new block device */
	  /* TODO, see push_ddt() */
      break;

    case 0x02:                 /* execute device driver request */
	  /* TODO */
      break;

    /* Note: see in2f.asm where this is handled */
    case 0x03:                 /* get drive data table */
      r.DS = FP_SEG(&nul_dev);
	  r.DI = FP_OFF(getddt(0));
      break;
    }

    return;
  }
  /* else (r.AH == 0x12) */

  switch (r.AL)
  {
    case 0x00:                 /* installation check */
      r.AL = 0xff;
      break;

    case 0x03:                 /* get DOS data segment */
      r.DS = FP_SEG(&nul_dev);
      break;

    case 0x06:                 /* invoke critical error */

      /* code, drive number, error, device header */
      r.AL = CriticalError(r.callerARG1 >> 8,
                           (r.callerARG1 & (EFLG_CHAR << 8)) ? 0 :
                           r.callerARG1 & 0xff, r.DI, MK_FP(r.BP, r.SI));
      break;

    case 0x08:                 /* decrease SFT reference count */
      {
        sft FAR *p = MK_FP(r.ES, r.DI);

        r.AX = p->sft_count;

        if (--p->sft_count == 0)
          --p->sft_count;
      }
      break;

    case 0x0a:                 /* perform critical error */
                               /* differs from 0x06 as uses current drive & error on stack */
      /* code, drive number, error, device header */
      r.AL = CriticalError(0x38, /* ignore/retry/fail - based on RBIL possible return values */
                           default_drive,
                           r.callerARG1, /* error, from RBIL passed on stack */
                           CDSp[(WORD)default_drive].cdsDpb->dpb_device);
      r.FLAGS |= FLG_CARRY;
      if (r.AL == 1) r.FLAGS &= ~FLG_CARRY;  /* carry clear if should retry */
      break;

    case 0x0b:                 /* sharing violation occurred */
      {
        /* ES:DI = SFT for previous open of file */
        sft FAR *sftp = MK_FP(r.ES, r.DI);
        /* default to don't retry, ie fail/abort/etc other than retry */
        r.FLAGS |= FLG_CARRY;
        /* from RBIL if SFT for FCB or compatibility mode without NOINHERIT call int24h */
        if ((sftp->sft_mode & O_FCB) || !(sftp->sft_mode & (O_SHAREMASK | O_NOINHERIT)))
        {
          r.AL = CriticalError(0x38, /* ignore/retry/fail - ??? */
                               default_drive,
                               r.callerARG1, /* error, from RBIL passed on stack */
                               CDSp[(WORD)default_drive].cdsDpb->dpb_device);
          /* clear carry if should retry */
          if (r.AL == 1) r.FLAGS &= ~FLG_CARRY;
        }
        r.AX = 0x20;  /* error sharing violation */
      }
      break;

    case 0x0c:                 /* perform "device open" for device, set owner for FCB */

      if (lpCurSft->sft_flags & SFT_FDEVICE)
      {
        request rq;

        rq.r_unit = 0;
        rq.r_status = 0;
        rq.r_command = C_OPEN;
        rq.r_length = sizeof(request);
        execrh((request FAR *) & rq, lpCurSft->sft_dev);
      }

      /* just do it always, not just for FCBs */
      lpCurSft->sft_psp = cu_psp;
      break;

    case 0x0d:                 /* get dos date/time */

      r.AX = dos_getdate();
      r.DX = dos_gettime();
      break;

    case 0x11:                 /* normalise ASCIIZ filename */
    {
      char c;
      char FAR *s = MK_FP(r.DS, r.SI);
      char FAR *t = MK_FP(r.ES, r.DI);

      do
      {
        c = *s++;
        /* uppercase character */
        /* for now, ASCII only because nls.c cannot handle DS!=SS */
        if (c >= 'a' && c <= 'z')
          c -= 'a' - 'A';
        else if (c == '/')
          c = '\\';
        *t++ = c;
      }
      while (c);
      break;
    }

    case 0x12:                 /* get length of asciiz string */

      r.CX = fstrlen(MK_FP(r.ES, r.DI)) + 1;

      break;

    case 0x13:
      /* uppercase character */
      /* for now, ASCII only because nls.c cannot handle DS!=SS */
      r.AL = (unsigned char)r.callerARG1;
      if (r.AL >= 'a' && r.AL <= 'z')
        r.AL -= 'a' - 'A';
      break;

    case 0x16:
      /* get address of system file table entry - used by NET.EXE
         BX system file table entry number ( such as returned from 2F/1220)
         returns
         ES:DI pointer to SFT entry
         BX relative entry number within SFT */
      {
        int rel_idx = idx_to_sft_(r.BX);

        if (rel_idx == -1)
        {
          r.FLAGS |= FLG_CARRY;
          break;
        }
        r.FLAGS &= ~FLG_CARRY;
        r.BX = rel_idx;
        r.ES = FP_SEG(lpCurSft);
        r.DI = FP_OFF(lpCurSft);
        break;
      }

    case 0x17:                 /* get current directory structure for drive - used by NET.EXE
                                   STACK: drive (0=A:,1=B,...)
                                   ; returns
                                   ;   CF set if error
                                   ;   DS:SI pointer to CDS for drive
                                   ;
                                   ; called like
                                   ;   push 2 (c-drive)
                                   ;   mov ax,1217
                                   ;   int 2f
                                   ;
                                   ; probable use: get sizeof(CDSentry)
                                 */
      {
        struct cds FAR *cdsp = get_cds_unvalidated(r.callerARG1 & 0xff);

        if (cdsp == NULL)
        {
          r.FLAGS |= FLG_CARRY;
          break;
        }
        r.DS = FP_SEG(cdsp);
        r.SI = FP_OFF(cdsp);
        r.FLAGS &= ~FLG_CARRY;
        break;
      }

    case 0x18:                 /* get caller's registers */

      r.DS = FP_SEG(user_r);
      r.SI = FP_OFF(user_r);
      break;

    case 0x1b:                 /* #days in February - valid until 2099. */

      r.AL = (r.CL & 3) ? 28 : 29;
      break;

    case 0x1f:                 /* build current directory structure */
      {
        /* this is similar to ARG1-'A', but case-insensitive.
         * Note: the letter is passed here, not number! */
        int drv = (r.callerARG1 & 0x1f) - 1;
        struct cds FAR *cdsp;
        if (drv < 0 || r.callerARG1 < 'A' || r.callerARG1 > 'z')
        {
          r.FLAGS |= FLG_CARRY;
          break;
        }
        cdsp = get_cds_unvalidated(drv);
        if (cdsp == NULL)
        {
          r.FLAGS |= FLG_CARRY;
          break;
        }
        strcpy(TempCDS.cdsCurrentPath, "?:\\");
        *TempCDS.cdsCurrentPath = (BYTE)(r.callerARG1 & 0xff);
        TempCDS.cdsBackslashOffset = 2;
        if (cdsp->cdsFlags)
        {
          TempCDS.cdsDpb = cdsp->cdsDpb;
          TempCDS.cdsFlags = CDSPHYSDRV;    /* don't inherit CDS flags */
        }
        else
        {
          TempCDS.cdsDpb = NULL;
          TempCDS.cdsFlags = 0;
        }
        TempCDS.cdsStrtClst = 0xffff;
        TempCDS.cdsParam = 0xffff;
        TempCDS.cdsStoreUData = 0xffff;
        r.CX = sizeof(TempCDS);
        r.ES = FP_SEG(&TempCDS);
        r.DI = FP_OFF(&TempCDS);
        r.FLAGS &= ~FLG_CARRY;
        break;
      }

    case 0x20:                 /* get job file table entry */
      {
        psp FAR *p = MK_FP(cu_psp, 0);
        unsigned char FAR *idx;

        if (r.BX >= p->ps_maxfiles)
        {
          r.AL = -DE_INVLDHNDL;
          r.FLAGS |= FLG_CARRY;
          break;
        }
        idx = &p->ps_filetab[r.BX];
        r.FLAGS &= ~FLG_CARRY;
        r.ES = FP_SEG(idx);
        r.DI = FP_OFF(idx);
      }
      break;

    case 0x21:                 /* truename */

      DosTruename(MK_FP(r.DS, r.SI), MK_FP(r.ES, r.DI));

      break;

    case 0x23:                 /* check if character device */
      {
        struct dhdr FAR *dhp;

        dhp = IsDevice((BYTE FAR *) DirEntBuffer.dir_name);

        if (dhp == NULL)
        {
          r.FLAGS |= FLG_CARRY;
          break;
        }
        r.BH = dhp->dh_attr;
        r.FLAGS &= ~FLG_CARRY;
      }
      break;

    case 0x25:                 /* get length of asciiz string */

      r.CX = fstrlen(MK_FP(r.DS, r.SI)) + 1;
      break;

    case 0x26:                 /* open file */
      r.FLAGS &= ~FLG_CARRY;
      CritErrCode = SUCCESS;
      lrc = DosOpen(MK_FP(r.DS, r.DX), O_LEGACY | O_OPEN | r.CL, 0);
      goto long_check;

    case 0x27:                 /* close file */
      r.FLAGS &= ~FLG_CARRY;
      CritErrCode = SUCCESS;
      rc = DosClose(r.BX);
      goto short_check;

#ifdef WITHFAT32
#ifdef WITHLFNAPI
    case 0x42:                 /* 64-bit move file pointer */
    {
      /* r.(DS:DX) points to 64-bit file position instead of r.(CX:DX) being 32-bit file position */
      UDWORD FAR *filepos = MK_FP(r.DS, r.DX);
      if (*(filepos+1) != 0) /* we currently only handle lower 32 bits, upper 32 bits must be 0 */
        goto error_invalid;
      r.BP = (UWORD)r.CL;
      r.CX = hiword(*filepos);
      r.DX = loword(*filepos);
      /* fall through to 32-bit move file pointer (0x28) */
    }
#endif
#endif
    case 0x28:                 /* move file pointer */
      /*
       * RBIL says: "sets user stack frame pointer to dummy buffer,
       * moves BP to AX, performs LSEEK, and restores frame pointer"
       * We obviously don't do it like that. Does this do any harm?! --L.G.
       */
      r.FLAGS &= ~FLG_CARRY;
      CritErrCode = SUCCESS;
      if (r.BP < 0x4200 || r.BP > 0x4202)
        goto error_invalid;
      lrc = DosSeek(r.BX, MK_ULONG(r.CX, r.DX), r.BP & 0xff, &rc);
      if (rc == SUCCESS)
      {
        r.DX = (UWORD)(lrc >> 16);
        r.AX = (UWORD) lrc;
      }
      goto short_check;

    case 0x29:                 /* read from file */
      r.FLAGS &= ~FLG_CARRY;
      CritErrCode = SUCCESS;
      lrc = DosRead(r.BX, r.CX, MK_FP(r.DS, r.DX));
      goto long_check;

    case 0x2a:                 /* Set FastOpen but does nothing. */

      r.FLAGS &= ~FLG_CARRY;
      break;

    case 0x2b:                 /* Device I/O Control */
      if (r.BP < 0x4400 || r.BP > 0x44ff)
        goto error_invalid;
      {
        lregs lr;
        lr.AX = r.BP;
        lr.BX = r.BX;
        lr.CX = r.CX;
        lr.DX = r.DX;
        lr.DI = r.DI;
        lr.SI = r.SI;
        lr.DS = r.DS;
        rc = DosDevIOctl(&lr);      /* can set critical error code! */
      }

      if (rc < SUCCESS)
      {
        r.AX = -rc;
        if (rc != DE_DEVICE && rc != DE_ACCESS)
          CritErrCode = r.AX;
        goto error_carry;
      }
      break;

    case 0x2c:                 /* added by James Tabor For Zip Drives
                                   Return Null Device Pointer          */
      /* by UDOS+RBIL: get header of SECOND device driver in device chain,
         omitting the NUL device TE */
      r.BX = FP_SEG(nul_dev.dh_next);
      r.AX = FP_OFF(nul_dev.dh_next);
      break;

    case 0x2d:                 /* Get Extended Error Code */
      r.AX = CritErrCode;

      break;

    case 0x2e:                 /* GET or SET error table addresse - ignored
                                   called by MS debug with  DS != DOSDS, printf
                                   doesn't work!! */
      break;

    case 0x2f:                 /* updates version returned by int 21/30h for all processes */
      if (r.DX)                /* set returned version from DX */
      {
        os_setver_major = r.DL;
        os_setver_minor = r.DH;
      }
      else                              /* set returned version from emulated true DOS version */
      {
        os_setver_major = os_major;
        os_setver_minor = os_minor;
      }
      break;

    default:
      if (r.AL <= 0x31)
      {
        put_string("unimplemented internal dos function INT2F/12");
        put_unsigned(r.AL, 16, 2);
        put_string("\n");
        r.FLAGS |= FLG_CARRY;
      }
  }
  return;
long_check:
  if (lrc >= SUCCESS)
  {
    r.AX = (UWORD)lrc;
    return;
  }
  rc = (int)lrc;
short_check:
  if (rc < SUCCESS)
    goto error_exit;
  return;
error_invalid:
  rc = DE_INVLDFUNC;
error_exit:
  r.AX = -rc;
  if (CritErrCode == SUCCESS)
    CritErrCode = r.AX;      /* Maybe set */
error_carry:
  r.FLAGS |= FLG_CARRY;

#undef r
}

/*
 * 2000/09/04  Brian Reifsnyder
 * Modified interrupts 0x25 & 0x26 to return more accurate error codes.
 */

