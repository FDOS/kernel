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

#ifdef FORSYS
#include <io.h>
#include <stdarg.h>
#endif

#ifdef _INIT
#define fstrlen init_fstrlen
#define handle_char init_handle_char
#define put_console init_put_console
#define ltob init_ltob
#define do_printf init_do_printf
#define printf init_printf
#define sprintf init_sprintf
#define charp init_charp
#endif

#ifdef VERSION_STRINGS
static BYTE *prfRcsId =
    "$Id$";
#endif

/* special console output routine */
/*#define DOSEMU */
#ifdef DOSEMU

#define MAX_BUFSIZE 80                       /* adjust if necessary */
static int buff_offset = 0;
static char buff[MAX_BUFSIZE];

VOID put_console(COUNT c)
{
  if (buff_offset >= MAX_BUFSIZE)
  {
    buff_offset = 0;
    printf("Printf buffer overflow!\n");
  }
  if (c == '\n')
  {
    buff[buff_offset] = 0;
    buff_offset = 0;
#ifdef __TURBOC__
    _ES = FP_SEG(buff);
    _DX = FP_OFF(buff);
    _AX = 0x13;
    __int__(0xe6);
#elif defined(I86)
    asm
      {
        push ds;
        pop es;
        mov dx, offset buff;
        mov ax, 0x13;
        int 0xe6;
      }
#endif
  }
  else
  {
    buff[buff_offset] = c;
    buff_offset++;
  }
}
#else
VOID put_console(COUNT c)
{
  if (c == '\n')
    put_console('\r');

#ifdef FORSYS
  write(1, &c, 1);              /* write character to stdout */
#else
#if defined(__TURBOC__)
  _AX = 0x0e00 | c;
  _BX = 0x0070;
  __int__(0x10);
#elif defined(I86)
  __asm
  {
    mov al, byte ptr c;
    mov ah, 0x0e;
    mov bx, 0x0070;
    int 0x10;
  }
#endif                          /* __TURBO__ */
#endif                          /*  FORSYS   */
}
#endif                          /*  DOSEMU   */

#if defined(DEBUG) || defined(FORSYS) || defined(_INIT)

#ifndef FORSYS
/* copied from bcc (Bruce's C compiler) stdarg.h */
typedef char *va_list;
#define va_start(arg, last) ((arg) = (char *) (&(last)+1))
#define va_arg(arg, type) (((type *)(arg+=sizeof(type)))[-1])
#define va_end(arg)
#endif

static BYTE *charp = 0;

STATIC VOID handle_char(COUNT);
STATIC VOID put_console(COUNT);
STATIC BYTE * ltob(LONG, BYTE *, COUNT);
STATIC COUNT do_printf(CONST BYTE *, REG va_list);
int CDECL printf(CONST BYTE * fmt, ...);

/* The following is user supplied and must match the following prototype */
VOID cso(COUNT);

#if defined(FORSYS) || defined(_INIT)
COUNT fstrlen(BYTE FAR * s)     /* don't want globals.h, sorry */
{
  int i = 0;

  while (*s++)
    i++;

  return i;
}
#else
COUNT /*ASMCFUNC*/ pascal fstrlen(BYTE FAR * s);   /* don't want globals.h, sorry */
#endif

/* special handler to switch between sprintf and printf */
STATIC VOID handle_char(COUNT c)
{
  if (charp == 0)
    put_console(c);
  else
    *charp++ = c;
}

/* ltob -- convert an long integer to a string in any base (2-16) */
BYTE *ltob(LONG n, BYTE * s, COUNT base)
{
  ULONG u;
  BYTE *p, *q;
  int c;

  u = n;

  if (base == -10)              /* signals signed conversion */
  {
    base = 10;
    if (n < 0)
    {
      u = -n;
      *s++ = '-';
    }
  }

  p = q = s;
  do
  {                             /* generate digits in reverse order */
    *p++ = "0123456789abcdef"[(UWORD) (u % base)];
  }
  while ((u /= base) > 0);

  *p = '\0';                    /* terminate the string */
  while (q < --p)
  {                             /* reverse the digits */
    c = *q;
    *q++ = *p;
    *p = c;
  }
  return s;
}

