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
    "$Id$";
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
  WORD sft_stclust;             /* 0b - Starting cluster                     */
  time sft_time;                /* 0d - File time                            */
  date sft_date;                /* 0f - File date                            */
  LONG sft_size;                /* 11 - File size                            */
  LONG sft_posit;               /* 15 - Current file position                */
  WORD sft_relclust;            /* 19 - File relative cluster                */
  WORD sft_cuclust;             /* 1b - File current cluster                 */
  WORD sft_dirdlust;            /* 1d - Sector containing cluster            */
  BYTE sft_diridx;              /* 1f - directory index                      */
  BYTE sft_name[11];            /* 20 - dir style file name                  */
  BYTE FAR *sft_bshare;         /* 2b - backward link of file sharing sft    */
  WORD sft_mach;                /* 2f - machine number - network apps        */
  WORD sft_psp;                 /* 31 - owner psp                            */
  WORD sft_shroff;              /* 33 - Sharing offset                       */
  WORD sft_status;              /* 35 - this sft status                      */
  BYTE FAR *sft_ifsptr;         /* 37 - pointer to IFS driver for file, 0000000h if native DOS */
} sft;

/* SFT Table header definition                                          */
typedef struct _sftheader {
  struct _sfttbl FAR *          /* link to next table in list   */
    sftt_next;
  WORD sftt_count;              /* # of handle definition       */
  /* entries, this table          */
} sftheader;

/* System File Definition List                                          */
typedef struct _sfttbl {
  struct _sfttbl FAR *          /* link to next table in list   */
    sftt_next;
  WORD sftt_count;              /* # of handle definition       */
  /* entries, this table          */
  sft sftt_table[SFTMAX];       /* The array of sft for block   */
} sfttbl;

/* defines for sft use                                                  */
#define SFT_MASK        0x0060  /* splits device data           */

/* mode bits                                                            */
#define SFT_MFCB        0x8000  /* entry is for fcb             */
#define SFT_MDENYNONE   0x0040  /* sharing bits                 */
#define SFT_MDENYREAD   0x0030  /*     "      "                 */
#define SFT_MDENYWRITE  0x0020  /*     "      "                 */
#define SFT_MEXCLUSIVE  0x0010  /*     "      "                 */
#define SFT_NOINHERIT   0x0080  /* inhibit inherting of file    */
#define SFT_NETFCB      0x0070  /* networked fcb                */
#define SFT_MSHAREMASK  0x0070  /* mask to isolate shared bits  */
#define SFT_MRDWR       0x0002  /* read/write bit               */
#define SFT_MWRITE      0x0001  /* write bit                    */
#define SFT_MREAD       0x0000  /* ~ write bit                  */
#define SFT_OMASK       0xfff3  /* valid open mask              */

/* flag bits                                                            */

/* the following bit is for redirection                                 */
#define SFT_FSHARED     0x8000  /* Networked access             */

/* the following entry differntiates char & block access                */
#define SFT_FDEVICE     0x0080  /* device entry                 */

/* the following bits are file (block) unique                           */
#define SFT_FDATE       0x4000  /* File date set                */
#define SFT_FDIRTY      0x0040  /* File has been written to     */
#define SFT_FDMASK      0x003f  /* File mask for drive no       */

/* the following bits are device (char) unique                          */
#define SFT_FIOCTL      0x4000  /* IOCTL support - device       */
#define SFT_FEOF        0x0040  /* device eof                   */
#define SFT_FBINARY     0x0020  /* device binary mode           */
#define SFT_FSPECIAL    0x0010  /* int 29 support               */
#define SFT_FCLOCK      0x0008  /* device is clock              */
#define SFT_FNUL        0x0004  /* device is nul                */
#define SFT_FCONOUT     0x0002  /* device is console output     */
#define SFT_FCONIN      0x0001  /* device is console input      */

/* Covienence defines                                                   */
#define sft_dcb         sft_dcb_or_dev._sft_dcb
#define sft_dev         sft_dcb_or_dev._sft_dev

#define sft_flags   sft_flags_union._sft_flags
#define sft_flags_hi  sft_flags_union._split_sft_flags._sft_flags_hi
#define sft_flags_lo  sft_flags_union._split_sft_flags._sft_flags_lo

/*
 * Log: sft.h,v 
 *
 * Revision 1.3  2000/03/09 06:06:38  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/04 18:50:25  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:39:35  jprice
 * New version without IPL.SYS
 *
 * Revision 1.3  1999/02/01 01:40:06  jprice
 * Clean up
 *
 * Revision 1.2  1999/01/22 04:17:40  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *         Rev 1.6   04 Jan 1998 23:14:18   patv
 *      Changed Log for strip utility
 *
 *         Rev 1.5   16 Jan 1997 12:46:04   patv
 *      pre-Release 0.92 feature additions
 *
 *         Rev 1.4   29 May 1996 21:25:18   patv
 *      bug fixes for v0.91a
 *
 *         Rev 1.3   19 Feb 1996  3:15:34   patv
 *      Added NLS, int2f and config.sys processing
 *
 *         Rev 1.2   01 Sep 1995 17:35:44   patv
 *      First GPL release.
 *
 *         Rev 1.1   30 Jul 1995 20:43:50   patv
 *      Eliminated version strings in ipl
 *
 *         Rev 1.0   02 Jul 1995 10:39:52   patv
 *      Initial revision.
 */
