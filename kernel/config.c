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
/* write to the Free Software Foundation, Inc.,                 */
/* 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.     */
/****************************************************************/

#include "portab.h"
#include "init-mod.h"
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
#define para2far(seg) ((mcb FAR *)MK_FP((seg), 0))

UWORD umb_start = 0, UMB_top = 0;
UWORD ram_top = 0; /* How much ram in Kbytes               */

static UBYTE ErrorAlreadyPrinted[128];


struct config Config = {
  0,
  NUMBUFF,
  NFILES,
  0,
  NFCBS,
  0,
  "command.com",
  " /P /E:256\r\n",
  NLAST,
  0,
  NSTACKS,
  0,
  128
      /* COUNTRY= is initialized within DoConfig() */
      , 0                       /* country ID */
      , 0                       /* codepage */
      , ""                      /* filename */
      , 0                       /* amount required memory */
      , 0                       /* pointer to loaded data */
      , 0                       /* strategy for command.com is low by default */
      , 0xFFFF                  /* default value for switches=/E:nnnn */
};
                        /* MSC places uninitialized data into COMDEF records,
                           that end up in DATA segment. this can't be tolerated
                           in INIT code.
                           please make sure, that ALL data in INIT is initialized !!
                         */
STATIC seg base_seg = 0;
STATIC seg umb_base_seg = 0;
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

STATIC VOID Config_Buffers(BYTE * pLine);
STATIC VOID sysScreenMode(BYTE * pLine);
STATIC VOID sysVersion(BYTE * pLine);
STATIC VOID CfgBreak(BYTE * pLine);
STATIC VOID Device(BYTE * pLine);
STATIC VOID DeviceHigh(BYTE * pLine);
STATIC VOID Files(BYTE * pLine);
STATIC VOID FilesHigh(BYTE * pLine);
STATIC VOID Fcbs(BYTE * pLine);
STATIC VOID CfgLastdrive(BYTE * pLine);
STATIC VOID CfgLastdriveHigh(BYTE * pLine);
STATIC BOOL LoadDevice(BYTE * pLine, char FAR *top, COUNT mode);
STATIC VOID Dosmem(BYTE * pLine);
STATIC VOID DosData(BYTE * pLine);
STATIC VOID Country(BYTE * pLine);
STATIC VOID InitPgm(BYTE * pLine);
STATIC VOID InitPgmHigh(BYTE * pLine);
STATIC VOID CmdInstall(BYTE * pLine);
STATIC VOID CmdInstallHigh(BYTE * pLine);


STATIC VOID CfgSwitchar(BYTE * pLine);
STATIC VOID CfgSwitches(BYTE * pLine);
STATIC VOID CfgFailure(BYTE * pLine);
STATIC VOID CfgIgnore(BYTE * pLine);
STATIC VOID CfgMenu(BYTE * pLine);

STATIC VOID CfgMenuEsc(BYTE * pLine);

STATIC VOID DoMenu(void);
STATIC VOID CfgMenuDefault(BYTE * pLine);
STATIC BYTE * skipwh(BYTE * s);
STATIC BYTE * scan(BYTE * s, BYTE * d);
STATIC BOOL isnum(char ch);
STATIC char * GetNumber(REG const char *p, int *num);
#if 0
STATIC COUNT tolower(COUNT c);
#endif
STATIC COUNT toupper(COUNT c);
STATIC VOID mcb_init(UCOUNT seg, UWORD size, BYTE type);
STATIC VOID mumcb_init(UCOUNT seg, UWORD size);

