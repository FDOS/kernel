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
#include "dyndata.h"



/*
  These are the far variables from the DOS data segment that we need here. The
  init procedure uses a different default DS data segment, which is discarded
  after use. I hope to clean this up to use the DOS List of List and Swappable
  Data Area obtained via INT21.

  -- Bart
 */
extern struct buffer FAR * FAR lastbuf;/* tail of ditto                        */
extern f_node_ptr FAR f_nodes; /* pointer to the array                 */
extern UWORD FAR f_nodes_cnt,           /* number of allocated f_nodes          */
             FAR first_mcb;             /* Start of user memory                 */

extern UBYTE FAR lastdrive, FAR nblkdev, FAR mem_access_mode,
             FAR uppermem_link;
extern struct dhdr 
    FAR blk_dev,               /* Block device (Disk) driver           */
    FAR nul_dev;
extern struct buffer FAR * FAR firstbuf;          /* head of buffers linked list          */

extern struct dpb FAR * FAR DPBp;
/* First drive Parameter Block          */
extern cdstbl FAR * FAR CDSp;
/* Current Directory Structure          */
extern sfttbl FAR * FAR sfthead;
/* System File Table head               */
extern sfttbl FAR * FAR FCBp;

extern BYTE FAR VgaSet,
            FAR _HMATextAvailable,        /* first byte of available CODE area    */
            FAR _HMATextStart[],          /* first byte of HMAable CODE area      */
            FAR _HMATextEnd[],
            FAR break_ena,                    /* break enabled flag                   */
            FAR os_major,                     /* major version number                 */
            FAR os_minor,                     /* minor version number                 */
            FAR switchar,
            FAR _InitTextStart,          /* first available byte of ram          */
            FAR ReturnAnyDosVersionExpected;

extern UWORD FAR ram_top,                /* How much ram in Kbytes               */
    FAR UMB_top,
    FAR umb_start,
    FAR uppermem_root;

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

#ifdef DEBUG
    #define DebugPrintf(x) printf x
#else    
    #define DebugPrintf(x)
#endif    

/*
 * $Log$
 * Revision 1.26  2001/07/28 18:13:06  bartoldeman
 * Fixes for FORMAT+SYS, FATFS, get current dir, kernel init memory situation.
 *
 * Revision 1.25  2001/07/22 01:58:58  bartoldeman
 * Support for Brian's FORMAT, DJGPP libc compilation, cleanups, MSCDEX
 *
 * Revision 1.24  2001/07/09 22:19:33  bartoldeman
 * LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
 *
 * Revision 1.23  2001/06/03 14:16:17  bartoldeman
 * BUFFERS tuning and misc bug fixes/cleanups (2024c).
 *
 * Revision 1.22  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.21  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.20  2001/04/16 14:44:29  bartoldeman
 * Removed debug printf.
 *
 * Revision 1.19  2001/04/16 14:28:32  bartoldeman
 * Kernel build 2024. Fixed critical error handler/config.sys/makefiles/UMBs
 *
 * Revision 1.18  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.17  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.16  2001/04/02 23:18:30  bartoldeman
 * Misc, zero terminated device names and redirector bugs fixed.
 *
 * Revision 1.15  2001/03/30 22:27:42  bartoldeman
 * Saner lastdrive handling.
 *
 * Revision 1.14  2001/03/30 19:30:00  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.13  2001/03/27 20:27:27  bartoldeman
 * dsk.c (reported by Nagy Daniel), inthndlr and int25/26 fixes by Tom Ehlert.
 *
 * Revision 1.12  2001/03/25 17:11:54  bartoldeman
 * Fixed sys.com compilation. Updated to 2023. Also: see history.txt.
 *
 * Revision 1.11  2001/03/22 04:55:36  bartoldeman
 * Fix prototypes.
 *
 * Revision 1.10  2001/03/21 02:56:25  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.9  2001/03/08 21:15:00  bartoldeman
 * Fixed handling of "DOS=UMB", use toupper instead of tolower consistently.
 *
 * Revision 1.8  2000/08/07 22:51:34  jimtabor
 * Remove unused code
 *
 * Revision 1.7  2000/08/07 03:03:12  jimtabor
 * Fix problem with devicehigh
 *
 * Revision 1.6  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.5  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.4  2000/05/26 19:25:19  jimtabor
 * Read History file for Change info
 *
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.15  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.14  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.13  2000/03/09 06:07:10  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.12  1999/09/23 04:40:46  jprice
 * *** empty log message ***
 *
 * Revision 1.10  1999/08/25 03:18:07  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.9  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.8  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.7  1999/04/16 12:21:21  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.6  1999/04/16 00:53:32  jprice
 * Optimized FAT handling
 *
 * Revision 1.5  1999/04/12 03:21:17  jprice
 * more ror4 patches.  Changes for multi-block IO
 *
 * Revision 1.4  1999/04/11 04:33:38  jprice
 * ror4 patches
 *
 * Revision 1.2  1999/04/04 22:57:47  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:40:46  jprice
 * New version without IPL.SYS
 *
 * Revision 1.6  1999/03/23 23:38:15  jprice
 * Now checks for a reads fdconfig.sys file, if exists
 *
 * Revision 1.5  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.3  1999/01/30 08:28:11  jprice
 * Clean up; Fixed bug with set attribute function.
 *
 * Revision 1.2  1999/01/22 04:13:25  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.6   22 Jan 1998  4:09:24   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.5   04 Jan 1998 23:15:18   patv
 * Changed Log for strip utility
 *
 *    Rev 1.4   04 Jan 1998 17:26:14   patv
 * Corrected subdirectory bug
 *
 *    Rev 1.3   16 Jan 1997 12:46:50   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.1   29 May 1996 21:03:44   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.0   19 Feb 1996  3:22:16   patv
 * Added NLS, int2f and config.sys processing
 */

