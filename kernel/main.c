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
/* write to the Free Software Foundation, Inc.,                 */
/* 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.     */
/****************************************************************/

#include "portab.h"
#include "init-mod.h"
#include "dyndata.h"

#ifdef VERSION_STRINGS
static BYTE *mainRcsId =
    "$Id$";
#endif

static char copyright[] =
    "(C) Copyright 1995-2004 Pasquale J. Villani and The FreeDOS Project.\n"
    "All Rights Reserved. This is free software and comes with ABSOLUTELY NO\n"
    "WARRANTY; you can redistribute it and/or modify it under the terms of the\n"
    "GNU General Public License as published by the Free Software Foundation;\n"
    "either version 2, or (at your option) any later version.\n";

struct _KernelConfig InitKernelConfig = { "", 0, 0, 0, 0, 0, 0, 0 };

STATIC VOID InitIO(void);

STATIC VOID update_dcb(struct dhdr FAR *);
STATIC VOID init_kernel(VOID);
STATIC VOID signon(VOID);
STATIC VOID kernel(VOID);
STATIC VOID FsConfig(VOID);
STATIC VOID InitPrinters(VOID);
STATIC VOID InitSerialPorts(VOID);
STATIC void CheckContinueBootFromHarddisk(void);

#ifdef _MSC_VER
BYTE _acrtused = 0;

__segment DosDataSeg = 0;       /* serves for all references to the DOS DATA segment 
                                   necessary for MSC+our funny linking model
                                 */
__segment DosTextSeg = 0;

struct lol FAR *LoL;

#else
struct lol FAR *LoL = &DATASTART;
#endif

VOID ASMCFUNC FreeDOSmain(void)
{
  unsigned char drv;

#ifdef _MSC_VER
  extern FAR prn_dev;
  DosDataSeg = (__segment) & DATASTART;
  DosTextSeg = (__segment) & prn_dev;
  LoL = &DATASTART;
#endif

                        /*  if the kernel has been UPX'ed,
                                CONFIG info is stored at 50:e2 ..fc
                            and the bootdrive (passed from BIOS)
                            at 50:e0
                        */

  if (fmemcmp(MK_FP(0x50,0xe0+2),"CONFIG",6) == 0)      /* UPX */
  {
    fmemcpy(&InitKernelConfig, MK_FP(0,0x5e0+2), sizeof(InitKernelConfig));

    drv = *(UBYTE FAR *)MK_FP(0,0x5e0) + 1;
    *(DWORD FAR *)MK_FP(0,0x5e0+2) = 0;
  }
  else
  {
    drv = LoL->BootDrive + 1;
    *(UBYTE FAR *)MK_FP(0,0x5e0) = drv - 1;
    fmemcpy(&InitKernelConfig, &LowKernelConfig, sizeof(InitKernelConfig));
  }

  if (drv >= 0x80)
    drv = 3; /* C: */
  LoL->BootDrive = drv;

  setvec(0, int0_handler);      /* zero divide */
  setvec(1, empty_handler);     /* single step */
  setvec(3, empty_handler);     /* debug breakpoint */
  setvec(6, int6_handler);      /* invalid opcode */


  CheckContinueBootFromHarddisk();



  /* clear the Init BSS area (what normally the RTL does */
  memset(_ib_start, 0, _ib_end - _ib_start);

  signon();
  init_kernel();

#ifdef DEBUG
  /* Non-portable message kludge alert!   */
  printf("KERNEL: Boot drive = %c\n", 'A' + LoL->BootDrive - 1);
#endif

  DoInstall();

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
  for (drive = 'C'; drive < 'A' + LoL->nblkdev; drive++)
  {
    filename[0] = drive;
    if ((fileno = open(filename, O_RDONLY)) >= 0)
      close(fileno);
  }
}

