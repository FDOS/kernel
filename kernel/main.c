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

#include "init-mod.h"

#define MAIN
#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *mainRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.4  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.3  2000/05/17 19:15:12  jimtabor
 * Cleanup, add and fix source.
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
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

extern UWORD DaysSinceEpoch;
extern WORD days[2][13];

INIT BOOL ReadATClock(BYTE *, BYTE *, BYTE *, BYTE *);
VOID FAR init_call_WritePCClock(ULONG);

INIT VOID configDone(VOID);
INIT static void InitIO(void);
INIT static COUNT BcdToByte(COUNT);
INIT static COUNT BcdToDay(BYTE *);

INIT static VOID init_kernel(VOID);
INIT static VOID signon(VOID);
INIT VOID kernel(VOID);
INIT VOID FsConfig(VOID);

INIT VOID main(void)
{
#ifdef KDB
  BootDrive = 1;
#endif
  init_kernel();
#ifdef DEBUG
  /* Non-portable message kludge alert!   */
  printf("KERNEL: Boot drive = %c\n", 'A' + BootDrive - 1);
#endif
  signon();
  kernel();
}

INIT static VOID init_kernel(void)
{
  COUNT i;
  os_major = MAJOR_RELEASE;
  os_minor = MINOR_RELEASE;
  cu_psp = DOS_PSP;
  nblkdev = 0;
  maxbksize = 0x200;
  switchar = '/';
  dosidle_flag = 1;

  /* Init oem hook - returns memory size in KB    */
  ram_top = init_oem();

/* Fake int 21h stack frame */
  user_r = (iregs FAR *) DOS_PSP + 0xD0;

#ifndef KDB
  for (i = 0x20; i <= 0x3f; i++)
    setvec(i, empty_handler);
#endif

  /* Initialize IO subsystem                                      */
  InitIO();
  syscon = (struct dhdr FAR *)&con_dev;
  clock = (struct dhdr FAR *)&clk_dev;

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

  /* Initialize the screen handler for backspaces                 */
  scr_pos = 0;
  break_ena = TRUE;

  /* Do first initialization of system variable buffers so that   */
  /* we can read config.sys later.  */
  lastdrive = Config.cfgLastdrive;
  PreConfig();

  /* Now config the temporary file system */
  FsConfig();

#ifndef KDB
  /* Now process CONFIG.SYS     */
  DoConfig();

  /* and do final buffer allocation. */
  PostConfig();

  /* Init the file system on emore time     */
  FsConfig();

  /* and process CONFIG.SYS one last time to load device drivers. */
  DoConfig();
  configDone();

  /* Now config the final file system     */
  FsConfig();

#endif
  /* Now to initialize all special flags, etc. */
  mem_access_mode = FIRST_FIT;
  verify_ena = FALSE;
  InDOS = 0;
  version_flags = 0;
  pDirFileNode = 0;
  dosidle_flag = 0;
}

