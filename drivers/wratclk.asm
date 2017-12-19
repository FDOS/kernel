;
; File:
;                          wratclk.asm
; Description:
;                  WriteATClock - sysclock support
;
;                       Copyright (c) 1995
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

                %include "../kernel/segs.inc"
                %include "../hdr/stacks.inc"

segment	HMA_TEXT

;
;       VOID WriteATClock(bcdDays, bcdHours, bcdMinutes, bcdSeconds)
;       BYTE *bcdDays;
;       BYTE bcdHours;
;       BYTE bcdMinutes;
;       BYTE bcdSeconds;
;
                global  WRITEATCLOCK
WRITEATCLOCK:
                push    bp
                mov     bp,sp
;               bcdSeconds = 4
;               bcdMinutes = 6
;               bcdHours = 8
;               bcdDays = 10
arg bcdDays, bcdHours, bcdMinutes, bcdSeconds
                mov     ch,byte [.bcdHours]
                mov     cl,byte [.bcdMinutes]
                mov     dh,byte [.bcdSeconds]
                mov     dl,0
                mov     ah,3
                int     1ah
                mov     bx,word [.bcdDays]
                mov     dx,word [bx]
                mov     cx,word [bx+2]
                mov     ah,5
                int     1ah
                pop     bp
                ret     8

