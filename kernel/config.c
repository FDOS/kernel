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
    "$Id: config.c 1705 2012-02-07 08:10:33Z perditionc $";
#endif

#ifdef DEBUG
#define DebugPrintf(x) printf x
#else
#define DebugPrintf(x)
#endif
#define para2far(seg) ((mcb FAR *)MK_FP((seg), 0))

/**
  Menu selection bar struct:
  x pos, ypos, string
*/
#define MENULINEMAX 80
#define MENULINESMAX 10
struct MenuSelector
{
  int x;
  int y;
  BYTE bSelected;
  BYTE Text[MENULINEMAX];
};

/** Structure below holds the menu-strings */
STATIC struct MenuSelector MenuStruct[MENULINESMAX] BSS_INIT({0});

int nMenuLine BSS_INIT(0);
int MenuColor = -1;

STATIC void WriteMenuLine(struct MenuSelector *menu)
{
  iregs r;
  unsigned char attr = (unsigned char)MenuColor;
  char *pText = menu->Text;

  if (pText[0] == 0)
    return;

  if(menu->bSelected)
    attr = ((attr << 4) | (attr >> 4));

  /* clear line */
  r.a.x   = 0x0600;
  r.b.b.h = attr;
  r.c.b.l = r.d.b.l = menu->x;
  r.c.b.h = r.d.b.h = menu->y;
  r.d.b.l += strlen(pText) - 1;
  init_call_intr(0x10, &r);

  /* set cursor position: */
  r.a.b.h = 0x02;
  r.b.b.h = 0;
  r.d.b.l = menu->x;
  r.d.b.h = menu->y;
  init_call_intr(0x10, &r);

  printf("%s", pText);
}

/* Deselect the previously selected line */
STATIC void DeselectLastLine(void)
{
  struct MenuSelector *menu;
  for (menu = MenuStruct; menu < &MenuStruct[MENULINESMAX]; menu++)
  {
    if (menu->bSelected)
    {
      /* deselect it: */
      menu->bSelected = 0;
      WriteMenuLine(menu);
      break;
    }
  }
}

STATIC void SelectLine(int MenuSelected)
{
  struct MenuSelector *menu;

  DeselectLastLine(); /* clear previous selection */
  menu = &MenuStruct[MenuSelected];
  menu->bSelected = 1;  /* set selection flag for this one */
  WriteMenuLine(menu);
}

UWORD umb_start BSS_INIT(0), UMB_top BSS_INIT(0);
UWORD ram_top BSS_INIT(0); /* How much ram in Kbytes               */
size_t ebda_size BSS_INIT(0);

static UBYTE ErrorAlreadyPrinted[128] BSS_INIT({0});

char master_env[128] BSS_INIT({0});
static char *envp = master_env;

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
  STACKSIZE
      /* COUNTRY= is initialized within DoConfig() */
      , 0                       /* strategy for command.com is low by default */
      , 0                       /* default value for switches=/E:nnnn */
};

STATIC seg base_seg BSS_INIT(0);
STATIC seg umb_base_seg BSS_INIT(0);
BYTE FAR *lpTop BSS_INIT(0);
STATIC unsigned nCfgLine BSS_INIT(0);
COUNT UmbState BSS_INIT(0);
STATIC BYTE szLine[256] BSS_INIT({0});
STATIC BYTE szBuf[256] BSS_INIT({0});

BYTE singleStep BSS_INIT(FALSE);        /* F8 processing */
BYTE SkipAllConfig BSS_INIT(FALSE);     /* F5 processing */
BYTE askThisSingleCommand BSS_INIT(FALSE);      /* ?device=  device?= */
BYTE DontAskThisSingleCommand BSS_INIT(FALSE);  /* !files=            */

COUNT MenuTimeout = -1;
BYTE  MenuSelected BSS_INIT(0);
UCOUNT MenuLine BSS_INIT(0);
UCOUNT Menus BSS_INIT(0);

STATIC VOID CfgMenuColor(BYTE * pLine);

STATIC VOID Config_Buffers(BYTE * pLine);
STATIC VOID CfgBuffersHigh(BYTE * pLine);
STATIC VOID sysScreenMode(BYTE * pLine);
STATIC VOID sysVersion(BYTE * pLine);
STATIC VOID CfgBreak(BYTE * pLine);
STATIC VOID Device(BYTE * pLine);
STATIC VOID DeviceHigh(BYTE * pLine);
STATIC VOID Files(BYTE * pLine);
STATIC VOID FilesHigh(BYTE * pLine);
STATIC VOID Fcbs(BYTE * pLine);
STATIC VOID CfgKeyBuf(BYTE * pLine);
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
STATIC VOID CmdSet(BYTE * pLine);


STATIC VOID CfgSwitchar(BYTE * pLine);
STATIC VOID CfgSwitches(BYTE * pLine);
STATIC VOID CfgFailure(BYTE * pLine);
STATIC VOID CfgIgnore(BYTE * pLine);
STATIC VOID CfgMenu(BYTE * pLine);

STATIC VOID CfgMenuEsc(BYTE * pLine);

STATIC VOID DoMenu(void);
STATIC VOID CfgMenuDefault(BYTE * pLine);
STATIC BYTE * skipwh(BYTE * s);
STATIC int iswh(unsigned char c);
STATIC BYTE * scan(BYTE * s, BYTE * d);
STATIC BOOL isnum(char ch);
#if 0
STATIC COUNT tolower(COUNT c);
#endif
STATIC char toupper(char c);
STATIC VOID strupr(char *s);
STATIC VOID mcb_init(UCOUNT seg, UWORD size, BYTE type);
STATIC VOID mumcb_init(UCOUNT seg, UWORD size);

STATIC VOID Stacks(BYTE * pLine);
STATIC VOID StacksHigh(BYTE * pLine);

STATIC VOID SetAnyDos(BYTE * pLine);
STATIC VOID SetIdleHalt(BYTE * pLine);
STATIC VOID Numlock(BYTE * pLine);
STATIC BYTE * GetNumArg(BYTE * pLine, COUNT * pnArg);
BYTE *GetStringArg(BYTE * pLine, BYTE * pszString);
STATIC int SkipLine(char *pLine);
#if 0
STATIC char * stristr(char *s1, char *s2);
#endif
STATIC char strcaseequal(const char * d, const char * s);
STATIC int LoadCountryInfoHardCoded(COUNT ctryCode);
STATIC void umb_init(void);

void HMAconfig(int finalize);
STATIC void config_init_buffers(int anzBuffers);     /* from BLOCKIO.C */

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
  signed char pass;
  config_sys_func_t *func;
};

