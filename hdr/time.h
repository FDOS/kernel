/****************************************************************/
/*                                                              */
/*                           time.h                             */
/*                                                              */
/*                  DOS General Time Structure                  */
/*                                                              */
/*                       January 21, 1993                       */
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

/* TC 2.01 complains if `time' is defined twice. -- ror4 */
#ifndef DOSC_TIME_H
#define DOSC_TIME_H

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *time_hRcsId =
    "$Id: time.h 942 2004-05-23 15:00:37Z bartoldeman $";
#endif
#endif

typedef UWORD time;

struct dostime
{
  unsigned char minute, hour, hundredth, second;
};

struct dosdate
{
  unsigned short year;
  unsigned char monthday, month;
};

#endif

