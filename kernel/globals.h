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

#ifdef VERSION_STRINGS
#ifdef MAIN
static BYTE *Globals_hRcsId =
    "$Id$";
#endif
#endif

#include "device.h"
#include "mcb.h"
#include "pcb.h"
#include "date.h"
#include "time.h"
#include "fat.h"
#include "fcb.h"
#include "tail.h"
#include "process.h"
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
#include "buffer.h"
#include "dcb.h"
#include "xstructs.h"

/* fatfs.c */
#ifdef WITHFAT32
VOID bpb_to_dpb(bpb FAR * bpbp, REG struct dpb FAR * dpbp, BOOL extended);
#else
VOID bpb_to_dpb(bpb FAR * bpbp, REG struct dpb FAR * dpbp);
#endif

#ifdef WITHFAT32
struct dpb FAR *GetDriveDPB(UBYTE drive, COUNT * rc);
#endif

extern struct dpb
FAR * ASM DPBp;                      /* First drive Parameter Block          */


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
#define PARSE_MAX       MAX_CDSPATH     /* maximum # of bytes in path   */
#define NAMEMAX         PARSE_MAX       /* Maximum path for CDS         */

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
#define CTL_P           0x10
#define CTL_Q           0x11
#define CTL_S           0x13
#define CTL_Z           0x1a
#define ESC             0x1b
#define CTL_BS          0x7f

#define F1              0x3b
#define F3              0x3d
#define LEFT            0x4b
#define RIGHT           0x4d

/* Blockio constants                                                    */
#define DSKWRITE        1       /* dskxfr function parameters   */
#define DSKREAD         2
#define DSKWRITEINT26   3
#define DSKREADINT25    4

/* FAT cluster special flags                                            */
#define FREE                    0x000

#ifdef WITHFAT32
#define LONG_LAST_CLUSTER       0x0FFFFFFFl
#define LONG_BAD                0x0FFFFFF7l
#else
#define LONG_LAST_CLUSTER       0xFFFF
#define LONG_BAD                0xFFF8
#endif
#define MASK16                  0xFFF8
#define BAD16                   0xFFF0
#define MASK12                  0xFF8
#define BAD12                   0xFF0

/* Keyboard buffer maximum size                                         */
#ifdef LINESIZE
#undef LINESIZE
#endif
#define LINESIZE KBD_MAXLENGTH

/* NLS character table type                                             */
typedef BYTE *UPMAP;

/*                                                                      */
/* External Assembly variables                                          */
/*                                                                      */
extern struct dhdr
FAR ASM clk_dev,                    /* Clock device driver                  */
  FAR ASM con_dev,                  /* Console device driver                */
  FAR ASM prn_dev,                  /* Generic printer device driver        */
  FAR ASM aux_dev,                  /* Generic aux device driver            */
  FAR ASM blk_dev;                  /* Block device (Disk) driver           */
extern UWORD ASM ram_top;           /* How much ram in Kbytes               */
extern COUNT *error_tos,        /* error stack                          */
  disk_api_tos,                 /* API handler stack - disk fns         */
  char_api_tos;                 /* API handler stack - char fns         */
extern BYTE FAR _InitTextStart; /* first available byte of ram          */
extern BYTE FAR _HMATextAvailable,      /* first byte of available CODE area    */
  FAR _HMATextStart[],          /* first byte of HMAable CODE area      */
  FAR _HMATextEnd[];            /* and the last byte of it              */
extern
BYTE DosLoadedInHMA;            /* if InitHMA has moved DOS up          */

extern struct ClockRecord
  ASM ClkRecord;

/*                                                                      */
/* Global variables                                                     */
/*                                                                      */
GLOBAL BYTE os_major,           /* major version number                 */
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

#if 0                           /* defined in MAIN.C now to save low memory */

GLOBAL BYTE copyright[] =
    "(C) Copyright 1995-2001 Pasquale J. Villani and The FreeDOS Project.\n"
    "All Rights Reserved. This is free software and comes with ABSOLUTELY NO\n"
    "WARRANTY; you can redistribute it and/or modify it under the terms of the\n"
    "GNU General Public License as published by the Free Software Foundation;\n"
    "either version 2, or (at your option) any later version.\n";

#endif