STATIC struct table commands[] = {
  /* first = switches! this one is special; some options will
     always be ran, others depends on F5/F8 and ? processing */
  {"SWITCHES", 0, CfgSwitches},

  /* rem is never executed by locking out pass                    */
  {"REM", 0, CfgIgnore},
  {";", 0,   CfgIgnore},

  {"MENUCOLOR",0,CfgMenuColor},

  {"MENUDEFAULT", 0, CfgMenuDefault},
  {"MENU", 0, CfgMenu},         /* lines to print in pass 0 */
  {"ECHO", 2, CfgMenu},         /* lines to print in pass 2 - install(high) */
  {"EECHO", 2, CfgMenuEsc},     /* modified ECHO (ea) */

  {"BREAK", 1, CfgBreak},
  {"BUFFERS", 1, Config_Buffers},
  {"BUFFERSHIGH", 1, CfgBuffersHigh}, /* as BUFFERS - we use HMA anyway */
  {"COMMAND", 1, InitPgm},
  {"COUNTRY", 1, Country},
  {"DOS", 1, Dosmem},
  {"DOSDATA", 1, DosData},
  {"FCBS", 1, Fcbs},
  {"KEYBUF", 1, CfgKeyBuf},	/* ea */
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
  {"IDLEHALT", 1, SetIdleHalt},   /* ea  */

  {"DEVICE", 2, Device},
  {"DEVICEHIGH", 2, DeviceHigh},
  {"INSTALL", 2, CmdInstall},
  {"INSTALLHIGH", 2, CmdInstallHigh},
  {"SET", 2, CmdSet},
  
  /* default action                                               */
  {"", -1, CfgFailure}
};

/* RE function for menu. */
int  findend(BYTE * s)
{
  int nLen = 0;
  /* 'marks' end if at least ten spaces, 0, or newline is found. */
  while (*s && (*s != 0x0d || *s != 0x0a) )
  {
    BYTE *p= skipwh(s);
    /* ah, more than 9 whitespaces ? We're done here (hrmph!) */
    if(p - s >= 10)
      break;
    nLen++;
    ++s;
  }
  return nLen;
}

BYTE *pLineStart BSS_INIT(0);

BYTE HMAState BSS_INIT(0);
#define HMA_NONE 0              /* do nothing */
#define HMA_REQ 1               /* DOS = HIGH detected */
#define HMA_DONE 2              /* Moved kernel to HMA */
#define HMA_LOW 3               /* Definitely LOW */

