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
static BYTE *miscRcsId =
    "$Id$";
#endif

#include "globals.h"
#ifndef I86

char *strcpy(REG BYTE * d, REG CONST BYTE * s)
{
  char *tmp = d;
  
  while ((*d++ = *s++) != '\0')
    ;

  return tmp;
}

VOID fstrcpy(REG BYTE FAR * d, REG CONST BYTE FAR * s)
{
  while (*s)
    *d++ = *s++;
  *d = '\0';
}

VOID fstrncpy(BYTE FAR * d, BYTE CONST FAR * s, REG size_t n)
{
  while (*s && n--)
    *d++ = *s++;
  *d = '\0';
}

VOID * memcpy(REG VOID * d, REG CONST VOID * s, REG size_t n)
{
  char *cd = d;
  CONST char *cs = s;
  
  while (n--)
    *cd++ = *cs++;
  return d;
}

VOID fmemcpy(REG VOID FAR * d, REG CONST VOID FAR * s, REG size_t n)
{
  while (n--)
    *((BYTE FAR *) d)++ = *((BYTE FAR *) s)++;
}

VOID fmemset(REG VOID FAR * s, REG int ch, REG size_t n)
{
  while (n--)
    *((BYTE FAR *) s)++ = ch;
}

#endif

