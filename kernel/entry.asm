;
; File:
;                          entry.asm
; Description:
;                      System call entry code
;
;                       Copyright (c) 1998
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
; $Log$
; Revision 1.10  2001/04/02 23:18:30  bartoldeman
; Misc, zero terminated device names and redirector bugs fixed.
;
; Revision 1.9  2001/03/30 19:30:06  bartoldeman
; Misc fixes and implementation of SHELLHIGH. See history.txt for details.
;
; Revision 1.8  2001/03/27 20:27:43  bartoldeman
; dsk.c (reported by Nagy Daniel), inthndlr and int25/26 fixes by Tom Ehlert.
;
; Revision 1.6  2001/03/24 22:13:05  bartoldeman
; See history.txt: dsk.c changes, warning removal and int21 entry handling.
;
; Revision 1.4  2001/03/21 02:56:25  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
; Revision 1.3  2000/05/25 20:56:21  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/08 04:29:59  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.5  2000/03/20 03:15:49  kernel
; Change in Entry.asm
;
; Revision 1.4  1999/09/23 04:40:46  jprice
; *** empty log message ***
;
; Revision 1.2  1999/08/10 17:57:12  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:40:53  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.3  1999/02/01 01:48:41  jprice
; Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
;
; Revision 1.2  1999/01/22 04:13:25  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
; Imported sources
;
;     Rev 1.1   06 Dec 1998  8:48:40   patv
;  New int 21h handler code.
;
;     Rev 1.0   07 Feb 1998 20:42:08   patv
;  Modified stack frame to match DOS standard
; $EndLog$

		%include "segs.inc"
                %include "stacks.inc"

segment	HMA_TEXT
                extern   _int21_syscall:wrt HGROUP
                extern   _int21_service:wrt HGROUP
                extern   _int2526_handler:wrt HGROUP
                extern   _set_stack:wrt HGROUP
                extern   _restore_stack:wrt HGROUP
                extern   _error_tos:wrt DGROUP
                extern   _char_api_tos:wrt DGROUP
                extern   _disk_api_tos:wrt DGROUP
                extern   _lpUserStack:wrt DGROUP
                extern   _user_r:wrt DGROUP
                extern   _ErrorMode:wrt DGROUP
                extern   _InDOS:wrt DGROUP
                extern   _cu_psp:wrt DGROUP
                extern   _MachineId:wrt DGROUP
                extern   critical_sp:wrt DGROUP

                extern   _api_sp:wrt DGROUP      ; api stacks - for context
                extern   _api_ss:wrt DGROUP      ; switching
                extern   _usr_sp:wrt DGROUP      ; user stacks
                extern   _usr_ss:wrt DGROUP
                extern   int21regs_seg:wrt DGROUP
                extern   int21regs_off:wrt DGROUP

                extern   _dosidle_flag:wrt DGROUP
                extern   _Int21AX:wrt DGROUP


                global  reloc_call_cpm_entry
                global  reloc_call_int20_handler
                global  reloc_call_int21_handler
                global  reloc_call_low_int25_handler
                global  reloc_call_low_int26_handler
                global  reloc_call_int27_handler


;
; MS-DOS CP/M style entry point
;
;       VOID FAR
;       cpm_entry(iregs UserRegs)
;
; This one is a strange one.  The call is to psp:0005h but it returns to the
; function after the call.  What we do is convert it to a normal call and
; fudge the stack to look like an int 21h call.
;
reloc_call_cpm_entry:
                ; Stack is:
                ;       return offset
                ;       psp seg
                ;       000ah
                ;
                push    bp              ; trash old return address
                mov     bp,sp
                xchg    bp,[2+bp]
                pop     bp
                pushf                   ; start setting up int 21h stack
                ;
                ; now stack is
                ;       return offset
                ;       psp seg
                ;       flags
                ;
                push    bp
                mov     bp,sp           ; set up reference frame
                ;
                ; reference frame stack is
                ;       return offset           bp + 6
                ;       psp seg                 bp + 4
                ;       flags                   bp + 2
                ;       bp              <---    bp
                ;
                push    ax
                mov     ax,[2+bp]        ; get the flags
                xchg    ax,[6+bp]        ; swap with return address
                mov     [2+bp],ax
                pop     ax              ; restore working registers
                pop     bp
                ;
                ; Done. Stack is
                ;       flags
                ;       psp seg (alias .COM cs)
                ;       return offset
                ;
                cmp     cl,024h
                jbe     cpm_error
                mov     ah,cl           ; get the call # from cl to ah
                jmp     short reloc_call_int21_handler    ; do the system call
