;
; File:
;                          rdatclk.asm
; Description:
;                 read the AT style clock from bios
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
; @Logfile:   C:/dos-c/src/drivers/rdatclk.asv  @
;
; @Header: /home/cvsroot/fdkernel/DRIVERS/RDATCLK.ASM,v 1.3 1999/04/12 03:19:44 jprice Exp @
;
; @Log: rdatclk.asm,v @
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.3  1999/04/12 03:19:44  jprice
; more ror4 patches
;
; Revision 1.2  1999/03/29 17:08:31  jprice
; ror4 changes
;
; Revision 1.1.1.1  1999/03/29 15:40:31  jprice
; New version without IPL.SYS
;
; Revision 1.2  1999/01/22 04:16:39  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:00  jprice
; Imported sources
;
;
;   Rev 1.2   29 Aug 1996 13:07:10   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:40   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  8:00:16   patv
;Initial revision.
;

        %include "..\kernel\segs.inc"

segment	HMA_TEXT

;
;COUNT ReadATClock(bcdDays, bcdHours, bcdMinutes, bcdSeconds)
;BYTE *bcdDays;
;BYTE *bcdHours;
;BYTE *bcdMinutes;
;BYTE *bcdSeconds;
;
                global  _ReadATClock
_ReadATClock:
		push	bp
		mov	bp, sp
		xor	cx, cx		; cx=dx=0 check if present
		xor	dx, dx		; if it returns non-zero
		clc			; necessary according to RBIL
                mov     ah,4		
                int     1ah
		jc	@RdATerror

		or	cx, cx
		jnz	@RdAT1140
		or	dx, dx
		jnz	@RdAT1140
@RdATerror:	mov	ax, 1
		pop	bp
                ret
@RdAT1140:
		clc
		mov	ah, 2
		int	1ah
		jc	@RdATerror
;               bcdSeconds = 10
;               bcdMinutes = 8
;               bcdHours = 6
;               bcdDays = 4
                mov     bx,word [bp+6]      ;bcdHours
                mov     byte [bx],ch        ;Hours
                mov     bx,word [bp+8]      ;bcdMinutes
                mov     byte [bx],cl        ;Minutes
                mov     bx,word [bp+10]     ;bcdSeconds
                mov     byte [bx],dh        ;Seconds
		clc
                mov     ah,4
                int     1ah
		jc	@RdATerror
                mov     bx,word [bp+4]      ;bcdDays
                mov     word [bx],dx        ;Days    
                mov     word [bx+2],cx
                sub     ax,ax
		pop	bp
                ret
