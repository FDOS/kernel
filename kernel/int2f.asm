;
; File:
;                           int2f.asm
; Description:
;                 multiplex interrupt support code
;
;                    Copyright (c) 1996, 1998
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
; $Id: int2f.asm 1591 2011-05-06 01:46:55Z bartoldeman $
;

        %include "segs.inc"
        %include "stacks.inc"

; macro to switch to an internal stack (if necessary), set DS == SS == DGROUP,
; and push the old SS:SP onto the internal stack
;
; destroys AX, SI, BP; turns on IRQs
;
; int2f does not really need to switch to a separate stack for MS-DOS
; compatibility; this is mainly to work around the C code's assumption
; that SS == DGROUP
;
; TODO: remove the need for this hackery  -- tkchia
%macro SwitchToInt2fStack 0
    mov ax,[cs:_DGROUP_]
    mov ds,ax
    mov si,ss
    mov bp,sp
    cmp ax,si
    jz %%already
    cli
    mov ss,ax
    extern int2f_stk_top
    mov sp,int2f_stk_top
    sti
%%already:
; well, GCC does not currently clobber function parameters passed on the
; stack; but just in case it decides to do that in the future, we push _two_
; copies of the old SS:SP:
;   - the second copy can be passed as a pointer parameter to a C function
;   - the first copy is used to actually restore the user stack later
    push si
    push bp
    push si
    push bp
%endmacro

; macro to switch back from an internal stack, i.e. undo SwitchToInt2fStack
;
; destroys BP; turns on IRQs  -- tkchia
%macro DoneInt2fStack 0
    pop bp
    pop bp
    pop bp
    cli
    pop ss
    mov sp,bp
    sti
%endmacro

segment	HMA_TEXT
            extern _cu_psp
            extern _HaltCpuWhileIdle
            extern _syscall_MUX14

            extern _DGROUP_

                global  reloc_call_int2f_handler
reloc_call_int2f_handler:
                sti                             ; Enable interrupts
                cmp     ah,11h                  ; Network interrupt?
                jne     Int2f3                  ; No, continue
Int2f1:
                or      al,al                   ; Installation check?
                jz      FarTabRetn              ; yes, just return
Int2f2:
                mov ax,1                        ; TE 07/13/01
                                                ; at least for redirected INT21/5F44
                                                ; --> 2f/111e
                                                ; the error code is AX=0001 = unknown function
                stc
FarTabRetn:
                retf    2                       ; Return far

WinIdle:					; only HLT if at haltlevel 2+
		push	ds
                mov     ds, [cs:_DGROUP_]
		cmp	byte [_HaltCpuWhileIdle],2
		pop	ds
		jb	FarTabRetn
		pushf
		sti
		hlt				; save some energy :-)
		popf
		push	ds
                mov     ds, [cs:_DGROUP_]
		cmp	byte [_HaltCpuWhileIdle],3
		pop	ds
		jb	FarTabRetn
		mov	al,0			; even admit we HLTed ;-)
		jmp	short FarTabRetn

Int2f3:         cmp     ax,1680h                ; Win "release time slice"
                je      WinIdle
                cmp     ah,16h
                je      FarTabRetn              ; other Win Hook return fast
                cmp     ah,12h
                je      IntDosCal               ; Dos Internal calls

                cmp     ax,4a01h
                je      IntDosCal               ; Dos Internal calls
                cmp     ax,4a02h
                je      IntDosCal               ; Dos Internal calls
%ifdef WITHFAT32
                cmp     ax,4a33h                ; Check DOS version 7
                jne     Check4Share
                xor     ax,ax                   ; no undocumented shell strings
                iret
Check4Share:
%endif
                cmp     ah,10h                  ; SHARE.EXE interrupt?
                je      Int2f1                  ; yes, do installation check
                cmp     ah,08h
                je      DriverSysCal            ; DRIVER.SYS calls
                cmp     ah,14h                  ; NLSFUNC.EXE interrupt?
                jne     Int2f?iret              ; yes, do installation check
