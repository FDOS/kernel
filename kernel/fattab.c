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
static BYTE *RcsId =
    "$Id$";
#endif

/************************************************************************/
/*                                                                      */
/*                      cluster/sector routines                         */
/*                                                                      */
/************************************************************************/

#ifndef ISFAT32
int ISFAT32(struct dpb FAR * dpbp)
{
  return _ISFAT32(dpbp);
}
#endif

struct buffer FAR *getFATblock(ULONG clussec, struct dpb FAR * dpbp)
{
  struct buffer FAR *bp;

  if (ISFAT12(dpbp))
  {
    clussec = (((unsigned)clussec << 1) + (unsigned)clussec) >> 1;
  }
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
  {
    clussec = clussec * SIZEOF_CLST32;
  }
#endif
  else
  {
    clussec = clussec * SIZEOF_CLST16;
  }
  clussec = clussec / dpbp->dpb_secsize + dpbp->dpb_fatstrt;
#ifdef WITHFAT32
  if (ISFAT32(dpbp) && (dpbp->dpb_xflags & FAT_NO_MIRRORING))
  {
    /* we must modify the active fat,
       it's number is in the 0-3 bits of dpb_xflags */
    clussec += (dpbp->dpb_xflags & 0xf) * dpbp->dpb_xfatsize;
  }
#endif

  bp = getblock(clussec, dpbp->dpb_unit);

  if (bp)
  {
    bp->b_flag &= ~(BFR_DATA | BFR_DIR);
    bp->b_flag |= BFR_FAT | BFR_VALID;
    bp->b_dpbp = dpbp;
    bp->b_copies = dpbp->dpb_fats;
    bp->b_offset = dpbp->dpb_fatsize;
#ifdef WITHFAT32
    if (ISFAT32(dpbp))
    {
      if (dpbp->dpb_xflags & FAT_NO_MIRRORING)
        bp->b_copies = 1;
    }
#endif
  }
  return bp;
}

#ifdef WITHFAT32
void read_fsinfo(struct dpb FAR * dpbp)
{
  struct buffer FAR *bp;
  struct fsinfo FAR *fip;

  bp = getblock(dpbp->dpb_xfsinfosec, dpbp->dpb_unit);
  bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT | BFR_DIRTY);
  bp->b_flag |= BFR_VALID;

  fip = (struct fsinfo FAR *)&bp->b_buffer[0x1e4];
  dpbp->dpb_xnfreeclst = fip->fi_nfreeclst;
  dpbp->dpb_xcluster = fip->fi_cluster;
}

void write_fsinfo(struct dpb FAR * dpbp)
{
  struct buffer FAR *bp;
  struct fsinfo FAR *fip;

  bp = getblock(dpbp->dpb_xfsinfosec, dpbp->dpb_unit);
  bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT);
  bp->b_flag |= BFR_VALID | BFR_DIRTY;

  fip = (struct fsinfo FAR *)&bp->b_buffer[0x1e4];
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

unsigned link_fat(struct dpb FAR * dpbp, CLUSTER Cluster1,
                REG CLUSTER Cluster2)
{
  struct buffer FAR *bp;

  /* Get the block that this cluster is in                */
  bp = getFATblock(Cluster1, dpbp);

  if (bp == NULL)
    return DE_BLKINVLD;

  if (ISFAT12(dpbp))
  {
    unsigned idx;
    REG UBYTE FAR *fbp0, FAR * fbp1;
    struct buffer FAR * bp1;

    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    idx = (unsigned) (((Cluster1 << 1) + Cluster1) >> 1) % dpbp->dpb_secsize;
    
    /* Test to see if the cluster straddles the block. If   */
    /* it does, get the next block and use both to form the */
    /* the FAT word. Otherwise, just point to the next      */
    /* block.                                               */
    fbp0 = &bp->b_buffer[idx];
    fbp1 = fbp0 + 1;
  
    if (idx >= (unsigned)dpbp->dpb_secsize - 1)
    {
      bp1 = getFATblock(Cluster1 + 1, dpbp);
      if (bp1 == 0)
        return DE_BLKINVLD;
      
      bp1->b_flag |= BFR_DIRTY | BFR_VALID;
      
      fbp1 = &bp1->b_buffer[0];
    }

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
  }
  else if (ELSE_ISFAT16(dpbp))
  {
    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    /* Finally, put the word into the buffer and mark the   */
    /* buffer as dirty.                                     */
    fputword(
      &bp->b_buffer[((unsigned)Cluster1 * SIZEOF_CLST16) % dpbp->dpb_secsize],
      (UWORD)Cluster2);
  }
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
  {
    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    /* Finally, put the word into the buffer and mark the   */
    /* buffer as dirty.                                     */
    fputlong(
      &bp->b_buffer[(UWORD) ((Cluster1 * SIZEOF_CLST32) % dpbp->dpb_secsize)],
      Cluster2);
  }
#endif
  else
    return DE_BLKINVLD;

  /* update the free space count                          */
  bp->b_flag |= BFR_DIRTY | BFR_VALID;
  if (Cluster2 == FREE)
  {
#ifdef WITHFAT32
    if (ISFAT32(dpbp) && dpbp->dpb_xnfreeclst != XUNKNCLSTFREE)
    {
      /* update the free space count for returned     */
      /* cluster                                      */
      ++dpbp->dpb_xnfreeclst;
      write_fsinfo(dpbp);
    }
    else
#endif
    if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
      ++dpbp->dpb_nfreeclst;
  }

  /*if (Cluster2 == FREE)
     { */
  /* update the free space count for returned     */
  /* cluster                                                                                                                                                                                                                                                                                                                                                                                              */
  /* ++dpbp->dpb_nfreeclst;
     } */

  /* update the free space count for removed      */
  /* cluster                                      */
  /* BUG: was counted twice for 2nd,.. cluster. moved to find_fat_free() */
  /* BO: don't completely understand this yet - leave here for now as
     a comment */
  /* else
     {
     --dpbp->dpb_nfreeclst;
     }   */
  return SUCCESS;
}

