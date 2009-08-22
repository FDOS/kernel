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
; $Id$
;


;       Code for stack switching during hardware interrupts.

; Format of interrupt sharing protocol interrupt handler entry point: 
; Offset  Size    Description     (Table 02568)
;  00h  2 BYTEs   short jump to actual start of interrupt handler, immediately
;                   following this data block (EBh 10h)
;  02h    DWORD   address of next handler in chain
;  06h    WORD    signature 424Bh
;  08h    BYTE    EOI flag
;                 00h software interrupt or secondary hardware interrupt handler
;                 80h primary hardware interrupt handler (will issue EOI to
;                       interrupt controller)
;  09h  2 BYTEs   short jump to hardware reset routine
;                 must point at a valid FAR procedure (may be just RETF)
;  0Bh  7 BYTEs   reserved (0) by IBM for future expansion

; Ralf Brown documents that irq 2, 3, 4, 5, 6, 10, 11, 12, 14, 15 use the above
; protocol..
; MS (http://support.microsoft.com/kb/84300/)
; documents that STACKS= implements stacks for interrupt vectors
; 02H, 08-0EH, 70H, and 72-77H.
; that means that we need to redirect NMI (INT 2), irq 0, 1, 8, 13 without sharing
; irq 9 (==irq2) and irq 7 (printer) are not handled at all

%include "segs.inc"

segment	_IRQTEXT

stack_size      dw      0
stack_top       dw      0
stack_offs      dw      0
stack_seg       dw      0

%macro irq 0
                call    general_irq_service
                dd      0
%endmacro

%macro irqshare 1
                jmp     short %%1
                dd      0
                dw      424bh
                db      0
                jmp     short retf%1
                times 7 db 0
%%1:            call    general_irq_service_share
%endmacro

nmi:            irq
irq_0:          irq
irq_1:          irq
irq_08:         irq
irq_0d:         irq

retf1:          retf
irq_2:          irqshare        1
irq_3:          irqshare        1
irq_4:          irqshare        1
irq_5:          irqshare        1
irq_6:          irqshare        1
irq_0a:         irqshare        2
irq_0b:         irqshare        2
irq_0c:         irqshare        2
irq_0e:         irqshare        2
irq_0f:         irqshare        2
retf2:          retf

                ; align to 100h to align _LOWTEXT for interrupt vectors
                ; in kernel.asm
                times (100h - ($ - stack_size)) db 0

segment _IO_TEXT

general_irq_service:
                push    bx
                mov     bx, sp
                mov     bx, [ss:bx+2]   ; return address->old ivec
                jmp     common_irq
        
general_irq_service_share:
                push    bx
                mov     bx, sp
                mov     bx, [ss:bx+2]   ; return address->old ivec
                sub     bx, irq_3 - irq_2 - 2
common_irq:
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
                call    far word [bx]

                cli
                add     [stack_top], ax

                pop     ax              ; get stored SS:SP
                pop     dx

                mov     ss, dx          ; switch back to old stack
                mov     sp, ax

return:         pop     ds              ; restore registers and return
                pop     ax
                pop     dx
                pop     bx
                add     sp, 2
                iret

dont_switch:    pushf
                call    far word [bx]
                jmp     short return


segment	INIT_TEXT

global  _init_stacks
; VOID    init_stacks(VOID FAR *stack_base, COUNT nStacks, WORD stackSize);

int_numbers:            db 2,8,9,70h,75h
int_numbers_share:      db 0ah,0bh,0ch,0dh,0eh,72h,73h,74h,76h,77h
        
_init_stacks:
                push    bp
                mov     bp, sp
                push    ds
                push    di
                push    si


		mov	ax,LGROUP
		mov	ds,ax
		mov	es,ax

                mov     bx, [bp+4]
                mov     dx, [bp+6]
                mov     ax, [bp+8]
                mov     cx, [bp+0ah]

                mov     [stack_size], cx
                mov     [stack_offs], bx
                mov     [stack_seg], dx

                mul     cx
                add     ax, bx
                ; stack_top = stack_size * nStacks + stack_seg:stack_offs
                mov     [stack_top], ax 

                xor     ax, ax
                mov     ds, ax

                mov     di, nmi + 3
                mov     dx, nmi
                mov     bx, int_numbers
                mov     cx, int_numbers_share - int_numbers
                mov     bp, irq_1 - irq_0
                call    set_vect

                inc     dx ; skip over retf (not di: go from nmi+3 to irq_2+2)
                mov     cx, _init_stacks - int_numbers_share
                mov     bp, irq_3 - irq_2
                call    set_vect

                pop     si
                pop     di
                pop     ds
                pop     bp
                ret

; set interrupt vectors:
; in: es=LGROUP, ds=0
; bx: pointer to int_numbers bytes in cs
; cx: number of vectors to set
; dx: pointer to es:nmi and so on (new interrupt vectors)
; di: pointer to es:nmi+3 and so on (pointer to place of old interrupt vectors)
; bp: difference in offset between irq structures
; out: bx, si, di updated, cx=0, ax destroyed
set_vect:
                mov     al, [cs:bx] ; get next int vector offset
                inc     bx
                cbw
                mov     si, ax
                shl     si, 1
                shl     si, 1       ; now ds:si -> int vector, es:di -> nmi+3, etc

                movsw               ; save old vector
                movsw

                cli
                mov     [si-4], dx  ; set new vector
                mov     [si-2], es
                sti

                add     dx, bp
                lea     di, [di+bp-4] ; update di, compensating for movsw
                loop    set_vect

                ret
