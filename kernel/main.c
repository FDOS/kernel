/****************************************************************/
/*                                                              */
/*                           main.c                             */
/*                            DOS-C                             */
/*                                                              */
/*                    Main Kernel Functions                     */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
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
#include "init-mod.h"
#include "dyndata.h"
#include "init-dat.h"

GLOBAL BYTE copyright[] =
    "(C) Copyright 1995-2001 Pasquale J. Villani and The FreeDOS Project.\n"
    "All Rights Reserved. This is free software and comes with ABSOLUTELY NO\n"
    "WARRANTY; you can redistribute it and/or modify it under the terms of the\n"
    "GNU General Public License as published by the Free Software Foundation;\n"
    "either version 2, or (at your option) any later version.\n";

/*
  These are the far variables from the DOS data segment that we need here. The
  init procedure uses a different default DS data segment, which is discarded
  after use. I hope to clean this up to use the DOS List of List and Swappable
  Data Area obtained via INT21.

  -- Bart
 */
extern UBYTE DOSFAR ASM nblkdev, DOSFAR ASM lastdrive;  /* value of last drive                  */

GLOBAL BYTE DOSFAR os_major,    /* major version number                 */
  DOSFAR os_minor,              /* minor version number                 */
  DOSFAR dosidle_flag, DOSFAR ASM BootDrive,        /* Drive we came up from                */
  DOSFAR ASM default_drive;         /* default drive for dos                */

GLOBAL BYTE DOSFAR os_release[];
GLOBAL seg DOSFAR RootPsp;      /* Root process -- do not abort         */

extern struct dpb FAR *DOSFAR ASM DPBp;     /* First drive Parameter Block          */
extern cdstbl FAR *DOSFAR ASM CDSp; /* Current Directory Structure          */

extern struct dhdr FAR *DOSFAR ASM clock,   /* CLOCK$ device                        */
  FAR * DOSFAR ASM syscon;          /* console device                       */
extern struct dhdr ASM DOSTEXTFAR con_dev,  /* console device drive                 */
  DOSTEXTFAR ASM clk_dev,           /* Clock device driver                  */
  DOSTEXTFAR ASM blk_dev;           /* Block device (Disk) driver           */
extern iregs FAR *DOSFAR ASM user_r;        /* User registers for int 21h call      */
extern BYTE FAR ASM _HMATextEnd[];

extern struct _KernelConfig FAR ASM LowKernelConfig;

#ifdef VERSION_STRINGS
static BYTE *mainRcsId =
    "$Id$";
#endif

struct _KernelConfig InitKernelConfig = { "", 0, 0, 0, 0, 0, 0 };

extern WORD days[2][13];
extern BYTE FAR *lpBase;
extern BYTE FAR *lpOldTop;
extern BYTE FAR *lpTop;
extern BYTE FAR *upBase;
extern BYTE ASM _ib_start[], ASM _ib_end[], ASM _init_end[];
extern UWORD ram_top;               /* How much ram in Kbytes               */

VOID configDone(VOID);
STATIC VOID InitIO(void);

STATIC VOID update_dcb(struct dhdr FAR *);
STATIC VOID init_kernel(VOID);
STATIC VOID signon(VOID);
STATIC VOID kernel(VOID);
STATIC VOID FsConfig(VOID);
STATIC VOID InitPrinters(VOID);

#ifdef _MSC_VER
BYTE _acrtused = 0;
#endif

#ifdef _MSC_VER
__segment DosDataSeg = 0;       /* serves for all references to the DOS DATA segment 
                                   necessary for MSC+our funny linking model
                                 */
__segment DosTextSeg = 0;

#endif

