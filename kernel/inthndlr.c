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

#ifdef VERSION_STRINGS
BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.22  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.21  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.20  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.19  2001/04/02 23:18:30  bartoldeman
 * Misc, zero terminated device names and redirector bugs fixed.
 *
 * Revision 1.18  2001/03/30 22:27:42  bartoldeman
 * Saner lastdrive handling.
 *
 * Revision 1.17  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.16  2001/03/27 20:27:43  bartoldeman
 * dsk.c (reported by Nagy Daniel), inthndlr and int25/26 fixes by Tom Ehlert.
 *
 * Revision 1.15  2001/03/25 17:11:54  bartoldeman
 * Fixed sys.com compilation. Updated to 2023. Also: see history.txt.
 *
 * Revision 1.14  2001/03/21 02:56:26  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.13  2001/03/08 21:00:00  bartoldeman
 * MCB chain corruption and DosFindNext fix (thanks Martin Stromberg and Tom Ehlert)
 *
 * Revision 1.12  2000/12/16 01:38:35  jimtabor
 * Added patches from Bart Oldeman
 *
 * Revision 1.11  2000/10/30 00:21:15  jimtabor
 * Adding Brian Reifsnyder Fix for Int 25/26
 *
 * 2000/09/04  Brian Reifsnyder
 * Modified interrupts 0x25 & 0x26 to return more accurate error codes.
 *
 * Revision 1.10  2000/10/29 23:51:56  jimtabor
 * Adding Share Support by Ron Cemer
 *
 * Revision 1.9  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.8  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.7  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.6  2000/05/17 19:15:12  jimtabor
 * Cleanup, add and fix source.
 *
 * Revision 1.5  2000/05/11 06:14:45  jimtabor
 * Removed #if statement
 *
 * Revision 1.4  2000/05/11 04:26:26  jimtabor
 * Added code for DOS FN 69 & 6C
 *
 * Revision 1.3  2000/05/09 00:30:11  jimtabor
 * Clean up and Release
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.24  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.22  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.21  2000/03/17 05:00:11  kernel
 * Fixed Func 0x32
 *
 * Revision 1.20  2000/03/16 03:28:49  kernel
 * *** empty log message ***
 *
 * Revision 1.19  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.18  1999/09/23 04:40:47  jprice
 * *** empty log message ***
 *
 * Revision 1.13  1999/09/14 01:18:36  jprice
 * ror4: fix int25 & 26 are not cached.
 *
 * Revision 1.12  1999/09/13 22:16:47  jprice
 * Fix 210B function
 *
 * Revision 1.11  1999/08/25 03:18:08  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.10  1999/08/10 18:07:57  jprice
 * ror4 2011-04 patch
 *
 * Revision 1.9  1999/08/10 18:03:43  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.8  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.7  1999/04/23 04:24:39  jprice
 * Memory manager changes made by ska
 *
 * Revision 1.6  1999/04/16 12:21:22  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.5  1999/04/11 04:33:39  jprice
 * ror4 patches
 *
 * Revision 1.3  1999/04/04 22:57:47  jprice
 * no message
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:04  jprice
 * New version without IPL.SYS
 *
 * Revision 1.9  1999/03/23 23:38:49  jprice
 * Now sets carry when we don't support a function
 *
 * Revision 1.8  1999/03/02 07:02:55  jprice
 * Added some comments.  Fixed some minor bugs.
 *
 * Revision 1.7  1999/03/01 05:45:08  jprice
 * Added some DEBUG ifdef's so that it will compile without DEBUG defined.
 *
 * Revision 1.6  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.5  1999/02/04 03:11:07  jprice
 * Formating
 *
 * Revision 1.4  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.3  1999/01/30 08:28:11  jprice
 * Clean up; Fixed bug with set attribute function.
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.14   06 Dec 1998  8:47:38   patv
 * Expanded due to improved int 21h handler code.
 *
 *    Rev 1.13   07 Feb 1998 20:38:46   patv
 * Modified stack fram to match DOS standard
 *
 *    Rev 1.12   22 Jan 1998  4:09:26   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.11   06 Jan 1998 20:13:18   patv
 * Broke apart int21_system from int21_handler.
 *
 *    Rev 1.10   04 Jan 1998 23:15:22   patv
 * Changed Log for strip utility
 *
 *    Rev 1.9   04 Jan 1998 17:26:16   patv
 * Corrected subdirectory bug
 *
 *    Rev 1.8   03 Jan 1998  8:36:48   patv
 * Converted data area to SDA format
 *
 *    Rev 1.7   01 Aug 1997  2:00:10   patv
 * COMPATIBILITY: Added return '$' in AL for function int 21h fn 09h
 *
 *    Rev 1.6   06 Feb 1997 19:05:54   patv
 * Added hooks for tsc command
 *
 *    Rev 1.5   22 Jan 1997 13:18:32   patv
 * pre-0.92 Svante Frey bug fixes.
 *
 *    Rev 1.4   16 Jan 1997 12:46:46   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:03:40   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   19 Feb 1996  3:21:48   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.1   01 Sep 1995 17:54:20   patv
 * First GPL release.
 *
 *    Rev 1.0   02 Jul 1995  8:33:34   patv
 * Initial revision.
 */

#ifdef TSC
static VOID StartTrace(VOID);
static bTraceNext = FALSE;
#endif

#if 0      /* Very suspicious, passing structure by value??
               Deactivated -- 2000/06/16 ska*/
/* Special entry for far call into the kernel                           */
#pragma argsused
VOID FAR int21_entry(iregs UserRegs)
{
  int21_handler(UserRegs);
}
#endif

