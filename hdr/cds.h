/****************************************************************/
/*                                                              */
/*                            cds.h                             */
/*                                                              */
/*                  Current Directory structures                */
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
static BYTE *Cds_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.5  2001/11/04 19:47:39  bartoldeman
 * kernel 2025a changes: see history.txt
 *
 * Revision 1.4  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
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
 * Revision 1.2  2000/03/09 06:06:38  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.1.1.1  1999/03/29 15:39:22  jprice
 * New version without IPL.SYS
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

 Rev 1.1   04 Jan 1998 23:14:18   patv
 Changed Log for strip utility

 Rev 1.0   19 Feb 1996  3:15:52   patv
 Added NLS, int2f and config.sys processing
 */

#define MAX_CDSPATH 67

struct cds
{
  BYTE cdsCurrentPath[MAX_CDSPATH];
  UWORD cdsFlags;
  struct dpb FAR *cdsDpb;

  union
  {
    BYTE FAR *
      _cdsRedirRec;
    struct
    {
      UWORD _cdsStrtClst;
      UWORD _cdsParam;
    }
    _cdsRedir;
  }
  _cdsUnion;

  UWORD cdsStoreUData;

  WORD cdsJoinOffset;

  BYTE cdsNetFlag1;
  BYTE FAR *cdsIfs;
  UWORD cdsNetFlags2;

};

#define cdsStrtClst _cdsUnion._cdsRedir._cdsStrtClst
#define cdsRedirRec _cdsUnion._cdsRedirRec
#define cdsParam _cdsUnion._cdsRedir._cdsParam

typedef struct _cdstbl
{
  struct cds cds_table[26];
}
cdstbl;

/* Bits for cdsFlags                                            */
#define CDSNETWDRV      0x8000
#define CDSPHYSDRV      0x4000
#define CDSJOINED       0x2000
#define CDSSUBST        0x1000
#define CDSVALID        (CDSNETWDRV | CDSPHYSDRV)

