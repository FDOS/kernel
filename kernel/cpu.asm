; File:
;                         cpu.asm
; Description:
;                   Query basic CPU running on
;
;                            DOS-C
;                   Copyright (c) 2012
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

%include "segs.inc"
segment INIT_TEXT

CPU 386
;*********************************************************************
;
; UWORD query_cpu() based on Eric Auer's public domain cpulevel.asm
; input: none
; output: ax = cpu, 0=8086/8088, 1=186/188, 2=286, 3=386+ 
    global _query_cpu
    _query_cpu:
        ; save registers, assumes enough space on stack & valid stack frame setup
        ;push ax - no need to save, return value saved here
        push bx
        push cx
        pushf       ; save flags

        ; begin check, assume x86 unless later family detected
        xor  bx, bx ; 808x or 186  highest detected family stored in bx
        push bx
        popf        ; try to clear all flag bits
        pushf       ; copy flags to ax so we can test if clear succeeded
        pop  ax
        and  ax, 0f000h
        cmp  ax, 0f000h
        jnz  is286  ; no the 4 msb stuck set to 1, so is a 808x or 8018x
        ; NEC V20/V30 support 186 instructions but
        ; do not mask the shift count like a 186.
        ; based on https://hg.pushbx.org/ecm/ldebug/file/7f3440d5824d/source/init.asm#l3071
        ; which is based on http://www.textfiles.com/hamradio/v20_bug.txt
        mov  ax, sp ; we use stack to do test
        mov  cx, 1  ; after pop still 1 if 8088/8086
		push cx
		dec  cx     ; after pop still 0 if NEC V20/V30
		; next instructions may lock system if breakpoint or trace flag set
		db 8Fh, 0C1h; pop r/m16 with operand cx on 808x, nop on NEC V20/V30
		mov  sp, ax ; reset stack to known good state (pre push, optional pop)
		xor  cx, cx ; cx is 1 if NEC, 0 if 808x
		jz   is808x 
		mov  bx, cx
		jmp  short cleanup
is808x:
        mov  ax,1   ; determine if 8086 or 186
        mov  cl,64  ; try to shift further than size of ax
        shr  ax,cl
        or   ax,ax
        jz   is086  ; 186 ignores the upper bits of cl
        mov  bx, 1  ; 186: above 808x, below 286
is086:  jmp  short cleanup
is286:  mov  bx, 2  ; at least 286
        mov  ax, 0f000h
        push ax
        popf        ; try to set 4 msb of flags
        pushf       ; copy flags to ax so we can test if clear succeeded
        pop  ax
        test ax, 0f000h
        jz cleanup  ; 4 msb stuck to 0: 80286
        mov bx, 3   ; at least 386
        
    cleanup:
        mov  ax, bx ; return CPU family
        popf
        pop  cx
        pop  bx
        retn

