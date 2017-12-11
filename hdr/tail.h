/****************************************************************/
/*                                                              */
/*                            tail.h                            */
/*                                                              */
/*                 Command tail data structures                 */
/*                                                              */
/*                         July 1, 1993                         */
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

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *tail_hRcsId =
    "$Id: tail.h 485 2002-12-09 00:17:15Z bartoldeman $";
#endif
#endif

#define CTBUFFERSIZE       127

typedef struct {
  UBYTE ctCount;                /* number of bytes returned             */
  char ctBuffer[CTBUFFERSIZE];  /* the buffer itself            */
} CommandTail;

