/****************************************************************/
/*                                                              */
/*                          globals.h                           */
/*                            DOS-C                             */
/*                                                              */
/*             Global data structures and declarations          */
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

/* $Logfile:   C:/usr/patv/dos-c/src/kernel/globals.h_v  $ */
#ifdef VERSION_STRINGS
#ifdef MAIN
static BYTE *Globals_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.11  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.10  2001/04/02 23:18:30  bartoldeman
 * Misc, zero terminated device names and redirector bugs fixed.
 *
 * Revision 1.9  2001/03/30 20:11:14  bartoldeman
 * Truly got DOS=HIGH reporting for INT21/AX=0x3306 working now.
 *
 * Revision 1.8  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.7  2001/03/21 02:56:26  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.6  2000/12/16 01:38:35  jimtabor
 * Added patches from Bart Oldeman
 *
 * Revision 1.5  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.17  2000/03/16 03:28:49  kernel
 * *** empty log message ***
 *
 * Revision 1.16  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.15  1999/09/23 04:40:47  jprice
 * *** empty log message ***
 *
 * Revision 1.13  1999/08/25 03:18:08  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.12  1999/08/10 18:03:43  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.11  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.10  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.9  1999/04/16 12:21:22  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.8  1999/04/16 00:53:33  jprice
 * Optimized FAT handling
 *
 * Revision 1.7  1999/04/12 03:21:17  jprice
 * more ror4 patches.  Changes for multi-block IO
 *
 * Revision 1.6  1999/04/11 04:33:39  jprice
 * ror4 patches
 *
 * Revision 1.4  1999/04/04 22:57:47  jprice
 * no message
 *
 * Revision 1.3  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.2  1999/03/29 17:05:09  jprice
 * ror4 changes
 *
 * Revision 1.1.1.1  1999/03/29 15:40:58  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.3  1999/01/30 08:26:46  jprice
 * Clean up; commented out copyright messages while we debug.
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *

 Rev 1.16   06 Dec 1998  8:45:56   patv
 Expanded due to new I/O subsystem.

 Rev 1.15   07 Feb 1998 20:38:00   patv
 Modified stack fram to match DOS standard

 Rev 1.14   02 Feb 1998 22:33:46   patv
 Fixed size of default_drive.  Caused failures when break_ena was not zero.

 Rev 1.13   22 Jan 1998  4:09:24   patv
 Fixed pointer problems affecting SDA

 Rev 1.12   04 Jan 1998 23:16:22   patv
 Changed Log for strip utility

 Rev 1.11   03 Jan 1998  8:36:50   patv
 Converted data area to SDA format

 Rev 1.10   06 Feb 1997 21:57:04   patv
 Changed version format string

 Rev 1.9   06 Feb 1997 21:35:08   patv
 Modified to support new version format

 Rev 1.8   22 Jan 1997 13:17:14   patv
 Changed to support version.h and pre-0.92 Svante Frey bug fixes.

 Rev 1.6   16 Jan 1997 12:47:00   patv
 pre-Release 0.92 feature additions

 Rev 1.5   13 Sep 1996 19:26:32   patv
 Fixed boot for hard drive

 Rev 1.4   29 Aug 1996 13:07:22   patv
 Bug fixes for v0.91b

 Rev 1.3   29 May 1996 21:03:34   patv
 bug fixes for v0.91a

 Rev 1.2   19 Feb 1996  3:23:04   patv
 Added NLS, int2f and config.sys processing

 Rev 1.1   01 Sep 1995 17:54:16   patv
 First GPL release.

 Rev 1.0   02 Jul 1995  8:31:00   patv
 Initial revision.
 */

#include "device.h"
#include "mcb.h"
#include "pcb.h"
#include "date.h"
#include "time.h"
#include "fat.h"
#include "fcb.h"
#include "tail.h"
#include "process.h"
#include "dcb.h"
#include "sft.h"
#include "cds.h"
#include "exe.h"
#include "fnode.h"
#include "dirmatch.h"
#include "file.h"
#include "clock.h"
#include "kbd.h"
#include "error.h"
#include "version.h"
#include "network.h"
#include "config.h"