STATIC VOID Stacks(BYTE * pLine);
STATIC VOID StacksHigh(BYTE * pLine);

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
STATIC void umb_init(void);

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
  /* first = switches! this one is special since it is asked for but
     also checked before F5/F8 */
  {"SWITCHES", 0, CfgSwitches},
  
  /* rem is never executed by locking out pass                    */
  {"REM", 0, CfgIgnore},
  {";", 0,   CfgIgnore},

  {"MENUDEFAULT", 0, CfgMenuDefault},   
  {"MENU", 0, CfgMenu},         /* lines to print in pass 0 */
  {"ECHO", 2, CfgMenu},         /* lines to print in pass 2 - install(high) */
  {"EECHO", 2, CfgMenuEsc},     /* modified ECHO (ea) */

  {"BREAK", 1, CfgBreak},
  {"BUFFERS", 1, Config_Buffers},
  {"COMMAND", 1, InitPgm},
  {"COUNTRY", 1, Country},
  {"DOS", 1, Dosmem},
  {"DOSDATA", 1, DosData},
  {"FCBS", 1, Fcbs},
  {"FILES", 1, Files},
  {"FILESHIGH", 1, FilesHigh},
  {"LASTDRIVE", 1, CfgLastdrive},
  {"LASTDRIVEHIGH", 1, CfgLastdriveHigh},
  {"NUMLOCK", 1, Numlock},
  {"SHELL", 1, InitPgm},
  {"SHELLHIGH", 1, InitPgmHigh},
  {"STACKS", 1, Stacks},
  {"STACKSHIGH", 1, StacksHigh},
  {"SWITCHAR", 1, CfgSwitchar},
  {"SCREEN", 1, sysScreenMode},   /* JPP */
  {"VERSION", 1, sysVersion},     /* JPP */
  {"ANYDOS", 1, SetAnyDos},       /* tom */

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

/* Do first time initialization.  Store last so that we can reset it    */
/* later.                                                               */
void PreConfig(void)
{
  memset(ErrorAlreadyPrinted,0,sizeof(ErrorAlreadyPrinted));

  /* Initialize the base memory pointers                          */

#ifdef DEBUG
  {
    printf("SDA located at 0x%p\n", internal_data);
  }
#endif
  /* Begin by initializing our system buffers                     */
#ifdef DEBUG
/*  printf("Preliminary %d buffers allocated at 0x%p\n", Config.cfgBuffers, buffers);*/
#endif

  LoL->DPBp =
      DynAlloc("DPBp", blk_dev.dh_name[0], sizeof(struct dpb));

  /* Initialize the file table                                    */
/*  f_nodes = (f_node_ptr)
      KernelAlloc(Config.cfgFiles * sizeof(struct f_node));*/

  LoL->f_nodes = (f_node_ptr)
      DynAlloc("f_nodes", Config.cfgFiles, sizeof(struct f_node));

  LoL->f_nodes_cnt = Config.cfgFiles;
  LoL->sfthead = MK_FP(FP_SEG(LoL), 0xcc); /* &(LoL->firstsftt) */
  /* LoL->FCBp = (sfttbl FAR *)&FcbSft; */
  /* LoL->FCBp = (sfttbl FAR *)
     KernelAlloc(sizeof(sftheader)
     + Config.cfgFiles * sizeof(sft)); */

  config_init_buffers(Config.cfgBuffers);


  LoL->CDSp = KernelAlloc(sizeof(struct cds) * LoL->lastdrive, 'L', 0);

#ifdef DEBUG
  printf("Preliminary:\n f_node 0x%x", LoL->f_nodes);
/*  printf(" FCB table 0x%p\n",LoL->FCBp);*/
  printf(" sft table 0x%p\n", LoL->sfthead);
  printf(" CDS table 0x%p\n", LoL->CDSp);
  printf(" DPB table 0x%p\n", LoL->DPBp);
#endif

  /* Done.  Now initialize the MCB structure                      */
  /* This next line is 8086 and 80x86 real mode specific          */
#ifdef DEBUG
  printf("Preliminary  allocation completed: top at %p\n", lpTop);
#endif
}

/* Do second pass initialization: near allocation and MCBs              */
void PreConfig2(void)
{
  struct sfttbl FAR *sp;
  unsigned ebda_size;

  /* initialize NEAR allocated things */

  /* Initialize the file table                                    */
  DynFree(LoL->f_nodes);
  LoL->f_nodes = (f_node_ptr)
      DynAlloc("f_nodes", Config.cfgFiles, sizeof(struct f_node));

  LoL->f_nodes_cnt = Config.cfgFiles;        /* and the number of allocated files */

  /* Initialize the base memory pointers from last time.          */
  /*
     if the kernel could be moved to HMA, everything behind the dynamic
     near data is free.
     otherwise, the kernel is moved down - behind the dynamic allocated data,
     and allocation starts after the kernel.
   */

  base_seg = LoL->first_mcb = FP_SEG(AlignParagraph((BYTE FAR *) DynLast() + 0x0f));

  ebda_size = 0;
  if (Config.ebda2move)
  {
    ebda_size = ebdasize();
    ram_top += ebda_size / 1024;
    if (ebda_size > Config.ebda2move)
      ebda_size = Config.ebda2move;
  }

  /* We expect ram_top as Kbytes, so convert to paragraphs */
  mcb_init(base_seg, ram_top * 64 - LoL->first_mcb - 1, MCB_LAST);

  sp = LoL->sfthead;
  sp = sp->sftt_next = KernelAlloc(sizeof(sftheader) + 3 * sizeof(sft), 'F', 0);
  sp->sftt_next = (sfttbl FAR *) - 1;
  sp->sftt_count = 3;

  if (ebda_size)  /* move the Extended BIOS Data Area from top of RAM here */
    movebda(ebda_size, FP_SEG(KernelAlloc(ebda_size, 'I', 0)));

  if (UmbState == 2)
    umb_init();
}

