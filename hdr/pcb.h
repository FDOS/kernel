/****************************************************************/
/*                                                              */
/*                             pcb.h                            */
/*                                                              */
/*           Process Control and Interrupt data structures      */
/*                                                              */
/*                       November 26, 1991                      */
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

/***************************************************************
       2000/03/22 ska
   There is a newly documented (though used previously) side effect
   of the definitions and assumptions made herein:
   The assembly sources may use a macro named "PUSH$ALL" to push
   all processor registers onto the stack, see example below:
           PUSH$ALL
           mov ax, sp
           ...
           push ax
           call _c_function
           pop cx
   The stack pointer immediately after the PUSH$ALL macro shall point to
   a structure used as an "iregs" structure within the C language.
   Therefore the internal of the structure "iregs" _must_ always
   match the implementation of the macro "PUSH$ALL".
*/

#ifndef __PCB_H
#define __PCB_H

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *pcb_hRcsId =
    "$Id: pcb.h 1316 2007-05-15 17:48:47Z bartoldeman $";
#endif
#endif

   /* Force one-byte alignment for all the internal structures, see above */
#include <algnbyte.h>
/*                                                                      */
/* interrupt handler structure definition                               */
/*                                                                      */
typedef union {
  UWORD x;                      /* access mode for ax, bx, etc.         */
  struct {
    UBYTE l;                    /* access mode for al, bl, etc.         */
    UBYTE h;                    /* access mode for ah, bh, etc.         */
  } b;
} xreg;

/* The structure assumes that:
   1) An interrupt was invoked, &
   2) the PUSH$ALL macro was invoked immediately after that.
   Furthermore, the PUSH$ALL macro must push ES first and AX last.
       -- 2000/03/22 ska*/
/* maps MS-DOS unique stacking order                                    */
typedef struct _iregss {
  xreg a, b, c, d;
  UWORD si, di, bp, ds, es;
  UWORD ip, cs, flags;
} iregs;

/* struct used for local copy of registers */
typedef struct {
  xreg a, b, c, d;
  UWORD si, di, ds, es;
} lregs;

/* Process control block for task switching                             */
typedef struct {
  UWORD pc_ss;
  UWORD pc_sp;
  iregs pc_regs;
} pcb;

/* Note: The following figure is not made by myself and I assume that
   the order of "ES" through "AX" are misinterpreted?!  -- 2000/03/22 ska*/

/* For MSC, the following offsets must match the assembly process       */
/* support offsets                                                      */
/* NOTE: Alignemnts must be set to 1 (-Zp1)                             */
/*      ss:     0                                                       */
/*      sp:     2                                                       */
/*      es:     4                                                       */
/*      ds:     6                                                       */
/*      di:     8                                                       */
/*      si:     10                                                      */
/*      bp:     12                                                      */
/*      sp:     14      NOTE: not used in this structure                */
/*      bx:     16                                                      */
/*      dx:     18                                                      */
/*      cx:     20                                                      */
/*      ax:     22                                                      */
/*      ip:     24                                                      */
/*      cs:     26                                                      */
/*      flags:  28                                                      */
/*                                                                      */
/* For Borland C, the following offsets must match the assembly process */
/* support offsets                                                      */
/*      ss:     0                                                       */
/*      sp:     2                                                       */
/*      bp:     4                                                       */
/*      di:     6                                                       */
/*      si:     8                                                       */
/*      ds:     10                                                      */
/*      es:     12                                                      */
/*      dx:     14                                                      */
/*      cx:     16                                                      */
/*      bx:     18                                                      */
/*      ax:     20                                                      */
/*      ip:     22                                                      */
/*      cs:     24                                                      */
/*      flags:  26                                                      */

#define BP      bp
#define DI      di
#define SI      si
#define DS      ds
#define ES      es
#define DX      d.x
#define CX      c.x
#define BX      b.x
#define AX      a.x
#define DH      d.b.h
#define CH      c.b.h
#define BH      b.b.h
#define AH      a.b.h
#define DL      d.b.l
#define CL      c.b.l
#define BL      b.b.l
#define AL      a.b.l
#define IP      ip
#define CS      cs
#define FLAGS   flags

#define FLG_ZERO        0x0040
#define FLG_CARRY       0x0001

   /* Allow default alignment from now on */
#include <algndflt.h>

#endif

