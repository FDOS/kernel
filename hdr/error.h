/****************************************************************/
/*                                                              */
/*                          error.h                             */
/*                                                              */
/*                    DOS-C error return codes                  */
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
static BYTE *error_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.4  2000/11/02 06:56:53  jimtabor
 * Fix Share Patch
 *
 * Revision 1.3  2000/05/25 20:56:19  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:28:22  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.1.1.1  1999/03/29 15:39:27  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/08 05:58:24  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/01 01:40:06  jprice
 * Clean up
 *
 * Revision 1.3  1999/01/30 08:21:43  jprice
 * Clean up
 *
 * Revision 1.2  1999/01/22 04:17:40  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *         Rev 1.6   06 Dec 1998  8:41:00   patv
 *      Added new errors for new I/O subsystem.
 *
 *         Rev 1.5   04 Jan 1998 23:14:16   patv
 *      Changed Log for strip utility
 *
 *         Rev 1.4   29 May 1996 21:25:18   patv
 *      bug fixes for v0.91a
 *
 *         Rev 1.3   19 Feb 1996  3:15:28   patv
 *      Added NLS, int2f and config.sys processing
 *
 *         Rev 1.2   01 Sep 1995 17:35:38   patv
 *      First GPL release.
 *
 *         Rev 1.1   30 Jul 1995 20:42:28   patv
 *      fixed ipl
 *
 *         Rev 1.0   02 Jul 1995 10:39:36   patv
 *      Initial revision.
 */

/* Internal system error returns                                        */
#define SUCCESS         0       /* Function was successful      */
#define DE_INVLDFUNC    -1      /* Invalid function number      */
#define DE_FILENOTFND   -2      /* File not found               */
#define DE_PATHNOTFND   -3      /* Path not found               */
#define DE_TOOMANY      -4      /* Too many open files          */
#define DE_ACCESS       -5      /* Access denied                */
#define DE_INVLDHNDL    -6      /* Invalid handle               */
#define DE_MCBDESTRY    -7      /* Memory control blocks shot   */
#define DE_NOMEM        -8      /* Insufficient memory          */
#define DE_INVLDMCB     -9      /* Invalid memory control block */
#define DE_INVLDENV     -10     /* Invalid enviornement         */
#define DE_INVLDFMT     -11     /* Invalid format               */
#define DE_INVLDACC     -12     /* Invalid access               */
#define DE_INVLDDATA    -13     /* Inavalid data                */
#define DE_INVLDDRV     -15     /* Invalid drive                */
#define DE_RMVCUDIR     -16     /* Attempt remove current dir   */
#define DE_DEVICE       -17     /* Not same device              */
#define DE_NFILES       -18     /* No more files                */
#define DE_WRTPRTCT     -19     /* No more files                */
#define DE_BLKINVLD     -20     /* invalid block                */
#define DE_SEEK         -25     /* error on file seek           */
#define DE_HNDLDSKFULL  -28     /* handle disk full (?)         */

#define DE_DEADLOCK	-36
#define DE_LOCK		-39

/* Critical error flags                                                 */
#define EFLG_READ       0x00    /* Read error                   */
#define EFLG_WRITE      0x01    /* Write error                  */
#define EFLG_RSVRD      0x00    /* Error in rserved area        */
#define EFLG_FAT        0x02    /* Error in FAT area            */
#define EFLG_DIR        0x04    /* Error in dir area            */
#define EFLG_DATA       0x06    /* Error in data area           */
#define EFLG_ABORT      0x08    /* Handler can abort            */
#define EFLG_RETRY      0x10    /* Handler can retry            */
#define EFLG_IGNORE     0x20    /* Handler can ignore           */
#define EFLG_CHAR       0x80    /* Error in char or FAT image   */

/* error results returned after asking user                             */
/* MS-DOS compatible -- returned by CriticalError                       */
#define CONTINUE        0
#define RETRY           1
#define ABORT           2
#define FAIL            3
