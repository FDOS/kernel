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
; Revision 1.2  2000/05/08 04:29:59  jimtabor
; Update CVS to 2020
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

segment	_TEXT
                extern   _int21_syscall:wrt TGROUP
                extern   _int25_handler:wrt TGROUP
                extern   _int26_handler:wrt TGROUP
                extern   _set_stack:wrt TGROUP
                extern   _restore_stack:wrt TGROUP
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


                global  _cpm_entry
                global  _int20_handler
                global  _int21_handler
                global  _low_int25_handler
                global  _low_int26_handler
                global  _int27_handler


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
_cpm_entry:
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
                jmp     short _int21_handler    ; do the system call
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
; Terminate the current process
;
;       VOID INRPT far
;       int20_handler(iregs UserRegs)
;
_int20_handler:
                mov     ah,0            ; terminate through int 21h


;
; MS-DOS system call entry point
;
;       VOID INRPT far
;       int21_handler(iregs UserRegs)
;
_int21_handler:
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
                PUSH$ALL

                ;
                ; Create kernel refernce frame.
                ;
                ; NB: At this point, SS != DS and won't be set that way
                ; until later when which stack to run on is determined.
                ;
                mov     bp,DGROUP
                mov     ds,bp

                ;
                ; Now DS is set, let's save our stack for rentry
                ;
                mov     bp,ss
                mov     word [_lpUserStack+2],bp
                mov     word [_user_r+2],bp
                mov     bp,sp
                mov     word [_lpUserStack],bp        ; store and init
                mov     word [_user_r],bp     ; store and init

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

int21_reentry:
                cmp     ah,33h
                je      int21_user
                cmp     ah,50h
                je      int21_user
                cmp     ah,51h
                je      int21_user
                cmp     ah,62h
                jne     int21_1

int21_user:     push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall
                pop     cx
                pop     cx
                jmp     int21_ret

int21_1:        sti
                cmp     byte [_ErrorMode],0
                je      int21_2
                mov     bp,ds
                mov     ss,bp
                mov     bp,_error_tos
                mov     sp,bp
                cli
                push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall
                jmp     short int21_exit

int21_2:        inc     byte [_InDOS]
                cmp     ah,0ch
                jg      int21_3
;
;   Make FreeDOS better than the others!
;
                cmp     byte [_dosidle_flag],0
                jne     int21_user

                mov     bp,ds
                mov     ss,bp
                mov     bp,_char_api_tos
                mov     sp,bp
                cli
                push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall
                jmp     short int21_exit

int21_3:
                call    dos_crit_sect

                mov     bp,ds
                mov     ss,bp
                mov     bp,_disk_api_tos
                mov     sp,bp
                cli
                ;
                ; Push the far pointer to the register frame for
                ; int21_syscall and remainder of kernel.
                ;
                push    word [_user_r+2]
                push    word [_user_r]
                call    _int21_syscall

                ;
                ; Recover registers from system call.  Registers and flags
                ; were modified by the system call.
                ;
int21_exit:     cli
                mov     bp,word [_user_r+2]
                mov     ss,bp
                mov     bp,word [_user_r]     ; store and init
                mov     sp,bp
                dec     byte [_InDOS]
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
_int27_handler:
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
                jmp     _int21_handler  ; terminate through int 21h

;
; I really do need to get rid of this because it's the only thing stopping
; us from being ROMABLE.
;
stkframe        dd      0

_low_int25_handler:
                sti
                pushf
                push    ax
                push    cx
                push    dx
                push    bx
                push    sp
                push    bp
                push    si
                push    di
                push    ds
                push    es

                mov     word [cs:stkframe], sp     ; save stack frame
                mov     word [cs:stkframe+2], ss

                cld
                mov     ax, DGROUP
                mov     ds, ax

                mov     word [_api_sp], _disk_api_tos
                mov     word [_api_ss], ds

                call    far _set_stack

                push    word [cs:stkframe+2]
                push    word [cs:stkframe]
                call    _int25_handler
                add     sp, byte 4

                call    far _restore_stack

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


_low_int26_handler:
                sti
                pushf
                push    ax
                push    cx
                push    dx
                push    bx
                push    sp
                push    bp
                push    si
                push    di
                push    ds
                push    es

                mov     word [cs:stkframe], sp     ; save stack frame
                mov     word [cs:stkframe+2], ss

                cld
                mov     ax, DGROUP
                mov     ds, ax

                mov     word [_api_sp], _disk_api_tos
                mov     word [_api_ss], ds

                call    far _set_stack

                push    word [cs:stkframe+2]
                push    word [cs:stkframe]
                call    _int26_handler
                add     sp, 4

                call    far _restore_stack

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
                retf


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
; Default Int 24h handler -- always returns fail
;
                global  _int24_handler
_int24_handler: mov     al,FAIL
                iret

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
