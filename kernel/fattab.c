/****************************************************************/
/*                                                              */
/*                          fattab.c                            */
/*                                                              */
/*                 FAT File System Table Functions              */
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

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.9  2001/11/04 19:47:39  bartoldeman
 * kernel 2025a changes: see history.txt
 *
 * Revision 1.8  2001/09/23 20:39:44  bartoldeman
 * FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
 *
 * Revision 1.7  2001/07/09 22:19:33  bartoldeman
 * LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
 *
 * Revision 1.6  2001/06/03 14:16:17  bartoldeman
 * BUFFERS tuning and misc bug fixes/cleanups (2024c).
 *
 * Revision 1.5  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.4  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.6  1999/08/10 18:03:42  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.5  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.4  1999/04/16 00:53:33  jprice
 * Optimized FAT handling
 *
 * Revision 1.3  1999/04/11 04:33:38  jprice
 * ror4 patches
 *
 * Revision 1.1.1.1  1999/03/29 15:42:09  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.7   09 Feb 1998  5:41:08   patv
 * Eliminated FAT12 EOF and error returns.
 *
 *    Rev 1.6   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   16 Jan 1997 12:46:22   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.4   29 May 1996 21:15:14   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:20:08   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:48:42   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:04:56   patv
 * Initial revision.
 */

#ifdef PROTO
UCOUNT link_fat12(struct dpb FAR *, CLUSTER, CLUSTER);
UCOUNT link_fat16(struct dpb FAR *, CLUSTER, CLUSTER);
UCOUNT link_fat32(struct dpb FAR *, CLUSTER, CLUSTER);
CLUSTER next_cl12(struct dpb FAR *, CLUSTER);
CLUSTER next_cl16(struct dpb FAR *, CLUSTER);
CLUSTER next_cl32(struct dpb FAR *, CLUSTER);
#else
UCOUNT link_fat12();
UCOUNT link_fat16();
UCOUNT link_fat32();
CLUSTER next_cl12();
CLUSTER next_cl16();
CLUSTER next_cl32();
#endif

/************************************************************************/
/*                                                                      */
/*                      cluster/sector routines                         */
/*                                                                      */
/************************************************************************/

#ifndef ISFAT32
int ISFAT32(struct dpb FAR *dpbp)
{
       return _ISFAT32(dpbp);
}
#endif


struct buffer FAR *getFATblock(CLUSTER cluster, struct dpb FAR *dpbp)
{
  ULONG sector;  
  struct buffer FAR *bp;

  if (ISFAT12(dpbp))
    {
        sector = ((cluster << 1) + cluster) >> 1;
    }
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
		{
        sector = (ULONG)cluster * SIZEOF_CLST32; 			  
		}
#endif
  else
    {
        sector = (ULONG)cluster * SIZEOF_CLST16; 
    }  
  sector = sector / dpbp->dpb_secsize + dpbp->dpb_fatstrt; 
#ifdef WITHFAT32
  if (ISFAT32(dpbp) && (dpbp->dpb_xflags & FAT_NO_MIRRORING)) {
    /* we must modify the active fat,
       it's number is in the 0-3 bits of dpb_xflags */
    sector += (dpbp->dpb_xflags & 0xf) * dpbp->dpb_xfatsize;
  }
#endif
  
  bp = getblock(sector, dpbp->dpb_unit);
  
  if (bp)
  {
    bp->b_flag &= ~(BFR_DATA | BFR_DIR);
    bp->b_flag |= BFR_FAT | BFR_VALID;
    bp->b_copies = dpbp->dpb_fats;
    bp->b_offset = dpbp->dpb_fatsize;
#ifdef WITHFAT32
    if (ISFAT32(dpbp))
    {
      if (dpbp->dpb_xflags & FAT_NO_MIRRORING)
        bp->b_copies = 1;
      bp->b_offset = dpbp->dpb_xfatsize;
    }
#endif
  }
  return bp;  
}

#ifdef WITHFAT32
void read_fsinfo(struct dpb FAR *dpbp)
{
  struct buffer FAR *bp;
  struct fsinfo FAR *fip;

  bp = getblock(dpbp->dpb_xfsinfosec, dpbp->dpb_unit);
  bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT | BFR_DIRTY);
  bp->b_flag |= BFR_VALID;

  fip = (struct fsinfo FAR *) & bp->b_buffer[0x1e4];
  dpbp->dpb_xnfreeclst = fip->fi_nfreeclst;
  dpbp->dpb_xcluster = fip->fi_cluster;
}

void write_fsinfo(struct dpb FAR *dpbp)
{
  struct buffer FAR *bp;
  struct fsinfo FAR *fip;

  bp = getblock(dpbp->dpb_xfsinfosec, dpbp->dpb_unit);
  bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT);
  bp->b_flag |= BFR_VALID | BFR_DIRTY;

  fip = (struct fsinfo FAR *) & bp->b_buffer[0x1e4];
  fip->fi_nfreeclst = dpbp->dpb_xnfreeclst;
  fip->fi_cluster = dpbp->dpb_xcluster;
}
#endif

