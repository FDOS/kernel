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
    "$Id: fattab.c 1631 2011-06-13 16:27:34Z bartoldeman $";
#endif

/************************************************************************/
/*                                                                      */
/*                      cluster/sector routines                         */
/*                                                                      */
/************************************************************************/

/* special "impossible" "Cluster2" value of 1 denotes reading the
   cluster number rather than overwriting it */
#define READ_CLUSTER 1

#ifndef ISFAT32
int ISFAT32(struct dpb FAR * dpbp)
{
  return _ISFAT32(dpbp);
}
#endif

void clusterMessage(const char * msg, CLUSTER clussec)
{
  put_string("Run chkdsk: Bad FAT ");
  put_string(msg);
#ifdef WITHFAT32
  put_unsigned((unsigned)(clussec >> 16), 16, 4);
#endif
  put_unsigned((unsigned)(clussec & 0xffffu), 16, 4);
  put_console('\n');
}

struct buffer FAR *getFATblock(struct dpb FAR * dpbp, CLUSTER clussec)
{
  /* *** why dpbp->dpb_unit? only useful to know in context of the dpbp...? *** */
  struct buffer FAR *bp = getblock(clussec, dpbp->dpb_unit);

  if (bp)
  {
    bp->b_flag &= ~(BFR_DATA | BFR_DIR);
    bp->b_flag |= BFR_FAT | BFR_VALID;
    bp->b_dpbp = dpbp;
    bp->b_copies = dpbp->dpb_fats;
    bp->b_offset = dpbp->dpb_fatsize; /* 0 for FAT32 but blockio.c knows that */
#ifdef WITHFAT32
    if (ISFAT32(dpbp))
    {
      if (dpbp->dpb_xflags & FAT_NO_MIRRORING)
        bp->b_copies = 1;
    }
#endif
  } else {
    clusterMessage("I/O: 0x",clussec);
  }
  return bp;
}

#ifdef WITHFAT32
void read_fsinfo(struct dpb FAR * dpbp)
{
  struct buffer FAR *bp;
  struct fsinfo FAR *fip;
  CLUSTER cluster;

  if (dpbp->dpb_xfsinfosec == 0xffff)
    return;

  bp = getblock(dpbp->dpb_xfsinfosec, dpbp->dpb_unit);
  bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT | BFR_DIRTY);
  bp->b_flag |= BFR_VALID;

  fip = (struct fsinfo FAR *)&bp->b_buffer[0x1e4];
  /* need to range check values because they may not be correct */
  cluster = fip->fi_nfreeclst;
  if (cluster >= dpbp->dpb_xsize)
    cluster = XUNKNCLSTFREE;
  dpbp->dpb_xnfreeclst = cluster;
  cluster = fip->fi_cluster;
  if (cluster < 2 || cluster > dpbp->dpb_xsize)
    cluster = UNKNCLUSTER;
  dpbp->dpb_xcluster = cluster;
}

void write_fsinfo(struct dpb FAR * dpbp)
{
  struct buffer FAR *bp;
  struct fsinfo FAR *fip;

  if (dpbp->dpb_xfsinfosec == 0xffff)
    return;

  bp = getblock(dpbp->dpb_xfsinfosec, dpbp->dpb_unit);
  bp->b_flag &= ~(BFR_DATA | BFR_DIR | BFR_FAT);
  bp->b_flag |= BFR_VALID;

  fip = (struct fsinfo FAR *)&bp->b_buffer[0x1e4];

  if (fip->fi_nfreeclst != dpbp->dpb_xnfreeclst ||
    fip->fi_cluster != dpbp->dpb_xcluster)
    bp->b_flag |= BFR_DIRTY; /* only flag for update if we had real news */

  fip->fi_nfreeclst = dpbp->dpb_xnfreeclst;
  fip->fi_cluster = dpbp->dpb_xcluster;
}
#endif

/*                                                              */
/* The FAT file system is difficult to trace through FAT table. */
/* There are two kinds of FATs,  12 bit and 16 bit. The 16 bit  */
/* FAT is the easiest, since it is nothing more than a series   */
/* of UWORDs.  The 12 bit FAT is difficult, because it packs 3  */
/* FAT entries into two BYTEs.  These are packed as follows:    */
/*                                                              */
/*      0x0003 0x0004 0x0005 0x0006 0x0007 0x0008 0x0009 ...    */
/*                                                              */
/*      are packed as                                           */
/*                                                              */
/*      0x03 0x40 0x00 0x05 0x60 0x00 0x07 0x80 0x00 0x09 ...   */
/*                                                              */
/*      12 bytes are compressed to 9 bytes                      */
/*                                                              */