/* Do third pass initialization.                                        */
/* Also, run config.sys to load drivers.                                */
void PostConfig(void)
{
  sfttbl FAR *sp;

  /* We could just have loaded FDXMS or HIMEM */
  if (HMAState == HMA_REQ && MoveKernelToHMA())
    HMAState = HMA_DONE;
  
  if (Config.cfgDosDataUmb)
  {
    Config.cfgFilesHigh = TRUE;
    Config.cfgLastdriveHigh = TRUE;
    Config.cfgStacksHigh = TRUE;
  }
        
  /* compute lastdrive ... */
  LoL->lastdrive = Config.cfgLastdrive;
  if (LoL->lastdrive < LoL->nblkdev)
    LoL->lastdrive = LoL->nblkdev;

  DebugPrintf(("starting FAR allocations at %x\n", base_seg));

  /* Begin by initializing our system buffers                     */
  /* dma_scratch = (BYTE FAR *) KernelAllocDma(BUFFERSIZE); */
#ifdef DEBUG
  /* printf("DMA scratchpad allocated at 0x%p\n", dma_scratch); */
#endif

  config_init_buffers(Config.cfgBuffers);

/* LoL->sfthead = (sfttbl FAR *)&basesft; */
  /* LoL->FCBp = (sfttbl FAR *)&FcbSft; */
  /* LoL->FCBp = KernelAlloc(sizeof(sftheader)
     + Config.cfgFiles * sizeof(sft)); */
  sp = LoL->sfthead->sftt_next;
  sp = sp->sftt_next = (sfttbl FAR *)
    KernelAlloc(sizeof(sftheader) + (Config.cfgFiles - 8) * sizeof(sft), 'F',
                Config.cfgFilesHigh);
  sp->sftt_next = (sfttbl FAR *) - 1;
  sp->sftt_count = Config.cfgFiles - 8;

  LoL->CDSp = KernelAlloc(sizeof(struct cds) * LoL->lastdrive, 'L', Config.cfgLastdriveHigh);

#ifdef DEBUG
  printf("Final: \n f_node 0x%x\n", LoL->f_nodes);
/*  printf(" FCB table 0x%p\n",LoL->FCBp);*/
  printf(" sft table 0x%p\n", LoL->sfthead->sftt_next);
  printf(" CDS table 0x%p\n", LoL->CDSp);
  printf(" DPB table 0x%p\n", LoL->DPBp);
#endif
  if (Config.cfgStacks)
  {
    VOID FAR *stackBase =
        KernelAlloc(Config.cfgStacks * Config.cfgStackSize, 'S',
                    Config.cfgStacksHigh);
    init_stacks(stackBase, Config.cfgStacks, Config.cfgStackSize);

    DebugPrintf(("Stacks allocated at %p\n", stackBase));
  }
  DebugPrintf(("Allocation completed: top at 0x%x\n", base_seg));

}

/* This code must be executed after device drivers has been loaded */
VOID configDone(VOID)
{
  if (UmbState == 1)
    para2far(base_seg)->m_type = MCB_LAST;

  if (HMAState != HMA_DONE)
  {
    mcb FAR *p;
    unsigned short kernel_seg;
    unsigned short hma_paras = (HMAFree+0xf)/16;

    allocmem(hma_paras, &kernel_seg);
    p = para2far(kernel_seg - 1);

    p->m_name[0] = 'S';
    p->m_name[1] = 'C';
    p->m_psp = 8;

    DebugPrintf(("HMA not available, moving text to %x\n", kernel_seg));
    MoveKernel(kernel_seg);

    kernel_seg += hma_paras + 1;

    DebugPrintf(("kernel is low, start alloc at %x", kernel_seg));
  }

  /* The standard handles should be reopened here, because
     we may have loaded new console or printer drivers in CONFIG.SYS */
}

