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
  put_string("\nPANIC: ");
  put_string(s);
  put_string("\nSystem halted");
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

