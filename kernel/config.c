/****************************************************************/
/*                                                              */
/*                          config.c                            */
/*                            DOS-C                             */
/*                                                              */
/*                config.sys Processing Functions               */
/*                                                              */
/*                      Copyright (c) 1996                      */
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
#include "init-dat.h"
#include "dyndata.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id$";
#endif

#ifdef DEBUG
#define DebugPrintf(x) printf x
#else
#define DebugPrintf(x)
#endif

#ifdef KDB
#include <alloc.h>

#define KernelAlloc(x) adjust_far((void far *)malloc((unsigned long)(x)))
#endif

/*
  These are the far variables from the DOS data segment that we need here. The
  init procedure uses a different default DS data segment, which is discarded
  after use. I hope to clean this up to use the DOS List of List and Swappable
  Data Area obtained via INT21.

  -- Bart
 */
extern f_node_ptr DOSFAR f_nodes;       /* pointer to the array                 */
extern UWORD DOSFAR f_nodes_cnt,        /* number of allocated f_nodes          */
  DOSFAR ASM first_mcb;             /* Start of user memory                 */

extern UBYTE DOSFAR ASM lastdrive, DOSFAR ASM nblkdev, DOSFAR ASM mem_access_mode,
    DOSFAR ASM uppermem_link;
extern struct dhdr
DOSTEXTFAR ASM blk_dev,             /* Block device (Disk) driver           */
  DOSFAR ASM nul_dev;
extern struct buffer FAR *DOSFAR ASM firstbuf;      /* head of buffers linked list          */
extern struct buffer FAR *DOSFAR firstAvailableBuf; /* first 'available' buffer   */
extern struct dpb FAR *DOSFAR ASM DPBp;
/* First drive Parameter Block          */
extern struct cds FAR *DOSFAR ASM CDSp;
/* Current Directory Structure          */
extern sfttbl FAR *DOSFAR ASM sfthead;
/* System File Table head               */
extern sfttbl FAR *DOSFAR ASM FCBp;

extern BYTE DOSFAR ASM VgaSet, DOSFAR _HMATextAvailable,    /* first byte of available CODE area    */
  FAR _HMATextStart[],          /* first byte of HMAable CODE area      */
  FAR _HMATextEnd[], DOSFAR ASM break_ena,  /* break enabled flag                   */
  DOSFAR os_major,              /* major version number                 */
  DOSFAR os_minor,              /* minor version number                 */
  DOSFAR ASM switchar, DOSFAR _InitTextStart,       /* first available byte of ram          */
  DOSFAR ReturnAnyDosVersionExpected;

extern UWORD DOSFAR ASM uppermem_root, DOSFAR ASM LoL_nbuffers;

UWORD umb_start = 0, UMB_top = 0;
UWORD ram_top = 0; /* How much ram in Kbytes               */

static UBYTE ErrorAlreadyPrinted[128];


struct config Config = {
  NUMBUFF,
  NFILES,
  NFCBS,
  0,
  "command.com",
  " /P /E:256\r\n",
  NLAST,
  NSTACKS,
  128
      /* COUNTRY= is initialized within DoConfig() */
      , 0                       /* country ID */
      , 0                       /* codepage */
      , ""                      /* filename */
      , 0                       /* amount required memory */
      , 0                       /* pointer to loaded data */
      , 0                       /* strategy for command.com is low by default */
};
                        /* MSC places uninitialized data into COMDEF records,
                           that end up in DATA segment. this can't be tolerated 
                           in INIT code.
                           please make sure, that ALL data in INIT is initialized !! 
                         */
BYTE FAR *lpBase = 0;
BYTE FAR *upBase = 0;
BYTE FAR *lpTop = 0;
BYTE FAR *lpOldTop = 0;
STATIC unsigned nCfgLine = 0;
STATIC COUNT nPass = 0;
COUNT UmbState = 0;
STATIC BYTE szLine[256] = { 0 };
STATIC BYTE szBuf[256] = { 0 };

BYTE singleStep = FALSE;        /* F8 processing */
BYTE SkipAllConfig = FALSE;     /* F5 processing */
BYTE askThisSingleCommand = FALSE;          /* ?device=  device?= */
BYTE DontAskThisSingleCommand = FALSE;      /* !files=            */

COUNT MenuTimeout = -1;
BYTE  MenuSelected = 0;
UCOUNT MenuLine     = 0;
UCOUNT Menus      = 0;

STATIC VOID zumcb_init(UCOUNT seg, UWORD size);
STATIC VOID mumcb_init(UCOUNT seg, UWORD size);

STATIC VOID Config_Buffers(BYTE * pLine);
STATIC VOID sysScreenMode(BYTE * pLine);
STATIC VOID sysVersion(BYTE * pLine);
STATIC VOID CfgBreak(BYTE * pLine);
STATIC VOID Device(BYTE * pLine);
STATIC VOID DeviceHigh(BYTE * pLine);
STATIC VOID Files(BYTE * pLine);
STATIC VOID Fcbs(BYTE * pLine);
STATIC VOID CfgLastdrive(BYTE * pLine);
STATIC BOOL LoadDevice(BYTE * pLine, char FAR *top, COUNT mode);
STATIC VOID Dosmem(BYTE * pLine);
STATIC VOID Country(BYTE * pLine);
STATIC VOID InitPgm(BYTE * pLine);
STATIC VOID InitPgmHigh(BYTE * pLine);
STATIC VOID CmdInstall(BYTE * pLine);
STATIC VOID CmdInstallHigh(BYTE * pLine);


STATIC VOID CfgSwitchar(BYTE * pLine);
STATIC VOID CfgFailure(BYTE * pLine);
STATIC VOID CfgIgnore(BYTE * pLine);
STATIC VOID CfgMenu(BYTE * pLine);
STATIC VOID DoMenu(void);
STATIC VOID CfgMenuDefault(BYTE * pLine);
STATIC BYTE * skipwh(BYTE * s);
STATIC BYTE * scan(BYTE * s, BYTE * d);
STATIC BOOL isnum(BYTE * pszString);
STATIC BYTE * GetNumber(REG BYTE * pszString, REG COUNT * pnNum);
#if 0
STATIC COUNT tolower(COUNT c);
#endif
STATIC COUNT toupper(COUNT c);
STATIC VOID mcb_init(UCOUNT seg, UWORD size);

STATIC VOID Stacks(BYTE * pLine);
STATIC VOID SetAnyDos(BYTE * pLine);
STATIC VOID Numlock(BYTE * pLine);
STATIC BYTE * GetNumArg(BYTE * pLine, COUNT * pnArg);
BYTE *GetStringArg(BYTE * pLine, BYTE * pszString);
STATIC int SkipLine(char *pLine);
#if 0
STATIC char * stristr(char *s1, char *s2);
#endif
STATIC COUNT strcasecmp(REG BYTE * d, REG BYTE * s);
STATIC int LoadCountryInfoHardCoded(char *filename, COUNT ctryCode, COUNT codePage);

void HMAconfig(int finalize);
VOID config_init_buffers(COUNT anzBuffers);     /* from BLOCKIO.C */

#ifdef I86
STATIC VOID FAR * AlignParagraph(VOID FAR * lpPtr);
#else
#define AlignParagraph(x) ((VOID *)x)
#endif

#define EOF 0x1a

STATIC struct table * LookUp(struct table *p, BYTE * token);

typedef void config_sys_func_t(BYTE * pLine);

struct table {
  BYTE *entry;
  BYTE pass;
  config_sys_func_t *func;
};

