;
; File:
;                          execrh.asm
; Description:
;             request handler for calling device drivers
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

segment	HMA_TEXT
                ; EXECRH
                ;       Execute Device Request
                ;
                ; execrh(rhp, dhp)
                ; request far *rhp;
                ; struct dhdr far *dhp;
                ;
;
; The stack is very critical in here.
;
        global  EXECRH
	global  INIT_EXECRH

%macro EXECRHM 0
                push    bp              ; perform c entry
                mov     bp,sp
                push    si
                push    ds              ; sp=bp-8

                lds     si,[bp+4]       ; ds:si = device header
                les     bx,[bp+8]       ; es:bx = request header


                mov     ax, [si+6]      ; construct strategy address
                mov     [bp+4], ax    

                push si                 ; the bloody fucking RTSND.DOS 
                push di                 ; driver destroys SI,DI (tom 14.2.03)

                call    far[bp+4]       ; call far the strategy

                pop di 
                pop si
                                

		mov     ax,[si+8]       ; construct 'interrupt' address
                mov     [bp+4],ax       ; construct interrupt address 
                call    far[bp+4]       ; call far the interrupt

                sti                     ; damm driver turn off ints
                cld                     ; has gone backwards
                pop     ds
                pop     si
                pop     bp
                ret     8
%endmacro

EXECRH:
	EXECRHM

segment INIT_TEXT

INIT_EXECRH:
	EXECRHM
