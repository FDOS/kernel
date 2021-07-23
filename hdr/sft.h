/****************************************************************/
/*                                                              */
/*                            sft.h                             */
/*                            DOS-C                             */
/*                                                              */
/*                 DOS System File Table Structure              */
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

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *sft_hRcsId =
    "$Id: sft.h 1429 2009-06-09 18:50:03Z bartoldeman $";
#endif
#endif

#define SFTMAX  128

/* Handle Definition entry                                              */
typedef struct {
  WORD sft_count;               /* 00 - reference count                      */
  WORD sft_mode;                /* 02 - open mode - see below                */
  BYTE sft_attrib;              /* 04 - file attribute - dir style           */

  union                         /* 05 */
  {
    WORD _sft_flags;
    struct {
      BYTE _sft_flags_lo;
      BYTE _sft_flags_hi;
    } _split_sft_flags;
  } sft_flags_union;

  union                         /* 07 */
  {
    struct dpb FAR *_sft_dcb;   /* The device control block     */
    struct dhdr FAR *_sft_dev;  /* device driver for char dev   */
  } sft_dcb_or_dev;
#ifdef WITHFAT32
  UWORD sft_relclust_high;      /* 0b - High part of relative cluster        */
#else
  CLUSTER sft_stclust;          /* 0b - Starting cluster                     */
#endif
  time sft_time;                /* 0d - File time                            */
  date sft_date;                /* 0f - File date                            */
  ULONG sft_size;               /* 11 - File size                            */
  ULONG sft_posit;              /* 15 - Current file position                */
  UWORD sft_relclust;           /* 19 - File relative cluster (low part)     */
  ULONG sft_dirsector;          /* 1b - Sector containing cluster            */
  UBYTE sft_diridx;             /* 1f - directory index                      */
  BYTE sft_name[11];            /* 20 - dir style file name                  */
#ifdef WITHFAT32
  CLUSTER sft_stclust;          /* 2b - Starting cluster                     */
#else
  BYTE FAR *sft_bshare;         /* 2b - backward link of file sharing sft    */
#endif
  WORD sft_mach;                /* 2f - machine number - network apps        */
  WORD sft_psp;                 /* 31 - owner psp                            */
  WORD sft_shroff;              /* 33 - Sharing offset                       */
  CLUSTER sft_cuclust;          /* 35 - File current cluster                 */
#ifdef WITHFAT32
  UWORD sft_pad;
#else
  BYTE FAR *sft_ifsptr;         /* 37 - pointer to IFS driver for file, 0000000h if native DOS */
#endif
} sft;

/* SFT Table header definition                                          */
typedef struct _sftheader {
  struct sfttbl FAR *           /* link to next table in list   */
    sftt_next;
  WORD sftt_count;              /* # of handle definition       */
  /* entries, this table          */
} sftheader;

/* System File Definition List                                          */
typedef struct sfttbl {
  struct sfttbl FAR *           /* link to next table in list   */
    sftt_next;
  WORD sftt_count;              /* # of handle definition       */
  /* entries, this table          */
  sft sftt_table[SFTMAX];       /* The array of sft for block   */
} sfttbl;

/* defines for sft use                                                  */
#define SFT_MASK        0x0060  /* splits device data           */

/* flag bits                                                            */

/* the following bit is for redirection                                 */
#define SFT_FSHARED     0x8000  /* Networked access             */

/* the following entry differntiates char & block access                */
#define SFT_FDEVICE     0x0080  /* device entry                 */

/* the following bits are file (block) unique                           */
#define SFT_FDATE       0x4000  /* File date set                */
#define SFT_FFIXEDMEDIA 0x0800  /* File on non-removable media - unused */
#define SFT_FCLEAN      0x0040  /* File has not been written to */
#define SFT_FDMASK      0x003f  /* File mask for drive no       */

/* the following bits are device (char) unique                          */
#define SFT_FIOCTL      0x4000  /* IOCTL support - device       */
#define SFT_FOCRM       0x0800  /* Open/Close/RM bit in device attribute*/
#define SFT_FEOF        0x0040  /* device eof                   */
#define SFT_FBINARY     0x0020  /* device binary mode           */
#define SFT_FSPECIAL    0x0010  /* int 29 support               */
#define SFT_FCLOCK      0x0008  /* device is clock              */
#define SFT_FNUL        0x0004  /* device is nul                */
#define SFT_FCONOUT     0x0002  /* device is console output     */
#define SFT_FCONIN      0x0001  /* device is console input      */

/* Convenience defines                                                   */
#define sft_dcb         sft_dcb_or_dev._sft_dcb
#define sft_dev         sft_dcb_or_dev._sft_dev

#define sft_flags   sft_flags_union._sft_flags
#define sft_flags_hi  sft_flags_union._split_sft_flags._sft_flags_hi
#define sft_flags_lo  sft_flags_union._split_sft_flags._sft_flags_lo

/* defines for LSEEK */
#define SEEK_SET 0u
#define SEEK_CUR 1u
#define SEEK_END 2u
