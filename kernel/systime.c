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
    "$Id: systime.c 683 2003-09-09 17:43:43Z bartoldeman $";
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
  BinaryCharIO(&clock, sizeof(struct ClockRecord), &ClkRecord, command);
}

void DosGetTime(struct dostime *dt)
{
  ExecuteClockDriverRequest(C_INPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return;

  dt->hour = ClkRecord.clkHours;
  dt->minute = ClkRecord.clkMinutes;
  dt->second = ClkRecord.clkSeconds;
  dt->hundredth = ClkRecord.clkHundredths;
}

int DosSetTime(const struct dostime *dt)
{
  if (dt->hour > 23 || dt->minute > 59 ||
      dt->second > 59 || dt->hundredth > 99)
     return DE_INVLDDATA;
 
  /* for ClkRecord.clkDays */
  ExecuteClockDriverRequest(C_INPUT);

  ClkRecord.clkHours = dt->hour;
  ClkRecord.clkMinutes = dt->minute;
  ClkRecord.clkSeconds = dt->second;
  ClkRecord.clkHundredths = dt->hundredth;

  ExecuteClockDriverRequest(C_OUTPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return char_error(&ClkReqHdr, (struct dhdr FAR *)clock);
  return SUCCESS;
}

unsigned char DosGetDate(struct dosdate *dd)
{
  UWORD c;
  const UWORD *pdays;
  UWORD Year, Month;

  ExecuteClockDriverRequest(C_INPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return 0;

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

  dd->year = Year;
  dd->month = Month;
  dd->monthday = c - pdays[Month - 1] + 1;

  /* Day of week is simple. Take mod 7, add 2 (for Tuesday        */
  /* 1-1-80) and take mod again                                   */

  return (ClkRecord.clkDays + 2) % 7;
}

int DosSetDate(const struct dosdate *dd)
{
  UWORD Year = dd->year;
  UWORD Month = dd->month;
  UWORD DayOfMonth = dd->monthday;
  const UWORD *pdays = is_leap_year_monthdays(Year);

  if (Year < 1980 || Year > 2099
      || Month < 1 || Month > 12
      || DayOfMonth < 1
      || DayOfMonth > pdays[Month] - pdays[Month - 1])
    return DE_INVLDDATA;

  ExecuteClockDriverRequest(C_INPUT);

  ClkRecord.clkDays = DaysFromYearMonthDay(Year, Month, DayOfMonth);

  ExecuteClockDriverRequest(C_OUTPUT);

  if (ClkReqHdr.r_status & S_ERROR)
    return char_error(&ClkReqHdr, (struct dhdr FAR *)clock);
  return SUCCESS;
}

