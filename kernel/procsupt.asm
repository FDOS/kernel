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

                ;; Construct the piece of code into the stack

		;; stack frame:		during generation of code piece
		;; <higher address>
		;; BP | SP | Meaning
		;;  7 | 11 | offset CALL FAR will push onto stack
		;;  5 |  9 | CALL FAR segment
		;;  3 |  7 | CALL FAR offset
		;;  2 |  6 | CALL FAR ??regain_control_int23  | instruction byte
		;;  0 |  4 | INT 23 <<should-be value of SP upon return>>
		;; -2 |  2 | segment of address of INT-23	\ To jump to INT 23
		;; -4 |  0 | offset of address of INT-23	/ via RETF
		;; Upon return from INT-23 the CALL FAR pushes the address of
		;; the byte immediately following the CALL FAR onto the stack.
		;; This value POPed and decremented by 7 is the value SP must
		;; contain, if the INT-23 was returned with RETF2/IRET.

  		sub sp, byte 8		;; code piece needs 7 bytes --> 4 words
  		push ss			;; prepare jump to INT-23 via RETF
  		push bp			;; will be offset / temp: saved BP
  		mov bp, sp
  		add bp, byte 4		;; position BP onto INT-23
  		mov word [bp], 23cdh		;; INT 23h
  		mov byte [bp+2], 9ah			;; CALL FAR immediate
  		mov word [bp+3], ??regain_control_int23
  		mov word [bp+5], cs

  		;; complete the jump to INT-23 via RETF and restore BP
  		xchg word [bp-4], bp

                clc			;; set default action --> resume
                ; invoke the int 23 handler its address has been constructed
                ;; on the stack
                retf

??regain_control_int23:

		;; stack frame:		constructed on entry to INT-23
		;; <higher address>
		; BP | SP | Meaning
		;;  7 | 11 | offset CALL FAR will push onto stack
		;;  5 |  9 | CALL FAR segment
		;;  3 |  7 | CALL FAR offset
		;;  2 |  6 | CALL FAR ??regain_control_int23  | instruction byte
		;;  0 |  4 | INT 23 <<should-be value of SP upon return>>
		;; -2 |  2 | segment of address of INT-23	\ To jump to INT 23
		;; -4 |  0 | offset of address of INT-23	/ via RETF
		;; Upon return from INT-23 the CALL FAR pushes the address of
		;; the byte immediately following the CALL FAR onto the stack.
		;; This value POPed and decremented by 7 is the value SP must
		;; contain, if the INT-23 was returned with RETF2/IRET.

		;; stack frame:		used during recovering from INT-23
		;; <higher address>
		;; BP | Meaning
		;;  1 | <<next word onto stack, or value SP has to become>>
		;;  0 | <<return address from CALL FAR>>
		;; -1 | saved BP
		;; -3 | saved AX
		;; -7 | INT 23 <<should-be value of SP upon return>>

		;; Somewhere on stack:
		;; SP | Meaning
		;;  4 | segment of return address of CALL FAR
		;;  2 | offset of return address of CALL FAR
		;;  0 | saved BP

				push bp
				mov bp, sp
				mov bp, [bp+2]		;; get should-be address + 7
				mov word [bp-3], ax		;; save AX
				pop ax				;; old BP
				mov word [bp-1], ax		;; preserve saved BP
				mov ax, bp
				dec ax			;; last used word of stack
				dec ax			;; Don't use SUB to keep Carry flag
				dec ax
				xchg ax, sp		;; AX := current stack; SP corrected
				;; Currently: BP - 7 == address of INT-23
				;; should be  AX + 4 --> IRET or RETF 2
				;; ==> Test if BP - 7 == AX + 4
				;; ==> Test if AX + 4 - BP + 7 == 0
				pushf			;; preserve Carry flag
				add ax, byte 4 + 7
				sub ax, bp		;; AX := SP + 4
				pop ax			;; saved Carry flag
				jz ??int23_ign_carry ;; equal -> IRET --> ignore Carry
									;; Carry is already cleared
				push ax
				popf			;; restore Carry flag

??int23_ign_carry:
				pop ax					;; Restore the original register
				jnc ??int23_respawn
				;; The user returned via RETF 0, Carry is set
				;; --> terminate program
				;; This is done by set the _break_flg and modify the
				;; AH value, which is passed to the _respawn_ call
				;; into 0, which is "Terminate program".
				push ds			;; we need DGROUP
				mov ds, [cs:_DGROUP_]
				inc byte [_break_flg]
				pop ds

				xor ah, ah		;; clear ah --> perform DOS-00 --> terminate

??int23_respawn:
				pop bp					;; Restore the original register
                jmp 	DGROUP:_int21_handler

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