#define LEFT    0
#define RIGHT   1

/* printf -- short version of printf to conserve space */
int CDECL printf(CONST BYTE * fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  charp = 0;
  return do_printf(fmt, arg);
}

int CDECL sprintf(BYTE * buff, CONST BYTE * fmt, ...)
{
  WORD ret;
  va_list arg;

  va_start(arg, fmt);
  charp = buff;
  ret = do_printf(fmt, arg);
  handle_char(NULL);
  return ret;
}

/*
ULONG FAR retcs(int i)
{
    char *p = (char*)&i;
    
    p -= 4;
    return *(ULONG *)p;
}
*/
COUNT do_printf(CONST BYTE * fmt, va_list arg)
{
  int base;
  BYTE s[11], FAR * p;
  int c, flag, size, fill;
  int longarg;
  long currentArg;

/*  
  long cs = retcs(1);
  put_console("0123456789ABCDEF"[(cs >> 28) & 0x0f]);
  put_console("0123456789ABCDEF"[(cs >> 24) & 0x0f]);
  put_console("0123456789ABCDEF"[(cs >> 20) & 0x0f]);
  put_console("0123456789ABCDEF"[(cs >> 16) & 0x0f]);
  put_console(':');
  put_console("0123456789ABCDEF"[(cs >> 12) & 0x0f]);
  put_console("0123456789ABCDEF"[(cs >>  8) & 0x0f]);
  put_console("0123456789ABCDEF"[(cs >>  4) & 0x0f]);
  put_console("0123456789ABCDEF"[(cs >>  0) & 0x0f]);
*/
  while ((c = *fmt++) != '\0')
  {
    if (c != '%')
    {
      handle_char(c);
      continue;
    }

    longarg = FALSE;
    size = 0;
    flag = RIGHT;
    fill = ' ';

    if (*fmt == '-')
    {
      flag = LEFT;
      fmt++;
    }

    if (*fmt == '0')
    {
      fill = '0';
      fmt++;
    }

    while (*fmt >= '0' && *fmt <= '9')
    {
      size = size * 10 + (*fmt++ - '0');
    }

    if (*fmt == 'l')
    {
      longarg = TRUE;
      fmt++;
    }

    c = *fmt++;
    switch (c)
    {
      case '\0':
        return 0;

      case 'c':  
        handle_char(va_arg(arg, int));
        continue;

      case 'p':
        {
          UWORD w0 = va_arg(arg, UWORD);
          char *tmp = charp;
          sprintf(s, "%04x:%04x", va_arg(arg, UWORD), w0);
          p = s;
          charp = tmp;
          goto do_outputstring;
        }

      case 's':
        p = va_arg(arg, char *);
        goto do_outputstring;

      case 'F':
        fmt++;
        /* we assume %Fs here */
      case 'S':
        p = va_arg(arg, char FAR *);
        goto do_outputstring;

      case 'i':
      case 'd':
        base = -10;
        goto lprt;

      case 'o':
        base = 8;
        goto lprt;

      case 'u':
        base = 10;
        goto lprt;

      case 'X':
      case 'x':
        base = 16;

      lprt:
        if (longarg)
          currentArg = va_arg(arg, long);
        else
          currentArg = base < 0 ? (long)va_arg(arg, int) :
              (long)va_arg(arg, unsigned int);
        ltob(currentArg, s, base);

        p = s;
      do_outputstring:

        size -= fstrlen(p);

        if (flag == RIGHT)
        {
          for (; size > 0; size--)
            handle_char(fill);
        }
        for (; *p != '\0'; p++)
          handle_char(*p);

        for (; size > 0; size--)
          handle_char(fill);

        continue;

      default:
        handle_char('?');

        handle_char(c);
        break;

    }
  }
  va_end(arg);
  return 0;
}

#endif
#if !defined(FORSYS) && !defined(_INIT)

extern void put_string(const char *);
extern void put_unsigned(unsigned, int, int);