/* Given the disk parameters, and a cluster number, this function
   looks at the FAT, and returns the next cluster in the clain. */
CLUSTER next_cluster(struct dpb FAR * dpbp, CLUSTER ClusterNum)
{
  struct buffer FAR *bp;
#ifdef DEBUG
  if (ClusterNum == LONG_LAST_CLUSTER)
    printf("fatal error: trying to do next_cluster(dpbp, EOC)!\n");
  if (ClusterNum == 0)
    printf("fatal error: trying to do next_cluster(dpbp, 0)!\n");
#endif

  /* Get the block that this cluster is in                */
  bp = getFATblock(ClusterNum, dpbp);

  if (bp == NULL)
    return 1; /* the only error code possible here */

  if (ISFAT12(dpbp))
  {
    union {
      UBYTE bytes[2];
      unsigned word;
    } clusterbuff;

    unsigned idx;

    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    idx = ((((unsigned)ClusterNum << 1) + (unsigned)ClusterNum) >> 1) %
      dpbp->dpb_secsize;

    clusterbuff.bytes[0] = bp->b_buffer[idx];

    clusterbuff.bytes[1] = bp->b_buffer[idx + 1];       /* next byte, will be overwritten,
                                                           if not valid */

    /* Test to see if the cluster straddles the block. If it */
    /* does, get the next block and use both to form the    */
    /* the FAT word. Otherwise, just point to the next      */
    /* block.                                               */
    if (idx >= (unsigned)dpbp->dpb_secsize - 1)
    {
      bp = getFATblock(ClusterNum + 1, dpbp);

      if (bp == 0)
        return LONG_BAD;

      clusterbuff.bytes[1] = bp->b_buffer[0];
    }

    /* Now to unpack the contents of the FAT entry. Odd and */
    /* even bytes are packed differently.                   */

#ifndef I86                     /* the latter assumes byte ordering */
    if (ClusterNum & 0x01)
      idx =
          ((clusterbuff.bytes[0] & 0xf0) >> 4) | (clusterbuff.bytes[1] << 4);
    else
      idx = clusterbuff.bytes[0] | ((clusterbuff.bytes[1] & 0x0f) << 8);
#else

    if (ClusterNum & 0x01)
      idx = (unsigned short)clusterbuff.word >> 4;
    else
      idx = clusterbuff.word & 0x0fff;
#endif

    if (idx >= MASK12)
      return LONG_LAST_CLUSTER;
    if (idx == BAD12)
      return LONG_BAD;
    return idx;
  }
  else if (ELSE_ISFAT16(dpbp))
  {
    UWORD res;

    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    /* and get the cluster number                           */

    res = fgetword(&bp->b_buffer[((unsigned)ClusterNum * SIZEOF_CLST16) %
                                 dpbp->dpb_secsize]);

    if (res >= MASK16)
      return LONG_LAST_CLUSTER;
    if (res == BAD16)
      return LONG_BAD;

    return res;
  }
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
  {
    UDWORD res;

    res = fgetlong(&bp->b_buffer[((unsigned)ClusterNum * SIZEOF_CLST32) %
                                 dpbp->dpb_secsize]);
    if (res > LONG_BAD)
      return LONG_LAST_CLUSTER;

    return res;
  }
#endif
  return LONG_LAST_CLUSTER;
}

