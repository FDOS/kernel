/****************************************************************/
/*                                                              */
/*                            fcb.h                             */
/*                                                              */
/*    FAT FCB and extended FCB data structures & declarations   */
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
static BYTE *fcb_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.1  2000/05/06 19:34:48  jhall1
 * Initial revision
 *
 * Revision 1.2  1999/09/20 21:24:54  jprice
 * *** empty log message ***
 *
 * Revision 1.1.1.1  1999/03/29 15:39:29  jprice
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
 *         Rev 1.6   04 Jan 1998 23:14:16   patv
 *      Changed Log for strip utility
 *
 *         Rev 1.5   16 Jan 1997 12:46:06   patv
 *      pre-Release 0.92 feature additions
 *
 *         Rev 1.4   29 May 1996 21:25:14   patv
 *      bug fixes for v0.91a
 *
 *         Rev 1.3   19 Feb 1996  3:15:32   patv
 *      Added NLS, int2f and config.sys processing
 *
 *         Rev 1.2   01 Sep 1995 17:35:42   patv
 *      First GPL release.
 *
 *         Rev 1.1   30 Jul 1995 20:43:48   patv
 *      Eliminated version strings in ipl
 *
 *         Rev 1.0   02 Jul 1995 10:39:40   patv
 *      Initial revision.
 */

/* fcb convience defines                                                */
/* block device info                                                    */
#define FID_CHARDEV     0x80    /* 1 defines character device   */
                                        /* 0 defines block file         */
#define FID_NOWRITE     0x40    /* 0 file dirty (write occured) */
                                        /* 1 file has no changes        */
#define FID_MASK        0x3f    /* file #                       */
/* char device info                                                     */
#define FID_EOF         0x40    /* 1 = no eof detected          */
                                        /* 0 = end of file on input     */
#define FID_BINARY      0x20    /* 1 = binary (raw) mode device */
                                        /* 0 = ascii (cooked) mode device */
#define FID_CLOCK       0x08    /* Clock device                 */
#define FID_NULL        0x04    /* Null device                  */
#define FID_CONOUT      0x02    /* Console output device        */
#define FID_CONIN       0x01    /* Console input device         */

#ifndef FNAME_SIZE
#define FNAME_SIZE      8       /* limit on file name           */
#endif

#ifndef FEXT_SIZE
#define FEXT_SIZE       3       /* limit on extension           */
#endif

#ifndef FDFLT_DRIVE
#define FDFLT_DRIVE     0       /* default drive                */
#endif

#define PARSE_SEP_STOP          0x01
#define PARSE_DFLT_DRIVE        0x02
#define PARSE_BLNK_FNAME        0x04
#define PARSE_BLNK_FEXT         0x08

#define PARSE_RET_NOWILD        0
#define PARSE_RET_WILD          1
#define PARSE_RET_BADDRIVE      0xff

#define FCB_READ  0
#define FCB_WRITE 1

/* File Control Block (FCB)                                             */
typedef struct
{
  BYTE fcb_drive;               /* Drive number 0=default, 1=A, etc     */
  BYTE fcb_fname[FNAME_SIZE];   /* File name                    */
  BYTE fcb_fext[FEXT_SIZE];     /* File name Extension          */
  UWORD fcb_cublock;            /* Current block number of              */
  /* 128 records/block, for seq. r/w      */
  UWORD fcb_recsiz;             /* Logical record size in bytes,        */
  /* default = 128                        */
  ULONG fcb_fsize;              /* File size in bytes                   */
  date fcb_date;                /* Date file created                    */
  time fcb_time;                /* Time of last write                   */
  /* the following are reserved by system                         */
  BYTE fcb_sftno;               /* Device ID                            */
  BYTE fcb_attrib_hi;           /* share info, dev attrib word hi       */
  BYTE fcb_attrib_lo;           /* dev attrib word lo, open mode        */
  UWORD fcb_strtclst;           /* file starting cluster                */
  UWORD fcb_dirclst;            /* cluster of the dir entry             */
  UBYTE fcb_diroff;             /* offset of the dir entry              */
  /* end reserved                                                 */
  UBYTE fcb_curec;              /* Current block number of              */
  ULONG fcb_rndm;               /* Current relative record number       */
}
fcb;

/* FAT extended fcb                                                     */
typedef struct
{
  UBYTE xfcb_flag;              /* 0xff indicates Extended FCB  */
  BYTE xfcb_resvrd[5];          /* Reserved                     */
  UBYTE xfcb_attrib;            /* Attribute                    */
  fcb xfcb_fcb;
}
xfcb;

typedef struct
{
  UBYTE renDriveID;             /* drive no.                    */
  BYTE renOldName[8];           /* Old Filename                 */
  BYTE renOldExtent[3];         /* Old File Extension           */
  BYTE renReserved1[5];
  BYTE renNewName[8];           /* New Filename                 */
  BYTE renNewExtent[3];         /* New FileExtension            */
  BYTE renReserved2[9];
}
rfcb;
