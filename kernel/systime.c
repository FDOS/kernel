/****************************************************************/
/*                                                              */
/*                           systime.c                          */
/*                                                              */
/*                    DOS/C Date/Time Functions                 */
/*                                                              */
/*                      Copyright (c) 1998                      */
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
#include "time.h"
#include "date.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id$";
#endif

const UWORD days[2][13] = {
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

extern request ASM ClkReqHdr;

/*
    return a pointer to an array with the days for that year
*/

const UWORD *is_leap_year_monthdays(UWORD y)
{
  /* this is correct in a strict mathematical sense   
     return ((y) & 3 ? days[0] : (y) % 100 ? days[1] : (y) % 400 ? days[0] : days[1]); */

  /* this will work until 2200 - long enough for me - and saves 0x1f bytes */

  if ((y & 3) || y == 2100)
    return days[0];

  return days[1];
}

UWORD DaysFromYearMonthDay(UWORD Year, UWORD Month, UWORD DayOfMonth)
{
  if (Year < 1980)
    return 0;

  return DayOfMonth - 1
      + is_leap_year_monthdays(Year)[Month - 1]
      + ((Year - 1980) * 365) + ((Year - 1980 + 3) / 4);

}

/* common - call the clock driver */
void ExecuteClockDriverRequest(BYTE command)
{
  ClkReqHdr.r_length = sizeof(request);
  ClkReqHdr.r_command = command;
  ClkReqHdr.r_count = sizeof(struct ClockRecord);
  ClkReqHdr.r_trans = (BYTE FAR *) (&ClkRecord);
  ClkReqHdr.r_status = 0;
  execrh((request FAR *) & ClkReqHdr, (struct dhdr FAR *)clock);
}

VOID DosGetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp,
                BYTE FAR * hdp)
{
  ExecuteClockDriverRequest(C_INPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return;

  *hp = ClkRecord.clkHours;
  *mp = ClkRecord.clkMinutes;
  *sp = ClkRecord.clkSeconds;
  *hdp = ClkRecord.clkHundredths;
}

COUNT DosSetTime(BYTE h, BYTE m, BYTE s, BYTE hd)
{
  /* for ClkRecord.clkDays */
  ExecuteClockDriverRequest(C_INPUT);

  ClkRecord.clkHours = h;
  ClkRecord.clkMinutes = m;
  ClkRecord.clkSeconds = s;
  ClkRecord.clkHundredths = hd;

  ExecuteClockDriverRequest(C_OUTPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return char_error(&ClkReqHdr, (struct dhdr FAR *)clock);
  return SUCCESS;
}

VOID DosGetDate(wdp, mp, mdp, yp)
BYTE FAR *wdp, FAR * mp, FAR * mdp;
COUNT FAR *yp;
{
  UWORD c;
  const UWORD *pdays;
  UWORD Year, Month;

  ExecuteClockDriverRequest(C_INPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return;

  for (Year = 1980, c = ClkRecord.clkDays;;)
  {
    pdays = is_leap_year_monthdays(Year);
    if (c >= pdays[12])
    {
      ++Year;
      c -= pdays[12];
    }
    else
      break;
  }

  /* c contains the days left and count the number of days for    */
  /* that year.  Use this to index the table.                     */
  Month = 1;
  while (c >= pdays[Month])
  {
    ++Month;
  }

  *mp = Month;
  *mdp = c - pdays[Month - 1] + 1;
  *yp = Year;

  /* Day of week is simple. Take mod 7, add 2 (for Tuesday        */
  /* 1-1-80) and take mod again                                   */

  *wdp = (ClkRecord.clkDays + 2) % 7;
}

COUNT DosSetDate(Month, DayOfMonth, Year)
UWORD Month, DayOfMonth, Year;
{
  const UWORD *pdays;
  pdays = is_leap_year_monthdays(Year);

  if (Year < 1980 || Year > 2099
      || Month < 1 || Month > 12
      || DayOfMonth < 1 || DayOfMonth > pdays[Month] - pdays[Month - 1])
    return DE_INVLDDATA;

  ExecuteClockDriverRequest(C_INPUT);

  ClkRecord.clkDays = DaysFromYearMonthDay(Year, Month, DayOfMonth);

  ExecuteClockDriverRequest(C_OUTPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return char_error(&ClkReqHdr, (struct dhdr FAR *)clock);
  return SUCCESS;
}

/*
 * Log: systime.c,v - see "cvs log systime.c" for newer entries
 *
 * Revision 1.3  1999/05/04 16:40:30  jprice
 * ror4 date fix
 *
 * Revision 1.2  1999/04/12 03:21:18  jprice
 * more ror4 patches.  Changes for multi-block IO
 *
 * Revision 1.1.1.1  1999/03/29 15:41:34  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/08 05:55:58  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/04 03:14:07  jprice
 * Formating.  Added comments.
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:27  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.6   06 Dec 1998  8:47:30   patv
 * Bug fixes.
 *
 *    Rev 1.5   04 Jan 1998 23:15:22   patv
 * Changed Log for strip utility
 *
 *    Rev 1.4   03 Jan 1998  8:36:50   patv
 * Converted data area to SDA format
 *
 *    Rev 1.3   29 May 1996 21:03:40   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   19 Feb 1996  3:21:34   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.1   01 Sep 1995 17:54:16   patv
 * First GPL release.
 *
 *    Rev 1.0   02 Jul 1995  8:32:20   patv
 * Initial revision.
 */
