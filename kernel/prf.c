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
#ifdef __GNUC__
#include <unistd.h>
#else
#include <io.h>
#endif
#include <stdarg.h>
#endif

#ifdef _INIT
#define handle_char init_handle_char
#define put_console init_put_console
#define ltob init_ltob
#define do_printf init_do_printf
#define printf init_printf
#define sprintf init_sprintf
#define charp init_charp
#endif

#include "debug.h"  /* must be below xx to init_xx */

/* special console output routine */
/*#define DOSEMU */
#ifdef DOSEMU

#define MAX_BUFSIZE 80                       /* adjust if necessary */
static int buff_offset = 0;
static char buff[MAX_BUFSIZE];

void put_console(int c)
{
  if (buff_offset >= MAX_BUFSIZE)
  {
    buff_offset = 0;
    DebugPrintf(("Printf buffer overflow!\n"));
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
#ifdef __WATCOMC__
void int29(char c);
#pragma aux int29 = "int 0x29" parm [al] modify exact [bx];
#endif

void put_console(int c)
{
  if (c == '\n')
    put_console('\r');

#ifdef FORSYS
  write(1, &c, 1);              /* write character to stdout */
#else
#if defined(__TURBOC__)
  _AL = c;
  __int__(0x29);
#elif defined(__WATCOMC__)
  int29(c);
#elif defined(__GNUC__)
  asm volatile("int $0x29" : : "a"(c) : "bx");
#elif defined(I86)
  __asm
  {
    mov al, byte ptr c;
    int 0x29;
  }
#endif                          /* __TURBO__ */
#endif                          /*  FORSYS   */
}
#endif                          /*  DOSEMU   */

#if defined(DEBUG_NEED_PRINTF) || defined(FORSYS) || defined(_INIT) || defined(TEST)

#if defined(DEBUG_NEED_PRINTF) && !defined(_INIT) && !defined(FORSYS)
/* need to use FAR pointers for resident DEBUG printf()s where SS != DS */
#define SSFAR FAR
#else
#define SSFAR
#endif

#ifndef FORSYS
/* copied from bcc (Bruce's C compiler) stdarg.h */
typedef char SSFAR *va_list;
#define va_start(arg, last) ((arg) = (va_list) (&(last)+1))
#define va_arg(arg, type) (((type SSFAR *)(arg+=sizeof(type)))[-1])
#define va_end(arg)
#endif

static BYTE SSFAR *charp = 0;

STATIC VOID handle_char(COUNT);
STATIC void ltob(LONG, BYTE SSFAR *, COUNT);
STATIC void do_printf(const char *, REG va_list);

/* special handler to switch between sprintf and printf */
STATIC VOID handle_char(COUNT c)
{
  if (charp == 0)
    put_console(c);
  else
    *charp++ = c;
}

/* ltob -- convert an long integer to a string in any base (2-16) */
STATIC void ltob(LONG n, BYTE SSFAR * s, COUNT base)
{
  ULONG u;
  BYTE SSFAR *p, SSFAR *q;
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

  p = s;
  do
  {                             /* generate digits in reverse order */
    *p++ = "0123456789abcdef"[(UWORD) (u % base)];
  }
  while ((u /= base) > 0);

  *p = '\0';                    /* terminate the string */
  for (q = s; q < --p; q++)
  {                             /* reverse the digits */
    c = *q;
    *q = *p;
    *p = c;
  }
}

#define LEFT    0
#define RIGHT   1
#define ZEROSFILL 2
#define LONGARG 4

/* printf -- short version of printf to conserve space */
int VA_CDECL printf(CONST char *fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  charp = 0;
  do_printf(fmt, arg);
  return 0;
}

#if defined(DEBUG_NEED_PRINTF) && !defined(_INIT) && !defined(FORSYS)
STATIC int VA_CDECL fsprintf(char FAR * buff, CONST char * fmt, ...)
{
  va_list arg;

  va_start(arg, fmt);
  charp = buff;
  do_printf(fmt, arg);
  handle_char('\0');
  return 0;
}
#else
#define fsprintf sprintf
#endif

int VA_CDECL sprintf(char * buff, CONST char * fmt, ...)
{
  va_list arg;

  va_start(arg, fmt);
  charp = buff;
  do_printf(fmt, arg);
  handle_char('\0');
  return 0;
}

STATIC void do_printf(CONST BYTE * fmt, va_list arg)
{
  int base;
  BYTE s[11];
  BYTE FAR * p;
  int size;
  unsigned char flags;

  for (;*fmt != '\0'; fmt++)
  {
    if (*fmt != '%')
    {
      handle_char(*fmt);
      continue;
    }

    fmt++;
    flags = RIGHT;

    if (*fmt == '-')
    {
      flags = LEFT;
      fmt++;
    }

    if (*fmt == '0')
    {
      flags |= ZEROSFILL;
      fmt++;
    }

    size = 0;
    while (1)
    {
      unsigned c = (unsigned char)(*fmt - '0');
      if (c > 9)
        break;
      fmt++;
      size = size * 10 + c;
    }

    if (*fmt == 'l')
    {
      flags |= LONGARG;
      fmt++;
    }

    switch (*fmt)
    {
      case '\0':
        va_end(arg);
        return;

      case 'c':
        handle_char(va_arg(arg, int));
        continue;

      case 'p':
        {
          UWORD w0 = va_arg(arg, unsigned);
          char SSFAR *tmp = charp;
          fsprintf(s, "%04x:%04x", va_arg(arg, unsigned), w0);
          p = s;
          charp = tmp;
          break;
        }

      case 's':
        p = va_arg(arg, char *);
        break;

      case 'F':
        fmt++;
        /* we assume %Fs here */
      case 'S':
        p = va_arg(arg, char FAR *);
        break;

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
        {
          long currentArg;
          if (flags & LONGARG)
            currentArg = va_arg(arg, long);
          else
          {
            currentArg = va_arg(arg, int);
            if (base >= 0)
              currentArg =  (long)(unsigned)currentArg;
          }
          ltob(currentArg, s, base);
          p = s;
        }
        break;

      default:
        handle_char('?');
      case '%':

        handle_char(*fmt);
        continue;

    }
    {
      size_t i = 0;
      while(p[i]) i++;
      size -= i;
    }

    if (flags & RIGHT)
    {
      int ch = ' ';
      if (flags & ZEROSFILL) ch = '0';
      for (; size > 0; size--)
        handle_char(ch);
    }
    for (; *p != '\0'; p++)
      handle_char(*p);

    for (; size > 0; size--)
      handle_char(' ');
  }
  va_end(arg);
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
#include <stdio.h>
#include <string.h>

void cso(char c)
{
  putchar(c);
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
  "1   ", "%-04x", 1, 0},
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
  "1   ", "%-04lx", 1, 0},
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
"ptr 1234:5678", "ptr %p", 0x5678, 0x1234}, {0}};

void test(char *should, char *format, unsigned lowint, unsigned highint)
{
  char b[100];

  sprintf(b, format, lowint, highint);

  printf("'%s' = '%s'\n", should, b);

  if (strcmp(b, should))
  {
    printf("\nhit ENTER\n");
    getchar();
  }
}

int main(void)
{
  int i;
  printf("hello world\n");

  for (i = 0; testarray[i].should; i++)
  {
    test(testarray[i].should, testarray[i].format, testarray[i].lowint,
         testarray[i].highint);
  }
  return 0;
}
#endif

