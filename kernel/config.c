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
#include "debug.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id$";
#endif

#define testbit(v,bit) ((UBYTE)((v) >> (UBYTE)(bit)) & 1)

static UBYTE MenuColor BSS_INIT(0);

static unsigned screenwidth(void)
{
  return peek(0, 0x44a);
}

static unsigned screenbottom(void)
{
  UBYTE row = peekb(0, 0x484);
  if (row == 0)
      row = 24;
  return row;
}

static unsigned screeny(void)
{
  iregs r;
  r.BH = peekb(0, 0x462);	/* active video page */
  r.AH = 0x03;			/* get cursor pos */
  init_call_intr(0x10, &r);
  return r.DH;
}

static void gotoxy(unsigned x, unsigned y)
{
  iregs r;
  r.BH = peekb(0, 0x462);	/* active video page */
  r.DL = x;
  r.DH = y;
  r.AH = 0x02;			/* set cursor pos */
  init_call_intr(0x10, &r);
}

static void ClearScreenArea(UBYTE attr, unsigned x, unsigned y, unsigned w, unsigned h)
{
  iregs r;
  r.BH = attr;
  r.CL = x; r.DL = x + w - 1;
  r.CH = y; r.DH = y + h - 1;
  r.AX = 0x0600;		/* clear rectangle */
  init_call_intr(0x10, &r);

  gotoxy(x, y);
}

static void say(PCStr s)
{
  printf(s);
}

static void say2(PCStr f, PCStr s)
{
  printf(f, s);
}

static void clearrow(void)
{
  unsigned width;
  say("\r");
  for (width = screenwidth(); --width;)
    say(" ");
  say("\r");
}

static seg_t umb_base_start BSS_INIT(0);
static size_t ebda_size BSS_INIT(0);

struct config Config = {
  /* UBYTE cfgDosDataUmb;    */ 0,
  /* BYTE  cfgBuffers;       */ NUMBUFF,
  /* UBYTE cfgFiles;         */ NFILES,
  /* UBYTE cfgFilesHigh;     */ 0,
  /* UBYTE cfgFcbs;          */ NFCBS,
  /* UBYTE cfgProtFcbs;      */ 0,
  /* char  cfgShell[256];    */ "command.com /P /E:256",
  /* UBYTE cfgLastdrive;     */ NLAST,
  /* UBYTE cfgLastdriveHigh; */ 0,
  /* BYTE  cfgStacks;        */ NSTACKS,
  /* BYTE  cfgStacksHigh;    */ 0,
  /* UWORD cfgStackSize;     */ STACKSIZE,
  /* UBYTE cfgP_0_startmode; */ 0, /* load command.com (low by default) */
  /* unsigned ebda2move;     */ 0, /* value for SWITCHES=/E:nnnn */
};

/* master_env copied over command line area in
   DOS_PSP, thus its size limited to 128 bytes */
static char master_env[128] = "PATH=.";

STATIC seg_t base_seg BSS_INIT(0);
STATIC seg_t umb_base_seg BSS_INIT(0);
VFP lpTop BSS_INIT(0);
STATIC unsigned nCfgLine BSS_INIT(0);

static unsigned nPass BSS_INIT(0);
static char configfile [] = "FDCONFIG.SYS";
STATIC char szLine[256] BSS_INIT({0});
STATIC char szBuf[256] BSS_INIT({0});

UBYTE askCommand BSS_INIT(0);

static int      MenuTimeout = -1;
static unsigned last_choice = 10; /* =non existing choice */
static unsigned line_choices BSS_INIT(0);
static unsigned all_choices BSS_INIT(0);

STATIC BOOL LoadDevice(PCStr, VFP top, int mode);
STATIC void CfgFailure(PCStr);

STATIC VOID DoMenu(void);
STATIC PCStr skipwh(PCStr);
STATIC PCStr scanword(PCStr, PStr);
STATIC PCStr scanverb(PCStr, PStr);
#define isdigit(ch) ((UBYTE)((ch) - '0') <= 9)
STATIC char toupper(char c);
static PStr strupr(PStr);
static PStr strcat(PStr d, PCStr s);
STATIC void mcb_init(seg_t, size_t, BYTE type);
STATIC void mumcb_init(seg_t, size_t);
#define mcb_next(seg) ((seg) + MK_SEG_PTR(mcb, seg)->m_size + 1)

STATIC PCStr GetNumArg(PCStr);
STATIC BOOL GetNumArg1(PCStr);
STATIC BOOL GetNumArg2(PCStr, int default2);
static void hintSkipAll(void);
static BOOL askSkipLine(void);
STATIC char strcasediff(PCStr, PCStr);
STATIC void LoadCountryInfoHardCoded(PCStr filename, int ccode, int cpage);
STATIC void umb_init(void);

STATIC void config_init_buffers(int anzBuffers);     /* from BLOCKIO.C */
STATIC void config_init_fnodes(int f_nodes_cnt);

#define EOF 0x1a

typedef void config_sys_func_t(PCStr);

STATIC config_sys_func_t
  CfgSwitches,
  CfgMenuColor, CfgMenuDefault, CfgMenu, CfgMenuEsc,
  CfgBreak, Config_Buffers, Country, Dosmem, DosData,
  Fcbs, Files, FilesHigh, CfgLastdrive, CfgLastdriveHigh,
  Numlock, CmdShell, CmdShellHigh,
  Stacks, StacksHigh, CfgSwitchar,
  sysScreenMode, sysVersion, SetAnyDos,
  Device, DeviceHigh, CmdInstall, CmdInstallHigh, CmdSet;

STATIC struct table {
  PCStr const entry;
  UBYTE pass;
  config_sys_func_t *const func;

} commands [] = {

  /* this one is special since it checked for /N/F before F5/F8 and
     asked for /K/E after menu; DoConfig() changes commands[0].pass */
  {"SWITCHES", 0, CfgSwitches},

  {"REM", 100, NULL},

  {"MENUCOLOR", 0, CfgMenuColor},
  {"MENUDEFAULT", 0, CfgMenuDefault},
  {"MENU", 1, CfgMenu},			/* lines to print in pass 1 */
  {"ECHO", 3, CfgMenu},			/* lines to print in pass 3 */
  {"EECHO", 3, CfgMenuEsc},		/* modified ECHO (ea) */

  {"BREAK", 2, CfgBreak},
  {"BUFFERS", 2, Config_Buffers},
  {"BUFFERSHIGH", 2, Config_Buffers},	/* currently dummy */
  {"COUNTRY", 2, Country},
  {"DOS", 2, Dosmem},
  {"DOSDATA", 2, DosData},
  {"FCBS", 2, Fcbs},
  {"FILES", 2, Files},
  {"FILESHIGH", 2, FilesHigh},
  {"LASTDRIVE", 2, CfgLastdrive},
  {"LASTDRIVEHIGH", 2, CfgLastdriveHigh},
  {"NUMLOCK", 2, Numlock},
  {"STACKS", 2, Stacks},
  {"STACKSHIGH", 2, StacksHigh},
  {"SWITCHAR", 2, CfgSwitchar},
  {"SCREEN", 2, sysScreenMode},		/* JPP */
  {"VERSION", 2, sysVersion},		/* JPP */
  {"ANYDOS", 2, SetAnyDos},		/* tom */

  {"DEVICE", 3, Device},
  {"DEVICEHIGH", 3, DeviceHigh},

  {"INSTALL", 4, CmdInstall},
  {"INSTALLHIGH", 4, CmdInstallHigh},
  {"SET", 4, CmdSet},
  {"SHELL", 4, CmdShell},
  {"SHELLHIGH", 4, CmdShellHigh},
};

enum {	HMA_NONE,		/* do nothing */
	HMA_REQ,		/* DOS=HIGH detected */
	HMA_DONE,		/* Moved kernel to HMA */
};

enum {	UMB_NONE,		/* do nothing */
	UMB_DONE,		/* UMB initialized */
	UMB_REQ,		/* DOS=UMB detected */
};

enum {LOC_CONV=0, LOC_HMA=1};  /* dup in global.h */

static UBYTE HMAState BSS_INIT(HMA_NONE);
static UBYTE UmbState BSS_INIT(UMB_NONE);