STATIC struct table commands[] = {
  /* rem is never executed by locking out pass                    */
  {"REM", 0, CfgIgnore},
  {";", 0,   CfgIgnore},

  {"MENUDEFAULT", 0, CfgMenuDefault},   
  {"MENU", 0, CfgMenu},         /* lines to print in pass 0 */
  {"ECHO", 2, CfgMenu},         /* lines to print in pass 2 - when devices are loaded */

  {"BREAK", 1, CfgBreak},
  {"BUFFERS", 1, Config_Buffers},
  {"COMMAND", 1, InitPgm},
  {"COUNTRY", 1, Country},
  {"DOS", 1, Dosmem},
  {"FCBS", 1, Fcbs},
  {"FILES", 1, Files},
  {"LASTDRIVE", 1, CfgLastdrive},
  {"NUMLOCK", 1, Numlock},
  {"SHELL", 1, InitPgm},
  {"SHELLHIGH", 1, InitPgmHigh},
  {"STACKS", 1, Stacks},
  {"SWITCHAR", 1, CfgSwitchar},
  {"SCREEN", 1, sysScreenMode}, /* JPP */
  {"VERSION", 1, sysVersion},   /* JPP */
  {"ANYDOS", 1, SetAnyDos},     /* tom */

  {"DEVICE", 2, Device},
  {"DEVICEHIGH", 2, DeviceHigh},
  {"INSTALL", 2, CmdInstall},
  {"INSTALLHIGH", 2, CmdInstallHigh},
  
  /* default action                                               */
  {"", -1, CfgFailure}
};

BYTE *pLineStart = 0;

BYTE HMAState = 0;
#define HMA_NONE 0              /* do nothing */
#define HMA_REQ 1               /* DOS = HIGH detected */
#define HMA_DONE 2              /* Moved kernel to HMA */
#define HMA_LOW 3               /* Definitely LOW */

STATIC void FAR* ConfigAlloc(COUNT bytes)
{
  VOID FAR *p;

  p = HMAalloc(bytes);

  if (p == NULL)
    p = KernelAlloc(bytes);

  /* printf("ConfigAlloc %d at %p\n", bytes, p); */

  return p;
}

/* Do first time initialization.  Store last so that we can reset it    */
/* later.                                                               */
void PreConfig(void)
{
  VgaSet = 0;
  UmbState = 0;   
  
  memset(ErrorAlreadyPrinted,0,sizeof(ErrorAlreadyPrinted));


  /* Initialize the base memory pointers                          */

#ifdef DEBUG
  {
    extern BYTE FAR ASM internal_data[];
    printf("SDA located at 0x%p\n", internal_data);
  }
#endif
  /* Begin by initializing our system buffers                     */
  /* the dms_scratch buffer is statically allocated
     in the DSK module */
  /* dma_scratch = (BYTE FAR *) KernelAllocDma(BUFFERSIZE); */
/*  DebugPrintf(("Preliminary DMA scratchpad allocated at 0x%p\n",dma_scratch));*/

/*  buffers = (struct buffer FAR *)
      KernelAlloc(Config.cfgBuffers * sizeof(struct buffer)); */
#ifdef DEBUG
/*  printf("Preliminary %d buffers allocated at 0x%p\n", Config.cfgBuffers, buffers);*/
#endif

  /* Initialize the file table                                    */
/*  f_nodes = (f_node_ptr)
      KernelAlloc(Config.cfgFiles * sizeof(struct f_node));*/

  f_nodes = (f_node_ptr)
      DynAlloc("f_nodes", Config.cfgFiles, sizeof(struct f_node));

  f_nodes_cnt = Config.cfgFiles;
  /* sfthead = (sfttbl FAR *)&basesft; */
  /* FCBp = (sfttbl FAR *)&FcbSft; */
  /* FCBp = (sfttbl FAR *)
     KernelAlloc(sizeof(sftheader)
     + Config.cfgFiles * sizeof(sft)); */

  lpBase = AlignParagraph((BYTE FAR *) DynLast() + 0x0f);

  config_init_buffers(Config.cfgBuffers);

  sfthead->sftt_next = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader) + (Config.cfgFiles - 5) * sizeof(sft));
  sfthead->sftt_next->sftt_next = (sfttbl FAR *) - 1;
  sfthead->sftt_next->sftt_count = Config.cfgFiles - 5;

  CDSp = KernelAlloc(sizeof(struct cds) * lastdrive);

  DPBp = KernelAlloc(blk_dev.dh_name[0] * sizeof(struct dpb));

#ifdef DEBUG
  printf("Preliminary:\n f_node 0x%x", f_nodes);
/*  printf(" FCB table 0x%p\n",FCBp);*/
  printf(" sft table 0x%p\n", sfthead->sftt_next);
  printf(" CDS table 0x%p\n", CDSp);
  printf(" DPB table 0x%p\n", DPBp);
#endif

  /* Done.  Now initialize the MCB structure                      */
  /* This next line is 8086 and 80x86 real mode specific          */
#ifdef DEBUG
  printf("Preliminary  allocation completed: top at 0x%p\n", lpBase);
#endif

#ifdef KDB
  lpBase = malloc(4096);
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);
#else
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);
#endif

  /* We expect ram_top as Kbytes, so convert to paragraphs */
  mcb_init(first_mcb, ram_top * 64 - first_mcb - 1);
}

/* Do second pass initialization.                                       */
/* Also, run config.sys to load drivers.                                */
void PostConfig(void)
{
  /* close all (device) files */

  /* compute lastdrive ... */
  lastdrive = Config.cfgLastdrive;
  if (lastdrive < nblkdev)
    lastdrive = nblkdev;

  /* initialize NEAR allocated things */

  /* Initialize the file table                                    */
  DynFree(f_nodes);
  f_nodes = (f_node_ptr)
      DynAlloc("f_nodes", Config.cfgFiles, sizeof(struct f_node));

  f_nodes_cnt = Config.cfgFiles;        /* and the number of allocated files */

  /* Initialize the base memory pointers from last time.          */
  /*
     if the kernel could be moved to HMA, everything behind the dynamic 
     near data is free.
     otherwise, the kernel is moved down - behind the dynamic allocated data,
     and allocation starts after the kernel.
   */

  lpBase = AlignParagraph((BYTE FAR *) DynLast() + 0x0f);

  DebugPrintf(("starting FAR allocations at %p\n", lpBase));

  /* Begin by initializing our system buffers                     */
  /* dma_scratch = (BYTE FAR *) KernelAllocDma(BUFFERSIZE); */
#ifdef DEBUG
  /* printf("DMA scratchpad allocated at 0x%p\n", dma_scratch); */
#endif

  config_init_buffers(Config.cfgBuffers);

/* sfthead = (sfttbl FAR *)&basesft; */
  /* FCBp = (sfttbl FAR *)&FcbSft; */
  /* FCBp = KernelAlloc(sizeof(sftheader)
     + Config.cfgFiles * sizeof(sft)); */
  sfthead->sftt_next =
      KernelAlloc(sizeof(sftheader) + (Config.cfgFiles - 5) * sizeof(sft));
  sfthead->sftt_next->sftt_next = (sfttbl FAR *) - 1;
  sfthead->sftt_next->sftt_count = Config.cfgFiles - 5;

  CDSp = KernelAlloc(sizeof(struct cds) * lastdrive);

  DPBp = KernelAlloc(blk_dev.dh_name[0] * sizeof(struct dpb));

#ifdef DEBUG
  printf("Final: \n f_node 0x%x\n", f_nodes);
/*  printf(" FCB table 0x%p\n",FCBp);*/
  printf(" sft table 0x%p\n", sfthead->sftt_next);
  printf(" CDS table 0x%p\n", CDSp);
  printf(" DPB table 0x%p\n", DPBp);
#endif
  if (Config.cfgStacks)
  {
    VOID FAR *stackBase =
        KernelAlloc(Config.cfgStacks * Config.cfgStackSize);
    init_stacks(stackBase, Config.cfgStacks, Config.cfgStackSize);

    DebugPrintf(("Stacks allocated at %p\n", stackBase));
  }
  DebugPrintf(("Allocation completed: top at 0x%p\n", lpBase));

}