#ifdef KDB
#include <alloc.h>

#define KernelAlloc(x) adjust_far((void far *)malloc((unsigned long)(x)))
#endif

struct config Config
 =
 {
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
     ,0                        /* country ID */
     ,0                        /* codepage */
     ,""                   /* filename */
     ,0                        /* amount required memory */
     ,0                        /* pointer to loaded data */
     ,0                        /* strategy for command.com is low by default */
 }
;

BYTE FAR *lpBase;
BYTE FAR *upBase;
BYTE FAR *lpTop;
BYTE FAR *lpOldTop;
static COUNT nCfgLine;
static COUNT nPass;
       COUNT UmbState;
static BYTE szLine[256];
static BYTE szBuf[256];

int singleStep    = FALSE;
int SkipAllConfig = FALSE;

INIT VOID  zumcb_init(UCOUNT seg, UWORD size);
INIT VOID  mumcb_init(UCOUNT seg, UWORD size);

INIT VOID Config_Buffers(BYTE * pLine);
INIT VOID sysScreenMode(BYTE * pLine);
INIT VOID sysVersion(BYTE * pLine);
INIT VOID Break(BYTE * pLine);
INIT VOID Device(BYTE * pLine);
INIT VOID DeviceHigh(BYTE * pLine);
INIT VOID Files(BYTE * pLine);
INIT VOID Fcbs(BYTE * pLine);
INIT VOID Lastdrive(BYTE * pLine);
INIT BOOL LoadDevice(BYTE * pLine, COUNT top, COUNT mode);
INIT VOID Dosmem(BYTE * pLine);
INIT VOID Country(BYTE * pLine);
INIT VOID InitPgm(BYTE * pLine);
INIT VOID InitPgmHigh(BYTE * pLine);
INIT VOID Switchar(BYTE * pLine);
INIT VOID CfgFailure(BYTE * pLine);
INIT VOID Stacks(BYTE * pLine);
INIT VOID SetAnyDos(BYTE * pLine);
INIT VOID Numlock(BYTE * pLine);
INIT BYTE *GetNumArg(BYTE * pLine, COUNT * pnArg);
INIT BYTE *GetStringArg(BYTE * pLine, BYTE * pszString);
INIT struct dhdr FAR *linkdev(struct dhdr FAR * dhp);
INIT UWORD initdev(struct dhdr FAR * dhp, BYTE FAR * cmdTail);
INIT int SkipLine(char *pLine);
INIT char *stristr(char *s1, char *s2);
INIT COUNT strcasecmp(REG BYTE *d, REG BYTE *s);

extern void HMAconfig(int finalize);
VOID config_init_buffers(COUNT anzBuffers); /* from BLOCKIO.C */

INIT static VOID FAR *AlignParagraph(VOID FAR * lpPtr);
#ifndef I86
#define AlignParagraph(x) (x)
#endif

#define EOF 0x1a

INIT struct table *LookUp(struct table *p, BYTE * token);

struct table
{
  BYTE *entry;
  BYTE pass;
    VOID(*func) (BYTE * pLine);
};

static struct table commands[] =
{
  {"BREAK", 1, Break},
  {"BUFFERS", 1, Config_Buffers},
  {"COMMAND", 1, InitPgm},
  {"COUNTRY", 1, Country},
  {"DEVICE", 2, Device},
  {"DEVICEHIGH", 2, DeviceHigh},
  {"DOS", 1, Dosmem},
  {"FCBS", 1, Fcbs},
  {"FILES", 1, Files},
  {"LASTDRIVE", 1, Lastdrive},
  {"NUMLOCK", 1, Numlock},
        /* rem is never executed by locking out pass                    */
  {"REM", 0, CfgFailure},
  {"SHELL", 1, InitPgm},
  {"SHELLHIGH", 1, InitPgmHigh},
  {"STACKS", 1, Stacks},
  {"SWITCHAR", 1, Switchar},
  {"SCREEN", 1, sysScreenMode}, /* JPP */
  {"VERSION", 1, sysVersion},   /* JPP */
  {"ANYDOS", 1, SetAnyDos},   /* JPP */
        /* default action                                               */
  {"", -1, CfgFailure}
};

