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
; $Id$
;

		%include "segs.inc"
        %include "stacks.inc"

segment	HMA_TEXT
            extern  _cu_psp:wrt DGROUP
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
Int2f3:
                cmp     ah,16h
                je      FarTabRetn              ; Win Hook return fast
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
               PUSH$ALL
               call _syscall_MUX14
               pop bp                  ; Discard incoming AX
               push ax                 ; Correct stack for POP$ALL
               POP$ALL
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
                extern  _Dyn:wrt DGROUP
                cmp     al, 3
                jne     Int2f?iret
                mov     ds, [cs:_DGROUP_]
                mov     di, _Dyn+2
                jmp     short Int2f?iret


;***********************************************************
; internal doscalls INT2F/11xx - handled through C 
;***********************************************************
IntDosCal:                
                        ; set up register frame
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

%IFDEF I386
  %ifdef WATCOM
    mov si,fs
    mov di,gs 
  %else 
    Protect386Registers    
  %endif
%endif          

    mov ds,[cs:_DGROUP_]
    extern   _int2F_12_handler
    call _int2F_12_handler

%IFDEF I386
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
		pop	dx		; sharemode;
		pop	cx		; openmode;
		pop	bx		; pspseg;
		pop	si		; filename
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
		mov	bx, [bp + 16]	; pspseg
		mov	cx, [bp + 14]	; fileno
		mov	si, [bp + 12]	; high word of ofs
		mov	di, [bp + 10]	; low word of ofs
		les	dx, [bp + 6]	; len
		or	ax, [bp + 4]	; allowcriter/unlock
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

                global  _remote_rmdir
_remote_rmdir:  
                mov     al, 01h
                jmp     short call_int2f

                global  _remote_mkdir
_remote_mkdir:  
                mov     al, 03h
                jmp     short call_int2f

                global  _remote_chdir
_remote_chdir:  
                mov     al, 05h
                jmp     short call_int2f

                global  _remote_close
_remote_close: 
                mov     al, 06h
                jmp     short call_int2f

                global  _remote_commit
_remote_commit: 
                mov     al, 07h
                jmp     short call_int2f

                global  _remote_read
_remote_read:   mov     al, 08h
                jmp     short call_int2f
        
                global  _remote_write        
_remote_write:  mov     al, 09h
                jmp     short call_int2f
        
                global  _remote_getfree
_remote_getfree:
                mov     al, 0ch
                jmp     short call_int2f

                global  _remote_setfattr
_remote_setfattr: 
                mov     al, 0eh
                jmp     short call_int2f

                global  _remote_getfattr
_remote_getfattr:
                mov     al, 0fh
                jmp     short call_int2f
                
                global  _remote_rename
_remote_rename: 
                mov     al, 11h
                jmp     short call_int2f

                global  _remote_delete
_remote_delete: 
                mov     al, 13h
                jmp     short call_int2f

                global  _remote_open
_remote_open: 
                mov     al, 16h
                jmp     short call_int2f

                global  _remote_creat
_remote_creat: 
                mov     al, 17h
                jmp     short call_int2f

                global  _remote_findfirst
_remote_findfirst: 
                mov     al, 1bh
                jmp     short call_int2f

                global  _remote_findnext
_remote_findnext: 
                mov     al, 1ch
                jmp     short call_int2f

                global  _remote_close_all
_remote_close_all: 
                mov     al, 1dh
                jmp     short call_int2f

                global  _remote_doredirect
_remote_doredirect:
                mov     al, 1eh
                jmp     short call_int2f

                global  _remote_printset
_remote_printset:
                mov     al, 1fh
                jmp     short call_int2f

                global  _remote_flushall
_remote_flushall: 
                mov     al, 20h
                jmp     short call_int2f

                global  _remote_lseek
_remote_lseek: 
                mov     al, 21h
                jmp     short call_int2f

                global  _QRemote_Fn
_QRemote_Fn
                mov     al, 23h
                jmp     short call_int2f
        
                global  _remote_printredir
_remote_printredir:
                mov     al, 25h
                jmp     short call_int2f

                global  _remote_extopen
_remote_extopen:
                mov     al, 2eh
                
