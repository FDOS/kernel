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
    "$Id$";
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

STATIC int BcdToByte(int x)
{
  return ((x >> 4) & 0xf) * 10 + (x & 0xf);
}

STATIC void DayToBcd(BYTE * x, unsigned mon, unsigned day, unsigned yr)
{
  x[1] = ByteToBcd(mon);
  x[0] = ByteToBcd(day);
  x[3] = ByteToBcd(yr / 100);
  x[2] = ByteToBcd(yr % 100);
}

WORD ASMCFUNC FAR clk_driver(rqptr rp)
{
  BYTE bcd_days[4], bcd_minutes, bcd_hours, bcd_seconds;

  switch (rp->r_command)
  {
    case C_INIT:
      /* If AT clock exists, copy AT clock time to system clock */
      if (!ReadATClock(bcd_days, &bcd_hours, &bcd_minutes, &bcd_seconds))
      {
        ticks_t seconds;
        
        DaysSinceEpoch =
            DaysFromYearMonthDay(100 * BcdToByte(bcd_days[3]) +
                                 BcdToByte(bcd_days[2]),
                                 BcdToByte(bcd_days[1]),
                                 BcdToByte(bcd_days[0]));

        /*
         * This is a rather tricky calculation. The number of timer ticks per
         * second is not exactly 18.2, but rather 1193180 / 65536
         * where 1193180 = 0x1234dc
         * The timer interrupt updates the midnight flag when the tick count
         * reaches 0x1800b0 -- ror4.
         */

        seconds =
          60 * (ticks_t)(60 * BcdToByte(bcd_hours) + BcdToByte(bcd_minutes)) +
          BcdToByte(bcd_seconds);
        WritePCClock(seconds * 0x12 + ((seconds * 0x34dc) >> 16));
      }
      /* rp->r_endaddr = device_end(); not needed - bart */
      rp->r_nunits = 0;
      return S_DONE;

    case C_INPUT:
      {
        struct ClockRecord clk;
        int tmp;
        ticks_t ticks;
        
        clk.clkDays = DaysSinceEpoch;
        
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
                
        fmemcpy(rp->r_trans, &clk,
                min(sizeof(struct ClockRecord), rp->r_count));
      }        
      return S_DONE;

    case C_OUTPUT:
      {
        int c;
        const unsigned short *pdays;
        unsigned Month, Day, Year;
        struct ClockRecord clk;
        ticks_t hs, Ticks;
        
        rp->r_count = min(rp->r_count, sizeof(struct ClockRecord));
        fmemcpy(&clk, rp->r_trans, rp->r_count);

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
        for (Year = 1980, c = clk.clkDays;;)
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
        
        /* c contains the days left and count the number of     */
        /* days for that year.  Use this to index the table.    */
        for (Month = 1; Month < 13; ++Month)
        {
          if (pdays[Month] > c)
          {
            Day = c - pdays[Month - 1] + 1;
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

/*
 * Log: sysclk.c,v - for newer entries do "cvs log sysclk.c"
 *
 * Revision 1.3  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/12 03:21:17  jprice
 * more ror4 patches.  Changes for multi-block IO
 *
 * Revision 1.1.1.1  1999/03/29 15:41:33  jprice
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
 *    Rev 1.4   04 Jan 1998 23:15:16   patv
 * Changed Log for strip utility
 *
 *    Rev 1.3   29 May 1996 21:03:48   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   19 Feb 1996  3:21:34   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.1   01 Sep 1995 17:54:18   patv
 * First GPL release.
 *
 *    Rev 1.0   02 Jul 1995  8:32:30   patv
 * Initial revision.
 */
