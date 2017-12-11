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

#ifdef VERSION_STRINGS
static char *RcsId =
    "$Id: initclk.c 1359 2008-03-09 16:11:10Z mceric $";
#endif

/*                                                                      */
/* WARNING - THIS DRIVER IS NON-PORTABLE!!!!                            */
/*                                                                      */

STATIC int InitBcdToByte(int x)
{
  return ((x >> 4) & 0xf) * 10 + (x & 0xf);
}

void Init_clk_driver(void)
{
  static iregs regsT = {0x200}; /* ah=0x02 */
  static iregs regsD = {0x400, 0, 0x1400, 0x101};
                      /* ah=4, ch=20^ ^cl=0, ^dh=dl=1 (2000/1/1)
                       * (above date will be set on error) */
  iregs dosregs;

  init_call_intr(0x1a, &regsT); /* get BIOS time */
  init_call_intr(0x1a, &regsD); /* get BIOS date */

  /* DosSetDate */
  dosregs.a.b.h = 0x2b;
  dosregs.c.x = 100 * InitBcdToByte(regsD.c.b.h) /* century */
                    + InitBcdToByte(regsD.c.b.l);/* year */
  /* A BIOS with y2k (year 2000) bug will always report year 19nn */
  if ((dosregs.c.x >= 1900) && (dosregs.c.x < 1980)) dosregs.c.x += 100;
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
}