VOID ASMCFUNC FreeDOSmain(void)
{
#ifdef _MSC_VER
  extern FAR DATASTART;
  extern FAR prn_dev;
  DosDataSeg = (__segment) & DATASTART;
  DosTextSeg = (__segment) & prn_dev;
#endif

                        
                        /*  if the kernel has been UPX'ed,
                                CONFIG info is stored at 50:e2 ..fc
                            and the bootdrive (passed from BIOS)
                            at 50:e0
                        */    
                        
  if (fmemcmp(MK_FP(0x50,0xe0+2),"CONFIG",6) == 0)      /* UPX */
        {
        fmemcpy(&InitKernelConfig, MK_FP(0x50,0xe0+2), sizeof(InitKernelConfig));
    
    BootDrive = *(BYTE FAR *)MK_FP(0x50,0xe0);

        BootDrive ++;
        
    if ((unsigned)BootDrive >= 0x80)
        BootDrive += 3-1-128;
    
    
    *(DWORD FAR *)MK_FP(0x50,0xe0+2) = 0;
     
    } 
  else
    {       

        fmemcpy(&InitKernelConfig, &LowKernelConfig, sizeof(InitKernelConfig));
    }

  setvec(0, int0_handler);      /* zero divide */
  setvec(1, empty_handler);     /* single step */
  setvec(3, empty_handler);     /* debug breakpoint */
  setvec(6, empty_handler);     /* invalid opcode */

  /* clear the Init BSS area (what normally the RTL does */
  memset(_ib_start, 0, _ib_end - _ib_start);

  init_kernel();

#ifdef DEBUG
  /* Non-portable message kludge alert!   */
  printf("KERNEL: Boot drive = %c\n", 'A' + BootDrive - 1);
#endif
  signon();
  kernel();
}

/*
    InitializeAllBPBs()
    
    or MakeNortonDiskEditorHappy()

    it has been determined, that FDOS's BPB tables are initialized,
    only when used (like DIR H:).
    at least one known utility (norton DE) seems to access them directly.
    ok, so we access for all drives, that the stuff gets build
*/
void InitializeAllBPBs(VOID)
{
  static char filename[] = "A:-@JUNK@-.TMP";
  int drive, fileno;
  for (drive = 'C'; drive < 'A' + nblkdev; drive++)
  {
    filename[0] = drive;
    if ((fileno = open(filename, O_RDONLY)) >= 0)
      close(fileno);
  }
}

STATIC void init_kernel(void)
{
  COUNT i;

  os_major = MAJOR_RELEASE;
  os_minor = MINOR_RELEASE;

  /* Init oem hook - returns memory size in KB    */
  ram_top = init_oem();

  /* move kernel to high conventional RAM, just below the init code */
  lpTop = MK_FP(ram_top * 64 - (FP_OFF(_init_end) + 15) / 16 -
                (FP_OFF(_HMATextEnd) + 15) / 16, 0);

  MoveKernel(FP_SEG(lpTop));
  lpOldTop = lpTop = MK_FP(FP_SEG(lpTop) - 0xfff, 0xfff0);

/* Fake int 21h stack frame */
  user_r = (iregs FAR *) MK_FP(DOS_PSP, 0xD0);

#ifndef KDB
  for (i = 0x20; i <= 0x3f; i++)
    setvec(i, empty_handler);
#endif

  /* Initialize IO subsystem                                      */
  InitIO();
  InitPrinters();

#ifndef KDB
  /* set interrupt vectors                                        */
  setvec(0x1b, got_cbreak);
  setvec(0x20, int20_handler);
  setvec(0x21, int21_handler);
  setvec(0x22, int22_handler);
  setvec(0x23, empty_handler);
  setvec(0x24, int24_handler);
  setvec(0x25, low_int25_handler);
  setvec(0x26, low_int26_handler);
  setvec(0x27, int27_handler);
  setvec(0x28, int28_handler);
  setvec(0x2a, int2a_handler);
  setvec(0x2f, int2f_handler);
#endif

  init_PSPSet(DOS_PSP);
  init_PSPInit(DOS_PSP);

  /* Do first initialization of system variable buffers so that   */
  /* we can read config.sys later.  */
  lastdrive = Config.cfgLastdrive;

  /*  init_device((struct dhdr FAR *)&blk_dev, NULL, NULL, ram_top); */
  blk_dev.dh_name[0] = dsk_init();

  PreConfig();

  /* Number of units */
  if (blk_dev.dh_name[0] > 0)
    update_dcb(&blk_dev);

  /* Now config the temporary file system */
  FsConfig();

#ifndef KDB
  /* Now process CONFIG.SYS     */
  DoConfig(0);
  DoConfig(1);

  /* Close all (device) files */
  for (i = 0; i < lastdrive; i++)
    close(i);

  /* and do final buffer allocation. */
  PostConfig();
  nblkdev = 0;
  update_dcb(&blk_dev);

  /* Init the file system one more time     */
  FsConfig();

  /* and process CONFIG.SYS one last time to load device drivers. */
  DoConfig(2);
  configDone();

  /* Close all (device) files */
  for (i = 0; i < lastdrive; i++)
    close(i);

  /* Now config the final file system     */
  FsConfig();

#endif
  InitializeAllBPBs();
}

