/****************************************************************/
/*                                                              */
/*                          dirmatch.h                          */
/*                                                              */
/*               FAT File System Match Data Structure           */
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
static BYTE *dirmatch_hRcsId =
    "$Id$";
#endif
#endif

typedef struct {
  UBYTE dm_drive;
  BYTE dm_name_pat[FNAME_SIZE + FEXT_SIZE];
  BYTE dm_attr_srch;
  UWORD dm_entry;
#ifdef WITHFAT32
  ULONG dm_dircluster;
#else
  UWORD dm_dircluster;
  UWORD reserved;
#endif

  struct {
    BITS                        /* directory has been modified  */
    f_dmod:1;
    BITS                        /* directory is the root        */
    f_droot:1;
    BITS                        /* fnode is new and needs fill  */
    f_dnew:1;
    BITS                        /* fnode is assigned to dir     */
    f_ddir:1;
    BITS                        /* filler to avoid a bad bug (feature?) in */
    f_filler:12;                /* TC 2.01           */
  } dm_flags;                   /* file flags                   */

  BYTE dm_attr_fnd;             /* found file attribute         */
  time dm_time;                 /* file time                    */
  date dm_date;                 /* file date                    */
  LONG dm_size;                 /* file size                    */
  BYTE dm_name[FNAME_SIZE + FEXT_SIZE + 2];     /* file name    */
} dmatch;