/* JPP: for testing/debuging disk IO */
/*#define DISPLAY_GETBLOCK */

/*                                                                      */
/* Convience switch for maintaining variables in a single location      */
/*                                                                      */
#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL extern
#endif

/*                                                                      */
/* Convience definitions of TRUE and FALSE                              */
/*                                                                      */
#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

/*                                                                      */
/* Constants and macros                                                 */
/*                                                                      */
/* Defaults and limits - System wide                                    */
#define PARSE_MAX       67      /* maximum # of bytes in path   */
#define NFILES          16      /* number of files in table     */
#define NFCBS           16      /* number of fcbs               */
#define NSTACKS         8       /* number of stacks             */
#define NLAST           6       /* last drive                   */
#define NAMEMAX         PARSE_MAX	/* Maximum path for CDS         */
#define NUMBUFF         6       /* Number of track buffers      */
                                        /* -- must be at least 3        */

/* 0 = CON, standard input, can be redirected                           */
/* 1 = CON, standard output, can be redirected                          */
/* 2 = CON, standard error                                              */
/* 3 = AUX, auxiliary                                                   */
/* 4 = PRN, list device                                                 */
/* 5 = 1st user file ...                                                */
#define STDIN           0
#define STDOUT          1
#define STDERR          2
#define STDAUX          3
#define STDPRN          4

/* internal error from failure or aborted operation                     */
#define ERROR           -1
#define OK              0

/* internal transfer direction flags                                    */
#define XFR_READ        1
#define XFR_WRITE       2

#define RDONLY          0
#define WRONLY          1
#define RDWR            2

/* special ascii code equates                                           */
#define SPCL            0x00
#define CTL_C           0x03
#define CTL_F           0x06
#define BELL            0x07
#define BS              0x08
#define HT              0x09
#define LF              0x0a
#define CR              0x0d
#define CTL_Q           0x11
#define CTL_S           0x13
#define CTL_Z           0x1a
#define ESC             0x1b
#define CTL_BS          0x7f

#define F3              0x3d
#define LEFT            0x4b
#define RIGHT           0x4d

/* Blockio constants                                                    */
#define DSKWRITE        1       /* dskxfr function parameters   */
#define DSKREAD         2

/* FAT cluster special flags                                            */
#define FREE                    0x000

#define LONG_LAST_CLUSTER       0xFFFF
#define LONG_MASK               0xFFF8
#define LONG_BAD                0xFFF0
#define LAST_CLUSTER            0x0FFF
#define MASK                    0xFF8
#define BAD                     0xFF0

/* Keyboard buffer maximum size                                         */
#ifdef LINESIZE
#undef LINESIZE
#endif
#define LINESIZE 256

/*                                                                      */
/* Data structures and unions                                           */
/*                                                                      */
/* Sector buffer structure                                              */
#define BUFFERSIZE 512
struct buffer
{
  struct buffer
  FAR *b_next;                  /* form linked list for LRU     */
  BYTE b_unit;                  /* disk for this buffer         */
  BYTE b_flag;                  /* buffer flags                 */
  ULONG b_blkno;                /* block for this buffer        */
  /* DOS-C: 0xffff for huge block numbers */
  BYTE b_copies;                /* number of copies to write    */
  UBYTE b_offset_lo;            /* span between copies (low)                                                    */
#if 0 /*TE*/
 union
  {
    struct dpb FAR *_b_dpbp;    /* pointer to DPB                                                                                                                                               */
    LONG _b_huge_blkno;         /* DOS-C: actual block number if >= 0xffff */
  }
  _b;
#endif  
  UBYTE b_offset_hi;            /* DOS-C: span between copies (high) */
  UBYTE b_unused;
  BYTE b_buffer[BUFFERSIZE];    /* 512 byte sectors for now     */
};

