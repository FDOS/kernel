/****************************************************************/
/*                                                              */
/*                          process.h                           */
/*                                                              */
/*            DOS exec data structures & declarations           */
/*                                                              */
/*                      November 23, 1991                       */
/*                                                              */
/*                      Copyright (c) 1995                      */
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

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *process_hRcsId =
    "$Id$";
#endif
#endif

/*  Modes available as first argument to the spawnxx functions. */

#define P_WAIT    0             /* child runs separately, parent waits until exit */
#define P_NOWAIT  1             /* both concurrent -- not implemented */
#define P_OVERLAY 2             /* child replaces parent, parent no longer exists */

typedef struct {
  union {
    struct {
      UWORD load_seg;
      UWORD reloc;
    } _load;
    struct {
      UWORD env_seg;
      CommandTail FAR *cmd_line;
      fcb FAR *fcb_1;
      fcb FAR *fcb_2;
      BYTE FAR *stack;
      BYTE FAR *start_addr;
    } _exec;
  } ldata;
} exec_blk;

#define exec    ldata._exec
#define load    ldata._load

typedef struct {
  UWORD ps_exit;                /* 00 CP/M-like exit point: int 20 */
  UWORD ps_size;                /* 02 segment of first byte beyond */
                                /*    memory allocated to program  */
  BYTE ps_fill1;                /* 04 single char fill             */

  /* CP/M-like entry point                                */
  UBYTE ps_farcall;             /* 05  far call opcode              */
  VOID(FAR ASMCFUNC * ps_reentry) (void);  /* 06  re-entry point          */
  intvec ps_isv22,	        /* 0a  terminate address */
         ps_isv23,        	/* 0e break address   */
         ps_isv24;        	/* 12 critical error address */
  UWORD ps_parent;              /* 16 parent psp segment           */
  UBYTE ps_files[20];           /* 18 file table - 0xff is unused  */
  UWORD ps_environ;             /* 2c environment paragraph        */
  BYTE FAR *ps_stack;           /* 2e user stack pointer - int 21  */
  UWORD ps_maxfiles;            /* 32 maximum open files           */
  UBYTE FAR *ps_filetab;        /* 34 open file table pointer      */
  VOID FAR *ps_prevpsp;         /* 38 previous psp pointer         */
  BYTE ps_fill2[20];            /* 3c */
  UBYTE ps_unix[3];             /* 50 unix style call - 0xcd 0x21 0xcb */
  BYTE ps_fill3[9];             /* 53 */
  union {
    struct {
      fcb _ps_fcb1;             /* 5c first command line argument */
    } _u1;
    struct {
      BYTE fill4[16];
      fcb _ps_fcb2;             /* second command line argument */
    } _u2;
    struct {
      BYTE fill5[36];
      CommandTail _ps_cmd;
    } _u3;
  } _u;
} psp;

#define ps_fcb1 _u._u1._ps_fcb1
#define ps_fcb2 _u._u2._ps_fcb2
#define ps_cmd  _u._u3._ps_cmd

