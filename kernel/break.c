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

extern void spawn_int23(void);

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.2  2000/03/09 06:07:10  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.1  1999/04/16 21:18:17  jprice
 * Steffen contributed.
 *
 */

#define CB_FLG *(UBYTE FAR*)MK_FP(0x40, 0x71)
#define CB_MSK 0x80

/* Check for ^Break.

 * Two sources are available:
 *                                                                              1) flag at 40:71 bit 7
 *                                                                              2) STDIN str‘am via con_break()
 */
int control_break(void)
{
  return (CB_FLG & CB_MSK) || con_break();
}

/*
 * Handles a ^Break state
 *
 * Actions:
 *                                                                              1) clear the ^Break flag
 *                                                                              2) clear the STDIN stream
 *                                                                              3) decrease the InDOS flag as the kernel drops back to user space
 *                                                                              4) invoke INT-23 and never come back
 */
void handle_break(void)
{
  CB_FLG &= ~CB_MSK;            /* reset the ^Break flag */
  KbdFlush();                   /* Er, this is con_flush() */
  if (!ErrorMode)               /* within int21_handler, InDOS is not incremented */
    if (InDOS)
      --InDOS;                  /* fail-safe */

  spawn_int23();                /* invoke user INT-23 and never come back */
}
