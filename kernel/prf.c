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

/*#define DOSEMU*/

#ifdef _INIT
#define fstrlen reloc_call_fstrlen
#define put_console init_put_console
#define ltob init_ltob
#define do_printf init_do_printf
#define printf init_printf
#define sprintf init_sprintf
#define charp init_charp
#define hexd init_hexd
#endif

COUNT ASMCFUNC fstrlen (BYTE FAR * s);        /* don't want globals.h, sorry */


#ifdef VERSION_STRINGS
static BYTE *prfRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.13  2001/11/04 19:47:39  bartoldeman
 * kernel 2025a changes: see history.txt
 *
 * Revision 1.12  2001/09/23 20:39:44  bartoldeman
 * FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
 *
 * Revision 1.11  2001/07/22 01:58:58  bartoldeman
 * Support for Brian's FORMAT, DJGPP libc compilation, cleanups, MSCDEX
 *
 * Revision 1.10  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.9  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.8  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.7  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.6  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.5  2001/03/21 02:56:26  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.4  2001/03/07 10:00:00 tomehlert
 * recoded for smaller object footprint, added main() for testing+QA
 *
 * $Log$
 * Revision 1.13  2001/11/04 19:47:39  bartoldeman
 * kernel 2025a changes: see history.txt
 *
 * Revision 1.12  2001/09/23 20:39:44  bartoldeman
 * FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
 *
 * Revision 1.11  2001/07/22 01:58:58  bartoldeman
 * Support for Brian's FORMAT, DJGPP libc compilation, cleanups, MSCDEX
 *
 * Revision 1.10  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.9  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.8  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.7  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.6  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.5  2001/03/21 02:56:26  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
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

static BYTE *charp = 0;

#ifdef PROTO
VOID handle_char(COUNT);
VOID put_console(COUNT);
BYTE *ltob(LONG, BYTE *, COUNT);
COUNT do_printf(CONST BYTE *, REG BYTE **);
#else
VOID handle_char();
VOID put_console();
BYTE *ltob();
COUNT do_printf();
#endif

/* The following is user supplied and must match the following prototype */
#ifdef PROTO
VOID cso(COUNT);
#else
VOID cso();
#endif

#ifdef FORSYS
COUNT fstrlen (BYTE FAR * s)        /* don't want globals.h, sorry */
{
    int i = 0;

    while (*s++)
       i++;

    return i;
}
#endif

