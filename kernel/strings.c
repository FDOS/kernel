/****************************************************************/
/*                                                              */
/*                          strings.c                           */
/*                                                              */
/*                Global String Handling Functions              */
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

#ifdef VERSION_STRINGS
static BYTE *stringsRcsId = "$Id$";
#endif

#ifndef I86
COUNT strlen(REG BYTE * s)
{
  REG WORD cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

COUNT fstrlen(REG BYTE FAR * s)
{
  REG WORD cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

VOID _fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s)
{
  while (*s != 0)
    *d++ = *s++;
  *d = 0;
}

VOID strncpy(REG BYTE * d, REG BYTE * s, COUNT l)
{
  COUNT idx = 1;
  while (*s != 0 && idx++ <= l)
    *d++ = *s++;
  *d = 0;
}

COUNT strcmp(REG BYTE * d, REG BYTE * s)
{
  while (*s != '\0' && *d != '\0')
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

COUNT fstrcmp(REG BYTE FAR * d, REG BYTE FAR * s)
{
  while (*s != '\0' && *d != '\0')
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

COUNT strncmp(REG BYTE * d, REG BYTE * s, COUNT l)
{
  COUNT index = 1;
  while (*s != '\0' && *d != '\0' && index++ <= l)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

COUNT fstrncmp(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l)
{
  COUNT index = 1;
  while (*s != '\0' && *d != '\0' && index++ <= l)
  {
    if (*d == *s)
      ++s, ++d;
    else
      return *d - *s;
  }
  return *d - *s;
}

VOID fstrncpy(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l)
{
  COUNT idx = 1;
  while (*s != 0 && idx++ <= l)
    *d++ = *s++;
  *d = 0;
}

BYTE *strchr(BYTE * s, BYTE c)
{
  REG BYTE *p;
  p = s - 1;
  do
  {
    if (*++p == c)
      return p;
  }
  while (*p);
  return 0;
}
#endif

/*
 * Log: strings.c,v - see "cvs log strings.c" for newer entries
 *
 * Revision 1.4  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.3  1999/08/25 03:18:09  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:32  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/04 03:14:07  jprice
 * Formating.  Added comments.
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:27  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.6   04 Jan 1998 23:15:16   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   31 Dec 1997  3:59:30   patv
 * Added new far string functions.
 *
 *    Rev 1.4   29 May 1996 21:03:30   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:54:22   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:51:58   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:33:46   patv
 * Initial revision.
 */