STATIC seg prev_mcb(seg cur_mcb, seg start)
{
  /* determine prev mcb */
  seg mcb_prev, mcb_next;
  mcb_prev = mcb_next = start;
  while (mcb_next < cur_mcb && para2far(mcb_next)->m_type == MCB_NORMAL)
  {
    mcb_prev = mcb_next;
    mcb_next += para2far(mcb_prev)->m_size + 1;
  }
  return mcb_prev;
}

STATIC void umb_init(void)
{
  UCOUNT umb_seg, umb_size;
  seg umb_max;

  if (UMB_get_largest(&umb_seg, &umb_size))
  {
    UmbState = 1;

    /* reset root */
    LoL->uppermem_root = ram_top * 64 - 1;

    /* create link mcb (below) */
    para2far(base_seg)->m_type = MCB_NORMAL;
    para2far(base_seg)->m_size--;
    mumcb_init(LoL->uppermem_root, umb_seg - LoL->uppermem_root - 1);

    /* setup the real mcb for the devicehigh block */
    mcb_init(umb_seg, umb_size - 2, MCB_NORMAL); 

    umb_base_seg = umb_max = umb_start = umb_seg;
    UMB_top = umb_size;

    /* there can be more UMB's ! 
       this happens, if memory mapped devces are in between 
       like UMB memory c800..c8ff, d8ff..efff with device at d000..d7ff
       However some of the xxxHIGH commands still only work with
       the first UMB.
    */

    while (UMB_get_largest(&umb_seg, &umb_size))
    {
      seg umb_prev, umb_next;

      /* setup the real mcb for the devicehigh block */
      mcb_init(umb_seg, umb_size - 2, MCB_NORMAL);

      /* determine prev and next umbs */
      umb_prev = prev_mcb(umb_seg, LoL->uppermem_root);
      umb_next = umb_prev + para2far(umb_prev)->m_size + 1;

      if (umb_seg < umb_max)
      {
        if (umb_next - umb_seg - umb_size == 0)
        {
          /* should the UMB driver return
             adjacent memory in several pieces */
          umb_size += para2far(umb_next)->m_size + 1;
          para2far(umb_seg)->m_size = umb_size;
        }
        else
        {
          /* create link mcb (above) */
          mumcb_init(umb_seg + umb_size - 1, umb_next - umb_seg - umb_size);
        }
      }
      else /* umb_seg >= umb_max */
      {
        umb_prev = umb_next;
      }

      if (umb_seg - umb_prev - 1 == 0)
        /* should the UMB driver return
           adjacent memory in several pieces */
        para2far(prev_mcb(umb_prev, LoL->uppermem_root))->m_size += umb_size;
      else
      {
        /* create link mcb (below) */
        mumcb_init(umb_prev, umb_seg - umb_prev - 1);
      }

      if (umb_seg > umb_max)
        umb_max = umb_seg;
    }
    para2far(umb_max)->m_size++;
    para2far(umb_max)->m_type = MCB_LAST;
    DebugPrintf(("UMB Allocation completed: start at 0x%x\n", umb_base_seg));
  }
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
    
    if (nPass == 0) /* pass 0 always executed (rem Menu prompt switches) */
    {
      (*(pEntry->func)) (pLine);
      continue;
    }
    else
    {        
      if (SkipLine(pLineStart))   /* F5/F8 processing */
        continue;
    }

    if ((pEntry->func != CfgMenu) && (pEntry->func != CfgMenuEsc))
      pLine = skipwh(pLine);

    if ('=' != *pLine && pEntry->func != CfgMenu && pEntry->func != CfgMenuEsc)
      CfgFailure(pLine);
    else                        /* YES. DO IT */
      (*(pEntry->func)) (skipwh(pLine + 1));
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

UWORD GetBiosKey(int timeout)
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
  if (!isnum(*pLine) && *pLine != '-')
  {
    CfgFailure(pLine);
    return (BYTE *) 0;
  }
  return (BYTE *)GetNumber(pLine, pnArg);
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

  LoL->os_major = major;
  LoL->os_minor = minor;
}

