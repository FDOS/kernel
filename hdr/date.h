/****************************************************************/
/*                                                              */
/*                           date.h                             */
/*                                                              */
/*                  DOS General Date Structure                  */
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

/* TC 2.01 complains if `date' is defined twice. -- ror4 */
#ifndef DOSC_DATE_H
#define DOSC_DATE_H

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *date_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.3  2000/05/25 20:56:19  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:28:22  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.2  1999/08/25 03:17:11  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.1.1.1  1999/03/29 15:39:23  jprice
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
 *         Rev 1.3   19 Feb 1996  3:15:30   patv
 *      Added NLS, int2f and config.sys processing
 *
 *         Rev 1.2   01 Sep 1995 17:35:40   patv
 *      First GPL release.
 *
 *         Rev 1.1   30 Jul 1995 20:43:48   patv
 *      Eliminated version strings in ipl
 *
 *         Rev 1.0   02 Jul 1995 10:39:28   patv
 *      Initial revision.
 */

/* FAT file date - takes the form of yyyy yyym mmmd dddd where physical */
/* year=1980+yyyyyy                                                     */

#define DT_YEAR(d)      (((d)>>9)&0x7f)
#define DT_MONTH(d)     (((d)>>5)&0x0f)
#define DT_DAY(d)       ((d)&0x1f)

#define DT_ENCODE(m,d,y) ((((m)&0x0f)<<5)|((d)&0x1f)|(((y)&0x7f)<<9))

#define EPOCH_WEEKDAY   2       /* Tuesday (i. e.-  0 == Sunday)        */
#define EPOCH_MONTH     1       /* January                              */
#define EPOCH_DAY       1       /* 1 for January 1                      */
#define EPOCH_YEAR      1980    /* for Tues 1-1-80 epoch                */

typedef UWORD date;

#endif