#ifndef KDB
INIT BYTE FAR *KernelAlloc(WORD nBytes);
INIT BYTE FAR *KernelAllocDma(WORD);
#endif

BYTE *pLineStart;

BYTE HMAState;
#define HMA_NONE 0 /* do nothing */
#define HMA_REQ 1  /* DOS = HIGH detected */
#define HMA_DONE 2 /* Moved kernel to HMA */
#define HMA_LOW 3  /* Definitely LOW */

void FAR * ConfigAlloc(COUNT bytes)
{
    VOID FAR *p;

    p = HMAalloc(bytes);
    
    if (p == NULL) p = KernelAlloc(bytes);

    /* printf("ConfigAlloc %d at %p\n", bytes, p);*/
    
    return p;
}    

/* Do first time initialization.  Store last so that we can reset it    */
/* later.                                                               */
INIT void PreConfig(void)
{
  /* Set pass number                                              */
  nPass = 0;
  VgaSet = 0;
  UmbState = 0; 
 
  /* Initialize the base memory pointers                          */
      
#ifdef DEBUG
 {
  extern BYTE FAR internal_data[];
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
      DynAlloc("f_nodes", Config.cfgFiles , sizeof(struct f_node));

  f_nodes_cnt = Config.cfgFiles;
  /* sfthead = (sfttbl FAR *)&basesft; */
  /* FCBp = (sfttbl FAR *)&FcbSft; */
  /* FCBp = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + Config.cfgFiles * sizeof(sft));*/

  lpBase = AlignParagraph((BYTE FAR *)DynLast()+0x0f);

  config_init_buffers( Config.cfgBuffers);

  sfthead->sftt_next = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + (Config.cfgFiles-5) * sizeof(sft));
  sfthead->sftt_next->sftt_next = (sfttbl FAR *) - 1;
  sfthead->sftt_next->sftt_count = Config.cfgFiles-5;

  CDSp = (cdstbl FAR *)
      KernelAlloc(0x58 * lastdrive);

  DPBp = (struct dpb FAR *)
      KernelAlloc(blk_dev.dh_name[0]*sizeof(struct dpb));

#ifdef DEBUG
  printf("Preliminary:\n f_node 0x%x",f_nodes);
/*  printf(" FCB table 0x%p\n",FCBp);*/
  printf(" sft table 0x%p\n",sfthead->sftt_next);
  printf(" CDS table 0x%p\n",CDSp);
  printf(" DPB table 0x%p\n",DPBp);
#endif

  /* Done.  Now initialize the MCB structure                      */
  /* This next line is 8086 and 80x86 real mode specific          */
#ifdef DEBUG
  printf("Preliminary  allocation completed: top at 0x%p\n",lpBase);
#endif

#ifdef KDB
  lpBase = malloc(4096);
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);
#else
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);
#endif

  /* We expect ram_top as Kbytes, so convert to paragraphs */
  mcb_init(first_mcb, ram_top*64 - first_mcb - 1);
  nPass = 1;
}