/* This code must be executed after device drivers has been loaded */
VOID configDone(VOID)
{
  if (HMAState != HMA_DONE)
  {
    lpBase = AlignParagraph(lpBase);

    DebugPrintf(("HMA not available, moving text to %x\n",
                 FP_SEG(lpBase)));
    MoveKernel(FP_SEG(lpBase));

    lpBase = AlignParagraph((BYTE FAR *) lpBase + HMAFree + 0x0f);

    DebugPrintf(("kernel is low, start alloc at %p", lpBase));

    /* final buffer processing, now upwards */
    HMAState = HMA_LOW;
    config_init_buffers(Config.cfgBuffers);
  }

  if (lastdrive < nblkdev)
  {

    DebugPrintf(("lastdrive %c too small upping it to: %c\n",
                 lastdrive + 'A', nblkdev + 'A' - 1));

    lastdrive = nblkdev;
    CDSp = KernelAlloc(sizeof(struct cds) * lastdrive);
  }
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);

  /* We expect ram_top as Kbytes, so convert to paragraphs */
  mcb_init(first_mcb, ram_top * 64 - first_mcb - 1);

  if (UmbState == 1)
  {

    UCOUNT umr_new = FP_SEG(upBase) + ((FP_OFF(upBase) + 0x0f) >> 4);
      
    mumcb_init(ram_top * 64 - 1, umb_start - 64 * ram_top);
/* Check if any devices were loaded in umb */
    if (umb_start != FP_SEG(upBase))
    {
/* make last block normal with SC for the devices */

      mumcb_init(umb_start, umr_new - umb_start - 1);

      zumcb_init(umr_new, (umb_start + UMB_top) - umr_new - 1);
      upBase += 16;
    }
    else
      umr_new = FP_SEG(upBase);

    {
      /* are there any more UMB's ?? 
         this happens, if memory mapped devces are in between 
         like UMB memory c800..c8ff, d8ff..efff with device at d000..d7ff
       */

      /*  TE - this code 
         a) isn't the best I ever wrote :-(
         b) works for 2 memory areas (no while(), use of UMB_top,...)
         and the first discovered is the larger one.
         no idea what happens, if the larger one is higher in memory.
         might work, though
       */

      UCOUNT umb_seg, umb_size, umbz_root;

      umbz_root = umr_new;

      if (UMB_get_largest(&umb_seg, &umb_size))
      {

        mcb_init(umbz_root, (umb_start + UMB_top) - umr_new - 1);

        /* change UMB 'Z' to 'M' */
        ((mcb FAR *) MK_FP(umbz_root, 0))->m_type = 'M';

        /* move to end */
        umbz_root += ((mcb FAR *) MK_FP(umbz_root, 0))->m_size + 1;

        /* create link mcb       */
        mumcb_init(umbz_root, umb_seg - umbz_root - 1);

        /* should the UMB driver return
           adjacent memory in several pieces */
        if (umb_seg - umbz_root - 1 == 0)
          ((mcb FAR *) MK_FP(umbz_root, 0))->m_psp = FREE_PSP;

        /* create new 'Z' mcb */
        zumcb_init(umb_seg, umb_size - 1);
      }
    }
  }

  DebugPrintf(("UMB Allocation completed: top at 0x%p\n", upBase));

  /* The standard handles should be reopened here, because
     we may have loaded new console or printer drivers in CONFIG.SYS */

}

VOID DoConfig(int pass)
{
  COUNT nFileDesc;
  BYTE *pLine;
  BOOL bEof;


  /* Set pass number                                              */
  nPass = pass;

  /* Check to see if we have a config.sys file.  If not, just     */
  /* exit since we don't force the user to have one.              */
  if ((nFileDesc = open("fdconfig.sys", 0)) >= 0)
  {
    DebugPrintf(("Reading FDCONFIG.SYS...\n"));
  }
  else
  {
    DebugPrintf(("FDCONFIG.SYS not found\n"));
    if ((nFileDesc = open("config.sys", 0)) < 0)
    {
      DebugPrintf(("CONFIG.SYS not found\n"));
      return;
    }
    DebugPrintf(("Reading CONFIG.SYS...\n"));
  }

  /* Have one -- initialize.                                      */
  nCfgLine = 0;
  bEof = 0;
  pLine = szLine;

  /* Read each line into the buffer and then parse the line,      */
  /* do the table lookup and execute the handler for that         */
  /* function.                                                    */

  for (; !bEof; nCfgLine++)
  {
    struct table *pEntry;

    pLineStart = szLine;

    /* read in a single line, \n or ^Z terminated */

    for (pLine = szLine;;)
    {
      if (read(nFileDesc, pLine, 1) <= 0)
      {
        bEof = TRUE;
        break;
      }

      /* immediately convert to upper case */
      *pLine = toupper(*pLine);

      if (pLine >= szLine + sizeof(szLine) - 3)
      {
        CfgFailure(pLine);
        printf("error - line overflow line %d \n", nCfgLine);
        break;
      }

      if (*pLine == '\n' || *pLine == EOF)      /* end of line */
        break;

      if (*pLine == '\r')       /* ignore */
        ;
      else
        pLine++;
    }

    *pLine = 0;
    pLine = szLine;

    /* Skip leading white space and get verb.               */
    pLine = scan(pLine, szBuf);

    /* If the line was blank, skip it.  Otherwise, look up  */
    /* the verb and execute the appropriate function.       */
    if (*szBuf == '\0')
      continue;

    pEntry = LookUp(commands, szBuf);

    if (pEntry->pass >= 0 && pEntry->pass != nPass)
      continue;
    
    if (nPass == 0) /* pass 0 always executed (rem Menu prompt) */
    {
      (*(pEntry->func)) (pLine);
      continue;
    }
    else
    {        
      if (SkipLine(pLineStart))   /* F5/F8 processing */
        continue;
    }

    if (pEntry->func != CfgMenu)
      pLine = skipwh(pLine);

    if ('=' != *pLine && pEntry->func != CfgMenu)
      CfgFailure(pLine);
    else                        /* YES. DO IT */
      (*(pEntry->func)) (skipwh(pLine + 1));

    /* might have been the UMB driver */
    if (UmbState == 2)
    {

      UCOUNT umb_seg, umb_size;

      if (UMB_get_largest(&umb_seg, &umb_size))
      {
        UmbState = 1;
        upBase = MK_FP(umb_seg, 0);
        UMB_top = umb_size;
        umb_start = umb_seg;

/* reset root */
        uppermem_root = ram_top * 64 - 1;
/* setup the real mcb for the devicehigh block */
        zumcb_init(umb_seg, UMB_top - 1);
        upBase += 16;
      }
    }

  }
  close(nFileDesc); 

  if (nPass == 0)
  {
    DoMenu();
  }
}

STATIC struct table * LookUp(struct table *p, BYTE * token)
{
  while (*(p->entry) != '\0')
  {
    if (strcasecmp(p->entry, token) == 0)
      break;
    else
      ++p;
  }
  return p;
}

/*
    get BIOS key with timeout:
    
    timeout < 0: no timeout
    timeout = 0: poll only once
    timeout > 0: timeout in seconds
    
    return
            0xffff : no key hit
            
            0xHH.. : scancode in upper  half
            0x..LL : asciicode in lower half
*/

STATIC ULONG GetBiosTime(VOID)
{
  return *(ULONG FAR *) (MK_FP(0x40, 0x6c));
}