STATIC void init_kernel(void)
{
  COUNT i;

  LoL->os_setver_major = LoL->os_major = MAJOR_RELEASE;
  LoL->os_setver_minor = LoL->os_minor = MINOR_RELEASE;

  /* Init oem hook - returns memory size in KB    */
  ram_top = init_oem();

  /* move kernel to high conventional RAM, just below the init code */
  lpTop = MK_FP(ram_top * 64 - (FP_OFF(_init_end) + 15) / 16 -
                (FP_OFF(_HMATextEnd) + 15) / 16, 0);

  MoveKernel(FP_SEG(lpTop));
  lpOldTop = lpTop = MK_FP(FP_SEG(lpTop) - 0xfff, 0xfff0);

  for (i = 0x20; i <= 0x3f; i++)
    setvec(i, empty_handler);

  /* Initialize IO subsystem                                      */
  InitIO();
  InitPrinters();
  InitSerialPorts();

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
  pokeb(0, 0x30 * 4, 0xea);
  pokel(0, 0x30 * 4 + 1, (ULONG)cpm_entry);

  init_PSPSet(DOS_PSP);
  set_DTA(MK_FP(DOS_PSP, 0x80));
  init_PSPInit(DOS_PSP);
  ((psp far *)MK_FP(DOS_PSP, 0))->ps_environ = DOS_PSP + 8;

  Init_clk_driver();

  /* Do first initialization of system variable buffers so that   */
  /* we can read config.sys later.  */
  LoL->lastdrive = Config.cfgLastdrive;

  /*  init_device((struct dhdr FAR *)&blk_dev, NULL, 0, &ram_top); */
  blk_dev.dh_name[0] = dsk_init();

  PreConfig();

  /* Number of units */
  if (blk_dev.dh_name[0] > 0)
    update_dcb(&blk_dev);

  /* Now config the temporary file system */
  FsConfig();

  /* Now process CONFIG.SYS     */
  DoConfig(0);
  DoConfig(1);

  /* initialize near data and MCBs */
  PreConfig2();
  /* and process CONFIG.SYS one last time for device drivers */
  DoConfig(2);


  /* Close all (device) files */
  for (i = 0; i < 20; i++)
    close(i);

  /* and do final buffer allocation. */
  PostConfig();

  /* Init the file system one more time     */
  FsConfig();
  
  configDone();

  InitializeAllBPBs();
}