/* Do second pass initialization.                                       */
/* Also, run config.sys to load drivers.                                */
INIT void PostConfig(void)
{
  /* close all (device) files */

    
  /* Set pass number                                              */
  nPass = 2;
  /* compute lastdrive ... */
  lastdrive = Config.cfgLastdrive;
  if (lastdrive < nblkdev )
    lastdrive = nblkdev ;

                        /* initialize NEAR allocated things */

  /* Initialize the file table                                    */
  DynFree(f_nodes);
  f_nodes = (f_node_ptr)
      DynAlloc("f_nodes", Config.cfgFiles , sizeof(struct f_node));
  
  f_nodes_cnt = Config.cfgFiles;   /* and the number of allocated files */


  /* Initialize the base memory pointers from last time.          */
  /*
    if the kernel could be moved to HMA, everything behind the dynamic 
    near data is free.
    otherwise, the kernel is moved down - behind the dynamic allocated data,
    and allocation starts after the kernel.
  */

  lpBase = AlignParagraph((BYTE FAR *)DynLast()+0x0f);
      
  DebugPrintf(("starting FAR allocations at %p\n",lpBase));
      

  /* Begin by initializing our system buffers                     */
  /* dma_scratch = (BYTE FAR *) KernelAllocDma(BUFFERSIZE); */
#ifdef DEBUG
  /* printf("DMA scratchpad allocated at 0x%p\n", dma_scratch); */
#endif


  config_init_buffers(Config.cfgBuffers);

/* sfthead = (sfttbl FAR *)&basesft; */
  /* FCBp = (sfttbl FAR *)&FcbSft; */
  /* FCBp = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + Config.cfgFiles * sizeof(sft));*/
  sfthead->sftt_next = (sfttbl FAR *)
      KernelAlloc(sizeof(sftheader)
                  + (Config.cfgFiles-5) * sizeof(sft));
  sfthead->sftt_next->sftt_next = (sfttbl FAR *) - 1;
  sfthead->sftt_next->sftt_count = Config.cfgFiles-5;

  CDSp = (cdstbl FAR *)
      KernelAlloc(0x58 * lastdrive);

  DPBp = (struct dpb FAR *)
      KernelAlloc(blk_dev.dh_name[0]*sizeof(struct dpb));


#ifdef DEBUG
  printf("Final: \n f_node 0x%x\n",f_nodes);
/*  printf(" FCB table 0x%p\n",FCBp);*/
  printf(" sft table 0x%p\n",sfthead->sftt_next);
  printf(" CDS table 0x%p\n",CDSp);
  printf(" DPB table 0x%p\n",DPBp);
#endif
  if (Config.cfgStacks)
  {
    VOID FAR *stackBase = KernelAlloc(Config.cfgStacks * Config.cfgStackSize);
    init_stacks(stackBase, Config.cfgStacks, Config.cfgStackSize);

    DebugPrintf(("Stacks allocated at %p\n",stackBase));
  }
  DebugPrintf(("Allocation completed: top at 0x%p\n",lpBase));

}

/* This code must be executed after device drivers has been loaded */
INIT VOID configDone(VOID)
{
  if ( HMAState != HMA_DONE )
      {
      lpBase = AlignParagraph(lpBase);
          
      DebugPrintf(("HMA not available, moving text to %x\n",FP_SEG(lpBase)));
      MoveKernel(FP_SEG(lpBase));
      
      lpBase = AlignParagraph((BYTE FAR *)lpBase + HMAFree + 0x0f);
      
      DebugPrintf(("kernel is low, start alloc at %p",lpBase));

      /* final buffer processing, now upwards */
      HMAState =  HMA_LOW;
      config_init_buffers( Config.cfgBuffers);
      }

  if (lastdrive < nblkdev) {

    DebugPrintf(("lastdrive %c too small upping it to: %c\n", lastdrive + 'A', nblkdev + 'A' -1));

    lastdrive = nblkdev;
    CDSp = (cdstbl FAR *)
       KernelAlloc(0x58 * lastdrive);
  }
  first_mcb = FP_SEG(lpBase) + ((FP_OFF(lpBase) + 0x0f) >> 4);

  /* We expect ram_top as Kbytes, so convert to paragraphs */
  mcb_init(first_mcb, ram_top*64 - first_mcb - 1);

    if(UmbState == 1)
    {

    mumcb_init(ram_top*64 - 1, umb_start - 64*ram_top);
/* Check if any devices were loaded in umb */
    if(umb_start != FP_SEG(upBase) ){
/* make last block normal with SC for the devices */
        
        UCOUNT umr_new = FP_SEG(upBase) + ((FP_OFF(upBase) + 0x0f) >> 4);
        
        mumcb_init(uppermem_root, umr_new - uppermem_root - 1);

        uppermem_root = umr_new;
        zumcb_init(uppermem_root, (umb_start + UMB_top ) - uppermem_root - 1);
        upBase += 16;
     }

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
        
        umbz_root = uppermem_root;
            
        if(UMB_get_largest(&umb_seg, &umb_size)){
            
            mcb_init(umbz_root, (umb_start + UMB_top ) - uppermem_root - 1);
            
                                            /* change UMB 'Z' to 'M' */
            ((mcb FAR *)MK_FP(umbz_root,0))->m_type = 'M';

                                            /* move to end */            
            umbz_root += ((mcb FAR *)MK_FP(umbz_root,0))->m_size + 1;
            
                                            /* create link mcb       */
            mumcb_init(umbz_root, umb_seg - umbz_root - 1);


                                                /* should the UMB driver return
                                                   adjacent memory in several pieces */
            if (umb_seg - umbz_root - 1 == 0)
                ((mcb FAR *)MK_FP(umbz_root,0))->m_psp = FREE_PSP;

                                                /* create new 'Z' mcb */
            zumcb_init(umb_seg, umb_size - 1);
            }            
        }
    }

  DebugPrintf(("UMB Allocation completed: top at 0x%p\n",upBase));

  /* The standard handles should be reopened here, because
     we may have loaded new console or printer drivers in CONFIG.SYS */


}

