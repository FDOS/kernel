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

/*
 * $Log$
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
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

#include "globals.h"

VOID scopy(REG BYTE * s, REG BYTE * d)
{
  while (*s)
    *d++ = *s++;
  *d = '\0';
}

VOID FAR init_call_scopy(REG BYTE * s, REG BYTE * d)
{
  scopy(s, d);
}

VOID fscopy(REG BYTE FAR * s, REG BYTE FAR * d)
{
  while (*s)
    *d++ = *s++;
  *d = '\0';
}

VOID fsncopy(BYTE FAR * s, BYTE FAR * d, REG COUNT n)
{
  while (*s && n--)
    *d++ = *s++;
  *d = '\0';
}

#ifndef ASMSUPT
VOID bcopy(REG BYTE * s, REG BYTE * d, REG COUNT n)
{
  while (n--)
    *d++ = *s++;
}

VOID fbcopy(REG VOID FAR * s, REG VOID FAR * d, REG COUNT n)
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

VOID FAR init_call_fbcopy(REG VOID FAR * s, REG VOID FAR * d, REG COUNT n)
{
  fbcopy(s, d, n);
}

VOID fmemset(VOID FAR *, int, COUNT);

VOID FAR init_call_fmemset(REG VOID FAR * s, REG int ch, REG COUNT n)
{
  fmemset(s, ch, n);
}