STATIC UWORD GetBiosKey(int timeout)
{
  iregs r;

  ULONG startTime = GetBiosTime();

  for (;;)
  {
    r.a.x = 0x0100;             /* are there keys available ? */
    init_call_intr(0x16, &r);

    if ((r.flags & 0x40) == 0)  /* yes - fetch and return     */
    {
      r.a.x = 0x0000;
      init_call_intr(0x16, &r);

      return r.a.x;
    }

    if (timeout < 0)
      continue;

    if (GetBiosTime() - startTime >= (unsigned)timeout * 18)
      break;
  }
  return 0xffff;
}

STATIC BOOL SkipLine(char *pLine)
{
  short key;

  if (InitKernelConfig.SkipConfigSeconds >= 0)
  {

    if (InitKernelConfig.SkipConfigSeconds > 0)
      printf("Press F8 to trace or F5 to skip CONFIG.SYS/AUTOEXEC.BAT");

    key = GetBiosKey(InitKernelConfig.SkipConfigSeconds);       /* wait 2 seconds */
    
    InitKernelConfig.SkipConfigSeconds = -1;

    if (key == 0x3f00)          /* F5 */
    {
      SkipAllConfig = TRUE;
    }
    if (key == 0x4200)          /* F8 */
    {
      singleStep = TRUE;
    }

    printf("\r%79s\r", "");     /* clear line */

    if (SkipAllConfig)
      printf("Skipping CONFIG.SYS/AUTOEXEC.BAT\n");
  }

  if (SkipAllConfig)
    return TRUE;

  /* 1?device=CDROM.SYS */
  /* 12?device=OAKROM.SYS */
  /* 123?device=EMM386.EXE NOEMS */
  if ( MenuLine != 0 && 
      (MenuLine & (1 << MenuSelected)) == 0)
    return TRUE;

  if (DontAskThisSingleCommand)     /* !files=30 */
    return FALSE;

  if (!askThisSingleCommand && !singleStep)
    return FALSE;

  printf("%s[Y,N]?", pLine);

  for (;;)
  {
    key = GetBiosKey(-1);

    switch (toupper(key & 0x00ff))
    {
      case 'N':
      case 'n':
        printf("N\n");
        return TRUE;

      case 0x1b:               /* don't know where documented
                                   ESCAPE answers all following questions
                                   with YES
                                 */
        singleStep = FALSE;     /* and fall through */

      case '\r':
      case '\n':
      case 'Y':
      case 'y':
        printf("Y\n");
        return FALSE;

    }

    if (key == 0x3f00)          /* YES, you may hit F5 here, too */
    {
      printf("N\n");
      SkipAllConfig = TRUE;
      return TRUE;
    }
  }

}

STATIC BYTE * GetNumArg(BYTE * pLine, COUNT * pnArg)
{
  /* look for NUMBER                               */
  pLine = skipwh(pLine);
  if (!isnum(pLine) && *pLine != '-')
  {
    CfgFailure(pLine);
    return (BYTE *) 0;
  }
  return GetNumber(pLine, pnArg);
}

BYTE *GetStringArg(BYTE * pLine, BYTE * pszString)
{
  /* look for STRING                               */
  pLine = skipwh(pLine);

  /* just return whatever string is there, including null         */
  return scan(pLine, pszString);
}

STATIC void Config_Buffers(BYTE * pLine)
{
  COUNT nBuffers;

  /* Get the argument                                             */
  if (GetNumArg(pLine, &nBuffers) == (BYTE *) 0)
    return;

  /* Got the value, assign either default or new value            */
  Config.cfgBuffers =
      (nBuffers < 0 ? nBuffers : max(Config.cfgBuffers, nBuffers));
}

STATIC VOID sysScreenMode(BYTE * pLine)
{
  COUNT nMode;

  /* Get the argument                                             */
  if (GetNumArg(pLine, &nMode) == (BYTE *) 0)
    return;

  if ((nMode != 0x11) && (nMode != 0x12) && (nMode != 0x14))
    return;

/* Modes
   0x11 (17)   28 lines
   0x12 (18)   43/50 lines
   0x14 (20)   25 lines
 */
#if defined(__TURBOC__)
  _AX = (0x11 << 8) + nMode;
  _BL = 0;
  __int__(0x10);
#elif defined(I86)
  asm
  {
    mov al, byte ptr nMode;
    mov ah, 0x11;
    mov bl, 0;
    int 0x10;
  }
#endif
}

STATIC VOID sysVersion(BYTE * pLine)
{
  COUNT major, minor;
  char *p;

  p = pLine;
  while (*p && *p != '.')
    p++;

  if (*p++ == '\0')
    return;

  /* Get major number */
  if (GetNumArg(pLine, &major) == (BYTE *) 0)
    return;

  /* Get minor number */
  if (GetNumArg(p, &minor) == (BYTE *) 0)
    return;

  printf("Changing reported version to %d.%d\n", major, minor);

  os_major = major;
  os_minor = minor;
}

STATIC VOID Files(BYTE * pLine)
{
  COUNT nFiles;

  /* Get the argument                                             */
  if (GetNumArg(pLine, &nFiles) == (BYTE *) 0)
    return;

  /* Got the value, assign either default or new value            */
  Config.cfgFiles = max(Config.cfgFiles, nFiles);
}

STATIC VOID CfgLastdrive(BYTE * pLine)
{
  /* Format:   LASTDRIVE = letter         */
  BYTE drv;

  pLine = skipwh(pLine);
  drv = *pLine & ~0x20;

  if (drv < 'A' || drv > 'Z')
  {
    CfgFailure(pLine);
    return;
  }
  drv -= 'A';
  drv++;                        /* Make real number */
  Config.cfgLastdrive = max(Config.cfgLastdrive, drv);
}

/*
    UmbState of confidence, 1 is sure, 2 maybe, 4 unknown and 0 no way.
*/

STATIC VOID Dosmem(BYTE * pLine)
{
  BYTE *pTmp;
  BYTE UMBwanted = FALSE;

/*    extern BYTE FAR INITDataSegmentClaimed; */

  pLine = GetStringArg(pLine, szBuf);

  for (pTmp = szBuf; *pTmp != '\0'; pTmp++)
    *pTmp = toupper(*pTmp);

  /* printf("DOS called with %s\n", szBuf); */

  for (pTmp = szBuf;;)
  {
    if (fmemcmp(pTmp, "UMB", 3) == 0)
    {
      UMBwanted = TRUE;
      pTmp += 3;
    }
    if (fmemcmp(pTmp, "HIGH", 4) == 0)
    {
      HMAState = HMA_REQ;
      pTmp += 4;
    }
/*        if (fmemcmp(pTmp, "CLAIMINIT",9) == 0) { INITDataSegmentClaimed = 0; pTmp += 9; }*/
    pTmp = skipwh(pTmp);

    if (*pTmp != ',')
      break;
    pTmp++;
  }

  if (UmbState == 0)
  {
    uppermem_link = 0;
    uppermem_root = 0xffff;
    UmbState = UMBwanted ? 2 : 0;
  }
  /* Check if HMA is available straight away */
  if (HMAState == HMA_REQ && MoveKernelToHMA())
  {
    HMAState = HMA_DONE;
  }
}

STATIC VOID CfgSwitchar(BYTE * pLine)
{
  /* Format: SWITCHAR = character         */

  GetStringArg(pLine, szBuf);
  switchar = *szBuf;
}

STATIC VOID Fcbs(BYTE * pLine)
{
  /*  Format:     FCBS = totalFcbs [,protectedFcbs]    */
  COUNT fcbs;

  if ((pLine = GetNumArg(pLine, &fcbs)) == 0)
    return;
  Config.cfgFcbs = fcbs;

  pLine = skipwh(pLine);

  if (*pLine == ',')
  {
    GetNumArg(++pLine, &fcbs);
    Config.cfgProtFcbs = fcbs;
  }

  if (Config.cfgProtFcbs > Config.cfgFcbs)
    Config.cfgProtFcbs = Config.cfgFcbs;
}