STATIC VOID Files(BYTE * pLine)
{
  COUNT nFiles;

  /* Get the argument                                             */
  if (GetNumArg(pLine, &nFiles) == (BYTE *) 0)
    return;

  /* Got the value, assign either default or new value            */
  Config.cfgFiles = max(Config.cfgFiles, nFiles);
  Config.cfgFilesHigh = 0;
}

STATIC VOID FilesHigh(BYTE * pLine)
{
  Files(pLine);
  Config.cfgFilesHigh = 1;
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
  Config.cfgLastdriveHigh = 0;
}

STATIC VOID CfgLastdriveHigh(BYTE * pLine)
{
  /* Format:   LASTDRIVEHIGH = letter         */
  CfgLastdrive(pLine);
  Config.cfgLastdriveHigh = 1;
}

/*
    UmbState of confidence, 1 is sure, 2 maybe, 4 unknown and 0 no way.
*/

STATIC VOID Dosmem(BYTE * pLine)
{
  BYTE *pTmp;
  BYTE UMBwanted = FALSE;

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
    LoL->uppermem_link = 0;
    LoL->uppermem_root = 0xffff;
    UmbState = UMBwanted ? 2 : 0;
  }
  /* Check if HMA is available straight away */
  if (HMAState == HMA_REQ && MoveKernelToHMA())
  {
    HMAState = HMA_DONE;
  }
}

STATIC VOID DosData(BYTE * pLine)
{
  BYTE *pTmp;

  pLine = GetStringArg(pLine, szBuf);

  for (pTmp = szBuf; *pTmp != '\0'; pTmp++)
    *pTmp = toupper(*pTmp);

  if (fmemcmp(szBuf, "UMB", 3) == 0)
    Config.cfgDosDataUmb = TRUE;
}

STATIC VOID CfgSwitchar(BYTE * pLine)
{
  /* Format: SWITCHAR = character         */

  GetStringArg(pLine, szBuf);
  init_switchar(*szBuf);
}

STATIC VOID CfgSwitches(BYTE * pLine)
{
  pLine = skipwh(pLine);
  if (commands[0].pass == 0) {
    /* compatibility "device foo.sys" */
    if ('=' != *pLine && ' ' != *pLine && '\t' != *pLine)
    {
      CfgFailure(pLine);
      return;
    }
    pLine = skipwh(pLine + 1);
  }
  while (*pLine)
  {
    if (*pLine == '/') {
      pLine++;
      switch(toupper(*pLine)) {
      case 'K':
        if (commands[0].pass == 1)
          kbdType = 0; /* force conv keyb */
        break;
      case 'N':
        InitKernelConfig.SkipConfigSeconds = -1;
        break;
      case 'F':
        InitKernelConfig.SkipConfigSeconds = 0;
        break;
      case 'E': /* /E[[:]nnnn]  Set the desired EBDA amount to move in bytes */
        {       /* Note that if there is no EBDA, this will have no effect */
          char *p;
          int n = 0;
          if (*++pLine == ':')
            pLine++;                    /* skip optional separator */
          if ((p = GetNumArg(pLine, &n)) == 0) {
            Config.ebda2move = 0;
            break;
          }
          pLine = p - 1;              /* p points past number */
          /* allowed values: [48..1024] bytes, multiples of 16
           * e.g. AwardBIOS: 48, AMIBIOS: 1024
           * (Phoenix, MRBIOS, Unicore = ????)
           */
          if (n >= 48 && n <= 1024)
          {
            Config.ebda2move = (n + 15) & 0xfff0;
            break;
          }
          /* else fall through (failure) */
        }
      default:
        CfgFailure(pLine);
      }
    } else {
      CfgFailure(pLine);
    }
    pLine = skipwh(pLine+1);
  }
  commands[0].pass = 1;
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
  Config.cfgStacksHigh = 0;
}