STATIC VOID FsConfig(VOID)
{
  struct dpb FAR *dpb = LoL->DPBp;
  int i;

  /* Initialize the current directory structures    */
  for (i = 0; i < LoL->lastdrive; i++)
  {
    struct cds FAR *pcds_table = &LoL->CDSp[i];

    fmemcpy(pcds_table->cdsCurrentPath, "A:\\\0", 4);

    pcds_table->cdsCurrentPath[0] += i;

    if (i < LoL->nblkdev && (ULONG) dpb != 0xffffffffl)
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

  /* Log-in the default drive. */
  init_setdrive(LoL->BootDrive - 1);

  /* The system file tables need special handling and are "hand   */
  /* built. Included is the stdin, stdout, stdaux and stdprn. */
  /* a little bit of shuffling is necessary for compatibility */

  /* sft_idx=0 is /dev/aux                                        */
  open("AUX", O_RDWR);

  /* handle 1, sft_idx=1 is /dev/con (stdout) */
  open("CON", O_RDWR);

  /* 3 is /dev/aux                */
  dup2(STDIN, STDAUX);

  /* 0 is /dev/con (stdin)        */
  dup2(STDOUT, STDIN);

  /* 2 is /dev/con (stdin)        */
  dup2(STDOUT, STDERR);

  /* 4 is /dev/prn                                                */
  open("PRN", O_WRONLY);

  /* Initialize the disk buffer management functions */
  /* init_call_init_buffers(); done from CONFIG.C   */
}

STATIC VOID signon()
{
  printf("\r%S", MK_FP(FP_SEG(LoL), FP_OFF(LoL->os_release)));

  printf("Kernel compatibility %d.%d - ", MAJOR_RELEASE, MINOR_RELEASE);

  printf(
#if defined(__BORLANDC__)
  "BORLANDC"
#elif defined(__TURBOC__)
  "TURBOC"
#elif defined(_MSC_VER)
  "MSC"
#elif defined(__WATCOMC__)
  "WATCOMC"
#elif defined(__GNUC__)
  "GNUC" /* this is hypothetical only */
#else
#error Unknown compiler
  generate some bullshit error here, as the compiler should be known
#endif
#if defined (I386)
    " - 80386 CPU required"
#elif defined (I186)
    " - 80186 CPU required"
#endif

#ifdef WITHFAT32
  " - FAT32 support"
#endif
  "\n\n%S", (void FAR *)copyright);
}

STATIC void kernel()
{
  exec_blk exb;
  CommandTail Cmd;
  int rc;

  exb.exec.env_seg = DOS_PSP + 8;
  if (master_env[0] == '\0')   /* some shells panic on empty master env. */
    strcpy(master_env, "PATH=.");
  fmemcpy(MK_FP(exb.exec.env_seg, 0), master_env, sizeof(master_env));

  /* process 0       */
  /* Execute command.com /P from the drive we just booted from    */
  memset(Cmd.ctBuffer, 0, sizeof(Cmd.ctBuffer));
  memcpy(Cmd.ctBuffer, Config.cfgInitTail, sizeof(Config.cfgInitTail));

  for (Cmd.ctCount = 0; Cmd.ctCount < sizeof(Cmd.ctBuffer); Cmd.ctCount++)
    if (Cmd.ctBuffer[Cmd.ctCount] == '\r')
      break;

  /* if stepping CONFIG.SYS (F5/F8), tell COMMAND.COM about it */

  if (Cmd.ctCount < sizeof(Cmd.ctBuffer) - 3)
  {
    char *insertString = NULL;

    if (singleStep)
      insertString = " /Y";     /* single step AUTOEXEC */

    if (SkipAllConfig)
      insertString = " /D";     /* disable AUTOEXEC */

    if (insertString)
    {

      /* insert /D, /Y as first argument */
      char *p, *q;

      for (p = Cmd.ctBuffer; p < &Cmd.ctBuffer[Cmd.ctCount]; p++)
      {
        if (*p == ' ' || *p == '\t' || *p == '\r')
        {
          for (q = &Cmd.ctBuffer[Cmd.ctCount - 1]; q >= p; q--)
            q[3] = q[0];

          memcpy(p, insertString, 3);

          Cmd.ctCount += 3;
          break;
        }
      }
    }
  }

  exb.exec.cmd_line = (CommandTail FAR *) & Cmd;
  exb.exec.fcb_1 = exb.exec.fcb_2 = (fcb FAR *) 0xfffffffful;
  
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

  if (LoL->nblkdev == 0)
    dpb = LoL->DPBp;
  else
  {
    for (dpb = LoL->DPBp; (ULONG) dpb->dpb_next != 0xffffffffl;
         dpb = dpb->dpb_next)
      ;
    dpb = dpb->dpb_next =
      KernelAlloc(nunits * sizeof(struct dpb), 'E', Config.cfgDosDataUmb);
  }

  for (Index = 0; Index < nunits; Index++)
  {
    dpb->dpb_next = dpb + 1;
    dpb->dpb_unit = LoL->nblkdev;
    dpb->dpb_subunit = Index;
    dpb->dpb_device = dhp;
    dpb->dpb_flags = M_CHANGED;
    if ((LoL->CDSp != 0) && (LoL->nblkdev < LoL->lastdrive))
    {
      LoL->CDSp[LoL->nblkdev].cdsDpb = dpb;
      LoL->CDSp[LoL->nblkdev].cdsFlags = CDSPHYSDRV;
    }
    ++dpb;
    ++LoL->nblkdev;
  }
  (dpb - 1)->dpb_next = (void FAR *)0xFFFFFFFFl;
}

/* If cmdLine is NULL, this is an internal driver */

BOOL init_device(struct dhdr FAR * dhp, char *cmdLine, COUNT mode,
                 char FAR **r_top)
{
  request rq;
  char name[8];

  if (cmdLine) {
    char *p, *q, ch;
    int i;

    p = q = cmdLine;
    for (;;)
    {
      ch = *p;
      if (ch == '\0' || ch == ' ' || ch == '\t')
        break;
      p++;
      if (ch == '\\' || ch == '/' || ch == ':')
        q = p; /* remember position after path */
    }
    for (i = 0; i < 8; i++) {
      ch = '\0';
      if (p != q && *q != '.')
        ch = *q++;
      /* copy name, without extension */
      name[i] = ch;
    }
  }

  rq.r_unit = 0;
  rq.r_status = 0;
  rq.r_command = C_INIT;
  rq.r_length = sizeof(request);
  rq.r_endaddr = *r_top;
  rq.r_bpbptr = (void FAR *)(cmdLine ? cmdLine : "\n");
  rq.r_firstunit = LoL->nblkdev;

  execrh((request FAR *) & rq, dhp);

/*
 *  Added needed Error handle
 */
  if ((rq.r_status & (S_ERROR | S_DONE)) == S_ERROR)
    return TRUE;

  if (cmdLine)
  {
    /* Don't link in device drivers which do not take up memory */
    if (rq.r_endaddr == (BYTE FAR *) dhp)
      return TRUE;

    /* Fix for multisegmented device drivers:                          */
    /*   If there are multiple device drivers in a single driver file, */
    /*   only the END ADDRESS returned by the last INIT call should be */
    /*   the used.  It is recommended that all the device drivers in   */
    /*   the file return the same address                              */

    if (FP_OFF(dhp->dh_next) == 0xffff)
    {
      KernelAllocPara(FP_SEG(rq.r_endaddr) + (FP_OFF(rq.r_endaddr) + 15)/16
                      - FP_SEG(dhp), 'D', name, mode);
    }

    /* Another fix for multisegmented device drivers:                  */
    /*   To help emulate the functionallity experienced with other DOS */
    /*   operating systems when calling multiple device drivers in a   */
    /*   single driver file, save the end address returned from the    */
    /*   last INIT call which will then be passed as the end address   */
    /*   for the next INIT call.                                       */

    *r_top = (char FAR *)rq.r_endaddr;
  }

  if (!(dhp->dh_attr & ATTR_CHAR) && (rq.r_nunits != 0))
  {
    dhp->dh_name[0] = rq.r_nunits;
    update_dcb(dhp);
  }

  if (dhp->dh_attr & ATTR_CONIN)
    LoL->syscon = dhp;
  else if (dhp->dh_attr & ATTR_CLOCK)
    LoL->clock = dhp;

  return FALSE;
}

STATIC void InitIO(void)
{
  struct dhdr far *device = &LoL->nul_dev;

  /* Initialize driver chain                                      */
  setvec(0x29, int29_handler);  /* Requires Fast Con Driver     */
  do {
    init_device(device, NULL, 0, &lpTop);
    device = device->dh_next;
  }
  while (FP_OFF(device) != 0xffff);
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

STATIC VOID InitSerialPorts(VOID)
{
  iregs r;
  int serial_ports, i;

  init_call_intr(0x11, &r);     /* get equipment list */

  serial_ports = (r.a.x >> 9) & 7;      /* bits 11-9 */

  for (i = 0; i < serial_ports; i++)
  {
    r.a.x = 0xA3;               /* initialize serial port to 2400,n,8,1 */
    r.d.x = i;
    init_call_intr(0x14, &r);
  }
}

/*****************************************************************
	if kernel.config.BootHarddiskSeconds is set,
	the default is to boot from harddisk, because
	the user is assumed to just have forgotten to
	remove the floppy/bootable CD from the drive.
	
	user has some seconds to hit ANY key to continue
	to boot from floppy/cd, else the system is 
	booted from HD
*/

EmulatedDriveStatus(int drive,char statusOnly)
{
  iregs r;
  char buffer[0x13];
  buffer[0] = 0x13;

  r.a.b.h = 0x4b;               /* bootable CDROM - get status */
  r.a.b.l = statusOnly;
  r.d.b.l = (char)drive;          
  r.si  = (int)buffer;
  init_call_intr(0x13, &r);     
  
  if (r.flags & 1)
  	return FALSE;
  
  return TRUE;	
}

STATIC void CheckContinueBootFromHarddisk(void)
{
  char *bootedFrom = "Floppy/CD";
  iregs r;
  int key;

  if (InitKernelConfig.BootHarddiskSeconds == 0)
    return;

  if (LoL->BootDrive >= 3)
  {
#if 0
    if (!EmulatedDriveStatus(0x80,1))
#endif
    {
      /* already booted from HD */
      return;
    }
  }
  else {
#if 0
    if (!EmulatedDriveStatus(0x00,1))
#endif
      bootedFrom = "Floppy";
  }

  printf("\n"
         "\n"
         "\n"
         "     Hit any key within %d seconds to continue booot from %s\n"
         "     Hit 'H' or    wait %d seconds to boot from Harddisk\n",
         InitKernelConfig.BootHarddiskSeconds,
         bootedFrom,
         InitKernelConfig.BootHarddiskSeconds
    );

  key = GetBiosKey(InitKernelConfig.BootHarddiskSeconds);
  
  if (key != -1 && (key & 0xff) != 'h' && (key & 0xff) != 'H')
  {
    /* user has hit a key, continue to boot from floppy/CD */
    printf("\n");
    return;
  }

  /* reboot from harddisk */
  EmulatedDriveStatus(0x00,0);
  EmulatedDriveStatus(0x80,0);

  /* now jump and run */
  r.a.x = 0x0201;
  r.c.x = 0x0001;
  r.d.x = 0x0080;
  r.b.x = 0x7c00;
  r.es  = 0;

  init_call_intr(0x13, &r);

  {
    void (far *reboot)(void) = (void (far*)(void)) MK_FP(0x0,0x7c00);

    (*reboot)();
  }
}
