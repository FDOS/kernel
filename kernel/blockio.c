/****************************************************************/
/*                                                              */
/*                          blockio.c                           */
/*                            DOS-C                             */
/*                                                              */
/*      Block cache functions and device driver interface       */
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
/*                                                              */
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *blockioRcsId =
    "$Id$";
#endif

#define b_next(bp) ((struct buffer FAR *)(MK_FP(FP_SEG(bp), bp->b_next)))
#define b_prev(bp) ((struct buffer FAR *)(MK_FP(FP_SEG(bp), bp->b_prev)))
#define bufptr(fbp) ((struct buffer FAR *)(MK_FP(FP_SEG(bp), fbp)))

/************************************************************************/
/*                                                                      */
/*                      block cache routines                            */
/*                                                                      */
/************************************************************************/
/* #define DISPLAY_GETBLOCK */

STATIC BOOL flush1(struct buffer FAR * bp);

/*
    this searches the buffer list for the given disk/block.
    
    returns:
    a far pointer to the buffer.

    If the buffer is found the UNCACHE bit is not set and else it is set.
        
    new:
        upper layer may set UNCACHE attribute
        UNCACHE buffers are recycled first.
        intended to be used for full sector reads into application buffer
        resets UNCACHE upon a "HIT" -- so then this buffer will not be
        recycled anymore.
*/

STATIC void move_buffer(struct buffer FAR *bp, size_t firstbp)
{
  /* connect bp->b_prev and bp->b_next */
  b_next(bp)->b_prev = bp->b_prev;
  b_prev(bp)->b_next = bp->b_next;

  /* insert bp between firstbp and firstbp->b_prev */
  bp->b_prev = bufptr(firstbp)->b_prev;
  bp->b_next = firstbp;
  b_next(bp)->b_prev = FP_OFF(bp);
  b_prev(bp)->b_next = FP_OFF(bp);
}

STATIC struct buffer FAR *searchblock(ULONG blkno, COUNT dsk)
{
  int fat_count = 0;
  struct buffer FAR *bp;
  size_t lastNonFat = 0;
  size_t uncacheBuf = 0;
  seg bufseg = FP_SEG(firstbuf);
  size_t firstbp = FP_OFF(firstbuf);

#ifdef DISPLAY_GETBLOCK
  printf("[searchblock %d, blk %ld, buf ", dsk, blkno);
#endif

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  bp = MK_FP(bufseg, firstbp);
  do
  {
    if ((bp->b_blkno == blkno) &&
        (bp->b_flag & BFR_VALID) && (bp->b_unit == dsk))
    {
      /* found it -- rearrange LRU links      */
#ifdef DISPLAY_GETBLOCK
      printf("HIT %04x:%04x]\n", FP_SEG(bp), FP_OFF(bp));
#endif
      bp->b_flag &= ~BFR_UNCACHE;  /* reset uncache attribute */
      if (FP_OFF(bp) != firstbp)
      {
        *(UWORD *)&firstbuf = FP_OFF(bp);
        move_buffer(bp, firstbp);
      }
      return bp;
    }

    if (bp->b_flag & BFR_UNCACHE)
      uncacheBuf = FP_OFF(bp);

    if (bp->b_flag & BFR_FAT)
      fat_count++;
    else
      lastNonFat = FP_OFF(bp);
    bp = b_next(bp);
  } while (FP_OFF(bp) != firstbp);

  /*
     now take either the last buffer in chain (not used recently)
     or, if we are low on FAT buffers, the last non FAT buffer
   */

  if (uncacheBuf)
  {
    bp = bufptr(uncacheBuf);
  }
  else if (bp->b_flag & BFR_FAT && fat_count < 3 && lastNonFat)
  {
    bp = bufptr(lastNonFat);
  }
  else
  {
    bp = b_prev(bufptr(firstbp));
  }

  bp->b_flag |= BFR_UNCACHE;  /* set uncache attribute */

#ifdef DISPLAY_GETBLOCK
  printf("MISS, replace %04x:%04x]\n", FP_SEG(bp), FP_OFF(bp));
#endif

  if (FP_OFF(bp) != firstbp)          /* move to front */
  {
    move_buffer(bp, firstbp);
    *(UWORD *)&firstbuf = FP_OFF(bp);
  }
  return bp;
}

BOOL DeleteBlockInBufferCache(ULONG blknolow, ULONG blknohigh, COUNT dsk, int mode)
{
  struct buffer FAR *bp = firstbuf;
        
  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  do
  {
    if (blknolow <= bp->b_blkno &&
        bp->b_blkno <= blknohigh &&
        (bp->b_flag & BFR_VALID) && (bp->b_unit == dsk))
    {
      if (mode == XFR_READ)
        flush1(bp);
      else
        bp->b_flag = 0;
    }
    bp = b_next(bp);
  }
  while (FP_OFF(bp) != FP_OFF(firstbuf));

  return FALSE;
}

