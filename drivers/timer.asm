;
; File:
;                          timer.asm
; Description:
;             Set a single timer and check when expired
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
; $Logfile:   C:/dos-c/src/drivers/timer.asv  $
;
; $Header$
;
; $Log$
; Revision 1.1  2000/05/06 19:34:45  jhall1
; Initial revision
;
; Revision 1.3  1999/08/10 17:21:08  jprice
; ror4 2011-01 patch
;
; Revision 1.2  1999/03/29 17:08:31  jprice
; ror4 changes
;
; Revision 1.1.1.1  1999/03/29 15:40:34  jprice
; New version without IPL.SYS
;
; Revision 1.2  1999/01/22 04:16:39  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:00  jprice
; Imported sources
;
;
;   Rev 1.2   29 Aug 1996 13:07:12   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:42   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  8:01:04   patv
;Initial revision.
;

group	TGROUP	_TEXT
group	DGROUP	_BSS

segment	_TEXT	class=CODE

;
;       void tmark()
;
	global	_tmark
_tmark:
        push    bp
        mov     bp,sp
        xor     ah,ah
        int     01aH                    ; get current time in ticks
        xor     ah,ah
        mov     word [LastTime],dx    ; and store it
        mov     word [LastTime+2],cx
        pop     bp
        ret


;
;       int tdelay(Ticks)
;
	global	_tdelay
_tdelay:
        push    bp
        mov     bp,sp
        sub     sp,byte 4
        xor     ah,ah
        int     01aH                    ; get current time in ticks
        xor     ah,ah
        mov     word [bp-4],dx      ; and save it to a local variable
        mov     word [bp-2],cx      ; "Ticks"
;
; Do a c equivalent of:
;
;               return Now >= (LastTime + Ticks);
;
        mov     ax,word [LastTime+2]
        mov     dx,word [LastTime]
        add     dx,word [bp+4]
        adc     ax,word [bp+6]
        cmp     ax,word [bp-2]
        ja      short tdel_1
        jne     short tdel_2
        cmp     dx,word [bp-4]
        ja      short tdel_1
tdel_2:
        mov     ax,1                    ; True return
        jmp     short tdel_3
tdel_1:
        xor     ax,ax                   ; False return
tdel_3:
        mov     sp,bp
        pop     bp
        ret


;
;       void twait(Ticks)
;
	global	_twait
_twait:
        push    bp
        mov     bp,sp
        sub     sp,byte 4
        call    _tmark                 ; mark a start
;
;       c equivalent
;               do
;                       GetNowTime(&Now);
;               while((LastTime + Ticks) < Now);
twait_1:
        xor     ah,ah
        int     01aH
        xor     ah,ah                           ; do GetNowTime
        mov     word [bp-4],dx                  ; and save it to "Now"
        mov     word [bp-2],cx
;
;       do comparison
;
        mov     ax,word [LastTime+2]
        mov     dx,word [LastTime]
        add     dx,word [bp+4]
        adc     ax,word [bp+6]
        cmp     ax,word [bp-2]
        jb      short twait_1
        jne     short twait_2
        cmp     dx,word [bp-4]
        jb      short twait_1
twait_2:
        mov     sp,bp
        pop     bp
        ret

segment	_BSS	align=2 class=BSS
LastTime:	resd	1
