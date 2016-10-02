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
    "$Id: globals.h 1705 2012-02-07 08:10:33Z perditionc $";
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
#include "dirmatch.h"
#include "fnode.h"
#include "file.h"
#include "clock.h"
#include "kbd.h"
#include "error.h"
#include "version.h"
#include "network.h"
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
#define NAMEMAX         MAX_CDSPATH     /* Maximum path for CDS         */

/* internal error from failure or aborted operation                     */
#define ERROR           -1
#define OK              0

/* internal transfer direction flags                                    */
#define XFR_READ        1
#define XFR_WRITE       2
#define XFR_FORCE_WRITE 3
/* flag to update fcb_rndm field */
#define XFR_FCB_RANDOM  4

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

#define INS             0x5200
#define DEL             0x5300

#define F1              0x3b00
#define F2              0x3c00
#define F3              0x3d00
#define F4              0x3e00
#define F5              0x3f00
#define F6              0x4000
#define LEFT            0x4b00
#define RIGHT           0x4d00

/* Blockio constants                                                    */
#define DSKWRITE        1       /* dskxfr function parameters   */
#define DSKREAD         2
#define DSKWRITEINT26   3
#define DSKREADINT25    4

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
extern COUNT *error_tos,        /* error stack                          */
  disk_api_tos,                 /* API handler stack - disk fns         */
  char_api_tos;                 /* API handler stack - char fns         */
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
extern BYTE ASM os_setver_major,/* editable major version number        */
  ASM os_setver_minor,          /* editable minor version number        */
  ASM os_major,                 /* major version number                 */
  ASM os_minor,                 /* minor version number                 */
  ASM rev_number,               /* minor version number                 */
  ASM version_flags;            /* minor version number                 */

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
    "(C) Copyright 1995-2006 Pasquale J. Villani and The FreeDOS Project.\n"
    "All Rights Reserved. This is free software and comes with ABSOLUTELY NO\n"
    "WARRANTY; you can redistribute it and/or modify it under the terms of the\n"
    "GNU General Public License as published by the Free Software Foundation;\n"
    "either version 2, or (at your option) any later version.\n";

#endif

GLOBAL const BYTE ASM os_release[]
#ifdef MAIN
    = KERNEL_VERSION_STRING
#if 0
    "For technical information and description of the DOS-C operating system\n\
consult \"FreeDOS Kernel\" by Pat Villani, published by Miller\n\
Freeman Publishing, Lawrence KS, USA (ISBN 0-87930-436-7).\n\
\n"
#endif
#endif
 ;

/* Globally referenced variables - WARNING: ORDER IS DEFINED IN         */
/* KERNEL.ASM AND MUST NOT BE CHANGED. DO NOT CHANGE ORDER BECAUSE THEY */
/* ARE DOCUMENTED AS UNDOCUMENTED (?) AND HAVE MANY  PROGRAMS AND TSRs  */
/* ACCESSING THEM                                                       */

extern UWORD ASM NetBios;
extern BYTE * ASM net_name;
extern BYTE ASM net_set_count;
extern BYTE ASM NetDelay, ASM NetRetry;

extern UWORD ASM first_mcb,         /* Start of user memory                 */
  ASM uppermem_root;                /* Start of umb chain (usually 9fff)    */
extern char * ASM inputptr;         /* pointer to unread CON input          */ 
extern sfttbl FAR * ASM sfthead;    /* System File Table head               */
extern struct dhdr
FAR * ASM clock,                    /* CLOCK$ device                        */
  FAR * ASM syscon;                 /* console device                       */
extern WORD ASM maxsecsize;         /* largest sector size in use (can use) */
extern struct buffer
FAR *ASM firstbuf;                  /* head of buffers linked list          */
enum {LOC_CONV=0, LOC_HMA=1};
extern unsigned char ASM bufloc;    /* 0=conv, 1=HMA                        */
extern void far * ASM deblock_buf;  /* pointer to workspace buffer      */
GLOBAL char FAR *firstAvailableBuf;
extern struct cds FAR * ASM CDSp;   /* Current Directory Structure          */
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
  ASM dosidle_flag, ASM Server_Call, ASM CritErrLocus, ASM CritErrAction, 
  ASM CritErrClass, ASM VgaSet, 
  ASM njoined;                      /* number of joined devices             */

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

#define PriPathName _PriPathBuffer._PriPathName

extern struct                   /* Alternate path name parsing buffer   */
{
  BYTE _SecPathName[128];
} ASM _SecPathBuffer;

#define SecPathName _SecPathBuffer._SecPathName

extern UWORD ASM wAttr;

extern BYTE ASM default_drive;      /* default drive for dos                */

extern dmatch ASM sda_tmp_dm;       /* Temporary directory match buffer     */
extern dmatch ASM sda_tmp_dm_ren;   /* 2nd Temporary directory match buffer */
extern BYTE
  ASM internal_data[],              /* sda areas                            */
  ASM swap_always[],                /*  "    "                              */
  ASM swap_indos[],                 /*  "    "                              */
  ASM tsr,                          /* true if program is TSR               */
  ASM break_flg,                    /* true if break was detected           */
  ASM break_ena;                    /* break enabled flag                   */
