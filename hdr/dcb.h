/****************************************************************/
/*                                                              */
/*                            dcb.h                             */
/*                                                              */
/*                DOS Device Control Block Structure            */
/*                                                              */
/*                       November 20, 1991                      */
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
static BYTE *clock_hRcsId =
    "$Id$";
#endif
#endif

/* Internal drive parameter block                               */
struct dpb {
  BYTE dpb_unit;                /* unit for error reporting     */
  BYTE dpb_subunit;             /* the sub-unit for driver      */
  UWORD dpb_secsize;            /* sector size                  */
  UBYTE dpb_clsmask;            /* mask (sectors/cluster-1)     */
  UBYTE dpb_shftcnt;            /* log base 2 of cluster size   */
  UWORD dpb_fatstrt;            /* FAT start sector             */
  UBYTE dpb_fats;               /* # of FAT copies              */
  UWORD dpb_dirents;            /* # of dir entries             */
  UWORD dpb_data;               /* start of data area           */
  UWORD dpb_size;               /* # of clusters+1 on media     */
  UWORD dpb_fatsize;            /* # of sectors / FAT           */
  UWORD dpb_dirstrt;            /* start sec. of root dir       */
  struct dhdr FAR *             /* pointer to device header     */
    dpb_device;
  UBYTE dpb_mdb;                /* media descr. byte            */
  BYTE dpb_flags;               /* -1 = force MEDIA CHK         */
  struct dpb FAR *              /* next dpb in chain            */
    dpb_next;                   /* -1 = end                     */
  UWORD dpb_cluster;            /* cluster # of first free      */
  /* -1 if not known              */
#ifndef WITHFAT32
  UWORD dpb_nfreeclst;          /* number of free clusters      */
  /* -1 if not known              */
#else
  union {
    struct {
      UWORD dpb_nfreeclst_lo;
      UWORD dpb_nfreeclst_hi;
    } dpb_nfreeclst_st;
    ULONG _dpb_xnfreeclst;      /* number of free clusters      */
    /* -1 if not known              */
  } dpb_nfreeclst_un;
#define dpb_nfreeclst dpb_nfreeclst_un.dpb_nfreeclst_st.dpb_nfreeclst_lo
#define dpb_xnfreeclst dpb_nfreeclst_un._dpb_xnfreeclst

  UWORD dpb_xflags;             /* extended flags, see bpb      */
  UWORD dpb_xfsinfosec;         /* FS info sector number,       */
  /* 0xFFFF if unknown            */
  UWORD dpb_xbackupsec;         /* backup boot sector number    */
  /* 0xFFFF if unknown            */
  ULONG dpb_xdata;
  ULONG dpb_xsize;              /* # of clusters+1 on media     */
  ULONG dpb_xfatsize;           /* # of sectors / FAT           */
  CLUSTER dpb_xrootclst;        /* starting cluster of root dir */
  CLUSTER dpb_xcluster;         /* cluster # of first free      */
  /* -1 if not known              */
#endif
};

#define UNKNCLUSTER      0x0000 /* see RBIL INT 21/AH=52 entry */
#define XUNKNCLSTFREE    0xffffffffl    /* unknown for DOS */
#define UNKNCLSTFREE     0xffff /* unknown for DOS */