cpm_error:      mov     al,0
                iret

;
; Restart the int 21h system call.  Call never returns.
;
;       VOID
;       RestartSysCall(VOID);
;
; NOTE: On exit, DS must point to kernel stack, SS:SP user stack after
; PUSH$ALL and BP == SP.
;
_RestartSysCall:
                cli                     ; no interrupts
                mov     bp,word [_lpUserStack+2] ;Get frame
                mov     ss,bp
                mov     bp,word [_lpUserStack]
                mov     sp,bp
                sti
                POP$ALL                 ; get the original regs
                jmp     short int21_reentry     ; restart the system call


;
; interrupt zero divide handler:
; print a message 'Interrupt divide by zero'
; Terminate the current process
;
;       VOID INRPT far
;       int20_handler(iregs UserRegs)
;

divide_by_zero_message db 0dh,0ah,'Interrupt divide by zero',0dh,0ah,0

                global reloc_call_int0_handler
reloc_call_int0_handler:
                
                mov si,divide_by_zero_message
                
zero_message_loop:
                mov al, [cs:si]
                test al,al
                je   zero_done

                inc si
                mov bx, 0070h
                mov ah, 0eh
                
                int  10h
                
                jmp short zero_message_loop
                
zero_done:
                mov ax,04c7fh       ; terminate with errorlevel 127                                                
                int 21h

;
; Terminate the current process
;
;       VOID INRPT far
;       int20_handler(iregs UserRegs)
;
reloc_call_int20_handler:
                mov     ah,0            ; terminate through int 21h


;
; MS-DOS system call entry point
;
;       VOID INRPT far
;       int21_handler(iregs UserRegs)
;
reloc_call_int21_handler:
                ;
                ; Create the stack frame for C call.  This is done to
                ; preserve machine state and provide a C structure for
                ; access to registers.
                ;
                ; Since this is an interrupt routine, CS, IP and flags were
                ; pushed onto the stack by the processor, completing the
                ; stack frame.
                ;
                ; NB: stack frame is MS-DOS dependent and not compatible
                ; with compiler interrupt stack frames.
                ;
                sti
                PUSH$ALL

                ;
                ; Create kernel refernce frame.
                ;
                ; NB: At this point, SS != DS and won't be set that way
                ; until later when which stack to run on is determined.
                ;
                mov     dx,DGROUP
                mov     ds,dx

int21_reentry:
                cmp     ah,33h
                je      int21_user
                cmp     ah,50h
                je      int21_user
                cmp     ah,51h
                je      int21_user
                cmp     ah,62h
                jne     int21_1

int21_user:     
                call    dos_crit_sect
                mov     bp,sp
                push    ss
                push    bp
                call    _int21_syscall
                pop     cx
                pop     cx
                jmp     int21_ret

;
; normal entry, use one of our 4 stacks
; 
; DX=DGROUP
; CX=STACK
; SI=userSS
; BX=userSP


int21_1:
                mov si,ss   ; save user stack, to be retored later
                mov bx,sp


                ;
                ; Now DS is set, let's save our stack for rentry (???TE)
                ;
                ; I don't know who needs that, but ... (TE)
                ;
                mov     word [_lpUserStack+2],ss
                mov     word [_user_r+2],ss
                mov     word [_lpUserStack],sp        ; store and init
                mov     word [_user_r],sp     ; store and init

                ;
                ; Decide which stack to run on.
                ;
                ; Unlike previous versions of DOS-C, we need to do this here
                ; to guarantee the user stack for critical error handling.
                ; We need to do the int 24h from this stack location.
                ;
                ; There are actually four stacks to run on. The first is the
                ; user stack which is determined by system call number in
                ; AH.  The next is the error stack determined by _ErrorMode.
                ; Then there's the character stack also determined by system
                ; call number.  Finally, all others run on the disk stack.
                ; They are evaluated in that order.


                cmp     byte [_InDOS],0
                jne     int21_onerrorstack

                cmp     byte [_ErrorMode],0
                je      int21_2

