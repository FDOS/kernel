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

/************************************************************************/
/*                                                                      */
/*                      block cache routines                            */
/*                                                                      */
/************************************************************************/
/* #define DISPLAY_GETBLOCK */

/*                                                                      */
/* Initialize the buffer structure                                      */
/*                                                                      */

/* Extract the block number from a buffer structure. */

#if 0 /*TE*/
STATIC ULONG getblkno(struct buffer FAR * bp)
{
  if (bp->b_blkno == 0xffffu)
    return bp->b_huge_blkno;
  else
    return bp->b_blkno;
}
#else
#define getblkno(bp) (bp)->b_blkno
#endif

/*  Set the block number of a buffer structure. (The caller should  */
/*  set the unit number before calling this function.)     */
#if 0 /*TE*/
STATIC VOID setblkno(struct buffer FAR * bp, ULONG blkno)
{
  if (blkno >= 0xffffu)
  {
    bp->b_blkno = 0xffffu;
    bp->b_huge_blkno = blkno;
  }
  else
  {
    bp->b_blkno = blkno;
/*    bp->b_dpbp = &blk_devices[bp->b_unit]; */

    bp->b_dpbp = CDSp[bp->b_unit].cdsDpb;

  }
}
#else
#define setblkno(bp, blkno) (bp)->b_blkno = (blkno)
#endif

/*
    this searches the buffer list for the given disk/block.
    
    returns:
    TRUE:
        the buffer is found
    FALSE:
        the buffer is not found
        *Buffp contains a block to flush and reuse later        
        
    new:
        upper layer may set UNCACHE attribute
        UNCACHE buffers are recycled first.
        intended to be used for full sector reads into application buffer        
    
*/

BOOL searchblock(ULONG blkno, COUNT dsk, struct buffer FAR ** pBuffp)
{
  int fat_count = 0;
  struct buffer FAR *bp;
  struct buffer FAR *lbp = NULL;
  struct buffer FAR *lastNonFat = NULL;
  struct buffer FAR *uncacheBuf = NULL;

#ifdef DISPLAY_GETBLOCK
  printf("[searchblock %d, blk %ld, buf ", dsk, blkno);
#endif

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  for (bp = firstbuf; bp != NULL; lbp = bp, bp = bp->b_next)
  {
    if ((getblkno(bp) == blkno) &&
        (bp->b_flag & BFR_VALID) && (bp->b_unit == dsk))
    {
      /* found it -- rearrange LRU links      */
      if (lbp != NULL)
      {
        lbp->b_next = bp->b_next;
        bp->b_next = firstbuf;
        firstbuf = bp;
      }
#ifdef DISPLAY_GETBLOCK
      printf("HIT %04x:%04x]\n", FP_SEG(bp), FP_OFF(bp));
#endif
      *pBuffp = bp;
      return TRUE;
    }

    if (bp->b_flag & BFR_UNCACHE)
      uncacheBuf = bp;

    if (bp->b_flag & BFR_FAT)
      fat_count++;
    else
      lastNonFat = bp;
  }

  /*
     now take either the last buffer in chain (not used recently)
     or, if we are low on FAT buffers, the last non FAT buffer
   */

  if (uncacheBuf)
  {
    lbp = uncacheBuf;
  }
  else
  {
    if (lbp->b_flag & BFR_FAT && fat_count < 3 && lastNonFat)
    {
      lbp = lastNonFat;
    }
  }

  lbp->b_flag &= ~BFR_UNCACHE;  /* reset uncache attribute */

  *pBuffp = lbp;

#ifdef DISPLAY_GETBLOCK
  printf("MISS, replace %04x:%04x]\n", FP_SEG(lbp), FP_OFF(lbp));
#endif

  if (lbp != firstbuf)          /* move to front */
  {
    for (bp = firstbuf; bp->b_next != lbp; bp = bp->b_next)
      ;
    bp->b_next = bp->b_next->b_next;
    lbp->b_next = firstbuf;
    firstbuf = lbp;
  }

  return FALSE;
}