/* Do first time initialization.  Store last so that we can reset it    */
/* later.                                                               */
void PreConfig(void)
{
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

  LoL->sfthead = MK_FP(FP_SEG(LoL), 0xcc); /* &(LoL->firstsftt) */
  /* LoL->FCBp = (sfttbl FAR *)&FcbSft; */
  /* LoL->FCBp = (sfttbl FAR *)
     KernelAlloc(sizeof(sftheader)
     + Config.cfgFiles * sizeof(sft)); */

  config_init_buffers(Config.cfgBuffers);

  LoL->CDSp = KernelAlloc(sizeof(struct cds) * LoL->lastdrive, 'L', 0);

#ifdef DEBUG
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

  /* initialize NEAR allocated things */

  /* Initialize the base memory pointers from last time.          */
  /*
     if the kernel could be moved to HMA, everything behind the dynamic
     near data is free.
     otherwise, the kernel is moved down - behind the dynamic allocated data,
     and allocation starts after the kernel.
   */

  base_seg = LoL->first_mcb = FP_SEG(AlignParagraph((BYTE FAR *) DynLast() + 0x0f));

  if (Config.ebda2move)
  {
    ebda_size = ebdasize();
    if (ebda_size > Config.ebda2move)
      ebda_size = Config.ebda2move;
    ram_top += ebda_size / 1024;
  }

  /* We expect ram_top as Kbytes, so convert to paragraphs */
  mcb_init(base_seg, ram_top * 64 - LoL->first_mcb - 1, MCB_LAST);

  sp = LoL->sfthead;
  sp = sp->sftt_next = KernelAlloc(sizeof(sftheader) + 3 * sizeof(sft), 'F', 0);
  sp->sftt_next = (sfttbl FAR *) - 1;
  sp->sftt_count = 3;

  if (ebda_size)  /* move the Extended BIOS Data Area from top of RAM here */
    movebda(ebda_size, FP_SEG(KernelAlloc(ebda_size, 'I', 0)));

//  if (UmbState == 2)
//    umb_init();
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

    kernel_seg = allocmem(hma_paras);
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
  void far *xms_addr;

  if ((xms_addr = DetectXMSDriver()) == NULL)
    return;

  if (UMB_get_largest(xms_addr, &umb_seg, &umb_size))
  {
    UmbState = 1;

    /* reset root */
    /* Note: since device drivers can change what is considered top of memory (e.g. move XBDA) we must requery */
    ram_top = init_oem();
    LoL->uppermem_root = ram_top * 64 - 1;

    /* create link mcb (below) */
    para2far(base_seg)->m_type = MCB_NORMAL;
    para2far(base_seg)->m_size--;
    mumcb_init(LoL->uppermem_root, umb_seg - LoL->uppermem_root - 1);

    /* setup the real mcb for the devicehigh block */
    mcb_init(umb_seg, umb_size - 2, MCB_NORMAL); 

    umb_base_seg = umb_max = umb_start = umb_seg;
    UMB_top = umb_size;

    /* there can be more UMBs ! 
       this happens, if memory mapped devces are in between 
       like UMB memory c800..c8ff, d8ff..efff with device at d000..d7ff
       However some of the xxxHIGH commands still only work with
       the first UMB.
    */

    while (UMB_get_largest(xms_addr, &umb_seg, &umb_size))
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

#ifdef MEMDISK_ARGS
struct memdiskinfo {
  UWORD bytes;               /* Total size of this structure, value >= 26 */
  UBYTE version_minor;       /* Memdisk minor version */
  UBYTE version;             /* Memdisk major version */
  UDWORD base;               /* Pointer to disk data in high memory */
  UDWORD size;               /* Size of disk in 512 byte sectors */
  char FAR * cmdline;        /* Command line; currently <= 2047 chars */
  ADDRESS oldint13;          /* Old INT 13h */
  ADDRESS oldint15;          /* Old INT 15h */
  UWORD olddosmem;           /* Amount of DOS memory before Memdisk loaded */
  UBYTE boot_id;             /* major >= 3, boot loader ID */
  UBYTE unused;
  UWORD DPT_offset;          /* >= 3.71, ES based offset to installed DPT, +16 is Old INT 1Eh */
};

/* query_memdisk() based on similar subroutine in Eric Auer's public domain getargs.asm which is based on IFMEMDSK */
struct memdiskinfo FAR * ASMCFUNC query_memdisk(UBYTE drive);

struct memdiskopt {
  BYTE * name;
  UWORD size;
};

/* preprocesses memdisk command line to allow simpler handling
  { is replaced by unsigned offset to start of next config line
  e.g.  "{{HI{HI}{HI{HI" --> "13HI4HI}3HI3HI"
  FreeDOS supports max 256 length config lines, memdisk 4 max command length 2047
*/
BYTE FAR * ProcessMemdiskLine(BYTE FAR *cLine)
{
  BYTE FAR *ptr;
  BYTE FAR *sLine = cLine;
  
  /* skip everything until end of line or starting { */
  for (; *cLine && (*cLine != '{'); ++cLine)
    ;
  sLine = cLine;
    
  for (ptr = cLine; *cLine; ptr = cLine)
  {
    /* skip everything until end of line or starting { */
    for (++cLine; *cLine && (*cLine != '{'); ++cLine)
      ;
     
    /* calc offset from previous { to next { or eol and replace previous { with offset */
    *ptr = (BYTE)(cLine - ptr);
  }

  return sLine;
}

/* Given a pointer to a memdisk command line and buffer will copy the next
   config.sys equivalent line to pLine and return updated cLine.
   Call repeatedly until end of string (*cLine == '\0').
   Each simulated line is indicated be enclosing the line in curly braces {}
   with the end } optional - the next { will indicate end/beginning and
   end of line.  MEMDISK options may appear nearly anywhere on line and are
   ignored - see memdisk_opts for list of recognized options.
*/
BYTE FAR * GetNextMemdiskLine(BYTE FAR *cLine, BYTE *pLine)
{
  STATIC struct memdiskopt memdiskopts[] = {
      {"initrd", 6}, {"BOOT_IMAGE", 10},
      {"floppy", 6}, {"harddisk", 8}, {"iso", 3}, 
      {"nopass", 6}, {"nopassany", 9},
      {"edd", 3}, {"noedd", 5}
/*
      {"c", 1}, {"h", 1}, {"s", 1}, 
      {"raw", 3}, {"bigraw", 6}, {"int", 3}, {"safeint", 7}
*/
  };

  int ws = TRUE;  /* treat start of line same as if whitespace seen */
  BYTE FAR * mf = NULL;   /* where last line split by memdisk option */
  BYTE FAR *ptr = cLine;  /* start of current cfg line, where { was */
  BYTE FAR *sLine = cLine;
  
  /* exit early if already at end of command line */
  if (!*cLine) return cLine;

  /* point to start of next line; terminates current line if no } found before here */
  cLine += *cLine;

  /* restore original character we overwrite with offset, for next iteration of cfg file */
  *ptr = '{'; 
  
  /* ASSERT ptr points to start of line { and cLine points to start of next line { (or eol)*/
  
  /* copy chars to pLine buffer until } or start of next line */  
  for (++ptr; (*ptr != '}') && (ptr < cLine); ++ptr)
  {
    /* if not in last {} then simply copy chars up to } (or next {) */
    if (*cLine) goto copy_char;

    /* otherwise if last character was whitespace (or start of line) check for memdisk option to skip */
    if (ws)
    {
      int i;
      for (i = 0; i < 9; ++i)
      {
        /* compare with option */
        if (fmemcmp(ptr, memdiskopts[i].name, memdiskopts[i].size) == 0) 
        {
          BYTE c = *(ptr + memdiskopts[i].size);
          /* ensure character after is end of line, =, or whitespace */
          if (!c || (c == '=') || iswh(c))
          {
            /* flag this line split */
            mf = ptr;
            
            /* matched option so point past it */
            ptr += memdiskopts[i].size;
            
            /* allow extra whitespace between option and = by skipping it */
            while (iswh(*ptr))
              ++ptr;

            /* if option has = value then skip it as well */
            if (*ptr == '=')
            {
              /* allow extra whitespace between = and value by skipping it */
              while (iswh(*ptr))
                ++ptr;
                
              /* skip past all characters after = */
              for (; (*ptr != '}') && (ptr < cLine) && !iswh(*ptr); ++ptr)
                ;
            }
              
            break; /* memdisk option found, no need to keep check rest in list */
          }
        }
      }
    }
      
    if (ptr < cLine)
    {
      ws = iswh(*ptr);
      
      /* allow replacing X=Y prior to memdisk options with X=Z after */
      /* on 1st pass if find a match we overwrite it with spaces */
      if (mf && (*ptr == '='))
      {
        BYTE FAR *old=sLine, FAR *new;
        /* check for = in command line */
        for (; old < mf; ++old)
        {
          for (; (*old != '=') && (old < mf); ++old)
            ;
          /* ASSERT ptr points to = after memdisk option and old points to = before memdisk option or mf */
          
          /* compare backwards to see if same option */
          for (new = ptr; (old >= sLine) && ((*old & 0xCD) == (*new & 0xCD)); --old, --new)
          {
            if (iswh(*old) || iswh(*new)) break;
          }
          
          /* if match found then overwrite, otherwise skip past the = */
          if (((old <= sLine) || iswh(*old)) && iswh(*new))
          {
            /* match found so overwrite with spaces */
            for(++old; !iswh(*old) && (old < mf); ++old)
              *old = ' '; 
          }
          else
          {
            for (; (*old != '=') && (old < mf); ++old)
              ;
          }
        }
      }
      
copy_char:
      *pLine = *ptr;
      ++pLine;
    }
  }
  *pLine = 0;

  /* return location to begin next scan from */    
  return cLine;
}

#endif


VOID DoConfig(int nPass)
{
  COUNT nFileDesc;
  BYTE *pLine;
  BOOL bEof = FALSE;
  
#ifdef MEMDISK_ARGS
  /* check if MEMDISK used for LoL->BootDrive, if so check for special appended arguments */
  struct memdiskinfo FAR *mdsk = NULL;
  BYTE FAR *cLine;
  /* memdisk check & usage requires 386+, DO NOT invoke if less than 386 */
  if (LoL->cpu >= 3)
  {
    UBYTE drv = (LoL->BootDrive < 3)?0x0:0x80; /* 1=A,2=B,3=C */
    mdsk = query_memdisk(drv);
    if (mdsk != NULL) 
    {
      cLine = ProcessMemdiskLine(mdsk->cmdline);
    }
  }
#endif

  if (nPass==0)
  {
    HaltCpuWhileIdle = 0; /* init to "no HLT while idle" */

#ifdef MEMDISK_ARGS
    if (mdsk != NULL)
    {
      printf("MEMDISK version %u.%02u  (%lu sectors)\n", mdsk->version, mdsk->version_minor, mdsk->size);
      DebugPrintf(("MEMDISK args:{%S}\n", mdsk->cmdline));
    }
    else
    {
      DebugPrintf(("MEMDISK not detected!\n"));
    }
#endif
  }


  /* Check to see if we have a config.sys file.  If not, just     */
  /* exit since we don't force the user to have one (but 1st      */
  /* also process MEMDISK passed config options if present).      */
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
      /* at this point no config file was found, may return early */
#ifdef MEMDISK_ARGS
      /* if memdisk in use then only assume end of file reached and proceed, else return early */
      if (mdsk != NULL)
        bEof = TRUE;
      else
#endif
        return;
    }
    else
    {
      DebugPrintf(("Reading CONFIG.SYS...\n"));
    }
  }

  nCfgLine = 0;  /* keep track of which line in file for errors   */

  /* Read each line into the buffer and then parse the line,      */
  /* do the table lookup and execute the handler for that         */
  /* function.                                                    */

#ifdef MEMDISK_ARGS
  for (; !bEof || (mdsk != NULL); nCfgLine++)
#else
  for (; !bEof; nCfgLine++)
#endif
  {
    struct table *pEntry;
    pLineStart = szLine;

#ifdef MEMDISK_ARGS
    if (!bEof)
    {
#endif
        
    /* read in a single line, \n or ^Z terminated */

    for (pLine = szLine;;)
    {
      if (read(nFileDesc, pLine, 1) == 0)
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

      if (*pLine != '\r')       /* ignore CR */
        pLine++;
    }

    *pLine = 0;
#ifdef MEMDISK_ARGS
    }
    else if (mdsk != NULL)
    {
      cLine = GetNextMemdiskLine(cLine, szLine);
      /* if end of memdisk command line reached, flag done */
      if (!*cLine)
        mdsk = NULL;
    }
#endif

    DebugPrintf(("CONFIG=[%s]\n", szLine));

    /* Skip leading white space and get verb.               */
    pLine = scan(szLine, szBuf);

    /* If the line was blank, skip it.  Otherwise, look up  */
    /* the verb and execute the appropriate function.       */
    if (*szBuf == '\0')
      continue;

    pEntry = LookUp(commands, szBuf);

	/* should config command be executed on this pass? */
    if (pEntry->pass >= 0 && pEntry->pass != nPass)
      continue;
    
	/* pass 0 always executed (rem Menu prompt switches) */
    if (nPass == 0) 
    {
      pEntry->func(pLine);
      continue;
    }
    else
    {
      if (SkipLine(pLineStart))   /* F5/F8/?/! processing */
        continue;
    }

    if ((pEntry->func != CfgMenu) && (pEntry->func != CfgMenuEsc))
    {
      /* compatibility "device foo.sys" */
      if (' ' != *pLine && '\t' != *pLine && '=' != *pLine)
      {
        CfgFailure(pLine);
        continue;
      }
      pLine = skipwh(pLine);
    }
    if ('=' == *pLine || pEntry->func == CfgMenu || pEntry->func == CfgMenuEsc)
      pLine = skipwh(pLine+1);

    /* YES. DO IT */
    pEntry->func(pLine);
  }
  close(nFileDesc); 

  if (nPass == 0)
  {
    DoMenu();
  }
}

