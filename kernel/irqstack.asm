; File:
;                         irqstack.asm
; Description:
;     Assembly support routines for hardware stack support
;
;                    Copyright (c) 1997, 1998
;                          Svante Frey
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
; $Logfile:   C:/dos-c/src/kernel/irqstack.asv  $
;
; $Id$
;
; $Log$
; Revision 1.1  2000/05/06 19:35:21  jhall1
; Initial revision
;
; Revision 1.3  1999/08/10 17:57:13  jprice
; ror4 2011-02 patch
;
; Revision 1.2  1999/04/16 12:21:22  jprice
; Steffen c-break handler changes
;
; Revision 1.1.1.1  1999/03/29 15:41:10  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.3  1999/02/01 01:48:41  jprice
; Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
;
; Revision 1.2  1999/01/22 04:13:26  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
; Imported sources
;
;
;     Rev 1.2   06 Dec 1998  8:49:08   patv
;  Bug fixes.
;
;     Rev 1.1   22 Jan 1997 13:15:34   patv
;  pre-0.92 Svante Frey bug fixes
;
;     Rev 1.0   16 Jan 1997 21:43:44   patv
;  Initial revision.
; $EndLog$
;


;       Code for stack switching during hardware interrupts.

group	TGROUP	_TEXT

segment	_TEXT	class=CODE

old_vectors     times 16 dd 0
stack_size      dw      0
stack_top       dw      0
stack_offs      dw      0
stack_seg       dw      0

irq_0:          push    bx
                mov     bx, 0 * 4
                jmp     short general_irq_service

irq_1:          push    bx
                mov     bx, 1 * 4
                jmp     short general_irq_service

irq_2:          push    bx
                mov     bx, 2 * 4
                jmp     short general_irq_service

irq_3:          push    bx
                mov     bx, 3 * 4
                jmp     short general_irq_service

irq_4:          push    bx
                mov     bx, 4 * 4
                jmp     short general_irq_service

irq_5:          push    bx
                mov     bx, 5 * 4
                jmp     short general_irq_service

irq_6:          push    bx
                mov     bx, 6 * 4
                jmp     short general_irq_service

irq_7:          push    bx
                mov     bx, 7 * 4
                jmp     short general_irq_service

irq_08:         push    bx
                mov     bx, 8 * 4
                jmp     short general_irq_service

irq_09:         push    bx
                mov     bx, 9 * 4
                jmp     short general_irq_service

irq_0a:         push    bx
                mov     bx, 0ah * 4
                jmp     short general_irq_service

irq_0b:         push    bx
                mov     bx, 0bh * 4
                jmp     short general_irq_service

irq_0c:         push    bx
                mov     bx, 0ch * 4
                jmp     short general_irq_service

irq_0d:         push    bx
                mov     bx, 0dh * 4
                jmp     short general_irq_service

irq_0e:         push    bx
                mov     bx, 0eh * 4
                jmp     short general_irq_service

irq_0f:         push    bx
                mov     bx, 0fh * 4
;                jmp     short general_irq_service

general_irq_service:
                push    dx
                push    ax
                push    ds

                mov     ax, cs
                mov     ds, ax

                mov     ax, [stack_top]
                cmp     ax, [stack_offs]
                jbe     dont_switch

                mov     dx, ss
                mov     ax, sp

                mov     ss, [stack_seg]
                mov     sp, [stack_top]

                push    dx              ; save old SS:SP on new stack
                push    ax

                mov     ax, [stack_size]
                sub     [stack_top], ax

                pushf
                call    far word [old_vectors+bx]

                cli
                add     [stack_top], ax

                pop     ax              ; get stored SS:SP
                pop     dx

                mov     ss, dx          ; switch back to old stack
                mov     sp, ax

                pop     ds              ; restore registers and return
                pop     ax
                pop     dx
                pop     bx
                iret

dont_switch:    pushf
                call    far word [old_vectors+bx]
                pop     ds
                pop     ax
                pop     dx
                pop     bx
                iret


segment	INIT_TEXT class=INIT

global  _init_stacks
; VOID    init_stacks(VOID FAR *stack_base, COUNT nStacks, WORD stackSize);

_init_stacks:
                push    bp
                mov     bp, sp
                push    ds
                push    di
                push    si


		mov	ax,_TEXT
		mov	ds,ax

                mov     bx, [bp+4]
                mov     dx, [bp+6]
                mov     ax, [bp+8]
                mov     cx, [bp+0ah]

                mov     [stack_size], cx
                mov     [stack_offs], bx
                mov     [stack_seg], dx

                mul     cx
                add     ax, bx
                mov     [stack_top], ax

                xor     ax, ax
                mov     ds, ax

		mov	ax, _TEXT
		mov	es, ax

                mov     di, old_vectors
                mov     si, 8 * 4
                mov     cx, 10h
                rep     movsw

                mov     si, 70h * 4
                mov     cx, 10h
                rep     movsw

                push    ds
                pop     es

                mov     di, 8 * 4
                mov     dx, irq_0
                call    set_vect

                mov     di, 70h * 4
                call    set_vect

                pop     si
                pop     di
                pop     ds
                pop     bp
                ret

set_vect:
                mov     cx, 8

set_next:       mov     ax, dx
                cli
                stosw
                mov     ax, _TEXT
                stosw
                sti
                add     dx, irq_1 - irq_0
                loop    set_next

                ret