#define b_dpbp          _b._b_dpbp
#define b_huge_blkno    _b._b_huge_blkno

#define BFR_DIRTY       0x40    /* buffer modified              */
#define BFR_VALID       0x20    /* buffer contains valid data   */
#define BFR_DATA        0x08    /* buffer is from data area     */
#define BFR_DIR         0x04    /* buffer is from dir area      */
#define BFR_FAT         0x02    /* buffer is from fat area      */
#define BFR_BOOT        0x01    /* buffer is boot disk          */

/* NLS character table type                                             */
typedef BYTE *UPMAP;

/*                                                                      */
/* External Assembly variables                                          */
/*                                                                      */
extern struct dhdr
FAR clk_dev,                    /* Clock device driver                  */
  FAR con_dev,                  /* Console device driver                */
  FAR prn_dev,                  /* Generic printer device driver        */
  FAR aux_dev,                  /* Generic aux device driver            */
  FAR blk_dev;                  /* Block device (Disk) driver           */
extern UWORD
  ram_top,                      /* How much ram in Kbytes               */
#ifdef I86

  api_sp,                       /* api stacks - for context             */
#endif

  api_ss,                       /* switching                            */
  usr_sp,                       /* user stack                           */
  usr_ss;
extern COUNT *
#ifdef MC68K
  api_sp,                       /* api stacks - for context             */
#endif

  error_tos,                    /* error stack                          */
  disk_api_tos,                 /* API handler stack - disk fns         */
  char_api_tos;                 /* API handler stack - char fns         */
extern BYTE
  FAR _InitTextStart;           /* first available byte of ram          */
extern BYTE
  FAR _HMATextAvailable,        /* first byte of available CODE area    */
  FAR _HMATextStart[],          /* first byte of HMAable CODE area      */
  FAR _HMATextEnd[];            /* and the last byte of it              */
extern 
  BYTE DosLoadedInHMA;          /* if InitHMA has moved DOS up          */
  
extern struct ClockRecord
  ClkRecord;

/*                                                                      */
/* Global variables                                                     */
/*                                                                      */
GLOBAL
seg master_env;                 /* Master environment segment           */

GLOBAL BYTE
  os_major,                     /* major version number                 */
  os_minor,                     /* minor version number                 */

  rev_number                    /* minor version number                 */
#ifdef MAIN
= REV_NUMBER,
#else
 ,
#endif

  version_flags                 /* minor version number                 */
#ifdef MAIN
= 0;
#else
  ; 
#endif

#ifdef DEBUG
GLOBAL WORD bDumpRegs
#ifdef MAIN
= FALSE;
#else
 ;
#endif
GLOBAL WORD bDumpRdWrParms
#ifdef MAIN
= FALSE;
#else
 ;
#endif
#endif

GLOBAL BYTE *copyright
#if 0
= "(C) Copyright 1995, 1996, 1997, 1998\nPasquale J. Villani\nAll Rights Reserved\n";
#else
 ;
#endif

GLOBAL BYTE *os_release
#ifdef MAIN
#if 0
= "DOS-C version %d.%d Beta %d [FreeDOS Release] (Build %d).\n\
\n\
DOS-C is free software; you can redistribute it and/or modify it under the\n\
terms of the GNU General Public License as published by the Free Software\n\
Foundation; either version 2, or (at your option) any later version.\n\n\
For technical information and description of the DOS-C operating system\n\
consult \"FreeDOS Kernel\" by Pat Villani, published by Miller\n\
Freeman Publishing, Lawrence KS, USA (ISBN 0-87930-436-7).\n\
\n";
#else
= "FreeDOS kernel version %d.%d.%d (Build %d) [" __DATE__ " " __TIME__ "]\n\n";
#endif
#else
 ;
#endif

/* Globally referenced variables - WARNING: ORDER IS DEFINED IN         */
/* KERNAL.ASM AND MUST NOT BE CHANGED. DO NOT CHANGE ORDER BECAUSE THEY */
/* ARE DOCUMENTED AS UNDOCUMENTED (?) AND HAVE MANY  PROGRAMS AND TSR'S */
/* ACCESSING THEM                                                       */

