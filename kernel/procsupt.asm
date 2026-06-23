;
; File:
;                         procsupt.asm
; Description:
;     Assembly support routines for process handling, etc.
;
;                     Copyright (c) 1995,1998
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
; $Id: procsupt.asm 1591 2011-05-06 01:46:55Z bartoldeman $
;


		%include "segs.inc"

                extern  _user_r

                extern  _break_flg     ; break detected flag
                extern  _term_type
                extern  _int21_handler ; far call system services

                %include "stacks.inc"

segment HMA_TEXT

                extern   _DGROUP_

;
;       Special call for switching processes
;
;       void exec_user(iregs far *irp, int disable_a20)
;
                global  _exec_user
_exec_user:

;                PUSH$ALL
;                mov     ds,[_DGROUP_]
;                cld
;
;
;
                pop     ax		      ; return address (unused)

                pop     bp		      ; irp (user ss:sp)
                pop	si
		pop	cx		      ; disable A20?
		or	cx,cx
		jz	do_iret

                cli
                mov     ss,si
                mov     sp,bp                   ; set-up user stack
                sti
;
                POP$ALL
                extern _ExecUserDisableA20
                jmp DGROUP:_ExecUserDisableA20
do_iret:
                extern _int21_iret
                jmp _int21_iret

segment _LOWTEXT


;; Called whenever the BIOS detects a ^Break state
                global  _got_cbreak
_got_cbreak:
	push ds
	push ax
	mov ax, 40h
	mov ds, ax
	or byte [71h], 80h	;; set the ^Break flag
	pop ax
	pop ds
	iret

segment	HMA_TEXT

;
;       Special call for switching processes during break handling
;
;       void interrupt far spawn_int23()
;
;
;       +---------------+
;       |     flags     |       22
;       +---------------+
;       |       cs      |       20
;       +---------------+
;       |       ip      |       18
;       +---------------+
;       |       es      |       16
;       +---------------+
;       |       ds      |       14
;       +---------------+
;       |       bp      |       12
;       +---------------+
;       |       di      |       10
;       +---------------+
;       |       si      |       8
;       +---------------+
;       |       dx      |       6
;       +---------------+
;       |       cx      |       4
;       +---------------+
;       |       bx      |       2
;       +---------------+
;       |       ax      |       0       <--- bp & sp after mov bp,sp
;       +---------------+
;
                global  _spawn_int23
_spawn_int23:

;; 1999/03/27 ska - comments: see cmt1.txt
		mov ds, [cs:_DGROUP_]		;; Make sure DS is OK
		mov bp, [_user_r]

                ; restore to user stack
                cli					;; Pre-8086 don't disable INT autom.
;*TE PATCH                       
;      CtrlC at DosInput (like C:>DATE does) 
;      Nukes the Kernel.
;      
;      it looks like ENTRY.ASM+PROCSUPT.ASM
;      got out of sync.
;      
;      spawn_int() assumes a stack layout at
;      usr_ss:usr:sp. but usr:ss currently contains 0
;      
;      this patch helps FreeDos to survive CtrlC,
;      but should clearly be done somehow else.
                mov     ss, [_user_r+2]
                RestoreSP

                sti

                ; get all the user registers back
                Restore386Registers
                POP$ALL

				; stack should now match entry to int21h
				
				; we need to call int 23h and be able to determine
				; on return if done via iret or [stc] retf
				; Note: formally we constructed int 23h call on stack
				; but some programs such as Invisible Lan redirector
				; save return address of int 23h call and later check
				; it match int 23h opcode, which if done on stack no
				; no longer matches (as stack space likely reused)

				; save current stack pointer so we can compare
				mov [cs:old_sp], sp
				; clear carry
				; note we ignore, but formally set on return meant abort program
				clc
				; invoke user int 23h
				int 23h
				
				; if old_sp == sp then continue (iret or retf 2 return)
				; else if old_sp == sp-2 then abort (retf 0 return)
				cmp [cs:old_sp], sp
				je ??int23_respawn
				
??int23_abort:
				;; --> terminate program
				;; This is done by set the _break_flg and modify the
				;; AH value, which is passed to the _respawn_ call
				;; into 0, which is "Terminate program".
				push ds			;; we need DGROUP
				mov ds, [cs:_DGROUP_]
				inc byte [_break_flg]
				mov byte [_term_type], 1
				; ecm: This is overwritten in the int 21h handler,
				;  but is passed from the int 23h caller to the
				;  terminate code in MS-DOS by writing this.
				; For us, break_flg is used in function 4Ch to
				;  re-set term_type to 1 later.
				pop ds

				xor ah, ah		;; clear ah --> perform DOS-00 --> terminate

??int23_respawn:
                jmp 	DGROUP:_int21_handler

; stores sp prior to int 23h call
; note: we assume int 23h call not invoked while int 23h call in progress
; value only matters between for duration of above set, int, cmp sequence				
old_sp dw 0

;
; interrupt enable and disable routines
;
;                public  _enable
;_enable         proc near
;                sti
;                ret
;_enable         endp
;
;                public  _disable
;_disable        proc near
;                cli
;                ret
;_disable        endp

        extern _p_0_tos,_P_0

; prepare to call process 0 (the shell) from P_0() in C

    global reloc_call_p_0
reloc_call_p_0:
        pop ax          ; return address (32-bit, unused)
        pop ax
        pop ax          ; fetch parameter 0 (32-bit) from the old stack
        pop dx
        mov ds,[cs:_DGROUP_]
        cli
        mov ss,[cs:_DGROUP_]
        mov sp,_p_0_tos ; load the dedicated process 0 stack
        sti
        push dx         ; pass parameter 0 onto the new stack
        push ax
        call _P_0       ; no return, allow parameter fetch from C
