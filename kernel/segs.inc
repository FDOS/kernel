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

group   PGROUP          PSP
group   LGROUP          _LOWTEXT _IO_TEXT _IO_FIXED_DATA _TEXT
group   DGROUP          _FIXED_DATA _BSS _DATA _DATAEND CONST CONST2 DCONST DYN_DATA
%ifdef WATCOM
group   TGROUP          HMA_TEXT_START HMA_TEXT HMA_TEXT_END INIT_TEXT_START INIT_TEXT INIT_TEXT_END
%else
group   TGROUP          HMA_TEXT_START HMA_TEXT HMA_TEXT_END
group   IGROUP          INIT_TEXT_START INIT_TEXT INIT_TEXT_END
%endif
group   I_GROUP         ID_B ID ID_E IC IDATA ICONST ICONST2 I_DATA IB_B I_BSS IB IB_E

segment PSP             class=PSP
segment	_LOWTEXT	class=LCODE
segment	_IO_TEXT	class=LCODE
segment	_IO_FIXED_DATA	class=LCODE align=2
segment	_TEXT		class=LCODE
segment	_FIXED_DATA	class=FDATA align=16
segment	_BSS		class=BSS align=2
segment _DATA           class=DATA align=2
segment _DATAEND        class=DATA align=1
;for WATCOM
segment CONST		class=DATA align=2
segment CONST2		class=DATA align=2
;for MSC
segment	DCONST      	class=DCONST   align=2     
segment	DYN_DATA        class=DYN_DATA
segment	HMA_TEXT_START	class=CODE align=16
segment	HMA_TEXT	class=CODE
segment HMA_TEXT_END	class=CODE
segment INIT_TEXT_START class=CODE align=16
segment	INIT_TEXT	class=CODE
segment	INIT_TEXT_END   class=CODE
segment ID_B            class=ID   align=16
segment ID              class=ID   align=2
segment ID_E            class=ID   align=2
segment IDATA           class=ID   align=2
segment	IC      	class=IC   align=2

;for Watcom
segment ICONST          class=FAR_DATA align=2
segment ICONST2         class=FAR_DATA align=2
segment I_DATA          class=FAR_DATA align=2

segment	IB_B      	class=IB   align=2
segment I_BSS           class=FAR_DATA align=2
segment	IB      	class=IB   align=2     
segment	IB_E      	class=IB   align=2
