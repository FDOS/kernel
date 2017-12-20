/****************************************************************/
/*                                                              */
/*                           sysclk.c                           */
/*                                                              */
/*                     System Clock Driver                      */
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
#include "globals.h"

#ifdef VERSION_STRINGS
static char *RcsId =
    "$Id: sysclk.c 681 2003-09-09 17:32:20Z bartoldeman $";
#endif

/*                                                                      */
/* WARNING - THIS DRIVER IS NON-PORTABLE!!!!                            */
/*                                                                      */
UWORD ASM DaysSinceEpoch = 0;
typedef UDWORD ticks_t;

STATIC int ByteToBcd(int x)
{
  return ((x / 10) << 4) | (x % 10);
}
/*
STATIC int BcdToByte(int x)
{
  return ((x >> 4) & 0xf) * 10 + (x & 0xf);
}
*/
STATIC void DayToBcd(BYTE * x, unsigned mon, unsigned day, unsigned yr)
{
  x[1] = ByteToBcd(mon);
  x[0] = ByteToBcd(day);
  x[3] = ByteToBcd(yr / 100);
  x[2] = ByteToBcd(yr % 100);
}

WORD ASMCFUNC FAR clk_driver(rqptr rp, ...)
{
  BYTE bcd_days[4], bcd_minutes, bcd_hours, bcd_seconds;

  switch (rp->r_command)
  {
    case C_INIT:

      /* done in initclk.c */
      /* rp->r_endaddr = device_end(); not needed - bart */
      rp->r_nunits = 0;
      return S_DONE;

    case C_INPUT:
      {
        struct ClockRecord clk;
        int tmp;
        ticks_t ticks;

        if (sizeof(struct ClockRecord) != rp->r_count)
          return failure(E_LENGTH);

        /* The scaling factor is now
           6553600/1193180 = 327680/59659 = 65536*5/59659 */

        ticks = 5 * ReadPCClock();
        ticks = ((ticks / 59659u) << 16) + ((ticks % 59659u) << 16) / 59659u;

        tmp = (int)(ticks / 6000);
        clk.clkHours = tmp / 60;
        clk.clkMinutes = tmp % 60;

        tmp = (int)(ticks % 6000);
        clk.clkSeconds = tmp / 100;
        clk.clkHundredths = tmp % 100;

        clk.clkDays = DaysSinceEpoch;

        fmemcpy(rp->r_trans, &clk, sizeof(struct ClockRecord));
      }
      return S_DONE;

    case C_OUTPUT:
      {
        const unsigned short *pdays;
        unsigned Day, Month, Year;
        struct ClockRecord clk;
        ticks_t hs, Ticks;

        if (sizeof(struct ClockRecord) != rp->r_count)
          return failure(E_LENGTH);

        fmemcpy(&clk, rp->r_trans, sizeof(struct ClockRecord));

        /* Set PC Clock first                                   */
        DaysSinceEpoch = clk.clkDays;
        hs = 6000 * (ticks_t)(60 * clk.clkHours + clk.clkMinutes) +
          (ticks_t)(100 * clk.clkSeconds + clk.clkHundredths);

        /* The scaling factor is now
           1193180/6553600 = 59659/327680 = 59659/65536/5 */

        Ticks = ((hs >> 16) * 59659u + (((hs & 0xffff) * 59659u) >> 16)) / 5;

        WritePCClock(Ticks);

        /* Now set AT clock                                     */
        /* Fix year by looping through each year, subtracting   */
        /* the appropriate number of days for that year.        */
        for (Year = 1980, Day = clk.clkDays;;)
        {
          pdays = is_leap_year_monthdays(Year);
          if (Day >= pdays[12])
          {
            ++Year;
            Day -= pdays[12];
          }
          else
            break;
        }

        /* Day contains the days left and count the number of   */
        /* days for that year.  Use this to index the table.    */
        for (Month = 1; Month < 13; ++Month)
        {
          if (pdays[Month] > Day)
          {
            Day = Day - pdays[Month - 1] + 1;
            break;
          }
        }

        DayToBcd(bcd_days, Month, Day, Year);
        bcd_minutes = ByteToBcd(clk.clkMinutes);
        bcd_hours = ByteToBcd(clk.clkHours);
        bcd_seconds = ByteToBcd(clk.clkSeconds);
        WriteATClock(bcd_days, bcd_hours, bcd_minutes, bcd_seconds);
      }
      return S_DONE;

    case C_OFLUSH:
    case C_IFLUSH:
      return S_DONE;

    case C_OUB:
    case C_NDREAD:
    case C_OSTAT:
    case C_ISTAT:
    default:
      return failure(E_FAILURE);        /* general failure */
  }
}