call_int2f:
                mov     ah, 11h
                push    bp
                mov     bp,sp
                push    es
                push    si
                push    di
                push    cx
                push    bx

                cmp     al, 0eh
                je      remote_setfattr
                cmp     al, 0fh
                je      remote_getfattr
                cmp     al, 1eh
                je      print_doredir
                cmp     al, 1fh
                je      print_doredir
                cmp     al, 25h
                je      remote_printredir

                les     di, [bp+4]
                cmp     al, 08h
                je      remote_rw
                cmp     al, 09h
                je      remote_rw                
                cmp     al, 0ch
                je      remote_getfree
                cmp     al, 21h        ; 21h, Lseek from eof
                je      lseekeof
                cmp     al, 23h
                je      qremote_fn
        
int2f_call_push:                
                push    word [bp+8]    ; very fakey, HaHa ;)
int2f_call:
                xor     cx, cx         ; set to succeed; clear carry and CX
                int     2fh
                pop     bx
                jnc     clear_ax
no_clear_ax:
                neg     ax
                xchg    cx, ax
clear_ax:       
                xchg    ax, cx         ; extended open -> status from CX in AX
                                       ; otherwise CX was set to zero above
no_neg_ax:
                pop     bx
                pop     cx
                pop     di
                pop     si
                pop     es
                pop     bp
                ret

lseekeof:              
                mov     dx, [bp+8]
                mov     cx, [bp+10]
                ; "fall through"

remote_getfattr:        
                clc                     ; set to succeed
                int     2fh
                jc      no_clear_ax
                jmp     short no_neg_ax

remote_setfattr:       
                push    word [bp+4]        
                jmp     short int2f_call

print_doredir:  
                push    ds
                mov     si,[bp+14]
                les     di,[bp+10]
                mov     dx,[bp+8]
                mov     cx,[bp+6]
                mov     bx,[bp+4]

                mov     ds, [bp+18]
                push    word [bp+16]    ; very fakey, HaHa ;)
                clc                     ; set to succeed
                int     2fh
                pop     bx
                pop     ds
                jc      no_clear_ax
                xor     cx, cx
                jmp     short clear_ax

remote_getfree:
                clc                     ; set to succeed
                int     2fh
                jc      no_clear_ax
                mov     di,[bp+8]
                mov     [di],ax
                mov     [di+2],bx
                mov     [di+4],cx
                mov     [di+6],dx
                xor     cx, cx
                jmp     short clear_ax

remote_printredir:       
                mov     dx, [bp+4]
                push    word [bp+6]
                jmp     short int2f_call

remote_rw:      jmp     short remote_rw1
                     
qremote_fn:
                push    ds
                lds     si,[bp+8]
                clc
                int     2fh
                pop     ds
                mov     ax,0xffff
                jc      no_neg_ax
                xor     cx, cx
                jmp     short clear_ax

remote_rw1:     mov     cx, [bp+8]
                clc                     ; set to succeed
                int     2fh
                jc      int2f_carry
                xor     ax, ax
int2f_carry:    neg     ax
                mov     di, [bp+10]
                mov     [di], ax
                mov     ax, cx
                jmp     no_neg_ax
                
                global  _remote_process_end
_remote_process_end:                     ; Terminate process
                mov     ds, [_cu_psp] 
                mov     al, 22h
                call    call_int2f
                push    ss
                pop     ds
                ret

;STATIC int ASMCFUNC remote_lock_unlock(sft FAR *sftp,     /* SFT for file */
;                             unsigned long ofs, /* offset into file */
;                             unsigned long len, /* length (in bytes) of region to lock or unlock */
;                            int unlock)
;                               one to unlock; zero to lock
		global _remote_lock_unlock
_remote_lock_unlock:
		push	bp
		mov	bp, sp
		push	di
		les	di, [bp + 4]	; sftp
		lea	dx, [bp + 8]	; parameter block on the stack!
		mov	bl, [bp + 16]	; unlock
		mov	ax, 0x110a
		mov	cx, 1
		int	0x2f
		mov	ah, 0
		jc	lock_error
		mov	al, 0
lock_error:
		neg	al
		pop	di
		pop	bp
		ret