INIT VOID DoConfig(VOID)
{
  COUNT nFileDesc;
  BYTE *pLine;
  BOOL bEof;

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

  for (;!bEof;nCfgLine++)
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
        
        if (pLine >= szLine + sizeof(szLine)-3)
            {
            CfgFailure(pLine);
            printf("error - line overflow line %d \n",nCfgLine);
            break;
            }    

        if (*pLine == '\n' ||
            *pLine == EOF  )  /* end of line */
            break;
        
        if (*pLine == '\r')  /* ignore */
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

      if ( SkipLine(pLineStart))      /* F5/F8 processing */
          continue;
          
      pLine = skipwh(pLine);

      if ('=' != *pLine)
          CfgFailure(pLine);
      else                              /* YES. DO IT */
          (*(pEntry->func)) (skipwh(pLine+1));



                            /* might have been the UMB driver */
      if(UmbState == 2){

            UCOUNT umb_seg, umb_size;
            
            if(UMB_get_largest(&umb_seg, &umb_size)){
                UmbState = 1;
                upBase    = MK_FP(umb_seg , 0);
                UMB_top   = umb_size;
                umb_start = umb_seg;
                
/* reset root */
                uppermem_root = umb_seg;
/* setup the real mcb for the devicehigh block */
                zumcb_init(umb_seg, UMB_top - 1);
            upBase += 16;
            }
        }

      }
  close(nFileDesc);
}

INIT struct table *LookUp(struct table *p, BYTE * token)
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

ULONG GetBiosTime(VOID)
{
   return *(ULONG FAR *)(MK_FP(0x40,0x6c));
}    
GetBiosKey(int timeout)
{
    iregs r;
    
    ULONG startTime = GetBiosTime();        

    for (;;)
    {
        r.a.x = 0x0100;                 /* are there keys available ? */
        init_call_intr(0x16,&r);

        if ((r.flags & 0x40) == 0)      /* yes - fetch and return     */
        {
            r.a.x = 0x0000;
            init_call_intr(0x16,&r);
            
            return r.a.x;
        }            

    if (timeout < 0)
        continue;
 
    if (GetBiosTime() - startTime >= timeout*18)
            break;
    }
    return 0xffff;
}    

INIT BOOL SkipLine(char *pLine)
{
  short key;
  
  static char initialized = FALSE;
  
  if (!initialized)
  {
              
        initialized = TRUE;
        
        printf("Press F8 to trace or F5 to skip CONFIG.SYS/AUTOEXEC.BAT");
        
        key = GetBiosKey(2);      /* wait 2 seconds */
            
        if (key == 0x3f00)        /* F5 */
        {
            SkipAllConfig = TRUE;
        }
        if (key == 0x4200)        /* F8 */
        {
            singleStep = TRUE;
        }
        
        printf("\r%79s\r","");      /* clear line */
        
        if (SkipAllConfig)
            printf("Skipping CONFIG.SYS/AUTOEXEC.BAT\n");
  }
    
  if (SkipAllConfig) 
    return TRUE;

  if (!singleStep)
    return FALSE;
            
  printf("%s[Y,N]?", pLine);
  
  for (;;)
  {
    key = GetBiosKey(-1);
    
    switch(toupper(key & 0x00ff))
    {
        case 'N':
        case 'n':
            printf("N\n");
            return TRUE;

        case 0x1b:          /* don't know where documented
                               ESCAPE answers all following questions
                               with YES
                            */
            singleStep = FALSE; /* and fall through */

        case '\r':    
        case '\n':    
        case 'Y':    
        case 'y':    
            printf("Y\n");
            return FALSE;
            
    }
    
    if (key == 0x3f00)        /* YES, you may hit F5 here, too */
    {
            printf("N\n");
            SkipAllConfig = TRUE;
            return TRUE;
    }
  }  
  
}

INIT BYTE *GetNumArg(BYTE * pLine, COUNT * pnArg)
{
  /* look for NUMBER                               */
  pLine = skipwh(pLine);
  if (!isnum(pLine))
  {
    CfgFailure(pLine);
    return (BYTE *) 0;
  }
  return GetNumber(pLine, pnArg);
}

INIT BYTE *GetStringArg(BYTE * pLine, BYTE * pszString)
{
  /* look for STRING                               */
  pLine = skipwh(pLine);

  /* just return whatever string is there, including null         */
  return scan(pLine, pszString);
}

INIT void Config_Buffers(BYTE * pLine)
{
  COUNT nBuffers;

  /* Get the argument                                             */
  if (GetNumArg(pLine, &nBuffers) == (BYTE *) 0)
    return;

  /* Got the value, assign either default or new value            */
  Config.cfgBuffers = max(Config.cfgBuffers, nBuffers);
}

INIT static VOID sysScreenMode(BYTE * pLine)
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
  _AX = (0x11 << 8) + nMode;
  _BL = 0;
  __int__(0x10);
}