STATIC VOID FsConfig(VOID)
{
  REG COUNT i;
  struct dpb FAR *dpb;

  /* Log-in the default drive.  */
  /* Get the boot drive from the ipl and use it for default.  */
  default_drive = BootDrive - 1;
  dpb = DPBp;

  /* Initialize the current directory structures    */
  for (i = 0; i < lastdrive; i++)
  {
    struct cds FAR *pcds_table = &CDSp->cds_table[i];

    fmemcpy(pcds_table->cdsCurrentPath, "A:\\\0", 4);

    pcds_table->cdsCurrentPath[0] += i;

    if (i < nblkdev && (ULONG) dpb != 0xffffffffl)
    {
      pcds_table->cdsDpb = dpb;
      pcds_table->cdsFlags = CDSPHYSDRV;
      dpb = dpb->dpb_next;
    }
    else
    {
      pcds_table->cdsFlags = 0;
    }
    pcds_table->cdsStrtClst = 0xffff;
    pcds_table->cdsParam = 0xffff;
    pcds_table->cdsStoreUData = 0xffff;
    pcds_table->cdsJoinOffset = 2;
  }

  /* The system file tables need special handling and are "hand   */
  /* built. Included is the stdin, stdout, stdaux and stdprn. */

  /* 0 is /dev/con (stdin) */
  open("CON", O_RDWR);

  /* 1 is /dev/con (stdout)     */
  dup2(STDIN, STDOUT);

  /* 2 is /dev/con (stderr)     */
  dup2(STDIN, STDERR);

  /* 3 is /dev/aux                                                */
  open("AUX", O_RDWR);

  /* 4 is /dev/prn                                                */
  open("PRN", O_WRONLY);

  /* Initialize the disk buffer management functions */
  /* init_call_init_buffers(); done from CONFIG.C   */
}

STATIC VOID signon()
{
  printf("\n%S", (void FAR *)os_release);

  printf("Kernel compatibility %d.%d", os_major, os_minor);

#if defined(__TURBOC__)
  printf(" - TURBOC");
#elif defined(_MSC_VER)
  printf(" - MSC");
#elif defined(__WATCOMC__)
  printf(" - WATCOMC");
#else
  generate some bullshit error here, as the compiler should be known
#endif
#if defined (I386)
    printf(" - 80386 CPU required");
#elif defined (I186)
    printf(" - 80186 CPU required");
#endif

#ifdef WITHFAT32
  printf(" - FAT32 support");
#endif
  printf("\n\n%S", (void FAR *)copyright);
}