/* Do first time initialization.  Store last so that we can reset it    */
/* later.                                                               */
void PreConfig(void)
{
  /* Initialize the base memory pointers                          */

  CfgDbgPrintf(("SDA located at 0x%p\n", internal_data));

  /* Begin by initializing our system buffers                     */

  LoL->DPBp =
      DynAlloc("DPBp", blk_dev.dh_name[0], sizeof(struct dpb));

  /* Initialize the file table                                    */
  CfgDbgPrintf(("Initializing fnodes\n"));
  config_init_fnodes(Config.cfgFiles);

  LoL->sfthead = MK_PTR(struct sfttbl, FP_SEG(LoL), 0xcc); /* &(LoL->firstsftt) */
  /* LoL->FCBp = (sfttbl FAR *)&FcbSft; */
  /* LoL->FCBp = (sfttbl FAR *)
     KernelAlloc(sizeof(sftheader)
     + Config.cfgFiles * sizeof(sft)); */

  config_init_buffers(Config.cfgBuffers);

  LoL->CDSp = KernelAlloc(sizeof(struct cds) * LoL->lastdrive, 'L', 0);

  CfgDbgPrintf(("Preliminary:\n f_node 0x%p\n", LoL->f_nodes));
/*  CfgDbgPrintf((" FCB table 0x%p\n",LoL->FCBp));*/
  CfgDbgPrintf((" CDS table 0x%p\n", LoL->CDSp));
  CfgDbgPrintf((" DPB table 0x%p\n", LoL->DPBp));

  /* Done.  Now initialize the MCB structure                      */
  /* This next line is 8086 and 80x86 real mode specific          */
  CfgDbgPrintf(("Preliminary allocation completed: top at %p\n", lpTop));

  /* initialize environment */
  fmemcpy(MK_PTR(char, DOS_PSP + 8, 0), master_env, sizeof(master_env));
}

static void KernelAllocSFT(sfttbl FAR *p, unsigned files, int high)
{
  p = p->sftt_next = (sfttbl FAR *)KernelAlloc(sizeof(sftheader) +
                                               files * sizeof(sft), 'F', high);
  p->sftt_count = files;
  p->sftt_next = (sfttbl FAR *)-1l;
}

#define EBDASEG 0x40e
#define RAMSIZE 0x413
#define RAM_size() peek(0, RAMSIZE)

/* Do pre-drivers initialization: near allocation and MCBs              */
static void PreConfig3(void)
{
  /* initialize NEAR allocated things */
  /* Initialize the base memory pointers from last time.          */
  /*
     if the kernel could be moved to HMA, everything behind the dynamic
     near data is free.
     otherwise, the kernel is moved down - behind the dynamic allocated data,
     and allocation starts after the kernel.
   */

  size_t ram_top = RAM_size(); /* how much conventional RAM, kb */
  seg_t ebdaseg = peek(0, EBDASEG);
  size_t ebdasz = peekb(ebdaseg, 0);
  if (Config.ebda2move && ram_top * 64 == ebdaseg && ebdasz <= 63)
  {
    ram_top += ebdasz;
    ebdasz *= 1024u;
    if (ebdasz > Config.ebda2move)
        ebdasz = (Config.ebda2move + 15) & -16;
    ebda_size = ebdasz;
  }

  /* We expect ram_top as Kbytes, so convert to paragraphs */
  base_seg = LoL->first_mcb = FP_SEG(alignNextPara(DynLast()));
  mcb_init(base_seg, ram_top * 64 - base_seg, MCB_LAST);

  /* allocate space in low memory for 3 file handles
     (SFT) in addition to 5 handles, builtin into LOL */
  KernelAllocSFT(LoL->sfthead, 3, 0);

  if (ebda_size)  /* move the Extended BIOS Data Area from top of RAM here */
  {
    seg_t new_seg = FP_SEG(KernelAlloc(ebda_size, 'I', 0));
    fmemcpy(MK_SEG_PTR(BYTE, new_seg), MK_SEG_PTR(const BYTE, ebdaseg), ebda_size);
    poke(0, EBDASEG, new_seg);
    poke(0, RAMSIZE, ram_top);
  }

  umb_init();
}

/* Do last initialization.                                        */
void PostConfig(void)
{
  /* We could just have loaded FDXMS or HIMEM */
  if (HMAState == HMA_REQ && MoveKernelToHMA())
    HMAState = HMA_DONE;

  if (Config.cfgDosDataUmb)
  {
    Config.cfgFilesHigh =
    Config.cfgStacksHigh =
    Config.cfgLastdriveHigh = 1;
  }

  /* compute lastdrive ... */
  {
    UBYTE drv = Config.cfgLastdrive;
    if (drv < LoL->nblkdev)
        drv = LoL->nblkdev;
    LoL->lastdrive = drv;
  }

  CfgDbgPrintf(("starting FAR allocations at %x\n", base_seg));

  /* Initialize the file table                                    */
  config_init_fnodes(Config.cfgFiles);

  /* Begin by initializing our system buffers                     */
  /* dma_scratch = (BYTE FAR *) KernelAllocDma(BUFFERSIZE); */
  /* CfgDbgPrintf(("DMA scratchpad allocated at 0x%p\n", dma_scratch)); */

  config_init_buffers(Config.cfgBuffers);

/* LoL->sfthead = (sfttbl FAR *)&basesft; */
  /* LoL->FCBp = (sfttbl FAR *)&FcbSft; */
  /* LoL->FCBp = KernelAlloc(sizeof(sftheader)
     + Config.cfgFiles * sizeof(sft)); */

  /* allocate space for remaining file handles (SFT); 5 are
     already builtin and 3 are allocated in PreConfig3() */
  KernelAllocSFT(LoL->sfthead->sftt_next,
                 Config.cfgFiles - 8, Config.cfgFilesHigh);

  LoL->CDSp = KernelAlloc(sizeof(struct cds) * LoL->lastdrive, 'L', Config.cfgLastdriveHigh);

  CfgDbgPrintf(("Final: \n f_node 0x%p\n", LoL->f_nodes));
/*  printf(" FCB table 0x%p\n",LoL->FCBp);*/
  CfgDbgPrintf((" CDS table 0x%p\n", LoL->CDSp));
  CfgDbgPrintf((" DPB table 0x%p\n", LoL->DPBp));

  if (Config.cfgStacks)
  {
    void _seg *stackBase =
        KernelAlloc(Config.cfgStacks * Config.cfgStackSize, 'S',
                    Config.cfgStacksHigh);
    init_stacks(stackBase, Config.cfgStacks, Config.cfgStackSize);

    CfgDbgPrintf(("Stacks allocated at %p\n", stackBase));
  }
  CfgDbgPrintf(("Allocation completed: top at 0x%x\n", base_seg));

}

/* This code must be executed after device drivers has been loaded */
VOID configDone(VOID)
{
  if (UmbState == UMB_DONE)
    MK_SEG_PTR(mcb, base_seg)->m_type = MCB_LAST;

  if (HMAState != HMA_DONE)
  {
    register size_t hma_paras = (HMAFree + 15) / 16;
    seg_t kernel_seg = allocmem(hma_paras);
    mcb _seg *p = MK_SEG_PTR(mcb, kernel_seg - 1);

    p->m_name[0] = 'S';
    p->m_name[1] = 'C';
    p->m_psp = 8;

    CfgDbgPrintf(("HMA not available, moving text to %x\n", kernel_seg));
    MoveKernel(kernel_seg);
    CfgDbgPrintf(("kernel is low, start alloc at %x\n",
                 kernel_seg + hma_paras + 1));
  }

  /* The standard handles should be reopened here, because
     we may have loaded new console or printer drivers in CONFIG.SYS */
}

