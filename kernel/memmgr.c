/****************************************************************/
/*                                                              */
/*                          memmgr.c                            */
/*                                                              */
/*               Memory Manager for Core Allocation             */
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

#ifdef VERSION_STRING
static BYTE *memmgrRcsId =
    "$Id$";
#endif

/*#define nxtMCBsize(mcb,size)	\
	MK_FP(far2para((VOID FAR *) (mcb)) + (size) + 1, 0) */

void FAR *nxtMCBsize(mcb FAR * Mcb, int size)
{
  return MK_FP(far2para((VOID FAR *) (Mcb)) + (size) + 1, 0);
}

#define nxtMCB(mcb) nxtMCBsize((mcb), (mcb)->m_size)

#define mcbFree(mcb) ((mcb)->m_psp == FREE_PSP)
#define mcbValid(mcb)			\
	((mcb)->m_type == MCB_NORMAL || (mcb)->m_type == MCB_LAST)

#define para2far(seg) (mcb FAR *)MK_FP((seg) , 0)

/*
 * Join any following unused MCBs to MCB 'p'.
 *  Return:
 *  SUCCESS: on success
 *  else: error number <<currently DE_MCBDESTRY only>>
 */
STATIC COUNT joinMCBs(mcb FAR * p)
{
  mcb FAR *q;

  /* loop as long as the current MCB is not the last one in the chain
     and the next MCB is unused */
  while (p->m_type == MCB_NORMAL && mcbFree(q = nxtMCB(p)))
  {
    if (!mcbValid(q))
      return DE_MCBDESTRY;
    /* join both MCBs */
    p->m_type = q->m_type;      /* possibly the next MCB is the last one */
    p->m_size += q->m_size + 1; /* one for q's MCB itself */
    q->m_type = 'K';            /* Invalidate the magic number */
  }

  return SUCCESS;
}

seg far2para(VOID FAR * p)
{
  return FP_SEG(p) + (FP_OFF(p) >> 4);
}

seg long2para(ULONG size)
{
  UWORD high = size >> 16;
  if ((UWORD) size > 0xfff0)
    high++;
  return (((UWORD) size + 0x0f) >> 4) + (high << 12);
}

/*
 * Add a displacement to a far pointer and return the result normalized.
 */
VOID FAR * add_far(VOID FAR * fp, ULONG off)
{
  UWORD off2;

  if (FP_SEG(fp) == 0xffff)
    return ((BYTE FAR *) fp) + FP_OFF(off);

#ifndef I86
  if (FP_SEG(fp) == 0)
    return ((BYTE FAR *) fp) + FP_OFF(off);
#endif

  off += FP_OFF(fp);
  off2 = ((off >> 16) << 12) + ((UWORD) off >> 4);

  return MK_FP(FP_SEG(fp) + off2, (UWORD) off & 0xf);
}

/*
 * Return a normalized far pointer
 */
void FAR * adjust_far(const void FAR * fp)
{
  /* and return an adddress adjusted to the nearest paragraph     */
  /* boundary.                                                    */

  if (FP_SEG(fp) == 0xffff)
    return (void FAR *)fp;

#ifndef I86
  if (FP_SEG(fp) == 0)
    return (void FAR *)fp;
#endif

  return MK_FP(FP_SEG(fp) + (FP_OFF(fp) >> 4), FP_OFF(fp) & 0xf);
}

#undef REG
#define REG

#if 1                           /* #ifdef KERNEL  KERNEL */
/* Allocate a new memory area. *para is assigned to the segment of the
   MCB rather then the segment of the data portion */
/* If mode == LARGEST, asize MUST be != NULL and will always recieve the
   largest available block, which is allocated.
   If mode != LARGEST, asize maybe NULL, but if not, it is assigned to the
   size of the largest available block only on failure.
   size is the minimum size of the block to search for,
   even if mode == LARGEST.
 */
