/****************************************************************/
/*                                                              */
/*                           misc.c                             */
/*                                                              */
/*                 Miscellaneous Kernel Functions               */
/*                                                              */
/*                      Copyright (c) 1993                      */
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

#ifdef VERSION_STRINGS
static BYTE *miscRcsId = "$Id$";
#endif

#include "globals.h"
#ifndef I86

VOID strcpy(REG BYTE * d, REG BYTE * s)
{
  while (*s)
    *d++ = *s++;
  *d = '\0';
}

VOID fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s)
{
  while (*s)
    *d++ = *s++;
  *d = '\0';
}

VOID fstrncpy(BYTE FAR * d, BYTE FAR * s, REG COUNT n)
{
  while (*s && n--)
    *d++ = *s++;
  *d = '\0';
}

VOID memcpy(REG VOID * d, REG VOID * s, REG COUNT n)
{
  while (n--)
    *d++ = *s++;
}

VOID fmemcpy(REG VOID FAR * d, REG VOID FAR * s, REG COUNT n)
{
  while (n--)
    *((BYTE FAR *) d)++ = *((BYTE FAR *) s)++;
}

VOID fmemset(REG VOID FAR * s, REG int ch, REG COUNT n)
{
  while (n--)
    *((BYTE FAR *) s)++ = ch;
}

#endif

/*
 * Log: misc.c,v - for newer entries see "cvs log misc.c"
 *
 * Revision 1.4  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.3  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.2  1999/04/23 04:24:39  jprice
 * Memory manager changes made by ska
 *
 * Revision 1.1.1.1  1999/03/29 15:42:19  jprice
 * New version without IPL.SYS
 *
 * Revision 1.3  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.5   04 Jan 1998 23:14:36   patv
 * Changed Log for strip utility
 *
 *    Rev 1.4   29 May 1996 21:15:18   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:20:12   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:48:46   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:28   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:06:28   patv
 * Initial revision.
 */

