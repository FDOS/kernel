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
; $Logfile:   D:/dos-c/src/kernel/int2f.asv  $
;
; $Id$
;
; $Log$
; Revision 1.14  2001/11/04 19:47:39  bartoldeman
; kernel 2025a changes: see history.txt
;
; Revision 1.13  2001/09/23 20:39:44  bartoldeman
; FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
;
; Revision 1.12  2001/08/19 12:58:36  bartoldeman
; Time and date fixes, Ctrl-S/P, findfirst/next, FCBs, buffers, tsr unloading
;
; Revision 1.11  2001/07/28 18:13:06  bartoldeman
; Fixes for FORMAT+SYS, FATFS, get current dir, kernel init memory situation.
;
; Revision 1.10  2001/07/22 01:58:58  bartoldeman
; Support for Brian's FORMAT, DJGPP libc compilation, cleanups, MSCDEX
;
; Revision 1.9  2001/07/09 22:19:33  bartoldeman
; LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
;
; Revision 1.8  2001/04/02 23:18:30  bartoldeman
; Misc, zero terminated device names and redirector bugs fixed.
;
; Revision 1.7  2001/03/21 02:56:26  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
; Revision 1.6  2000/08/06 05:50:17  jimtabor
; Add new files and update cvs with patches and changes
;
; Revision 1.5  2000/06/21 18:16:46  jimtabor
; Add UMB code, patch, and code fixes
;
; Revision 1.4  2000/05/25 20:56:21  jimtabor
; Fixed project history
;
; Revision 1.3  2000/05/17 19:15:12  jimtabor
; Cleanup, add and fix source.
;
; Revision 1.2  2000/05/08 04:30:00  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.4  2000/03/31 05:40:09  jtabor
; Added Eric W. Biederman Patches
;
; Revision 1.3  2000/03/09 06:07:11  kernel
; 2017f updates by James Tabor
;
; Revision 1.2  1999/08/10 17:57:12  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:40:59  jprice
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
;    Rev 1.2   06 Dec 1998  8:48:12   patv
; Bug fixes.
;
;    Rev 1.1   29 May 1996 21:03:46   patv
; bug fixes for v0.91a
;
;    Rev 1.0   19 Feb 1996  3:34:38   patv
; Initial revision.
; $EndLog$
;

		%include "segs.inc"
        %include "stacks.inc"

segment	HMA_TEXT
            extern  _cu_psp:wrt DGROUP
            extern _syscall_MUX14:wrt HMA_TEXT

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
                cmp     ah,10h                  ; SHARE.EXE interrupt?
                je      Int2f1                  ; yes, do installation check
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



;***********************************************************
; internal doscalls INT2F/11xx - handled through C 
;***********************************************************
IntDosCal:                
                        ; set up register frame
;struct int2f12regs
;{
;  UWORD es,ds;
;  UWORD di,si,bp,bx,dx,cx,ax;
;  UWORD ip,cs,flags;
;  UWORD callerARG1; 
;};
    push ax
    push cx
    push dx
    push bx
    push bp
    push si
    push di
    push ds
    push es

    mov ax,DGROUP
    mov ds,ax    
                extern   _int2F_12_handler:wrt HGROUP
    call _int2F_12_handler
    
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
                
call_int2f:
                mov     ah, 11h
                push    bp
                mov     bp,sp
                push    es
                push    si
                push    di
                push    dx
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
                cmp     al, 21h        ; 21h, Lseek from eof
                je      lseekeof
                cmp     al, 23h
                je      qremote_fn
                cmp     al, 25h
                je      remote_printredir

                les     di, [bp+4]
                cmp     al, 08h
                je      remote_rw
                cmp     al, 09h
                je      remote_rw                
                cmp     al, 0ch
                je      remote_getfree
        
int2f_call_push:                
                push    word [bp+8]    ; very fakey, HaHa ;)
int2f_call:
                stc                     ; set to fail
                int     2fh
                pop     bx
                jc      no_clear_ax
clear_ax:       
                xor     ax,ax
no_clear_ax:
                neg     ax
no_neg_ax:              
                pop     bx
                pop     cx
                pop     dx
                pop     di
                pop     si
                pop     es
                pop     bp
                ret

lseekeof:              
                mov     dx, [bp+8]
                mov     cx, [bp+10]
                jmp     int2f_call_push
        
remote_getfattr:        
                stc                     ; set to fail
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
                stc                     ; set to fail
                int     2fh
                pop     bx
                pop     ds
                jc      no_clear_ax
                jmp     short clear_ax

remote_getfree:
                stc                     ; set to fail
                int     2fh
                jc      no_clear_ax
                mov     di,[bp+8]
                mov     [di],ax
                mov     [di+2],bx
                mov     [di+4],cx
                mov     [di+6],dx
                jmp     short clear_ax

remote_printredir:       
                mov     dx, [bp+4]
                push    word [bp+6]
                jmp     short int2f_call
                     
qremote_fn:     
                lds     si,[bp+4]
                les     di,[bp+8]
                stc
                int     2fh
                mov     ax,0xffff
                jc      no_neg_ax
                jmp     short clear_ax

remote_rw:      mov     cx, [bp+8]
                stc                     ; set to fail
                int     2fh
                jc      int2f_carry
                xor     ax, ax
int2f_carry:    neg     ax
                mov     di, [bp+10]
                mov     [di], ax
                mov     ax, cx
                jmp     short no_neg_ax
                
                global  _remote_process_end
_remote_process_end:                     ; Terminate process
                mov     ds, [_cu_psp] 
                mov     al, 22h
                call    call_int2f
                push    ss
                pop     ds
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

                mov cx,bx                       ; *seg = segment
                mov bx, [bp+4]
                mov [bx],cx

                mov bx, [bp+6]                  ; *size = size
                mov [bx],dx

umbt_ret:
                mov     sp,bp
                pop     bp
                ret                ; this was called NEAR!!

umbt_error:     xor ax,ax
                jmp umbt_ret
