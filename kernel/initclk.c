/****************************************************************/
/*                                                              */
/*                           initclk.c                           */
/*                                                              */
/*                     System Clock Driver - initialization     */
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
#include "init-mod.h"
#include "init-dat.h"

#ifdef VERSION_STRINGS
static char *RcsId =
    "$Id$";
#endif

/*                                                                      */
/* WARNING - THIS DRIVER IS NON-PORTABLE!!!!                            */
/*                                                                      */

STATIC int InitByteToBcd(int x)
{
  return ((x / 10) << 4) | (x % 10);
}

STATIC int InitBcdToByte(int x)
{
  return ((x >> 4) & 0xf) * 10 + (x & 0xf);
}

STATIC void InitDayToBcd(BYTE * x, unsigned mon, unsigned day, unsigned yr)
{
  x[1] = InitByteToBcd(mon);
  x[0] = InitByteToBcd(day);
  x[3] = InitByteToBcd(yr / 100);
  x[2] = InitByteToBcd(yr % 100);
}

void Init_clk_driver(void)
{
  iregs regsT, regsD, dosregs;

  regsT.a.b.h = 0x02;
  regsT.d.x = regsT.c.x = 0;
  regsT.flags = 0;
  init_call_intr(0x1a, &regsT);

  if ((regsT.flags & 0x0001) || (regsT.c.x == 0 && regsT.d.x == 0))
  {
    goto error;                 /* error */
  }

  regsD.a.b.h = 0x04;
  regsD.d.x = regsD.c.x = 0;
  regsD.flags = 0;
  init_call_intr(0x1a, &regsD);

  if ((regsD.flags & 0x0001) || regsD.c.x == 0 || regsD.d.x == 0)
  {
    goto error;
  }

  /* DosSetDate */
  dosregs.a.b.h = 0x2b;
  dosregs.c.x = 100 * InitBcdToByte(regsD.c.b.h) +
      InitBcdToByte(regsD.c.b.l);
  dosregs.d.b.h = InitBcdToByte(regsD.d.b.h);   /* month */
  dosregs.d.b.l = InitBcdToByte(regsD.d.b.l);   /* day   */
  init_call_intr(0x21, &dosregs);

  /* DosSetTime */
  dosregs.a.b.h = 0x2d;
  dosregs.c.b.l = InitBcdToByte(regsT.c.b.l);   /* minutes */
  dosregs.c.b.h = InitBcdToByte(regsT.c.b.h);   /* hours   */
  dosregs.d.b.h = InitBcdToByte(regsT.d.b.h);   /*seconds */
  dosregs.d.b.l = 0;
  init_call_intr(0x21, &dosregs);

error:;

}
