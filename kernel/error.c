/****************************************************************/
/*                                                              */
/*                           error.c                            */
/*                                                              */
/*               Main Kernel Error Handler Functions            */
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

#ifdef VERSION_STRINGS
static BYTE *errorRcsId =
    "$Id$";
#endif

#include "globals.h"

#ifdef DEBUG
/* error registers                                      */
VOID dump(void)
{
  printf("Register Dump [AH = %02x CS:IP = %04x:%04x FLAGS = %04x]\n",
         error_regs.AH, error_regs.CS, error_regs.IP, error_regs.FLAGS);
  printf("AX:%04x BX:%04x CX:%04x DX:%04x\n",
         error_regs.AX, error_regs.BX, error_regs.CX, error_regs.DX);
  printf("SI:%04x DI:%04x DS:%04x ES:%04x\n",
         error_regs.SI, error_regs.DI, error_regs.DS, error_regs.ES);
}
#endif

/* issue a panic message for corrupted data structures          */
VOID panic(BYTE * s)
{
  printf("\nPANIC: %s\nSystem halted\n", s);
  for (;;) ;
}

#ifdef IPL
/* issue an internal error message                              */
VOID fatal(BYTE * err_msg)
{
  printf("\nInternal IPL error - %s\nSystem halted\n", err_msg);
  exit(-1);
}
#else
/* issue an internal error message                              */
#if 0
VOID fatal(BYTE * err_msg)
{
  printf("\nInternal kernel error - \n");
  panic(err_msg);
}
#endif
#endif

/* Abort, retry or fail for character devices                   */
COUNT char_error(request * rq, struct dhdr FAR * lpDevice)
{
  return CriticalError(EFLG_CHAR | EFLG_ABORT | EFLG_RETRY | EFLG_IGNORE,
                       0, rq->r_status & S_MASK, lpDevice);
}

/* Abort, retry or fail for block devices                       */
COUNT block_error(request * rq, COUNT nDrive, struct dhdr FAR * lpDevice)
{
  return CriticalError(EFLG_ABORT | EFLG_RETRY | EFLG_IGNORE,
                       nDrive, rq->r_status & S_MASK, lpDevice);
}

/*
 * Log: error.c,v - for newer entries do "cvs log error.c"
 *
 * Revision 1.2  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.1.1.1  1999/03/29 15:41:55  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.5   06 Dec 1998  8:43:54   patv
 * Now handles errors like MS-DOS.
 *
 *    Rev 1.4   04 Jan 1998 23:14:36   patv
 * Changed Log for strip utility
 *
 *    Rev 1.3   29 May 1996 21:15:10   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:46   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:06:14   patv
 * Initial revision.
 */
