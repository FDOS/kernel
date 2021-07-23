;
; File:
;                          console.asm
; Description:
;                      Console device driver
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
; $Header$
;

                %include "io.inc"


segment	_IO_FIXED_DATA

                global  ConTable
ConTable        db      0Ah
                dw      ConInit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      ConRead
                dw      CommonNdRdExit
                dw      CommonNdRdExit
                dw      ConInpFlush
                dw      ConWrite
                dw      ConWrite
                dw      _IOExit

CTL_PRT_SCREEN  equ     7200h
CTL_P           equ     10h

segment	_LOWTEXT

uScanCode	db	0		; Scan code for con: device

global          _kbdType
_kbdType        db      0		; 00 for 84key, 10h for 102key        

                global  ConInit
ConInit:
	        xor	ax,ax
	        mov	ds,ax
	        mov	al,[496h]
		and	al,10h
		mov	byte[cs:_kbdType],al ; enhanced keyboard if bit 4 set
                jmp     _IOExit

;
; Name:
;       ConRead
;
; Function:
;       Read to address in es:di characters from the keyboard.  Cx contains
;       a count of how many characters are to be transferred.
;
; Description:
;       Calls KbdRdChar to read the characters.  Destroys ax.
;
                global  ConRead
ConRead:
                jcxz    ConRead2                ; Exit if read of zero

ConRead1:
                call    KbdRdChar               ; Get a char from kbd in al
                stosb                           ; Store al to es:[di]
                loop    ConRead1                ; Loop until all are read

ConRead2:
                jmp     _IOExit


readkey:        
       		mov     ah,[cs:_kbdType]
                int     16h
checke0:        cmp	al,0xe0                 ; must check for 0xe0 scan code
        	jne	.ret
		or	ah,ah			; check for Greek alpha
		jz	.ret
		mov	al,0			; otherwise destroy the 0xe0
.ret:		retn
        
;
; Name:
;       KbdRdChar
;
; Function:
;       Read a character from the keyboard.
;
; Description:
;       This subroutine reads a character from the keyboard. It also handles
;       a couple of special functions. 
;       It converts ctrl-printscreen to a control-P.
;       It also accounts for extended scan codes by saving off
;       the high byte of the return and returning it if it was non-zero on
;       the previous read.
;
                global  KbdRdChar
KbdRdChar:
                xor     ax,ax                   ; Zero the scratch register
                xchg    [cs:uScanCode],al	; and swap with scan code
		; now AL is set if previous key was extended,
		; and previous is erased in any case
                or      al,al                   ; Test to see if it was set
                jnz     KbdRdRtn                ; Exit if it was, returning it
		call	readkey     		; get keybd char in al, ah=scan
                or      ax,ax                   ; Zero ?
                jz      KbdRdChar               ; Loop if it is
                cmp     ax,CTL_PRT_SCREEN       ; Ctrl-Print screen?
                jne     KbdRd1                  ; Nope, keep going
                mov     al,CTL_P                        ; Yep, make it ^P
KbdRd1:
                or      al,al                   ; Extended key?
                jnz     KbdRdRtn                ; Nope, just exit
                mov     [cs:uScanCode],ah	; Yep, save the scan code
KbdRdRtn:
                retn

;
; Name:
;       CommonNdRdExit
;
; Function:
;       Checks the keyboard input buffer.
;
; Description:
;       Calls int 16 (get status). Sets Busy-Flag in status field. Destroys ax.
;
                global  CommonNdRdExit
CommonNdRdExit:		; *** tell if key waiting and return its ASCII if yes
                mov     al,[cs:uScanCode]       ; Test for last scan code
			; now AL is set if previous key was extended,
                or      al,al                   ; Was it zero ?
                jnz     ConNdRd2                ; Jump if there's a char waiting
                mov     ah,1
		add     ah,[cs:_kbdType]
                int     16h                     ; Get status, if zf=0  al=char
                jz      ConNdRd4                ; Jump if no char available
                or      ax,ax                   ; Also check for ax=0 as apparently some
                jz      ConNdRd4                ; int16h handlers set ax=0 to indicate unsupported function
		call	checke0			; check for e0 scancode
                or      ax,ax                   ; Zero ?
                jnz     ConNdRd1                ; Jump if not zero
		call	readkey
                jmp     short CommonNdRdExit
		; if char was there but 0, fetch and retry...
		; (why do we check uScanCode here?)

ConNdRd1:
                cmp     ax,CTL_PRT_SCREEN       ; Was ctl+prntscrn key pressed?
                jne     ConNdRd2                ; Jump if not
                mov     al,CTL_P

ConNdRd2:
                lds     bx,[cs:_ReqPktPtr]         ; Set the status
		cmp     byte[bx+2],6		; input status call?
		je      ConNdRd3
                mov     [bx+0Dh],al             ; return the ASCII of that key

ConNdRd3:
                jmp     _IOExit

ConNdRd4:
                jmp     _IODone



                global  ConInpFlush
ConInpFlush:    ; *** flush that keyboard queue
                call    KbdInpChar		; get all available keys
                jmp     _IOExit			; do not even remember the last one



KbdInpChar:	; *** get ??00 or the last waiting key after flushing the queue
		xor	ax,ax
                mov     byte [cs:uScanCode],al
KbdInpCh1:	
                mov     ah,1
		add	ah,[cs:_kbdType]
                int     16h                     ; get status, if zf=0  al=char
                jz      KbdInpRtnZero           ; Jump if zero
		; returns 0 or the last key that was waiting in AL
		call	readkey
                jmp     short KbdInpCh1
                ; just read any key that is waiting, then check if
                ; more keys are waiting. if not, return AL of this
                ; key (which is its ASCII). AH (scan) discarded!
KbdInpRtnZero:  mov ah,1        ; if anybody wants "1 if no key was waiting"!
KbdInpRtn:
                retn


                global  ConWrite
ConWrite:
                jcxz    ConNdRd3                ; Exit if nothing to write
ConWr1:
                mov     al,[es:di]
                inc     di
                int     29h                     ; Do fast output call
                loop    ConWr1                  ; Loop if more to output
                jmp     _IOExit

CBreak:
                mov     byte [cs:uScanCode],3   ; Put a ^C into the buffer
IntRetn:
                iret


;                global  _cso
;_cso
;                push    bp
;                mov     bp,sp
;                push    ax
;                mov     ax,[bp+4]
;                int     29h
;                pop     ax
;                pop     bp
;                retn

                global  _int29_handler
_int29_handler:
                push    ax
                push    si
                push    di
                push    bp
                push    bx
                mov     ah,0Eh
                mov     bx,7
                int     10h                     ; write char al, teletype mode
                pop     bx
                pop     bp
                pop     di
                pop     si
                pop     ax
                iret