Int2f?14:      ;; MUX-14 -- NLSFUNC API
               ;; all functions are passed to syscall_MUX14
               push bp                 ; Preserve BP later on
               Protect386Registers
               PUSH$ALL
               SwitchToInt2fStack
               call _syscall_MUX14
               DoneInt2fStack
               pop bp                  ; Discard incoming AX
               push ax                 ; Correct stack for POP$ALL
               POP$ALL
               Restore386Registers
               mov bp, sp
               or ax, ax
               jnz Int2f?14?1          ; must return set carry
                   ;; -6 == -2 (CS), -2 (IP), -2 (flags)
                   ;; current SP = on old_BP
               and BYTE [bp-6], 0feh   ; clear carry as no error condition
               pop bp
               iret
Int2f?14?1:        or BYTE [bp-6], 1
               pop bp
Int2f?iret:
               iret

; DRIVER.SYS calls - now only 0803.
DriverSysCal:
                extern  _Dyn
                cmp     al, 3
                jne     Int2f?iret
                mov     ds, [cs:_DGROUP_]
                mov     di, _Dyn+2
                jmp     short Int2f?iret


;**********************************************************************
; internal dos calls INT2F/12xx and INT2F/4A01,4A02 - handled through C 
;**********************************************************************
IntDosCal:                
                        ; set up register structure
;struct int2f12regs
;{
;  [space for 386 regs]
;  UWORD es,ds;
;  UWORD di,si,bp,bx,dx,cx,ax;
;  UWORD ip,cs,flags;
;  UWORD callerARG1; 
;}      
    push ax
    push cx
    push dx
    push bx
    push bp
    push si
    push di
    push ds
    push es

    cld

%if XCPU >= 386
  %ifdef WATCOM
    mov si,fs
    mov di,gs 
  %else 
    Protect386Registers    
  %endif
%endif          

    SwitchToInt2fStack
    extern   _int2F_12_handler
    call _int2F_12_handler
    DoneInt2fStack

%if XCPU >= 386
  %ifdef WATCOM
    mov fs,si
    mov gs,di
  %else
    Restore386Registers    
  %endif
%endif      
    
    pop es
    pop ds
    pop di
    pop si
    pop bp
    pop bx
    pop dx
    pop cx
    pop ax
    
    iret

		global	SHARE_CHECK
SHARE_CHECK:
		mov	ax, 0x1000
		int	0x2f
		ret
           
;           DOS calls this to see if it's okay to open the file.
;           Returns a file_table entry number to use (>= 0) if okay
;           to open.  Otherwise returns < 0 and may generate a critical
;           error.  If < 0 is returned, it is the negated error return
;           code, so DOS simply negates this value and returns it in
;           AX.
; STATIC int share_open_check(char * filename,
;				/* pointer to fully qualified filename */
;                            unsigned short pspseg,
;				/* psp segment address of owner process */
;			     int openmode,
;				/* 0=read-only, 1=write-only, 2=read-write */
;			     int sharemode) /* SHARE_COMPAT, etc... */
		global SHARE_OPEN_CHECK
SHARE_OPEN_CHECK:
		mov	es, si		; save si
		pop	ax		; return address
		popargs	si,bx,cx,dx	; filename,pspseg,openmode,sharemode;
		push	ax		; return address
		mov	ax, 0x10a0
		int	0x2f	     	; returns ax
		mov	si, es		; restore si
		ret

;          DOS calls this to record the fact that it has successfully
;          closed a file, or the fact that the open for this file failed.
; STATIC void share_close_file(int fileno)  /* file_table entry number */

		global	SHARE_CLOSE_FILE
SHARE_CLOSE_FILE:
		pop	ax
		pop	bx
		push	ax
		mov	ax, 0x10a1
		int	0x2f
		ret

;          DOS calls this to determine whether it can access (read or
;          write) a specific section of a file.  We call it internally
;          from lock_unlock (only when locking) to see if any portion
;          of the requested region is already locked.  If pspseg is zero,
;          then it matches any pspseg in the lock table.  Otherwise, only
;          locks which DO NOT belong to pspseg will be considered.
;          Returns zero if okay to access or lock (no portion of the
;          region is already locked).  Otherwise returns non-zero and
;          generates a critical error (if allowcriter is non-zero).
;          If non-zero is returned, it is the negated return value for
;          the DOS call.
;STATIC int share_access_check(unsigned short pspseg,
;				/* psp segment address of owner process */
;                              int fileno,       /* file_table entry number */
;                              unsigned long ofs,        /* offset into file */
;                              unsigned long len,        /* length (in bytes) of region to access */
;                              int allowcriter)          /* allow a critical error to be generated */
		global SHARE_ACCESS_CHECK