extern void FAR * ASM dta;          /* Disk transfer area (kludge)          */
extern seg ASM cu_psp;              /* current psp segment                  */
extern iregs FAR * ASM user_r;      /* User registers for int 21h call      */

extern struct dirent            /* Temporary directory entry            */
  ASM DirEntBuffer;

extern fcb FAR * ASM sda_lpFcb;     /* Pointer to users fcb                 */

extern sft FAR * ASM lpCurSft;

extern BYTE ASM verify_ena,         /* verify enabled flag                  */
  ASM switchar;                     /* switch char                          */
extern UWORD ASM return_code;       /* Process termination rets             */

extern UBYTE ASM BootDrive,         /* Drive we came up from                */
  ASM CPULevel,                     /* CPU family, 0=8086, 1=186, ...       */
  ASM scr_pos;                      /* screen position for bs, ht, etc      */
/*extern WORD
  NumFloppies; !!*//* How many floppies we have            */

extern keyboard ASM kb_buf;
extern char ASM local_buffer[LINEBUFSIZE0A];
extern UBYTE DiskTransferBuffer[/*SEC_SIZE*/];

extern struct cds
  ASM TempCDS;

/* start of uncontrolled variables                                      */

#ifdef DEBUG
GLOBAL iregs error_regs;        /* registers for dump                   */

GLOBAL WORD dump_regs;          /* dump registers of bad call           */

#endif

/*                                                                      */
/* Function prototypes - automatically generated                        */
/*                                                                      */
#include "proto.h"

/* Process related functions - not under automatic generation.  */
/* Typically, these are in ".asm" files.                        */
VOID ASMCFUNC FAR cpm_entry(VOID)
/*INRPT FAR handle_break(VOID) */ ;
COUNT ASMCFUNC
    CriticalError(COUNT nFlag, COUNT nDrive, COUNT nError,
                           struct dhdr FAR * lpDevice);

VOID ASMCFUNC FAR CharMapSrvc(VOID);
#if 0
VOID ASMCFUNC FAR set_stack(VOID);
VOID ASMCFUNC FAR restore_stack(VOID);
#endif
/*VOID INRPT FAR handle_break(VOID); */

ULONG ASMPASCAL ReadPCClock(VOID);
VOID ASMPASCAL WriteATClock(BYTE *, BYTE, BYTE, BYTE);
VOID ASMPASCAL WritePCClock(ULONG);
intvec getvec(unsigned char);
#ifdef __WATCOMC__
#pragma aux (pascal) ReadPCClock modify exact [ax cx dx]
#pragma aux (pascal) WriteATClock modify exact [ax bx cx dx]
#pragma aux (pascal) WritePCClock modify exact [ax cx dx]
#endif

/*                                                              */
/* special word packing prototypes                              */
/*                                                              */
#ifdef NATIVE
#define getlong(vp) (*(UDWORD *)(vp))
#define getword(vp) (*(UWORD *)(vp))
#define getbyte(vp) (*(UBYTE *)(vp))
#define fgetlong(vp) (*(UDWORD FAR *)(vp))
#define fgetword(vp) (*(UWORD FAR *)(vp))
#define fgetbyte(vp) (*(UBYTE FAR *)(vp))
#define fputlong(vp, l) (*(UDWORD FAR *)(vp)=l)
#define fputword(vp, w) (*(UWORD FAR *)(vp)=w)
#define fputbyte(vp, b) (*(UBYTE FAR *)(vp)=b)
#else
UDWORD getlong(VOID *);
UWORD getword(VOID *);
UBYTE getbyte(VOID *);
UDWORD fgetlong(VOID FAR *);
UWORD fgetword(VOID FAR *);
UBYTE fgetbyte(VOID FAR *);
VOID fputlong(VOID FAR *, UDWORD);
VOID fputword(VOID FAR *, UWORD);
VOID fputbyte(VOID FAR *, UBYTE);
#endif

#ifndef __WATCOMC__
#define setvec setvec_resident
#endif
void setvec(unsigned char intno, intvec vector);
/*#define is_leap_year(y) ((y) & 3 ? 0 : (y) % 100 ? 1 : (y) % 400 ? 0 : 1) */

/* ^Break handling */
#ifdef __WATCOMC__
#pragma aux (cdecl) spawn_int23 aborts;
#endif
void ASMCFUNC spawn_int23(void);        /* procsupt.asm */
void ASMCFUNC DosIdle_hlt(void);        /* dosidle.asm */

GLOBAL BYTE ASM ReturnAnyDosVersionExpected;
GLOBAL BYTE ASM HaltCpuWhileIdle;

/* near fnodes:
 * fnode[0] is used internally for almost all cases.
 * fnode[1] is only used for:
 * 1) rename (target)
 * 2) rmdir (checks if the directory to remove is empty)
 * 3) commit (copies, than closes fnode[0])
 * 3) merge_file_changes (for SHARE)
 */
GLOBAL struct f_node fnode[2];