/*      LoadCountryInfo():
 *      Searches a file in the COUNTRY.SYS format for an entry
 *      matching the specified code page and country code, and loads
 *      the corresponding information into memory. If code page is 0,
 *      the default code page for the country will be used.
 *
 *      Returns TRUE if successful, FALSE if not.
 */

#if 0
STATIC BOOL LoadCountryInfo(char *filename, UWORD ctryCode, UWORD codePage)
{
  printf("Sorry, the COUNTRY= statement has been temporarily disabled\n");

  UNREFERENCED_PARAMETER(codePage);
  UNREFERENCED_PARAMETER(ctryCode);
  UNREFERENCED_PARAMETER(filename);

  return FALSE;
} 
#endif

STATIC VOID Country(BYTE * pLine)
{
  /* Format: COUNTRY = countryCode, [codePage], filename   */
  COUNT ctryCode;
  COUNT codePage = NLS_DEFAULT;
  char  *filename = "";

  if ((pLine = GetNumArg(pLine, &ctryCode)) == 0)
    goto error;


    /*  currently 'implemented' 
                 COUNTRY=49     */

#if 0
  pLine = skipwh(pLine);
  if (*pLine == ',')
  {
    pLine = skipwh(pLine + 1);

    if (*pLine != ',')
      if ((pLine = GetNumArg(pLine, &codePage)) == 0)
        goto error;

    pLine = skipwh(pLine);
    if (*pLine == ',')
    {
      GetStringArg(++pLine, szBuf);
      filename = szBuf;
    }
  }  
#endif
      
  if (!LoadCountryInfoHardCoded(filename, ctryCode, codePage))
    return;
  
error:  
  CfgFailure(pLine);
}

STATIC VOID Stacks(BYTE * pLine)
{
  COUNT stacks;

  /* Format:  STACKS = stacks [, stackSize]       */
  pLine = GetNumArg(pLine, &stacks);
  Config.cfgStacks = stacks;

  pLine = skipwh(pLine);

  if (*pLine == ',')
  {
    GetNumArg(++pLine, &stacks);
    Config.cfgStackSize = stacks;
  }

  if (Config.cfgStacks)
  {
    if (Config.cfgStackSize < 32)
      Config.cfgStackSize = 32;
    if (Config.cfgStackSize > 512)
      Config.cfgStackSize = 512;
    if (Config.cfgStacks > 64)
      Config.cfgStacks = 64;
  }
}

STATIC VOID InitPgmHigh(BYTE * pLine)
{
  InitPgm(pLine);
  Config.cfgP_0_startmode = 0x80;
}

STATIC VOID InitPgm(BYTE * pLine)
{
  /* Get the string argument that represents the new init pgm     */
  pLine = GetStringArg(pLine, Config.cfgInit);

  /* Now take whatever tail is left and add it on as a single     */
  /* string.                                                      */
  strcpy(Config.cfgInitTail, pLine);

  /* and add a DOS new line just to be safe                       */
  strcat(Config.cfgInitTail, "\r\n");

  Config.cfgP_0_startmode = 0;
}

STATIC VOID CfgBreak(BYTE * pLine)
{
  /* Format:      BREAK = (ON | OFF)      */
  GetStringArg(pLine, szBuf);
  break_ena = strcasecmp(szBuf, "OFF") ? 1 : 0;
}

STATIC VOID Numlock(BYTE * pLine)
{
  extern VOID ASMCFUNC keycheck(void);

  /* Format:      NUMLOCK = (ON | OFF)      */
  BYTE FAR *keyflags = (BYTE FAR *) MK_FP(0x40, 0x17);

  GetStringArg(pLine, szBuf);

  *keyflags &= ~32;
  *keyflags |= strcasecmp(szBuf, "OFF") ? 32 : 0;
  keycheck();
}

STATIC VOID DeviceHigh(BYTE * pLine)
{
  if (UmbState == 1)
  {
    if (LoadDevice(pLine, MK_FP(umb_start + UMB_top, 0), TRUE) == DE_NOMEM)
    {
      printf("Not enough free memory in UMB's: loading low\n");
      LoadDevice(pLine, lpTop, FALSE);
    }
  }
  else
  {
    printf("UMB's unavailable!\n");
    LoadDevice(pLine, lpTop, FALSE);
  }
}

STATIC void Device(BYTE * pLine)
{
  LoadDevice(pLine, lpTop, FALSE);
}

STATIC BOOL LoadDevice(BYTE * pLine, char FAR *top, COUNT mode)
{
  exec_blk eb;
  struct dhdr FAR *dhp;
  struct dhdr FAR *next_dhp;
  BOOL result;

  if (mode)
    dhp = AlignParagraph(upBase);
  else
    dhp = AlignParagraph(lpBase);

  /* Get the device driver name                                   */
  GetStringArg(pLine, szBuf);

  /* The driver is loaded at the top of allocated memory.         */
  /* The device driver is paragraph aligned.                      */
  eb.load.reloc = eb.load.load_seg = FP_SEG(dhp);

#ifdef DEBUG
  printf("Loading device driver %s at segment %04x\n", szBuf, FP_SEG(dhp));
#endif

  if ((result = init_DosExec(3, &eb, szBuf)) != SUCCESS)
  {
    CfgFailure(pLine);
    return result;
  }

  strcpy(szBuf, pLine);

  /* TE this fixes the loading of devices drivers with
     multiple devices in it. NUMEGA's SoftIce is such a beast
   */

  /* add \r\n to the command line */
  strcat(szBuf, "\r\n");

  for (next_dhp = NULL; FP_OFF(next_dhp) != 0xffff &&
       (result = init_device(dhp, szBuf, mode, top)) == SUCCESS;
       dhp = next_dhp)
  {
    next_dhp = dhp->dh_next;
    if (FP_SEG(next_dhp) == 0xffff)
      /*  Does this ever occur with FP_OFF(next_dhp) != 0xffff ??? */
      next_dhp = MK_FP(FP_SEG(dhp), FP_OFF(next_dhp));
#ifdef DEBUG
    else if (FP_OFF(next_dhp) != 0xffff)        /* end of internal chain */
      printf("multisegmented device driver found, next %p\n", next_dhp);
    /* give warning message */
#endif
    /* Link in device driver and save nul_dev pointer to next */
    dhp->dh_next = nul_dev.dh_next;
    nul_dev.dh_next = dhp;
  }
  /* We could just have loaded FDXMS or HIMEM */
  if (HMAState == HMA_REQ && MoveKernelToHMA())
  {
    /* final HMA processing: */
    /* final buffer processing, now upwards */
    HMAState = HMA_DONE;
    config_init_buffers(Config.cfgBuffers);
  }

  return result;
}

STATIC VOID CfgFailure(BYTE * pLine)
{
  BYTE *pTmp = pLineStart;

  /* suppress multiple printing of same unrecognized lines */

  if (nCfgLine < sizeof(ErrorAlreadyPrinted)*8)
  {
    if (ErrorAlreadyPrinted[nCfgLine/8] & (1 << (nCfgLine%8)))
      return;
        
    ErrorAlreadyPrinted[nCfgLine/8] |= (1 << (nCfgLine%8));
  }
  printf("CONFIG.SYS error in line %d\n", nCfgLine);
  printf(">>>%s\n   ", pTmp);
  while (++pTmp != pLine)
    printf(" ");
  printf("^\n");
}

