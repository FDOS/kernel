/****************************************************************/
/*                                                              */
/*                           lol.h                              */
/*                                                              */
/*              DOS List of Lists structure                     */
/*                                                              */
/*                      Copyright (c) 2003                      */
/*                         Bart Oldeman                         */
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
/* License along with DOS-C; if not, write to the Free Software */
/* Foundation, Inc., 59 Temple Place, Suite 330,                */
/* Boston, MA  02111-1307  USA.                                 */
/****************************************************************/

enum {LOC_CONV=0, LOC_HMA=1};

/* note: we start at DOSDS:0, but the "official" list of lists starts a
   little later at DOSDS:26 (this is what is returned by int21/ah=52) */

struct lol {
  char filler[0x22];
  char *inputptr;              /* -4 Pointer to unread CON input          */
  unsigned short first_mcb;    /* -2 Start of user memory                 */
  struct dpb far *DPBp;        /*  0 First drive Parameter Block          */
  struct sfttbl far *sfthead;  /*  4 System File Table head               */
  struct dhdr far *clock;      /*  8 CLOCK$ device                        */
  struct dhdr far *syscon;     /*  c console device                       */
  unsigned short maxsecbytes;  /* 10 max bytes per sector for any blkdev  */
  void far *inforecptr;        /* 12 pointer to disk buffer info record   */
  struct cds far *CDSp;        /* 16 Current Directory Structure          */
  struct sfttbl far *FCBp;     /* 1a FCB table pointer                    */
  unsigned short nprotfcb;     /* 1e number of protected fcbs             */
  unsigned char nblkdev;       /* 20 number of block devices              */
  unsigned char lastdrive;     /* 21 value of last drive                  */
  struct dhdr nul_dev;         /* 22 NUL device driver header(no pointer!)*/
  unsigned char njoined;       /* 34 number of joined devices             */
  unsigned short specialptr;   /* 35 pointer to list of spec. prog(unused)*/
  void far *setverPtr;         /* 37 pointer to SETVER list               */
  void (*a20ptr)(void);        /* 3b pointer to fix A20 ctrl              */
  unsigned short recentpsp;    /* 3d PSP of most recently exec'ed prog    */
  unsigned short nbuffers;     /* 3f Number of buffers                    */
  unsigned short nlookahead;   /* 41 Number of lookahead buffers          */
  unsigned char BootDrive;     /* 43 bootdrive (1=A:)                     */
  unsigned char dwordmoves;    /* 44 use dword moves (unused)             */
  unsigned short xmssize;      /* 45 extended memory size in KB           */ 
  struct buffer far *firstbuf; /* 47 head of buffers linked list          */
  unsigned short dirtybuf;     /* 4b number of dirty buffers              */
  struct buffer far *lookahead;/* 4d pointer to lookahead buffer          */
  unsigned short slookahead;   /* 51 number of lookahead sectors          */
  unsigned char bufloc;        /* 53 BUFFERS loc (1=HMA)                  */
  char far *deblock_buf;       /* 54 pointer to workspace buffer          */
  char filler2[5];             /* 58 ???/unused                           */
  unsigned char int24fail;     /* 5d int24 fail while making i/o stat call*/
  unsigned char memstrat;      /* 5e memory allocation strat during exec  */
  unsigned char a20count;      /* 5f nr. of int21 calls for which a20 off */
  unsigned char VgaSet;        /* 60 bitflags switches=/w, int21/4b05     */
  unsigned short unpack;       /* 61 offset of unpack code start          */
  unsigned char uppermem_link; /* 63 UMB Link flag                        */
  unsigned short min_pars;     /* 64 minimum para req by program execed   */
  unsigned short uppermem_root;/* 66 Start of umb chain (usually 9fff)    */
  unsigned short last_para;    /* 68 para: start scanning during memalloc */
  /* FreeDOS specific entries */
  unsigned char os_setver_minor;/*6a settable minor DOS version           */
  unsigned char os_setver_major;/*6b settable major DOS version           */
  unsigned char os_minor;      /* 6c minor DOS version                    */
  unsigned char os_major;      /* 6d major DOS version                    */
  unsigned char rev_number;    /* 6e minor DOS version                    */
  unsigned char version_flags; /* 6f DOS version flags                    */
  struct f_node FAR *f_nodes;  /* 70 pointer to the array                 */
  unsigned short f_nodes_cnt;  /* 74 number of allocated f_nodes          */
  char *os_release;            /* 76 near pointer to os_release string    */
};