GLOBAL BYTE os_release[]
#ifdef MAIN
#if 0
    = "DOS-C version %d.%d Beta %d [FreeDOS Release] (Build %d).\n"
#endif
    = "FreeDOS kernel version " KERNEL_VERSION_STRING
    " (Build " KERNEL_BUILD_STRING ") [" __DATE__ " " __TIME__ "]\n"
#if 0
    "For technical information and description of the DOS-C operating system\n\
consult \"FreeDOS Kernel\" by Pat Villani, published by Miller\n\
Freeman Publishing, Lawrence KS, USA (ISBN 0-87930-436-7).\n\
\n"
#endif
#endif
 ;

/* Globally referenced variables - WARNING: ORDER IS DEFINED IN         */
/* KERNAL.ASM AND MUST NOT BE CHANGED. DO NOT CHANGE ORDER BECAUSE THEY */
/* ARE DOCUMENTED AS UNDOCUMENTED (?) AND HAVE MANY  PROGRAMS AND TSR'S */
/* ACCESSING THEM                                                       */

extern UWORD ASM NetBios;
extern BYTE * ASM net_name;
extern BYTE ASM net_set_count;
extern BYTE ASM NetDelay, ASM NetRetry;

extern UWORD ASM first_mcb,         /* Start of user memory                 */
  ASM UMB_top, ASM umb_start, ASM uppermem_root;    /* Start of umb chain ? */
extern sfttbl FAR * ASM sfthead;    /* System File Table head               */
extern struct dhdr
FAR * ASM clock,                    /* CLOCK$ device                        */
  FAR * ASM syscon;                 /* console device                       */
extern WORD ASM maxbksize;          /* Number of Drives in system           */
extern struct buffer
FAR *ASM firstbuf;                  /* head of buffers linked list          */
extern cdstbl FAR * ASM CDSp;       /* Current Directory Structure          */
extern
struct cds FAR * ASM current_ldt;
extern LONG ASM current_filepos;    /* current file position                */
extern sfttbl FAR * ASM FCBp;       /* FCB table pointer                    */
extern WORD ASM nprotfcb;           /* number of protected fcbs             */
extern UBYTE ASM nblkdev,           /* number of block devices              */
  ASM lastdrive,                    /* value of last drive                  */
  ASM uppermem_link,                /* UMB Link flag */
  ASM PrinterEcho;                  /* Printer Echo Flag                    */

extern UWORD ASM LoL_nbuffers;      /* Number of buffers                    */

extern struct dhdr
  ASM nul_dev;
extern UBYTE ASM mem_access_mode;   /* memory allocation scheme             */
extern BYTE ASM ErrorMode,          /* Critical error flag                  */
  ASM InDOS,                        /* In DOS critical section              */
  ASM OpenMode,                     /* File Open Attributes                 */
  ASM SAttr,                        /* Attrib Mask for Dir Search           */
  ASM dosidle_flag, ASM Server_Call, ASM CritErrLocus, ASM CritErrAction, ASM CritErrClass, VgaSet, njoined;        /* number of joined devices             */

extern UWORD ASM Int21AX;
extern COUNT ASM CritErrCode;
extern BYTE FAR * ASM CritErrDev;

extern struct dirent
  ASM SearchDir;

extern struct {
  COUNT nDrive;
  BYTE szName[FNAME_SIZE + 1];
  BYTE szExt[FEXT_SIZE + 1];
} ASM FcbSearchBuffer;

extern struct                   /* Path name parsing buffer             */
{
  BYTE _PriPathName[128];
} ASM _PriPathBuffer;

extern struct {
  BYTE _fname[FNAME_SIZE];
  BYTE _fext[FEXT_SIZE + 1];    /* space for 0 */
} ASM szNames;

#define PriPathName _PriPathBuffer._PriPathName
#define szDirName TempCDS.cdsCurrentPath
#define szFileName szNames._fname
#define szFileExt szNames._fext

extern struct                   /* Alternate path name parsing buffer   */
{
  BYTE _SecPathName[128];
} ASM _SecPathBuffer;

#define SecPathName _SecPathBuffer._SecPathName

extern UWORD ASM wAttr;

extern BYTE ASM default_drive;      /* default drive for dos                */