STATIC void umb_init(void)
{
  CVFP xms_addr;
  seg_t umb_seg;
  size_t umb_size;

  if (UmbState == UMB_REQ &&
      (xms_addr = DetectXMSDriver()) != NULL &&
      UMB_get_largest(xms_addr, &umb_seg, &umb_size))
  {
    seg_t umb_furthest = umb_seg;

    /* SAFETY: new block may be too small or below memory top */
    seg_t base_top = RAM_size() * 64 - 1;
    if (umb_size < 2 || umb_furthest <= base_top)
      return;

    UmbState = UMB_DONE;

    /* reset root */
    LoL->uppermem_root = base_top;
    umb_base_seg = umb_furthest;

    /* create link mcb (below) */
    /* (it prefixes hole between UMBs) */
    MK_SEG_PTR(mcb, base_seg)->m_type = MCB_NORMAL;
    MK_SEG_PTR(mcb, base_seg)->m_size--;
    mumcb_init(base_top, umb_seg - base_top);

    /* setup the real mcb for the devicehigh block */
    mcb_init(umb_seg, umb_size, MCB_NORMAL);

    /* there can be more UMBs !
       this happens, if memory mapped devices are in between
       like UMB memory c800..c8ff, d8ff..efff with device at d000..d7ff
       However some of the xxxHIGH commands still only work with
       the first UMB.
    */
    while (UMB_get_largest(xms_addr, &umb_seg, &umb_size))
    {
      if (umb_furthest < umb_seg)
      {
        seg_t umb_hole = mcb_next(umb_furthest);

        /* SAFETY: new block may (1) be adjacent to old block,
           (2) overlap it or (3) be inside. (2) and (3) are errors
           and shouldn't happen, but handle them anyway. */
        if (umb_hole >= umb_seg)
        {
          size_t overlap = umb_hole - umb_seg;
          if (overlap < umb_size)
            /* join adjacent/overlapped new block to prev block */
            MK_SEG_PTR(mcb, umb_furthest)->m_size += umb_size - overlap;
          continue;
        }

        /* SAFETY: new block may be too small */
        if (umb_size < 2)
          continue;

        /* create link mcb (below) */
        MK_SEG_PTR(mcb, umb_furthest)->m_size--;
        umb_furthest = umb_seg;
        umb_hole--;
        mumcb_init(umb_hole, umb_seg - umb_hole);
      }
      else /* umb_seg <= umb_furthest */
      {
        seg_t umb_prev, umb_hole, umb_next, umb_cur_next;

        /* SAFETY: umb_seg may point into base memory */
        if (base_top >= umb_seg)
          continue;

        /* find previous block */
        for (umb_hole = base_top;;)
        {
          umb_next = mcb_next(umb_hole);
          /* ASSUME umb_hole->m_type == 'M' && umb_hole->m_psp == 8 &&
                    umb_next->m_type == 'M' && umb_next->m_psp != 8 &&
                    umb_hole < umb_next <= umb_furthest */
          if (umb_seg <= umb_next)
            break;
          umb_prev = umb_next;
          umb_hole = mcb_next(umb_next);
        }
        /* NOW base_top == umb_hole < umb_seg <= umb_next ||
               base_top  < umb_prev < umb_seg <= umb_next */

        /* SAFETY: new block may be inside previous block */
        umb_cur_next = umb_seg + umb_size;
        if (umb_cur_next <= umb_hole)
          continue;

        /* SAFETY: umb_seg may point below hole,
           so, use ">=" instead "==" */
        if (umb_hole + 1 >= umb_seg && umb_hole != base_top)
          /* join adjacent/overlapped new block to prev block */
          umb_size = umb_cur_next - (umb_seg = umb_prev);
        else
        {
          /* SAFETY: new block may be too small */
          if (umb_size < 2 && umb_cur_next < umb_next)
            continue;
          /* adjust link mcb below */
          MK_SEG_PTR(mcb, umb_hole)->m_size = umb_seg - umb_hole - 1;
        }

        /* SAFETY: new block may overlap next block,
           so, use ">=" instead "==" */
        if (umb_cur_next >= umb_next)
        {
          /* join adjacent/overlapped next block to new block */
          if (umb_furthest == umb_next)
              umb_furthest = umb_seg;
          if (umb_base_seg == umb_next)
              umb_base_seg = umb_seg;
          umb_size = mcb_next(umb_next) - umb_seg;
        }
        else /* umb_cur_next < umb_next */
        {
          /* create link mcb (above) */
          umb_size--, umb_cur_next--;
          mumcb_init(umb_cur_next, umb_next - umb_cur_next);
        }
      } /* else */

      /* setup the real mcb for the devicehigh block */
      mcb_init(umb_seg, umb_size, MCB_NORMAL);
    } /* while */

    MK_SEG_PTR(mcb, umb_furthest)->m_type = MCB_LAST;
    umb_base_start = umb_base_seg;

    DebugPrintf(("UMB Allocation completed: start at 0x%x\n", umb_base_seg));
  } /* if */
}

STATIC const struct table * LookUp(CStr token)
{
  const struct table *p = commands;
  do
    if (!strcasediff(p->entry, token))
      return p;
  while (++p < ENDOF(commands));
  p = NULL;
  return p;
}

static void DoConfig_(void)
{
  int nFileDesc, done;

  if (askCommand & ASK_SKIPALL)
    return;

  /* Check to see if we have a config.sys file.  If not, just     */
  /* exit since we don't force the user to have one.              */
  /*strcpy (configfile, "FDCONFIG.SYS");*/
  if ((nFileDesc = open(configfile, 0)) < 0)
  {
    CfgDbgPrintf(("%s not found\n", configfile));
    strcpy (configfile, "CONFIG.SYS");
    if ((nFileDesc = open(configfile, 0)) < 0)
    {
      CfgDbgPrintf(("%s not found\n", configfile));
      return;
    }
  }
  CfgDbgPrintf(("Reading %s...\n", configfile));

  /* Read each line into the buffer and then parse the line,      */
  /* do the table lookup and execute the handler for that         */
  /* function.                                                    */
  done = nCfgLine = 0;
  do
  {
    PStr q;
    PCStr p;
    const struct table *pEntry;

    /* read in a single line, \n or ^Z terminated */
    nCfgLine++;
    for (q = szLine;;)
    {
      /* if EOF already found, error reading, or EOF char read then we're done */
      if (read(nFileDesc, q, 1) != 1 || *q == EOF)
      {
        done++;
        break;
      }

      if (*q == '\n')		/* end of line */
        break;

      if (*q != '\r')		/* ignore CR */
      {
        q++;
        if (q >= szLine + sizeof szLine - 1)
        {
          /* *q = 0; */ /* static memory already zeroed */
          CfgFailure(q);
          say("error - line overflow\n");
          break;
        }
      }
    } /* for */
    *q = 0;			/* terminate line - make ASCIIZ */

    p = skipwh(szLine);
    if (*p == '\0' || *p == ';')
      continue;			/* skip empty line and comment */

    p = scanverb(p, szBuf);	/* extract verb */
    pEntry = LookUp(szBuf);
    if (pEntry == NULL)
    {
      if (nPass == 2)		/* only at pass 2 (after menu)... */
        CfgFailure(p);		/* ...say error for wrong verb */
      continue;
    }

    if (nPass != pEntry->pass ||
        nPass > 1 &&		/* after menu: check "123?device=" */
         !testbit(line_choices, last_choice))
      continue;

    if (pEntry->func != CfgMenu && pEntry->func != CfgMenuEsc)
    {
      if (*p)
      {
        if (*p != ' ' && *p != '\t' && *p != '=')
        {
          CfgFailure(p);
          continue;
        }
        p = skipwh(p);
        if (*p == '=')		/* accept "device foo.sys" without '=' */
          p = skipwh(p + 1);
      }
      if (nPass > 1 && askSkipLine()) /* after menu: processing "?" */
        continue;
    }
    if (*p == ' ' || *p == '\t')
      p++;

    /* YES. DO IT */
    pEntry->func(p);
  } while (!done && !(askCommand & ASK_SKIPALL));

  close(nFileDesc);
  nPass++;
}

void DoConfig()
{
  DoConfig_();			/* switches=/f/n, menucolor=	*/
  hintSkipAll();
  if (!(askCommand & ASK_SKIPALL))
  {
    if (MenuColor)		/* color defined?		*/
      ClearScreenArea(MenuColor, 0, 0, screenwidth(), screenbottom() + 1);
    DoConfig_();		/* show MENU, find choices	*/
    if (all_choices)
    {
      if (!testbit(all_choices, last_choice))
      {
        unsigned ac;
        last_choice = 0;	/* find lowest existing choice	*/
        for (ac = all_choices; !testbit(ac, 0); ac >>= 1)
          last_choice++;
      }
      if ((all_choices - 1) & all_choices) /* more than one choice? */
        DoMenu();
      if (!(askCommand & ASK_SKIPALL))
      {
        static char choice[] = "CONFIG=0";
        choice[7] = (UBYTE)'0' + last_choice;
        CmdSet(choice);		/* show choice in environment	*/
      }
    }
    commands[0].pass = 2;	/* switches=/k/e at pass 2	*/
    DoConfig_();		/* break=, files=, etc.		*/
  }
  PreConfig3();
  DoConfig_();			/* device=			*/
}