BOOL DeleteBlockInBufferCache(ULONG blknolow, ULONG blknohigh, COUNT dsk)
{
  struct buffer FAR *bp;

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  for (bp = firstbuf; bp != NULL; bp = bp->b_next)
  {
    if (blknolow <= getblkno(bp) &&
        getblkno(bp) <= blknohigh &&
        (bp->b_flag & BFR_VALID) && (bp->b_unit == dsk))
    {
      flush1(bp);
    }
  }

  return FALSE;
}

#if TOM
void dumpBufferCache(void)
{
  struct buffer FAR *bp;
  int printed = 0;

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  for (bp = firstbuf; bp != NULL; bp = bp->b_next)
  {
    printf("%8lx %02x ", getblkno(bp), bp->b_flag);
    if (++printed % 6 == 0)
      printf("\n");
  }
  printf("\n");
}
#endif
/*                                                                      */
/*      Return the address of a buffer structure containing the         */
/*      requested block.                                                */
/*                                                                      */
/*      returns:                                                        */
/*              requested block with data                               */
/*      failure:                                                        */
/*              returns NULL                                            */
/*                                                                      */
struct buffer FAR *getblock(ULONG blkno, COUNT dsk)
{
  struct buffer FAR *bp;

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  if (searchblock(blkno, dsk, &bp))
  {
    return (bp);
  }

  /* The block we need is not in a buffer, we must make a buffer  */
  /* available, and fill it with the desired block                */

  /* take the buffer that lbp points to and flush it, then read new block. */
  if (!flush1(bp))
    return NULL;

  /* Fill the indicated disk buffer with the current track and sector */

  if (dskxfer(dsk, blkno, (VOID FAR *) bp->b_buffer, 1, DSKREAD))
  {
    return NULL;
  }

  bp->b_flag = BFR_VALID | BFR_DATA;
  bp->b_unit = dsk;
  setblkno(bp, blkno);

  return bp;

}

/*
    exactly the same as getblock(), but the data will be completely
    overwritten. so there is no need to read from disk first
 */
struct buffer FAR *getblockOver(ULONG blkno, COUNT dsk)
{
  struct buffer FAR *bp;

  /* Search through buffers to see if the required block  */
  /* is already in a buffer                               */

  if (searchblock(blkno, dsk, &bp))
  {
    return bp;
  }

  /* The block we need is not in a buffer, we must make a buffer  */
  /* available. */

  /* take the buffer than lbp points to and flush it, then make it available. */
  if (flush1(bp))               /* success              */
  {
    bp->b_flag = 0;
    bp->b_unit = dsk;
    setblkno(bp, blkno);
    return bp;
  }
  else
    /* failure              */
  {
    return NULL;
  }
}

/*                                                                      */
/*      Mark all buffers for a disk as not valid                        */
/*                                                                      */
VOID setinvld(REG COUNT dsk)
{
  REG struct buffer FAR *bp;

  bp = firstbuf;
  while (bp)
  {
    if (bp->b_unit == dsk)
      bp->b_flag = 0;
    bp = bp->b_next;
  }
}

/*                                                                      */
/*                      Flush all buffers for a disk                    */
/*                                                                      */
/*      returns:                                                        */
/*              TRUE on success                                         */
/*                                                                      */
BOOL flush_buffers(REG COUNT dsk)
{
  REG struct buffer FAR *bp;
  REG BOOL ok = TRUE;

  bp = firstbuf;
  while (bp)
  {
    if (bp->b_unit == dsk)
      if (!flush1(bp))
        ok = FALSE;
    bp = bp->b_next;
  }
  return ok;
}