#ifndef KDB
void FAR * KernelAlloc(size_t nBytes)
{
  BYTE FAR *lpAllocated;

  lpBase = AlignParagraph(lpBase);
  lpAllocated = lpBase;

  if (0xffff - FP_OFF(lpBase) <= nBytes)
  {
    UWORD newOffs = (FP_OFF(lpBase) + nBytes) & 0xFFFF;
    UWORD newSeg = FP_SEG(lpBase) + 0x1000;

    lpBase = MK_FP(newSeg, newOffs);
  }
  else
    lpBase += nBytes;

  fmemset(lpAllocated, 0, nBytes);

  return lpAllocated;
}
#endif

#ifdef I86
#if 0
STATIC BYTE FAR * KernelAllocDma(WORD bytes)
{
  BYTE FAR *allocated;

  lpBase = AlignParagraph(lpBase);
  if ((FP_SEG(lpBase) & 0x0fff) + (bytes >> 4) > 0x1000)
    lpBase = MK_FP((FP_SEG(lpBase) + 0x0fff) & 0xf000, 0);
  allocated = lpBase;
  lpBase += bytes;
  return allocated;
}
#endif

STATIC void FAR * AlignParagraph(VOID FAR * lpPtr)
{
  UWORD uSegVal;

  /* First, convert the segmented pointer to linear address       */
  uSegVal = FP_SEG(lpPtr);
  uSegVal += (FP_OFF(lpPtr) + 0xf) >> 4;
  if (FP_OFF(lpPtr) > 0xfff0)
    uSegVal += 0x1000;          /* handle overflow */

  /* and return an adddress adjusted to the nearest paragraph     */
  /* boundary.                                                    */
  return MK_FP(uSegVal, 0);
}
#endif

STATIC BYTE * skipwh(BYTE * s)
{
  while (*s && (*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t'))
    ++s;
  return s;
}

STATIC BYTE * scan(BYTE * s, BYTE * d)
{
  askThisSingleCommand = FALSE;
  DontAskThisSingleCommand = FALSE;

  s = skipwh(s);

  MenuLine = 0;

  /* does the line start with "123?" */

  if (isnum(s))
  {
    unsigned numbers = 0;
    for ( ; isnum(s); s++)
        numbers |= 1 << (*s -'0');
    
    if (*s == '?')
    {
      MenuLine = numbers;
      Menus |= numbers;    
      s = skipwh(s+1);
    }
  }

  
  /* !dos=high,umb    ?? */
  if (*s == '!')
  {
    DontAskThisSingleCommand = TRUE;
    s = skipwh(s+1);
  }

  if (*s == ';')
  {
    /* semicolon is a synonym for rem */
    *d++ = *s++;
  }
  else
    while (*s &&
           !(*s == 0x0d
             || *s == 0x0a || *s == ' ' || *s == '\t' || *s == '='))
    {
      if (*s == '?')
      {
        askThisSingleCommand = TRUE;
        s++;
      }
      else
        *d++ = *s++;
    }
  *d = '\0';
  return s;
}

#if 0
BYTE *scan_seperator(BYTE * s, BYTE * d)
{
  s = skipwh(s);
  if (*s)
    *d++ = *s++;
  *d = '\0';
  return s;
}
#endif

STATIC BOOL isnum(BYTE * pLine)
{
  return (*pLine >= '0' && *pLine <= '9');
}

/* JPP - changed so will accept hex number. */
STATIC BYTE * GetNumber(REG BYTE * pszString, REG COUNT * pnNum)
{
  BYTE Base = 10;
  BOOL Sign = FALSE;

  *pnNum = 0;
  if (*pszString == '-')
  {
    pszString++;
    Sign = TRUE;
  }

  while (isnum(pszString) || toupper(*pszString) == 'X')
  {
    if (toupper(*pszString) == 'X')
    {
      Base = 16;
      pszString++;
    }
    else
      *pnNum = *pnNum * Base + (*pszString++ - '0');
  }
  if (Sign)
    *pnNum = -*pnNum;
  return pszString;
}

/* Yet another change for true portability (WDL)                        */
#if 0
STATIC COUNT tolower(COUNT c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));
  else
    return c;
}
#endif

/* Yet another change for true portability (PJV) */
STATIC COUNT toupper(COUNT c)
{
  if (c >= 'a' && c <= 'z')
    return (c - ('a' - 'A'));
  else
    return c;
}

/* The following code is 8086 dependant                         */

#if 1                           /* ifdef KERNEL */
STATIC VOID mcb_init(UCOUNT seg, UWORD size)
{
  COUNT i;

  mcb FAR *mcbp = MK_FP(seg, 0);

  mcbp->m_type = MCB_LAST;
  mcbp->m_psp = FREE_PSP;

  mcbp->m_size = (UmbState > 0 ? size - 1 : size);

  for (i = 0; i < 8; i++)
    mcbp->m_name[i] = '\0';
  mem_access_mode = FIRST_FIT;
}

STATIC VOID zumcb_init(UCOUNT seg, UWORD size)
{
  COUNT i;
  mcb FAR *mcbp = MK_FP(seg, 0);

  mcbp->m_type = MCB_LAST;
  mcbp->m_psp = FREE_PSP;
  mcbp->m_size = size;
  for (i = 0; i < 8; i++)
    mcbp->m_name[i] = '\0';

}

STATIC VOID mumcb_init(UCOUNT seg, UWORD size)
{
  COUNT i;
  mcb FAR *mcbp = MK_FP(seg, 0);

  static char name[8] = "SC\0\0\0\0\0\0";

  mcbp->m_type = MCB_NORMAL;
  mcbp->m_psp = 8;
  mcbp->m_size = size;
  for (i = 0; i < 8; i++)
    mcbp->m_name[i] = name[i];
}
#endif

char *strcat(register char * d, register const char * s)
{
  char *tmp = d;
  while (*d != 0)
    ++d;
  strcpy(d, s);
  return tmp;
}

#if 0
/* see if the second string is contained in the first one, ignoring case */
STATIC char * stristr(char *s1, char *s2)
{
  int loop;

  for (; *s1; s1++)
    for (loop = 0;; loop++)
    {
      if (s2[loop] == 0)        /* found end of string 2 -> success */
      {
        return s1;              /* position where s2 was found */
      }
      if (toupper(s1[loop]) != toupper(s2[loop]))
        break;
    }

  return NULL;
}
#endif

/* compare two ASCII strings ignoring case */
STATIC COUNT strcasecmp(REG BYTE * d, REG BYTE * s)
{
  while (*s != '\0' && *d != '\0')
  {

    if (toupper(*d) == toupper(*s))
      ++s, ++d;

    else
      return toupper(*d) - toupper(*s);

  }

  return toupper(*d) - toupper(*s);
}

/*
    moved from BLOCKIO.C here.
    that saves some relocation problems    
*/

VOID config_init_buffers(COUNT anzBuffers)
{
  REG WORD i;
  struct buffer FAR *pbuffer;
  BYTE FAR *tmplpBase = lpBase;
  unsigned wantedbuffers = anzBuffers;

  if (anzBuffers < 0)
  {
    wantedbuffers = anzBuffers = -anzBuffers;
  }
  else if (HMAState == HMA_DONE)
  {
    anzBuffers = (0xfff0 - HMAFree) / sizeof(struct buffer);
  }

  anzBuffers = max(anzBuffers, 6);
  if (anzBuffers > 99)
  {
    printf("BUFFERS=%u not supported, reducing to 99\n", anzBuffers);
    anzBuffers = 99;
  }
  LoL_nbuffers = anzBuffers;
  
  lpTop = lpOldTop;

  if (HMAState == HMA_NONE || HMAState == HMA_REQ)
    lpTop = lpBase = lpTop - (anzBuffers * sizeof(struct buffer) + 0xf);
  
  firstbuf = ConfigAlloc(sizeof(struct buffer) * anzBuffers);
  pbuffer = firstbuf;

  DebugPrintf(("init_buffers (size %u) at", sizeof(struct buffer)));
  DebugPrintf((" (%p)", firstbuf));

  pbuffer->b_prev = FP_OFF(pbuffer + (anzBuffers-1));
  for (i = 1; i < anzBuffers; ++i)
  {
    if (i == wantedbuffers)
    {
      firstAvailableBuf = pbuffer;
    }      
    pbuffer->b_next = FP_OFF(pbuffer + 1);
    pbuffer++;
    pbuffer->b_prev = FP_OFF(pbuffer - 1);
  }
  pbuffer->b_next = FP_OFF(pbuffer - (anzBuffers-1));

    /* now, we can have quite some buffers in HMA
       -- up to 50 for KE38616.
       so we fill the HMA with buffers
       but not if the BUFFERS count is negative ;-)
     */

  DebugPrintf((" done\n"));

  if (FP_SEG(pbuffer) == 0xffff)
    printf("Kernel: allocated %d Diskbuffers = %u Bytes in HMA\n",
           anzBuffers, anzBuffers * sizeof(struct buffer));

  if (HMAState == HMA_NONE || HMAState == HMA_REQ)
    lpBase = tmplpBase;
}