COUNT DosMemAlloc(UWORD size, COUNT mode, seg FAR * para,
                  UWORD FAR * asize)
{
  REG mcb FAR *p;
  mcb FAR *foundSeg;
  mcb FAR *biggestSeg;
  /* Initialize                                           */

searchAgain:

  p = para2far(first_mcb);

  biggestSeg = foundSeg = NULL;
/*
    Hack to the Umb Region direct for now. Save time and program space.
*/
  if (uppermem_link && uppermem_root != 0xffff)
  {
    COUNT tmpmode = (mode == LARGEST ? mem_access_mode : mode);
    if ((mode != LARGEST || size == 0xffff) &&
        (tmpmode & (FIRST_FIT_UO | FIRST_FIT_U)))
      p = para2far(uppermem_root);
  }

  /* Search through memory blocks                         */
  FOREVER
  {
    /* check for corruption                         */
    if (!mcbValid(p))
      return DE_MCBDESTRY;

    if (mcbFree(p))
    {                           /* unused block, check if it applies to the rule */
      if (joinMCBs(p) != SUCCESS)       /* join following unused blocks */
        return DE_MCBDESTRY;    /* error */

      if (!biggestSeg || biggestSeg->m_size < p->m_size)
        biggestSeg = p;

      if (p->m_size >= size)
      {                         /* if the block is too small, ignore */
        /* this block has a "match" size, try the rule set */
        switch (mode)
        {
          case LAST_FIT:       /* search for last possible */
          case LAST_FIT_U:
          case LAST_FIT_UO:
          default:
            foundSeg = p;
            break;

          case LARGEST:        /* grab the biggest block */
            /* it is calculated when the MCB chain
               was completely checked */
            break;

          case BEST_FIT:       /* first, but smallest block */
          case BEST_FIT_U:
          case BEST_FIT_UO:
            if (!foundSeg || foundSeg->m_size > p->m_size)
              /* better match found */
              foundSeg = p;
            break;

          case FIRST_FIT:      /* first possible */
          case FIRST_FIT_U:
          case FIRST_FIT_UO:
            foundSeg = p;
            goto stopIt;        /* OK, rest of chain can be ignored */

        }
      }
    }

    if (p->m_type == MCB_LAST)
      break;                    /* end of chain reached */

    p = nxtMCB(p);              /* advance to next MCB */
  }

  if (mode == LARGEST && biggestSeg && biggestSeg->m_size >= size)
    *asize = (foundSeg = biggestSeg)->m_size;

  if (!foundSeg || !foundSeg->m_size)
  {                             /* no block to fullfill the request */
    if ((mode != LARGEST) && (mode & FIRST_FIT_U) &&
	uppermem_link && uppermem_root != 0xffff)
    {
      mode &= ~FIRST_FIT_U;
      goto searchAgain;
    }
    if (asize)
      *asize = biggestSeg ? biggestSeg->m_size : 0;
    return DE_NOMEM;
  }

stopIt:                        /* reached from FIRST_FIT on match */

  if (mode != LARGEST && size != foundSeg->m_size)
  {
    /* Split the found buffer because it is larger than requested */
    /* foundSeg := pointer to allocated block
       p := pointer to MCB that will form the rest of the block
     */
    if ((mode == LAST_FIT) || (mode == LAST_FIT_UO)
        || (mode == LAST_FIT_U))
    {
      /* allocate the block from the end of the found block */
      p = foundSeg;
      p->m_size -= size + 1;    /* size+1 paragraphes are allocated by
                                   the new segment (+1 for MCB itself) */
      foundSeg = nxtMCB(p);

      /* initialize stuff because foundSeg > p */
      foundSeg->m_type = p->m_type;
      p->m_type = MCB_NORMAL;
    }
    else
    {                           /* all other modes allocate from the beginning */
      p = nxtMCBsize(foundSeg, size);
      p->m_size = foundSeg->m_size - size - 1;

      /* initialize stuff because p > foundSeg  */
      p->m_type = foundSeg->m_type;
      foundSeg->m_type = MCB_NORMAL;
    }

    /* Already initialized:
       p->m_size, ->m_type, foundSeg->m_type
     */
    p->m_psp = FREE_PSP;        /* unused */

    foundSeg->m_size = size;
  }

  /* Already initialized:
     foundSeg->m_size, ->m_type
   */
  foundSeg->m_psp = cu_psp;     /* the new block is for current process */
  foundSeg->m_name[0] = '\0';

  *para = far2para((VOID FAR *) (BYTE FAR *) foundSeg);
  return SUCCESS;
}

/*
 * Unlike the name and the original prototype could suggest, this function
 * is used to return the _size_ of the largest available block rather than
 * the block itself.
 *
 * Known bug: a memory area with a size of the data area of 0 (zero) is
 * not considered a "largest" block. <<Perhaps this is a feature ;-)>>
 */