int21_onerrorstack:                
                mov     cx,_error_tos
                

                cli
                mov     ss,dx
                mov     sp,cx
                sti
                
                push    si  ; user SS:SP
                push    bx
                
                call    _int21_service
                jmp     short int21_exit_nodec
                

int21_2:        inc     byte [_InDOS]
                mov     cx,_char_api_tos
                or      ah,ah   
                jz      int21_3
                cmp     ah,0ch
                jle     int21_normalentry
                cmp     ah,59h
                je      int21_normalentry

int21_3:
                call    dos_crit_sect
                mov     cx,_disk_api_tos

int21_normalentry:

                cli
                mov     ss,dx
                mov     sp,cx
                sti

                ;
                ; Push the far pointer to the register frame for
                ; int21_syscall and remainder of kernel.
                ;
                
                push    si  ; user SS:SP
                push    bx
                call    _int21_service

int21_exit:     dec     byte [_InDOS]

                ;
                ; Recover registers from system call.  Registers and flags
                ; were modified by the system call.
                ;

                
int21_exit_nodec:
                pop bx      ; get back user stack
                pop si

                cli
                mov     ss,si
                mov     sp,bx
                sti
int21_ret:      POP$ALL

                ;
                ; ... and return.
                ;
                iret
;
;   end Dos Critical Section 0 thur 7
;
;
dos_crit_sect:
                mov     [_Int21AX],ax       ; needed!
                push    ax                  ; This must be here!!!
                mov     ah,82h              ; re-enrty sake before disk stack
                int     2ah                 ; Calling Server Hook!
                pop     ax
                ret

;
; Terminate the current process
;
;       VOID INRPT far
;       int27_handler(iregs UserRegs)
;
reloc_call_int27_handler:
                ;
                ; First convert the memory to paragraphs
                ;
                add     dx,byte 0fh     ; round up
                rcr     dx,1
                shr     dx,1
                shr     dx,1
                shr     dx,1
                ;
                ; ... then use the standard system call
                ;
                mov     ax,3100h
                jmp     reloc_call_int21_handler  ; terminate through int 21h

;
;

reloc_call_low_int26_handler:
                sti
                pushf
                push    ax
                mov     ax,026h
                jmp     int2526
reloc_call_low_int25_handler:
                sti
                pushf
                push    ax
                mov     ax,025h
int2526:                
                push    cx
                push    dx
                push    bx
                push    sp
                push    bp
                push    si
                push    di
                push    ds
                push    es

               
                mov     cx, sp     ; save stack frame
                mov     dx, ss

                cld
                mov     bx, DGROUP
                mov     ds, bx

                ; save away foreground process' stack
                push    word [_usr_ss]
                push    word [_usr_sp]

                mov     word [_usr_ss],ss
                mov     word [_usr_sp],sp

                ; setup our local stack
                cli
                mov     ss,bx
                mov     sp,_disk_api_tos
                sti

                push    dx                      ; SS:SP -> user stack
                push    cx
                push    ax                      ; was set on entry = 25,26
                call    _int2526_handler
                add     sp, byte 4


                ; restore foreground stack here
                cli
                mov     ss,word [_usr_ss]
                mov     sp,word [_usr_sp]

                pop     word [_usr_sp]
                pop     word [_usr_ss]

                pop     es
                pop     ds
                pop     di
                pop     si
                pop     bp
                pop     bx      ; pop off sp value
                pop     bx
                pop     dx
                pop     cx
                pop     ax
                popf
                retf            ; Bug-compatiblity with MS-DOS.
                                ; This function is supposed to leave the original
                                ; flag image on the stack.



CONTINUE        equ     00h
RETRY           equ     01h
ABORT           equ     02h
FAIL            equ     03h

OK_IGNORE       equ     20h
OK_RETRY        equ     10h
OK_FAIL         equ     08h

PSP_PARENT      equ     16h
PSP_USERSP      equ     2eh
PSP_USERSS      equ     30h



;
; COUNT
; CriticalError(COUNT nFlag, COUNT nDrive, COUNT nError, struct dhdr FAR *lpDevice);
;
                global  _CriticalError
