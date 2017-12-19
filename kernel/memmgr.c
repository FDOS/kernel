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
    "$Id: memmgr.c 1338 2007-07-20 20:52:33Z mceric $";
#endif

#define nxtMCBsize(mcb,size) MK_FP(FP_SEG(mcb) + (size) + 1, FP_OFF(mcb))
#define nxtMCB(mcb) nxtMCBsize((mcb), (mcb)->m_size)

#define mcbFree(mcb) ((mcb)->m_psp == FREE_PSP)
#define mcbValid(mcb)	( ((mcb)->m_size != 0xffff) && \
        ((mcb)->m_type == MCB_NORMAL || (mcb)->m_type == MCB_LAST) )
#define mcbFreeable(mcb)			\
	((mcb)->m_type == MCB_NORMAL || (mcb)->m_type == MCB_LAST)

#define para2far(seg) (mcb FAR *)MK_FP((seg) , 0)

/*
 * Join any following unused MCBs to MCB 'p'.
 *  Return:
 *  SUCCESS: on success
 *  else: error number <<currently DE_MCBDESTRY only>>
 */
STATIC COUNT joinMCBs(seg para)
{
  mcb FAR *p = para2far(para);
  mcb FAR *q;

  /* loop as long as the current MCB is not the last one in the chain
     and the next MCB is unused */
  while (p->m_type == MCB_NORMAL)
  {
    q = nxtMCB(p);
    if (!mcbFree(q))
      break;
    if (!mcbValid(q))
      return DE_MCBDESTRY;
    /* join both MCBs */
    p->m_type = q->m_type;      /* possibly the next MCB is the last one */
    p->m_size += q->m_size + 1; /* one for q's MCB itself */
#if 0				/* this spoils QB4's habit to double-free: */
    q->m_type = 'K';            /* Invalidate the magic number (whole MCB) */
#else
    q->m_size = 0xffff;		/* mark the now unlinked MCB as "fake" */
#endif
  }

  return SUCCESS;
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
COUNT DosMemAlloc(UWORD size, COUNT mode, seg *para, UWORD *asize)
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
  if ((uppermem_link & 1) && uppermem_root != 0xffff)
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
      if (joinMCBs(FP_SEG(p)) != SUCCESS)       /* join following unused blocks */
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
	(uppermem_link & 1) && uppermem_root != 0xffff)
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

      /* initialize stuff because p > foundSeg  */
      p->m_type = foundSeg->m_type;
      p->m_size = foundSeg->m_size - size - 1;
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

  *para = FP_SEG(foundSeg);
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
COUNT DosMemLargest(UWORD *size)
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

  if (!para)                    /* let esp. the kernel call this fct with para==0 */
    return DE_INVLDMCB;

  /* Initialize                                           */
  p = para2far(para);

  /* check for corruption                         */
  if (!mcbFreeable(p))	/* does not have to be valid, freeable is enough */
    return DE_INVLDMCB;

  /* Mark the mcb as free so that we can later    */
  /* merge with other surrounding free MCBs       */
  p->m_psp = FREE_PSP;
  fmemset(p->m_name, '\0', 8);

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
  REG mcb FAR *p; mcb FAR * q;

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
    if (joinMCBs(FP_SEG(p)) != SUCCESS)
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
    q->m_size = p->m_size - size - 1;
    q->m_type = p->m_type;

    p->m_size = size;

    /* Make certian the old psp is not last (if it was)     */
    p->m_type = MCB_NORMAL;

    /* Mark the mcb as free so that we can later    */
    /* merge with other surrounding free MCBs       */
    q->m_psp = FREE_PSP;
    fmemset(q->m_name, '\0', 8);

    /* try to join q with the free MCBs following it if possible */
    if (joinMCBs(FP_SEG(q)) != SUCCESS)
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
  BYTE oldumbstate = uppermem_link & 1;

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

  fmemcpy(buff, mcbp->m_name, 8);
  buff[8] = '\0';
  printf
      ("%04x:%04x -> |%s| m_type = 0x%02x '%c'; m_psp = 0x%04x; m_size = 0x%04x\n",
       FP_SEG(mcbp), FP_OFF(mcbp), *buff == '\0' ? "*NO-ID*" : buff,
       mcbp->m_type, mcbp->m_type > ' ' ? mcbp->m_type : ' ', mcbp->m_psp,
       mcbp->m_size);
}
#endif

void DosUmbLink(unsigned n)
{
  REG mcb FAR *p;
  REG mcb FAR *q;

  if (uppermem_root == 0xffff)
    return;

  p = para2far(first_mcb);
  if (n > 1 || (uppermem_link & 1) == n)
    return;
  while (FP_SEG(p) != uppermem_root && p->m_type != MCB_LAST)
  {
    if (!mcbValid(p))
      return;
    q = p;
    p = nxtMCB(p);
  }
  if (n == 0)
  {
    if (q->m_type == MCB_NORMAL && FP_SEG(p) == uppermem_root)
      q->m_type = MCB_LAST;
  }
  else if (p->m_type == MCB_LAST)
    p->m_type = MCB_NORMAL;
  else
    return;
  uppermem_link = n;
}

#endif

