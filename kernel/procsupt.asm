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
; $Logfile:   C:/usr/patv/dos-c/src/kernel/procsupt.asv  $
;
; $Id$
;
; $Log$
; Revision 1.5  2001/03/24 22:13:05  bartoldeman
; See history.txt: dsk.c changes, warning removal and int21 entry handling.
;
; Revision 1.4  2001/03/21 02:56:26  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
; Revision 1.3  2000/05/25 20:56:21  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/08 04:30:00  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.4  1999/08/10 17:57:13  jprice
; ror4 2011-02 patch
;
; Revision 1.3  1999/04/23 22:38:36  jprice
; Fixed got_cbreak function.
;
; Revision 1.2  1999/04/16 12:21:22  jprice
; Steffen c-break handler changes
;
; Revision 1.1.1.1  1999/03/29 15:41:27  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.3  1999/02/01 01:48:41  jprice
; Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
;
; Revision 1.2  1999/01/22 04:13:27  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
; Imported sources
;
;   Rev 1.4   06 Dec 1998  8:46:44   patv
;Bug fixes.
;
;   Rev 1.3   07 Feb 1998 20:42:08   patv
;Modified stack fram to match DOS standard
;
;   Rev 1.2   29 May 1996 21:03:36   patv
;bug fixes for v0.91a
;
;   Rev 1.1   01 Sep 1995 17:54:24   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  9:05:58   patv
;Initial revision.
; $EndLog$
;


		%include "segs.inc"

                extern  _api_sp:wrt DGROUP      ; api stacks - for context
                extern  _api_ss:wrt DGROUP      ; switching
                extern  _usr_sp:wrt DGROUP      ; user stacks
                extern  _usr_ss:wrt DGROUP

                extern  _kstackp:wrt TGROUP     ; kernel stack
                extern  _ustackp:wrt TGROUP     ; new task stack

                extern  _break_flg:wrt DGROUP   ; break detected flag
                extern  _int21_handler:wrt TGROUP ; far call system services

                %include "stacks.inc"

segment	_TEXT

                extern   _DGROUP_:wrt TGROUP

;
;       Special call for switching processes
;
;       void interrupt far exec_user(irp)
;       iregs far *irp;
;
                global  _exec_user
_exec_user:

;                PUSH$ALL
;                mov     ds,[_DGROUP_]
;                cld
;
;
;
                mov     bp,sp

                mov     ax,word [bp+6]        ; irp (user ss:sp)
                mov     dx,word [bp+8]
                cli
                mov     ss,dx
                mov     sp,ax                   ; set-up user stack
                sti
;
                POP$ALL
                iret




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
				mov ax, DGROUP		;; Make sure DS is OK
				mov ds, ax

                ; restore to user stack
                cli					;; Pre-8086 don't disable INT autom.
                mov     ss,[_usr_ss]
                mov     sp,[_usr_sp]
                sti

                ; get all the user registers back
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

                stc			;; set default action --> terminate
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
				mov bp, DGROUP
				mov ds, bp
				inc byte [_break_flg]
				pop ds

				xor ah, ah		;; clear ah --> perform DOS-00 --> terminate

??int23_respawn:
				pop bp					;; Restore the original register
                jmp 	_int21_handler


                global _init_call_spawn_int23
_init_call_spawn_int23:
                call _spawn_int23
                retf
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