/* special console output routine */
VOID
put_console(COUNT c)
{
  if (c == '\n')
    put_console('\r');

#ifdef FORSYS
  write(1,&c,1);      /* write character to stdout */
#else  
#if defined(__TURBOC__)   	
  _AX = 0x0e00 | c;
  _BX = 0x0070;
  __int__(0x10);
#else
	__asm {
		mov al, byte ptr c;
		mov ah, 0x0e;
		mov bx, 0x0070;
		int 0x10;
	}  
#endif /* __TURBO__*/
#endif  
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
BYTE *
  ltob(LONG n, BYTE * s, COUNT base)
{
  ULONG u;
  BYTE *p, *q;
  int  c;

    u = n;

  if (base == -10)              /* signals signed conversion */
  	{
    base = 10;
	if (n < 0 )
  		{
    	u = -n;
    	*s++ = '-';
  		}
  	}
  	
  p = q = s;
  do
  {                             /* generate digits in reverse order */
  	static char hexDigits[] = "0123456789abcdef";
  
    *p++ = hexDigits[(UWORD)(u % base)];
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
#ifdef DOSEMU
WORD printf(CONST BYTE * fmt, ...)
{
  WORD ret;

  static char buff[80]; /* adjust if necessary */	
  charp = buff;
  ret = do_printf(fmt, (BYTE **)&fmt + 1);
  handle_char(NULL);
  _ES = FP_SEG(buff);
  _DX = FP_OFF(buff);
  _AX = 0x13;
  __int__(0xe6);
  return ret;
}
#else
WORD printf(CONST BYTE * fmt, ...)
{
  charp = 0;
  return do_printf(fmt, (BYTE **)&fmt + 1);
}
#endif

WORD
sprintf(BYTE * buff, CONST BYTE * fmt, ...)
{
  WORD ret;

  charp = buff;
  ret = do_printf(fmt, (BYTE **)&fmt + 1);
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
COUNT
  do_printf(CONST BYTE * fmt, BYTE ** arg)
{
  int base;
  BYTE s[11],
      FAR *p;
  int c,
    flag,
    size,
    fill;
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

    if ( *fmt == '-')
    {
      flag = LEFT;
      fmt++;
    }
    
    if ( *fmt == '0')
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
	        handle_char(*(COUNT *) arg++);
    	    continue;

      case 'p':
            {
            WORD w[2];
            static char pointerFormat[] = "%04x:%04x";
            w[1] = *((unsigned int*) arg)++;
            w[0] = *((unsigned int*) arg)++;
            do_printf(pointerFormat,(BYTE**)&w);
    	    continue;
    	    }

      case 's':
            p = *((BYTE **) arg)++;
	    goto do_outputstring;
            
      case 'F':
            fmt++;
            /* we assume %Fs here */
      case 'S':
            p = *((BYTE FAR **) arg)++;
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
          		currentArg = *((LONG *) arg)++;
          	else
          		if (base < 0) currentArg = *((int*) arg)++;	
          		else          currentArg = *((unsigned int*) arg)++;	
          			
          
            ltob(currentArg, s, base);

            p = s;
do_outputstring:
            
            size  -= fstrlen(p);
            
            if (flag == RIGHT )
            {
              for ( ; size > 0; size--)
                handle_char(fill);
            }
            for (; *p != '\0'; p++)
              handle_char(*p);

            for ( ; size > 0; size--)
                handle_char(fill);
                
            continue;

          default:
            handle_char('?');
          
            handle_char(c);
            break;


    }
  }
  return 0;
}

void hexd(char *title,UBYTE FAR *p,COUNT numBytes)
{
    int loop;
    printf("%s%04x|", title, FP_SEG(p));
    for (loop = 0; loop < numBytes; loop++)
        printf("%02x ", p[loop]);
    printf("|");
    
    for (loop = 0; loop < numBytes; loop++)
        printf("%c", p[loop] < 0x20 ? '.' : p[loop]);
    printf("\n");    
}    



#ifdef TEST
/*
	this testprogram verifies that the strings are printed correctly
	( or the way, I expect them to print)
	
	compile like (note -DTEST !)

	c:\tc\tcc -DTEST -DI86 -I..\hdr prf.c
	
	and run. if strings are wrong, the program will wait for the ANYKEY

*/
#include <c:\tc\include\conio.h>
void cso(char c) { putch(c); }


struct  {
	char *should;
	char *format;
	unsigned lowint;
	unsigned highint;
	
} testarray[] = 
	{ 
		{ "hello world", "%s %s", (unsigned)"hello",(unsigned)"world"},
		{ "hello", "%3s", (unsigned)"hello",0},
		{ "  hello", "%7s", (unsigned)"hello",0},
		{ "hello  ", "%-7s", (unsigned)"hello",0},
		{ "hello", "%s", (unsigned)"hello",0},
		
		
		
		{ "1",	"%d", 1, 0},
		{ "-1", "%d", -1,0},
		{ "65535", "%u", -1,0},
		{ "-32768", "%d", 0x8000,0},
		{ "32767", "%d", 0x7fff,0},
		{ "-32767", "%d", 0x8001,0},
		
		{"8000", "%x", 0x8000, 0},
		{"   1", "%4x", 1, 0},
		{"0001", "%04x", 1, 0},
		{"1   ", "%-4x", 1, 0},
		{"1000", "%-04x", 1, 0},

		{ "1",	"%ld", 1, 0},
		{ "-1", "%ld", -1,-1},
		{ "65535", "%ld", -1,0},
		{ "65535", "%u", -1,0},
		{"8000", "%lx", 0x8000, 0},
		{"80000000", "%lx", 0,0x8000},
		{"   1", "%4lx", 1, 0},
		{"0001", "%04lx", 1, 0},
		{"1   ", "%-4lx", 1, 0},
		{"1000", "%-04lx", 1, 0},
		
		{ "-2147483648", "%ld", 0,0x8000},
		{  "2147483648", "%lu", 0,0x8000},
		{  "2147483649", "%lu", 1,0x8000},
		{ "-2147483647", "%ld", 1,0x8000},
		{ "32767", "%ld", 0x7fff,0},

		{ "ptr 1234:5678", "ptr %p", 0x5678,0x1234},
		
		
		
		0
	};
	
test(char *should, char *format, unsigned lowint, unsigned highint)
{
	char b[100];
	
	sprintf(b, format, lowint,highint);
	
	printf("'%s' = '%s'\n", should, b);
	
	if (strcmp(b,should)) { printf("\nhit the ANYKEY\n"); getch(); }
}	


main()
{
	int i;
	printf("hello world\n");
	
	for (i = 0; testarray[i].should; i++)
	{
		test(testarray[i].should,testarray[i].format, testarray[i].lowint, testarray[i].highint);
	}
}
#endif
