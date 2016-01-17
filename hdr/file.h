/****************************************************************/
/*                                                              */
/*                            file.h                            */
/*                                                              */
/*                      DOS File mode flags                     */
/*                                                              */
/*                       December 1, 1991                       */
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

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *file_hRcsId =
    "$Id: file.h 831 2004-03-27 00:27:11Z bartoldeman $";
#endif
#endif

/* 0 = CON, standard input, can be redirected                           */
/* 1 = CON, standard output, can be redirected                          */
/* 2 = CON, standard error                                              */
/* 3 = AUX, auxiliary                                                   */
/* 4 = PRN, list device                                                 */
/* 5 = 1st user file ...                                                */
#define STDIN           0
#define STDOUT          1
#define STDERR          2
#define STDAUX          3
#define STDPRN          4

/* mode bits                                                    */
#define O_VALIDMASK     0xfff3  /* valid open mask              */

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_ACCMODE       0x0003

/* bits 2, 3 reserved */

/* bits 4, 5, 6 sharing modes */
/* see PC Mag Nov 15, 1988 "Networked Database" p234 by Frank J Derfler Jr */
#define O_SHAREMASK     0x0070  /* mask to isolate shared bits  */

#define O_COMPAT        0x0000  /* default, compatibility mode  */
#define O_DENYALL       0x0010  /* sharing bits                 */
#define O_DENYWRITE     0x0020  /*     "      "                 */
#define O_DENYREAD      0x0030  /*     "      "                 */
#define O_DENYNONE      0x0040  /*     "      "                 */
#define O_NETFCB        0x0070  /* networked fcb                */

#define O_NOINHERIT     0x0080
#define O_OPEN          0x0100 /* not     */
#define O_TRUNC         0x0200 /*    both */
#define O_CREAT         0x0400
#define O_LEGACY        0x0800
#define O_LARGEFILE     0x1000
#define O_NOCRIT        0x2000
#define O_SYNC          0x4000
#define O_FCB           0x8000

/* status for extended open */
enum {S_OPENED = 1, S_CREATED = 2, S_REPLACED = 3};

