;
; File:
;                          printer.asm
; Description:
;                      Printer device driver
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

		%define PRT_TIMEOUT    01h
		%define PRT_IOERROR    08h
		%define PRT_SELECTED   10h
		%define PRT_OUTOFPAPER 20h
		%define PRT_ACK	       40h
		%define PRT_NOTBUSY    80h

		%define PRT_WRITECHAR  00h
		%define PRT_INITPORT   01h
		%define PRT_GETSTATUS  02h

segment	_IO_FIXED_DATA

                global  LptTable
LptTable        db      18h
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      _IOSuccess
                dw      _IODone
                dw      _IOExit
                dw      _IOExit
                dw      PrtWrite
                dw      PrtWrite
                dw      PrtOutStat
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      PrtOutBsy
                dw      _IOExit
                dw      _IOExit
                dw      PrtGenIoctl
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      _IOCommandError


segment	_IO_TEXT
                global  uPrtNo
uPrtNo          db      0
uPrtQuantum     dw      50h
                dw      50h, 50h
                db      50h, 00h

PrtWrite:
                jcxz    PrtWr3                  ; Exit if nothing to write

PrtCharLoop:                                    ; next character loop
		 
                mov     bx, 2			; number of retries
PrtRetryTwice:
                mov     ah, PRT_GETSTATUS       ; get status, ah=2
                call    PrtIOCall               ;
                jnz     PrtWr4

		mov     al,[es:di]

                mov     ah, PRT_WRITECHAR       ; print character, ah=0
                call    PrtIOCall               ; (0800)

                jnz     PrtWr4                  ; NZ = error, retry

                inc     di
                loop    PrtCharLoop             ; next character
PrtWr3:
                jmp     _IOExit
PrtWr4:						; repeat
                dec     bx
                jnz     PrtRetryTwice
PrtWr5:
                jmp     _IOErrCnt



PrtOutStat:
                call    GetPrtStat
                jnz     PrtWr5
                mov     al, E_PAPER
                test    ah, PRT_OUTOFPAPER
                jnz     PrtWr5
                test    ah, PRT_NOTBUSY
                jnz     PrtWr3
                jmp     _IODone



GetPrtStat:
                mov     ah,PRT_GETSTATUS

PrtIOCall:
                call    GetUnitNum
                int     17h             ; print char al, get status ah

                mov     al, ah          ; if (stat & 0x30) == 0x30 return 10;
                and     al, PRT_SELECTED|PRT_OUTOFPAPER
                cmp     al, PRT_SELECTED|PRT_OUTOFPAPER
		mov     al, E_WRITE
                je      ret_error_code

                test    ah, PRT_OUTOFPAPER|PRT_IOERROR|PRT_TIMEOUT          ; 29h
                mov     al, E_NOTRDY
		jz      ret_error_code

                test    ah, PRT_OUTOFPAPER	; 20h 
                mov     al, E_WRITE
                jz      ret_error_code		; not out of paper -> E_WRITE

ret_error_code_9:
                mov     al, E_PAPER  

ret_error_code:
                cmp     al, E_NOTRDY            ; 2 = no error
                retn



PrtOutBsy:
                push    ds
                push    es
                pop     ds
                mov     si,di
PrtOtBsy1:
                push    cx
                push    bx
                xor     bx,bx
                mov     bl,[cs:uPrtNo]
                shl     bx,1
                mov     cx,[cs:uPrtQuantum+bx]
                pop     bx
PrtOtBsy2:
                call    GetPrtStat
                jnz     PrtOtBsy3
                test    ah, PRT_NOTBUSY
                loopz   PrtOtBsy2
                pop     cx
                jz      PrtOtBsy4
                lodsb
                xor     ah,ah
                call    PrtIOCall
                jnz     PrtOtBsy4
                loop    PrtOtBsy1
                pop     ds
                lds     bx,[cs:_ReqPktPtr]
                sub     [bx+12h],cx
                jmp     _IOExit
PrtOtBsy3:
                pop     cx
PrtOtBsy4:
                pop     ds
                lds     bx,[cs:_ReqPktPtr]
                sub     [bx+12h],cx
                jmp     _IOErrorExit



PrtGenIoctl:
                les     di,[cs:_ReqPktPtr]
                cmp     byte [es:di+0Dh],5
                je      PrtGnIoctl2
PrtGnIoctl1:
                jmp     _IOCommandError
PrtGnIoctl2:
                mov     al,[es:di+0Eh]
                les     di,[es:di+13h]
                xor     bx,bx
                mov     bl,[cs:uPrtNo]
                shl     bx,1
                mov     cx,[cs:uPrtQuantum+bx]
                cmp     al,65h
                je      PrtGnIoctl3
                cmp     al,45h
                jne     PrtGnIoctl1
                mov     cx,[es:di]
PrtGnIoctl3:
                mov     [cs:uPrtQuantum+bx],cx
                mov     [es:di],cx
                jmp     _IOExit



;
; some comments to last changes (TE, 23/09/01)
;
; original implementation didn't print at all  - on my machine,LPT2
;
; maybe this one's not much better either,
; but should print a little bit
;
; the status bits = AH
; 
;               1                       0
; 80 - BUSY  not busy               busy
; 40 - ACK   transfer finished      not yet finished
; 20 - PAP   no paper available     paper OK
; 10 - ONOF  printer online         not online
; 08 - ERR   some error             no error
; 01 - TIM   some error when transfer   OK
;
; some states
; 30 - there is no printer at all
; c8 - there is a printer without power
; 10 - printer with power, but not initialized
; 90 - this one is fine
;
; you must not simply print without asking for status
; as the BIOS has a LARGE timeout before aborting
;

; Log: printer.asm,v
;
; Revision 1.2  1999/08/10 17:57:13  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:41:26  jprice
; New version without IPL.SYS
;
; Revision 1.1  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; EndLog
;

