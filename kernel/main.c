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

/* The Holy Copyright Message. Do NOT remove it or you'll be cursed forever! */

static char copyright[] =
"Copyright 1995-2004 Pasquale J. Villani and The FreeDOS Project.\n"
"This free software has ABSOLUTELY NO WARRANTY and is licensed under\n"
"the GNU General Public License (http://www.gnu.org/licenses/gpl.html)\n\n";

struct _KernelConfig InitKernelConfig BSS_INIT({0});

STATIC VOID init_internal_devices(void);

STATIC VOID update_dcb(struct dhdr FAR *);
STATIC VOID init_kernel(VOID);
STATIC VOID signon(VOID);
STATIC VOID init_shell(VOID);
STATIC VOID FsConfig(VOID);
STATIC VOID InitPrinters(VOID);
STATIC VOID InitSerialPorts(VOID);
STATIC void CheckContinueBootFromHarddisk(void);
STATIC void setup_int_vectors(void);

#ifdef _MSC_VER
BYTE _acrtused = 0;

__segment DosDataSeg = 0;       /* serves for all references to the DOS DATA segment 
                                   necessary for MSC+our funny linking model
                                 */
__segment DosTextSeg = 0;

#endif

struct lol FAR * const LoL = &DATASTART;

void ASMCFUNC FreeDOSmain(void)
{
#ifdef _MSC_VER
  extern FAR prn_dev;
  DosDataSeg = (__segment) & DATASTART;
  DosTextSeg = (__segment) & prn_dev;
#endif

  /* clear the Init BSS area (what normally the RTL does */
  memset(_ib_start, 0, _ib_end - _ib_start);

                        /*  if the kernel has been UPX'ed,
                                CONFIG info is stored at 50:e2 ..fc
                            and the bootdrive (passed from BIOS)
                            at 50:e0
                        */

  {
    UBYTE drv;
    UBYTE FAR *p = MK_PTR(UBYTE, 0, 0x5e2);
    if (fmemcmp(p, "CONFIG", 6) == 0) /* UPXed */
      drv = p[-2];		/* stored there by stub from exeflat.c */

      /* !!! stub, added by exeflat.c for UPXed kernel, should store
         boot drive# in the CONFIG-block, not outside (below) it. --avb */

    else
    {
      drv = LoL->BootDrive;

      /* !!! kernel.asm should store boot drive# from BL into
         LowKernelConfig instead LoL->BootDrive. --avb		*/

      p[-2] = drv;		/* used in initdisk.c		*/

      /* !!! initdisk.c:ReadAllPartitionTables() should get boot drive#
         from InitKernelConfig, not at fixed address 0:5e0. --avb */

      p = (UBYTE FAR*)&LowKernelConfig;
    }

    /* !!! boot drive# should be get from InitKernelConfig after
       copying there FAR memory (from 0:5e0 or LowKernelConfig). --avb */

    drv++;			/* A:=1, B:=2			*/
    if (drv > 0x80)
      drv = 3;			/* C:				*/
    LoL->BootDrive = drv;

    fmemcpy(&InitKernelConfig, p, sizeof InitKernelConfig);
  }

  setup_int_vectors();

  CheckContinueBootFromHarddisk();

  signon();
  init_kernel();
  init_shell();

  init_call_p_0(&Config); /* execute process 0 (the shell) */
}

