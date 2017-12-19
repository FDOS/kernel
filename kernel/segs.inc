; File:
;                           segs.inc
; Description:
;     Segment definitions for the kernel
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

; CPU specification -- putting it here because all .asm files include this
; file __NASM_VER__ was introduced in NASM after CPU -- ver 0.98 doesn't
; understand it
%ifdef __NASM_VER__
%if XCPU == 86
CPU 8086
%else
CPU XCPU
%endif
%endif

; for OW on Linux:
%ifdef owlinux
%define WATCOM
%endif

%ifidn __OUTPUT_FORMAT__, obj
group   PGROUP          PSP
group   LGROUP          _IRQTEXT _LOWTEXT _IO_TEXT _IO_FIXED_DATA _TEXT
group   DGROUP          _FIXED_DATA _BSS _DATA _DATAEND CONST CONST2 DCONST DYN_DATA
%ifdef WATCOM
group   TGROUP          HMA_TEXT_START HMA_TEXT HMA_TEXT_END INIT_TEXT_START INIT_TEXT INIT_TEXT_END
%define IGROUP          TGROUP
group   I_GROUP         ID_B I_DATA ICONST ICONST2 ID_E IB_B I_BSS IB_E
%else
group   TGROUP          HMA_TEXT_START HMA_TEXT HMA_TEXT_END
group   IGROUP          INIT_TEXT_START INIT_TEXT INIT_TEXT_END
group   I_GROUP         ID_B ID ID_E IC IDATA IB_B IB IB_E
%endif
%define class(x) class=x
%define nobits
%define exec
%define INITSIZE init_end wrt INIT_TEXT
%define INITTEXTSIZE __INIT_DATA_START wrt INIT_TEXT

%else ; using ELF

BITS 16
; groups are defined in the linker script kernel.ld
extern PGROUP
extern DGROUP
extern LGROUP
extern TGROUP
extern IGROUP
extern I_GROUP
%define class(x)
%define stack
extern INITSIZE
%define INITTEXTSIZE __InitTextEnd

%endif

segment PSP             class(PSP)
segment	_IRQTEXT	class(LCODE) exec
segment	_LOWTEXT	class(LCODE) exec
segment	_IO_TEXT	class(LCODE) exec
segment	_IO_FIXED_DATA	class(LCODE) align=2
segment	_TEXT		class(LCODE) exec
segment	_FIXED_DATA	class(FDATA) align=16
segment	_BSS		class(BSS) align=2
segment _DATA           class(DATA) align=2
segment _DATAEND        class(DATA) align=1
;for WATCOM
segment CONST		class(DATA) align=2
segment CONST2		class(DATA) align=2
;for MSC
segment	DCONST      	class(DCONST) align=2
segment	DYN_DATA        class(DYN_DATA)
segment	HMA_TEXT_START	class(CODE) align=16
segment	HMA_TEXT	class(CODE) exec
segment HMA_TEXT_END	class(CODE) align=16
segment INIT_TEXT_START class(CODE) align=16
segment	INIT_TEXT	class(CODE) exec
segment	INIT_TEXT_END   class(CODE) align=16

%ifdef WATCOM
segment ID_B            class(FAR_DATA) align=16
segment I_DATA          class(FAR_DATA) align=2
segment ICONST          class(FAR_DATA) align=2
segment ICONST2         class(FAR_DATA) align=2
segment ID_E            class(FAR_DATA) align=2
segment IB_B            class(FAR_DATA) align=2
segment I_BSS           class(FAR_DATA) align=2
segment IB_E            class(FAR_DATA) align=2
%else
segment ID_B            class(ID)  align=16
segment ID              class(ID)  align=2
segment IDATA           class(ID)  align=2
segment ID_E            class(ID)  align=2
segment	IC      	class(IC)  align=2
segment	IB_B      	class(IB)  align=2 nobits
segment	IB      	class(IB)  align=2 nobits
segment	IB_E      	class(IB)  align=2 nobits
%endif