#if TOM
void dumpBufferCache(void)
{
  struct buffer FAR *bp = firstbuf;
  int printed = 0;

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  do
  {
    printf("%8lx %02x ", bp->b_blkno, bp->b_flag);
    if (++printed % 6 == 0)
      printf("\n");
    bp = b_next(bp);
  }
  while (FP_OFF(bp) != FP_OFF(firstbuf));
  printf("\n");
}
#endif
/*                                                                      */
/*      Return the address of a buffer structure containing the         */
/*      requested block.                                                */
/*      if overwrite is set, then no need to read first                 */
/*                                                                      */
/*      returns:                                                        */
/*              requested block with data                               */
/*      failure:                                                        */
/*              returns NULL                                            */
/*                                                                      */
struct buffer FAR *getblk(ULONG blkno, COUNT dsk, BOOL overwrite)
{
  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  struct buffer FAR *bp = searchblock(blkno, dsk);

  if (!(bp->b_flag & BFR_UNCACHE))
  {
    return bp;
  }

  /* The block we need is not in a buffer, we must make a buffer  */
  /* available, and fill it with the desired block                */

  /* take the buffer that lbp points to and flush it, then read new block. */
  if (!flush1(bp))
    return NULL;

  /* Fill the indicated disk buffer with the current track and sector */

  if (!overwrite && dskxfer(dsk, blkno, bp->b_buffer, 1, DSKREAD))
  {
    return NULL;
  }

  bp->b_flag = BFR_VALID | BFR_DATA;
  bp->b_unit = dsk;
  bp->b_blkno = blkno;

  return bp;
}

/*                                                                      */
/*      Mark all buffers for a disk as not valid                        */
/*                                                                      */
VOID setinvld(REG COUNT dsk)
{
  struct buffer FAR *bp = firstbuf;

  do
  {
    if (bp->b_unit == dsk)
      bp->b_flag = 0;
    bp = b_next(bp);
  }
  while (FP_OFF(bp) != FP_OFF(firstbuf));
}

/*                                                                      */
/*                      Flush all buffers for a disk                    */
/*                                                                      */
/*      returns:                                                        */
/*              TRUE on success                                         */
/*                                                                      */
BOOL flush_buffers(REG COUNT dsk)
{
  struct buffer FAR *bp = firstbuf;
  REG BOOL ok = TRUE;

  bp = firstbuf;
  do
  {
    if (bp->b_unit == dsk)
      if (!flush1(bp))
        ok = FALSE;
    bp = b_next(bp);
  }
  while (FP_OFF(bp) != FP_OFF(firstbuf));
  return ok;
}

/*                                                                      */
/*      Write one disk buffer                                           */
/*                                                                      */
STATIC BOOL flush1(struct buffer FAR * bp)
{
/* All lines with changes on 9/4/00 by BER marked below */

  UWORD result;                 /* BER 9/4/00 */

  if ((bp->b_flag & (BFR_VALID | BFR_DIRTY)) == (BFR_VALID | BFR_DIRTY))
  {
    /* BER 9/4/00  */
    result = dskxfer(bp->b_unit, bp->b_blkno, bp->b_buffer, 1, DSKWRITE);
    if (bp->b_flag & BFR_FAT)
    {
      UWORD b_copies = bp->b_copies;
      ULONG blkno = bp->b_blkno;
#ifdef WITHFAT32
      ULONG b_offset = bp->b_offset;
      if (b_offset == 0) /* FAT32 FS */
        b_offset = bp->b_dpbp->dpb_xfatsize;
#else
      UWORD b_offset = bp->b_offset;
#endif
      while (--b_copies > 0)
      {
        blkno += b_offset;
        /* BER 9/4/00 */
        result = dskxfer(bp->b_unit, blkno, bp->b_buffer, 1, DSKWRITE);
      }
    }
  }
  else
    result = 0;                 /* This negates any error code returned in result...BER */
  /* and 0 returned, if no errors occurred - tom          */
  bp->b_flag &= ~BFR_DIRTY;     /* even if error, mark not dirty */
  if (result != 0)              /* otherwise system has trouble  */
    bp->b_flag &= ~BFR_VALID;   /* continuing.           */
  return (TRUE);                /* Forced to TRUE...was like this before dskxfer()  */
  /* returned error codes...BER */
}

