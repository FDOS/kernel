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
static BYTE *fnode_hRcsId =
    "$Id$";
#endif
#endif

struct f_node {
  UWORD f_count;                /* number of uses of this file  */
  COUNT f_mode;                 /* read, write, read-write, etc */

  struct {
    BITS f_dmod:1;              /* directory has been modified  */
    BITS f_droot:1;             /* directory is the root        */
    BITS f_dnew:1;              /* fnode is new and needs fill  */
    BITS f_ddir:1;              /* fnode is assigned to dir     */
    BITS f_ddate:1;             /* date set using setdate       */
  } f_flags;                    /* file flags                   */

  struct dirent f_dir;          /* this file's dir entry image  */

  ULONG f_diroff;               /* offset of the dir entry      */
  CLUSTER f_dirstart;           /* the starting cluster of dir  */
  /* when dir is not root         */
  struct dpb FAR *f_dpb;        /* the block device for file    */

  ULONG f_offset;               /* byte offset for next op      */
  ULONG f_highwater;            /* the largest offset ever      */
  CLUSTER f_back;               /* the cluster we were at       */
  CLUSTER f_cluster_offset;     /* relative cluster number within file */
  CLUSTER f_cluster;            /* the cluster we are at        */
  UWORD f_sector;               /* the sector in the cluster    */
  UWORD f_boff;                 /* the byte in the cluster      */
};

typedef struct f_node *f_node_ptr;

struct lfn_inode {
  UNICODE l_name[261];          /* Long file name string          */
                                /* If the string is empty,        */
                                /* then file has the 8.3 name     */
  struct dirent l_dir;          /* Directory entry image          */
  ULONG l_diroff;               /* Current directory entry offset */
};
  
typedef struct lfn_inode FAR * lfn_inode_ptr;
