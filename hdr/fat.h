/****************************************************************/
/*                                                              */
/*                            fat.h                             */
/*                                                              */
/*       FAT File System data structures & declarations         */
/*                                                              */
/*                      November 26, 1991                       */
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
static BYTE *fat_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.6  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.5  2000/08/06 04:18:21  jimtabor
 * See history.txt
 *
 * Revision 1.3  2000/05/25 20:56:19  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:28:22  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.2  1999/05/03 06:28:00  jprice
 * Changed some variables from signed to unsigned.
 *
 * Revision 1.1.1.1  1999/03/29 15:39:28  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/01 01:40:06  jprice
 * Clean up
 *
 * Revision 1.3  1999/01/30 08:21:43  jprice
 * Clean up
 *
 * Revision 1.2  1999/01/22 04:17:40  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *         Rev 1.5   04 Jan 1998 23:14:18   patv
 *      Changed Log for strip utility
 *
 *         Rev 1.4   29 May 1996 21:25:14   patv
 *      bug fixes for v0.91a
 *
 *         Rev 1.3   19 Feb 1996  3:15:30   patv
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

/* FAT file system attribute bits                                       */
#define D_NORMAL        0       /* normal                       */
#define D_RDONLY        0x01    /* read-only file               */
#define D_HIDDEN        0x02    /* hidden                       */
#define D_SYSTEM        0x04    /* system                       */
#define D_VOLID         0x08    /* volume id                    */
#define D_DIR           0x10    /* subdir                       */
#define D_ARCHIVE       0x20    /* archive bit                  */
    /* /// Added D_DEVICE bit.  - Ron Cemer */
#define D_DEVICE        0x40    /* device bit                   */

/* FAT file name constants                                              */
#define FNAME_SIZE              8
#define FEXT_SIZE               3

/* FAT deleted flag                                                     */
#define DELETED         0xe5    /* if first char, delete file   */

/* FAT cluster to physical conversion macros                            */
#define clus_add(cl_no)         ((ULONG) (((ULONG) cl_no - 2L) \
                                        * (ULONG) cluster_size \
                                        + (ULONG) data_start))

#define clus2phys(cl_no,cl_size,d_st)   ((ULONG) (((ULONG) cl_no - 2L) \
                                        * (ULONG) cl_size \
                                        + (ULONG) d_st))

/* Test for 16 bit or 12 bit FAT                                        */
#define SIZEOF_CLST16   2
#define FAT_MAGIC       4086
#define FAT_MAGIC16     ((unsigned)65526l)
#define FAT_MAGIC32     268435456l

#define ISFAT32(dpbp)   (((dpbp)->dpb_size)>FAT_MAGIC16 || ((dpbp)->dpb_size)<=FAT_MAGIC32 )
#define ISFAT16(dpbp)   (((dpbp)->dpb_size)>FAT_MAGIC || ((dpbp)->dpb_size)<=FAT_MAGIC16 )
#define ISFAT12(dpbp)   (((dpbp)->dpb_size)<=FAT_MAGIC)

/* FAT file system directory entry                                      */
struct dirent
{
  UBYTE dir_name[FNAME_SIZE];   /* Filename                     */
  UBYTE dir_ext[FEXT_SIZE];     /* Filename extension           */
  UBYTE dir_attrib;             /* File Attribute               */
  BYTE dir_reserved[10];        /* reserved                     */
  time dir_time;                /* Time file created/updated    */
  date dir_date;                /*  Date file created/updated   */
  UWORD dir_start;              /* Starting cluster             */
  /* 1st available = 2            */
  ULONG dir_size;               /* File size in bytes           */
};

/*                                                                      */
/* filesystem sizeof(dirent) - may be different from core               */
/*                                                                      */

#define DIR_NAME        0
#define DIR_EXT         FNAME_SIZE
#define DIR_ATTRIB      FNAME_SIZE+FEXT_SIZE
#define DIR_RESERVED    FNAME_SIZE+FEXT_SIZE+1
#define DIR_TIME        FNAME_SIZE+FEXT_SIZE+11
#define DIR_DATE        FNAME_SIZE+FEXT_SIZE+13
#define DIR_START       FNAME_SIZE+FEXT_SIZE+15
#define DIR_SIZE        FNAME_SIZE+FEXT_SIZE+17

#define DIRENT_SIZE     32