STATIC VOID StacksHigh(BYTE * pLine)
{
  Stacks(pLine);
  Config.cfgStacksHigh = 1;
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
  seg base, start;
  char *p;

  if (mode)
  {
    base = umb_base_seg;
    start = umb_start;
  }
  else
  {
    base = base_seg;
    start = LoL->first_mcb;
  }

  if (base == start)
    base++;
  base++;
  
  /* Get the device driver name                                   */
  GetStringArg(pLine, szBuf);

  /* The driver is loaded at the top of allocated memory.         */
  /* The device driver is paragraph aligned.                      */
  eb.load.reloc = eb.load.load_seg = base;

#ifdef DEBUG
  printf("Loading device driver %s at segment %04x\n", szBuf, base);
#endif

  if ((result = init_DosExec(3, &eb, szBuf)) != SUCCESS)
  {
    CfgFailure(pLine);
    return result;
  }

  strcpy(szBuf, pLine);
  /* uppercase the device driver command */
  for (p = szBuf; *p != '\0'; p++)
    *p = toupper(*p);

  /* TE this fixes the loading of devices drivers with
     multiple devices in it. NUMEGA's SoftIce is such a beast
   */

  /* add \r\n to the command line */
  strcat(szBuf, " \r\n");

  dhp = MK_FP(base, 0);

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
    /* Link in device driver and save LoL->nul_dev pointer to next */
    dhp->dh_next = LoL->nul_dev.dh_next;
    LoL->nul_dev.dh_next = dhp;
  }

  /* might have been the UMB driver or DOS=UMB */
  if (UmbState == 2)
    umb_init();

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

struct submcb 
{
  char type;
  unsigned short start;
  unsigned short size;
  char unused[3];
  char name[8];
};

void FAR * KernelAllocPara(size_t nPara, char type, char *name, int mode)
{
  seg base, start;
  struct submcb FAR *p;

  if (UmbState != 1)
    mode = 0;

  if (mode)
  {
    base = umb_base_seg;
    start = umb_start;
  }
  else
  {
    base = base_seg;
    start = LoL->first_mcb;
  }

  /* create the special DOS data MCB if it doesn't exist yet */
  DebugPrintf(("kernelallocpara: %x %x %x %c %d\n", start, base, nPara, type, mode));

  if (base == start)
  {
    mcb FAR *p = para2far(base);
    base++;
    mcb_init(base, p->m_size - 1, p->m_type);
    mumcb_init(FP_SEG(p), 0);
    p->m_name[1] = 'D';
  }

  nPara++;
  mcb_init(base + nPara, para2far(base)->m_size - nPara, para2far(base)->m_type);
  para2far(start)->m_size += nPara;

  p = (struct submcb FAR *)para2far(base);
  p->type = type;
  p->start = FP_SEG(p)+1;
  p->size = nPara-1;
  if (name)
    fmemcpy(p->name, name, 8);
  base += nPara;
  if (mode)
    umb_base_seg = base;
  else
    base_seg = base;
  return MK_FP(FP_SEG(p)+1, 0);
}

void FAR * KernelAlloc(size_t nBytes, char type, int mode)
{
  void FAR *p;
  size_t nPara = (nBytes + 15)/16;

  if (LoL->first_mcb == 0)
  {
    /* prealloc */
    lpTop = MK_FP(FP_SEG(lpTop) - nPara, FP_OFF(lpTop));
    return AlignParagraph(lpTop);
  }
  else
  {
    p = KernelAllocPara(nPara, type, NULL, mode);
  }
  fmemset(p, 0, nBytes);
  return p;
}

