/****************************************************************/
/*                                                              */
/*                            kbd.h                             */
/*                                                              */
/*    Buffered Keyboard Input data structures & declarations    */
/*                                                              */
/*                         July 5, 1993                         */
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
static BYTE *kbd_hRcsId =
    "$Id: kbd.h 485 2002-12-09 00:17:15Z bartoldeman $";
#endif
#endif

#define LINEBUFSIZECON  128
#define KBD_MAXLENGTH   LINEBUFSIZECON+1 /* the above + LF */
#define LINEBUFSIZE0A   256 /* maximum length for int21/ah=0a */

/* Keyboard buffer                                                      */
typedef struct {
  UBYTE kb_size;                /* size of buffer in bytes              */
  UBYTE kb_count;               /* number of bytes returned             */
  BYTE kb_buf[KBD_MAXLENGTH];   /* the buffer itself            */
} keyboard;

