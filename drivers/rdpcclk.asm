;
; File:
;                         rdpcclk.asm
; Description:
;                 read the PC style clock from bios
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

        %include "..\kernel\segs.inc"

segment	HMA_TEXT

;
;       BOOL ReadPCClock(Ticks)
;       ULONG *Ticks;
;
                global  _ReadPCClock
_ReadPCClock:
                xor     ah,ah
                int     1ah
		extern  _DaysSinceEpoch   ;            ; update days if necessary

                mov     ah,0

                add     word [_DaysSinceEpoch  ],ax    ;   *some* BIOS's accumulate several days
                adc     word [_DaysSinceEpoch+2],0     ;

                mov     ax,dx                          ; set return value
                mov     dx,cx 

                ret

