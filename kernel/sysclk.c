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
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
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

#ifdef PROTO
BOOL ReadPCClock(ULONG *);
VOID WriteATClock(BYTE *, BYTE, BYTE, BYTE);
VOID WritePCClock(ULONG);
COUNT BcdToByte(COUNT);
COUNT BcdToWord(BYTE *, UWORD *, UWORD *, UWORD *);
COUNT ByteToBcd(COUNT);
VOID DayToBcd(BYTE *, UWORD *, UWORD *, UWORD *);
#else
BOOL ReadPCClock();
VOID WriteATClock();
VOID WritePCClock();
COUNT BcdToByte();
COUNT BcdToWord();
COUNT ByteToBcd();
VOID DayToBcd();
#endif

/*                                                                      */
/* WARNING - THIS DRIVER IS NON-PORTABLE!!!!                            */
/*                                                                      */

WORD days[2][13] =
{
  {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
  {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
};

static struct ClockRecord clk;

static BYTE bcdDays[4];
static UWORD Month,
  Day,
  Year;
static BYTE bcdMinutes;
static BYTE bcdHours;
static BYTE bcdHundredths;
static BYTE bcdSeconds;

static ULONG Ticks;
UWORD DaysSinceEpoch = 0;

WORD clk_driver(rqptr rp)
{
  REG COUNT count,
    c;
  BYTE FAR *cp;

  switch (rp->r_command)
  {
    case C_INIT:
      rp->r_endaddr = device_end();
      rp->r_nunits = 0;
      return S_DONE;

    case C_INPUT:
      count = rp->r_count;
      if (count > sizeof(struct ClockRecord))
          count = sizeof(struct ClockRecord);
      {
        ULONG remainder,
          hs;
        if (ReadPCClock(&Ticks))
          ++DaysSinceEpoch;
        clk.clkDays = DaysSinceEpoch;
        /*
         * Another tricky calculation (after the one in `main.c'). This time
         * we do have a problem with overflow, because we need to extract the
         * 1/100s portion too. The scaling factor is now
         * (100 x 86400) / 0x1800b0 = 108000 / 19663. -- ror4
         */
        hs = 0;
        if (Ticks >= 64 * 19663ul)
        {
          hs += 64 * 108000ul;
          Ticks -= 64 * 19663ul;
        }
        if (Ticks >= 32 * 19663ul)
        {
          hs += 32 * 108000ul;
          Ticks -= 32 * 19663ul;
        }
        if (Ticks >= 16 * 19663ul)
        {
          hs += 16 * 108000ul;
          Ticks -= 16 * 19663ul;
        }
        if (Ticks >= 8 * 19663ul)
        {
          hs += 8 * 108000ul;
          Ticks -= 8 * 19663ul;
        }
        if (Ticks >= 4 * 19663ul)
        {
          hs += 4 * 108000ul;
          Ticks -= 4 * 19663ul;
        }
        if (Ticks >= 2 * 19663ul)
        {
          hs += 2 * 108000ul;
          Ticks -= 2 * 19663ul;
        }
        if (Ticks >= 19663ul)
        {
          hs += 108000ul;
          Ticks -= 19663ul;
        }
        /*
         * Now Ticks < 19663, so Ticks * 108000 < 2123604000 < ULONG_MAX.
         * *phew* -- ror4
         */
        hs += Ticks * 108000ul / 19663ul;
        clk.clkHours = hs / 360000ul;
        remainder = hs % 360000ul;
        clk.clkMinutes = remainder / 6000ul;
        remainder %= 6000ul;
        clk.clkSeconds = remainder / 100ul;
        clk.clkHundredths = remainder % 100ul;
      }
      fbcopy((BYTE FAR *) & clk, rp->r_trans, count);
      return S_DONE;

    case C_OUTPUT:
      count = rp->r_count;
      if (count > sizeof(struct ClockRecord))
          count = sizeof(struct ClockRecord);
      rp->r_count = count;
      fbcopy(rp->r_trans, (BYTE FAR *) & clk, count);

      /* Set PC Clock first                                   */
      DaysSinceEpoch = clk.clkDays;
      {
        ULONG hs;
        hs = 360000ul * clk.clkHours +
            6000ul * clk.clkMinutes +
            100ul * clk.clkSeconds +
            clk.clkHundredths;
        Ticks = 0;
        if (hs >= 64 * 108000ul)
        {
          Ticks += 64 * 19663ul;
          hs -= 64 * 108000ul;
        }
        if (hs >= 32 * 108000ul)
        {
          Ticks += 32 * 19663ul;
          hs -= 32 * 108000ul;
        }
        if (hs >= 16 * 108000ul)
        {
          Ticks += 16 * 19663ul;
          hs -= 16 * 108000ul;
        }
        if (hs >= 8 * 108000ul)
        {
          Ticks += 8 * 19663ul;
          hs -= 8 * 108000ul;
        }
        if (hs >= 4 * 108000ul)
        {
          Ticks += 4 * 19663ul;
          hs -= 4 * 108000ul;
        }
        if (hs >= 2 * 108000ul)
        {
          Ticks += 2 * 19663ul;
          hs -= 2 * 108000ul;
        }
        if (hs >= 108000ul)
        {
          Ticks += 19663ul;
          hs -= 108000ul;
        }
        Ticks += hs * 19663ul / 108000ul;
      }
      WritePCClock(Ticks);

      /* Now set AT clock                                     */
      /* Fix year by looping through each year, subtracting   */
      /* the appropriate number of days for that year.        */
      for (Year = 1980, c = clk.clkDays; c > 0;)
      {
        count = is_leap_year(Year) ? 366 : 365;
        if (c >= count)
        {
          ++Year;
          c -= count;
        }
        else
          break;
      }

      /* c contains the days left and count the number of     */
      /* days for that year.  Use this to index the table.    */
      for (Month = 1; Month < 13; ++Month)
      {
        if (days[count == 366][Month] > c)
        {
          Day = c - days[count == 366][Month - 1] + 1;
          break;
        }
      }
      DayToBcd((BYTE *) bcdDays, &Month, &Day, &Year);
      bcdMinutes = ByteToBcd(clk.clkMinutes);
      bcdHours = ByteToBcd(clk.clkHours);
      bcdSeconds = ByteToBcd(clk.clkSeconds);
      WriteATClock(bcdDays, bcdHours, bcdMinutes, bcdSeconds);
      return S_DONE;

    case C_OFLUSH:
    case C_IFLUSH:
      return S_DONE;

    case C_OUB:
    case C_NDREAD:
    case C_OSTAT:
    case C_ISTAT:
    default:
      return failure(E_FAILURE);	/* general failure */
  }
}

COUNT ByteToBcd(COUNT x)
{
  return ((x / 10) << 4) | (x % 10);
}

VOID DayToBcd(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr)
{
  x[1] = ByteToBcd(*mon);
  x[0] = ByteToBcd(*day);
  x[3] = ByteToBcd(*yr / 100);
  x[2] = ByteToBcd(*yr % 100);
}

/* Used by `main.c'. */
VOID FAR init_call_WritePCClock(ULONG ticks)
{
  WritePCClock(ticks);
}
