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
    "$Id$";
#endif

#define CB_FLG *(UBYTE FAR*)MK_FP(0x0, 0x471)
#define CB_MSK 0x80

/* Check for ^Break/^C.
 * Three sources are available:
 *       1) flag at 40:71 bit 7
 *       2) CON stream (if STDIN is redirected somewhere else)
 *       3) input stream (most likely STDIN)
 * Actions:
 *       1) echo ^C
 *       2) clear the STDIN stream
 *       3) decrease the InDOS flag as the kernel drops back to user space
 *       4) invoke INT-23 and never come back
 */
unsigned char check_handle_break(struct dhdr FAR **pdev, int sft_out)
{
  unsigned char c = CTL_C;
  if (CB_FLG & CB_MSK)
    CB_FLG &= ~CB_MSK;            /* reset the ^Break flag */
  else
    c = (unsigned char)ndread(&syscon);
  if (c == CTL_C)
  {
    sft_out = -1;
    pdev = &syscon;
  }
  else if (*pdev != syscon)
    c = (unsigned char)ndread(pdev);
  if (c == CTL_C)
  {
    con_flush(pdev);
    echo_ctl_c(pdev, sft_out);
    if (!ErrorMode)      /* within int21_handler, InDOS is not incremented */
      if (InDOS)
        --InDOS;         /* fail-safe */

    spawn_int23();       /* invoke user INT-23 and never come back */
  }
  return c;
}