COUNT DosMemLargest(UWORD FAR * size)
{
  seg dummy;
  *size = 0;
  DosMemAlloc(0xffff, LARGEST, &dummy, size);
  if (mem_access_mode & 0x80) /* then the largest block is probably low! */
  {
    UWORD lowsize = 0;
    mem_access_mode &= ~0x80;
    DosMemAlloc(0xffff, LARGEST, &dummy, &lowsize);
    mem_access_mode |= 0x80;
    if (lowsize > *size)
      *size = lowsize;
  }
  return *size ? SUCCESS : DE_NOMEM;
}

/*
 * Deallocate a memory block. para is the segment of the MCB itself
 * This function can be called with para == 0, which eases other parts
 * of the kernel.
 */
COUNT DosMemFree(UWORD para)
{
  REG mcb FAR *p;
  COUNT i;

  if (!para)                    /* let esp. the kernel call this fct with para==0 */
    return DE_INVLDMCB;

  /* Initialize                                           */
  p = para2far(para);

  /* check for corruption                         */
  if (!mcbValid(p))
    return DE_INVLDMCB;

  /* Mark the mcb as free so that we can later    */
  /* merge with other surrounding free mcb's      */
  p->m_psp = FREE_PSP;
  for (i = 0; i < 8; i++)
    p->m_name[i] = '\0';

#if 0
  /* Moved into allocating functions -- 1999/04/21 ska */
  /* Now merge free blocks                        */

  for (p = (mcb FAR *) (MK_FP(first_mcb, 0)); p->m_type != MCB_LAST; p = q)
  {
    /* make q a pointer to the next block   */
    q = nxtMCB(p);
    /* and test for corruption              */
    if (q->m_type != MCB_NORMAL && q->m_type != MCB_LAST)
      return DE_MCBDESTRY;
    if (p->m_psp != FREE_PSP)
      continue;

    /* test if next is free - if so merge   */
    if (q->m_psp == FREE_PSP)
    {
      /* Always flow type down on free */
      p->m_type = q->m_type;
      p->m_size += q->m_size + 1;
      /* and make pointers the same   */
      /* since the next free is now   */
      /* this block                   */
      q = p;
    }
  }
#endif
  return SUCCESS;
}

/*
 * Resize an allocated memory block.
 * para is the segment of the data portion of the block rather than
 * the segment of the MCB itself.
 *
 * If the block shall grow, it is resized to the maximal size less than
 * or equal to size. This is the way MS DOS is reported to work.
 */
COUNT DosMemChange(UWORD para, UWORD size, UWORD * maxSize)
{
  REG mcb FAR *p, FAR * q;
  REG COUNT i;

  /* Initialize                                                   */
  p = para2far(para - 1);       /* pointer to MCB */

  /* check for corruption                                         */
  if (!mcbValid(p))
    return DE_MCBDESTRY;

  /* check if to grow the block                                   */
  if (size > p->m_size)
  {
    /* first try to make the MCB larger by joining with any following
       unused blocks */
    if (joinMCBs(p) != SUCCESS)
      return DE_MCBDESTRY;

    if (size > p->m_size)
    {                           /* block is still too small */
      if (maxSize)
        *maxSize = p->m_size;
      return DE_NOMEM;
    }
  }

  /*       shrink it down                                         */
  if (size < p->m_size)
  {
    /* make q a pointer to the new next block               */
    q = nxtMCBsize(p, size);
    /* reduce the size of p and add difference to q         */
    q->m_type = p->m_type;
    q->m_size = p->m_size - size - 1;

    p->m_size = size;

    /* Make certian the old psp is not last (if it was)     */
    p->m_type = MCB_NORMAL;

    /* Mark the mcb as free so that we can later    */
    /* merge with other surrounding free mcb's      */
    q->m_psp = FREE_PSP;
    for (i = 0; i < 8; i++)
      q->m_name[i] = '\0';

    /* try to join q with the free mcb's following it if possible */
    if (joinMCBs(q) != SUCCESS)
      return DE_MCBDESTRY;
  }

  /* MS network client NET.EXE: DosMemChange sets the PSP              *
   *               not tested, if always, or only on success         TE*
   * only on success seems more logical to me - Bart                                                                                                                   */
  p->m_psp = cu_psp;

  return SUCCESS;
}

/*
 * Check the MCB chain for allocation corruption
 */