/*                                                              */
/* The FAT file system is difficult to trace through FAT table. */
/* There are two kinds of FAT's, 12 bit and 16 bit. The 16 bit  */
/* FAT is the easiest, since it is noting more than a series of */
/* UWORD's. The 12 bit FAT is difficult, because it packs 3 FAT */
/* entries into two BYTE's. The are packed as follows:          */
/*                                                              */
/*      0x0003 0x0004 0x0005 0x0006 0x0007 0x0008 0x0009 ...    */
/*                                                              */
/*      are packed as                                           */
/*                                                              */
/*      0x03 0x40 0x00 0x05 0x60 0x00 0x07 0x80 0x00 0x09 ...   */
/*                                                              */
/*      12 bytes are compressed to 9 bytes                      */
/*                                                              */

UCOUNT link_fat(struct dpb FAR *dpbp, CLUSTER Cluster1, REG CLUSTER Cluster2)
{
  UCOUNT res;
      
  if (ISFAT12(dpbp))
    res = link_fat12(dpbp, Cluster1, Cluster2);
  else if (ISFAT16(dpbp))
    res = link_fat16(dpbp, Cluster1, Cluster2);
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
    res = link_fat32(dpbp, Cluster1, Cluster2);
#endif
  else
    return DE_BLKINVLD;


  /* update the free space count                          */

  if (res == SUCCESS && Cluster2 == FREE)
  {
#ifdef WITHFAT32
    if (ISFAT32(dpbp) && dpbp->dpb_xnfreeclst != XUNKNCLSTFREE)
    {
      /* update the free space count for returned     */
      /* cluster					    */
      ++dpbp->dpb_xnfreeclst;
      write_fsinfo(dpbp);
    } else
#endif
    if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
      ++dpbp->dpb_nfreeclst;
  }

        /*if (Cluster2 == FREE)
            { */
            /* update the free space count for returned     */
            /* cluster					    */
            /* ++dpbp->dpb_nfreeclst;
            }*/

	/* update the free space count for removed      */
        /* cluster                                      */
        /* BUG: was counted twice for 2nd,.. cluster. moved to find_fat_free() */
	/* BO: don't completely understand this yet - leave here for now as
           a comment */
	/* else
          {
            --dpbp->dpb_nfreeclst;
          }   */
  return res;  
}

#ifdef WITHFAT32
UCOUNT link_fat32(struct dpb FAR *dpbp, CLUSTER  Cluster1, CLUSTER Cluster2)
{
  UCOUNT idx;
  struct buffer FAR *bp;

  /* Get the block that this cluster is in                */
  bp = getFATblock(Cluster1, dpbp);

  if (bp == NULL)
    return DE_BLKINVLD;

  /* form an index so that we can read the block as a     */
  /* byte array                                           */
  idx = (UWORD)((Cluster1 * SIZEOF_CLST32) % dpbp->dpb_secsize);

  /* Finally, put the word into the buffer and mark the   */
  /* buffer as dirty.                                     */
  fputlong((DWORD FAR *) & Cluster2, (VOID FAR *) & (bp->b_buffer[idx]));
  bp->b_flag |= BFR_DIRTY | BFR_VALID;

  /* Return successful.                                   */

  return SUCCESS;
}

#endif

UCOUNT link_fat16(struct dpb FAR *dpbp, CLUSTER  Cluster1, CLUSTER Cluster2)
{
  UCOUNT idx;
  struct buffer FAR *bp;

  /* Get the block that this cluster is in                */
  bp = getFATblock(Cluster1, dpbp);

  if (bp == NULL)
    return DE_BLKINVLD;

  /* form an index so that we can read the block as a     */
  /* byte array                                           */
  idx = (UWORD)(( Cluster1 * SIZEOF_CLST16) % dpbp->dpb_secsize);

  /* Finally, put the word into the buffer and mark the   */
  /* buffer as dirty.                                     */
  fputword((WORD FAR *) & Cluster2, (VOID FAR *) & (bp->b_buffer[idx]));
  bp->b_flag |= BFR_DIRTY | BFR_VALID;

  /* Return successful.                                   */

  return SUCCESS;
}

