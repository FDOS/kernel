; File:
;                         apisupt.asm
; Description:
;     Assembly support routines for stack manipulation, etc.
;
;                    Copyright (c) 1995, 1998
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

		%include "segs.inc"

segment HMA_TEXT
%if 0

                extern  _api_sp:wrt DGROUP      ; api stacks - for context
                extern  _api_ss:wrt DGROUP      ; switching
                extern  _usr_sp:wrt DGROUP      ; user stacks
                extern  _usr_ss:wrt DGROUP

                global  _set_stack
;
; void set_stack(void) -
;       save current stack and setup our local stack
;
_set_stack:

                ; save foreground stack

                ; we need to get the return values from the stack
                ; since the current stack will change
                pop     ax                      ;get return offset

                ; Save the flags so that we can restore correct interrupt
                ; state later. We need to disable interrupts so that we
                ; don't trash memory with new sp-old ss combination
                pushf
                pop     dx
                cli

                ; save bp
                push    bp

                mov     cx, sp
                neg     cx

                ; save away foreground process' stack
                push    word [_usr_ss]
                push    word [_usr_sp]

                mov     word [_usr_ss],ss
                mov     word [_usr_sp],sp

                ; setup our local stack
                mov     ss,word [_api_ss]
                mov     sp,word [_api_sp]

                add     cx, sp
                add     bp, cx

                ; setup for ret
                push    ax

                ; now restore interrupt state
                push    dx
                popf

                ret

;
; void restore_stack(void) -
;       restore foreground stack, throw ours away
;
                global  _restore_stack
_restore_stack:

        ; we need to get the return values from the stack
        ; since the current stack will change
                pop     cx                      ;get return offset

                ; Save the flags so that we can restore correct interrupt
                ; state later. We need to disable interrupts so that we
                ; don't trash memory with new sp-old ss combination
                pushf
                pop     dx
                cli

                ; save background stack
                mov     word [_api_ss],ss
                mov     word [_api_sp],sp

                ; restore foreground stack here
                mov     ss,word [_usr_ss]
                mov     sp,word [_usr_sp]

                pop     word [_usr_sp]
                pop     word [_usr_ss]

                ; make bp relative to our stack frame
                pop     bp
                ;mov     bp,sp

                ; setup for ret
                push    cx

                ; now restore interrupt state
                push    dx
                popf

                ret
%endif

; Log: apisupt.asm,v 
;
; Revision 1.2  1999/08/10 17:57:12  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:40:38  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.3  1999/02/01 01:48:41  jprice
; Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
;
; Revision 1.2  1999/01/22 04:13:25  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
; Imported sources
;
;   Rev 1.4   06 Dec 1998  8:47:02   patv
;Expanded due to new I/O subsystem.
;
;   Rev 1.3   16 Jan 1997 12:46:44   patv
;pre-Release 0.92 feature additions
;
;   Rev 1.2   29 May 1996 21:03:38   patv
;bug fixes for v0.91a
;
;   Rev 1.1   01 Sep 1995 17:54:26   patv
;First GPL release.
;
;   Rev 1.0   02 Jul 1995  9:04:50   patv
;Initial revision.
; EndLog
;