extern UWORD NetBios;
extern BYTE *net_name;
extern BYTE net_set_count;
extern BYTE NetDelay,
  NetRetry;

extern UWORD
  first_mcb,                    /* Start of user memory                 */
  UMB_top,
  umb_start,
  uppermem_root;                /* Start of umb chain ? */
extern struct dpb
FAR *DPBp;                      /* First drive Parameter Block          */
extern sfttbl
  FAR * sfthead;                /* System File Table head               */
extern struct dhdr
FAR *clock,                     /* CLOCK$ device                        */
  FAR * syscon;                 /* console device                       */
extern WORD
  maxbksize;                    /* Number of Drives in system           */
extern struct buffer
FAR *firstbuf;                  /* head of buffers linked list          */
extern cdstbl
  FAR * CDSp;                   /* Current Directory Structure          */
extern
struct cds FAR *current_ldt;
extern sfttbl
  FAR * FCBp;                   /* FCB table pointer                    */
extern WORD
  nprotfcb;                     /* number of protected fcbs             */
extern UBYTE
  nblkdev,                      /* number of block devices              */
  lastdrive,                    /* value of last drive                  */
  uppermem_link;                /* UMB Link flag */
extern struct dhdr
  nul_dev;
extern BYTE
  LocalPath[PARSE_MAX + 3];     /* Room for drive spec                  */
extern UBYTE
  mem_access_mode;              /* memory allocation scheme             */
extern BYTE
  ErrorMode,                    /* Critical error flag                  */
  InDOS,                        /* In DOS critical section              */
  OpenMode,                     /* File Open Attributes                 */
  SAttr,                        /* Attrib Mask for Dir Search           */
  dosidle_flag,
  Server_Call,
  CritErrLocus,
  CritErrAction,
  CritErrClass,
  VgaSet,
  njoined;                      /* number of joined devices             */

extern UWORD Int21AX;
extern COUNT CritErrCode;
extern BYTE FAR * CritErrDev;

extern struct dirent
  SearchDir;

extern struct
{
  COUNT nDrive;
  BYTE szName[FNAME_SIZE + 1];
  BYTE szExt[FEXT_SIZE + 1];
}
FcbSearchBuffer;

extern union                    /* Path name parsing buffer             */
{
  BYTE _PriPathName[128];
  struct
  {
    BYTE _dname[NAMEMAX];
    BYTE _fname[FNAME_SIZE];
    BYTE _fext[FEXT_SIZE];
  }
  _f;
}
_PriPathBuffer;
#define PriPathName _PriPathBuffer._PriPathName
#define szDirName _PriPathBuffer._f._dname
#define szFileName _PriPathBuffer._f._fname
#define szFileExt _PriPathBuffer._f._fext
#define szPriDirName _PriPathBuffer._f._dname
#define szPriFileName _PriPathBuffer._f._fname
#define szPriFileExt _PriPathBuffer._f._fext

extern union                    /* Alternate path name parsing buffer   */
{
  BYTE _SecPathName[128];
  struct
  {
    BYTE _dname[NAMEMAX];
    BYTE _fname[FNAME_SIZE];
    BYTE _fext[FEXT_SIZE];
  }
  _f;
}
_SecPathBuffer;
#define SecPathName _SecPathBuffer._SecPathName
#define szSecDirName _SecPathBuffer._f._dname
#define szSecFileName _SecPathBuffer._f._fname
#define szSecFileExt _SecPathBuffer._f._fext

extern UWORD
  wAttr;

extern BYTE
  default_drive;                /* default drive for dos                */

extern BYTE
  TempBuffer[],                 /* Temporary general purpose buffer     */
  FAR internal_data[],          /* sda areas                            */
  FAR swap_always[],            /*  "    "                              */
  FAR swap_indos[],             /*  "    "                              */
  tsr,                          /* true if program is TSR               */
  break_flg,                    /* true if break was detected           */
  break_ena,                    /* break enabled flag                   */
  FAR * dta;                    /* Disk transfer area (kludge)          */