; extern UWORD ASMCFUNC call_nls(UWORD subfct, struct nlsInfoBlock *nlsinfo,
; UWORD bp, UWORD cp, UWORD cntry, UWORD bufsize, UWORD FAR *buf, UWORD *id);

		global _call_nls
_call_nls:
		push	bp
		mov	bp, sp
		push	si
		mov	al, [bp + 4]	; subfct
		mov	ah, 0x14
		mov	si, [bp + 6]	; nlsinfo
		mov	bx, [bp + 10]	; cp
		mov	dx, [bp + 12]	; cntry
		mov	cx, [bp + 14]	; bufsize
		les	di, [bp + 16]	; buf
		push	bp
		mov	bp, [bp + 8]	; bp
		int	0x2f
		pop	bp
		mov	bp, [bp + 20]	; store id (in SS:) unless it's NULL
		or	bp, bp
		jz	nostore
		mov	[bp], bx
nostore:
		pop	si
		pop	bp
		ret

%if 0
; int_2f_111e_call(iregs FAR *iregs)
; 
; set up all registers to the int21 entry registers
; call int2f/111e
; copy returned registers into int21 entry registers back
;
;  disabled: does not work better than previous implementation
                global  _int_2f_111e_call
_int_2f_111e_call:

    push bp
    mov  bp,sp
    push si
    push di
    push ds

    lds  si, [bp+4]     ; ds:si -> iregs
    
    mov  ax, [si  ]
    mov  bx, [si+2]
    mov  cx, [si+4]
    mov  dx, [si+6]
    push word [si+8]            ; si
    mov  di, [si+10]
    mov  bp, [si+12]
    mov  es, [si+16]
    mov  ds, [si+14]
    pop  si

    push ax
    mov  ax, 111eh
    int  2fh
    jc   fault
    pop  ax                     ; restore orig value of ax if no errors
    push ax    
fault:        

    pushf
    push ds
    push si
    push bp
    
    mov bp,sp
    lds si,[bp+4+6+10]        ; 4=fun, 6=si,di,ds, 10 additional bytes on stack
    
    pop word [si+12]          ; bp
    pop word [si+ 8]          ; si
    pop word [si+14]          ; ds
    pop word [si+22]          ; flags
    add sp,2                ; pushed function value

    mov [si  ],ax

    cmp ax, 5f02h             ; 5f02 is special: it manipulates the user stack directly
    je  skip5f02    
    mov [si+2],bx
    mov [si+4],cx
skip5f02:
        
    mov [si+6],dx
    mov [si+10],di
    mov [si+16],es
    
    pop ds
    pop di
    pop si
    pop bp
    ret        
%endif
          
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
;  B1h    no UMB's are available
;  B2h    UMB segment number is invalid
;
;


segment INIT_TEXT
                ; int UMB_get_largest(UCOUNT *seg, UCOUNT *size);
                global _UMB_get_largest
                
_UMB_get_largest:
                push    bp
                mov     bp,sp

                sub     sp,4            ; for the far call

                mov     ax,4300h        ; is there a xms driver installed?
                int     2fh
                cmp     al,80h
                jne     umbt_error

                mov     ax,4310h
                int     2fh


                mov     [bp-2],es              ; save driver entry point
                mov     [bp-4],bx

                mov     dx,0xffff       ; go for broke!
                mov     ax,1000h        ; get the umb's
                call    far [bp-4]      ; Call the driver
;
;       bl = 0xB0 and  ax = 0 so do it again.
;
                cmp     bl,0xb0         ; fail safe
                jne     umbt_error

                and     dx,dx           ; if it returns a size of zero.
                je      umbt_error

                mov     ax,1000h        ; dx set with largest size
                call    far [bp-4]      ; Call the driver

                cmp     ax,1
                jne     umbt_error
                                        ; now return the segment
                                        ; and the size

                mov 	cx,bx           ; *seg = segment
                mov 	bx, [bp+4]
                mov 	[bx],cx

                mov 	bx, [bp+6]      ; *size = size
                mov 	[bx],dx

umbt_ret:
                mov     sp,bp
                pop     bp
                ret                	; this was called NEAR!!

umbt_error:     xor 	ax,ax
                jmp 	short umbt_ret
