/****************************************************************/
/*                                                              */
/*                            prf.c                             */
/*                                                              */
/*                  Abbreviated printf Function                 */
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
static BYTE *prfRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.3  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:42:20  jprice
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
 *    Rev 1.4   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.3   29 May 1996 21:15:10   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:42   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:05:10   patv
 * Initial revision.
 */

static BYTE *charp;

#ifdef PROTO
VOID handle_char(COUNT);
VOID put_console(COUNT);
BYTE *ltob(LONG, BYTE *, COUNT);
static BYTE *itob(COUNT, BYTE *, COUNT);
COUNT do_printf(CONST BYTE *, REG BYTE **);
#else
VOID handle_char();
VOID put_console();
BYTE *ltob();
static BYTE *itob();
COUNT do_printf();
#endif

/* The following is user supplied and must match the following prototype */
#ifdef PROTO
VOID cso(COUNT);
#else
VOID cso();
#endif

/* special console output routine */
VOID
put_console(COUNT c)
{
  if (c == '\n')
    cso('\r');
  cso(c);
}

/* special handler to switch between sprintf and printf */
static VOID
  handle_char(COUNT c)
{
  if (charp == 0)
    put_console(c);
  else
    *charp++ = c;
}

/* ltob -- convert an long integer to a string in any base (2-16) */
static BYTE *
  ltob(LONG n, BYTE * s, COUNT base)
{
  ULONG u;
  REG BYTE *p,
   *q;
  REG negative,
    c;

  if (n < 0 && base == -10)
  {
    negative = 1;
    u = -n;
  }
  else
  {
    negative = 0;
    u = n;
  }
  if (base == -10)              /* signals signed conversion */
    base = 10;
  p = q = s;
  do
  {                             /* generate digits in reverse order */
    *p++ = "0123456789abcdef"[u % base];
  }
  while ((u /= base) > 0);
  if (negative)
    *p++ = '-';
  *p = '\0';                    /* terminate the string */
  while (q < --p)
  {                             /* reverse the digits */
    c = *q;
    *q++ = *p;
    *p = c;
  }
  return s;
}

/* itob -- convert an long integer to a string in any base (2-16) */
static BYTE *
  itob(COUNT n, BYTE * s, COUNT base)
{
  UWORD u;
  REG BYTE *p,
   *q;
  REG negative,
    c;

  if (n < 0 && base == -10)
  {
    negative = 1;
    u = -n;
  }
  else
  {
    negative = 0;
    u = n;
  }
  if (base == -10)              /* signals signed conversion */
    base = 10;
  p = q = s;
  do
  {                             /* generate digits in reverse order */
    *p++ = "0123456789abcdef"[u % base];
  }
  while ((u /= base) > 0);
  if (negative)
    *p++ = '-';
  *p = '\0';                    /* terminate the string */
  while (q < --p)
  {                             /* reverse the digits */
    c = *q;
    *q++ = *p;
    *p = c;
  }
  return s;
}

#define NONE    0
#define LEFT    1
#define RIGHT   2

/* printf -- short version of printf to conserve space */
WORD FAR
  init_call_printf(CONST BYTE * fmt, BYTE * args)
{
  charp = 0;
  return do_printf(fmt, &args);
}

WORD
sprintf(BYTE * buff, CONST BYTE * fmt, BYTE * args)
{
  WORD ret;

  charp = buff;
  ret = do_printf(fmt, &args);
  handle_char(NULL);
  return ret;
}

static COUNT
  do_printf(CONST BYTE * fmt, REG BYTE ** arg)
{
  REG base;
  BYTE s[11],
   *p,
   *ltob();
  BYTE c,
    slen,
    flag,
    size,
    fill;

  flag = NONE;
  size = 0;
  while ((c = *fmt++) != '\0')
  {
    if (size == 0 && flag == NONE && c != '%')
    {
      handle_char(c);
      continue;
    }
    if (flag == NONE && *fmt == '0')
    {
      flag = RIGHT;
      fill = '0';
    }
    switch (*fmt)
    {
      case '-':
        flag = RIGHT;
        fill = *(fmt + 1) == '0' ? '0' : ' ';
        continue;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if (flag == NONE)
          flag = LEFT;
        size = *fmt++ - '0';
        while ((c = *fmt++) != '\0')
        {
          switch (c)
          {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              size = size * 10 + (c - '0');
              continue;

            default:
              --fmt;
              break;
          }
          break;
        }
        break;
    }
    switch (c = *fmt++)
    {
      case 'c':
        handle_char(*(COUNT *) arg++);
        continue;

      case 'd':
        base = -10;
        goto prt;

      case 'o':
        base = 8;
        goto prt;

      case 'u':
        base = 10;
        goto prt;

      case 'x':
        base = 16;

      prt:
        itob(*((COUNT *) arg)++, s, base);
        if (flag == RIGHT || flag == LEFT)
        {
          for (slen = 0, p = s; *p != '\0'; p++)
            ++slen;
        }
        if (flag == RIGHT && slen < size)
        {
          WORD i;

          for (i = size - slen; i > 0; i--)
            handle_char(fill);
        }
        for (p = s; *p != '\0'; p++)
          handle_char(*p);
        if (flag == LEFT)
        {
          WORD i;
          BYTE sp = ' ';

          for (i = size - slen; i > 0; i--)
            handle_char(sp);
        }
        size = 0;
        flag = NONE;
        continue;

      case 'l':
        switch (c = *fmt++)
        {
          case 'd':
            base = -10;
            goto lprt;

          case 'o':
            base = 8;
            goto lprt;

          case 'u':
            base = 10;
            goto lprt;

          case 'x':
            base = 16;

          lprt:
            ltob(*((LONG *) arg)++, s, base);
            if (flag == RIGHT || flag == LEFT)
            {
              for (slen = 0, p = s; *p != '\0'; p++)
                ++slen;
            }
            if (flag == RIGHT && slen < size)
            {
              WORD i;

              for (i = size - slen; i > 0; i--)
                handle_char(fill);
            }
            for (p = s; *p != '\0'; p++)
              handle_char(*p);
            if (flag == LEFT)
            {
              WORD i;
              BYTE sp = ' ';

              for (i = size - slen; i > 0; i--)
                handle_char(sp);
            }
            size = 0;
            flag = NONE;
            continue;

          default:
            handle_char(c);
        }

      case 's':
        for (p = *arg; *p != '\0'; p++)
        {
          --size;
          handle_char(*p);
        }
        for (; size > 0; size--)
          handle_char(' ');
        ++arg;
        size = 0;
        flag = NONE;
        continue;

      default:
        handle_char(c);
        continue;
    }
  }
  return 0;
}