void hexd(char *title, UBYTE FAR * p, COUNT numBytes)
{
  int loop, start = 0;
  put_string(title);
  if (numBytes > 16)
    put_console('\n');

  for (start = 0; start < numBytes; start += 16)
  {
    put_unsigned(FP_SEG(p), 16, 4);
    put_console(':');
    put_unsigned(FP_OFF(p + start), 16, 4);
    put_console('|');
    for (loop = start; loop < numBytes && loop < start+16;loop++)
    {
      put_unsigned(p[loop], 16, 2);
      put_console(' ');
    }   
    for (loop = start; loop < numBytes && loop < start+16;loop++)
      put_console(p[loop] < 0x20 ? '.' : p[loop]);
    put_console('\n');
  }
}

/* put_unsigned -- print unsigned int in base 2--16 */
void put_unsigned(unsigned n, int base, int width)
{
  char s[6];
  int i;

  for (i = 0; i < width; i++)
  {                             /* generate digits in reverse order */
    s[i] = "0123456789abcdef"[(UWORD) (n % base)];
    n /= base;
  }

  while(i != 0)
  {                             /* print digits in reverse order */
    put_console(s[--i]);
  }
}

void put_string(const char *s)
{
  while(*s != '\0')
    put_console(*s++);
}

#endif

#ifdef TEST
/*
	this testprogram verifies that the strings are printed correctly
	( or the way, I expect them to print)
	
	compile like (note -DTEST !)

	c:\tc\tcc -DTEST -DI86 -I..\hdr prf.c
	
	and run. if strings are wrong, the program will wait for the ANYKEY

*/
#include <conio.h>
void cso(char c)
{
  putch(c);
}

struct {
  char *should;
  char *format;
  unsigned lowint;
  unsigned highint;

} testarray[] = {
  {
  "hello world", "%s %s", (unsigned)"hello", (unsigned)"world"},
  {
  "hello", "%3s", (unsigned)"hello", 0},
  {
  "  hello", "%7s", (unsigned)"hello", 0},
  {
  "hello  ", "%-7s", (unsigned)"hello", 0},
  {
  "hello", "%s", (unsigned)"hello", 0},
  {
  "1", "%d", 1, 0},
  {
  "-1", "%d", -1, 0},
  {
  "65535", "%u", -1, 0},
  {
  "-32768", "%d", 0x8000, 0},
  {
  "32767", "%d", 0x7fff, 0},
  {
  "-32767", "%d", 0x8001, 0},
  {
  "8000", "%x", 0x8000, 0},
  {
  "   1", "%4x", 1, 0},
  {
  "0001", "%04x", 1, 0},
  {
  "1   ", "%-4x", 1, 0},
  {
  "1000", "%-04x", 1, 0},
  {
  "1", "%ld", 1, 0},
  {
  "-1", "%ld", -1, -1},
  {
  "65535", "%ld", -1, 0},
  {
  "65535", "%u", -1, 0},
  {
  "8000", "%lx", 0x8000, 0},
  {
  "80000000", "%lx", 0, 0x8000},
  {
  "   1", "%4lx", 1, 0},
  {
  "0001", "%04lx", 1, 0},
  {
  "1   ", "%-4lx", 1, 0},
  {
  "1000", "%-04lx", 1, 0},
  {
  "-2147483648", "%ld", 0, 0x8000},
  {
  "2147483648", "%lu", 0, 0x8000},
  {
  "2147483649", "%lu", 1, 0x8000},
  {
  "-2147483647", "%ld", 1, 0x8000},
  {
  "32767", "%ld", 0x7fff, 0},
  {
"ptr 1234:5678", "ptr %p", 0x5678, 0x1234}, 0};

test(char *should, char *format, unsigned lowint, unsigned highint)
{
  char b[100];

  sprintf(b, format, lowint, highint);

  printf("'%s' = '%s'\n", should, b);

  if (strcmp(b, should))
  {
    printf("\nhit the ANYKEY\n");
    getch();
  }
}

main()
{
  int i;
  printf("hello world\n");

  for (i = 0; testarray[i].should; i++)
  {
    test(testarray[i].should, testarray[i].format, testarray[i].lowint,
         testarray[i].highint);
  }
}
#endif