/*
    Undocumented feature: 
    
    ANYDOS 
        will report to MSDOS programs just the version number
        they expect. be careful with it!
*/

STATIC VOID SetAnyDos(BYTE * pLine)
{
  UNREFERENCED_PARAMETER(pLine);
  ReturnAnyDosVersionExpected = TRUE;
}

STATIC VOID CfgIgnore(BYTE * pLine)
{
  UNREFERENCED_PARAMETER(pLine);
}

/*
   'MENU'ing stuff
   
   although it's worse then MSDOS's , its better then nothing 
   
*/

STATIC VOID CfgMenu(BYTE * pLine)
{
  printf("%s\n",pLine);
}               

STATIC VOID DoMenu(void)
{   
  if (Menus == 0)
    return;      
    
  InitKernelConfig.SkipConfigSeconds = -1;  

  Menus |= 1 << 0;          /* '0' Menu always allowed */

  printf("\n\n");

  for (;;)
  {
    int key,i;

RestartInput:    
    printf("\rSinglestepping (F8) is :%s - ", singleStep ? "ON " : "OFF");
    
    printf("please select a Menu[");
    
    for (i = 0; i <= 9; i++)
      if (Menus & (1 << i))
        printf("%c", '0' + i);
    printf("]");            
    
    printf(" (default=%d)", MenuSelected);
    
    if (MenuTimeout >= 0)
      printf(" - %d \b", MenuTimeout);
    else	
      printf("     \b\b\b\b\b");

    key = GetBiosKey(MenuTimeout >= 0 ? 1 : -1);

    if (key == -1)              /* timeout, take default */
    {
      if (MenuTimeout > 0)
      {
        MenuTimeout--;
        goto RestartInput;
      }
      break;            
    }        
    else 
      MenuTimeout = -1;

    if (key == 0x3f00)          /* F5 */
    {
      SkipAllConfig = TRUE;
      break;
    }
    if (key == 0x4200)          /* F8 */
    {
      singleStep = !singleStep;
    }
    
    key &= 0xff;
    
    if (key == '\r')            /* CR - use default */
    {
      break;
    }
    if (key == 0x1b)            /* ESC - use default */
    {
      break;
    }
    
    printf("%c", key);
    
    if (key >= '0' && key <= '9')
      if (Menus & (1 << (key - '0')))
      {
        MenuSelected = key - '0'; break;
      }
  }
  printf("\n");
}

STATIC VOID CfgMenuDefault(BYTE * pLine)
{
  COUNT num = 0;

  pLine = skipwh(pLine);
  
  if ('=' != *pLine)
  {
    CfgFailure(pLine);
    return;
  }
  pLine = skipwh(pLine + 1);
  
  /* Format:  STACKS = stacks [, stackSize]       */
  pLine = GetNumArg(pLine, &num);
  MenuSelected = num;
  pLine = skipwh(pLine);

  if (*pLine == ',')
  {
    GetNumArg(++pLine, &MenuTimeout);
  }
}





/*********************************************************************************
    National specific things.
    this handles only Date/Time/Currency, and NOT codepage things.
    Some may consider this a hack, but I like to see 24 Hour support. tom.
*********************************************************************************/


struct CountrySpecificInfo {
  short CountryID;    /*  = W1 W437   # Country ID & Codepage */
  short CodePage;
  short DateFormat;           /*    Date format: 0/1/2: U.S.A./Europe/Japan */
  char  CurrencyString[5];    /* '$' ,'EUR'   */
  char  ThousandSeparator[2]; /* ','          # Thousand's separator */
  char  DecimalPoint[2];      /* '.'        # Decimal point        */
  char  DateSeparator[2];     /* '-'  */
  char  TimeSeparator[2];     /* ':'  */
  char  CurrencyFormat;       /* = 0  # Currency format (bit array) 
                                 0Fh    BYTE    currency format
                                 bit 2 = set if currency symbol replaces decimal point
                                 bit 1 = number of spaces between value and currency symbol
                                 bit 0 = 0 if currency symbol precedes value
                                 1 if currency symbol follows value    
                              */
  char  CurrencyPrecision;    /* = 2  # Currency precision           */
  char  TimeFormat;           /* = 0  # time format: 0/1: 12/24 houres */
};


#define _DATE_MDY 0 /* mm/dd/yy */
#define _DATE_DMY 1  /* dd.mm.yy */
#define _DATE_YMD 2  /* yy/mm/dd */

#define _TIME_12 0
#define _TIME_24 1

struct CountrySpecificInfo specificCountriesSupported[] = {
    
