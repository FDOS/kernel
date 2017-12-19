;
; File:
;                          serial.asm
; Description:
;                      Serial device driver
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

                global  ComTable
ComTable        db      0Ah
                dw      _IOExit
                dw      _IOExit
                dw      _IOExit
                dw      _IOCommandError
                dw      ComRead
                dw      ComNdRead
                dw      ComInStat
                dw      ComInpFlush
                dw      ComWrite
                dw      ComWrite
                dw      ComOutStat



segment	_IO_TEXT

                extern   CommonNdRdExit

ComRead:
                jcxz    ComRd3
                call    GetComStat
                xor     ax,ax
                xchg    [bx],al
                or      al,al
                jnz     ComRd2
ComRd1:
                call    BiosRdCom
                or      ah,ah   ; timeout?
                js      ComRd1  ; yes, try again
ComRd2:
                stosb
                loop    ComRd1

ComRd3:
                jmp     _IOExit


BiosRdCom:
                mov     ah,2
                call    ComIOCall
                test    ah,0Eh
                jz      BiosRdRetn
                add     sp,byte 2
                xor     al,al
                or      al,0B0h
                jmp     _IOErrCnt
BiosRdRetn:
                retn


ComInStat:	; similar to ComNdRead but returns no char, only flags
                call    GetComStat
                mov     al,[bx]
                or      al,al
		jnz	ComInAvail
		call	ComRdStatus
		test	ah,1	; char waiting
		jz	ComNdRtn
;                test    al,20h	; DSR (why do we test this?)
;                jz      ComNdRtn
ComInAvail:	jmp	_IOExit	; return "ready"
		

ComNdRead:
                call    GetComStat
                mov     al,[bx]
                or      al,al
                jnz     ComNdRd1
                call    ComRdStatus
                test    ah,1	; char waiting
                jz      ComNdRtn
;                test    al,20h	; DSR (why do we test this?)
;                jz      ComNdRtn
                call    BiosRdCom
                call    GetComStat
                mov     [bx],al
ComNdRd1:
                jmp     CommonNdRdExit	; return that char
ComNdRtn:
                jmp     _IODone	; return "busy"


ComOutStat:
                call    ComRdStatus
                test    al,20h
                jz      ComNdRtn
                test    ah,20h
                jz      ComNdRtn
                jmp     _IOExit	; return "ready"


ComRdStatus:
                mov     ah,3
                call    ComIOCall
                retn


ComIOCall:
                call    GetUnitNum
                int     14h                     ; RS-232 get char al, ah=return status
                retn


ComInpFlush:
                call    GetComStat
                mov     byte [bx],0
                jmp     _IOExit


ComWrite:
                jcxz    ComRd3
ComWr1:
                mov     al,[es:di]
                inc     di
                mov     ah,1
                call    ComIOCall
                test    ah,80h
                jz      ComWr2
                mov     al,0Ah
                jmp     _IOErrCnt
ComWr2:
                loop    ComWr1
                jmp     _IOExit


GetComStat:
                call    GetUnitNum
                mov     bx,dx
                add     bx,ComStatArray
                retn

segment	_DATA

ComStatArray    db      0, 0, 0, 0

; Revision 1.2  1999/08/10 17:57:13  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:41:31  jprice
; New version without IPL.SYS
;
; Revision 1.1  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; EndLog
;