/* either read the value at Cluster1 (if Cluster2 is READ_CLUSTER) */
/* or write the Cluster2 value to the FAT entry at Cluster1        */
/* Read is always via next_cluster wrapper which has extra checks  */
/* It might make sense to manually check old values before a write */
/* returns: the cluster number (or 1 on error) for read mode       */
/* returns: SUCCESS (or 1 on error) for write mode                 */
CLUSTER link_fat(struct dpb FAR * dpbp, CLUSTER Cluster1,
                 REG CLUSTER Cluster2)
{
  struct buffer FAR *bp;
  unsigned idx;
  unsigned secdiv; /* FAT entries per sector; nibbles for FAT12! */
  unsigned char wasfree;
  CLUSTER clussec = Cluster1;
  CLUSTER max_cluster = dpbp->dpb_size;

#ifdef WITHFAT32
  if (ISFAT32(dpbp))
    max_cluster = dpbp->dpb_xsize;
#endif
 
  if (clussec <= 1 || clussec > max_cluster) /* try to read out of range? */
  {
    clusterMessage("index: 0x",clussec); /* bad array offset */
    return 1;
  }

  /* Cluster2 can 0 (FREE) or 1 (READ_CLUSTER), a cluster nr. >= 2, */
  /* (range check this case!) LONG_LAST_CLUSTER or LONG_BAD here... */
  if (Cluster2 < LONG_BAD && Cluster2 > max_cluster) /* writing bad value? */
  {
    clusterMessage("write: 0x",Cluster2); /* refuse to write bad value */
    return 1;
  }

  secdiv = dpbp->dpb_secsize;
  if (ISFAT12(dpbp))
  {
    clussec = (unsigned)clussec * 3;
    secdiv *= 2;
  }
  else /* FAT16 or FAT32 */
  {
    secdiv /= 2;
#ifdef WITHFAT32
    if (ISFAT32(dpbp))
      secdiv /= 2;
#endif
  }

  /* idx is a pointer to an index which is the nibble offset of the FAT
     entry within the sector for FAT12, or word offset for FAT16, or
     dword offset for FAT32 */
  idx = (unsigned)(clussec % secdiv);
  clussec /= secdiv;
  clussec += dpbp->dpb_fatstrt;
#ifdef WITHFAT32
  if (ISFAT32(dpbp) && (dpbp->dpb_xflags & FAT_NO_MIRRORING))
  {
    /* we must modify the active fat,
       it's number is in the 0-3 bits of dpb_xflags */
    clussec += (dpbp->dpb_xflags & 0xf) * dpbp->dpb_xfatsize;
  }
#endif

  /* Get the block that this cluster is in                */
  bp = getFATblock(dpbp, clussec);

  if (bp == NULL) {
    return 1; /* the only error code possible here */
  }

  if (ISFAT12(dpbp))
  {
    REG UBYTE FAR *fbp0; REG UBYTE FAR * fbp1;
    struct buffer FAR * bp1;
    unsigned cluster, cluster2;

    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    idx /= 2;

    /* Test to see if the cluster straddles the block. If   */
    /* it does, get the next block and use both to form the */
    /* the FAT word. Otherwise, just point to the next      */
    /* block.                                               */
    fbp0 = &bp->b_buffer[idx];

    /* pointer to next byte, will be overwritten, if not valid */
    fbp1 = fbp0 + 1;

    if (idx >= (unsigned)dpbp->dpb_secsize - 1)
    {
      /* blockio.c LRU logic ensures that bp != bp1 */
      bp1 = getFATblock(dpbp, (unsigned)clussec + 1);
      if (bp1 == 0)
        return 1; /* the only error code possible here */
      
      if (Cluster2 != READ_CLUSTER)
        bp1->b_flag |= BFR_DIRTY | BFR_VALID;
      
      fbp1 = &bp1->b_buffer[0];
    }

    cluster = *fbp0 | (*fbp1 << 8);
    {
      unsigned res = cluster;

      /* Now to unpack the contents of the FAT entry. Odd and */
      /* even bytes are packed differently.                   */

      if (Cluster1 & 0x01)
        cluster >>= 4;
      cluster &= 0x0fff;

      if ((unsigned)Cluster2 == READ_CLUSTER)
      {
        if (cluster >= MASK12)
          return LONG_LAST_CLUSTER;
        if (cluster == BAD12)
          return LONG_BAD;
        return cluster;
      }

      wasfree = 0;
      if (cluster == FREE)
        wasfree = 1;

      cluster = res;
    }

    /* Cluster2 may be set to LONG_LAST_CLUSTER == 0x0FFFFFFFUL or 0xFFFF */
    /* -- please don't remove this mask!                                  */
    cluster2 = (unsigned)Cluster2 & 0x0fff;

    /* Now pack the value in                                */
    if ((unsigned)Cluster1 & 0x01)
    {
      cluster &= 0x000f;
      cluster2 <<= 4;
    }
    else
    {
      cluster &= 0xf000;
    }
    cluster |= cluster2;
    *fbp0 = (UBYTE)cluster;
    *fbp1 = (UBYTE)(cluster >> 8);
  }
  else if (ISFAT16(dpbp)) 
  {
    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    /* and get the cluster number                           */
    UWORD res = fgetword(&bp->b_buffer[idx * 2]);
    if ((unsigned)Cluster2 == READ_CLUSTER)
    {
      if (res >= MASK16)
        return LONG_LAST_CLUSTER;
      if (res == BAD16)
        return LONG_BAD;

      return res;
    }
    /* Finally, put the word into the buffer and mark the   */
    /* buffer as dirty.                                     */
    fputword(&bp->b_buffer[idx * 2], (UWORD)Cluster2);
    wasfree = 0;
    if (res == FREE)
      wasfree = 1;
  }
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
  {
    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    UDWORD res = fgetlong(&bp->b_buffer[idx * 4]) & LONG_LAST_CLUSTER;
    if (Cluster2 == READ_CLUSTER)
    {
      if (res > LONG_BAD)
        return LONG_LAST_CLUSTER;

      return res;
    }
    /* Finally, put the word into the buffer and mark the   */
    /* buffer as dirty.                                     */
    fputlong(&bp->b_buffer[idx * 4], Cluster2 & LONG_LAST_CLUSTER);
    wasfree = 0;
    if (res == FREE)
      wasfree = 1;
  }
#endif
  else {
    put_string("Bad DPB!\n"); /* FAT1x size field > 65525U (see fat.h) */
    return 1;
  }

  /* update the free space count                          */
  bp->b_flag |= BFR_DIRTY | BFR_VALID;
  if (Cluster2 == FREE || wasfree)
  {
    int adjust = 0;
    if (!wasfree)
      adjust = 1;
    else if (Cluster2 != FREE)
      adjust = -1;
#ifdef WITHFAT32
    if (ISFAT32(dpbp) && dpbp->dpb_xnfreeclst != XUNKNCLSTFREE)
    {
      /* update the free space count for returned     */
      /* cluster                                      */
      dpbp->dpb_xnfreeclst += adjust;
      write_fsinfo(dpbp);
    }
    else
#endif
    if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
      dpbp->dpb_nfreeclst += adjust;
  }
  return SUCCESS;
}

