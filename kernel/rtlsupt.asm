; File:
;                         rtlsupt.asm
; Description:
;     Assembly support routines for long mul/div 
;     was forced to do that for WATCOM C, which has _near 
;     LMUL/LDIV routines. shouldn't harm for others
;
;                    Copyright (c) 2001
;                       tom ehlert
;                       All Rights Reserved
;
; This file is part of DOS-C.
;
; DOS-C is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version
; 2, or (at your option) any later version.
;
; DOS-C is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
; the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
; $Id$
;
;

		%include "segs.inc"


segment	_TEXT
    
;
; cdecl calling conventions
;    
; ULONG FAR MULULUS(ULONG mul1, USHORT mul2) - MULtiply ULong by UShort
;
%IFNDEF I386
P8086       
%ELSE
P386				; Turn on 386 instructions.
%ENDIF ; I386


		global	_MULULUS
_MULULUS:

		push bp
		mov  bp,sp
		mov  bx,[bp+6+4]	; short mul2
		mov  ax,[bp+6+2]	; high part of mul1
		mul	 bx
		mov  cx,ax
		mov  ax,[bp+6+0]	; low part of mul1
		mul  bx
		add  dx,cx			; add in high part of result
		
		pop bp
		retf