STATIC void PSPInit(void)
{
  psp _seg *p = MK_SEG_PTR(psp, DOS_PSP);

  fmemset(p, 0, sizeof(psp));	/* Clear out new psp first	*/

  /* initialize all entries and exits				*/
  p->ps_exit = 0x20cd;		/* CP/M-like exit point:	*/
				/* INT 20 opcode		*/
				/* CP/M-like entry point:	*/
  p->ps_farcall = 0x9a;		/* FAR CALL opcode...		*/
  p->ps_reentry = MK_FP(0, 0x30 * 4); /* ...entry address	*/
  p->ps_unix[0] = 0xcd;		/* unix style call:		*/
  p->ps_unix[1] = 0x21;		/* INT 21/RETF opcodes		*/
  p->ps_unix[2] = 0xcb;

  /* parent-child relationships					*/
  /*p->ps_parent = 0;*/		/* parent psp segment		*/
  p->ps_prevpsp = (VFP)-1l;	/* previous psp address		*/

  /* Environment and memory useage parameters			*/
  /*p->ps_size = 0;*/		/* segment of memory beyond	*/
				/* memory allocated to program	*/
  /*p->ps_environ = 0;*/	/* environment paragraph	*/

  /*p->ps_isv22 = NULL;*/	/* terminate handler		*/
  /*p->ps_isv23 = NULL;*/	/* break handler		*/
  /*p->ps_isv24 = NULL;*/	/* critical error handler	*/

  /*p->ps_stack = NULL;*/	/* user stack pointer - int 21	*/

  /* File System parameters					*/
  p->ps_maxfiles = sizeof p->ps_files; /* size of file table	*/
  fmemset(p->ps_filetab = p->ps_files, 0xff, sizeof p->ps_files);

  /*p->ps_fcb1.fcb_drive = 0;*/ /* 1st command line argument	*/
  /*fmemset(p->ps_fcb1.fcb_fname, ' ', FNAME_SIZE + FEXT_SIZE);*/
  /*p->ps_fcb2.fcb_drive = 0;*/ /* 2nd command line argument	*/
  /*fmemset(p->ps_fcb2.fcb_fname, ' ', FNAME_SIZE + FEXT_SIZE);*/

  /* this area reused for master environment			*/
  /*p->ps_cmd.ctCount = 0;*/	/* local command line		*/
  /*p->ps_cmd.ctBuffer[0] = '\r';*/ /* command tail		*/
}

#ifndef __WATCOMC__
/* for WATCOMC we can use the ones in task.c */
intvec getvec(unsigned char intno)
{
  intvec iv;
  disable();
  iv = *(intvec FAR *)MK_FP(0,4 * (intno));
  enable();
  return iv;
}

void setvec(unsigned char intno, intvec vector)
{
  disable();
  *(intvec FAR *)MK_FP(0,4 * intno) = vector;
  enable();
}
#endif

STATIC void setup_int_vectors(void)
{
  static struct vec
  {
    unsigned char intno;
    size_t handleroff;
  } vectors[] =
    {
      /* all of these are in the DOS DS */
      { 0x0, FP_OFF(int0_handler) },   /* zero divide */
      { 0x1, FP_OFF(empty_handler) },  /* single step */
      { 0x3, FP_OFF(empty_handler) },  /* debug breakpoint */
      { 0x6, FP_OFF(int6_handler) },   /* invalid opcode */
      { 0x20, FP_OFF(int20_handler) },
      { 0x21, FP_OFF(int21_handler) },
      { 0x22, FP_OFF(int22_handler) },
      { 0x24, FP_OFF(int24_handler) },
      { 0x25, FP_OFF(low_int25_handler) },
      { 0x26, FP_OFF(low_int26_handler) },
      { 0x27, FP_OFF(int27_handler) },
      { 0x28, FP_OFF(int28_handler) },
      { 0x2a, FP_OFF(int2a_handler) },
      { 0x2f, FP_OFF(int2f_handler) }
    };
  struct vec *pvec;
  int i;

  for (i = 0x23; i <= 0x3f; i++)
    setvec(i, empty_handler);
  for (pvec = vectors; pvec < ENDOF(vectors); pvec++)
    setvec(pvec->intno, (intvec)MK_FP(FP_SEG(empty_handler), pvec->handleroff));
  pokeb(0, 0x30 * 4, 0xea);
  pokel(0, 0x30 * 4 + 1, (ULONG)cpm_entry);

  /* these two are in the device driver area LOWTEXT (0x70) */
  setvec(0x1b, got_cbreak);
  setvec(0x29, int29_handler);  /* required for printf! */
}

STATIC void init_kernel(void)
{
  COUNT i;

  LoL->os_setver_major = LoL->os_major = MAJOR_RELEASE;
  LoL->os_setver_minor = LoL->os_minor = MINOR_RELEASE;
  LoL->rev_number = REVISION_SEQ;

  /* move kernel to high conventional RAM, just below the init code */
#ifdef __WATCOMC__
  lpTop = MK_FP(_CS, 0);
#else
  lpTop = MK_FP(_CS - (FP_OFF(_HMATextEnd) + 15) / 16, 0);
#endif

  MoveKernel(FP_SEG(lpTop));
  /* lpTop should be para-aligned				*/
  lpTop = MK_FP(FP_SEG(lpTop) - 0xfff, 0xfff0);

  /* Initialize IO subsystem                                      */
  init_internal_devices();
  InitPrinters();
  InitSerialPorts();

  init_PSPSet(DOS_PSP);
  set_DTA(MK_FP(DOS_PSP, 0x80));
  PSPInit();

  Init_clk_driver();

  /* Do first initialization of system variable buffers so that   */
  /* we can read config.sys later.  */
  LoL->lastdrive = Config.cfgLastdrive;

  blk_dev.dh_name[0] = dsk_init();

  PreConfig();

  /* Number of units */
  if (blk_dev.dh_name[0] > 0)
    update_dcb(&blk_dev);

  /* Now config the temporary file system */
  FsConfig();

  /* Now process CONFIG.SYS     */
  DoConfig();

  /* Close all (device) files */
  for (i = 0; i < 20; i++)
    close(i);

  /* and do final buffer allocation. */
  PostConfig();

  /* Init the file system one more time     */
  FsConfig();

  configDone();

  DoInstall();
}

