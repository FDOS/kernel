/****************************************************************/
/*                                                              */
/*                          syspack.c                           */
/*                                                              */
/*            System Disk Byte Order Packing Functions          */
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
/*                                                              */
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *syspackRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.5  2001/11/04 19:47:39  bartoldeman
 * kernel 2025a changes: see history.txt
 *
 * Revision 1.4  2001/09/23 20:39:44  bartoldeman
 * FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
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
 * Revision 1.1.1.1  1999/03/29 15:42:21  jprice
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
 *    Rev 1.3   29 May 1996 21:15:12   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:42   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:05:34   patv
 * Initial revision.
 */

#ifdef NONNATIVE
VOID getlong(REG VOID * vp, LONG * lp)
{
  *lp = (((BYTE *) vp)[0] & 0xff) +
      ((((BYTE *) vp)[1] & 0xff) << 8) +
      ((((BYTE *) vp)[2] & 0xff) << 16) +
      ((((BYTE *) vp)[3] & 0xff) << 24);
}

VOID getword(REG VOID * vp, WORD * wp)
{
  *wp = (((BYTE *) vp)[0] & 0xff) + ((((BYTE *) vp)[1] & 0xff) << 8);
}

VOID getbyte(VOID * vp, BYTE * bp)
{
  *bp = *((BYTE *) vp);
}

VOID fgetword(REG VOID FAR * vp, WORD FAR * wp)
{
  *wp = (((BYTE FAR *) vp)[0] & 0xff) + ((((BYTE FAR *) vp)[1] & 0xff) << 8);
}

VOID fgetlong(REG VOID FAR * vp, LONG FAR * lp)
{
  *lp = (((BYTE *) vp)[0] & 0xff) +
      ((((BYTE *) vp)[1] & 0xff) << 8) +
      ((((BYTE *) vp)[2] & 0xff) << 16) +
      ((((BYTE *) vp)[3] & 0xff) << 24);
}

VOID fgetbyte(VOID FAR * vp, BYTE FAR * bp)
{
  *bp = *((BYTE FAR *) vp);
}

VOID fputlong(LONG FAR * lp, VOID FAR * vp)
{
  REG BYTE FAR *bp = (BYTE FAR *) vp;

  bp[0] = *lp & 0xff;
  bp[1] = (*lp >> 8) & 0xff;
  bp[2] = (*lp >> 16) & 0xff;
  bp[3] = (*lp >> 24) & 0xff;
}

VOID fputword(WORD FAR * wp, VOID FAR * vp)
{
  REG BYTE FAR *bp = (BYTE FAR *) vp;

  bp[0] = *wp & 0xff;
  bp[1] = (*wp >> 8) & 0xff;
}

VOID fputbyte(BYTE FAR * bp, VOID FAR * vp)
{
  *(BYTE FAR *) vp = *bp;
}

VOID getdirent(BYTE FAR * vp, struct dirent FAR * dp)
{
  fbcopy(&vp[DIR_NAME], dp->dir_name, FNAME_SIZE);
  fbcopy(&vp[DIR_EXT], dp->dir_ext, FEXT_SIZE);
  fgetbyte(&vp[DIR_ATTRIB], (BYTE FAR *) & dp->dir_attrib);
  fgetword(&vp[DIR_TIME], (WORD FAR *) & dp->dir_time);
  fgetword(&vp[DIR_DATE], (WORD FAR *) & dp->dir_date);
  fgetword(&vp[DIR_START], (WORD FAR *) & dp->dir_start);
  fgetlong(&vp[DIR_SIZE], (LONG FAR *) & dp->dir_size);
}

VOID putdirent(struct dirent FAR * dp, BYTE FAR * vp)
{
  REG COUNT i;
  REG BYTE FAR *p;

  fbcopy(dp->dir_name, &vp[DIR_NAME], FNAME_SIZE);
  fbcopy(dp->dir_ext, &vp[DIR_EXT], FEXT_SIZE);
  fputbyte((BYTE FAR *) & dp->dir_attrib, &vp[DIR_ATTRIB]);
  fputword((WORD FAR *) & dp->dir_time, &vp[DIR_TIME]);
  fputword((WORD FAR *) & dp->dir_date, &vp[DIR_DATE]);
  fputword((WORD FAR *) & dp->dir_start, &vp[DIR_START]);
  fputlong((LONG FAR *) & dp->dir_size, &vp[DIR_SIZE]);
  for (i = 0, p = (BYTE FAR *) & vp[DIR_RESERVED]; i < 10; i++)
    *p++ = NULL;
}
#endif