COUNT DosMemCheck(void)
{
  REG mcb FAR *p;
  REG mcb FAR *pprev = 0;

  /* Initialize                                           */
  p = para2far(first_mcb);

  /* Search through memory blocks                         */
  while (p->m_type != MCB_LAST) /* not all MCBs touched */
  {
    /* check for corruption                         */
    if (p->m_type != MCB_NORMAL)
    {
      put_string("dos mem corrupt, first_mcb=");
      put_unsigned(first_mcb, 16, 4);
      hexd("\nprev ", pprev, 16);
      hexd("notMZ", p, 16);
      return DE_MCBDESTRY;
    }

    /* not corrupted - but not end, bump the pointer */
    pprev = p;
    p = nxtMCB(p);
  }
  return SUCCESS;
}

COUNT FreeProcessMem(UWORD ps)
{
  mcb FAR *p;
  BYTE oldumbstate = uppermem_link;

  /* link in upper memory to free those , too */
  DosUmbLink(1);

  /* Search through all memory blocks                         */
  for (p = para2far(first_mcb);; p = nxtMCB(p))
  {

    if (!mcbValid(p))           /* check for corruption */
      return DE_MCBDESTRY;

    if (p->m_psp == ps)
      DosMemFree(FP_SEG(p));

    if (p->m_type == MCB_LAST)
      break;
  }

  DosUmbLink(oldumbstate);

  return SUCCESS;
}

#if 0
        /* seems to be superceeded by DosMemLargest
           -- 1999/04/21 ska */
COUNT DosGetLargestBlock(UWORD FAR * block)
{
  UWORD sz = 0;
  mcb FAR *p;
  *block = sz;

  /* Initialize                                           */
  p = (mcb FAR *) (MK_FP(first_mcb, 0));

  /* Search through memory blocks                         */
  for (;;)
  {
    /* check for corruption                         */
    if (p->m_type != MCB_NORMAL && p->m_type != MCB_LAST)
      return DE_MCBDESTRY;

    if (p->m_psp == FREE_PSP && p->m_size > sz)
      sz = p->m_size;

    /* not corrupted - if last we're OK!            */
    if (p->m_type == MCB_LAST)
      break;
    p = nxtMCB(p);
  }
  *block = sz;
  return SUCCESS;
}
#endif

#ifdef DEBUG
VOID show_chain(void)
{
  mcb FAR *p;
  p = para2far(first_mcb);

  for (;;)
  {
    mcb_print(p);
    if (p->m_type == MCB_LAST || p->m_type != MCB_NORMAL)
      return;
    else
      p = nxtMCB(p);
  }
}

VOID mcb_print(mcb FAR * mcbp)
{
  static BYTE buff[9];

  fmemcpy((BYTE FAR *) buff, (BYTE FAR *) (mcbp->m_name), 8);
  buff[8] = '\0';
  printf
      ("%04x:%04x -> |%s| m_type = 0x%02x '%c'; m_psp = 0x%04x; m_size = 0x%04x\n",
       FP_SEG(mcbp), FP_OFF(mcbp), *buff == '\0' ? "*NO-ID*" : buff,
       mcbp->m_type, mcbp->m_type > ' ' ? mcbp->m_type : ' ', mcbp->m_psp,
       mcbp->m_size);
}
#endif

VOID DosUmbLink(BYTE n)
{
  REG mcb FAR *p;
  REG mcb FAR *q;

  if (uppermem_root == 0xffff)
    return;

  q = p = para2far(first_mcb);
/* like a xor thing! */
  if ((uppermem_link == 1) && (n == 0))
  {
    while (FP_SEG(p) != uppermem_root)
    {
      if (mcbFree(p))
        joinMCBs(p);
      if (!mcbValid(p))
        goto DUL_exit;
      q = p;
      p = nxtMCB(p);
    }

    if (q->m_type == MCB_NORMAL) 
      q->m_type = MCB_LAST;
    uppermem_link = n;

  }
  else if ((uppermem_link == 0) && (n == 1))
  {
    while (q->m_type != MCB_LAST)
    {
      if (!mcbValid(q))
        goto DUL_exit;
      q = nxtMCB(q);
    }

    q->m_type = MCB_NORMAL;
    uppermem_link = n;
  }
DUL_exit:
  return;
}

#endif

/*
 * Log: memmgr.c,v - for newer log entries do "cvs log memmgr.c"
 *
 * Revision 1.4  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.3  1999/08/25 03:18:09  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.2  1999/04/23 04:24:39  jprice
 * Memory manager changes made by ska
 *
 * Revision 1.1.1.1  1999/03/29 15:41:20  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.6   04 Jan 1998 23:15:18   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   16 Jan 1997 12:47:00   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.4   29 May 1996 21:03:34   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:54:20   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:51:58   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:33:08   patv
 * Initial revision.
 */
