;
; File:
;                         stacks.inc
; Description:
;               Macro support for register stack frame
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
; $Id$
;

;
; Standard stack frame used throughout DOS-C
;
;       MS-DOS specific
;
;       +---------------+
;       |    irp hi     |       26
;       +---------------+
;       |    irp low    |       24
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
;       |       ax      |       0
;       +---------------+
;

;; Note: The order of the pushed registers _must_ match with the definition
;; of the "iregs" structure within PCB.H, because a pointer to the last
;; pushed register is used as a pointer to a "iregs" structure within the
;; called C sources!       -- 2000/03/22 ska

; Don't use `struc RegFrame' etc. here because it interferes with segment
; definitions.
reg_ax    equ 0
reg_bx    equ 2
reg_cx    equ 4
reg_dx    equ 6
reg_si    equ 8
reg_di    equ 10
reg_bp    equ 12
reg_ds    equ 14
reg_es    equ 16
reg_ip    equ 18
reg_cs    equ 20
reg_flags equ 22
irp_low   equ 24
irp_hi    equ 26

%macro  PUSH$ALL  0
                push    es
                push    ds
                push    bp
                push    di
                push    si
                push    dx
                push    cx
                push    bx
                push    ax
%endmacro

%macro  POP$ALL   0
                pop     ax
                pop     bx
                pop     cx
                pop     dx
                pop     si
                pop     di
                pop     bp
                pop     ds
                pop     es
%endmacro

; I386.inc - 10/25/01 by tom ehlert
;
; compiling the kernel for 386 will (sometimes) change the
; high part of (some) registers, which will be (sometimes) be used 
; later
;
; assumption:
; we have never seen MSVC (our only I386 compiler) to use
; anything but eax,ecx, edx
; so we only protect eax, ebx, ecx, edx to conserve stack space
;
; to save even more stack space, we save only HIGH part of regs
; at some expense of slower execution. it's easier anyway :-)
;


%IFNDEF I386
	; no need to save/restore  anything

;	error 1 2 3
	%macro  ProtectHighPartOfRegistersOn386 0
	%endmacro
	
	%macro  RestoreHighPartOfRegistersOn386 0
	%endmacro


%ELSE           
	%macro  ProtectHighPartOfRegistersOn386  0

		ror eax,16
		push ax
		ror eax,16
		ror ebx,16
		push bx
		ror ebx,16
		ror ecx,16
		push cx
		ror ecx,16
		ror edx,16
		push dx
		ror edx,16

	%endmacro

	%macro  RestoreHighPartOfRegistersOn386  0
		ror edx,16
		pop  dx
		ror edx,16
		ror ecx,16
		pop  cx
		ror ecx,16
		ror ebx,16
		pop  bx
		ror ebx,16
		ror eax,16
		pop  ax
		ror eax,16
	%endmacro

%ENDIF

; Log: stacks.inc,v 
; Revision 1.3  2000/03/09 06:06:38  kernel
; 2017f updates by James Tabor
;
; Revision 1.2  1999/08/10 17:56:26  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:39:35  jprice
; New version without IPL.SYS
;
; Revision 1.3  1999/02/01 01:40:06  jprice
; Clean up
;
; Revision 1.2  1999/01/22 04:17:40  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
; Imported sources
;
;
;   Rev 1.0   07 Feb 1998 20:59:16   patv
;Modified stack frame to match DOS standard
; EndLog

