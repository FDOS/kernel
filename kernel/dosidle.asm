; File:
;                         DosIdle.asm
; Description:
;                   Dos Idle Interrupt Call
;
;                            DOS-C
;                   Copyright (c) 1995, 1999
;                        James B. Tabor
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
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
;
        %include "segs.inc"

PSP_USERSP      equ     2eh
PSP_USERSS      equ     30h

segment HMA_TEXT

                global  _DosIdle_int
                global  _DosIdle_hlt

                extern   _InDOS
                extern   _cu_psp
                extern   _MachineId
                extern   critical_sp
                extern   _user_r
		; variables as the following are "part of" module inthndlr.c
		; because of the define MAIN before include globals.h there!
                extern   _HaltCpuWhileIdle
                extern   _DGROUP_
;
_DosIdle_hlt:
                push    ds
                mov     ds, [cs:_DGROUP_]
                cmp     byte [_HaltCpuWhileIdle],1
                jb      DosId0
                pushf
                sti
                hlt                ; save some energy :-)
                popf
DosId0:         pop     ds
                retn
;
_DosIdle_int:
                call    _DosIdle_hlt
                push    ds
                mov     ds, [cs:_DGROUP_]
                cmp     byte [_InDOS],1
                ja      DosId1
                call    Do_DosI
DosId1:
                pop     ds
                retn

Do_DosI:
                push    ax
                push    es
                push    word [_MachineId]
                push    word [_user_r]
                push    word [_user_r+2]
                mov     es,word [_cu_psp]
                push    word [es:PSP_USERSS]
                push    word [es:PSP_USERSP]

                int     28h

                mov     es,word [_cu_psp]
                pop     word [es:PSP_USERSP]
                pop     word [es:PSP_USERSS]
                pop     word [_user_r+2]
                pop     word [_user_r]
                pop     word [_MachineId]
                pop     es
                pop     ax
                ret

; segment _DATA		; belongs to DGROUP
; whatever	db whatever

