/****************************************************************/
/*                                                              */
/*                           fnode.h                            */
/*                                                              */
/*              Internal File Node for FAT File System          */
/*                                                              */
/*                       January 4, 1992                        */
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
static BYTE *fnode_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.5  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.4  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
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
 * Revision 1.3  2000/03/09 06:06:38  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/16 00:52:10  jprice
 * Optimized FAT handling
 *
 * Revision 1.1.1.1  1999/03/29 15:39:30  jprice
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
 *         Rev 1.4   29 May 1996 21:25:16   patv
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
 *         Rev 1.0   02 Jul 1995 10:39:44   patv
 *      Initial revision.
 */

struct f_node
{
  UWORD f_count;                /* number of uses of this file  */
  COUNT f_mode;                 /* read, write, read-write, etc */

  struct
  {
    BITS f_dmod:1;              /* directory has been modified  */
    BITS f_droot:1;             /* directory is the root        */
    BITS f_dnew:1;              /* fnode is new and needs fill  */
    BITS f_ddir:1;              /* fnode is assigned to dir     */
    BITS f_dfull:1;             /* directory is full            */
    BITS f_dremote:1;           /* Remote Fake FNode            */
  }
  f_flags;                      /* file flags                   */

  struct dirent f_dir;          /* this file's dir entry image  */

  ULONG f_diroff;               /* offset of the dir entry      */
  UWORD f_dirstart;             /* the starting cluster of dir  */
  /* when dir is not root         */
  struct dpb FAR *f_dpb;        /* the block device for file    */

  ULONG f_dsize;                /* file size (for directories)  */
  ULONG f_offset;               /* byte offset for next op      */
  ULONG f_highwater;            /* the largest offset ever      */
  UWORD f_back;                 /* the cluster we were at       */
  ULONG f_cluster_offset;       /* byte offset that the next 3 point to */
  UWORD f_cluster;              /* the cluster we are at        */
  UWORD f_sector;               /* the sector in the cluster    */
  UWORD f_boff;                 /* the byte in the cluster      */
};