/*                                                                      */
/*      Write all disk buffers                                          */
/*                                                                      */
BOOL flush(void)
{
  REG struct buffer FAR *bp = firstbuf;
  REG BOOL ok;

  ok = TRUE;
  do
  {
    if (!flush1(bp))
      ok = FALSE;
    bp->b_flag &= ~BFR_VALID;
    bp = b_next(bp);
  }
  while (FP_OFF(bp) != FP_OFF(firstbuf));

  remote_flushall();

  return (ok);
}

/************************************************************************/
/*                                                                      */
/*              Device Driver Interface Functions                       */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Transfer one or more blocks to/from disk                             */
/*                                                                      */

UWORD dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks,
              COUNT mode)
{
  register struct dpb FAR *dpbp = get_dpb(dsk);
  if (dpbp == NULL)
  {
    return 0x0201;              /* illegal command */
  }

#if TOM
#define KeyboardShiftState() (*(BYTE FAR *)(MK_FP(0x40,0x17)))

  if (KeyboardShiftState() & 0x01)
  {
    printf("dskxfer:%s %x - %lx %u\n", mode == DSKWRITE ? "write" : "read",
           dsk, blkno, numblocks);
    if ((KeyboardShiftState() & 0x03) == 3)
      dumpBufferCache();
  }
#endif

  for (;;)
  {
    IoReqHdr.r_length = sizeof(request);
    IoReqHdr.r_unit = dpbp->dpb_subunit;

    switch (mode)
    {
      case DSKWRITE:
        if (verify_ena)
        {
          IoReqHdr.r_command = C_OUTVFY;
          break;
        }
        /* else fall through */
      case DSKWRITEINT26:
        IoReqHdr.r_command = C_OUTPUT;
        break;

      case DSKREADINT25:
      case DSKREAD:
        IoReqHdr.r_command = C_INPUT;
        break;
      default:
        return 0x0100;          /* illegal command */
    }

    IoReqHdr.r_status = 0;
    IoReqHdr.r_meddesc = dpbp->dpb_mdb;
    IoReqHdr.r_count = numblocks;
    if (blkno >= MAXSHORT)
    {
      IoReqHdr.r_start = HUGECOUNT;
      IoReqHdr.r_huge = blkno;
    }
    else
      IoReqHdr.r_start = blkno;
    /*
     * Some drivers normalise transfer address so HMA transfers are disastrous!
     * Then transfer block through xferbuf (DiskTransferBuffer doesn't work!)
     * (But this won't work for multi-block HMA transfers... are there any?)
     */
    if (FP_SEG(buf) >= 0xa000 && numblocks == 1 && bufloc != LOC_CONV)
    {
      IoReqHdr.r_trans = deblock_buf;
      if (mode == DSKWRITE)
        fmemcpy(deblock_buf, buf, SEC_SIZE);
      execrh((request FAR *) & IoReqHdr, dpbp->dpb_device);
      if (mode == DSKREAD)
        fmemcpy(buf, deblock_buf, SEC_SIZE);
    }
    else
    {
      IoReqHdr.r_trans = (BYTE FAR *) buf;
      execrh((request FAR *) & IoReqHdr, dpbp->dpb_device);
    }
    if ((IoReqHdr.r_status & (S_ERROR | S_DONE)) == S_DONE)
      break;

    /* INT25/26 (_SEEMS_ TO) return immediately with 0x8002,
       if drive is not online,...

       normal operations (DIR) wait for ABORT/RETRY

       other condition codes not tested
     */
    if (mode >= DSKWRITEINT26)
      return (IoReqHdr.r_status);

  loop:
    switch (block_error(&IoReqHdr, dpbp->dpb_unit, dpbp->dpb_device))
    {
      case ABORT:
      case FAIL:
        return (IoReqHdr.r_status);

      case RETRY:
        continue;

      case CONTINUE:
        break;

      default:
        goto loop;
    }
    break;
  }                             /* retry loop */
/* *** Changed 9/4/00  BER */
  return 0;                     /* Success!  Return 0 for a successful operation. */
/* End of change */

}

/*
       this removes any (additionally allocated) buffers 
       from the HMA buffer chain, because they get allocated to the 'user'
*/     
     
void AllocateHMASpace (size_t lowbuffer, size_t highbuffer)
{
  REG struct buffer FAR *bp = firstbuf;
  int n = LoL_nbuffers;

  do
  {
    if (FP_OFF(bp) < highbuffer && FP_OFF(bp+1) >= lowbuffer)
    {
      flush1(bp);
      /* unlink bp from buffer chain */

      b_prev(bp)->b_next = bp->b_next;
      b_next(bp)->b_prev = bp->b_prev;
      if (bp == firstbuf)
        firstbuf = b_next(bp);
      LoL_nbuffers--;
    }
    bp = b_next(bp);
  }
  while (--n);
}
