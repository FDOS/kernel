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
				mov si, [bx+8]
				mov di, [bx+10]
				mov bp, [bx+12]
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
				mov [bx+8], si
				mov [bx+10], di
				mov [bx+12], bp
				pop ax
				mov [bx+14], ax
				mov [bx+16], es
				pop ax
				mov [bx+22], ax

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
				mov si, [bx+8]
				mov di, [bx+10]
				mov bp, [bx+12]
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
				mov [bx+8], si
				mov [bx+10], di
				mov [bx+12], bp
				pop ax
				mov [bx+14], ax
				mov [bx+16], es
				pop ax
				mov [bx+22], ax

			    pop     es
				pop		ds
                pop     di
                pop     si
                pop     bp
                ret


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

;; int open(const char *pathname, int flags); 
    global _open
_open: 
        ;; first implementation of init calling DOS through ints:
        mov bx, sp
        mov ah, 3dh
        ;; we know that SS=DS during init stage.
        mov al, [bx+4]
        mov dx, [bx+2]
        int 21h
        ;; AX has file handle

common_exit:        
        jnc common_no_error
common_error:
        mov ax, -1
common_no_error:
        ret

;; int close(int fd);
    global _close
_close:         
        mov bx, sp
        mov bx, [bx+2]
        mov ah, 3eh
        int 21h
        jmp short common_exit

;; UCOUNT read(int fd, void *buf, UCOUNT count); 
    global _read
_read: 
        mov bx, sp
        mov cx, [bx+6]
        mov dx, [bx+4]
        mov bx, [bx+2]
        mov ah, 3fh
        int 21h
        jmp short common_exit

;; int dup2(int oldfd, int newfd); 
    global _dup2
_dup2:
        mov bx, sp
        mov cx, [bx+4]
        mov bx, [bx+2]
        mov ah, 46h
        int 21h
        jmp short common_exit
        
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

;; COUNT init_DosExec(COUNT mode, exec_blk * ep, BYTE * lp)
    global _init_DosExec
_init_DosExec:        
        mov ah, 4bh
        mov bx, sp
        mov al, [bx+2]
        push ds
        pop es
        mov dx, [bx+6]          ; filename
        mov bx, [bx+4]          ; exec block
        int 21h
        jc short exec_no_error
        xor ax, ax
exec_no_error        
        ret

;; int allocmem(UWORD size, seg *segp)
    global _allocmem
_allocmem:        
        mov ah, 48h
        mov bx, sp
        mov bx, [bx+2]
        int 21h
        jc short common_error
        mov bx, sp
        mov bx, [bx+4]
        mov [bx], ax
        xor ax, ax
        ret
                        