extern BYTE ASM TempBuffer[],       /* Temporary general purpose buffer     */
  FAR ASM internal_data[],          /* sda areas                            */
  FAR ASM swap_always[],            /*  "    "                              */
  FAR ASM swap_indos[],             /*  "    "                              */
  ASM tsr,                          /* true if program is TSR               */
  ASM break_flg,                    /* true if break was detected           */
  ASM break_ena;                    /* break enabled flag                   */
extern BYTE FAR * ASM dta;          /* Disk transfer area (kludge)          */
extern seg ASM cu_psp;              /* current psp segment                  */
extern iregs FAR * ASM user_r;      /* User registers for int 21h call      */

extern struct dirent            /* Temporary directory entry            */
  ASM DirEntBuffer;

extern fcb FAR * ASM lpFcb;         /* Pointer to users fcb                 */

extern sft FAR * ASM lpCurSft;

extern BYTE ASM verify_ena,         /* verify enabled flag                  */
  ASM switchar,                     /* switch char                          */
  ASM return_mode,                  /* Process termination rets             */
  ASM return_code;                  /*     "        "       "               */

extern BYTE ASM BootDrive,          /* Drive we came up from                */
  ASM scr_pos;                      /* screen position for bs, ht, etc      */
/*extern WORD
  NumFloppies; !!*//* How many floppies we have            */

extern keyboard ASM kb_buf;

extern struct cds
  ASM TempCDS;

/* start of uncontrolled variables                                      */
GLOBAL seg RootPsp;             /* Root process -- do not abort         */

/* don't know what it should do, but its no longer in use TE
GLOBAL struct f_node
 *pDirFileNode;
*/

#ifdef DEBUG
GLOBAL iregs error_regs;        /* registers for dump                   */

GLOBAL WORD dump_regs;          /* dump registers of bad call           */

#endif

GLOBAL f_node_ptr f_nodes;      /* pointer to the array                 */

GLOBAL UWORD f_nodes_cnt;       /* number of allocated f_nodes          */

/*                                                                      */
/* Function prototypes - automatically generated                        */
/*                                                                      */
#include "proto.h"

/* Process related functions - not under automatic generation.  */
/* Typically, these are in ".asm" files.                        */
VOID ASMCFUNC FAR cpm_entry(VOID)
/*INRPT FAR handle_break(VOID) */ ;
VOID enable(VOID), disable(VOID);
COUNT ASMCFUNC
    CriticalError(COUNT nFlag, COUNT nDrive, COUNT nError,
                           struct dhdr FAR * lpDevice);

#ifdef PROTO
VOID ASMCFUNC FAR CharMapSrvc(VOID);
VOID ASMCFUNC FAR set_stack(VOID);
VOID ASMCFUNC FAR restore_stack(VOID);
/*VOID INRPT FAR handle_break(VOID); */
VOID ASMCFUNC ReadPCClock(ULONG *);
BYTE FAR * ASMCFUNC device_end(VOID);
COUNT ASMCFUNC kb_data(VOID);
COUNT ASMCFUNC kb_input(VOID);
COUNT ASMCFUNC kb_init(VOID);
VOID ASMCFUNC setvec(UWORD, intvec);
intvec ASMCFUNC getvec(UWORD);
COUNT con(COUNT);
#else
VOID FAR CharMapSrvc();
VOID FAR set_stack();
VOID FAR restore_stack();
WORD execrh();
VOID exit();
/*VOID INRPT FAR handle_break(); */
BYTE FAR *device_end();
COUNT kb_data();
COUNT kb_input();
COUNT kb_init();
VOID setvec();
intvec getvec();
COUNT con();
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
#define setvec(n, isr)  (void)(*(intvec FAR *)MK_FP(0,4 * (n)) = (isr))
#endif
/*#define is_leap_year(y) ((y) & 3 ? 0 : (y) % 100 ? 1 : (y) % 400 ? 0 : 1) */

/* ^Break handling */
void ASMCFUNC spawn_int23(void);        /* procsupt.asm */
int control_break(void);        /* break.c */
void handle_break(void);        /* break.c */

GLOBAL BYTE ReturnAnyDosVersionExpected;

GLOBAL COUNT UnusedRetVal;      /* put unused errors here (to save stack space) */

/*
 * Log: globals.h,v 
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
