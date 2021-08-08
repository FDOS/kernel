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
		%include "stacks.inc"

%macro INTR 1
        
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
				pushf

arg nr, {rp,%1}
                mov	ax, [.nr]		; interrupt number
                mov	[cs:%%intr_1-1], al
                jmp 	short %%intr_2		; flush the instruction cache
%%intr_2:
%if %1 == 4
		lds	bx, [.rp]		; regpack structure FAR
%else
		mov	bx, [.rp]		; regpack structure
%endif
		mov	ax, [bx]
		mov	cx, [bx+4]
		mov	dx, [bx+6]
		mov	si, [bx+8]
		mov	di, [bx+10]
		mov	bp, [bx+12]
		push	word [bx+14]		; ds
		mov	es, [bx+16]
		push word [bx+22]			; flags
		popf
		mov	bx, [bx+2]
		pop	ds
		int	0
%%intr_1:

		pushf
		push	ds
		push	bx
		mov	bx, sp
%if %1 == 4
%ifdef WATCOM
		lds bx, [ss:bx+(14+8)+4]		; FAR address of REGPACK, pascal convention
%else
		lds bx, [ss:bx+14+.rp-bp]	; FAR address of REGPACK, SP=BX + skip saved registers (14) + 
%endif
%else
		mov	ds, [ss:bx+8]
%ifdef WATCOM
		mov	bx, [ss:bx+26]			; NEAR address of REGPACK, pascal convention
%else
		mov	bx, [ss:bx+14+.rp-bp]	; NEAR address of REGPACK
%endif
%endif
		mov	[bx], ax
		pop	word [bx+2]				; bx
		mov	[bx+4], cx
		mov	[bx+6], dx
		mov	[bx+8], si
		mov	[bx+10], di
		mov	[bx+12], bp
		pop	word [bx+14]			; ds
		mov	[bx+16], es
		pop	word [bx+22]			; flags

		; restore all registers to values from INTR entry
		popf
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
%endmacro

segment	HMA_TEXT

;
;       void ASMPASCAL call_intr(WORD nr, struct REGPACK FAR *rp)
;
		global	CALL_INTR
CALL_INTR:
		INTR 4 ; rp is far, DWORD argument
		ret 6

;; COUNT ASMPASCAL res_DosExec(COUNT mode, exec_blk * ep, BYTE * lp)
    global RES_DOSEXEC
RES_DOSEXEC:
        pop es                  ; ret address
        popargs ax,bx,dx        ; mode, exec block, filename
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
        popargs bx,dx,cx ; fd, buf, count
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
		INTR 2	; rp is near, WORD argument
		ret 4

;
; int init_call_XMScall( (WORD FAR * driverAddress)(), WORD AX, WORD DX)
;
; this calls HIMEM.SYS 
;
                global INIT_CALL_XMSCALL
INIT_CALL_XMSCALL:
            pop  bx         ; ret address
            popargs {es,cx},ax,dx

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
        popargs dx,ax  ; pathname, flags
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
        popargs bx,dx,cx ; fd,buf,count
        push ax        ; ret address
        mov ah, 3fh
        jmp short common_int21

;; int dup2(int oldfd, int newfd); 
    global DUP2
DUP2:
        pop ax         ; ret address
        popargs bx,cx  ; oldfd,newfd
        push ax        ; ret address
        mov ah, 46h
        jmp short common_int21
        
;
; ULONG ASMPASCAL lseek(int fd, long position);
;
    global LSEEK
LSEEK:
        pop ax         ; ret address
        popargs bx,{cx,dx} ; fd, position high:low
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
        popargs ax,bx,dx        ; mode, exec block, filename
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
        popargs {bx,dx}  ; seg:off(dta)
        push ax          ; ret address
        mov ah, 1ah
        push ds
        mov ds, bx
        int 21h
        pop ds
        ret
