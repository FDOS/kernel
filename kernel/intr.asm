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

segment	HMA_TEXT
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


                global	_int3
_int3:
                int 3
                retf


segment	INIT_TEXT
%if 0
;
;       void init_call_intr(nr, rp)
;       REG int nr
;       REG struct REGPACK *rp
;
; same stuff as above, but in INIT_SEGMENT
                global	_init_call_intr
_init_call_intr:
                push    bp                      ; Standard C entry
                mov     bp,sp
                push    si
                push    di
                push	ds
                push    es

                mov ax, [bp+4]			; interrupt number
                mov [CS:init_intr?1-1], al
                jmp short init_intr?2		; flush the instruction cache
init_intr?2 	mov bx, [bp+6]			; regpack structure
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
init_intr?1:

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


%endif
;
; int init_call_XMScall( (WORD FAR * driverAddress)(), WORD AX, WORD DX)
;
; this calls HIMEM.SYS 
;
                global _init_call_XMScall
_init_call_XMScall:
            push bp
            mov  bp,sp
            
            mov  ax,[bp+8]
            mov  dx,[bp+10]
            call far [bp+4]

            pop  bp
            ret
            
; void FAR *DetectXMSDriver(VOID)
global _DetectXMSDriver
_DetectXMSDriver:
        mov ax, 4300h
        int 2fh                 ; XMS installation check

        cmp al, 80h
        je detected
        xor ax, ax
        xor dx, dx
        ret

detected:
        push es
        push bx
        mov ax, 4310h           ; XMS get driver address
        int 2fh
        
        mov ax, bx
        mov dx, es
        pop bx
        pop es
        ret        

global _keycheck        
_keycheck:      
        mov ah, 1
        int 16h
        ret                

;; COUNT init_DosOpen(BYTE *fname, COUNT mode)
    global _init_DosOpen
_init_DosOpen: 
        ;; first implementation of init calling DOS through ints:
        mov bx, sp
        mov ah, 3dh
        ;; we know that SS=DS during init stage.
        mov al, [bx+4]
        mov dx, [bx+2]
        int 21h
common_exit:        
        jnc open_no_error
        ;; AX has file handle
        neg ax
        ;; negative value for error code
open_no_error:
        ret

;; COUNT init_DosClose(COUNT hndl)
    global _init_DosClose
_init_DosClose:
        mov bx, sp
        mov bx, [bx+2]
        mov ah, 3eh
        int 21h
        jmp common_exit

;; COUNT init_DosRead(COUNT hndl, BYTE *bp, UCOUNT n)
    global _init_DosRead
_init_DosRead:
        mov bx, sp
        mov cx, [bx+6]
        mov dx, [bx+4]
        mov bx, [bx+2]
        mov ah, 3fh
        int 21h
        jmp common_exit

;; VOID init_PSPInit(seg psp_seg)
    global _init_PSPInit
_init_PSPInit:        
        push si
        mov ah, 55h
        mov bx, sp
        mov dx, [bx+4]
        xor si, si
        int 21h
        pop si
        ret
                        