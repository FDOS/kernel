;
; File:
;                           int2f.asm
; Description:
;                 multiplex interrupt support code
;
;                    Copyright (c) 1996, 1998
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
; $Logfile:   D:/dos-c/src/kernel/int2f.asv  $
;
; $Id$
;
; $Log$
; Revision 1.7  2001/03/21 02:56:26  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
; Revision 1.6  2000/08/06 05:50:17  jimtabor
; Add new files and update cvs with patches and changes
;
; Revision 1.5  2000/06/21 18:16:46  jimtabor
; Add UMB code, patch, and code fixes
;
; Revision 1.4  2000/05/25 20:56:21  jimtabor
; Fixed project history
;
; Revision 1.3  2000/05/17 19:15:12  jimtabor
; Cleanup, add and fix source.
;
; Revision 1.2  2000/05/08 04:30:00  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.4  2000/03/31 05:40:09  jtabor
; Added Eric W. Biederman Patches
;
; Revision 1.3  2000/03/09 06:07:11  kernel
; 2017f updates by James Tabor
;
; Revision 1.2  1999/08/10 17:57:12  jprice
; ror4 2011-02 patch
;
; Revision 1.1.1.1  1999/03/29 15:40:59  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.3  1999/02/01 01:48:41  jprice
; Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
;
; Revision 1.2  1999/01/22 04:13:26  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:01  jprice
; Imported sources
;
;
;    Rev 1.2   06 Dec 1998  8:48:12   patv
; Bug fixes.
;
;    Rev 1.1   29 May 1996 21:03:46   patv
; bug fixes for v0.91a
;
;    Rev 1.0   19 Feb 1996  3:34:38   patv
; Initial revision.
; $EndLog$
;

		%include "segs.inc"
        %include "stacks.inc"

segment	HMA_TEXT
            extern  _nul_dev:wrt DGROUP
            extern  _umb_start:wrt DGROUP
            extern  _UMB_top:wrt DGROUP
            extern _syscall_MUX14:wrt HMA_TEXT

                global  reloc_call_int2f_handler
reloc_call_int2f_handler:
                sti                             ; Enable interrupts
                cmp     ah,11h                  ; Network interrupt?
                jne     Int2f3                  ; No, continue
Int2f1:
                or      al,al                   ; Installation check?
                jz      FarTabRetn              ; yes, just return
Int2f2:
                stc
FarTabRetn:
                retf    2                       ; Return far
Int2f3:
                cmp     ah,16h
                je      FarTabRetn              ; Win Hook return fast
                cmp     ah,12h
                je      IntDosCal               ; Dos Internal calls
                cmp     ah,10h                  ; SHARE.EXE interrupt?
                je      Int2f1                  ; yes, do installation check
                cmp     ah,14h                  ; NLSFUNC.EXE interrupt?
                jne     Int2f?iret              ; yes, do installation check
Int2f?14:      ;; MUX-14 -- NLSFUNC API
               ;; all functions are passed to syscall_MUX14
               push bp                 ; Preserve BP later on
               PUSH$ALL
               call _syscall_MUX14
               pop bp                  ; Discard incoming AX
               push ax                 ; Correct stack for POP$ALL
               POP$ALL
               mov bp, sp
               or ax, ax
               jnz Int2f?14?1          ; must return set carry
                   ;; -6 == -2 (CS), -2 (IP), -2 (flags)
                   ;; current SP = on old_BP
               and BYTE [bp-6], 0feh   ; clear carry as no error condition
               pop bp
               iret
Int2f?14?1:        or BYTE [bp-6], 1
               pop bp
Int2f?iret:
               iret

;
;return dos data seg.
IntDosCal:
                cmp     al,03
                jne     IntDosCal_1
                push    ax
                mov     ax, seg _nul_dev
                mov     ds,ax
                pop     ax
                clc
                jmp     FarTabRetn
;
;Set FastOpen but does nothing.
IntDosCal_1:
                cmp     al,02ah
                jne     IntDosCal_2
                clc
                jmp     FarTabRetn
;
;   added by James Tabor For Zip Drives
;Return Null Device Pointer
IntDosCal_2:
                cmp     al,02ch
                jne     Int2f2
                mov     ax,_nul_dev
                mov     bx,seg _nul_dev
                clc
                jmp     FarTabRetn

; Int 2F Multipurpose Remote System Calls
;
; added by James Tabor jimtabor@infohwy.com
;
; int_2f_Remote_call(ax,bx,cx,dx,[es:di],si, return data * ptr)
; assume ss == ds after setup of stack in entry
; sumtimes return data *ptr is the push stack word
;
                global  _int2f_Remote_call
_int2f_Remote_call:
                push    bp
                mov     bp,sp
                push    es
                push    ds
                push    si
                push    di
                push    dx
                push    cx
                push    bx

                push    ss              ; hay, did I say assume
                pop     ds

                mov     si,[bp+16]
                les     di,[bp+12]
                mov     dx,[bp+10]
                mov     cx,[bp+8]
                mov     bx,[bp+6]
                mov     ax,[bp+4]

                cmp     al,08h              ; R/W Remote File
                je      short int2f_r_1
                cmp     al,09h
                jne     short int2f_r_2
