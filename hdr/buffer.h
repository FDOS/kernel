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
static BYTE *buffer_hRcsId =
    "$Id$";
#endif
#endif

#include "dsk.h"                /* only for MAX_SEC_SIZE        */
#define BUFFERSIZE MAX_SEC_SIZE

struct buffer {
  UWORD b_next;                 /* next buffer in LRU list      */
  UWORD b_prev;                 /* previous buffer in LRU list  */
  BYTE b_unit;                  /* disk for this buffer         */
  BYTE b_flag;                  /* buffer flags                 */
  ULONG b_blkno;                /* block for this buffer        */
  UBYTE b_copies;               /* number of copies to write    */
  UWORD b_offset;               /* offset in sectors between copies
                                   to write for FAT sectors     */
  struct dpb FAR *b_dpbp;       /* pointer to DPB               */
  UWORD b_remotesz;             /* size of remote buffer if remote */
  BYTE b_padding;
  UBYTE b_buffer[BUFFERSIZE];   /* 512 byte sectors for now     */
};

#define BFR_DIRTY       0x40    /* buffer modified              */
#define BFR_VALID       0x20    /* buffer contains valid data   */
#define BFR_DATA        0x08    /* buffer is from data area     */
#define BFR_DIR         0x04    /* buffer is from dir area      */
#define BFR_FAT         0x02    /* buffer is from fat area      */
#define BFR_UNCACHE     0x01    /* buffer to be released ASAP   */

