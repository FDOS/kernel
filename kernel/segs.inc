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
group   TGROUP          _TEXT _IO_TEXT _IO_FIXED_DATA
group   DGROUP          _FIXED_DATA _BSS _DATA _DATAEND CONST CONST2 DCONST DYN_DATA
group   I_GROUP         INIT_TEXT_START INIT_TEXT INIT_TEXT_END ID_B ID ID_E IC IDATA ICONST ICONST2 I_DATA I_BSS IB_B IB IB_E
group   HGROUP          HMA_TEXT_START HMA_TEXT HMA_TEXT_END

segment PSP             class=PSP
segment	_TEXT		class=CODE
segment	_IO_TEXT	class=CODE
segment	_IO_FIXED_DATA	class=CODE align=2
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
segment	HMA_TEXT_START	class=HMA align=16
segment	HMA_TEXT	class=HMA
segment HMA_TEXT_END	class=HMA
segment INIT_TEXT_START class=INIT align=16
segment	INIT_TEXT	class=INIT
segment	INIT_TEXT_END   class=INIT	
segment ID_B            class=ID   align=2
segment ID              class=ID   align=2
segment ID_E            class=ID   align=2
segment IDATA           class=ID   align=2
segment	IC      	class=IC   align=2

;for Watcom
segment ICONST          class=FAR_DATA align=2
segment ICONST2         class=FAR_DATA align=2
segment I_DATA          class=FAR_DATA align=2
segment I_BSS           class=FAR_DATA align=2

segment	IB_B      	class=IB   align=2
segment	IB      	class=IB   align=2     
segment	IB_E      	class=IB   align=2

; Log: segs.inc,v 
;
; Revision 1.2  1999/08/10 17:57:13  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:41:30  jprice
; New version without IPL.SYS
;
; Revision 1.1  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
;
;    Rev 1.0   06 Dec 1998  8:12:36   patv
; Initial revision.
; EndLog
;