/* Normal entry.  This minimizes user stack usage by avoiding local     */
/* variables needed for the rest of the handler.                        */
/* this here works on the users stack !! and only very few functions 
   are allowed                                                          */
VOID int21_syscall(iregs FAR * irp)
{
  Int21AX = irp->AX;

  switch (irp->AH)
  {
      /* DosVars - get/set dos variables                              */
    case 0x33:
      switch (irp->AL)
      {
          /* Get Ctrl-C flag                                      */
        case 0x00:
          irp->DL = break_ena ? TRUE : FALSE;
          break;

          /* Set Ctrl-C flag                                      */
        case 0x01:
          break_ena = irp->DL ? TRUE : FALSE;
          break;

          /* Get Boot Drive                                       */
        case 0x05:
          irp->DL = BootDrive;
          break;

          /* Get DOS-C version                                    */
        case 0x06:
          irp->BL = os_major;
          irp->BH = os_minor;
          irp->DL = rev_number;
          irp->DH = version_flags; /* bit3:runs in ROM,bit 4: runs in HMA*/
          break;
          
        case 0x02:  /* andrew schulman: get/set extended control break 
                       should be done */
        case 0x03:  /* DOS 7 does not set AL */
        case 0x07:  /* neither here */

        default:    /* set AL=0xFF as error, NOT carry */
          irp->AL  = 0xff;
          break;

          /* Toggle DOS-C rdwrblock trace dump                    */
        case 0xfd:
#ifdef DEBUG
          bDumpRdWrParms = !bDumpRdWrParms;
#endif
          break;

          /* Toggle DOS-C syscall trace dump                      */
        case 0xfe:
#ifdef DEBUG
          bDumpRegs = !bDumpRegs;
#endif
          break;

          /* Get DOS-C release string pointer                     */
        case 0xff:
          irp->DX = FP_SEG(os_release);
          irp->AX = FP_OFF(os_release);
          break;
      }
      break;

      /* Set PSP                                                      */
    case 0x50:
      cu_psp = irp->BX;
      break;

      /* Get PSP                                                      */
    case 0x51:
      irp->BX = cu_psp;
      break;

      /* UNDOCUMENTED: return current psp                             */
    case 0x62:
      irp->BX = cu_psp;
      break;

      /* Normal DOS function - DO NOT ARRIVE HERE          */
    default:
      break;
  }
}