STATIC void kernel()
{
#if 0
  BYTE FAR *ep, *sp;
#endif
  exec_blk exb;
  CommandTail Cmd;
  int rc;

#ifndef KDB
  static BYTE master_env[] = "PATH=.\0\0\0\0\0";
/*  static BYTE *path = "PATH=.";*/
#endif

#ifdef KDB
  kdb();
#else
#if 0
  /* create the master environment area   */

  if (allocmem(0x2, &exb.exec.env_seg))
    init_fatal("cannot allocate master environment space");

  /* populate it with the minimum environment */
  ++exb.exec.env_seg;
  ep = MK_FP(exb.exec.env_seg, 0);

  for (sp = path; *sp != 0;)
    *ep++ = *sp++;

  *ep++ = '\0';
  *ep++ = '\0';
  *((int FAR *)ep) = 0;
  ep += sizeof(int);
#else
  exb.exec.env_seg = DOS_PSP + 8;
  fmemcpy(MK_FP(exb.exec.env_seg, 0), master_env, sizeof(master_env));
#endif
#endif

  RootPsp = ~0;

  /* process 0       */
  /* Execute command.com /P from the drive we just booted from    */
  fstrncpy(Cmd.ctBuffer, Config.cfgInitTail,
           sizeof(Config.cfgInitTail) - 1);

  for (Cmd.ctCount = 0; Cmd.ctCount < 127; Cmd.ctCount++)
    if (Cmd.ctBuffer[Cmd.ctCount] == '\r')
      break;

  /* if stepping CONFIG.SYS (F5/F8), tell COMMAND.COM about it */

  if (Cmd.ctCount < 127 - 3)
  {
    extern int singleStep;
    extern int SkipAllConfig;
    char *insertString = NULL;

    if (singleStep)
      insertString = " /Y";     /* single step AUTOEXEC */

    if (SkipAllConfig)
      insertString = " /D";     /* disable AUTOEXEC */

    if (insertString)
    {

      /* insert /D, /Y as first argument */
      int cmdEnd, i, slen = strlen(insertString);

      for (cmdEnd = 0; cmdEnd < 127; cmdEnd++)
      {
        if (Cmd.ctBuffer[cmdEnd] == ' ' ||
            Cmd.ctBuffer[cmdEnd] == '\t' || Cmd.ctBuffer[cmdEnd] == '\r')
        {
          for (i = 127 - slen; i >= cmdEnd; i--)
            Cmd.ctBuffer[i + slen] = Cmd.ctBuffer[i];

          fmemcpy(&Cmd.ctBuffer[cmdEnd], insertString, slen);

          Cmd.ctCount += slen;

          break;
        }
      }
    }
  }

  exb.exec.cmd_line = (CommandTail FAR *) & Cmd;
  exb.exec.fcb_1 = exb.exec.fcb_2 = (fcb FAR *) 0;

#ifdef DEBUG
  printf("Process 0 starting: %s\n\n", Config.cfgInit);
#endif

  while ((rc =
          init_DosExec(Config.cfgP_0_startmode, &exb,
                       Config.cfgInit)) != SUCCESS)
  {
    BYTE *pLine;
    printf("\nBad or missing Command Interpreter: %d - %s\n", rc,
           Cmd.ctBuffer);
    printf
        ("\nPlease enter the correct location (for example C:\\COMMAND.COM):\n");
    rc = read(STDIN, Cmd.ctBuffer, sizeof(Cmd.ctBuffer) - 1);
    Cmd.ctBuffer[rc] = '\0';

    /* Get the string argument that represents the new init pgm     */
    pLine = GetStringArg(Cmd.ctBuffer, Config.cfgInit);

    /* Now take whatever tail is left and add it on as a single     */
    /* string.                                                      */
    strcpy(Cmd.ctBuffer, pLine);

    /* and add a DOS new line just to be safe                       */
    strcat(Cmd.ctBuffer, "\r\n");

    Cmd.ctCount = rc - (pLine - Cmd.ctBuffer);

#ifdef DEBUG
    printf("Process 0 starting: %s\n\n", Config.cfgInit);
#endif
  }
  printf("\nSystem shutdown complete\nReboot now.\n");
  for (;;) ;
}

/* check for a block device and update  device control block    */
STATIC VOID update_dcb(struct dhdr FAR * dhp)
{
  REG COUNT Index;
  COUNT nunits = dhp->dh_name[0];
  struct dpb FAR *dpb;

  if (nblkdev == 0)
    dpb = DPBp;
  else
  {
    for (dpb = DPBp; (ULONG) dpb->dpb_next != 0xffffffffl;
         dpb = dpb->dpb_next)
      ;
    dpb = dpb->dpb_next =
        (struct dpb FAR *)KernelAlloc(nunits * sizeof(struct dpb));
  }

  for (Index = 0; Index < nunits; Index++)
  {
    dpb->dpb_next = dpb + 1;
    dpb->dpb_unit = nblkdev;
    dpb->dpb_subunit = Index;
    dpb->dpb_device = dhp;
    dpb->dpb_flags = M_CHANGED;
    if ((CDSp != 0) && (nblkdev < lastdrive))
    {
      CDSp->cds_table[nblkdev].cdsDpb = dpb;
      CDSp->cds_table[nblkdev].cdsFlags = CDSPHYSDRV;
    }
    ++dpb;
    ++nblkdev;
  }
  (dpb - 1)->dpb_next = (void FAR *)0xFFFFFFFFl;
}

/* If cmdLine is NULL, this is an internal driver */