/*
    get BIOS key with timeout:
    timeout < 0: no timeout, remove returned key from keyboard buffer
    timeout = 0: poll only once
    timeout > 0: timeout for poll in seconds
    return
            0      : no key hit (only for timeout >= 0)
            0xHH.. : scancode in upper  half
            0x..LL : asciicode in lower half
*/

#define GetBiosTime() peekl(0, 0x46c)

#define ESC	27
#define K_F5	0x3F00
#define K_F8	0x4200
#define K_Left	0x4B00
#define K_Right	0x4D00
#define K_Up	0x4800
#define K_Down	0x5000

unsigned GetBiosKey(int timeout)
{
  iregs r;
  if (timeout >= 0)
  {
    ULONG startTime = GetBiosTime();
    do
    {
      r.AH = 0x01;		/* are there keys available ? */
      init_call_intr(0x16, &r);
      if (!(r.flags & FLG_ZERO))
        return r.AX;
    } while ((unsigned)(GetBiosTime() - startTime) < timeout * 18u);
    return 0;
  }

  /* key available or blocking wait (timeout < 0): fetch it */
  r.AH = 0x00;
  init_call_intr(0x16, &r);
  return r.AX;
}

static void hintSkipAll(void)
{
  int timeout = InitKernelConfig.SkipConfigSeconds;
  if (timeout >= 0)
  {
    unsigned key;

    if ((all_choices - 1) & all_choices) /* more than one choice? */
      timeout = 0;

    if (timeout > 0)
      say2("Press F8 to trace or F5 to skip %s/AUTOEXEC.BAT", configfile);
    key = GetBiosKey(timeout);
    clearrow();			/* clear hint line		*/

    if (key == K_F8)		/* F8 */
    {
      askCommand |= ASK_TRACE;
      GetBiosKey(-1);		/* remove key from buffer	*/
    }
    else if (key == K_F5)	/* F5 */
    {
      askCommand |= ASK_SKIPALL;
      say2("Bypassing %s and AUTOEXEC.BAT files.\n", configfile);
      GetBiosKey(-1);		/* remove key from buffer	*/
    }
  }
}

static BOOL askSkipLine(void)
{
  /* !device= never ask / ?device= always ask / device= ask if ASK_TRACE */
  /* "!device?=" will not be asked... */
  if ((askCommand & (ASK_NOASK | ASK_YESALL)) || /* "!" or Esc */
      !(askCommand & (ASK_ASK | ASK_TRACE)))	 /* not ("?" or trace) */
    return FALSE; /* do not skip, and do not ask either */

  say2("%s[Y,n]?", szLine);
  for (;;)
  {
    unsigned key = GetBiosKey(-1); /* wait keypress */
    if (key == K_F5)		/* YES, you may hit F5 here, too */
    {
      askCommand |= ASK_SKIPALL;
      key = 'N';
    }
    if (key == K_F8)		/* F8 */
      key = ESC;
    switch (toupper((UBYTE)key))
    {
      case 'N':
        say("N\n");
        return TRUE;

      case ESC:			/* Esc answers all following questions YES */
        askCommand &= ~ASK_TRACE;
        askCommand |= ASK_YESALL;
        /* and fall through */

      case '\r':
      case '\n':
      case 'Y':
        say("Y\n");
        return FALSE;
    }
  } /* for */
}

static int numarg BSS_INIT(0);

/* JPP - changed so will accept hex number. */
/* ea - changed to accept hex digits in hex numbers */
STATIC PCStr GetNumArg(PCStr p)
{
  static char digits[] = "0123456789ABCDEF";
  unsigned char base = 10;
  int sign = 1;
  int n = 0;

  /* look for NUMBER                               */
  p = skipwh(p);
  if (!isdigit(*p))
  {
    if (*p != '-')
    {
      CfgFailure(p);
      return NULL;
    }
    sign = -1;
    p++;
  }

  for( ; *p; p++)
  {
    char ch = toupper(*p);
    if (ch == 'X')
      base = 16;
    else
    {
      PCStr q = strchr(digits, ch);
      if (q == NULL)
        break;
      n = n * base + (q - digits);
    }
  }
  numarg = n * sign;
  return p;
}

STATIC BOOL isEOL(PCStr p)
{
  if (*p) /* garbage at line end? */
  {
    CfgFailure(p);
    return FALSE;
  }
  return TRUE;
}

/* Format: nnn EOL */
STATIC BOOL GetNumArg1(PCStr p)
{
  p = GetNumArg(p);
  if (p == NULL)
    return FALSE;
  return isEOL(skipwh(p));
}

static int numarg1 BSS_INIT(0);

/* Format: nnn [, nnn] EOL */
STATIC BOOL GetNumArg2(PCStr p, int default2)
{
  p = GetNumArg(p);
  if (p == NULL)
    return FALSE;

  numarg1 = numarg;
  numarg = default2;
  p = skipwh(p);
  if (*p == ',')
    return GetNumArg1(p + 1);

  return isEOL(p);
}

/* Format: BUFFERS [=] nnn [, nnn] */
STATIC void Config_Buffers(PCStr p)
{
  if (GetNumArg2(p, 0))
    Config.cfgBuffers = (UBYTE)numarg1;
  /* Second argument (0..8 buffers for read-ahead) not supported */
}

/* Set screen mode - rewritten to use init_call_intr() by RE / ICD */
/* Format: SCREEN [=] nnn */
STATIC void sysScreenMode(PCStr p)
{
  if (GetNumArg1(p))
  {
    unsigned mode = numarg;
    if (mode >= 0x10)
    {
      /* Modes
         0x11 (17)   28 lines
         0x12 (18)   43/50 lines
         0x14 (20)   25 lines
       */
      if (mode != 0x11 && mode != 0x12 && mode != 0x14)
        return; /* do nothing; invalid screenmode */
      mode |= 0x1100;
    }
    {
      iregs r;
      r.BL = 0;			/* block to load for AH=0x11 */
      r.AX = mode;		/* set videomode */
      init_call_intr(0x10, &r);
    }
  }
}

/* Format: VERSION [=] nn.nn */
STATIC void sysVersion(PCStr p)
{
  int major;
  p = GetNumArg(p);
  major = numarg;
  if (p == NULL || *p != '.' || !GetNumArg1(p + 1))
    return;

  printf("Changing reported version to %d.%d\n",
         LoL->os_setver_major = (UBYTE)major,
         LoL->os_setver_minor = (UBYTE)numarg);
}

/* Format: FILES [=] nnn */
/* Format: FILESHIGH [=] nnn */
static void _Files(PCStr p, UBYTE high)
{
  if (GetNumArg1(p))
  {
    UBYTE nFiles = (UBYTE)numarg;
    if (Config.cfgFiles < nFiles)
        Config.cfgFiles = nFiles;
    Config.cfgFilesHigh = high;
  }
}

STATIC void Files(PCStr p)		{ _Files(p, 0); }

STATIC void FilesHigh(PCStr p)		{ _Files(p, 1); }

/* Format: LASTDRIVE [=] letter */
/* Format: LASTDRIVEHIGH [=] letter */
static void _CfgLastdrive(PCStr p, UBYTE high)
{
  BYTE drv = toupper(*p);
  if (drv < 'A' || drv > 'Z' || p[1])
  {
    /* no or wrong character or garbage at line end? */
    CfgFailure(p);
    return;
  }

  drv -= 'A' - 1; /* Make real number */
  if (Config.cfgLastdrive < drv)
      Config.cfgLastdrive = drv;
  Config.cfgLastdriveHigh = high;
}

STATIC void CfgLastdrive(PCStr p)	{ _CfgLastdrive(p, 0); }

STATIC void CfgLastdriveHigh(PCStr p)	{ _CfgLastdrive(p, 1); }