#ifdef I86
#if 0
STATIC BYTE FAR * KernelAllocDma(WORD bytes, char type)
{
  if ((base_seg & 0x0fff) + (bytes >> 4) > 0x1000) {
    KernelAllocPara((base_seg + 0x0fff) & 0xf000 - base_seg, type, NULL, 0);
  }
  return KernelAlloc(bytes, type);
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

  if (isnum(*s))
  {
    unsigned numbers = 0;
    for ( ; isnum(*s); s++)
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

STATIC BOOL isnum(char ch)
{
  return (ch >= '0' && ch <= '9');
}

/* JPP - changed so will accept hex number. */
/* ea - changed to accept hex digits in hex numbers */
STATIC char * GetNumber(REG const char *p, int *num)
{
  unsigned char base = 10;
  int sign = 1;
  int n = 0;

  if (*p == '-')
  {
    p++;
    sign = -1;
  }

  for(;;p++)
  {
    unsigned char ch = toupper((unsigned char)*p);
    if (ch == 'X')
    {
      base = 16;
      continue;
    }
    if (isnum(ch))
    {
      n = n * base + ch - '0';
    }
    else if (base == 16 && (ch<='F') && (ch>='A'))
    {
      n = n * base + 10 + ch - 'A';
    }
    else
    {
      break;
    }
  }
  *num = n * sign;
  return (char *)p;
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
STATIC VOID mcb_init_copy(UCOUNT seg, UWORD size, mcb *near_mcb)
{
  near_mcb->m_size = size;
  fmemcpy(MK_FP(seg, 0), near_mcb, sizeof(mcb));
}

STATIC VOID mcb_init(UCOUNT seg, UWORD size, BYTE type)
{
  static mcb near_mcb = {0};
  near_mcb.m_type = type;
  mcb_init_copy(seg, size, &near_mcb);
}

STATIC VOID mumcb_init(UCOUNT seg, UWORD size)
{
  static mcb near_mcb = {
    MCB_NORMAL,
    8, 0,
    {0,0,0},
    {"SC"}
  };
  mcb_init_copy(seg, size, &near_mcb);
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
  LoL->nbuffers = anzBuffers;
  
  lpTop = lpOldTop;

  LoL->inforecptr = &LoL->firstbuf;
  
  {
    size_t bytes = sizeof(struct buffer) * anzBuffers;
    pbuffer = HMAalloc(bytes);

    if (pbuffer == NULL)
    {
      pbuffer = KernelAlloc(bytes, 'B', 0);
    }
    else
    {
      LoL->bufloc = LOC_HMA;
      LoL->deblock_buf = KernelAlloc(SEC_SIZE, 'B', 0);
    }
  }

  LoL->firstbuf = pbuffer;

  DebugPrintf(("init_buffers (size %u) at", sizeof(struct buffer)));
  DebugPrintf((" (%p)", LoL->firstbuf));

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

STATIC VOID CfgMenuEsc(BYTE * pLine)
{
  BYTE * check;
  for (check = pLine; check[0]; check++)
    if (check[0] == '$') check[0] = 27;	/* translate $ to ESC */
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

  /* Bulgaria - by Luchezar Georgiev */ ,{
    359,                        /*  = W1 W437   # Country ID & Codepage */
    855,
    _DATE_DMY,                  /*    Date format: 0/1/2: U.S.A./Europe/Japan */
    "BGL",                      /* '$' ,'EUR'   */
    " ",                        /* ','          # Thousand's separator */
    ",",                        /* '.'        # Decimal point        */
    ".",                        /* '-'  DateSeparator */
    ":",                        /* ':'  TimeSeparator */
    3,                          /*  # Currency format (bit array) */
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
        Luchezar Georgiev (BUL)
	Yuki Mitsui (JAP)
	Aitor Santamar­a Merino (SP)
*/	

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
struct instCmds {
  char buffer[128];
  int mode;
} InstallCommands[10];

#ifdef DEBUG
#define InstallPrintf(x) printf x
#else
#define InstallPrintf(x)
#endif

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

STATIC VOID InstallExec(struct instCmds *icmd)
{                            
  BYTE filename[128], *args, *d, *cmd = icmd->buffer;
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

  if (init_DosExec(icmd->mode, &exb, filename) != SUCCESS)
  {
    CfgFailure(cmd);
  }
}		

STATIC void free(seg segment)
{
  iregs r;
        
  r.a.b.h = 0x49;				/* free memory	*/
  r.es  = segment;
  init_call_intr(0x21, &r);
}

VOID DoInstall(void)
{
  int i;
  iregs r;
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

  allocmem(((unsigned)_init_end+15)/16, &installMemory);

  InstallPrintf(("allocated memory at %x\n",installMemory));

  for (i = 0; i < numInstallCmds; i++)
  {
    InstallPrintf(("%d:%s\n",i,InstallCommands[i].buffer));
    
    r.a.x = 0x5801;             /* set memory allocation strategy */
    r.b.b.l = InstallCommands[i].mode;
    init_call_intr(0x21, &r);
    
    InstallExec(&InstallCommands[i]);
  }      

  r.a.x = 0x5801;             /* set memory allocation strategy */
  r.b.b.l = 0x00;			    /*low memory, high			*/
  init_call_intr(0x21, &r);
    
  free(installMemory);
  
  InstallPrintf(("Done with installing commands\n"));
  return;
}
