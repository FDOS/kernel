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
static BYTE *stringsRcsId =
    "$Id$";
#endif

#ifndef I86
size_t strlen(REG CONST BYTE * s)
{
  REG size_t cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

size_t fstrlen(REG CONST BYTE FAR * s)
{
  REG size_t cnt = 0;

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

char *strncpy(register char *d, register const char *s, size_t l)
{
  size_t idx = 1;
  char *tmp = d;
  while (*s != 0 && idx++ <= l)
    *d++ = *s++;
  *d = 0;
  return tmp;
}

int strcmp(REG CONST BYTE * d, REG CONST BYTE * s)
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

int strncmp(register const char *d, register const char *s, size_t l)
{
  size_t index = 1;
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

char *strchr(const char * s, int c)
{
  REG CONST BYTE *p;
  p = s - 1;
  do
  {
    if (*++p == (char)c)
      return (char *)p;
  }
  while (*p);
  return 0;
}

void *memchr(const void * s, int c)
{
  REG unsigned char *p;
  p = (unsigned char *)s - 1;
  do
  {
    if (*++p == (unsigned char)c)
      return (void *)p;
  }
  while (*p);
  return 0;
}
#endif