UCOUNT link_fat12(struct dpb FAR *dpbp, CLUSTER Cluster1, CLUSTER Cluster2)
{
  REG UBYTE FAR *fbp0,
    FAR * fbp1;
  UCOUNT idx;
  struct buffer FAR *bp,
    FAR * bp1;

  /* Get the block that this cluster is in                */
  bp = getFATblock(Cluster1 , dpbp);
  if (bp == NULL)
    return DE_BLKINVLD;

  /* form an index so that we can read the block as a     */
  /* byte array                                           */
  idx = (UCOUNT)(((Cluster1 << 1) + Cluster1) >> 1) % dpbp->dpb_secsize;

  /* Test to see if the cluster straddles the block. If   */
  /* it does, get the next block and use both to form the */
  /* the FAT word. Otherwise, just point to the next      */
  /* block.                                               */
  if (idx >= dpbp->dpb_secsize - 1)
  {
    bp1 = getFATblock(Cluster1 + 1,dpbp);
    if (bp1 == 0)
      return DE_BLKINVLD;

    bp1->b_flag |= BFR_DIRTY | BFR_VALID;

    fbp1 = (UBYTE FAR *) & (bp1->b_buffer[0]);
  }
  else
    fbp1 = (UBYTE FAR *) & (bp->b_buffer[idx + 1]);
  fbp0 = (UBYTE FAR *) & (bp->b_buffer[idx]);
  bp->b_flag |= BFR_DIRTY | BFR_VALID;

  /* Now pack the value in                                */
  if (Cluster1 & 0x01)
  {
    *fbp0 = (*fbp0 & 0x0f) | ((Cluster2 & 0x0f) << 4);
    *fbp1 = (Cluster2 >> 4) & 0xff;
  }
  else
  {
    *fbp0 = Cluster2 & 0xff;
    *fbp1 = (*fbp1 & 0xf0) | ((Cluster2 >> 8) & 0x0f);
  }

  return SUCCESS;
}

/* Given the disk parameters, and a cluster number, this function
   looks at the FAT, and returns the next cluster in the clain. */
CLUSTER next_cluster(struct dpb FAR *dpbp, CLUSTER ClusterNum)
{
  struct buffer FAR *bp;
  if (ClusterNum == LONG_LAST_CLUSTER) printf("fatal error: trying to do next_cluster(dpbp, EOC)!\n");


  /* Get the block that this cluster is in                */
  bp = getFATblock(ClusterNum, dpbp);

  if (bp == NULL)
    return DE_BLKINVLD;

  if (ISFAT12(dpbp))
  {
    union {
     UBYTE  bytes[2];
     UCOUNT word;
    } clusterbuff;
      
    UCOUNT idx;

    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    idx = (UCOUNT)(((ClusterNum << 1) + ClusterNum) >> 1) % dpbp->dpb_secsize;

    clusterbuff.bytes[0] = bp->b_buffer[idx];

    clusterbuff.bytes[1] = bp->b_buffer[idx+1]; /* next byte, will be overwritten,
                                                 if not valid */

    /* Test to see if the cluster straddles the block. If it */
    /* does, get the next block and use both to form the    */
    /* the FAT word. Otherwise, just point to the next      */
    /* block.                                               */
    if (idx >= dpbp->dpb_secsize - 1)
    {
      bp = getFATblock(ClusterNum +1, dpbp);

      if (bp == 0)
        return LONG_BAD;

      clusterbuff.bytes[1] = bp->b_buffer[0];
    }

    /* Now to unpack the contents of the FAT entry. Odd and */
    /* even bytes are packed differently.                   */
    
#ifndef I86     /* the latter assumes byte ordering */
    if (ClusterNum & 0x01)
      idx = ((clusterbuff.byte[0] & 0xf0) >> 4) | (clusterbuff.byte[1] << 4);
    else
      idx = clusterbuff.byte[0] | ((clusterbuff.byte[0] & 0x0f) << 8);
#else
  
    if (ClusterNum & 0x01)
      idx = (unsigned short)clusterbuff.word >> 4;
    else
      idx =                 clusterbuff.word & 0x0fff;
#endif      

    if (idx >= MASK12)
      return LONG_LAST_CLUSTER;
    if (idx == BAD12)
      return LONG_BAD;
    return idx;
  }
  else if (ISFAT16(dpbp))
  {    
    UWORD res;

#ifndef I86
    UCOUNT idx;

    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    idx = (ClusterNum * SIZEOF_CLST16) % dpbp->dpb_secsize;

    /* Get the cluster number,                              */
  
    fgetword((VOID FAR *) & (bp->b_buffer[idx]), (WORD FAR *) & res);

#else
    /* this saves 2 WORDS of stack :-) */
    
    res = *(UWORD FAR *)&(bp->b_buffer[(UCOUNT)((ClusterNum * SIZEOF_CLST16) % dpbp->dpb_secsize)]);
#endif    
    if (res >= MASK16) return LONG_LAST_CLUSTER;
    if (res == BAD16) return LONG_BAD;

    return res;
  }
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
  {
    UDWORD res;

    res = *(UDWORD FAR *)&(bp->b_buffer[(UCOUNT)((ClusterNum * SIZEOF_CLST32) % dpbp->dpb_secsize)]);
    if (res > LONG_BAD) return LONG_LAST_CLUSTER;

    return res;    
  }
#endif
  return LONG_LAST_CLUSTER;
}