STATIC VOID FsConfig(VOID)
{
  struct dpb FAR *dpb = LoL->DPBp;
  int i;

  /* Initialize the current directory structures    */
  for (i = 0; i < LoL->lastdrive; i++)
  {
    struct cds FAR *pcds_table = &LoL->CDSp[i];
    pcds_table->cdsCurrentPath[0] = 'A' + i;
    pcds_table->cdsCurrentPath[1] = ':';
    pcds_table->cdsCurrentPath[2] = '\\';
    pcds_table->cdsCurrentPath[3] = '\0';
    pcds_table->cdsFlags = 0;
    if (i < LoL->nblkdev && (LONG) dpb != -1l)
    {
      pcds_table->cdsDpb = dpb;
      pcds_table->cdsFlags = CDSPHYSDRV;
      dpb = dpb->dpb_next;
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
  printf("\r%S"
         "Kernel compatibility %d.%d - "
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
  "\n\n%s",
         MK_FP(FP_SEG(LoL), FP_OFF(LoL->os_release)),
         MAJOR_RELEASE, MINOR_RELEASE, copyright);
}

STATIC void init_shell()
{
  /* if stepping CONFIG.SYS (F5/F8), tell COMMAND.COM about it	*/
  /* (insert /D, /Y as first argument)				*/
  if (askCommand & (ASK_TRACE | ASK_SKIPALL))
  {
    PStr p = Config.cfgShell - 1; /* find end of command name	*/

    /* too long line -> truncate it to make space for "/Y \0"	*/
    Config.cfgShell[sizeof Config.cfgShell - 4] = '\0';

    do p++; while ((UBYTE)*p > ' ' && *p != '/');

    if (*p == ' ' || *p == '\t')
      p++;			/* place option after space	*/

    {
      PStr q = p;
      while (*q++);		/* find end of command line	*/
      /* shift tail to right by 3 to make room for option	*/
      do
      {
        q--;
        q[3] = q[0];
      } while (q > p);
    }

    p[0] = '/', p[1] = 'Y', p[2] = ' ';	/* single step AUTOEXEC	*/
    if (askCommand & ASK_SKIPALL)
      p[1] = 'D';			/* disable AUTOEXEC	*/
  }
}

/* check for a block device and update device control block	*/
STATIC VOID update_dcb(struct dhdr FAR * dhp)
{
  int nunits = dhp->dh_name[0];
  struct dpb FAR *dpb = LoL->DPBp;

  if (LoL->nblkdev)
  {
    while ((LONG) dpb->dpb_next != -1l)
      dpb = dpb->dpb_next;
    dpb = dpb->dpb_next =
      KernelAlloc(nunits * sizeof(struct dpb), 'E', Config.cfgDosDataUmb);
  }

  {
    int i = 0;
    do
    {
      dpb->dpb_next = dpb + 1;
      dpb->dpb_unit = LoL->nblkdev;
      if (LoL->nblkdev < LoL->lastdrive && LoL->CDSp)
      {
        LoL->CDSp[LoL->nblkdev].cdsDpb = dpb;
        LoL->CDSp[LoL->nblkdev].cdsFlags = CDSPHYSDRV;
      }
      LoL->nblkdev++;
      dpb->dpb_subunit = i;
      dpb->dpb_device = dhp;
      dpb->dpb_flags = M_CHANGED;
      dpb++;
      i++;
    } while (i < nunits);
  }
  (dpb - 1)->dpb_next = (VFP)-1l;
}

/* If r_top is NULL, this is an internal driver */
BOOL init_device(struct dhdr FAR * dhp, PCStr cmdLine, int mode, VFP *r_top)
{
  request rq;

  rq.r_unit = 0;
  rq.r_status = 0;
  rq.r_command = C_INIT;
  rq.r_length = sizeof(request);
  rq.r_endaddr = r_top ? *r_top : lpTop;
  rq.r_bpbptr = (VFP)cmdLine;
  rq.r_firstunit = LoL->nblkdev;

  execrh(&rq, dhp);

/*
 *  Added needed Error handle
 */
  if ((rq.r_status & (S_ERROR | S_DONE)) == S_ERROR)
    return TRUE;

  if (r_top)
  {
    /* Don't link in device drivers which do not take up memory */
    /* ie device drivers that fail to load and return top==CS:0 */
    if (rq.r_endaddr == (BYTE FAR *) dhp)
      return TRUE;

    /* Don't link in block device drivers which indicate no units */
    if (!(dhp->dh_attr & ATTR_CHAR) && !rq.r_nunits)
    {
      rq.r_endaddr = (BYTE FAR *) dhp;
      return TRUE;
    }


    /* Fix for multisegmented device drivers:                          */
    /*   If there are multiple device drivers in a single driver file, */
    /*   only the END ADDRESS returned by the last INIT call should be */
    /*   the used.  It is recommended that all the device drivers in   */
    /*   the file return the same address                              */

    if (FP_OFF(dhp->dh_next) == 0xffff)
    {
      char name[8];
      PCStr q;
      {
        UBYTE ch;
        PCStr p = cmdLine;
        q = p;			/* position after path		*/
        do			/* find driver name after path	*/
        {
          ch = *p;
          p++;
          if (ch == ':' || ch == '\\' || ch == '/')
            q = p;
        } while (ch > ' ');
      }
      {
        int i = 0;
        do			/* extract driver name		*/
        {
          UBYTE ch = *q;
          if (ch <= ' ' || ch == '.')
            ch = '\0';		/* copy name, without extension	*/
          name[i] = ch;
          if (ch == '\0')
            break;
          i++, q++;
        } while (i < sizeof name);
      }
      KernelAllocPara(FP_SEG(alignNextPara(rq.r_endaddr)) - FP_SEG(dhp),
                      'D', name, mode);
    }

    /* Another fix for multisegmented device drivers:                  */
    /*   To help emulate the functionallity experienced with other DOS */
    /*   operating systems when calling multiple device drivers in a   */
    /*   single driver file, save the end address returned from the    */
    /*   last INIT call which will then be passed as the end address   */
    /*   for the next INIT call.                                       */

    *r_top = rq.r_endaddr;
  }

  /* if block device (not character) and unit count is nonzero */
  if (!(dhp->dh_attr & ATTR_CHAR) && rq.r_nunits)
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

STATIC void init_internal_devices(void)
{
  struct dhdr far *device = &LoL->nul_dev;

  /* Initialize driver chain                                      */
  do {
    /* ??? is cmdLine "\n" for internal devices required? --avb	*/
    init_device(device, "\n", 0, NULL);
    device = device->dh_next;
  }
  while (FP_OFF(device) != 0xffff);
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

STATIC int EmulatedDriveStatus(int drive,char statusOnly)
{
  iregs r;
  char buffer[0x13];
  buffer[0] = 0x13;

  r.AH = 0x4b;			/* bootable CDROM - get status	*/
  r.AL = statusOnly;
  r.DL = (char)drive;
  r.SI = (int)buffer;
  init_call_intr(0x13, &r);
  return r.FLAGS & 1;		/* carry flag */
}

STATIC void CheckContinueBootFromHarddisk(void)
{
  if (InitKernelConfig.BootHarddiskSeconds <= 0         /* feature disabled */
   || LoL->BootDrive >= 3 && EmulatedDriveStatus(0x80,1)) /* booted from HD */
    return;

  printf("\n\nTo boot from hard disk, press 'H' or wait %d seconds\n"
             "To boot from floppy/CD, press any other key NOW!\n",
         InitKernelConfig.BootHarddiskSeconds);

  if (GetBiosKey(InitKernelConfig.BootHarddiskSeconds))
  {
    unsigned key = GetBiosKey(-1); /* remove key from buffer	*/
    if ((UBYTE)key != 'h' && (UBYTE)key != 'H')
      /* user has hit a key, continue to boot from floppy/CD	*/
      return;
  }

  /* reboot from harddisk */
  EmulatedDriveStatus(0x00,0);
  EmulatedDriveStatus(0x80,0);

  {
    iregs r;
    r.AX = 0x0201;
    r.CX = 0x0001;
    r.DX = 0x0080;
    r.BX = 0x7c00;
    r.ES = 0;
    init_call_intr(0x13, &r);
  }

  /* now jump and run */
  ((void (far*)(void)) MK_FP(0,0x7c00))(); /* jump to boot sector */
}
