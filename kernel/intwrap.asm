;
; File:
;                           intwrap.asm
; Description:
;                 support for hooking misc interrupts
;                 BIOS disk interrupt support code
;                 warm boot support code
;
;                    Copyright (c) 2005
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

            %include "segs.inc"
            %include "stacks.inc"

segment	HMA_TEXT
            extern _DGROUP_

            ; defined in kernel.asm
            extern _UserInt13  ; actual BIOS int13 disk handler used by kernel
            extern _BIOSInt19  ; original int19 (reboot)


; receives int 13h request, invokes original handler,
; but passes to C routine in inthndr.c on error
; this lets successful calls proceed with minimal overhead
; while allowing us to handle errors, store disk change, ...

            global  reloc_call_int13_handler
reloc_call_int13_handler:
    cli             ; disable other interrupts for now
    stc             ; force error unless BIOS clears
    push dx         ; store BIOS drive # for error handling usage

;TODO FIX this alters DS which is used by some subfunctions!!!
    push ds         ; get segment of kernel DATA
    mov  ds, [cs:_DGROUP_]
    pushf           ; simulate int call so returns back here (flags+cs:ip on stack)
    call far [ds:_UserInt13]
    pop  ds          ; restore ds

    jc   int13err   ; test if error, if not return to caller

int13iret:
    inc sp          ; clean up stack
    inc sp
    retf 2          ; return to caller leaving flags asis

int13err:
    pushf           ; don't mess up flags

    cmp  ah, 06h    ; disk changed
    je   int13wrap

    ; add check for other errors here, such as DMA issues

    popf
    jmp  int13iret  ; pass error asis back to user


int13wrap:
%IF XCPU < 186
    push bp
    push bp
    mov  bp, sp
    mov  [bp+2], word 13h ; do the push 0x13 onto stack
    pop  bp         ; clean up stack frame (leaving just 0x13 pushed on stack)
%ELSE                
    push 13h        ; the 186+ way to push a constant on the stack
%ENDIF

    ; at this point stack has initial flags, called cs,ip, initial dx,
    ; flags returned from int13 call, and value 13h
    ; fall through to intWrapCall to invoke C handler
    

; assumes interrupt # and single word param already pushed on stack
; pushes all registers on stack
; calls C handler
; restores registers (replacing value if changed by C code)
; pops int# and param off stack
; returns via iret
intWrapCall:
                    ; set up register frame
    push ax
    push cx
    push dx
    push bx
    push bp
    push si
    push di
    push ds
    push es

    cld

    Protect386Registers    ; ensure 386+ registers possibly modified by kernel safe

    mov ds,[cs:_DGROUP_]
    push ds                ; mov es, ds
    pop  es
    extern _intXX_filter
    call _intXX_filter

    Restore386Registers    
    
    pop es
    pop ds
    pop di
    pop si
    pop bp
    pop bx
    pop dx
    pop cx
    pop ax

    add sp, 2              ; pop int# off stack
    popf                   ; restore flags
    inc sp                 ; pop param off stack (without altering flags)
    inc sp
    retf 2                 ; iret but ignore pushed flags



%IF 0
; receives int 19h request, resets various int values,
; clears hma if in use, and finally invokes original handler,

            global  reloc_call_int19_handler
reloc_call_int19_handler:
                           
    mov  ds, [cs:_DGROUP_]
    lds  ax,[_BIOSInt19]   ; iret calls original handler, doesn't return
    add  sp, 4             ; so pop callers return address off stack
    push ds                ; and replace with address of original handler
    push ax
    push ax                ; param, ignored
    pushf                  ; flags, ignored
    mov  ax, 19h           ; handler for int 19h
    push ax
    jmp  intWrapCall
%ENDIF
