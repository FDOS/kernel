; File:
;                         nls.asm
; Description:
;     Assembly support routines for nls functions.
;
;                    Copyright (c) 1995, 1998
;                       Pasquale J. Villani
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


		%include "segs.inc"
		%include "stacks.inc"

segment	HMA_TEXT
                global  _reloc_call_CharMapSrvc
                extern  _DosUpChar
                extern  _DGROUP_
;
; CharMapSrvc:
;       User callable character mapping service.
;       Part of Function 38h
;
_reloc_call_CharMapSrvc:

                Protect386Registers
                push    ds
                push    es
;                push    bp
;                push    si
;                push    di
                push    dx
                push    cx
                push    bx

                push    ax          ; arg of _upChar
                mov     ds,[cs:_DGROUP_]

                call    _DosUpChar
                ;add     sp, byte 2	// next POP retrieves orig AX

                pop bx
                mov ah, bh		; keep hibyte untouched

                pop     bx
                pop     cx
                pop     dx
;                pop     di
;                pop     si
;                pop     bp
                pop     es
                pop     ds
                Restore386Registers
                retf                            ; Return far