extern seg
  cu_psp;                       /* current psp segment                  */
extern iregs
  FAR * user_r;                 /* User registers for int 21h call      */

extern struct dirent            /* Temporary directory entry            */
  DirEntBuffer;

extern request                  /* I/O Request packets                  */
  CharReqHdr,
  IoReqHdr,
  MediaReqHdr;

extern fcb
  FAR * lpFcb;                  /* Pointer to users fcb                 */

extern sfttbl
  FAR * lpCurSft;

extern BYTE
  verify_ena,                   /* verify enabled flag                  */
  switchar,                     /* switch char                          */
  return_mode,                  /* Process termination rets             */
  return_code;                  /*     "        "       "               */

extern BYTE
  BootDrive,                    /* Drive we came up from                */
  scr_pos;                      /* screen position for bs, ht, etc      */
extern WORD
  NumFloppies;                  /* How many floppies we have            */

extern keyboard
  kb_buf;

extern struct cds
  TempCDS;

/* start of uncontrolled variables                                      */
GLOBAL seg
  RootPsp;                      /* Root process -- do not abort         */

GLOBAL struct f_node
 *pDirFileNode;

GLOBAL iregs error_regs;        /* registers for dump                   */

GLOBAL WORD
  dump_regs;                    /* dump registers of bad call           */

GLOBAL struct f_node FAR
* f_nodes;                      /* pointer to the array                 */

GLOBAL UWORD f_nodes_cnt;       /* number of allocated f_nodes          */

GLOBAL struct buffer
FAR *lastbuf;                   /* tail of ditto                        */
/*  FAR * buffers;              /* pointer to array of track buffers    */

GLOBAL BYTE                     /* scratchpad used for working around                                           */
  FAR * dma_scratch;            /* DMA transfers during disk I/O                                                */

GLOBAL iregs
  FAR * ustackp,                /* user stack                           */
  FAR * kstackp;                /* kernel stack                         */

/* Start of configuration variables                                     */
extern struct config
{
  UBYTE cfgBuffers;             /* number of buffers in the system      */
  UBYTE cfgFiles;               /* number of available files            */
  UBYTE cfgFcbs;                /* number of available FCBs             */
  UBYTE cfgProtFcbs;            /* number of protected FCBs             */
  BYTE cfgInit[NAMEMAX];        /* init of command.com          */
  BYTE cfgInitTail[NAMEMAX];    /* command.com's tail           */
  UBYTE cfgLastdrive;           /* last drive                           */
  BYTE cfgStacks;               /* number of stacks                     */
  UWORD cfgStackSize;           /* stacks size for each stack           */
   /* COUNTRY=
       In Pass #1 these information is collected and in PostConfig()
       the NLS package is loaded into memory.
           -- 2000/06/11 ska*/
  WORD cfgCSYS_cntry;          /* country ID to be loaded */
  WORD cfgCSYS_cp;             /* requested codepage; NLS_DEFAULT if default */
  BYTE cfgCSYS_fnam[NAMEMAX];  /* filename of COUNTRY= */
  WORD cfgCSYS_memory;         /* number of bytes required for the NLS pkg;
                                   0 if none */
  VOID FAR *cfgCSYS_data;      /* where the loaded data is for PostConfig() */
  UBYTE cfgP_0_startmode;      /* load command.com high or not */
} Config
#ifdef CONFIG
=
{
  NUMBUFF,
  NFILES,
  NFCBS,
  0,
  "command.com",
  " /P\r\n",
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
};
#else
;
#endif

/*                                                                      */
/* Function prototypes - automatically generated                        */
/*                                                                      */
#include "proto.h"

