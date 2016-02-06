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
; write to the Free Software Foundation, Inc.,
; 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
;

		%include "segs.inc"

%macro INTR 0
        
                push    bp                      ; Standard C entry
                mov     bp,sp
                push    si
                push    di
%ifdef WATCOM
                push    bx
                push    cx
                push    dx
                push    es
%endif
                push	ds

                mov	ax, [bp+6]		; interrupt number
                mov	[cs:%%intr_1-1], al
                jmp 	short %%intr_2		; flush the instruction cache
%%intr_2	mov	bx, [bp+4]		; regpack structure
		mov	ax, [bx]
		mov	cx, [bx+4]
		mov	dx, [bx+6]
		mov	si, [bx+8]
		mov	di, [bx+10]
		mov	bp, [bx+12]
		push	word [bx+14]		; ds
		mov	es, [bx+16]
		mov	bx, [bx+2]
		pop	ds
		int	0
%%intr_1:

		pushf
		push	ds
		push	bx
		mov	bx, sp
		mov	ds, [ss:bx+6]
%ifdef WATCOM
		mov	bx, [ss:bx+24]		; address of REGPACK
%else
		mov	bx, [ss:bx+16]		; address of REGPACK
%endif
		mov	[bx], ax
		pop	word [bx+2]
		mov	[bx+4], cx
		mov	[bx+6], dx
		mov	[bx+8], si
		mov	[bx+10], di
		mov	[bx+12], bp
		pop	word [bx+14]
		mov	[bx+16], es
		pop	word [bx+22]

		pop	ds
%ifdef WATCOM
                pop     es
                pop     dx
                pop     cx
                pop     bx
%endif
		pop	di
		pop	si
		pop	bp
		ret     4
%endmacro

segment	HMA_TEXT

;; COUNT ASMPASCAL res_DosExec(COUNT mode, exec_blk * ep, BYTE * lp)
    global RES_DOSEXEC
RES_DOSEXEC:
        pop es                  ; ret address
        pop dx                  ; filename
        pop bx                  ; exec block
        pop ax                  ; mode
        push es                 ; ret address
        mov ah, 4bh
        push ds                 
        pop es                  ; es = ds
        int 21h
        jc short no_exec_error
        xor ax, ax
no_exec_error:
        ret

;; UCOUNT ASMPASCAL res_read(int fd, void *buf, UCOUNT count); 
    global RES_READ
RES_READ:
        pop ax         ; ret address
        pop cx         ; count
        pop dx         ; buf
        pop bx         ; fd
        push ax        ; ret address
        mov ah, 3fh
        int 21h
        jnc no_read_error
        mov ax, -1
no_read_error:
        ret

segment	INIT_TEXT
;
;       void init_call_intr(nr, rp)
;       REG int nr
;       REG struct REGPACK *rp
;
		global	INIT_CALL_INTR
INIT_CALL_INTR:
		INTR

;
; int init_call_XMScall( (WORD FAR * driverAddress)(), WORD AX, WORD DX)
;
; this calls HIMEM.SYS 
;
                global INIT_CALL_XMSCALL
INIT_CALL_XMSCALL:
            pop  bx         ; ret address
            pop  dx
            pop  ax
            pop  cx         ; driver address
            pop  es

            push cs         ; ret address
            push bx
            push es         ; driver address ("jmp es:cx")
            push cx
            retf
            
; void FAR *DetectXMSDriver(VOID)
global DETECTXMSDRIVER
DETECTXMSDRIVER:
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

global KEYCHECK
KEYCHECK:
        mov ah, 1
        int 16h
        ret                

;; int open(const char *pathname, int flags); 
    global INIT_DOSOPEN
INIT_DOSOPEN: 
        ;; init calling DOS through ints:
        pop bx         ; ret address
        pop ax         ; flags
        pop dx         ; pathname
        push bx        ; ret address
        mov ah, 3dh
        ;; AX will have the file handle

common_int21:
        int 21h
        jnc common_no_error
        mov ax, -1
common_no_error:
        ret

;; int close(int fd);
    global CLOSE
CLOSE:         
        pop ax         ; ret address
        pop bx         ; fd
        push ax        ; ret address
        mov ah, 3eh
        jmp short common_int21

;; UCOUNT read(int fd, void *buf, UCOUNT count); 
    global READ
READ: 
        pop ax         ; ret address
        pop cx         ; count
        pop dx         ; buf
        pop bx         ; fd
        push ax        ; ret address
        mov ah, 3fh
        jmp short common_int21

;; int dup2(int oldfd, int newfd); 
    global DUP2
DUP2:
        pop ax         ; ret address
        pop cx         ; newfd
        pop bx         ; oldfd
        push ax        ; ret address
        mov ah, 46h
        jmp short common_int21
        
;
; ULONG ASMPASCAL lseek(int fd, long position);
;
    global LSEEK
LSEEK:
        pop ax         ; ret address
        pop dx         ; position low
        pop cx         ; position high
        pop bx         ; fd
        push ax        ; ret address
        mov ax,4200h   ; origin: start of file
        int 21h
        jnc     seek_ret        ; CF=1?
        sbb     ax,ax           ;  then dx:ax = -1, else unchanged
        sbb     dx,dx
seek_ret:
        ret
        
;; VOID init_PSPSet(seg psp_seg)
    global INIT_PSPSET
INIT_PSPSET:
        pop ax         ; ret address
        pop bx         ; psp_seg
        push ax        ; ret_address
	mov ah, 50h
        int 21h
        ret

;; COUNT init_DosExec(COUNT mode, exec_blk * ep, BYTE * lp)
    global INIT_DOSEXEC
INIT_DOSEXEC:
        pop es                  ; ret address
        pop dx                  ; filename
        pop bx                  ; exec block
        pop ax                  ; mode
        push es                 ; ret address
        mov ah, 4bh
        push ds                 
        pop es                  ; es = ds
        int 21h
        jc short exec_no_error
        xor ax, ax
exec_no_error:
        ret

;; int init_setdrive(int drive)
   global INIT_SETDRIVE
INIT_SETDRIVE:
	mov ah, 0x0e
common_dl_int21:
        pop bx                  ; ret address
        pop dx                  ; drive/char
        push bx
        int 21h
        ret

;; int init_switchar(int char)
   global INIT_SWITCHAR
INIT_SWITCHAR:
	mov ax, 0x3701
	jmp short common_dl_int21

;
; seg ASMPASCAL allocmem(UWORD size);
;
    global ALLOCMEM
ALLOCMEM:
        pop ax           ; ret address
        pop bx           ; size
        push ax          ; ret address
        mov ah, 48h
        int 21h
        sbb bx, bx       ; carry=1 -> ax=-1
        or  ax, bx       ; segment
        ret
                        
;; void set_DTA(void far *dta)        
    global SET_DTA
SET_DTA:
        pop ax           ; ret address
        pop dx           ; off(dta)
        pop bx           ; seg(dta)
        push ax          ; ret address
        mov ah, 1ah
        push ds
        mov ds, bx
        int 21h
        pop ds
        ret