/*                                                                      */
/*      Write one disk buffer                                           */
/*                                                                      */
BOOL flush1(struct buffer FAR * bp)
{
/* All lines with changes on 9/4/00 by BER marked below */

  UWORD result;                 /* BER 9/4/00 */

  if ((bp->b_flag & BFR_VALID) && (bp->b_flag & BFR_DIRTY))
  {
    result = dskxfer(bp->b_unit, getblkno(bp), (VOID FAR *) bp->b_buffer, 1, DSKWRITE); /* BER 9/4/00  */
    if (bp->b_flag & BFR_FAT)
    {
      struct dpb FAR *dpbp = bp->b_dpbp;
      UWORD b_copies = dpbp->dpb_fats;
      ULONG b_offset = dpbp->dpb_fatsize;
      ULONG blkno = getblkno(bp);
#ifdef WITHFAT32
      if (ISFAT32(dpbp))
      {
        if (dpbp->dpb_xflags & FAT_NO_MIRRORING)
          b_copies = 1;
        b_offset = dpbp->dpb_xfatsize;
      }
#endif
      while (--b_copies > 0)
      {
        blkno += b_offset;
        result = dskxfer(bp->b_unit, blkno, bp->b_buffer, 1, DSKWRITE);    /* BER 9/4/00 */
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
  REG struct buffer FAR *bp;
  REG BOOL ok;

  ok = TRUE;
  bp = firstbuf;
  while (bp)
  {
    if (!flush1(bp))
      ok = FALSE;
    bp->b_flag &= ~BFR_VALID;
    bp = bp->b_next;
  }

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

/* Changed to UWORD  9/4/00  BER */
UWORD dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks,
              COUNT mode)
/* End of change */
{
/*  REG struct dpb *dpbp = &blk_devices[dsk]; */

  REG struct dpb FAR *dpbp = CDSp[dsk].cdsDpb;

  if ((UCOUNT) dsk >= lastdrive)
  {
    return 0x0201;              /* illegal command */
  }
  if ((CDSp[dsk].cdsFlags & (CDSPHYSDRV | CDSNETWDRV)) != CDSPHYSDRV)
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
    IoReqHdr.r_trans = (BYTE FAR *) buf;
    IoReqHdr.r_count = numblocks;
    if (blkno >= MAXSHORT)
    {
      IoReqHdr.r_start = HUGECOUNT;
      IoReqHdr.r_huge = blkno;
    }
    else
      IoReqHdr.r_start = blkno;
    execrh((request FAR *) & IoReqHdr, dpbp->dpb_device);
    if (!(IoReqHdr.r_status & S_ERROR) && (IoReqHdr.r_status & S_DONE))
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
 * 2000/9/04   Brian Reifsnyder
 * Modified dskxfer() such that error codes are now returned.
 * Functions that rely on dskxfer() have also been modified accordingly.
 */

/*
 * Log: blockio.c,v - for newer entries do "cvs log blockio.c"
 *
 * Revision 1.15  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.14  2000/03/09 06:07:10  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.13  1999/08/25 03:18:07  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.12  1999/08/10 18:03:39  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.11  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.10  1999/05/03 04:55:35  jprice
 * Changed getblock & getbuf so that they leave at least 3 buffer for FAT data.
 *
 * Revision 1.9  1999/04/21 01:44:40  jprice
 * no message
 *
 * Revision 1.8  1999/04/18 05:28:39  jprice
 * no message
 *
 * Revision 1.7  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.6  1999/04/16 00:53:32  jprice
 * Optimized FAT handling
 *
 * Revision 1.5  1999/04/12 23:41:53  jprice
 * Using getbuf to write data instead of getblock
 * using getblock made it read the block before it wrote it
 *
 * Revision 1.4  1999/04/11 05:28:10  jprice
 * Working on multi-block IO
 *
 * Revision 1.3  1999/04/11 04:33:38  jprice
 * ror4 patches
 *
 * Revision 1.1.1.1  1999/03/29 15:41:43  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/01 01:43:27  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.3  1999/01/30 08:25:34  jprice
 * Clean up; Fixed bug with set attribute function.  If you tried to
 * change the attributes of a directory, it would erase it.
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.8   06 Dec 1998  8:43:16   patv
 * Changes in block I/O because of new I/O subsystem.
 *
 *    Rev 1.7   22 Jan 1998  4:09:00   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.6   04 Jan 1998 23:14:36   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   03 Jan 1998  8:36:02   patv
 * Converted data area to SDA format
 *
 *    Rev 1.4   16 Jan 1997 12:46:34   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:15:10   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:46   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:28   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:04:06   patv
 * Initial revision.
 */