/* Process related functions - not under automatic generation.  */
/* Typically, these are in ".asm" files.                        */
VOID
FAR cpm_entry(VOID),
INRPT FAR re_entry(VOID)        /*,
                                   INRPT FAR handle_break(VOID) */ ;
VOID
enable(VOID),
disable(VOID);
COUNT
CriticalError(
    COUNT nFlag, COUNT nDrive, COUNT nError, struct dhdr FAR * lpDevice);

#ifdef PROTO
VOID FAR CharMapSrvc(VOID);
VOID FAR set_stack(VOID);
VOID FAR restore_stack(VOID);
WORD execrh(request FAR *, struct dhdr FAR *);
VOID exit(COUNT);
/*VOID INRPT FAR handle_break(VOID); */
VOID tmark(VOID);
BOOL tdelay(LONG);
BYTE FAR *device_end(VOID);
COUNT kb_data(VOID);
COUNT kb_input(VOID);
COUNT kb_init(VOID);
VOID setvec(UWORD, VOID(INRPT FAR *) ());
BYTE FAR *getvec(UWORD);
COUNT con(COUNT);
VOID getdirent(BYTE FAR *, struct dirent FAR *);
VOID putdirent(struct dirent FAR *, BYTE FAR *);
#else
VOID FAR CharMapSrvc();
VOID FAR set_stack();
VOID FAR restore_stack();
WORD execrh();
VOID exit();
/*VOID INRPT FAR handle_break(); */
VOID tmark();
BOOL tdelay();
BYTE FAR *device_end();
COUNT kb_data();
COUNT kb_input();
COUNT kb_init();
VOID setvec();
BYTE FAR *getvec();
COUNT con();
VOID getdirent();
VOID putdirent();
#endif

/*                                                              */
/* special word packing prototypes                              */
/*                                                              */
#ifdef NATIVE
#define getlong(vp, lp) (*(LONG *)(lp)=*(LONG *)(vp))
#define getword(vp, wp) (*(WORD *)(wp)=*(WORD *)(vp))
#define getbyte(vp, bp) (*(BYTE *)(bp)=*(BYTE *)(vp))
#define fgetlong(vp, lp) (*(LONG FAR *)(lp)=*(LONG FAR *)(vp))
#define fgetword(vp, wp) (*(WORD FAR *)(wp)=*(WORD FAR *)(vp))
#define fgetbyte(vp, bp) (*(BYTE FAR *)(bp)=*(BYTE FAR *)(vp))
#define fputlong(lp, vp) (*(LONG FAR *)(vp)=*(LONG FAR *)(lp))
#define fputword(wp, vp) (*(WORD FAR *)(vp)=*(WORD FAR *)(wp))
#define fputbyte(bp, vp) (*(BYTE FAR *)(vp)=*(BYTE FAR *)(bp))
#else
#ifdef PROTO
VOID getword(VOID *, WORD *);
VOID getbyte(VOID *, BYTE *);
VOID fgetlong(VOID FAR *, LONG FAR *);
VOID fgetword(VOID FAR *, WORD FAR *);
VOID fgetbyte(VOID FAR *, BYTE FAR *);
VOID fputlong(LONG FAR *, VOID FAR *);
VOID fputword(WORD FAR *, VOID FAR *);
VOID fputbyte(BYTE FAR *, VOID FAR *);
#else
VOID getword();
VOID getbyte();
VOID fgetlong();
VOID fgetword();
VOID fgetbyte();
VOID fputlong();
VOID fputword();
VOID fputbyte();
#endif
#endif

#ifdef I86
#define setvec(n, isr)  (void)(*(VOID (INRPT FAR * FAR *)())(4 * (n)) = (isr))
#endif
/*#define is_leap_year(y) ((y) & 3 ? 0 : (y) % 100 ? 1 : (y) % 400 ? 0 : 1) */

/* ^Break handling */
void spawn_int23(void);         /* procsupt.asm */
int control_break(void);        /* break.c */
void handle_break(void);        /* break.c */


GLOBAL BYTE ReturnAnyDosVersionExpected;
