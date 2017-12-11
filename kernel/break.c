/****************************************************************/
/*                                                              */
/*                            break.c                           */
/*                            FreeDOS                           */
/*                                                              */
/*   Control Break detection and handling                       */
/*                                                              */
/*                   Copyright (c) 1999                         */
/*                         Steffen Kaiser                       */
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
#include "proto.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id: break.c 885 2004-04-14 15:40:51Z bartoldeman $";
#endif

#define CB_FLG *(UBYTE FAR*)MK_FP(0x0, 0x471)
#define CB_MSK 0x80

/* Check for ^Break/^C.
 * Three sources are available:
 *       1) flag at 40:71 bit 7
 *       2) syscon stream (usually CON:)
 *       3) i/o stream (if unequal to syscon, e.g. AUX)
 */

unsigned char ctrl_break_pressed(void)
{
  return CB_FLG & CB_MSK;
}

unsigned char check_handle_break(struct dhdr FAR **pdev)
{
  unsigned char c = CTL_C;
  if (!ctrl_break_pressed())
    c = (unsigned char)ndread(&syscon);
  if (c != CTL_C && *pdev != syscon)
    c = (unsigned char)ndread(pdev);
  if (c == CTL_C)
    handle_break(pdev, -1);
  return c;
}

/*
 * Handles a ^Break state
 *
 * Actions:
 *       1) clear the ^Break flag
 *       2) clear the STDIN stream
 *       3) echo ^C to sft_out or pdev if sft_out==-1
 *       4) decrease the InDOS flag as the kernel drops back to user space
 *       5) invoke INT-23 and never come back
 */

void handle_break(struct dhdr FAR **pdev, int sft_out)
{
  char *buf = "^C\r\n";

  CB_FLG &= ~CB_MSK;            /* reset the ^Break flag */
  con_flush(pdev);
  if (sft_out == -1)
    cooked_write(pdev, 4, buf);
  else
    DosRWSft(sft_out, 4, buf, XFR_FORCE_WRITE);
  if (!ErrorMode)               /* within int21_handler, InDOS is not incremented */
    if (InDOS)
      --InDOS;                  /* fail-safe */

  spawn_int23();                /* invoke user INT-23 and never come back */
}