VOID int21_service(iregs FAR * r)
{
  COUNT rc = 0,
	  rc1;
  ULONG lrc;
  psp FAR *p = MK_FP(cu_psp, 0);
  void FAR *FP_DS_DX = MK_FP(r->DS, r->DX); /* this is saved so often,
                                               that this saves ~100 bytes */

    
#define CLEAR_CARRY_FLAG()  r->FLAGS &= ~FLG_CARRY
#define SET_CARRY_FLAG()    r->FLAGS |= FLG_CARRY

  p->ps_stack = (BYTE FAR *) r;

#ifdef DEBUG
  if (bDumpRegs)
  {
    fbcopy((VOID FAR *) user_r, (VOID FAR *) & error_regs, sizeof(iregs));
    printf("System call (21h): %02x\n", user_r->AX);
    dump_regs = TRUE;
    dump();
  }
#endif

  if(r->AH >=0x38 && r->AH <= 0x4F)
      CLEAR_CARRY_FLAG();
      /* Clear carry by default for these functions */

dispatch:

  /* Check for Ctrl-Break */
  switch (r->AH)
  {
    default:
      if (!break_ena)
        break;
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
      if (control_break())
        handle_break();
  }

  /* The dispatch handler                                         */
  switch (r->AH)
  {
      /* int 21h common error handler                                 */
    case 0x64:
    error_invalid:
      r->AX = -DE_INVLDFUNC;
      goto error_out;
    error_exit:
      r->AX = -rc;
    error_out:
      CritErrCode = r->AX;  /* Maybe set */
      SET_CARRY_FLAG();
      break;

       /* case 0x00:   --> Simulate a DOS-4C-00 */

      /* Read Keyboard with Echo                      */
    case 0x01:
      Do_DosIdle_loop();
      r->AL = _sti();
      sto(r->AL);
      break;

      /* Display Character                                            */
    case 0x02:
      sto(r->DL);
      break;

      /* Auxiliary Input                                                      */
    case 0x03:
     {
      COUNT scratch;
      GenericRead(STDAUX, 1, (BYTE FAR *) & r->AL, (COUNT FAR *) & scratch, TRUE);
      break;
     }

      /* Auxiliary Output                                                     */
    case 0x04:
     {
      COUNT scratch;
      DosWrite(STDAUX, 1, (BYTE FAR *) & r->DL, (COUNT FAR *) &scratch);
      break;
     }
      /* Print Character                                                      */
    case 0x05:
     {       
      COUNT scratch;       
      DosWrite(STDPRN, 1, (BYTE FAR *) & r->DL, (COUNT FAR *) &scratch);
      break;      
     }

      /* Direct Console I/O                                            */
    case 0x06:
      if (r->DL != 0xff)
        sto(r->DL);
      else if (StdinBusy())
      {
        r->AL = 0x00;
        r->FLAGS |= FLG_ZERO;
      }
      else
      {
        r->FLAGS &= ~FLG_ZERO;
        r->AL = _sti();
      }
      break;

      /* Direct Console Input                                         */
    case 0x07:
      /* Read Keyboard Without Echo                                   */
    case 0x08:
      Do_DosIdle_loop();
      r->AL = _sti();
      break;

      /* Display String                                               */
    case 0x09:
      {
        static COUNT scratch;
        BYTE FAR * q;
        q = FP_DS_DX;
        while (*q != '$')
          ++q;
        DosWrite(STDOUT, FP_OFF(q) - FP_OFF(FP_DS_DX), FP_DS_DX, (COUNT FAR *) & scratch);
      }
      r->AL = '$';
      break;

      /* Buffered Keyboard Input                                      */
    case 0x0a:
      ((keyboard FAR *) FP_DS_DX)->kb_count = 0;
      sti((keyboard FAR *) FP_DS_DX);
      ((keyboard FAR *) FP_DS_DX)->kb_count -= 2;
      break;

      /* Check Stdin Status                                           */
    case 0x0b:
      if (StdinBusy())
        r->AL = 0x00;
      else
        r->AL = 0xFF;
      break;

      /* Flush Buffer, Read Keayboard                                 */
    case 0x0c:
      KbdFlush();
      switch (r->AL)
      {
        case 0x01:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x0a:
          r->AH = r->AL;
          goto dispatch;

        default:
          r->AL = 0x00;
          break;
      }
      break;

      /* Reset Drive                                                  */
    case 0x0d:
      flush();
      break;

      /* Set Default Drive                                            */
    case 0x0e:
      r->AL = DosSelectDrv(r->DL);
      break;

    case 0x0f:
      if (FcbOpen(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x10:
      if (FcbClose(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x11:
      if (FcbFindFirst(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x12:
      if (FcbFindNext(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x13:
      if (FcbDelete(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x14:
      {
        if (FcbRead(FP_DS_DX, &CritErrCode))
          r->AL = 0;
        else
          r->AL = CritErrCode;
        break;
      }

    case 0x15:
      {
        if (FcbWrite(FP_DS_DX, &CritErrCode))
          r->AL = 0;
        else
          r->AL = CritErrCode;
        break;
      }

    case 0x16:
      if (FcbCreate(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    case 0x17:
      if (FcbRename(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

    default:
#ifdef DEBUG
       printf("Unsupported INT21 AH = 0x%x, AL = 0x%x.\n", r->AH, r->AL);
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
      r->AL = 0;
      break;

      /* Get Default Drive                                            */
    case 0x19:
      r->AL = default_drive;
      break;

      /* Set DTA                                                      */
    case 0x1a:
      {
        psp FAR *p = MK_FP(cu_psp, 0);

        p->ps_dta = FP_DS_DX;
        dos_setdta(p->ps_dta);
      }
      break;

      /* Get Default Drive Data                                       */
    case 0x1b:
      {
        BYTE FAR *p;

        FatGetDrvData(0,
                      (COUNT FAR *) & r->AX,
                      (COUNT FAR *) & r->CX,
                      (COUNT FAR *) & r->DX,
                      (BYTE FAR **) & p);
        r->DS = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

      /* Get Drive Data                                               */
    case 0x1c:
      {
        BYTE FAR *p;

        FatGetDrvData(r->DL,
                      (COUNT FAR *) & r->AX,
                      (COUNT FAR *) & r->CX,
                      (COUNT FAR *) & r->DX,
                      (BYTE FAR **) & p);
        r->DS = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

      /* Get default DPB                                              */
    case 0x1f:
      if (default_drive < lastdrive)
      {
        struct dpb FAR *dpb = CDSp->cds_table[default_drive].cdsDpb;
        if (dpb == 0)
        {
          r->AL = 0xff;
          CritErrCode = 0x0f;
          break;
        }

        r->DS = FP_SEG(dpb);
        r->BX = FP_OFF(dpb);
        r->AL = 0;
      }
      else{
        r->AL = 0xff;
        CritErrCode = 0x0f;
        }
      break;

      /* Random read using FCB */
    case 0x21:
      {
        if (FcbRandomIO(FP_DS_DX, &CritErrCode, FcbRead))
          r->AL = 0;
        else
          r->AL = CritErrCode;
        break;
      }

      /* Random write using FCB */
    case 0x22:
      {
        if (FcbRandomIO(FP_DS_DX, &CritErrCode, FcbWrite))
          r->AL = 0;
        else
          r->AL = CritErrCode;
        break;
      }

      /* Get file size in records using FCB */
    case 0x23:
      if (FcbGetFileSize(FP_DS_DX))
        r->AL = 0;
      else
        r->AL = 0xff;
      break;

      /* Set random record field in FCB */
    case 0x24:
      FcbSetRandom(FP_DS_DX);
      break;

      /* Set Interrupt Vector                                         */
    case 0x25:
      {
        VOID(INRPT FAR * p) () = FP_DS_DX;

        setvec(r->AL, p);
      }
      break;

      /* Dos Create New Psp                                           */
    case 0x26:
      {
        psp FAR *p = MK_FP(cu_psp, 0);

        new_psp((psp FAR *) MK_FP(r->DX, 0), p->ps_size);
      }
      break;

      /* Read random record(s) using FCB */
    case 0x27:
      {
        if (FcbRandomBlockRead(FP_DS_DX, r->CX, &CritErrCode))
          r->AL = 0;
        else
          r->AL = CritErrCode;
        break;
      }

      /* Write random record(s) using FCB */
    case 0x28:
      {
        if (FcbRandomBlockWrite(FP_DS_DX, r->CX, &CritErrCode))
          r->AL = 0;
        else
          r->AL = CritErrCode;
        break;
      }

      /* Parse File Name                                              */
    case 0x29:
      {
        BYTE FAR *lpFileName;

        lpFileName = MK_FP(r->DS, r->SI);
        r->AL = FcbParseFname(r->AL,
                              &lpFileName,
                              MK_FP(r->ES, r->DI));
        r->DS = FP_SEG(lpFileName);
        r->SI = FP_OFF(lpFileName);
      }
      break;

      /* Get Date                                                     */
    case 0x2a:
      DosGetDate(
                  (BYTE FAR *) & (r->AL),	/* WeekDay              */
                  (BYTE FAR *) & (r->DH),	/* Month                */
                  (BYTE FAR *) & (r->DL),	/* MonthDay             */
                  (COUNT FAR *) & (r->CX));	/* Year                 */
      break;

      /* Set Date                                                     */
    case 0x2b:
      rc = DosSetDate(
                       (BYTE FAR *) & (r->DH),	/* Month                */
                       (BYTE FAR *) & (r->DL),	/* MonthDay             */
                       (COUNT FAR *) & (r->CX));	/* Year                 */
      if (rc != SUCCESS)
        r->AL = 0xff;
      else
        r->AL = 0;
      break;

      /* Get Time                                                     */
    case 0x2c:
      DosGetTime(
                  (BYTE FAR *) & (r->CH),	/* Hour                 */
                  (BYTE FAR *) & (r->CL),	/* Minutes              */
                  (BYTE FAR *) & (r->DH),	/* Seconds              */
                  (BYTE FAR *) & (r->DL));	/* Hundredths           */
      break;

      /* Set Date                                                     */
    case 0x2d:
      rc = DosSetTime(
                       (BYTE FAR *) & (r->CH),	/* Hour                 */
                       (BYTE FAR *) & (r->CL),	/* Minutes              */
                       (BYTE FAR *) & (r->DH),	/* Seconds              */
                       (BYTE FAR *) & (r->DL));	/* Hundredths           */
      if (rc != SUCCESS)
        r->AL = 0xff;
      else
        r->AL = 0;
      break;

      /* Set verify flag                                              */
    case 0x2e:
      verify_ena = (r->AL ? TRUE : FALSE);
      break;

      /* Get DTA                                                      */
    case 0x2f:
      r->ES = FP_SEG(dta);
      r->BX = FP_OFF(dta);
      break;

      /* Get DOS Version                                              */
    case 0x30:
      r->AL = os_major;
      r->AH = os_minor;
      r->BH = OEM_ID;
      r->CH = REVISION_MAJOR;   /* JPP */
      r->CL = REVISION_MINOR;
      r->BL = REVISION_SEQ;
      
      if (ReturnAnyDosVersionExpected)  
      {
                            /* TE for testing purpose only and NOT 
                               to be documented:
                               return programs, who ask for version == XX.YY
                               exactly this XX.YY. 
                               this makes most MS programs more happy.
                            */
        UBYTE FAR *retp = MK_FP(r->cs, r->ip);
        
        if (     retp[0] == 0x3d  &&     /* cmp ax, xxyy */
                (retp[3] == 0x75 || retp[3] == 0x74)) /* je/jne error    */
        {
            r->AL = retp[1];
            r->AH = retp[2];
        }
        else if(retp[0] == 0x86 &&      /* xchg al,ah   */
                retp[1] == 0xc4 &&
                retp[2] == 0x3d &&      /* cmp ax, xxyy */
               (retp[5] == 0x75 || retp[5] == 0x74)) /* je/jne error    */                               
        {
            r->AL = retp[4];
            r->AH = retp[3];
        }                
            
      }
      
      break;

      /* Keep Program (Terminate and stay resident) */
    case 0x31:
      DosMemChange(cu_psp, r->DX < 6 ? 6 : r->DX, 0);
      return_mode = 3;
      return_code = r->AL;
      tsr = TRUE;
      return_user();
      break;

      /* Get DPB                                                      */
    case 0x32:
      r->DL = ( r->DL == 0 ? default_drive : r->DL - 1);
      if (r->DL < lastdrive)
      {
        struct dpb FAR *dpb = CDSp->cds_table[r->DL].cdsDpb;
        if (dpb == 0 ||
            (CDSp->cds_table[r->DL].cdsFlags & CDSNETWDRV) ||
             media_check(dpb) < 0)
        {
          r->AL = 0xff;
          CritErrCode = 0x0f;
          break;
        }
        r->DS = FP_SEG(dpb);
        r->BX = FP_OFF(dpb);
        r->AL = 0;
      }
      else {
        r->AL = 0xFF;
        CritErrCode = 0x0f;

        }
      break;

/*
    case 0x33:  
    see int21_syscall
*/
      /* Get InDOS flag                                               */
    case 0x34:
      {
        BYTE FAR *p;

        p = (BYTE FAR *) ((BYTE *) & InDOS);
        r->ES = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

      /* Get Interrupt Vector                                         */
    case 0x35:
      {
        BYTE FAR *p;

        p = getvec((COUNT) r->AL);
        r->ES = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

      /* Dos Get Disk Free Space                                      */
    case 0x36:
      DosGetFree(
                  r->DL,
                  (COUNT FAR *) & r->AX,
                  (COUNT FAR *) & r->BX,
                  (COUNT FAR *) & r->CX,
                  (COUNT FAR *) & r->DX);
      break;

      /* Undocumented Get/Set Switchar                                */
    case 0x37:
      switch (r->AL)
      {
          /* Get switch character */
        case 0x00:
          r->DL = switchar;
          r->AL = 0x00;
          break;

          /* Set switch character */
        case 0x01:
          switchar = r->DL;
          r->AL = 0x00;
          break;

        default:
          goto error_invalid;
      }
      break;

      /* Get/Set Country Info                                         */
    case 0x38:
      {
      	UWORD cntry = r->AL;

      	if(cntry == 0)
      		cntry = (UWORD)-1;
      	else if(cntry == 0xff)
      		cntry = r->BX;

        if (0xffff == r->DX) {
        	/* Set Country Code */
            if((rc = DosSetCountry(cntry)) < 0)
        		goto error_invalid;
        } else {
        	/* Get Country Information */
            if((rc = DosGetCountryInformation(cntry, FP_DS_DX)) < 0)
        		goto error_invalid;
        	r->AX = r->BX = cntry;
        }
      }
      break;

      /* Dos Create Directory                                         */
    case 0x39:
      rc = DosMkdir((BYTE FAR *) FP_DS_DX);
      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Dos Remove Directory                                         */
    case 0x3a:
      rc = DosRmdir((BYTE FAR *) FP_DS_DX);
      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Dos Change Directory                                         */
    case 0x3b:
      if ((rc = DosChangeDir((BYTE FAR *) FP_DS_DX)) < 0)
        goto error_exit;
      break;

      /* Dos Create File                                              */
    case 0x3c:
      if ((rc = DosCreat(FP_DS_DX, r->CX)) < 0)
        goto error_exit;
      else
        r->AX = rc;
      break;

      /* Dos Open                                                     */
    case 0x3d:
      if ((rc = DosOpen(FP_DS_DX, r->AL)) < 0)
        goto error_exit;
      else
        r->AX = rc;
      break;

      /* Dos Close                                                    */
    case 0x3e:
      if ((rc = DosClose(r->BX)) < 0)
        goto error_exit;
      break;

      /* Dos Read                                                     */
    case 0x3f:
      rc1 = DosRead(r->BX, r->CX, FP_DS_DX, (COUNT FAR *) & rc);
      if (rc != SUCCESS)
        goto error_exit;
      else
        r->AX = rc1;
      break;

      /* Dos Write                                                    */
    case 0x40:
      rc1 = DosWrite(r->BX, r->CX, FP_DS_DX, (COUNT FAR *) & rc);
      if (rc != SUCCESS)
        goto error_exit;
      else
        r->AX = rc1;
      break;

      /* Dos Delete File                                              */
    case 0x41:
      rc = DosDelete((BYTE FAR *) FP_DS_DX);
      if (rc < 0)
        goto error_exit;
      break;

      /* Dos Seek                                                     */
    case 0x42:
      if ((rc = DosSeek(r->BX, (LONG) ((((LONG) (r->CX)) << 16) + r->DX), r->AL, &lrc)) < 0)
        goto error_exit;
      else
      {
        r->DX = (lrc >> 16);
        r->AX = (UWORD)lrc;
      }
      break;

      /* Get/Set File Attributes                                      */
    case 0x43:
      switch (r->AL)
      {
        case 0x00:
          rc = DosGetFattr((BYTE FAR *) FP_DS_DX, (UWORD FAR *) & r->CX);
          break;

        case 0x01:
          rc = DosSetFattr((BYTE FAR *) FP_DS_DX, (UWORD FAR *) & r->CX);
          break;

        default:
          goto error_invalid;
      }
      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Device I/O Control                                           */
    case 0x44:
      rc = DosDevIOctl(r);

      if (rc != SUCCESS)
        goto error_exit;
      break;

      /* Duplicate File Handle                                        */
    case 0x45:
      rc = DosDup(r->BX);
      if (rc < SUCCESS)
        goto error_exit;
      else
        r->AX = rc;
      break;

      /* Force Duplicate File Handle                                  */
    case 0x46:
      rc = DosForceDup(r->BX, r->CX);
      if (rc < SUCCESS)
        goto error_exit;
      break;

      /* Get Current Directory                                        */
    case 0x47:
      if ((rc = DosGetCuDir(r->DL, MK_FP(r->DS, r->SI))) < 0)
        goto error_exit;
      else
        r->AX = 0x0100;         /*jpp: from interrupt list */
      break;

      /* Allocate memory */
    case 0x48:
      if ((rc = DosMemAlloc(r->BX, mem_access_mode, &(r->AX), &(r->BX))) < 0)
      {
        DosMemLargest(&(r->BX));
        goto error_exit;
      }
      else
        ++(r->AX);              /* DosMemAlloc() returns seg of MCB rather than data */
      break;

      /* Free memory */
    case 0x49:
      if ((rc = DosMemFree((r->ES) - 1)) < 0)
        goto error_exit;
      break;

      /* Set memory block size */
    case 0x4a:
      {
        UWORD maxSize;

        if ((rc = DosMemChange(r->ES, r->BX, &maxSize)) < 0)
        {
          if (rc == DE_NOMEM)
            r->BX = maxSize;

#if 0
          if (cu_psp == r->ES)
          {

            psp FAR *p;

            p = MK_FP(cu_psp, 0);
            p->ps_size = r->BX + cu_psp;
          }
#endif
          goto error_exit;
        }

        break;
      }

      /* Load and Execute Program */
    case 0x4b:
      break_flg = FALSE;

      if ((rc = DosExec(r->AL, MK_FP(r->ES, r->BX), FP_DS_DX))
          != SUCCESS)
        goto error_exit;
      break;

      /* Terminate Program                                            */
    case 0x00:
      r->AX = 0x4c00;

      /* End Program                                                  */
    case 0x4c:
      if (cu_psp == RootPsp
          || ((psp FAR *) (MK_FP(cu_psp, 0)))->ps_parent == cu_psp)
        break;
      tsr = FALSE;
        int2f_Remote_call(REM_PROCESS_END, 0, 0, 0, 0, 0, 0);
        int2f_Remote_call(REM_CLOSEALL, 0, 0, 0, 0, 0, 0);
      if (ErrorMode)
      {
        ErrorMode = FALSE;
        return_mode = 2;
      }
      else if (break_flg)
      {
        break_flg = FALSE;
        return_mode = 1;
      }
      else
      {
        return_mode = 0;
      }
      return_code = r->AL;
      if (DosMemCheck() != SUCCESS)
        panic("MCB chain corrupted");
#ifdef TSC
      StartTrace();
#endif
      return_user();
      break;

      /* Get Child-program Return Value                               */
    case 0x4d:
      r->AL = return_code;
      r->AH = return_mode;
      break;

      /* Dos Find First                                               */
    case 0x4e:
      /* dta for this call is set on entry.  This     */
      /* needs to be changed for new versions.        */
      if ((rc = DosFindFirst((UCOUNT) r->CX, (BYTE FAR *) FP_DS_DX)) < 0)
        goto error_exit;
      r->AX = 0;
      break;

      /* Dos Find Next                                                */
    case 0x4f:
      /* dta for this call is set on entry.  This     */
      /* needs to be changed for new versions.        */
      if ((rc = DosFindNext()) < 0)
      {
        if (rc == DE_FILENOTFND)
          rc = DE_NFILES;
        goto error_exit;
      }
      else
        r->AX = -SUCCESS;
      break;
/*
    case 0x50:  
    case 0x51:
    see int21_syscall
*/
      /* ************UNDOCUMENTED************************************* */
      /* Get List of Lists                                            */
    case 0x52:
      {
        BYTE FAR *p;

        p = (BYTE FAR *) & DPBp;
        r->ES = FP_SEG(p);
        r->BX = FP_OFF(p);
      }
      break;

    case 0x53:
      /*  DOS 2+ internal - TRANSLATE BIOS PARAMETER BLOCK TO DRIVE PARAM BLOCK */
      bpb_to_dpb((bpb FAR *)MK_FP(r->DS, r->SI), (struct dpb FAR *)MK_FP(r->ES, r->BP));
      break;
      
      /* Get verify state                                             */
    case 0x54:
      r->AL = (verify_ena ? TRUE : FALSE);
      break;

      /* ************UNDOCUMENTED************************************* */
      /* Dos Create New Psp & set p_size                              */
    case 0x55:
      new_psp((psp FAR *) MK_FP(r->DX, 0), r->SI);
      cu_psp = r->DX;
      break;

      /* Dos Rename                                                   */
    case 0x56:
      rc = DosRename((BYTE FAR *) FP_DS_DX, (BYTE FAR *) MK_FP(r->ES, r->DI));
      if (rc < SUCCESS)
        goto error_exit;
      else
        CLEAR_CARRY_FLAG();
      break;

      /* Get/Set File Date and Time                                   */
    case 0x57:
      CLEAR_CARRY_FLAG();
      switch (r->AL)
      {
        case 0x00:
          rc = DosGetFtime(
                            (COUNT) r->BX,	/* Handle               */
                            (date FAR *) & r->DX,	/* FileDate             */
                            (time FAR *) & r->CX);	/* FileTime             */
          if (rc < SUCCESS)
            goto error_exit;
          break;

        case 0x01:
          rc = DosSetFtime(
                            (COUNT) r->BX,	/* Handle               */
                            (date FAR *) & r->DX,	/* FileDate             */
                            (time FAR *) & r->CX);	/* FileTime             */
          if (rc < SUCCESS)
            goto error_exit;
          break;

        default:
          goto error_invalid;
      }
      break;

      /* Get/Set Allocation Strategy                                  */
    case 0x58:
      CLEAR_CARRY_FLAG();
      switch (r->AL)
      {
        case 0x00:
          r->AL = mem_access_mode;
          r->AH = 0;
          break;

        case 0x01:
        {
            switch (r->BL)
            {
            case LAST_FIT:
            case LAST_FIT_U:
            case LAST_FIT_UO:
            case BEST_FIT:
            case BEST_FIT_U:
            case BEST_FIT_UO:
            case FIRST_FIT:
            case FIRST_FIT_U:
            case FIRST_FIT_UO:
                mem_access_mode = r->BL;
                break;

            default:
                goto error_invalid;
            }
        }
            break;

        case 0x02:
            r->AL = uppermem_link;
            break;

        case 0x03:
            if (uppermem_root) {
                DosUmbLink(r->BL);
                break;
            } /* else fall through */            

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
        r->AX = CritErrCode;
        r->ES = FP_SEG(CritErrDev);
        r->DI = FP_OFF(CritErrDev);
        r->CH = CritErrLocus;
        r->BH = CritErrClass;
        r->BL = CritErrAction;
        CLEAR_CARRY_FLAG();
      break;

      /* Create Temporary File */
    case 0x5a:
      if ((rc = DosMkTmp(FP_DS_DX, r->CX)) < 0)
        goto error_exit;
      else
      {
        r->AX = rc;
        CLEAR_CARRY_FLAG();
      }
      break;

      /* Create New File */
    case 0x5b:
      if (!IsDevice(FP_DS_DX) && (rc = DosOpen(FP_DS_DX, 0)) >= 0)
      {
        DosClose(rc);
        r->AX = 80;
        goto error_out;
      }
      else
      {
        if ((rc = DosCreat(FP_DS_DX, r->CX)) < 0)
          goto error_exit;
        else
        {
          r->AX = rc;
          CLEAR_CARRY_FLAG();
        }
      }
      break;

/* /// Added for SHARE.  - Ron Cemer */
      /* Lock/unlock file access */
    case 0x5c:
      if ((rc = DosLockUnlock
        (r->BX,
         (((unsigned long)r->CX)<<16)|(((unsigned long)r->DX)&0xffffL),
         (((unsigned long)r->SI)<<16)|(((unsigned long)r->DI)&0xffffL),
         ((r->AX & 0xff) != 0))) != 0)
          goto error_exit;
      CLEAR_CARRY_FLAG();
      break;
/* /// End of additions for SHARE.  - Ron Cemer */

      /* UNDOCUMENTED: server, share.exe and sda function             */
    case 0x5d:
      switch (r->AL)
      {
          /* Remote Server Call */
        case 0x00:
          {
            UWORD FAR *x = FP_DS_DX;
            r->AX = x[0];
            r->BX = x[1];
            r->CX = x[2];
            r->DX = x[3];
            r->SI = x[4];
            r->DI = x[5];
            r->DS = x[6];
            r->ES = x[7];
          }
          goto dispatch;

        case 0x06:
          r->DS = FP_SEG(internal_data);
          r->SI = FP_OFF(internal_data);
          r->CX = swap_always - internal_data;
          r->DX = swap_indos - internal_data;
          CLEAR_CARRY_FLAG();
          break;

        case 0x07:
        case 0x08:
        case 0x09:
	  rc = -int2f_Remote_call(REM_PRINTREDIR, 0, 0, r->DX, 0, 0, (MK_FP(0, Int21AX)));
	  if (rc != SUCCESS)
            goto error_exit;
          CLEAR_CARRY_FLAG();
          break;
        default:
          goto error_invalid;
      }
      break;

    case 0x5e:
      CLEAR_CARRY_FLAG();
      switch (r->AL)
      {
        case 0x00:
          r->CX = get_machine_name(FP_DS_DX);
          break;

        case 0x01:
          set_machine_name(FP_DS_DX, r->CX);
          break;

        default:
          rc = -int2f_Remote_call(REM_PRINTSET, r->BX, r->CX, r->DX, (MK_FP(r->ES, r->DI)), r->SI, (MK_FP(r->DS, Int21AX)));
	  if (rc != SUCCESS) goto error_exit;
          r->AX=SUCCESS;
          break;
      }
      break;

    case 0x5f:
      CLEAR_CARRY_FLAG();
      switch (r->AL)
      {
        case 0x07:
          if (r->DL < lastdrive) {
            CDSp->cds_table[r->DL].cdsFlags |= 0x100;
	  }
          break;

        case 0x08:
          if (r->DL < lastdrive) {
            CDSp->cds_table[r->DL].cdsFlags &= ~0x100;
	  }
          break;

        default:
          rc = -int2f_Remote_call(REM_DOREDIRECT, r->BX, r->CX, r->DX,
                                 (MK_FP(r->ES, r->DI)), r->SI, (MK_FP(r->DS, Int21AX)));
	  if (rc != SUCCESS)
            goto error_exit;
          r->AX=SUCCESS;
          break;
      }
      break;

    case 0x60:                 /* TRUENAME */
      CLEAR_CARRY_FLAG();
      if ((rc = truename(MK_FP(r->DS, r->SI),
                      adjust_far(MK_FP(r->ES, r->DI)), TRUE)) != SUCCESS)
        goto error_exit;
      break;

#ifdef TSC
      /* UNDOCUMENTED: no-op                                          */
      /*                                                              */
      /* DOS-C: tsc support                                           */
    case 0x61:
#ifdef DEBUG
      switch (r->AL)
      {
        case 0x01:
          bTraceNext = TRUE;
          break;

        case 0x02:
          bDumpRegs = FALSE;
          break;
      }
#endif
      r->AL = 0x00;
      break;
#endif

      /* UNDOCUMENTED: return current psp                             
    case 0x62: is in int21_syscall
      r->BX = cu_psp;
      break;
      */
      
      /* UNDOCUMENTED: Double byte and korean tables                  */
    case 0x63:
      {
#ifdef DBLBYTE
        static char dbcsTable[2] =
        {
          0, 0
        };
        void FAR *dp = &dbcsTable;

        r->DS = FP_SEG(dp);
        r->SI = FP_OFF(dp);
        r->AL = 0;
#else
        /* not really supported, but will pass.                 */
        r->AL = 0x00;           /*jpp: according to interrupt list */
#endif
        break;
      }
/*
    case 0x64:
      see above (invalid)
*/      

      /* Extended country info                                        */
    case 0x65:
    	switch(r->AL) {
    	case 0x20:				/* upcase single character */
            r->DL = DosUpChar(r->DL);
            break;
        case 0x21:				/* upcase memory area */
            DosUpMem(FP_DS_DX, r->CX);
            break;
        case 0x22:				/* upcase ASCIZ */
            DosUpString(FP_DS_DX);
            break;
    	case 0xA0:				/* upcase single character of filenames */
            r->DL = DosUpFChar(r->DL);
            break;
        case 0xA1:				/* upcase memory area of filenames */
            DosUpFMem(FP_DS_DX, r->CX);
            break;
        case 0xA2:				/* upcase ASCIZ of filenames */
            DosUpFString(FP_DS_DX);
            break;
        case 0x23:				/* check Yes/No response */
            r->AX = DosYesNo(r->DL);
            break;
      	default:
            if ((rc = DosGetData(
                         r->AL, r->BX, r->DX, r->CX,
                         MK_FP(r->ES, r->DI))) < 0) {
#ifdef NLS_DEBUG
   printf("DosGetData() := %d\n", rc);
#endif
               goto error_exit;
            }
#ifdef NLS_DEBUG
   printf("DosGetData() returned successfully\n", rc);
#endif

            break;
         }
		CLEAR_CARRY_FLAG();
      break;
      

      /* Code Page functions */
    case 0x66: {
    	int rc;
      switch (r->AL)
      {
        case 1:
          rc = DosGetCodepage(&r->BX, &r->DX);
			break;
        case 2:
          rc = DosSetCodepage(r->BX, r->DX);
          break;

        default:
          goto error_invalid;
      }
      if(rc != SUCCESS)
      	goto error_exit;
      CLEAR_CARRY_FLAG();
      break;
     }

      /* Set Max file handle count */
    case 0x67:
      if ((rc = SetJFTSize(r->BX)) != SUCCESS)
        goto error_exit;
      else
        CLEAR_CARRY_FLAG();
      break;

      /* Flush file buffer -- COMMIT FILE -- dummy function right now.  */
    case 0x68:
    case 0x6a:
      CLEAR_CARRY_FLAG();
      break;

      /* Get/Set Serial Number */
    case 0x69:
      rc = ( r->BL == 0 ? default_drive : r->BL - 1);
      if (rc < lastdrive)
      {
        UWORD saveCX = r->CX;
        if (CDSp->cds_table[rc].cdsFlags & CDSNETWDRV) {
          goto error_invalid;
        }
        switch(r->AL){
            case 0x00:
            r->AL = 0x0d;
            r->CX = 0x0866;
            rc = DosDevIOctl(r);
            break;

            case 0x01:
            r->AL = 0x0d;
            r->CX = 0x0846;
            rc = DosDevIOctl(r);
            break;
        }
        r->CX = saveCX;
        if (rc != SUCCESS)
          goto error_exit;
        CLEAR_CARRY_FLAG();
        break;
      }
      else
        r->AL = 0xFF;
      break;
/*
    case 0x6a: see case 0x68
    case 0x6b: dummy func: return AL=0
*/    
    /* Extended Open-Creat, not fully functional. (bits 4,5,6 of BH) */
    case 0x6c:
      {
        COUNT x = 0;
      
        if (r->AL != 0 || r->DH != 0 ||
              (r->DL&0x0f) > 0x2 || (r->DL&0xf0) > 0x10)
            goto error_invalid;
        CLEAR_CARRY_FLAG();
        if ((rc = DosOpen(MK_FP(r->DS, r->SI),
                          (r->DL&0x0f) == 0x1 ? r->BL : 0)) < 0)
        {
            if (r->DL < 0x10)
                goto error_exit;
            /* else try to create below */
        }
        else switch (r->DL & 0x0f)
        {
          case 0x0:
            /* fail if file exists */
            DosClose(rc);
            rc = DE_FILEEXISTS;
            goto error_exit;
          case 0x1:
            /* file exists and opened: OK */
            r->CX = 0x01;
            goto break_out;
          case 0x2:  
            /* file exists: replace/open */
            DosClose(rc);
            x = 1;
            break;
        }
        /* cases 0x00, 0x01 are finished now */
        if ((rc = DosCreat(MK_FP(r->DS, r->SI), r->CX)) < 0)
            goto error_exit;
            
        r->CX = x+2;
break_out:        
        r->AX = rc;
        break;
      }


    /* case 0x6d and above not implemented : see default; return AL=0 */
        
  }

#ifdef DEBUG
  if (bDumpRegs)
  {
    fbcopy((VOID FAR *) user_r, (VOID FAR *) & error_regs,
           sizeof(iregs));
    dump_regs = TRUE;
    dump();
  }
#endif
}

/* terminate handler */
VOID INRPT FAR int22_handler(void)
{
}

#if 0
	/* No kernel INT-23 handler required no longer -- 1999/04/15 ska */
/* ctrl-Break handler */
#pragma argsused
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags)
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

/* Structures needed for int 25 / int 26 */
struct HugeSectorBlock
{
  ULONG blkno;
  WORD nblks;
  BYTE FAR *buf;
};

struct int25regs
{
  UWORD es,
    ds;
  UWORD di,
    si,
    bp,
    sp;
  UWORD bx,
    dx,
    cx,
    ax;
  UWORD flags,
    ip,
    cs;
};

/* 
    this function is called from an assembler wrapper function 
*/
VOID int2526_handler(WORD mode, struct int25regs FAR * r)
{
  ULONG blkno;
  UWORD nblks;
  BYTE  FAR *buf;
  UBYTE drv;
  
  if (mode == 0x26) mode = DSKWRITE;
  else              mode = DSKREAD;
  
  drv = r->ax;

  if (drv >= lastdrive)
  {
    r->ax = 0x202;
    r->flags |= FLG_CARRY;
    return;
  }
  

  nblks = r->cx;
  blkno = r->dx;
    
  buf = MK_FP(r->ds, r->bx);
    
  if (nblks == 0xFFFF)
  {
    /*struct HugeSectorBlock FAR *lb = MK_FP(r->ds, r->bx);*/
    blkno = ((struct HugeSectorBlock FAR *)buf)->blkno;
    nblks = ((struct HugeSectorBlock FAR *)buf)->nblks;
    buf   = ((struct HugeSectorBlock FAR *)buf)->buf;
  }
  

  InDOS++;

  r->ax=dskxfer(drv, blkno, buf, nblks, mode);

  if (mode == DSKWRITE)
    if (r->ax <= 0)
        setinvld(drv);

  if (r->ax > 0)
  {
    r->flags |= FLG_CARRY;
    --InDOS;
    return;
  }


  r->ax = 0;
  r->flags &= ~FLG_CARRY;
  --InDOS;
  
}
/*
VOID int25_handler(struct int25regs FAR * r) { int2526_handler(DSKREAD,r); }
VOID int26_handler(struct int25regs FAR * r) { int2526_handler(DSKWRITE,r); }
*/




#ifdef TSC
static VOID StartTrace(VOID)
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