SHARE_ACCESS_CHECK:
		mov	ax, 0x10a2
share_common:
		push	bp
		mov	bp, sp
		push	si
		push	di
arg pspseg, fileno, {ofs,4}, {len,4}, allowcriter
		mov	bx, [.pspseg] ; pspseg
		mov	cx, [.fileno] ; fileno
		mov	si, [.ofs+2] ; high word of ofs
		mov	di, [.ofs] ; low word of ofs
		les	dx, [.len] ; len
		or	ax, [.allowcriter] ; allowcriter/unlock
		int	0x2f
		pop	di
		pop	si
		pop	bp
		ret	14		; returns ax

;          DOS calls this to lock or unlock a specific section of a file.
;          Returns zero if successfully locked or unlocked.  Otherwise
;          returns non-zero.
;          If the return value is non-zero, it is the negated error
;          return code for the DOS 0x5c call. */
;STATIC int share_lock_unlock(unsigned short pspseg,     /* psp segment address of owner process */
;                             int fileno,        /* file_table entry number */
;                             unsigned long ofs, /* offset into file */
;                             unsigned long len, /* length (in bytes) of region to lock or unlock */
;                             int unlock)       /* one to unlock; zero to lock */
		global	SHARE_LOCK_UNLOCK
SHARE_LOCK_UNLOCK:
		mov	ax,0x10a4
		jmp	short share_common

; Int 2F Multipurpose Remote System Calls
;
; added by James Tabor jimtabor@infohwy.com
; changed by Bart Oldeman
;
; assume ss == ds after setup of stack in entry
; sumtimes return data *ptr is the push stack word
;

remote_lseek:   ; arg is a pointer to the long seek value
                mov     bx, cx
                mov     dx, [bx]
                mov     cx, [bx+2]
                ; "fall through"

remote_getfattr:        
                clc                    ; set to succeed
                int     2fh
                jc      ret_neg_ax
                jmp     short ret_int2f

remote_lock_unlock:
		mov	dx, cx   	; parameter block (dx) in arg
		mov	bx, cx
		mov	bl, [bx + 8]	; unlock or not
		mov	cx, 1
		int	0x2f
		jnc	ret_set_ax_to_carry
		mov	ah, 0
                jmp     short ret_neg_ax

;long ASMPASCAL network_redirector_mx(unsigned cmd, void far *s, void *arg)
                global NETWORK_REDIRECTOR_MX
NETWORK_REDIRECTOR_MX:
                pop     bx             ; ret address
                popargs ax,{es,dx},cx  ; cmd (ax), seg:off s
                                       ; stack value (arg); cx in remote_rw
                push    bx             ; ret address
call_int2f:
                push    bp
                push    si
                push    di
                cmp     al, 0fh
                je      remote_getfattr

                mov     di, dx         ; es:di -> s and dx is used for 1125!
                cmp     al, 08h
                je      remote_rw
                cmp     al, 09h
                je      remote_rw
                cmp     al, 0ah
                je      remote_lock_unlock
                cmp     al, 21h
                je      remote_lseek
                cmp     al, 22h
                je      remote_process_end
                cmp     al, 23h
                je      qremote_fn

                push    cx             ; arg
                cmp     al, 0ch
                je      remote_getfree
                cmp     al, 1eh
                je      remote_print_doredir
                cmp     al, 1fh
                je      remote_print_doredir

int2f_call:
                xor     cx, cx         ; set to succeed; clear carry and CX
                int     2fh
                pop     bx
                jnc     ret_set_ax_to_cx
ret_neg_ax:
                neg     ax
ret_int2f:
                pop     di
                pop     si
                pop     bp
                ret

ret_set_ax_to_cx:                      ; ext_open or rw -> status from CX in AX
                                       ; otherwise CX was set to zero above
                xchg    ax, cx         ; set ax:=cx (one byte shorter than mov)
                jmp     short ret_int2f

remote_print_doredir:                  ; di points to an lregs structure
                mov     es,[di+0xe]
                mov     bx,[di+2]
                mov     cx,[di+4]
                mov     dx,[di+6]
                mov     si,[di+8]
                lds     di,[di+0xa]

                clc                     ; set to succeed
                int     2fh
                pop     bx              ; restore stack and ds=ss
                push    ss
                pop     ds
                jc      ret_neg_ax
