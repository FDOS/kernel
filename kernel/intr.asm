; File:
;                         intr.asm
; Description:
;       Assembly implementation of calling an interrupt
;
;                    Copyright (c) 2000
;                       Steffen Kaiser
;                       All Rights Reserved
;
; This file is part of FreeDOS.
;
; FreeDOS is free software; you can redistribute it and/or
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


		%include "segs.inc"

segment	_TEXT
;
;       void intr(nr, rp)
;       REG int nr
;       REG struct REGPACK *rp
;
;
                global	_intr
_intr:
                push    bp                      ; Standard C entry
                mov     bp,sp
                push    si
                push    di
                push	ds
                push    es

                mov ax, [bp+4]			; interrupt number
                mov [CS:intr?1-1], al
                jmp short intr?2		; flush the instruction cache
intr?2			mov bx, [bp+6]			; regpack structure
				mov ax, [bx]
				mov cx, [bx+4]
				mov dx, [bx+6]
				mov bp, [bx+8]
				mov di, [bx+10]
				mov si, [bx+12]
				push Word [bx+14]			; ds
				mov es, [bx+16]
				mov bx, [bx+2]
				pop ds

				int 0
intr?1:

				pushf
				push ds
				push bx
				mov bx, sp
				mov ds, [SS:bx+8]
				mov bx, [ss:bx+20]		; address of REGPACK
				mov [bx], ax
				pop ax
				mov [bx+2], ax
				mov [bx+4], cx
				mov [bx+6], dx
				mov [bx+8], bp
				mov [bx+10], di
				mov [bx+12], si
				pop ax
				mov [bx+14], ax
				mov [bx+16], es
				pop ax
				mov [bx+18], ax

			    pop     es
				pop		ds
                pop     di
                pop     si
                pop     bp
                ret