STATIC struct table * LookUp(struct table *p, BYTE * token)
{
  while (p->entry[0] != '\0' && !strcaseequal(p->entry, token))
    ++p;
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
#define GetBiosTime() peekl(0, 0x46c)

UWORD GetBiosKey(int timeout)
{
  iregs r;

  ULONG startTime = GetBiosTime();

  if (timeout >= 0)
  {
    do
    {
      /* optionally HLT here - timer will IRQ even if no keypress */
      r.a.x = 0x0100;             /* are there keys available ? */
      init_call_intr(0x16, &r);
      if (!(r.flags & FLG_ZERO)) {
        r.a.x = 0x0000;
        init_call_intr(0x16, &r); /* there is a key, so better fetch it! */
        return r.a.x;
      }
    } while ((unsigned)(GetBiosTime() - startTime) < timeout * 18u);
    return 0xffff;
  }

  /* blocking wait (timeout < 0): fetch it */
#if 0
  do {
      /* optionally HLT here */
      r.a.x = 0x0100;
      init_call_intr(0x16, &r);
  } while (r.flags & FLG_ZERO);
#endif
  r.a.x = 0x0000;
  init_call_intr(0x16, &r);
  return r.a.x;
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
    else if (key == 0x4200)     /* F8 */
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

/* JPP - changed so will accept hex number. */
/* ea - changed to accept hex digits in hex numbers */
STATIC char *GetNumArg(char *p, int *num)
{
  static char digits[] = "0123456789ABCDEF";
  unsigned char base = 10;
  int sign = 1;
  int n = 0;

  /* look for NUMBER                               */
  p = skipwh(p);
  if (*p == '-')
  {
    p++;
    sign = -1;
  }
  else if (!isnum(*p))
  {
    CfgFailure(p);
    return NULL;
  }

  for( ; *p; p++)
  {
    char ch = toupper(*p);
    if (ch == 'X')
      base = 16;
    else
    {
      char *q = strchr(digits, ch);
      if (q == NULL)
        break;
      n = n * base + (q - digits);
    }
  }
  *num = n * sign;
  return p;
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
  if (GetNumArg(pLine, &nBuffers))
    Config.cfgBuffers = nBuffers;
}

STATIC void CfgBuffersHigh(BYTE * pLine)
{
  Config_Buffers(pLine);
  printf("Note: BUFFERS will be in HMA or low RAM, not in UMB\n");
}

/**
  Set screen mode - rewritten to use init_call_intr() by RE / ICD
*/
STATIC VOID sysScreenMode(BYTE * pLine)
{
  iregs r;
  COUNT nMode;
  COUNT nFunc = 0x11;

  /* Get the argument                                             */
  if (GetNumArg(pLine, &nMode) == (BYTE *) 0)
    return;

  if(nMode<0x10)
    nFunc = 0; /* set lower screenmode */
  else if ((nMode != 0x11) && (nMode != 0x12) && (nMode != 0x14))
    return; /* do nothing; invalid screenmode */

/* Modes
   0x11 (17)   28 lines
   0x12 (18)   43/50 lines
   0x14 (20)   25 lines
 */
  /* move cursor to pos 0,0: */
  r.a.b.h = nFunc; /* set videomode */
  r.a.b.l = nMode;
  r.b.b.l = 0;
  init_call_intr(0x10, &r);
}

STATIC VOID sysVersion(BYTE * pLine)
{
  COUNT major, minor;
  char *p = strchr(pLine, '.');

  if (p == NULL)
    return;

  p++;

  /* Get major number */
  if (GetNumArg(pLine, &major) == (BYTE *) 0)
    return;

  /* Get minor number */
  if (GetNumArg(p, &minor) == (BYTE *) 0)
    return;

  printf("Changing reported version to %d.%d\n", major, minor);

  LoL->os_setver_major = major; /* not the internal os_major */
  LoL->os_setver_minor = minor; /* not the internal os_minor */
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
  drv = toupper(*pLine);

  if (drv < 'A' || drv > 'Z')
  {
    CfgFailure(pLine);
    return;
  }
  drv -= 'A' - 1;               /* Make real number */
  if (drv > Config.cfgLastdrive)
    Config.cfgLastdrive = drv;
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

  strupr(szBuf);

  /* printf("DOS called with %s\n", szBuf); */

  for (pTmp = szBuf;;)
  {
    if (memcmp(pTmp, "UMB", 3) == 0)
    {
      UMBwanted = TRUE;
      pTmp += 3;
    }
    if (memcmp(pTmp, "HIGH", 4) == 0)
    {
      HMAState = HMA_REQ;
      pTmp += 4;
    }
/*        if (memcmp(pTmp, "CLAIMINIT",9) == 0) { INITDataSegmentClaimed = 0; pTmp += 9; }*/
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
  pLine = GetStringArg(pLine, szBuf);
  strupr(szBuf);

  if (memcmp(szBuf, "UMB", 3) == 0)
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
  if (*pLine == '=')
  {
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
          int n = 0;
          if (*++pLine == ':')
            pLine++;                    /* skip optional separator */
          if (!(isnum(*pLine) || (*pLine == '-')))
          {
            pLine--;
            break;
          }
          pLine = GetNumArg(pLine, &n) - 1;
          /* allowed values: [48..1024] bytes, multiples of 16
           * e.g. AwardBIOS: 48, AMIBIOS: 1024
           * (Phoenix, MRBIOS, Unicore = ????)
           */
          if (n == -1)
          {
            Config.ebda2move = 0xffff;
            break;
          }
          else if (n >= 48 && n <= 1024)
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

/* 
   Keyboard buffer relocation: KEYBUF=start[,end]
   Select a new location for the  keyboard buffer  at 0x40:xx,
   for example 0x40:0xac-0xff, but 0x50:5-0xff ("basica" only?)
   feels safer? 0x60:0-0xff is scratch, we use it as SHELL PSP.
   (sys / boot sector load_segment / LOADSEG, exeflat call in
   makefile, DOS_PSP in mcb.h, main.c P_0, task.c, kernel.asm)
   (50:e0..ff used as early kernel boot drive / config buffer)
*/
STATIC VOID CfgKeyBuf(BYTE * pLine)
{
  /*  Format:     KEYBUF = startoffset [,endoffset]    */
  UWORD FAR *keyfill = (UWORD FAR *) MK_FP(0x40, 0x1a);
  UWORD FAR *keyrange = (UWORD FAR *) MK_FP(0x40, 0x80);
  COUNT startbuf, endbuf;

  if ((pLine = GetNumArg(pLine, &startbuf)) == 0)
    return;
  pLine = skipwh(pLine);
  endbuf = (startbuf | 0xff)+1;	/* default end: end of the same "page" */
  if (*pLine == ',')
  {
    if ((pLine = GetNumArg(++pLine, &endbuf)) == 0)
      return;
  }
  startbuf &= 0xfffe;
  endbuf &= 0xfffe;
  if (endbuf<startbuf || (endbuf-startbuf)<=0x20 ||
    ((startbuf & 0xff00) != ((endbuf-1) & 0xff00)) )
    startbuf = 0;		/* flag as bad: too small or page wrap */
  if (startbuf<0xac || (startbuf>=0x100 && startbuf<0x105) || startbuf>0x1de)
  {				/* 50:0 / 50:4 are for prtscr / A:/B: DJ */
    printf("Must start at 0xac..0x1de, not 0x100..0x104\n");
    return;
  }
  keyfill[0] = startbuf;
  keyfill[1] = startbuf;
  keyrange[0] = startbuf;
  keyrange[1] = endbuf;
  keycheck();
}

/*      LoadCountryInfo():
 *      Searches a file in the COUNTRY.SYS format for an entry
 *      matching the specified code page and country code, and loads
 *      the corresponding information into memory. If code page is 0,
 *      the default code page for the country will be used.
 *
 *      Returns TRUE if successful, FALSE if not.
 */
STATIC BOOL LoadCountryInfo(char *filenam, UWORD ctryCode, UWORD codePage)
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
  static int entries, count;
  int fd, i;
  char *filename = filenam == NULL ? "\\COUNTRY.SYS" : filenam;
  BOOL rc = FALSE;

  if ((fd = open(filename, 0)) < 0)
  {
    if (filenam == NULL)
      return !LoadCountryInfoHardCoded(ctryCode);
    printf("%s not found\n", filename);
    return rc;
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
    if (entry.country != ctryCode || entry.codepage != codePage && codePage)
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
      if (hdr[i].id < 1 || hdr[i].id > 7 || hdr[i].id == 3)
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
         && codePage)
          continue;
        nlsPackageHardcoded.cntry = entry.country;
        nlsPackageHardcoded.cp = entry.codepage;
        subf_data.length =      /* MS-DOS "CTYINFO" is up to 38 bytes */
                min(subf_data.length, sizeof(struct CountrySpecificInfo));
      }
      if (hdr[i].id == 7)
      {
        if (subf_data.length == 0)
        {
          /* if DBCS table (in country.sys) is empty, clear internal table */
          *(DWORD *)(subf_data.buffer) = 0L;
          fmemcpy((BYTE FAR *)(table[hdr[i].id].p), subf_data.buffer, 4);
        }
        else
        {
          fmemcpy((BYTE FAR *)(table[hdr[i].id].p) + 2, subf_data.buffer, subf_data.length);
          /* write length */
          *(UWORD *)(subf_data.buffer) = subf_data.length;
          fmemcpy((BYTE FAR *)(table[hdr[i].id].p), subf_data.buffer, 2);
        }
        continue;
      }
      
      fmemcpy((BYTE FAR *)(table[hdr[i].id].p) + 2, subf_data.buffer,
                                /* skip length ^*/  subf_data.length);
    }
    rc = TRUE;
    goto ret;
  }
  printf("could not find country info for country ID %u\n", ctryCode);
ret:
  close(fd);
  return rc;
}

STATIC VOID Country(BYTE * pLine)
{
  /* Format: COUNTRY = countryCode, [codePage], filename   */
  COUNT ctryCode;
  COUNT codePage = 0;
  char  *filename = NULL;

  if ((pLine = GetNumArg(pLine, &ctryCode)) == 0)
    goto error;

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
      
  if (LoadCountryInfo(filename, ctryCode, codePage))
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
  static char init[NAMEMAX];
  static char inittail[NAMEMAX];

  Config.cfgInit = init;
  Config.cfgInitTail = inittail;

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
  break_ena = strcaseequal(szBuf, "OFF") ? 0 : 1;
}

STATIC VOID Numlock(BYTE * pLine)
{
  /* Format:      NUMLOCK = (ON | OFF)      */
  BYTE FAR *keyflags = (BYTE FAR *) MK_FP(0x40, 0x17);

  GetStringArg(pLine, szBuf);

  *keyflags &= ~32;
  if (!strcaseequal(szBuf, "OFF")) *keyflags |= 32;
  keycheck();
}

STATIC VOID DeviceHigh(BYTE * pLine)
{
  if (UmbState == 1)
  {
    if (LoadDevice(pLine, MK_FP(umb_start + UMB_top, 0), TRUE) == DE_NOMEM)
    {
      printf("Not enough free memory in UMBs: loading low\n");
      LoadDevice(pLine, lpTop, FALSE);
    }
  }
  else
  {
    printf("UMBs unavailable!\n");
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
  strupr(szBuf);

  /* TE this fixes the loading of devices drivers with
     multiple devices in it. NUMEGA's SoftIce is such a beast
   */

  /* add \r\n to the command line */
  strcat(szBuf, " \r\n");

  dhp = MK_FP(base, 0);

  /* NOTE - Modification for multisegmented device drivers:          */
  /*   In order to emulate the functionallity experienced with other */
  /*   DOS operating systems, the original 'top' end address is      */
  /*   updated with the end address returned from the INIT request.  */
  /*   The updated end address is then used when issuing the next    */
  /*   INIT request for the following device driver within the file  */

  for (next_dhp = NULL; FP_OFF(next_dhp) != 0xffff &&
       (result = init_device(dhp, szBuf, mode, &top)) == SUCCESS;
       dhp = next_dhp)
  {
    next_dhp = MK_FP(FP_SEG(dhp), FP_OFF(dhp->dh_next));
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

  /* if no umb available force low allocation */
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
    p = AlignParagraph(lpTop);
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

STATIC int iswh(unsigned char c)
{
  return (c == '\r' || c == '\n' || c == '\t' || c == ' ');
}

STATIC BYTE * skipwh(BYTE * s)
{
  while (iswh(*s))
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
    while (*s && !iswh(*s) && *s != '=')
    {
      if (*s == '?')
        askThisSingleCommand = TRUE;
      else
        *d++ = *s;
      s++;
    }
  *d = '\0';
  return s;
}

STATIC BOOL isnum(char ch)
{
  return (ch >= '0' && ch <= '9');
}

/* Yet another change for true portability (PJV) */
STATIC char toupper(char c)
{
  if (c >= 'a' && c <= 'z')
    c -= 'a' - 'A';
  return c;
}

/* Convert string s to uppercase */
STATIC VOID strupr(char *s)
{
  while (*s) {
    *s = toupper(*s);
    s++;
  }
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
  static mcb near_mcb BSS_INIT({0});
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
  strcpy(d + strlen(d), s);
  return d;
}

/* compare two ASCII strings ignoring case */
STATIC char strcaseequal(const char * d, const char * s)
{
  char ch;
  while ((ch = toupper(*s++)) == toupper(*d++))
    if (ch == '\0')
      return 1;
  return 0;
}

/*
    moved from BLOCKIO.C here.
    that saves some relocation problems    
*/

STATIC void config_init_buffers(int wantedbuffers)
{
  struct buffer FAR *pbuffer;
  unsigned buffers = 0;

  /* fill HMA with buffers if BUFFERS count >=0 and DOS in HMA        */
  if (wantedbuffers < 0)
    wantedbuffers = -wantedbuffers;
  else if (HMAState == HMA_DONE)
    buffers = (0xfff0 - HMAFree) / sizeof(struct buffer);

  if (wantedbuffers < 6)         /* min 6 buffers                     */
    wantedbuffers = 6;
  if (wantedbuffers > 99)        /* max 99 buffers                    */
  {
    printf("BUFFERS=%u not supported, reducing to 99\n", wantedbuffers);
    wantedbuffers = 99;
  }
  if (wantedbuffers > buffers)   /* more specified than available -> get em */
    buffers = wantedbuffers;

  LoL->nbuffers = buffers;
  LoL->inforecptr = &LoL->firstbuf;
  {
    size_t bytes = sizeof(struct buffer) * buffers;
    pbuffer = HMAalloc(bytes);

    if (pbuffer == NULL)
    {
      pbuffer = KernelAlloc(bytes, 'B', 0);
      if (HMAState == HMA_DONE)
        firstAvailableBuf = MK_FP(0xffff, HMAFree);
    }
    else
    {
      LoL->bufloc = LOC_HMA;
      /* space in HMA beyond requested buffers available as user space */
      firstAvailableBuf = pbuffer + wantedbuffers;
    }
  }
  LoL->deblock_buf = DiskTransferBuffer;
  LoL->firstbuf = pbuffer;

  DebugPrintf(("init_buffers (size %u) at", sizeof(struct buffer)));
  DebugPrintf((" (%p)", LoL->firstbuf));

  buffers--;
  pbuffer->b_prev = FP_OFF(pbuffer + buffers);
  {
    int i = buffers;
    do
    {
      pbuffer->b_next = FP_OFF(pbuffer + 1);
      pbuffer++;
      pbuffer->b_prev = FP_OFF(pbuffer - 1);
    }
    while (--i);
  }
  pbuffer->b_next = FP_OFF(pbuffer - buffers);

    /* now, we can have quite some buffers in HMA
       -- up to 50 for KE38616.
       so we fill the HMA with buffers
       but not if the BUFFERS count is negative ;-)
     */

  DebugPrintf((" done\n"));

  if (FP_SEG(pbuffer) == 0xffff)
  {
    buffers++;
    printf("Kernel: allocated %d Diskbuffers = %u Bytes in HMA\n",
           buffers, buffers * sizeof(struct buffer));
  }
}

/*
    Undocumented feature:  ANYDOS 
        will report to MSDOS programs just the version number
        they expect. be careful with it!
*/
STATIC VOID SetAnyDos(BYTE * pLine)
{
  UNREFERENCED_PARAMETER(pLine);
  ReturnAnyDosVersionExpected = TRUE;
}

/*
   Kernel built-in energy saving: IDLEHALT=haltlevel
   -1 max savings, 0 never HLT, 1 safe kernel only HLT,
   2 (3) also hooks int2f.1680 (and sets al=0)
*/
STATIC VOID SetIdleHalt(BYTE * pLine)
{
  COUNT haltlevel;
  if (GetNumArg(pLine, &haltlevel))
    HaltCpuWhileIdle = haltlevel; /* 0 for no HLT, 1..n more, -1 max */
}

STATIC VOID CfgIgnore(BYTE * pLine)
{
  UNREFERENCED_PARAMETER(pLine);
}

/*
   'MENU'ing stuff   
   although it's worse then MSDOS's , its better then nothing    
*/

STATIC void ClearScreen(unsigned char attr);
STATIC VOID CfgMenu(BYTE * pLine)
{
  int nLen;
  BYTE *pNumber = pLine;

  printf("%s\n",pLine);
  if (MenuColor == -1)
    return;

  pLine = skipwh(pLine);

  /* skip drawing characters in cp437, which is what we'll have
     just after booting! */
  while ((unsigned char)*pLine >= 0xb0 && (unsigned char)*pLine < 0xe0)
    pLine++;

  pLine = skipwh(pLine);  /* skip more whitespaces... */

  /* now I'm expecting a number here if this is a menu-choice line. */
  if (isnum(pLine[0]))
  {
    struct MenuSelector *menu = &MenuStruct[pLine[0]-'0'];

    menu->x = (pLine-pNumber);  /* xpos is at start of number */
    menu->y = nMenuLine;
    /* copy menu text: */
    nLen = findend(pLine); /* length is until cr/lf, null or three spaces */

    /* max 40 chars including nullterminator
       (change struct at top of file if you want more...) */
    if (nLen > MENULINEMAX-1)
      nLen = MENULINEMAX-1;
    memcpy(menu->Text, pLine, nLen);
    menu->Text[nLen] = 0;  /* nullTerminate */
  }
  nMenuLine++;
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
  iregs r;
  int key = -1;
  if (Menus == 0)
    return;

  InitKernelConfig.SkipConfigSeconds = -1;

  if (MenuColor == -1)
    Menus |= 1 << 0;          /* '0' Menu always allowed */

  nMenuLine+=2; /* use this to position "select menu" text (ypos): */

  for (;;)
  {
    int i, j;

RestartInput:

    if (MenuColor != -1)
    {
      SelectLine(MenuSelected); /* select current line. */

      /* set new cursor position: */
      r.a.b.h = 0x02;
      r.b.b.h = 0;
      r.d.b.l = 3;
      r.d.b.h = nMenuLine;

      init_call_intr(0x10, &r);  /* set cursor pos */
    }

    printf("Select from Menu [");

    for (i = 0, j = 1; i <= 9; i++, j<<=1)
      if (Menus & j)
        printf("%c", '0' + i);
    printf("], or press [ENTER]");

    if (MenuColor != -1)
      printf(" (Selection=%d) ", MenuSelected);

    if (MenuTimeout >= 0)
      printf("- %d \b", MenuTimeout);
    else
      printf("    \b\b\b\b\b");

    if (MenuColor != -1)
      printf("\r\n\n  ");
    else
      printf(" -");

    printf(" Singlestepping (F8) is: %s \r", singleStep ? "ON " : "OFF");

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
    else if(key == 0x4800 && MenuColor != -1)      /* arrow up */
    {
      if(MenuSelected>=1 && (Menus & (1 << (MenuSelected-1))) )
      {
        MenuSelected--;
      }
    }
    else if(key == 0x5000 && MenuColor != -1)      /* arrow down */
    {
      if(MenuSelected<MENULINESMAX-1 && (Menus & (1 << (MenuSelected+1))) )
      {
        MenuSelected++;
      }
    }

    key &= 0xff;

    if (key == '\r' || key == 0x1b) /* CR/ESC - use default */
      break;

    if (isnum(key) && (Menus & (1 << (key - '0'))))
    {
      MenuSelected = key - '0';
      break;
    }
  }
  printf("\n");

  /* export the current selected config  menu */
  sprintf(envp, "CONFIG=%c", MenuSelected+'0');
  envp += 9;
  if (MenuColor != -1)
    ClearScreen(0x7);
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

STATIC void ClearScreen(unsigned char attr)
{
  /* scroll down (newlines): */
  iregs r;
  unsigned char rows;

  /* clear */
  r.a.x = 0x0600;
  r.b.b.h = attr;
  r.c.x = 0;
  r.d.b.l = peekb(0x40, 0x4a) - 1; /* columns */
  rows = peekb(0x40, 0x84);
  if (rows == 0) rows = 24;
  r.d.b.h = rows;
  init_call_intr(0x10, &r);

  /* move cursor to pos 0,0: */
  r.a.b.h = 0x02; /* set cursorpos */
  r.b.b.h = 0;    /* displaypage: */
  r.d.x = 0;  /* pos 0,0 */
  init_call_intr(0x10, &r);
  MenuColor = attr;
}

/**
  MENUCOLOR[=] fg[, bg]
*/
STATIC void CfgMenuColor(BYTE * pLine)
{
  int num = 0;
  unsigned char fg, bg = 0;

  pLine = skipwh(pLine);

  if ('=' == *pLine)
    pLine = skipwh(pLine + 1);

  pLine = GetNumArg(pLine, &num);
  if (pLine == 0)
    return;
  fg = (unsigned char)num;

  pLine = skipwh(pLine);

  if (*pLine == ',')
  {
    pLine = GetNumArg(skipwh(pLine+1), &num);
    if (pLine == 0)
      return;
    bg = (unsigned char)num;
  }
  ClearScreen((bg << 4) | fg);
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

struct CountrySpecificInfoSmall {
  short CountryID;    /*  = W1 W437   # Country ID */
  char  DateFormat;           /*    Date format: 0/1/2: U.S.A./Europe/Japan */
  char  CurrencyString[3];    /* '$' ,'EUR'   */
  char  ThousandSeparator;    /* ','          # Thousand's separator */
  char  DecimalPoint;         /* '.'        # Decimal point        */
  char  DateSeparator;        /* '-'  */
  char  TimeSeparator;        /* ':'  */
  char  CurrencyFormat;       /* = 0  # Currency format (bit array)  */
  char  CurrencyPrecision;    /* = 2  # Currency precision           */
  char  TimeFormat;           /* = 0  # time format: 0/1: 12/24 houres */
};

struct CountrySpecificInfoSmall specificCountriesSupported[] = {

/* table rewritten by Bernd Blaauw
Country ID  : international numbering
Date format : M = Month, D = Day, Y = Year (4digit); 0=USA, 1=Europe, 2=Japan
Currency    : $ = dollar, EUR = EURO, United Kingdom uses the pound sign
Thousands   : separator for thousands (1,000,000 bytes; Dutch: 1.000.000 bytes)
Decimals    : separator for decimals (2.5KB; Dutch: 2,5KB)
Datesep     : Date separator (2/4/2004 or 2-4-2004 for example)
Timesep     : usually ":" is used to separate hours, minutes and seconds
Currencyf   : Currency format (bit array)
Currencyp   : Currency precision
Timeformat  : 0=12 hour format (AM/PM), 1=24 hour format (16:12 means 4:12 PM)

  ID  Date     currency  1000 0.1 date time C digit time       Locale/Country
-----------------------------------------------------------------------------*/
{  1,_DATE_MDY,"$"       ,',','.', '/',':', 0 , 2,_TIME_12},/* United States */
{  2,_DATE_YMD,"$"       ,',','.', '-',':', 0 , 2,_TIME_24},/* Canada French */
{  3,_DATE_MDY,"$"       ,',','.', '/',':', 0 , 2,_TIME_12},/* Latin America */
{  7,_DATE_DMY,"RUB"     ,' ',',', '.',':', 3 , 2,_TIME_24},/* Russia        */
{ 31,_DATE_DMY,"EUR"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* Netherlands   */
{ 32,_DATE_DMY,"EUR"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* Belgium       */
{ 33,_DATE_DMY,"EUR"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* France        */
{ 34,_DATE_DMY,"EUR"     ,'.','\'','-',':', 0 , 2,_TIME_24},/* Spain         */
{ 36,_DATE_DMY,"$HU"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* Hungary       */
{ 38,_DATE_DMY,"$YU"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* Yugoslavia    */
{ 39,_DATE_DMY,"EUR"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* Italy         */
{ 41,_DATE_DMY,"SF"      ,'.',',', '.',':', 0 , 2,_TIME_24},/* Switserland   */
{ 42,_DATE_YMD,"$YU"     ,'.',',', '.',':', 0 , 2,_TIME_24},/* Czech/Slovakia*/
{ 44,_DATE_DMY,"\x9c"    ,'.',',', '/',':', 0 , 2,_TIME_24},/* United Kingdom*/
{ 45,_DATE_DMY,"DKK"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* Denmark       */
{ 46,_DATE_YMD,"SEK"     ,',','.', '-',':', 0 , 2,_TIME_24},/* Sweden        */
{ 47,_DATE_DMY,"NOK"     ,',','.', '.',':', 0 , 2,_TIME_24},/* Norway        */
{ 48,_DATE_YMD,"PLN"     ,',','.', '.',':', 0 , 2,_TIME_24},/* Poland        */
{ 49,_DATE_DMY,"EUR"     ,'.',',', '.',':', 1 , 2,_TIME_24},/* Germany       */
{ 54,_DATE_DMY,"$ar"     ,'.',',', '/',':', 1 , 2,_TIME_12},/* Argentina     */
{ 55,_DATE_DMY,"$ar"     ,'.',',', '/',':', 1 , 2,_TIME_24},/* Brazil        */
{ 61,_DATE_MDY,"$"       ,'.',',', '/',':', 0 , 2,_TIME_24},/* Int. English  */
{ 81,_DATE_YMD,"\x81\x8f",',','.', '/',':', 0 , 2,_TIME_12},/* Japan         */
{351,_DATE_DMY,"EUR"     ,'.',',', '-',':', 0 , 2,_TIME_24},/* Portugal      */
{358,_DATE_DMY,"EUR"     ,' ',',', '.',':',0x3, 2,_TIME_24},/* Finland       */
{359,_DATE_DMY,"BGL"     ,' ',',', '.',':', 3 , 2,_TIME_24},/* Bulgaria      */
{380,_DATE_DMY,"UAH"     ,' ',',', '.',':', 3 , 2,_TIME_24},/* Ukraine       */
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
	Aitor Santamaria Merino (SP)
*/

STATIC int LoadCountryInfoHardCoded(COUNT ctryCode)
{
  struct CountrySpecificInfoSmall *country;

  /* printf("cntry: %u, CP%u, file=\"%s\"\n", ctryCode, codePage, filename);  */



  for (country = specificCountriesSupported;
       country < specificCountriesSupported + LENGTH(specificCountriesSupported);
       country++)
  {
    if (country->CountryID == ctryCode)
    {
      nlsCountryInfoHardcoded.C.CountryID = country->CountryID;
      nlsCountryInfoHardcoded.C.DateFormat = country->DateFormat;
      nlsCountryInfoHardcoded.C.CurrencyString[0] = country->CurrencyString[0];
      nlsCountryInfoHardcoded.C.CurrencyString[1] = country->CurrencyString[1];
      nlsCountryInfoHardcoded.C.CurrencyString[2] = country->CurrencyString[2];
      nlsCountryInfoHardcoded.C.ThousandSeparator[0] = country->ThousandSeparator;
      nlsCountryInfoHardcoded.C.DecimalPoint[0] = country->DecimalPoint;
      nlsCountryInfoHardcoded.C.DateSeparator[0] = country->DateSeparator;
      nlsCountryInfoHardcoded.C.TimeSeparator[0] = country->TimeSeparator;
      nlsCountryInfoHardcoded.C.CurrencyFormat = country->CurrencyFormat;
      nlsCountryInfoHardcoded.C.CurrencyPrecision = country->CurrencyPrecision;
      nlsCountryInfoHardcoded.C.TimeFormat = country->TimeFormat;
      return 0;
    }
  }

  printf("could not find country info for country ID %u\n", ctryCode);
  printf("current supported countries are ");

  for (country = specificCountriesSupported;
       country < specificCountriesSupported + LENGTH(specificCountriesSupported);
       country++)
  {
    printf("%u ", country->CountryID);
  }
  printf("\n");

  return 1;
}


/* ****************************************************************
** implementation of INSTALL=NANSI.COM /P /X /BLA 
*/

int  numInstallCmds BSS_INIT(0);
struct instCmds {
  char buffer[128];
  int mode;
} InstallCommands[10] BSS_INIT({0});

#ifdef DEBUG
#define InstallPrintf(x) printf x
#else
#define InstallPrintf(x)
#endif

STATIC VOID _CmdInstall(BYTE * pLine,int mode)
{
  struct instCmds *cmd;

  InstallPrintf(("Installcmd %d:%s\n",numInstallCmds,pLine));
  
  if (numInstallCmds > LENGTH(InstallCommands))
  {
    printf("Too many Install commands given (%d max)\n",LENGTH(InstallCommands));
    CfgFailure(pLine);
    return;
  }
  cmd = &InstallCommands[numInstallCmds];
  memcpy(cmd->buffer,pLine,127);
  cmd->buffer[127] = 0;
  cmd->mode = mode;
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
  *args = strlen(&args[1]);
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

/* set memory allocation strategy */
STATIC void set_strategy(unsigned char strat)
{
  iregs r;

  r.a.x = 0x5801;
  r.b.b.l = strat;
  init_call_intr(0x21, &r);
}

VOID DoInstall(void)
{
  int i;
  unsigned short installMemory; 
  struct instCmds *cmd;

  if (numInstallCmds == 0)
    return;
  
  InstallPrintf(("Installing commands now\n"));

  /* grab memory for this install code 
     we KNOW, that we are executing somewhere at top of memory
     we need to protect the INIT_CODE from other programs
     that will be executing soon
  */

  set_strategy(LAST_FIT);
  installMemory = ((unsigned)_init_end + ebda_size + 15) / 16;
#ifdef __WATCOMC__
  installMemory += (_InitTextEnd - _InitTextStart + 15) / 16;
#endif
  installMemory = allocmem(installMemory);

  InstallPrintf(("allocated memory at %x\n",installMemory));

  for (i = 0, cmd = InstallCommands; i < numInstallCmds; i++, cmd++)
  {
    InstallPrintf(("%d:%s\n",i,cmd->buffer));
    set_strategy(cmd->mode);
    InstallExec(cmd);
  }
  set_strategy(FIRST_FIT);
  free(installMemory);
  
  InstallPrintf(("Done with installing commands\n"));
  return;
}

STATIC VOID CmdSet(BYTE *pLine)
{
  pLine = GetStringArg(pLine, szBuf);
  pLine = skipwh(pLine);  /* scan() stops at the equal sign or space */
  if (*pLine == '=')      /* equal sign is required */
  {
    int size;
    strupr(szBuf);        /* all environment variables must be uppercase */
    strcat(szBuf, "=");
    pLine = skipwh(++pLine);
    strcat(szBuf, pLine); /* append the variable value (may include spaces) */
    size = strlen(szBuf);
    if (size < master_env + sizeof(master_env) - envp - 1)
    {                     /* must end with two consequtive zeros */
      strcpy(envp, szBuf);
      envp += size + 1;   /* add next variables starting at the second zero */
    }
    else
      printf("Master environment is full - can't add \"%s\"\n", szBuf);
  }
  else
    printf("Invalid SET command: \"%s\"\n", szBuf);
}