ret_set_ax_to_carry:                    ; carry => -1 else 0 (SUCCESS)
                sbb     ax, ax
                jmp     short ret_int2f

remote_getfree:
                clc                     ; set to succeed
                int     2fh
                pop     di              ; retrieve pushed pointer arg
                jc      ret_set_ax_to_carry
                mov     [di],ax
                mov     [di+2],bx
                mov     [di+4],cx
                mov     [di+6],dx
                jmp     short ret_set_ax_to_carry

remote_rw:
                clc                    ; set to succeed
                int     2fh
                jc      ret_min_dx_ax
                xor     dx, dx         ; dx:ax := dx:cx = bytes read
                jmp     short ret_set_ax_to_cx
ret_min_dx_ax:  neg     ax
                cwd
                jmp     short ret_int2f
                
qremote_fn:
                mov     bx, cx
                lds     si, [bx]
                jmp     short int2f_restore_ds

remote_process_end:                   ; Terminate process
                mov     ds, [_cu_psp]
int2f_restore_ds:
                clc
                int     2fh
                push    ss
                pop     ds
                jmp     short ret_set_ax_to_carry

; extern UWORD ASMPASCAL call_nls(UWORD bp, UWORD FAR *buf,
;	UWORD subfct, UWORD cp, UWORD cntry, UWORD bufsize);

		extern _nlsInfo
		global CALL_NLS
CALL_NLS:
		pop	es		; ret addr
		pop	cx		; bufsize
		pop	dx		; cntry
		pop	bx		; cp
		pop	ax		; sub fct
		mov	ah, 0x14
		push	es		; ret addr
		push	bp
		mov	bp, sp
		push	si
		push	di
		mov	si, _nlsInfo	; nlsinfo
		les	di, [bp + 4]	; buf
		mov	bp, [bp + 8]	; bp
		int	0x2f
		mov	dx, bx          ; return id in high word
		pop	di
		pop	si
		pop	bp
		ret	6

; extern UWORD ASMPASCAL floppy_change(UWORD drives)

		global FLOPPY_CHANGE
FLOPPY_CHANGE:
		pop	cx		; ret addr
		pop	dx		; drives
		push	cx		; ret addr
		mov	ax, 0x4a00
		xor	cx, cx
		int	0x2f
		mov	ax, cx		; return
		ret

;
; Test to see if a umb driver has been loaded.
; if so, retrieve largest available block+size
;
; From RB list and Dosemu xms.c.
;
; Call the XMS driver "Request upper memory block" function with:
;     AH = 10h
;     DX = size of block in paragraphs
; Return: AX = status
;         0001h success
;         BX = segment address of UMB
;         DX = actual size of block
;         0000h failure
;         BL = error code (80h,B0h,B1h) (see #02775)
;         DX = largest available block
;
; (Table 02775)
; Values for XMS error code returned in BL:
;  00h    successful
;  80h    function not implemented
;  B0h    only a smaller UMB is available
;  B1h    no UMBs are available
;  B2h    UMB segment number is invalid
;

segment INIT_TEXT
                ; int ASMPASCAL UMB_get_largest(void FAR * driverAddress,
                ;                UCOUNT * seg, UCOUNT * size);
arg {driverAddress,4}, argseg, size
		global UMB_GET_LARGEST
                
UMB_GET_LARGEST:
                push    bp
                mov     bp,sp

                mov     dx,0xffff       ; go for broke!
                mov     ax,1000h        ; get the UMBs
                call    far [.driverAddress] ; Call the driver

;
;       bl = 0xB0 and  ax = 0 so do it again.
;
                cmp     bl,0xb0         ; fail safe
                jne     umbt_error

                and     dx,dx           ; if it returns a size of zero.
                je      umbt_error

                mov     ax,1000h        ; dx set with largest size
                call    far [.driverAddress] ; Call the driver

                cmp     ax,1
                jne     umbt_error
                                        ; now return the segment
                                        ; and the size

                mov 	cx,bx           ; *seg = segment
                mov 	bx, [.argseg]
                mov 	[bx],cx

                mov 	bx, [.size]      ; *size = size
                mov 	[bx],dx

umbt_ret:
                pop     bp
                ret     8           	; this was called NEAR!!

umbt_error:     xor 	ax,ax
                jmp 	short umbt_ret