/* UmbState of confidence, UMB_DONE is sure, UMB_REQ maybe, UMB_NONE no way.
   Transitions: UMB_NONE -> UMB_NONE/UMB_REQ depending on DOS=UMB, try init
   (UMB_REQ -> UMB_DONE) after each driver load, as it could have been the
   UMB driver.
   If UMB really found, state UMB_DONE is reached and MCBs are adjusted.
*/
/* opt = HIGH | UMB
   Format: DOS [=] opt {, opt}
*/
STATIC void Dosmem(PCStr p)
{
  UBYTE UMBwanted = UMB_NONE, HMAwanted = HMA_NONE;
  for (;;)
  {
    PCStr q = scanword(p, szBuf);
    if (!strcasediff(szBuf, "UMB"))
      UMBwanted = UMB_REQ;
    else if (!strcasediff(szBuf, "HIGH"))
      HMAwanted = HMA_REQ;
/*  else if (!strcasediff(szBuf, "CLAIMINIT"))
 *    INITDataSegmentClaimed = 0;
 */
    else
    {
      CfgFailure(p);
      return;
    }
    p = skipwh(q);
    if (*p != ',')
    {
      if (*p == '\0')
        break;
      CfgFailure(p);
      return;
    }
    p++;
  } /* for */

  if (UmbState == UMB_NONE)
  {
    LoL->uppermem_link = 0;
    LoL->uppermem_root = 0xffff;
    UmbState = UMBwanted;
  }
  /* Check if HMA is available straight away */
  if (HMAwanted == HMA_REQ)
    HMAState = MoveKernelToHMA() ? HMA_DONE : HMA_REQ;
}

/* Format: DOSDATA [=] UMB */
STATIC void DosData(PCStr p)
{
  if (!strcasediff(p, "UMB"))
    Config.cfgDosDataUmb = TRUE;
  else
    CfgFailure(p);
}

/* Format: SWITCHAR [=] character */
STATIC void CfgSwitchar(PCStr p)
{
  if (*p == '\0' || p[1])
  {
    /* no character or garbage at line end */
    CfgFailure(p);
    return;
  }
  init_switchar(*p);
}

/* Format: SWITCHES [=] { /K | /N | /F | /E[[:]nnn] } */
STATIC void CfgSwitches(PCStr p)
{
  do
  {
    if (*p != '/')
    {
      if (nPass)
        CfgFailure(p);
      return;
    }
    switch(toupper(*++p))
    {
      case 'K':
        if (nPass)
          kbdType = 0; /* force conv keyb */
        p++;
        break;
      case 'N':
        InitKernelConfig.SkipConfigSeconds = -1;
        p++;
        break;
      case 'F':
        InitKernelConfig.SkipConfigSeconds = 0;
        p++;
        break;
      case 'E': /* /E[[:]nnnn]  Set the desired EBDA amount to move in bytes */
                /* Note that if there is no EBDA, this will have no effect */
        if (*++p == ':')
          p++;                  /* skip optional separator */
        if (isdigit(*p))
        {
          p = GetNumArg(p);
          if (p == NULL)
            return;
          /* allowed values: [48..1024] bytes, multiples of 16
           * e.g. AwardBIOS: 48, AMIBIOS: 1024
           * (Phoenix, MRBIOS, Unicore = ????)
           */
          if (nPass)
            Config.ebda2move = numarg;
        }
        break;
      default:
        if (nPass)
          CfgFailure(p);
	return;
    } /* switch */
    p = skipwh(p);
  } while (*p);
}

/* Format: FCBS [=] totalFcbs [, protectedFcbs] */
STATIC void Fcbs(PCStr p)
{
  if (GetNumArg2(p, Config.cfgProtFcbs))
  {
    UBYTE fcbs = (UBYTE)numarg1, prot = (UBYTE)numarg;
    Config.cfgFcbs = fcbs;
    if (prot > fcbs)
        prot = fcbs;
    Config.cfgProtFcbs = prot;
  }
}

/*      LoadCountryInfo():
 *      Searches a file in the COUNTRY.SYS format for an entry
 *      matching the specified code page and country code, and loads
 *      the corresponding information into memory. If code page is 0,
 *      the default code page for the country will be used.
 */

STATIC void LoadCountryInfo(PCStr filename, int ccode, int cpage)
{
  /* COUNTRY.SYS file data structures - see RBIL tables 2619-2622 */

  struct {      /* file header */
    char name[8];       /* "\377COUNTRY.SYS" */
    char reserved[11];
    ULONG offset;       /* offset of first entry in file */
  } header;
  struct {      /* entry */
    int length;         /* length of entry, not counting this word, = 12 */
    int country;        /* country ID */
    int codepage;       /* codepage ID */
    int reserved[2];
    ULONG offset;       /* offset of country-subfunction-header in file */
  } entry;
  struct subf_hdr { /* subfunction header */
    int length;         /* length of entry, not counting this word, = 6 */
    int id;             /* subfunction ID */
    ULONG offset;       /* offset within file of subfunction data entry */
  };
  static struct {   /* subfunction data */
    char signature[8];  /* \377CTYINFO|UCASE|LCASE|FUCASE|FCHAR|COLLATE|DBCS */
    int length;         /* length of following table in bytes */
    UBYTE buffer[256];
  } subf_data;
  struct subf_tbl {
    char sig[8];        /* signature for each subfunction data */
    void FAR *p;        /* pointer to data in nls_hc.asm to be copied to */
  };
  static struct subf_tbl table[8] = {
    {"\377       ", NULL},                  /* 0, unused */
    {"\377CTYINFO", &nlsCntryInfoHardcoded},/* 1 */
    {"\377UCASE  ", &nlsUpcaseHardcoded},   /* 2 */
    {"\377LCASE  ", NULL},                  /* 3, not supported [yet] */
    {"\377FUCASE ", &nlsFUpcaseHardcoded},  /* 4 */
    {"\377FCHAR  ", &nlsFnameTermHardcoded},/* 5 */
    {"\377COLLATE", &nlsCollHardcoded},     /* 6 */
    {"\377DBCS   ", &nlsDBCSHardcoded}      /* 7, not supported [yet] */
  };
  static struct subf_hdr hdr[8];
  int fd, entries, count, i;

  if ((fd = open(filename, 0)) < 0)
  {
    printf("%s not found\n", filename);
    return;
  }
  if (read(fd, &header, sizeof(header)) != sizeof(header))
  {
    printf("Error reading %s\n", filename);
    goto ret;
  }
  if (memcmp(header.name, "\377COUNTRY", sizeof(header.name)))
  {
err:printf("%s has invalid format\n", filename);
    goto ret;
  }
  if (lseek(fd, header.offset) == 0xffffffffL
    || read(fd, &entries, sizeof(entries)) != sizeof(entries))
    goto err;
  for (i = 0; i < entries; i++)
  {
    if (read(fd, &entry, sizeof(entry)) != sizeof(entry) || entry.length != 12)
      goto err;
    if (entry.country != ccode || entry.codepage != cpage && cpage)
      continue;
    if (lseek(fd, entry.offset) == 0xffffffffL
      || read(fd, &count, sizeof(count)) != sizeof(count)
      || count > LENGTH(hdr)
      || read(fd, &hdr, sizeof(struct subf_hdr) * count)
                      != sizeof(struct subf_hdr) * count)
      goto err;
    for (i = 0; i < count; i++)
    {
      if (hdr[i].length != 6)
        goto err;
      if (hdr[i].id < 1 || hdr[i].id > 6 || hdr[i].id == 3)
        continue;
      if (lseek(fd, hdr[i].offset) == 0xffffffffL
       || read(fd, &subf_data, 10) != 10
       || memcmp(subf_data.signature, table[hdr[i].id].sig, 8) && (hdr[i].id !=4
       || memcmp(subf_data.signature, table[2].sig, 8))  /* UCASE for FUCASE ^*/
       || read(fd, subf_data.buffer, subf_data.length) != subf_data.length)
        goto err;
      if (hdr[i].id == 1)
      {
        if (((struct CountrySpecificInfo *)subf_data.buffer)->CountryID
                                                     != entry.country
         || ((struct CountrySpecificInfo *)subf_data.buffer)->CodePage
                                                     != entry.codepage
         && cpage)
          continue;
        nlsPackageHardcoded.cntry = entry.country;
        nlsPackageHardcoded.cp = entry.codepage;
        subf_data.length =      /* MS-DOS "CTYINFO" is up to 38 bytes */
                min(subf_data.length, sizeof(struct CountrySpecificInfo));
      }
      fmemcpy((BYTE FAR *)(table[hdr[i].id].p) + 2, subf_data.buffer,
                                /* skip length ^*/  subf_data.length);
    }
    goto ret;
  }
  printf("couldn't find info for country ID %u\n", ccode);
ret:
  close(fd);
}