INIT static VOID sysVersion(BYTE * pLine)
{
  COUNT major,
    minor;
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

INIT static VOID Files(BYTE * pLine)
{
  COUNT nFiles;

  /* Get the argument                                             */
  if (GetNumArg(pLine, &nFiles) == (BYTE *) 0)
    return;

  /* Got the value, assign either default or new value            */
  Config.cfgFiles = max(Config.cfgFiles, nFiles);
}

INIT static VOID Lastdrive(BYTE * pLine)
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
  drv++;                    /* Make real number*/
  Config.cfgLastdrive = max(Config.cfgLastdrive, drv);
}

/*
    UmbState of confidence, 1 is sure, 2 maybe, 4 unknown and 0 no way.
*/


INIT STATIC VOID Dosmem(BYTE * pLine)
{
    BYTE *pTmp;
    BYTE  UMBwanted = FALSE;

/*    extern BYTE FAR INITDataSegmentClaimed; */

    pLine = GetStringArg(pLine, szBuf);

    for (pTmp = szBuf; *pTmp != '\0'; pTmp++)
        *pTmp = toupper(*pTmp);

    printf("DOS called with %s\n", szBuf);

    for (pTmp = szBuf ; ; )
    {
        if (fmemcmp(pTmp, "UMB" ,3) == 0) { UMBwanted = TRUE; pTmp += 3; }
        if (fmemcmp(pTmp, "HIGH",4) == 0) { HMAState = HMA_REQ; pTmp += 4; }
/*        if (fmemcmp(pTmp, "CLAIMINIT",9) == 0) { INITDataSegmentClaimed = 0; pTmp += 9; }*/
        pTmp = skipwh(pTmp);

        if (*pTmp != ',')
            break;
        pTmp++;    
    }        
    
    
    if(UmbState == 0){
        uppermem_link = 0;
        uppermem_root = 0;
        UmbState = UMBwanted ? 2 : 0;
    }
    /* Check if HMA is available straight away */
    if (HMAState == HMA_REQ && MoveKernelToHMA()){
        HMAState = HMA_DONE;
    }
}

INIT static VOID Switchar(BYTE * pLine)
{
  /* Format: SWITCHAR = character         */

  GetStringArg(pLine, szBuf);
  switchar = *szBuf;
}

INIT static VOID Fcbs(BYTE * pLine)
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

INIT BOOL LoadCountryInfo(char *filename, UWORD ctryCode, UWORD codePage)
{
/* printf("cntry: %u, CP%u, file=\"%s\"\n", ctryCode, codePage, filename); */
	printf("Sorry, the COUNTRY= statement has been temporarily disabled\n");
	
	UNREFERENCED_PARAMETER(codePage);
	UNREFERENCED_PARAMETER(ctryCode);
	UNREFERENCED_PARAMETER(filename);
	
	return FALSE;
}

INIT static VOID Country(BYTE * pLine)
{
  /* Format: COUNTRY = countryCode, [codePage], filename  */
  COUNT ctryCode;
  COUNT codePage;

  if ((pLine = GetNumArg(pLine, &ctryCode)) == 0)
    return;

  pLine = skipwh(pLine);
  if (*pLine == ',')
  {
    pLine = skipwh(pLine + 1);

    if (*pLine == ',')
    {
      codePage = NLS_DEFAULT;
    }
    else
    {
      if ((pLine = GetNumArg(pLine, &codePage)) == 0)
        return;
    }

    pLine = skipwh(pLine);
    if (*pLine == ',')
    {
      GetStringArg(++pLine, szBuf);

      if (LoadCountryInfo(szBuf, ctryCode, codePage))
        return;
    }
  }
  CfgFailure(pLine);
}

INIT static VOID Stacks(BYTE * pLine)
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

INIT static VOID InitPgmHigh(BYTE * pLine)
{
  InitPgm(pLine);
  Config.cfgP_0_startmode = 0x80;
}


INIT static VOID InitPgm(BYTE * pLine)
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

INIT static VOID Break(BYTE * pLine)
{
  /* Format:      BREAK = (ON | OFF)      */
  GetStringArg(pLine, szBuf);
  break_ena = strcasecmp(szBuf, "OFF") ? 1 : 0;
}

INIT static VOID Numlock(BYTE * pLine)
{
  extern VOID keycheck();
    
  /* Format:      NUMLOCK = (ON | OFF)      */
  BYTE FAR *keyflags = (BYTE FAR *)MK_FP(0x40,0x17);

  GetStringArg(pLine, szBuf);

  *keyflags &= ~32;
  *keyflags |= strcasecmp(szBuf, "OFF") ? 32 : 0;
  keycheck();
}

