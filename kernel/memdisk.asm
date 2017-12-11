; File:
;                         memdisk.asm
; Description:
;                   Query for memdisk provided config.sys parameters
;
;                            DOS-C
;                   Copyright (c) 2011
;                        FreeDOS
;                      All Rights Reserved
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
;

; requires 386+ registers, check LoL->CPU >=3 prior to calling (or use 386 build)

%include "segs.inc"
segment INIT_TEXT

CPU 386
;*********************************************************************
;
; query_memdisk() based on similar subroutine in Eric Auer's public domain getargs.asm which is based on IFMEMDSK
; input: drive (in AL) to query if memdisk provided disk
; output: a far * to a memdiskinfo structure as defined by memdisk (see config.c) 
; struct memdiskinfo FAR * query_memdisk(UBYTE drive);
	global _query_memdisk
	_query_memdisk:
		; save registers, assumes enough space on stack & valid stack frame setup, ax & dx return values
		push es
		push di
		push ebx
		push ecx
		push edx                ; we only care about high word
		push eax                ; we only care about high word
        mov edx,53490000h       ; magic3 +
        mov dl, al              ; drive number (only argument, assumed to be in AL)
        mov eax,454d0800h       ; magic1 + AH=8 (get geometry)
        mov ecx,444d0000h       ; magic2
        mov ebx,3f4b0000h       ; magic4
        int 13h                 ; BIOS DISK API
        shr eax,16              ; ignore AX
        shr ebx,16              ; ignore BX
        shr ecx,16              ; ignore CX (geometry C/S)
        shr edx,16              ; ignore DX (geometry H in DH)
        cmp ax,4d21h            ; magic5
        jnz nomemdisk
        cmp cx,4d45h            ; magic6
        jnz nomemdisk
        cmp dx,4944h            ; magic7
        jnz nomemdisk
        cmp bx,4b53h            ; magic8
        jnz nomemdisk
		jmp cleanup

	nomemdisk:
		xor di, di              ; return NULL;
		mov es, di
		
	cleanup:
		pop eax
		pop edx
		mov ax, di              ; return MK_FP(es, di);
		mov dx, es
		pop ecx
		pop ebx
		pop di
		pop es
		retn