/* Format: COUNTRY [=] countryCode [, [codePage] [, filename]]   */
STATIC void Country(PCStr p)
{
  int ccode;
  PCStr filename = "\\COUNTRY.SYS";

  p = GetNumArg(p);
  if (p == NULL)
    return;

  ccode = numarg;
  numarg = 0;
  p = skipwh(p);
  if (*p == ',')
  {
    p = skipwh(p + 1);
    if (*p != ',')
    {
      p = GetNumArg(p);
      if (p == NULL)
        return;
      p = skipwh(p);
    }
    if (*p == ',')
      filename = p + 1;
  }

  if (*p && *p != ',') /* garbage at line end? */
  {
    CfgFailure(p);
    return;
  }

  LoadCountryInfo(filename, ccode, numarg);
}

/* Format: STACKS [=] stacks [, stackSize] */
/* Format: STACKSHIGH [=] stacks [, stackSize] */
static void _Stacks(PCStr p, UBYTE high)
{
  if (GetNumArg2(p, Config.cfgStackSize))
  {
    UBYTE stacks = (UBYTE)numarg1;
    UWORD sz = numarg;
    if (stacks > 64)
        stacks = 64;
    if (sz < 32)
        sz = 32;
    if (sz > 512)
        sz = 512;
    Config.cfgStacks = stacks;
    Config.cfgStackSize = sz;
    Config.cfgStacksHigh = high;
  }
}

STATIC void Stacks(PCStr p)		{ _Stacks(p, 0); }

STATIC void StacksHigh(PCStr p)		{ _Stacks(p, 1); }

/* Format: SHELL [=] command */
STATIC void CmdShell(PCStr p)
{
  Config.cfgP_0_startmode = 0;
  /* assume strlen(p)+1 <= sizeof Config.cfgShell */
  strcpy(Config.cfgShell, p);
}

/* Format: SHELLHIGH [=] command */
STATIC void CmdShellHigh(PCStr p)
{
  Config.cfgP_0_startmode = 0x80;
  /* assume strlen(p)+1 <= sizeof Config.cfgShell */
  strcpy(Config.cfgShell, p);
}

/* Format: BREAK [=] (ON | OFF) */
STATIC void CfgBreak(PCStr p)
{
  if (!strcasediff(p, "ON"))
    break_ena = 1;
  else if (!strcasediff(p, "OFF"))
    break_ena = 0;
  else
    CfgFailure(p);
}

/* Format: NUMLOCK [=] (ON | OFF) */
STATIC void Numlock(PCStr p)
{
  UBYTE FAR *keyflags = MK_PTR(UBYTE, 0, 0x417);
  if (!strcasediff(p, "ON"))
    *keyflags |= 32;
  else if (!strcasediff(p, "OFF"))
    *keyflags &= ~32;
  else
  {
    CfgFailure(p);
    return;
  }
  keycheck();
}

/* Format: DEVICEHIGH [=] command */
STATIC void DeviceHigh(PCStr p)
{
  if (UmbState != UMB_DONE || /* UMB not initialized? */
      LoadDevice(p, MK_SEG_PTR(void, mcb_next(umb_base_start)), TRUE) == DE_NOMEM)
    Device(p);
}

/* Format: DEVICE [=] command */
STATIC void Device(PCStr p)
{
  LoadDevice(p, lpTop, FALSE);
}

STATIC BOOL LoadDevice(PCStr p, VFP top, int mode)
{
  int ret;

  seg_t base = base_seg;
  seg_t start = LoL->first_mcb;
  if (mode)
  {
    base = umb_base_seg;
    start = umb_base_start;
  }

  if (base == start)
    base++;
  base++;

  /* Get the device driver name                                   */
  {
    PStr d = szBuf;
    PCStr s = p;
    for (; (UBYTE)*s > ' '; d++, s++)
      *d = *s;
    *d = '\0';
  }

  /* The driver is loaded at the top of allocated memory.         */
  /* The device driver is paragraph aligned.                      */
  {
    exec_blk eb;
    eb.load.reloc = eb.load.load_seg = base;

#ifdef DEBUG
    printf("Loading device driver %s at segment %04x\n", szBuf, base);
#endif

    ret = init_DosExec(3, &eb, szBuf);
    if (ret != SUCCESS)
    {
      CfgFailure(p);
      return ret;
    }
  }

  strcpy(szBuf, p);
  /* uppercase the device driver command */
  /* add \r\n to the command line */
  strcat(strupr(szBuf), " \r\n");

  /* TE this fixes the loading of devices drivers with
     multiple devices in it. NUMEGA's SoftIce is such a beast
   */

  /* NOTE - Modification for multisegmented device drivers:          */
  /*   In order to emulate the functionallity experienced with other */
  /*   DOS operating systems, the original 'top' end address is      */
  /*   updated with the end address returned from the INIT request.  */
  /*   The updated end address is then used when issuing the next    */
  /*   INIT request for the following device driver within the file  */

  {
    ofs_t next = 0;
    do
    {
      struct dhdr FAR *dhp = MK_PTR(struct dhdr, base, next);
      /* init_device returns FALSE on SUCCESS, TRUE otherwise */
      ret = init_device(dhp, szBuf, mode, &top);
      if (ret) break;

      next = FP_OFF(dhp->dh_next);

      /* Link in device driver and save LoL->nul_dev pointer to next */
      dhp->dh_next = LoL->nul_dev.dh_next;
      LoL->nul_dev.dh_next = dhp;
    } while (next != 0xffff);
  }

  /* might have been the UMB driver -> try UMB initialization */
  umb_init();

  return ret;
}

STATIC void CfgFailure(PCStr p)
{
  printf("Error in %s line %d:\n"
         "%s\n", configfile, nCfgLine, szLine);
  gotoxy(p - szLine, screeny());
  say("^\n");
}

struct submcb
{
  char type;
  unsigned short start;
  unsigned short size;
  char unused[3];
  char name[8];
};

void _seg * KernelAllocPara(size_t nPara, UBYTE type, CStr name, int mode)
{
  seg_t base, start;
  struct submcb _seg *p;

  if (mode && UmbState == UMB_DONE)
  {
    base = umb_base_seg;
    start = umb_base_start;
  }
  else
  {
    mode = 0;
    base = base_seg;
    start = LoL->first_mcb;
  }

  /* create the special DOS data MCB if it doesn't exist yet */
  DebugPrintf(("kernelallocpara: %x %x %x %c %d\n", start, base, nPara, type, mode));

  if (base == start)
  {
    mcb _seg *p = MK_SEG_PTR(mcb, base);
    mcb_init(++base, p->m_size, p->m_type);
    mumcb_init(FP_SEG(p), 1);
    p->m_name[1] = 'D';
  }

  nPara++;
  mcb_init(base + nPara, MK_SEG_PTR(mcb, base)->m_size - nPara + 1,
                         MK_SEG_PTR(mcb, base)->m_type);
  MK_SEG_PTR(mcb, start)->m_size += nPara;

  p = MK_SEG_PTR(struct submcb, base);
  p->type = type;
  p->start = FP_SEG(p)+1;
  p->size = nPara - 1;
  if (name)
    fmemcpy(p->name, name, 8);
  base += nPara;
  if (mode)
    umb_base_seg = base;
  else
    base_seg = base;
  return MK_SEG_PTR(void, FP_SEG(p) + 1);
}

