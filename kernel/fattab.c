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

/* special "impossible" "Cluster2" value of 1 denotes reading the
   cluster number rather than overwriting it */
#define READ_CLUSTER 1

#ifndef ISFAT32
int ISFAT32(struct dpb FAR * dpbp)
{
  return _ISFAT32(dpbp);
}
#endif

/* idx is a pointer to an index which is the nibble offset of the FAT
   entry within the sector for FAT12, or word offset for FAT16, or
   dword offset for FAT32 */
struct buffer FAR *getFATblock(CLUSTER clussec, struct dpb FAR * dpbp,
                               unsigned *idx)
{
  unsigned secdiv;
  struct buffer FAR *bp;
  CLUSTER max_cluster = dpbp->dpb_size;

#ifdef WITHFAT32
  if (ISFAT32(dpbp))
    max_cluster = dpbp->dpb_xsize;
#endif
 
  if (clussec <= 1 || clussec > max_cluster)
  {
    put_string("run CHKDSK: trying to access invalid cluster 0x");
#ifdef WITHFAT32
    put_unsigned((unsigned)(clussec >> 16), 16, 4);
#endif
    put_unsigned((unsigned)(clussec & 0xffffu), 16, 4);
    put_console('\n');
    return NULL;
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
  if (idx)
    *idx = (unsigned)(clussec % secdiv);
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
/* FAT is the easiest, since it is nothing more than a series   */
/* of UWORD's. The 12 bit FAT is difficult, because it packs 3  */
/* FAT entries into two BYTE's. These are packed as follows:    */
/*                                                              */
/*      0x0003 0x0004 0x0005 0x0006 0x0007 0x0008 0x0009 ...    */
/*                                                              */
/*      are packed as                                           */
/*                                                              */
/*      0x03 0x40 0x00 0x05 0x60 0x00 0x07 0x80 0x00 0x09 ...   */
/*                                                              */
/*      12 bytes are compressed to 9 bytes                      */
/*                                                              */

CLUSTER link_fat(struct dpb FAR * dpbp, CLUSTER Cluster1,
                 REG CLUSTER Cluster2)
{
  struct buffer FAR *bp;
  unsigned idx;

  /* Get the block that this cluster is in                */
  bp = getFATblock(Cluster1, dpbp, &idx);

  if (bp == NULL)
    return 1; /* the only error code possible here */

  if (ISFAT12(dpbp))
  {
    REG UBYTE FAR *fbp0, FAR * fbp1;
    struct buffer FAR * bp1;
    union {
      UBYTE bytes[2];
      unsigned word;
    } clusterbuff;

    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    idx /= 2;

    /* Test to see if the cluster straddles the block. If   */
    /* it does, get the next block and use both to form the */
    /* the FAT word. Otherwise, just point to the next      */
    /* block.                                               */
    clusterbuff.bytes[0] = bp->b_buffer[idx];

    /* next byte, will be overwritten, if not valid */
    clusterbuff.bytes[1] = bp->b_buffer[idx + 1];
    fbp0 = &bp->b_buffer[idx];
    fbp1 = fbp0 + 1;
  
    if (idx >= (unsigned)dpbp->dpb_secsize - 1)
    {
      bp1 = getFATblock((unsigned)Cluster1 + 1, dpbp, NULL);
      if (bp1 == 0)
        return 1; /* the only error code possible here */
      
      if (Cluster2 != READ_CLUSTER)
        bp1->b_flag |= BFR_DIRTY | BFR_VALID;
      
      fbp1 = &bp1->b_buffer[0];
      clusterbuff.bytes[1] = bp1->b_buffer[0];
    }

    if (Cluster2 == READ_CLUSTER)
    {
      /* Now to unpack the contents of the FAT entry. Odd and */
      /* even bytes are packed differently.                   */

#ifndef I86                     /* the latter assumes byte ordering */
      if (Cluster1 & 0x01)
        idx =
          ((clusterbuff.bytes[0] & 0xf0) >> 4) | (clusterbuff.bytes[1] << 4);
      else
        idx = clusterbuff.bytes[0] | ((clusterbuff.bytes[1] & 0x0f) << 8);
#else

      if (Cluster1 & 0x01)
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

    /* Now pack the value in                                */
    if ((unsigned)Cluster1 & 0x01)
    {
      *fbp0 = (*fbp0 & 0x0f) | (UBYTE)((UBYTE)Cluster2 << 4);
      *fbp1 = (UBYTE)((unsigned)Cluster2 >> 4);
    }
    else
    {
      *fbp0 = (UBYTE)Cluster2;
      /* Cluster2 may be set to LONG_LAST_CLUSTER == 0x0FFFFFFFUL or 0xFFFF */
      /* -- please don't optimize to (UBYTE)((unsigned)Cluster2 >> 8)!      */
      *fbp1 = (*fbp1 & 0xf0) | ((UBYTE)((unsigned)Cluster2 >> 8) & 0x0f);
    }
  }
  else if (ISFAT16(dpbp)) 
  {
    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    if (Cluster2 == READ_CLUSTER)
    {
      /* and get the cluster number                           */
      UWORD res = fgetword(&bp->b_buffer[idx * 2]);
      if (res >= MASK16)
        return LONG_LAST_CLUSTER;
      if (res == BAD16)
        return LONG_BAD;

      return res;
    }
    /* Finally, put the word into the buffer and mark the   */
    /* buffer as dirty.                                     */
    fputword(&bp->b_buffer[idx * 2], (UWORD)Cluster2);
  }
#ifdef WITHFAT32
  else if (ISFAT32(dpbp))
  {
    /* form an index so that we can read the block as a     */
    /* byte array                                           */
    if (Cluster2 == READ_CLUSTER)
    {
      UDWORD res = fgetlong(&bp->b_buffer[idx * 4]);
      if (res > LONG_BAD)
        return LONG_LAST_CLUSTER;

      return res;
    }
    /* Finally, put the word into the buffer and mark the   */
    /* buffer as dirty.                                     */
    fputlong(&bp->b_buffer[idx * 4], Cluster2);
  }
#endif
  else
    return 1;

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
  return link_fat(dpbp, ClusterNum, READ_CLUSTER);
}