INIT static VOID DeviceHigh(BYTE * pLine)
{
    if(UmbState == 1)
    {
        if (LoadDevice(pLine, UMB_top, TRUE) == DE_NOMEM)
        {
            printf("Not enough free memory in UMB's: loading low\n");
            LoadDevice(pLine, ram_top, FALSE);
        }
    }
    else
    {
        printf("UMB's unavailable!\n");
        LoadDevice(pLine, ram_top, FALSE);
    }
}

INIT void Device(BYTE * pLine)
{
    LoadDevice(pLine, ram_top, FALSE);
}

INIT BOOL LoadDevice(BYTE * pLine, COUNT top, COUNT mode)
{
  exec_blk eb;
  struct dhdr FAR *dhp;
  struct dhdr FAR *next_dhp;
  BOOL result;

  if(mode)
    dhp = AlignParagraph(upBase);
  else
    dhp = AlignParagraph(lpBase);

  /* Get the device driver name                                   */
  GetStringArg(pLine, szBuf);

  /* The driver is loaded at the top of allocated memory.         */
  /* The device driver is paragraph aligned.                      */
  eb.load.reloc = eb.load.load_seg = FP_SEG(dhp);

#ifdef DEBUG
  printf("Loading device driver %s at segment %04x\n",
         szBuf, FP_SEG(dhp));
#endif


  if (init_DosExec(3, &eb, szBuf) == SUCCESS)
  {
        strcpy(szBuf, pLine);

    /* TE this fixes the loading of devices drivers with
       multiple devices in it. NUMEGA's SoftIce is such a beast
    */   
    
    /* add \r\n to the command line */
    strcat(szBuf, "\r\n");
    
    
    for (next_dhp=NULL; FP_OFF(next_dhp) != 0xffff &&
             (result=init_device(dhp, szBuf, mode, top))==SUCCESS
            ; dhp = next_dhp)
    { 
      next_dhp = dhp->dh_next;
      if (FP_SEG(next_dhp) == 0xffff)
        /*  Does this ever occur with FP_OFF(next_dhp) != 0xffff ??? */
      next_dhp = MK_FP(FP_SEG(dhp), FP_OFF(next_dhp));
#ifdef DEBUG      
      else if (FP_OFF(next_dhp) != 0xffff)  /* end of internal chain */
        printf("multisegmented device driver found, next %p\n",next_dhp);
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
      config_init_buffers( Config.cfgBuffers);
    }
  }
  else
    CfgFailure(pLine);
  return result;
}

INIT static VOID CfgFailure(BYTE * pLine)
{
  BYTE *pTmp = pLineStart;

  printf("CONFIG.SYS error in line %d\n", nCfgLine);
  printf(">>>%s\n   ", pTmp);
  while (++pTmp != pLine)
    printf(" ");
  printf("^\n");
}

#ifndef KDB
INIT BYTE FAR *KernelAlloc(WORD nBytes)
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

  fmemset( lpAllocated, 0, nBytes);

  return lpAllocated;
}
#endif

#ifdef I86
INIT BYTE FAR *KernelAllocDma(WORD bytes)
{
  BYTE FAR *allocated;

  lpBase = AlignParagraph(lpBase);
  if ((FP_SEG(lpBase) & 0x0fff) + (bytes >> 4) > 0x1000)
    lpBase = MK_FP((FP_SEG(lpBase) + 0x0fff) & 0xf000, 0);
  allocated = lpBase;
  lpBase += bytes;
  return allocated;
}

INIT void FAR *AlignParagraph(VOID FAR * lpPtr)
{
  UWORD uSegVal;

  /* First, convert the segmented pointer to linear address       */
  uSegVal = FP_SEG(lpPtr);
  uSegVal += (FP_OFF(lpPtr)+0xf) >> 4;
  if (FP_OFF(lpPtr) > 0xfff0)
      uSegVal += 0x1000; /* handle overflow */

  /* and return an adddress adjusted to the nearest paragraph     */
  /* boundary.                                                    */
  return MK_FP(uSegVal, 0);
}
#endif

INIT BYTE *
  skipwh(BYTE * s)
{
  while (*s && (*s == 0x0d || *s == 0x0a || *s == ' ' || *s == '\t'))
    ++s;
  return s;
}

INIT BYTE *
  scan(BYTE * s, BYTE * d)
{
  s = skipwh(s);
  while (*s &&
         !(*s == 0x0d
           || *s == 0x0a
           || *s == ' '
           || *s == '\t'
           || *s == '='))
    *d++ = *s++;
  *d = '\0';
  return s;
}

INIT BYTE *scan_seperator(BYTE * s, BYTE * d)
{
  s = skipwh(s);
  if (*s)
    *d++ = *s++;
  *d = '\0';
  return s;
}

INIT BOOL isnum(BYTE * pLine)
{
  return (*pLine >= '0' && *pLine <= '9');
}

/* JPP - changed so will accept hex number. */
INIT BYTE *GetNumber(REG BYTE * pszString, REG COUNT * pnNum)
{
  BYTE Base = 10;

  *pnNum = 0;
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
  return pszString;
}