/* Given the disk parameters, and a cluster number, this function */
/* looks at the FAT, and returns the next cluster in the clain or */
/* 0 if there is no chain, 1 on error, LONG_LAST_CLUSTER at end.  */
CLUSTER next_cluster(struct dpb FAR * dpbp, CLUSTER ClusterNum)
{
  CLUSTER candidate, following, max_cluster;
  candidate = link_fat(dpbp, ClusterNum, READ_CLUSTER);
  /* empty (0) error (1) bad (LONG_BAD) last (>LONG_BAD) need no checks */
#if 0
  if (candidate == ClusterNum)
    return 1; /* chain has a tiny loop - easy but boring error check */
#endif
  if (candidate < 2 || candidate >= LONG_BAD)
    return candidate;
  max_cluster = dpbp->dpb_size;
#ifdef WITHFAT32
  if (ISFAT32(dpbp))
    max_cluster = dpbp->dpb_xsize;
#endif
  /* FAT entry points to a possibly invalid next cluster */
  following = link_fat(dpbp, candidate, READ_CLUSTER);
  if (following<2 || (following < LONG_BAD && following > max_cluster))
  {
    /* chain must not contain free or out of range clusters */
    clusterMessage("value: 0x",following); /* read returned bad value */
    return 1; /* only possible error code here */
  }
  /* without checking "following", a chain can dangle to a free cluster: */
  /* if that cluster is later used by another chain, you get cross links */
  return candidate;
}

/* check if the selected cluster is free (faster than next_cluster) */
BOOL is_free_cluster(struct dpb FAR * dpbp, CLUSTER ClusterNum)
{
  return (link_fat(dpbp, ClusterNum, READ_CLUSTER) == FREE);
}