void _seg * KernelAlloc(size_t nBytes, UBYTE type, int mode)
{
  void _seg *p;
  size_t nPara = (nBytes + 15) / 16;

  if (LoL->first_mcb == 0)
  {
    /* prealloc */
    /* note: lpTop is already para-aligned */
    p = alignNextPara(lpTop = MK_FP(FP_SEG(lpTop) - nPara, FP_OFF(lpTop)));
  }
  else
  {
    p = KernelAllocPara(nPara, type, NULL, mode);
  }
  
  fmemset(p, 0x0, nBytes);
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

void _seg * alignNextPara(CVFP p)
{
  /* First, convert the segmented pointer to linear address       */
  seg_t seg = FP_OFF(p);
  if (seg)
    seg = (seg - 1) / 16 + 1;
  seg += FP_SEG(p);

  /* return an address adjusted to the nearest paragraph boundary */
  return MK_SEG_PTR(void, seg);
}
#endif

STATIC PCStr skipwh(PCStr s)
{
  s--;
  do
    s++;
  while (*s == ' ' || *s == '\t');
  return s;
}

STATIC PCStr scanword(PCStr s, PStr d)
{
  s = skipwh(s);
  while (*s >= 'a' && *s <= 'z' ||
         *s >= 'A' && *s <= 'Z')
    *d++ = *s++;
  *d = '\0';
  return s;
}

STATIC PCStr scanverb(PCStr s, PStr d)
{
  askCommand &= ~(ASK_ASK | ASK_NOASK);
  line_choices = 0xffff;	/* statement in all menus	*/

  for (;; s++)
  {
    s = skipwh(s);

    if (*s == '!')		/* "!dos" ?			*/
      askCommand |= ASK_NOASK;

    else if (*s == '?')		/* "?device" ?			*/
      askCommand |= ASK_ASK;

    else
    {
      UBYTE ch = *s - (UBYTE)'0';
      if (ch <= 9)		/* "123?device" ?		*/
      {
        PCStr p = s;
        unsigned digits = 0;
        do
        {
          digits |= 1 << ch;
          ch = *++p - (UBYTE)'0';
        } while (ch <= 9);

        if (*p != '?')
          break;

        s = p;
        line_choices = digits;
        all_choices |= digits;
      }
      else
        break;
    } /* else */
  } /* for */

  s = scanword(s, d);

  if (*s == '?')		/* "device?" ?			*/
  {
    askCommand |= ASK_ASK;
    s++;
  }
  return s;
}

/* Yet another change for true portability (PJV) */
STATIC char toupper(char c)
{
  if (c >= 'a' && c <= 'z')
    c -= 'a' - 'A';
  return c;
}

/* Convert string s to uppercase */
static PStr strupr(PStr s)
{
  PStr d = s;
  for (;; d++)
  {
    char ch = *d;
    if (ch == '\0')
      break;
    *d = toupper(ch);
  }
  return s;
}

/* The following code is 8086 dependant                         */

#if 1                           /* ifdef KERNEL */
STATIC void mcb_init_copy(seg_t seg, size_t size, mcb *near_mcb)
{
  near_mcb->m_size = size - 1;
  fmemcpy(MK_SEG_PTR(mcb, seg), near_mcb, sizeof(mcb));
}

STATIC void mcb_init(seg_t seg, size_t size, BYTE type)
{
  static mcb near_mcb BSS_INIT({0});
  near_mcb.m_type = type;
  mcb_init_copy(seg, size, &near_mcb);
}

STATIC void mumcb_init(seg_t seg, size_t size)
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

static PStr strcat(PStr d, PCStr s)
{
  strcpy(d + strlen(d), s);
  return d;
}

/* compare two ASCII strings ignoring case */
STATIC char strcasediff(PCStr d, PCStr s)
{
  while (toupper(*s) == toupper(*d))
  {
    if (*s == '\0')
      return 0;
    s++, d++;
  }
  return 1;
}

/*
    moved from BLOCKIO.C here.
    that saves some relocation problems
*/

STATIC void config_init_buffers(int wantedbuffers)
{
  struct buffer FAR *pbuffer;
  unsigned buffers = 0;

  /* fill HMA with buffers if BUFFERS count >=0 and DOS in HMA          */
  if (wantedbuffers < 0)
    wantedbuffers = -wantedbuffers;
  else if (HMAState == HMA_DONE)
    buffers = (0xfff0 - HMAFree) / sizeof(struct buffer);

  if (wantedbuffers < 6)                /* min 6 buffers                */
    wantedbuffers = 6;
  if (wantedbuffers > 99)               /* max 99 buffers               */
  {
    printf("BUFFERS=%u not supported, reducing to 99\n", wantedbuffers);
    wantedbuffers = 99;
  }
  if (buffers < wantedbuffers)          /* not less than requested      */
    buffers = wantedbuffers;

  LoL->nbuffers = buffers;
  LoL->inforecptr = &LoL->firstbuf;
  {
    size_t bytes = sizeof(struct buffer) * buffers;
    if ((pbuffer = HMAalloc(bytes)) == NULL)
    {
      pbuffer = KernelAlloc(bytes, 'B', 0);
      if (HMAState == HMA_DONE)
        firstAvailableBuf = MK_FP(0xffff, HMAFree);
    }
    else
    {
      LoL->bufloc = LOC_HMA;

      /* space in HMA beyond requested buffers available as user space  */
      firstAvailableBuf = pbuffer + wantedbuffers;
    }
  }
  LoL->deblock_buf = DiskTransferBuffer;
  LoL->firstbuf = pbuffer;

  DebugPrintf(("init_buffers (size %u) at (%p)",
               sizeof(struct buffer), pbuffer));

  buffers--;
  pbuffer->b_prev = FP_OFF(pbuffer + buffers);
  {
    int i = buffers;
    do
    {
      pbuffer->b_next = FP_OFF(pbuffer + 1);
      pbuffer++;
      pbuffer->b_prev = FP_OFF(pbuffer - 1);
    } while (--i);
  }
  pbuffer->b_next = FP_OFF(pbuffer - buffers);

  DebugPrintf((" done\n"));

  if (FP_SEG(pbuffer) == /*HMASEG*/0xFFFF)
  {
    buffers++;
    printf("Kernel: allocated %d Diskbuffers = %u Bytes in HMA\n",
           buffers, buffers * sizeof(struct buffer));
  }
}

STATIC void config_init_fnodes(int f_nodes_cnt)
{
  struct f_node FAR *p;
  size_t bytes;

  /* number of allocated files */
  LoL->f_nodes_cnt = f_nodes_cnt;
  bytes = f_nodes_cnt * sizeof(struct f_node);

  p = HMAalloc(bytes);
  if (p == NULL)
    p = KernelAlloc(bytes, 'F', 0);
  LoL->f_nodes = p;
}

/*
    Undocumented feature:
    Format: ANYDOS [=]
        will report to MSDOS programs just the version number
        they expect. be careful with it!
*/
STATIC void SetAnyDos(PCStr p)
{
  UNREFERENCED_PARAMETER(p);
  ReturnAnyDosVersionExpected = TRUE;
}

/* Format: EECHO string */
STATIC void CfgMenuEsc(PCStr p)
{
  char ch;
  do
  {
    ch = *p; p++;
    if (ch == '\0')
      ch = '\n';
    if (ch == '$')		/* translate $ to ESC */
      ch = ESC;
    printf("%c", ch);
  } while (ch != '\n');
}

/*
  'MENU'ing stuff
  although it's worse then MSDOS's , its better then nothing
  Menu selection bar struct:
  x pos, y pos, string
*/

static struct MenuSelector
{
  unsigned x, y;
  UBYTE len;
  char text[80];
} MenuStruct[10] BSS_INIT({0});

static unsigned nextMenuRow BSS_INIT(0);

/* Format: MENU string */
/* Format: ECHO string */
STATIC void CfgMenu(PCStr p)
{
  struct MenuSelector *menu;
  UBYTE ch;
  unsigned len;
  int spaces;

  say2("%s\n", p);
  nextMenuRow++;

  /* find digit */
  for (len = 0;; p++, len++)
  {
    ch = *p;
    if (ch == '\0')
      return;
    ch -= (UBYTE)'0';
    if (ch <= 9)
      break;
  }

  menu = &MenuStruct[ch];
  menu->x = len; /* start position of digit */
  menu->y = nextMenuRow - 1;

  /* copy menu text (up to null or 10+ spaces) */
  len = 0;
  do
  {
    ch = *p;
    if (ch == '\0')
      break;
    menu->text[len] = ch;
    p++, len++;
    if (ch > ' ')
    {
      menu->len = len;
      spaces = 11;
    }
  } while (--spaces && len < sizeof menu->text - 1);
  menu->text[menu->len] = '\0';
}

STATIC void SelectLine(unsigned i)
{
  struct MenuSelector *menu = MenuStruct;
  do
  {
    if (menu->len)
    {
      UBYTE attr = MenuColor;
      if (i == 0)		/* selected line?		*/
	/* swap colors and clear blinking attribute */
	attr = ((attr << 4) | (attr >> 4)) & 0x7f;

      /* redraw line */
      ClearScreenArea(attr, menu->x, menu->y, menu->len, 1);
      say2("%s", menu->text);
    }
    menu++, i--;
  } while (menu < ENDOF(MenuStruct));
}

static unsigned show_choices(unsigned choicey)
{
  unsigned x;
  UBYTE i;

  gotoxy(2, choicey);
  say("Enter a choice: [");
  x = 19;			/* =strlen("  Enter a choice: [") */
  i = 0;
  do
  {
    if (testbit(all_choices, i))
    {
      if (i < last_choice)
        x++;
      printf("%c", (UBYTE)'0' + i);
    }
    i++;
  } while (i <= 9);
  say("]");
  return x;
}

STATIC VOID DoMenu(void)
{
  unsigned choicey;
  say("\n\n\n");		/* make sure there are 3 free lines */
  choicey = screeny() - 2;

  gotoxy(0, screenbottom());
  say2("F5=Bypass startup files  F8=Confirm each line of %s/AUTOEXEC.BAT",
       configfile);

  for (;;)
  {
    unsigned key;
    UBYTE c;

    gotoxy(75, screenbottom());
    say(askCommand & ASK_TRACE ? "[Y]" : "[N]");

    if (MenuColor && nextMenuRow + 1 == choicey)
      SelectLine(last_choice);	/* invert color of selected line */

    if (MenuTimeout >= 0)
    {
      show_choices(choicey);
      printf("  Time remaining: %d ", MenuTimeout);

      gotoxy(show_choices(choicey), choicey);
      key = GetBiosKey(1);	/* poll keyboard 1 second	*/
      if (key == 0)
      {
        if (MenuTimeout == 0)
          break;		/* timeout, take default	*/
        MenuTimeout--;
        continue;
      }

      MenuTimeout = -1;
      clearrow();		/* clear "Time remaining"	*/
    }

    gotoxy(show_choices(choicey), choicey);
    key = GetBiosKey(-1);	/* remove key from buffer	*/

    if (key == K_F5)		/* F5 */
    {
      askCommand |= ASK_SKIPALL;
      break;
    }
    if (key == K_F8)		/* F8 */
      askCommand ^= ASK_TRACE;

    c = last_choice;

    if (key == K_Up || key == K_Left)
      while (c > 0 && !testbit(all_choices, --c));

    else if (key == K_Down || key == K_Right)
      while (c < 9 && !testbit(all_choices, ++c));

    else
    {
      unsigned k = key;		/* hint for compiler optimizer	*/
      c = (UBYTE)k;
      if (c == '\r')		/* CR - use current choice	*/
        break;
      c -= (UBYTE)'0';
    }

    if (testbit(all_choices, c))
      last_choice = c;
  } /* for */

  gotoxy(0, screenbottom());
  clearrow();			/* clear hint line at bottom	*/
  gotoxy(0, choicey + 2);
}

/* Format: MENUDEFAULT [=] menu [, waitsecs] */
STATIC void CfgMenuDefault(PCStr p)
{
  if (GetNumArg2(p, MenuTimeout))
  {
    last_choice = numarg1;
    /*if (last_choice > 10)
          last_choice = 10;*/
    MenuTimeout = numarg;
  }
}

/* Format: MENUCOLOR [=] foreground [, background] */
STATIC void CfgMenuColor(PCStr p)
{
  if (GetNumArg2(p, 0))
  {
    UBYTE attr = (UBYTE)((numarg << 4) | numarg1);
    if (attr == 0)
        attr = 0x07; /* white on black */
    MenuColor = attr;
  }
}

/* ****************************************************************
** implementation of INSTALL=NANSI.COM /P /X /BLA
*/

STATIC void free(seg_t seg)
{
  iregs r;
  r.ES = seg;
  r.AH = 0x49;			/* free memory */
  init_call_intr(0x21, &r);
}

/* set memory allocation strategy */
STATIC void set_strategy(UBYTE strat)
{
  iregs r;
  r.BL = strat;
  r.AX = 0x5801;
  init_call_intr(0x21, &r);
}

/* Format: INSTALL [=] command */
/* Format: INSTALLHIGH [=] command */
STATIC void _CmdInstall(PCStr p, int mode)
{
  CommandTail args;
  PStr pf;
  unsigned len;
  exec_blk exb;

  for (pf = szBuf;; p++, pf++)
  {
    UBYTE ch = *p;
    if (ch <= ' ' || ch == '/')
      break;
    *pf = ch;
  }
  *pf = '\0';

  len = strlen(p);
  if (len > sizeof args.ctBuffer - 2)
      len = sizeof args.ctBuffer - 2; /* trim too long line */
  args.ctCount = (UBYTE)len;
  args.ctBuffer[len] = '\r';
  args.ctBuffer[len+1] = '\0';
  memcpy(args.ctBuffer, p, len);

  set_strategy(mode);
  exb.exec.env_seg = DOS_PSP + 8;
  exb.exec.cmd_line = &args;
  /*exb.exec.fcb_1 = exb.exec.fcb_2 = NULL;*/ /* unimportant */

  DebugPrintf(("cmd[%s] args [%u,%s]\n", szBuf, args.ctCount, args.ctBuffer));

  if (init_DosExec(mode, &exb, szBuf) != SUCCESS)
    CfgFailure(p);
}

STATIC void CmdInstall(PCStr p)		{ _CmdInstall(p, 0); }

STATIC void CmdInstallHigh(PCStr p)	{ _CmdInstall(p, 0x80); }

VOID DoInstall(void)
{
  seg_t kernel;

  /* grab memory for this install code:
     we are executing somewhere at top of memory and need to protect
     the INIT_CODE from other programs that will be executing soon
  */

  set_strategy(LAST_FIT);
  kernel = allocmem(((unsigned)_init_end + ebda_size + 15) / 16);

  DebugPrintf(("Installing commands now (kernel at %X)\n", kernel));

  DoConfig_();

  set_strategy(FIRST_FIT);
  free(kernel);

  DebugPrintf(("Done with installing commands\n"));
}

/* master_env copied over command line area in
   DOS_PSP, thus its size limited to 128 bytes */
/* static char master_env[128] = "PATH=."; */

  /* !!! dirty hack: because bug in old FreeCOM, which wrongly
     process empty environment in MS-DOS style, garbage empty
     environment by dummy variable: --avb
  */
static PStr envp = master_env + 7; /* sizeof("PATH=.") + 1 zero */

/* Format: SET var = string */
STATIC void CmdSet(PCStr p)
{
  PStr q;
  p = skipwh(scanword(p, szBuf));
  if (*p != '=')		/* equal sign is required	*/
  {
    CfgFailure(p);
    return;
  }

  /* environment variables must be uppercase			*/
  strcat(strupr(szBuf), "=");

  {
    PStr pm = master_env;	/* find duplication		*/
    for (q = pm; pm < envp; q = pm)
    {
      PCStr v = szBuf;
      while (*v == *pm)		/* compare variables		*/
	v++, pm++;
      while (*++pm);		/* find end of definition	*/
      pm++;
      if (*v == '\0')		/* variable found?		*/
      {
	while (pm < envp)
	  *q++ = *pm++;		/* remove duplication		*/
	break;
      }
    } /* for */
  }

  p = skipwh(p + 1);
  if (*p)			/* add new definition?		*/
  {
    size_t sz = strlen(strcat(szBuf, p)) + 1;

    /* environment ends by empty ASCIIZ (one null character) */
    if (sz >= master_env + sizeof master_env - q)
    {
      CfgFailure(p);
      say("Out of environment space\n");
      return;
    }
    strcpy(q, szBuf);
    q += sz;
  }

  envp = q;
  *q = '\0';			/* "add" empty ASCIIZ string	*/
  fmemcpy(MK_PTR(char, DOS_PSP + 8, 0), master_env, q + 1 - master_env);
}