_CriticalError:
                ;
                ; Skip critical error routine if handler is active
                ;
                cmp     byte [_ErrorMode],0
                je      CritErr05               ; Jump if equal

                mov     ax,FAIL
                retn
                ;
                ; Do local error processing
                ;
CritErr05:
                ;
                ; C Entry
                ;
                push    bp
                mov     bp,sp
                push    si
                push    di
                ;
                ; Get parameters
                ;
                mov     ah,byte [bp+4]      ; nFlags
                mov     al,byte [bp+6]      ; nDrive
                mov     di,word [bp+8]      ; nError
                ;
                ;       make bp:si point to dev header
                ;
                mov     si,word [bp+10]     ; lpDevice Offset
                mov     bp,word [bp+12]     ; lpDevice segment
                ;
                ; Now save real ss:sp and retry info in internal stack
                ;
                cli
                mov     es,[_cu_psp]
                push    word [es:PSP_USERSS]
                push    word [es:PSP_USERSP]
                push    word [_MachineId]
                push    word [int21regs_seg]
                push    word [int21regs_off]
                push    word [_api_sp]
                push    word [_api_ss]
                push    word [_usr_sp]
                push    word [_usr_ss]
                push    word [_user_r+2]
                push    word [_user_r]
                mov     [critical_sp],sp
                ;
                ; do some clean up because user may never return
                ;
                inc     byte [_ErrorMode]
                dec     byte [_InDOS]
                ;
                ; switch to user's stack
                ;
                mov     ss,[es:PSP_USERSS]
                mov     sp,[es:PSP_USERSP]
                ;
                ; and call critical error handler
                ;
                int     24h                     ; DOS Critical error handler

                ;
                ; recover context
                ;
                cld
                cli
                mov     bp, DGROUP
                mov     ds,bp
                mov     ss,bp
                mov     sp,[critical_sp]
                pop     word [_user_r]
                pop     word [_user_r+2]
                pop     word [_usr_ss]
                pop     word [_usr_sp]
                pop     word [_api_ss]
                pop     word [_api_sp]
                pop     word [int21regs_off]
                pop     word [int21regs_seg]
                pop     word [_MachineId]
                mov     es,[_cu_psp]
                pop     word [es:PSP_USERSP]
                pop     word [es:PSP_USERSS]
                sti                             ; Enable interrupts
                ;
                ; clear flags
                ;
                mov     byte [_ErrorMode],0
                inc     byte [_InDOS]
                ;
                ; Check for ignore and force fail if not ok
                cmp     al,CONTINUE
                jne     CritErr10               ; not ignore, keep testing
                test    bh,OK_IGNORE
                jnz     CritErr10
                mov     al,FAIL
                ;
                ; Check for retry and force fail if not ok
                ;
CritErr10:
                cmp     al,RETRY
                jne     CritErr20               ; not retry, keep testing
                test    bh,OK_RETRY
                jnz     CritErr20
                mov     al,FAIL
                ;
                ; You know the drill, but now it's different.
                ; check for fail and force abort if not ok
                ;
CritErr20:
                cmp     al,FAIL
                jne     CritErr30               ; not fail, do exit processing
                test    bh,OK_FAIL
                jnz     CritErr30
                mov     al,ABORT
                ;
                ; OK, if it's abort we do extra processing.  Otherwise just
                ; exit.
                ;
CritErr30:
                cmp     al,ABORT
                je      CritErrAbort            ; process abort

CritErrExit:
                xor     ah,ah                   ; clear out top for return
                pop     di
                pop     si
                pop     bp
                ret

                ;
                ; Abort processing.
                ;
CritErrAbort:
                mov     ax,[_cu_psp]
                mov     es,ax
                cmp     ax,[es:PSP_PARENT]
                mov     al,FAIL
                jz      CritErrExit
                cli
                mov     bp,word [_user_r+2]   ;Get frame
                mov     ss,bp
                mov     es,bp
                mov     bp,word [_user_r]
                mov     sp,bp
                mov     byte [_ErrorMode],1        ; flag abort
                mov     ax,4C00h
                mov     [es:reg_ax],ax
                sti
                jmp     int21_reentry           ; restart the system call