BOOL init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine, COUNT mode,
                 COUNT r_top)
{
  request rq;

  UCOUNT maxmem = ((UCOUNT) r_top << 6) - FP_SEG(dhp);

  if (maxmem >= 0x1000)
    maxmem = 0xFFFF;
  else
    maxmem <<= 4;

  rq.r_unit = 0;
  rq.r_status = 0;
  rq.r_command = C_INIT;
  rq.r_length = sizeof(request);
  rq.r_endaddr = MK_FP(FP_SEG(dhp), maxmem);
  rq.r_bpbptr = (void FAR *)(cmdLine ? cmdLine : "\n");
  rq.r_firstunit = nblkdev;

  execrh((request FAR *) & rq, dhp);

/*
 *  Added needed Error handle
 */
  if (rq.r_status & S_ERROR)
    return TRUE;

  if (cmdLine)
  {
    if (mode)
    {
      /* Don't link in device drivers which do not take up memory */
      if (rq.r_endaddr == (BYTE FAR *) dhp)
        return TRUE;
      else
        upBase = rq.r_endaddr;
    }
    else
    {
      if (rq.r_endaddr == (BYTE FAR *) dhp)
        return TRUE;
      else
        lpBase = rq.r_endaddr;
    }
  }

  if (!(dhp->dh_attr & ATTR_CHAR) && (rq.r_nunits != 0))
  {
    dhp->dh_name[0] = rq.r_nunits;
    update_dcb(dhp);
  }

  if (dhp->dh_attr & ATTR_CONIN)
    syscon = dhp;
  else if (dhp->dh_attr & ATTR_CLOCK)
    clock = dhp;

  return FALSE;
}

STATIC void InitIO(void)
{
  /* Initialize driver chain                                      */
  setvec(0x29, int29_handler);  /* Requires Fast Con Driver     */
  init_device(&con_dev, NULL, NULL, ram_top);
  init_device(&clk_dev, NULL, NULL, ram_top);
}

/* issue an internal error message                              */
VOID init_fatal(BYTE * err_msg)
{
  printf("\nInternal kernel error - %s\nSystem halted\n", err_msg);
  for (;;) ;
}

/*
       Initialize all printers
 
       this should work. IMHO, this might also be done on first use
       of printer, as I never liked the noise by a resetting printer, and
       I usually much more often reset my system, then I print :-)
 */

STATIC VOID InitPrinters(VOID)
{
  iregs r;
  int num_printers, i;

  init_call_intr(0x11, &r);     /* get equipment list */

  num_printers = (r.a.x >> 14) & 3;     /* bits 15-14 */

  for (i = 0; i < num_printers; i++)
  {
    r.a.x = 0x0100;             /* initialize printer */
    r.d.x = i;
    init_call_intr(0x17, &r);
  }
}

/*
 * Log: main.c,v - for newer log entries do "cvs log main.c"
 *
 * Revision 1.14  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.13  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.12  1999/09/23 04:40:48  jprice
 * *** empty log message ***
 *
 * Revision 1.10  1999/08/25 03:18:09  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.9  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.8  1999/04/16 12:21:22  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.7  1999/04/16 00:53:33  jprice
 * Optimized FAT handling
 *
 * Revision 1.6  1999/04/12 03:21:17  jprice
 * more ror4 patches.  Changes for multi-block IO
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
 * Revision 1.1.1.1  1999/03/29 15:41:18  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.3  1999/01/30 08:28:12  jprice
 * Clean up; Fixed bug with set attribute function.
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.12   06 Dec 1998  8:45:30   patv
 * Changed due to new I/O subsystem.
 *
 *    Rev 1.11   22 Jan 1998  4:09:24   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.10   04 Jan 1998 23:15:20   patv
 * Changed Log for strip utility
 *
 *    Rev 1.9   04 Jan 1998 17:26:16   patv
 * Corrected subdirectory bug
 *
 *    Rev 1.8   03 Jan 1998  8:36:48   patv
 * Converted data area to SDA format
 *
 *    Rev 1.7   06 Feb 1997 21:35:46   patv
 * Modified to support new version format and changed debug message to
 * output drive letter instead of number.
 *
 *    Rev 1.6   22 Jan 1997 13:05:02   patv
 * Now does correct default drive initialization.
 *
 *    Rev 1.5   16 Jan 1997 12:47:00   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:03:32   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.1   01 Sep 1995 17:54:18   patv
 * First GPL release.
 *
 *    Rev 1.0   02 Jul 1995  8:33:18   patv
 * Initial revision.
 */