/* Yet another change for true portability (WDL)                        */
#if 0
INIT COUNT tolower(COUNT c)
{
  if (c >= 'A' && c <= 'Z')
    return (c + ('a' - 'A'));
  else
    return c;
}
#endif

/* Yet another change for true portability (PJV) */
INIT COUNT toupper(COUNT c)
{
  if (c >= 'a' && c <= 'z')
    return (c - ('a' - 'A'));
  else
    return c;
}

/* The following code is 8086 dependant                         */

#if 1           /* ifdef KERNEL */
INIT VOID
  mcb_init(UCOUNT seg, UWORD size)
{
  COUNT i;

  mcb FAR * mcbp = MK_FP(seg,0);
  
  mcbp->m_type = MCB_LAST;
  mcbp->m_psp = FREE_PSP;

/*  if(UmbState == 1)*/

      mcbp->m_size = (size - 1);
/*
  mcbp->m_size = size;
*/
  for (i = 0; i < 8; i++)
    mcbp->m_name[i] = '\0';
  mem_access_mode = FIRST_FIT;
}

INIT VOID
  zumcb_init(UCOUNT seg, UWORD size)
{
  COUNT i;
  mcb FAR * mcbp = MK_FP(seg,0);
  
  mcbp->m_type = MCB_LAST;
  mcbp->m_psp = FREE_PSP;
  mcbp->m_size = size;
  for (i = 0; i < 8; i++)
    mcbp->m_name[i] = '\0';

}

INIT VOID
  mumcb_init(UCOUNT seg, UWORD size)
{
  COUNT i;
  mcb FAR * mcbp = MK_FP(seg,0);
  
  static char name[8] = "SC\0\0\0\0\0\0";

  mcbp->m_type = MCB_NORMAL;
  mcbp->m_psp = 8;
  mcbp->m_size = size;
  for (i = 0; i < 8; i++)
    mcbp->m_name[i] = name[i];
}
#endif

INIT VOID
  strcat(REG BYTE * d, REG BYTE * s)
{
  while (*d != 0)
    ++d;
  strcpy(d, s);
}

/* see if the second string is contained in the first one, ignoring case */
char *stristr(char *s1, char *s2)
{
    int loop;
    
    for ( ; *s1 ; s1++)
        for ( loop = 0; ; loop++)
        {
            if (s2[loop] == 0)  /* found end of string 2 -> success */
            {
                return s1;      /* position where s2 was found */
            }
        if (toupper(s1[loop]) != toupper(s2[loop]) )
            break;
        }    
        
    return NULL;
}

/* compare two ASCII strings ignoring case */
INIT COUNT strcasecmp(REG BYTE *d, REG BYTE *s)
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
  int HMAcount = 0;
  BYTE FAR *tmplpBase = lpBase;
  
  anzBuffers = max(anzBuffers,6);
  if (anzBuffers > 99)
    {
    printf("BUFFERS=%u not supported, reducing to 99\n",anzBuffers);
    anzBuffers = 99;
    }
  
  lpTop = lpOldTop;

  if (HMAState == HMA_NONE || HMAState == HMA_REQ)
    lpTop = lpBase = lpTop - anzBuffers * (sizeof (struct buffer) + 0xf);
  
  firstbuf = ConfigAlloc(sizeof (struct buffer));
  
  pbuffer = firstbuf;  

  for (i = 0; ; ++i)
  {
    if (FP_SEG(pbuffer) == 0xffff) HMAcount++;
    
    lastbuf = pbuffer;
    
    pbuffer->b_unit = 0;
    pbuffer->b_flag = 0;
    pbuffer->b_blkno = 0;
    pbuffer->b_copies = 0;
    pbuffer->b_offset_lo = 0;
    pbuffer->b_offset_hi = 0;
    pbuffer->b_next = NULL;

    DebugPrintf(("init_buffers buffer %d at %p\n",i, pbuffer));

    if (i < (anzBuffers - 1))
        pbuffer->b_next = ConfigAlloc(sizeof (struct buffer));

    if (pbuffer->b_next == NULL)
        break;  
        
    pbuffer = pbuffer->b_next;        
  }
  
  DebugPrintf(("Kernel: allocated %d Diskbuffers = %u Bytes in HMA\n",
                            HMAcount, HMAcount*sizeof (struct buffer)));
  if (HMAState == HMA_NONE || HMAState == HMA_REQ)
      lpBase = tmplpBase;
}

/*
    Undocumented feature: 
    
    ANYDOS 
        will report to MSDOS programs just the version number
        they expect. be careful with it!
*/        
    

INIT VOID SetAnyDos(BYTE * pLine)
{
    UNREFERENCED_PARAMETER(pLine);
    ReturnAnyDosVersionExpected = TRUE;
}