int2f_r_1:
                call    int2f_call
                jnc     short int2f_skip1
                jmp     int2f_rfner
int2f_skip1:
                xor     ax,ax
                les     di,[bp+18]          ; do return data stuff
                mov     [es:di],cx
                jmp     short int2f_rfner
int2f_r_2:
                cmp     al,0ch              ; Get Remote DPB
                jne     short int2f_r_3
                call    int2f_call
                jc      int2f_rfner
                les     di,[bp+18]
                mov     [es:di+0],ax
                mov     [es:di+2],bx
                mov     [es:di+4],cx
                mov     [es:di+6],dx
                xor     ax,ax
                jmp     short int2f_rfner
int2f_r_3:
                cmp     al,0fh              ; Get Remote File Attrib
                jne     short int2f_r_4
                call    int2f_call
                jc      short int2f_rfner
                mov     si,di
                les     di,[bp+18]      ; pointer to struct
                mov     [es:di+0],ax
                mov     [es:di+2],si    ; lo
                mov     [es:di+4],bx    ; high
                mov     [es:di+6],cx
                mov     [es:di+8],dx
                xor     ax,ax
                jmp     short int2f_rfner
int2f_r_4:
                cmp     al,01eh
                je      short int2f_r_5
                cmp     al,01fh
                jne     short int2f_r_6
int2f_r_5:
                push    ds
                push    word [bp+20]
                pop     ds
                call    int2f_call
                pop     ds
                jc      short int2f_rfner
                xor     ax,ax
                jmp     short int2f_rfner
int2f_r_6:
                cmp     al,021h             ; Lseek from eof
                jne     short int2f_r_7
                call    int2f_call
                jc      short int2f_rfner
                les     di,[bp+18]
                mov     [es:di],ax
                mov     [es:di+2],dx
                xor     ax,ax
                jmp     short int2f_rfner
int2f_r_7:
;
;   everything else goes through here.
;
                call    int2f_call
                jc      int2f_rfner
                xor     ax,ax
int2f_rfner:
                pop     bx
                pop     cx
                pop     dx
                pop     di
                pop     si
                pop     ds
                pop     es
                pop     bp
                ret
;
;  Pull this one out of the Chain.
;
                global  _QRemote_Fn
_QRemote_Fn
                push    bp
                mov     bp,sp
                push    es
                push    ds
                push    si
                push    di
                mov     ax,1123h
                lds     si,[bp+4]
                les     di,[bp+8]
                stc
                int     2fh
                mov     ax,0xffff
                jnc     QRemote_Fn_out
                xor     ax,ax
QRemote_Fn_out:	
                pop     di
                pop     si
                pop     ds
                pop     es
                pop     bp
                ret


int2f_call:
                push    bp
                push    word [bp+18]    ; very fakey, HaHa ;)
                stc                     ; set to fail
                int     2fh
                pop     bp
                pop     bp
                ret
;
; Test to see if a umb driver has been loaded.
;
; From RB list and Dosemu xms.c.
;
; Call the XMS driver "Request upper memory block" function with:
;     AH = 10h
;     DX = size of block in paragraphs
; Return: AX = status
;         0001h success
;         BX = segment address of UMB
;         DX = actual size of block
;         0000h failure
;         BL = error code (80h,B0h,B1h) (see #02775)
;         DX = largest available block
;
; (Table 02775)
; Values for XMS error code returned in BL:
;  00h    successful
;  80h    function not implemented
;  B0h    only a smaller UMB is available
;  B1h    no UMB's are available
;  B2h    UMB segment number is invalid
;
;

segment INIT_TEXT
                global  _Umb_Test
_Umb_Test
                push    bp
                mov     bp,sp
                push    es
                push    ds
                push    dx
                push    bx

                mov     ax, DGROUP
                mov     ds, ax

                mov     ax,4300h        ; is there a xms driver installed?
                int     2fh
                cmp     al,80h
                jne     umbt_error

                mov     ax,4310h
                int     2fh

                push    es              ; save driver entry point
                push    bx

                mov     dx,0xffff       ; go for broke!
                mov     ax,1000h        ; get the umb's
                push    cs              ; setup far return
                push    word umbt1
                push    es              ; push the driver entry point
                push    bx
                retf                    ; Call the driver
umbt1:
;
;       bl = 0xB0 and  ax = 0 so do it again.
;
                cmp     bl,0xb0         ; fail safe
                je      umbtb
                add     sp,4
                jmp     umbt_error
umbtb:
                and     dx,dx           ; if it returns a size of zero.
                jne     umbtc
                add     sp,4
                jmp     umbt_error

umbtc:
                pop     bx              ; restore driver entry
                pop     es

                mov     ax,1000h        ; dx set with largest size
                push    cs
                push    word umbt2
                push    es
                push    bx
                retf
umbt2:
                cmp     ax,1
                jne     umbt_error

                mov     word [_umb_start], bx   ; save the segment
                mov     word [_UMB_top], dx     ; and the true size

umbt_error:     dec     ax

                pop     bx
                pop     dx
                pop     ds
                pop     es
                pop     bp
                ret                ; this was called NEAR!!
