/****************************************************************/
/*                                                              */
/*                            clock.h                           */
/*                                                              */
/*           Clock Driver data structures & declarations        */
/*                                                              */
/*                      November 26, 1991                       */
/*                                                              */
/*                 Adapted to DOS/NT June 12, 1993              */
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
static BYTE *clock_hRcsId =
    "$Id$";
#endif
#endif

struct ClockRecord {
  UWORD clkDays;                /* days since Jan 1, 1980.              */
  UBYTE clkMinutes;             /* residual minutes.                    */
  UBYTE clkHours;               /* residual hours.                      */
  UBYTE clkHundredths;          /* residual hundredths of a second.     */
  UBYTE clkSeconds;             /* residual seconds.                    */
};

