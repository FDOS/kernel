/****************************************************************/
/*                                                              */
/*                            buffer.h                          */
/*                                                              */
/* Sector buffer structure                                      */
/*                                                              */
/*                      Copyright (c) 2001                      */
/*			Bart Oldeman				*/
/*								*/
/*			Largely taken from globals.h:		*/
/*			Copyright (c) 1995, 1996                */
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
 * Revision 1.4  2001/08/19 12:58:34  bartoldeman
 * Time and date fixes, Ctrl-S/P, findfirst/next, FCBs, buffers, tsr unloading
 *
 * Revision 1.3  2001/07/24 16:56:29  bartoldeman
 * fixes for FCBs, DJGPP ls, DBLBYTE, dyninit allocation (2024e).
 *
 * Revision 1.2  2001/06/03 14:16:17  bartoldeman
 * BUFFERS tuning and misc bug fixes/cleanups (2024c).
 *
 * Revision 1.1  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 *         Rev 1.0   20 Apr 2001 17:30:00   Bart Oldeman
 *      Initial revision.
 */

#define BUFFERSIZE 512
struct buffer
{
  WORD b_dummy;                 /* dummy self pointing word to keep MFT from crashing */	
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

#define BFR_UNCACHE     0x80    /* indication, not really used  */
#define BFR_DIRTY       0x40    /* buffer modified              */
#define BFR_VALID       0x20    /* buffer contains valid data   */
#define BFR_DATA        0x08    /* buffer is from data area     */
#define BFR_DIR         0x04    /* buffer is from dir area      */
#define BFR_FAT         0x02    /* buffer is from fat area      */
#define BFR_BOOT        0x01    /* buffer is boot disk          */
