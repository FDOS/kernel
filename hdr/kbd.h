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
static BYTE *kbd_hRcsId = "$Id$";
#endif
#endif


#define KBD_MAXLENGTH   256

/* Keyboard buffer                                                      */
typedef struct
{
  UBYTE kb_size;                /* size of buffer in bytes              */
  UBYTE kb_count;               /* number of bytes returned             */
  BYTE kb_buf[KBD_MAXLENGTH];   /* the buffer itself            */
}
keyboard;

/*
 * Log: kbd.h,v 
 *
 * Revision 1.1.1.1  1999/03/29 15:39:31  jprice
 * New version without IPL.SYS
 *
 * Revision 1.3  1999/02/01 01:40:06  jprice
 * Clean up
 *
 * Revision 1.2  1999/01/22 04:17:40  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *         Rev 1.5   04 Jan 1998 23:14:18   patv
 *      Changed Log for strip utility
 *
 *         Rev 1.4   29 May 1996 21:25:20   patv
 *      bug fixes for v0.91a
 *
 *         Rev 1.3   19 Feb 1996  3:15:28   patv
 *      Added NLS, int2f and config.sys processing
 *
 *         Rev 1.2   01 Sep 1995 17:35:38   patv
 *      First GPL release.
 *
 *         Rev 1.1   30 Jul 1995 20:42:22   patv
 *      fixed ipl
 *
 *         Rev 1.0   02 Jul 1995 10:39:46   patv
 *      Initial revision.
 */