INIT VOID FsConfig(VOID)
{
  REG COUNT i;
  date Date;
  time Time;
  BYTE x;

  /* Get the start-up date and time                               */
  Date = dos_getdate();
  Time = dos_gettime();

  /* Initialize the file tables */
  for (i = 0; i < Config.cfgFiles; i++)
    f_nodes[i].f_count = 0;

  /* The system file tables need special handling and are "hand   */
  /* built. Included is the stdin, stdout, stdaux and atdprn. */
  sfthead->sftt_next = (sfttbl FAR *) - 1;
  sfthead->sftt_count = Config.cfgFiles;
  for (i = 0; i < sfthead->sftt_count; i++)
  {
    sfthead->sftt_table[i].sft_count = 0;
    sfthead->sftt_table[i].sft_status = -1;
  }
  /* 0 is /dev/con (stdin) */
  sfthead->sftt_table[0].sft_count = 1;
  sfthead->sftt_table[0].sft_mode = SFT_MREAD;
  sfthead->sftt_table[0].sft_attrib = 0;
  sfthead->sftt_table[0].sft_flags =
      ((con_dev.dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF | SFT_FCONIN | SFT_FCONOUT;
  sfthead->sftt_table[0].sft_psp = DOS_PSP;
  sfthead->sftt_table[0].sft_date = Date;
  sfthead->sftt_table[0].sft_time = Time;
  fbcopy(
          (VOID FAR *) "CON        ",
          (VOID FAR *) sfthead->sftt_table[0].sft_name, 11);
  sfthead->sftt_table[0].sft_dev = (struct dhdr FAR *)&con_dev;

  /* 1 is /dev/con (stdout)     */
  sfthead->sftt_table[1].sft_count = 1;
  sfthead->sftt_table[1].sft_mode = SFT_MWRITE;
  sfthead->sftt_table[1].sft_attrib = 0;
  sfthead->sftt_table[1].sft_flags =
      ((con_dev.dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF | SFT_FCONIN | SFT_FCONOUT;
  sfthead->sftt_table[1].sft_psp = DOS_PSP;
  sfthead->sftt_table[1].sft_date = Date;
  sfthead->sftt_table[1].sft_time = Time;
  fbcopy(
          (VOID FAR *) "CON        ",
          (VOID FAR *) sfthead->sftt_table[1].sft_name, 11);
  sfthead->sftt_table[1].sft_dev = (struct dhdr FAR *)&con_dev;

  /* 2 is /dev/con (stderr)     */
  sfthead->sftt_table[2].sft_count = 1;
  sfthead->sftt_table[2].sft_mode = SFT_MWRITE;
  sfthead->sftt_table[2].sft_attrib = 0;
  sfthead->sftt_table[2].sft_flags =
      ((con_dev.dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF | SFT_FCONIN | SFT_FCONOUT;
  sfthead->sftt_table[2].sft_psp = DOS_PSP;
  sfthead->sftt_table[2].sft_date = Date;
  sfthead->sftt_table[2].sft_time = Time;
  fbcopy(
          (VOID FAR *) "CON        ",
          (VOID FAR *) sfthead->sftt_table[2].sft_name, 11);
  sfthead->sftt_table[2].sft_dev = (struct dhdr FAR *)&con_dev;

  /* 3 is /dev/aux                                                */
  sfthead->sftt_table[3].sft_count = 1;
  sfthead->sftt_table[3].sft_mode = SFT_MRDWR;
  sfthead->sftt_table[3].sft_attrib = 0;
  sfthead->sftt_table[3].sft_flags =
      ((aux_dev.dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE;
  sfthead->sftt_table[3].sft_psp = DOS_PSP;
  sfthead->sftt_table[3].sft_date = Date;
  sfthead->sftt_table[3].sft_time = Time;
  fbcopy(
          (VOID FAR *) "AUX        ",
          (VOID FAR *) sfthead->sftt_table[3].sft_name, 11);
  sfthead->sftt_table[3].sft_dev = (struct dhdr FAR *)&aux_dev;

  /* 4 is /dev/prn                                                */
  sfthead->sftt_table[4].sft_count = 1;
  sfthead->sftt_table[4].sft_mode = SFT_MWRITE;
  sfthead->sftt_table[4].sft_attrib = 0;
  sfthead->sftt_table[4].sft_flags =
      ((prn_dev.dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE;
  sfthead->sftt_table[4].sft_psp = DOS_PSP;
  sfthead->sftt_table[4].sft_date = Date;
  sfthead->sftt_table[4].sft_time = Time;
  fbcopy(
          (VOID FAR *) "PRN        ",
          (VOID FAR *) sfthead->sftt_table[4].sft_name, 11);
  sfthead->sftt_table[4].sft_dev = (struct dhdr FAR *)&prn_dev;

  /* Log-in the default drive.  */
  /* Get the boot drive from the ipl and use it for default.  */
  default_drive = BootDrive - 1;

  /* Initialzie the current directory structures    */
  for (i = 0; i < lastdrive + 1; i++)
  {

    fbcopy((VOID FAR *) "A:\\\0",
           (VOID FAR *) CDSp->cds_table[i].cdsCurrentPath, 4);
    CDSp->cds_table[i].cdsCurrentPath[0] += i;

    if (i < nblkdev)
    {
      CDSp->cds_table[i].cdsDpb = &blk_devices[i];
      CDSp->cds_table[i].cdsFlags = CDSPHYSDRV;
    }
    else
    {
      CDSp->cds_table[i].cdsFlags = 0;
    }
    CDSp->cds_table[i].cdsStrtClst = 0xffff;
    CDSp->cds_table[i].cdsParam = 0xffff;
    CDSp->cds_table[i].cdsStoreUData = 0xffff;
    CDSp->cds_table[i].cdsJoinOffset = 2;
  }

  /* Initialze the disk buffer management functions */
  init_buffers();
}

INIT static VOID signon()
{
  printf("\nFreeDOS Kernel compatibility %d.%d\n%s\n",
         os_major, os_minor, copyright);
  printf(os_release,
         REVISION_MAJOR, REVISION_MINOR, REVISION_SEQ,
         BUILD);
}

INIT static VOID kernel()
{
  seg asize;
  BYTE FAR *ep,
   *sp;
  COUNT ret_code;
#ifndef KDB
  static BYTE *path = "PATH=.";
#endif

#ifdef KDB
  kdb();
#else
  /* create the master environment area   */
  if (DosMemAlloc(0x20, FIRST_FIT, (seg FAR *) & master_env, (seg FAR *) & asize) < 0)
    fatal("cannot allocate master environment space");

  /* populate it with the minimum environment */
  ++master_env;
  ep = MK_FP(master_env, 0);

  for (sp = path; *sp != 0;)
    *ep++ = *sp++;

  *ep++ = '\0';
  *ep++ = '\0';
  *((int FAR *)ep) = 0;
  ep += sizeof(int);
#endif
  RootPsp = ~0;
  p_0();
}

extern BYTE FAR *lpBase;

/* If cmdLine is NULL, this is an internal driver */

VOID init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine)
{
  request rq;
  ULONG memtop = ((ULONG) ram_top) << 10;
  ULONG maxmem = memtop - ((ULONG) FP_SEG(dhp) << 4);

  if (maxmem >= 0x10000)
    maxmem = 0xFFFF;

  rq.r_unit = 0;
  rq.r_status = 0;
  rq.r_command = C_INIT;
  rq.r_length = sizeof(request);
  rq.r_endaddr = MK_FP(FP_SEG(dhp), maxmem);
  rq.r_bpbptr = (void FAR *)(cmdLine ? cmdLine : "\n");
  rq.r_firstunit = nblkdev;

  execrh((request FAR *) & rq, dhp);

  if (cmdLine)
    lpBase = rq.r_endaddr;

  /* check for a block device and update  device control block    */
  if (!(dhp->dh_attr & ATTR_CHAR) && (rq.r_nunits != 0))
  {
    REG COUNT Index;

    for (Index = 0; Index < rq.r_nunits; Index++)
    {
      if (nblkdev)
        blk_devices[nblkdev - 1].dpb_next = &blk_devices[nblkdev];

      blk_devices[nblkdev].dpb_next = (void FAR *)0xFFFFFFFF;
      blk_devices[nblkdev].dpb_unit = nblkdev;
      blk_devices[nblkdev].dpb_subunit = Index;
      blk_devices[nblkdev].dpb_device = dhp;
      blk_devices[nblkdev].dpb_flags = M_CHANGED;
      if ((CDSp != 0) && (nblkdev <= lastdrive))
      {
        CDSp->cds_table[nblkdev].cdsDpb = &blk_devices[nblkdev];
        CDSp->cds_table[nblkdev].cdsFlags = CDSPHYSDRV;
      }
      ++nblkdev;
    }
  }
  DPBp = &blk_devices[0];
}

struct dhdr FAR *link_dhdr(struct dhdr FAR * lp, struct dhdr FAR * dhp, BYTE FAR * cmdLine)
{
  lp->dh_next = dhp;
  init_device(dhp, cmdLine);
  return dhp;
}

INIT static void InitIO(void)
{
  BYTE bcd_days[4],
    bcd_minutes,
    bcd_hours,
    bcd_seconds;
  ULONG ticks;

  /* Initialize driver chain                                      */
  nul_dev.dh_next = (struct dhdr FAR *)&con_dev;
  setvec(0x29, int29_handler);  /* Requires Fast Con Driver     */
  init_device((struct dhdr FAR *)&con_dev, NULL);
  init_device((struct dhdr FAR *)&clk_dev, NULL);
  init_device((struct dhdr FAR *)&blk_dev, NULL);
  /* If AT clock exists, copy AT clock time to system clock */
  if (!ReadATClock(bcd_days, &bcd_hours, &bcd_minutes, &bcd_seconds))
  {
    DaysSinceEpoch = BcdToDay(bcd_days);
    /*
     * This is a rather tricky calculation. The number of timer ticks per
     * second is not exactly 18.2, but rather 0x1800b0 / 86400 = 19663 / 1080
     * (the timer interrupt updates the midnight flag when the tick count
     * reaches 0x1800b0). Fortunately, 86400 * 19663 = 1698883200 < ULONG_MAX,
     * so we can simply multiply the number of seconds by 19663 without
     * worrying about overflow. :) -- ror4
     */
    ticks = (3600ul * BcdToByte(bcd_hours) +
             60ul * BcdToByte(bcd_minutes) +
             BcdToByte(bcd_seconds)) * 19663ul / 1080ul;
    WritePCClock(ticks);
  }
}

INIT static COUNT BcdToByte(COUNT x)
{
  return ((((x) >> 4) & 0xf) * 10 + ((x) & 0xf));
}

INIT static COUNT BcdToDay(BYTE * x)
{
  UWORD mon,
    day,
    yr;

  mon = BcdToByte(x[1]) - 1;
  day = BcdToByte(x[0]) - 1;
  yr = 100 * BcdToByte(x[3]) + BcdToByte(x[2]);
  if (yr < 1980)
    return 0;
  else
  {
    day += days[is_leap_year(yr)][mon];
    while (--yr >= 1980)
      day += is_leap_year(yr) ? 366 : 365;
    return day;
  }
}

