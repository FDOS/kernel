/****************************************************************/
/*                                                              */
/*                            buffer.h                          */
/*                                                              */
/* Sector buffer structure                                      */
/*                                                              */
/*                      Copyright (c) 2001                      */
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
static BYTE *buffer_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.1  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 *         Rev 1.0   20 Apr 2001 17:30:00   Bart Oldeman
 *      Initial revision.
 */

#define BUFFERSIZE 512
struct buffer
{
  struct buffer
  FAR *b_next;                  /* form linked list for LRU     */
  BYTE b_unit;                  /* disk for this buffer         */
  BYTE b_flag;                  /* buffer flags                 */
  ULONG b_blkno;                /* block for this buffer        */
  /* DOS-C: 0xffff for huge block numbers */
  BYTE b_copies;                /* number of copies to write    */
  UBYTE b_offset_lo;            /* span between copies (low)                                                    */
#if 0 /*TE*/
 union
  {
    struct dpb FAR *_b_dpbp;    /* pointer to DPB                                                                                                                                               */
    LONG _b_huge_blkno;         /* DOS-C: actual block number if >= 0xffff */
  }
  _b;
#endif  
  UBYTE b_offset_hi;            /* DOS-C: span between copies (high) */
  UBYTE b_unused;
  BYTE b_buffer[BUFFERSIZE];    /* 512 byte sectors for now     */
};

#define b_dpbp          _b._b_dpbp
#define b_huge_blkno    _b._b_huge_blkno

#define BFR_DIRTY       0x40    /* buffer modified              */
#define BFR_VALID       0x20    /* buffer contains valid data   */
#define BFR_DATA        0x08    /* buffer is from data area     */
#define BFR_DIR         0x04    /* buffer is from dir area      */
#define BFR_FAT         0x02    /* buffer is from fat area      */
#define BFR_BOOT        0x01    /* buffer is boot disk          */