  /* US */ {
    1,                          /*  = W1 W437   # Country ID & Codepage */
    437,      
    _DATE_MDY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$",                        /* '$' ,'EUR'   */
    ",",                        /* ','          # Thousand's separator */
    ".",                        /* '.'        # Decimal point        */
    "/",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_12                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Canadian French */ ,{
    2,                          /*  = W1 W437   # Country ID & Codepage */
    863,      
    _DATE_YMD,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$",                        /* '$' ,'EUR'   */
    ",",                        /* ','          # Thousand's separator */
    ".",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Latin America */ ,{
    3,                          /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_MDY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$",                        /* '$' ,'EUR'   */
    ",",                        /* ','          # Thousand's separator */
    ".",                        /* '.'        # Decimal point        */
    "/",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_12                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Russia - by arkady */ ,{
    7,                          /*  = W1 W437   # Country ID & Codepage */
    866,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "RUB",                      /* '$' ,'EUR'   */
                                /* should be "\xE0", but as no codepage
                                   support exists (yet), better to leave it as 'Rubels'
                                */       						
    " ",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    3,                          /*  Currency format : currency follows, after blank */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* DUTCH */ ,{
    31,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Belgium */ ,{
    32,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* France */ ,{
    33,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Spain */ ,{
    33,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    "'",                        /*      Decimal point - by aitor       */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Hungary */ ,{
    36,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$HU",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Yugoslavia */ ,{
    38,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$YU",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Italy */ ,{
    39,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Switzerland */ ,{
    41,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "SF",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Czechoslovakia */ ,{
    42,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_YMD,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$YU",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* UK */ ,{
    44,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "\x9c",                      /* Pound sign    */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "/",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Denmark */ ,{
    45,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "DKK",                      /*     */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  /* Sweden */ ,{
    46,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_YMD,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "SEK",                      /*     */
    ",",                        /* ','          # Thousand's separator */
    ".",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Norway */ ,{
    47,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "NOK",                      /*     */
    ",",                        /* ','          # Thousand's separator */
    ".",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Poland */ ,{
    48,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_YMD,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "PLN",                      /*  michael tyc: PLN means PoLish New zloty, I think)   */
    ",",                        /* ','          # Thousand's separator */
    ".",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* GERMAN */ ,{
    49,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    1,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* Argentina */ ,{
    54,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$ar",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "/",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    1,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_12                    /* = 0  # time format: 0/1: 12/24 houres */
  }

  /* Brazil */ ,{
    55,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$ar",                      /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "/",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    1,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }
  
  /* International English */ ,{
    61,                         /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_MDY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "$",                        /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "/",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }

  /* Japan - Yuki Mitsui */ ,{
    81,                       /*  = W1 W437   # Country ID & Codepage */
    932,      
    _DATE_YMD,                  /*    Date format: 0/1/2:U.S.A./Europe/Japan */
    "\x81\x8f",                 /* '$' ,'EUR'   */
    ",",                        /* ','        # Thousand's separator */
    ".",                        /* '.'        # Decimal point        */
    "/",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision          */
    _TIME_12                    /* = 0  # time format: 0/1: 12/24 houres 
                                 */
  }

  /* Portugal */ ,{
    351,                        /*  = W1 W437   # Country ID & Codepage */
    850,      
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                        /* '$' ,'EUR'   */
    ".",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    "-",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0,                          /* = 0  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 houres */
  }

  /* Finland - by wolf */ ,{
    358,                        /*  = W1 W437   # Country ID & Codepage */
    850,
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "EUR",                      /* '$' ,'EUR'   */
    " ",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    0x3,                        /*  # Currency format (bit array) */
    2,                          /* = 2  # Currency precision           */
    _TIME_24                    /* = 0  # time format: 0/1: 12/24 hours */
  }

  /* Ukraine - by Oleg Deribas */ ,{
    380,         /*  = W380 W848   # Country ID & Codepage  */
    848,
    _DATE_DMY,   /* Date format: 0/1/2: U.S.A./Europe/Japan */
    "UAH",       /* Currency */
    " ",         /* ' '  # Thousand's separator */
    ",",         /* ','  # Decimal point        */
    ".",         /* '.'  DateSeparator */
    ":",         /* ':'  TimeSeparator */
    3,           /* = 3  # Currency format (bit array)    */
    2,           /* = 2  # Currency precision             */
    _TIME_24     /* = 1  # time format: 0/1: 12/24 houres */
  }

};    


/* contributors to above table:

	tom ehlert (GER)
	bart oldeman (NL)
	wolf (FIN)
	Michael H.Tyc (POL)
	Oleg Deribas (UKR)
	Arkady Belousov (RUS)
	Yuki Mitsui (JAP)
	Aitor Santamar­a Merino (SP)
*/	



extern struct {
  char  ThisIsAConstantOne;
  short TableSize;
  
  struct CountrySpecificInfo C;
  
} FAR nlsCountryInfoHardcoded;



STATIC int LoadCountryInfoHardCoded(char *filename, COUNT ctryCode, COUNT codePage)
{
  int i;
  UNREFERENCED_PARAMETER(codePage);
  UNREFERENCED_PARAMETER(filename);

  /* printf("cntry: %u, CP%u, file=\"%s\"\n", ctryCode, codePage, filename);  */



  for (i = 0; i < sizeof(specificCountriesSupported)/sizeof(specificCountriesSupported[0]); i++)
  {
    if (specificCountriesSupported[i].CountryID == ctryCode)
    {
      int codepagesaved = nlsCountryInfoHardcoded.C.CodePage;

      fmemcpy(&nlsCountryInfoHardcoded.C.CountryID, 
              &specificCountriesSupported[i],
              min(nlsCountryInfoHardcoded.TableSize, sizeof(struct CountrySpecificInfo)));

      nlsCountryInfoHardcoded.C.CodePage = codepagesaved;

      return 0;
    }
  }
  
  printf("could not find country info for country ID %u\n", ctryCode);
  printf("current supported countries are ");

  for (i = 0; i < sizeof(specificCountriesSupported)/sizeof(specificCountriesSupported[0]); i++)
  {
    printf("%u ",specificCountriesSupported[i].CountryID);
  }
  printf("\n");       

  return 1;
}


/* ****************************************************************
** implementation of INSTALL=NANSI.COM /P /X /BLA 
*/

int  numInstallCmds = 0;
struct {
  char buffer[128];
  int mode;
} InstallCommands[10];

#define InstallPrintf(x)
/*#define InstallPrintf(x) printf x*/
                

STATIC VOID _CmdInstall(BYTE * pLine,int mode)
{
  InstallPrintf(("Installcmd %d:%s\n",numInstallCmds,pLine));
  
  if (numInstallCmds > LENGTH(InstallCommands))
  {
    printf("Too many Install commands given (%d max)\n",LENGTH(InstallCommands));
    CfgFailure(pLine);
    return;
  }   
  fmemcpy(InstallCommands[numInstallCmds].buffer,pLine,127);
  InstallCommands[numInstallCmds].buffer[127] = 0;
  InstallCommands[numInstallCmds].mode        = mode;
  numInstallCmds++;
}       
STATIC VOID CmdInstall(BYTE * pLine)
{
  _CmdInstall(pLine,0);
}
STATIC VOID CmdInstallHigh(BYTE * pLine)
{
  _CmdInstall(pLine,0x80);	/* load high, if possible */
}

void InstallExec(char *cmd)
{                            
  BYTE filename[128],*args,*d;
  exec_blk exb;

  InstallPrintf(("installing %s\n",cmd));

  cmd=skipwh(cmd);     

  for (args = cmd, d = filename; ;args++,d++)
  {
    *d = *args;
    if (*d <= 0x020 || *d == '/')
      break;
  }
  *d = 0;

  args--;
  *args = 0;
  while (args[*args+1])
    ++*args;        
  args[*args+1] = '\r';	
  args[*args+2] = 0;

  exb.exec.env_seg  = 0;
  exb.exec.cmd_line = (CommandTail FAR *) args;
  exb.exec.fcb_1 = exb.exec.fcb_2 = (fcb FAR *) 0xfffffffful;


  InstallPrintf(("cmd[%s] args [%u,%s]\n",filename,*args,args+1));

  if (init_DosExec(0, &exb, filename) != SUCCESS)
  {
    CfgFailure(cmd);
  }
}		


VOID DoInstall(void)
{
  int i;
  iregs r;
  extern BYTE ASM _init_end[];
  unsigned short installMemory; 


  if (numInstallCmds == 0)
    return;
  
  InstallPrintf(("Installing commands now\n"));

  /* grab memory for this install code 
     we KNOW, that we are executing somewhere at top of memory
     we need to protect the INIT_CODE from other programs
     that will be executing soon
  */
  
  r.a.x = 0x5801;             /* set memory allocation strategy */
  r.b.b.l = 0x02;			    /*low memory, last fit			*/
  init_call_intr(0x21, &r);

  r.a.b.h = 0x48;				/* alloc memory	*/
  r.b.x = ((unsigned)_init_end+15)/16;
  init_call_intr(0x21, &r);
  installMemory = r.a.x;


  InstallPrintf(("allocated memory at %x\n",installMemory));

  for (i = 0; i < numInstallCmds; i++)
  {
    InstallPrintf(("%d:%s\n",i,InstallCommands[i].buffer));
    
    r.a.x = 0x5801;             /* set memory allocation strategy */
    r.b.b.l = InstallCommands[i].mode;
    init_call_intr(0x21, &r);
    
    InstallExec(InstallCommands[i].buffer);
  }      

  r.a.x = 0x5801;             /* set memory allocation strategy */
  r.b.b.l = 0x00;			    /*low memory, high			*/
  init_call_intr(0x21, &r);
    
  r.a.b.h = 0x49;				/* alloc memory	*/
  r.es  = installMemory;
  init_call_intr(0x21, &r);
  
  InstallPrintf(("Done with installing commands\n"));
  return;
}
