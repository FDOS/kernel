; File:
;                         asmsupt.asm
; Description:
;       Assembly support routines for miscellaneous functions
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
; version 1.4 by tom.ehlert@ginko.de
; added some more functions
; changed bcopy, scopy, sncopy,...
; to      memcpy, strcpy, strncpy
; Bart Oldeman: optimized a bit: see /usr/include/bits/string.h from 
; glibc 2.2
;
; $Id$
;

		%include "segs.inc"

segment HMA_TEXT

;*********************************************************************
; this implements some of the common string handling functions
;
; every function has 1 entry
;
;   NEAR FUNC()
;
; currently done:
;
; fmemcpyBack(void FAR *dest, void FAR *src, int count)
;  memcpy(void     *dest, void     *src, int count)
; fmemcpy(void FAR *dest, void FAR *src, int count)
;  memset(void *dest, int ch, int count);
; fmemset(void FAR *dest, int ch, int count);
; fstrncpy(void FAR*dest, void FAR *src, int count);
;  strcpy (void    *dest, void     *src);
; fstrcpy (void FAR*dest, void FAR *src);
;  strlen (void    *dest);
; fstrlen (void FAR*dest);
; fmemchr (BYTE FAR *src , int ch);
; fstrchr (BYTE FAR *src , int ch);
;  strchr (BYTE     *src , int ch);
; fstrcmp (BYTE FAR *s1 , BYTE FAR *s2);
;  strcmp (BYTE     *s1 , BYTE     *s2);
; fstrncmp(BYTE FAR *s1 , BYTE FAR *s2, int count);
;  strncmp(BYTE     *s1 , BYTE     *s2, int count);
; fmemcmp(BYTE FAR *s1 , BYTE FAR *s2, int count);
;  memcmp(BYTE     *s1 , BYTE     *s2, int count);

;***********************************************
; pascal_setup - set up the standard calling frame for C-functions
;                and save registers needed later
;                also preload the args for the near functions
;                di=arg1
;                si=arg2
;                cx=arg3
;
pascal_setup:
                pop     ax                      ; get return address
                
                push    bp                      ; Standard C entry
                mov     bp,sp
                push    si
                push    di
                push    ds
                ; Set both ds and es to same segment (for near copy)
                push    ds
                pop     es

                ; Set direction to autoincrement
                cld

                mov bl,6       ; majority (4) wants that
                mov cx,[4+bp]  ; majority (8) wants that (near and far)
                mov si,[6+bp]  ; majority (3) wants that (near)
                mov di,[8+bp]  ; majority (3) wants that (near)
                
                jmp ax



        
;***********************************************
;
;       VOID memcpy(REG BYTE *s, REG BYTE *d, REG COUNT n);
;
                global  MEMCPY
MEMCPY:
                call pascal_setup

                ;mov cx,[4+bp] - preset above
                ;mov si,[6+bp] - preset above
                ;mov di,[8+bp] - preset above

                ;mov bl,6 - preset above


domemcpy:
                ; And do the built-in byte copy, but do a 16-bit transfer
                ; whenever possible.
                shr     cx,1
                rep     movsw
                jnc     memcpy_return
                movsb
memcpy_return:
%if 0                           ; only needed for fmemcpyback
                cld
%endif        

;
; pascal_return - pop saved registers and do return
;
        
                jmp short pascal_return



;************************************************************
;
;       VOID fmemcpy(REG BYTE FAR *d, REG BYTE FAR *s,REG COUNT n);
;       VOID fmemcpyBack(REG BYTE FAR *d, REG BYTE FAR *s,REG COUNT n);
;
                global  FMEMCPY
%if 0
                global  FMEMCPYBACK
FMEMCPYBACK:
                std             ; force to copy the string in reverse order
%endif
FMEMCPY:
                call pascal_setup

                ; Get the repetition count, n preset above
                ; mov     cx,[bp+4]

                ; Get the far source pointer, s
                lds     si,[bp+6]

                ; Get the far destination pointer d
                les     di,[bp+10]
		mov	bl,10

                jmp short domemcpy

;***************************************************************
;
;       VOID fmemset(REG VOID FAR *d, REG BYTE ch, REG COUNT n);
;
                global  FMEMSET
FMEMSET:
                call pascal_setup

                ; Get the repetition count, n - preset above
                ; mov     cx,[bp+4]

                ; Get the fill byte ch
                mov     ax,[bp+6]
                
                ; Get the far source pointer, s
                les     di,[bp+8]
		mov	bl,8

domemset:                
                mov		ah, al

                shr    cx,1
                rep     stosw
                jnc     pascal_return
                stosb
                
                jmp  short pascal_return

;***************************************************************
;
;       VOID memset(REG VOID *d, REG BYTE ch, REG COUNT n);
;
                global  MEMSET
MEMSET:
                call pascal_setup
                
                ; Get the repitition count, n - preset above
                ; mov     cx,[bp+4]

                ; Get the char ch
                mov     ax, [bp+6]

                ; Get the far source pointer, d - preset above
                ; mov      di,[bp+8]

		;mov	bl, 6   ; preset above

                jmp short domemset

;*****
pascal_return:
		pop	ds
                pop     di
                pop     si
                pop     bp

		pop	cx
		sub	bh,bh
		add	sp,bx
		jmp	cx
                
;***************************************************************
%if 0
nix pascal - still untested
                global  _fstrncpy
_fstrncpy:
                call pascal_setup

                ; Get the source pointer, ss
                lds             si,[bp+8]

                ; and the destination pointer, d
                les             di,[bp+4]

                mov             cx,[bp+12]
                
                jcxz    pascal_return
                ;;                 dec     cx
                ;          jcxz    store_one_byte
strncpy_loop:   lodsb
                stosb
                test al,al
                loopnz strncpy_loop
                
store_one_byte: xor al,al
                                    ; the spec for strncpy() would require
                                    ; rep stosb 
                                    ; to fill remaining part of buffer
                stosb
                
                jmp  short pascal_return
%endif                
;*****************************************************************
                
; fstrcpy (void FAR*dest, void FAR *src);


                global  FSTRCPY
FSTRCPY:
                call pascal_setup

                ; Get the source pointer, ss
                lds   si,[bp+4]

                ; and the destination pointer, d
                les   di,[bp+8]

		mov   bl,8
                
                jmp short dostrcpy

;******
                global  STRCPY
STRCPY:
                call pascal_setup


                ; Get the source pointer, ss
                mov   si,[bp+4]

                ; and the destination pointer, d
                mov   di,[bp+6]
		mov   bl,4

dostrcpy:

strcpy_loop:                
                lodsb
                stosb
                test al,al
                jne  strcpy_loop
                
		jmp  short pascal_return

;******************************************************************                
                
                global  FSTRLEN
FSTRLEN:
                call pascal_setup

                ; Get the source pointer, ss
                les   di,[bp+4]
		mov   bl,4

                jmp short dostrlen

;**********************************************
                global  STRLEN
STRLEN:
                call pascal_setup
                ; Get the source pointer, ss
                mov   di,[bp+4]
		mov   bl,2

dostrlen:           
                mov al,0
                mov cx,0xffff
                repne scasb

                mov ax,cx
                not ax                
                dec ax

                jmp short pascal_return

;************************************************************
; strchr (BYTE *src , int ch);

                global  STRCHR
STRCHR:
                call pascal_setup

                ; Get the source pointer, ss
                ; mov             cx,[bp+4] - preset above
                ; mov             si,[bp+6] - preset above
		mov bl,4

strchr_loop:                
                lodsb
                cmp  al,cl
                je   strchr_found
                test al,al
                jne  strchr_loop
                
strchr_retzero:
                xor ax, ax               ; return NULL if not found
                mov dx, ax               ; for fstrchr()
                jmp short pascal_return
                
strchr_found:
                mov ax, si
                mov dx, ds               ; for fstrchr()
strchr_found1:
                dec ax

                jmp short pascal_return


;*****
;  fstrchr (BYTE     far *src , int ch);
                global  FSTRCHR
FSTRCHR:
                call pascal_setup

                ; Get ch (preset above)
                ;mov cx, [bp+4]
                
                ;and the source pointer, src
                lds si, [bp+6]

		;mov	bl, 6 - preset above

                jmp short strchr_loop

;******
                global  FMEMCHR
FMEMCHR:
                call pascal_setup

                ; Get the length - preset above
                ; mov cx, [bp+4]

                ; and the search value
                mov ax, [bp+6]

                ; and the source pointer, ss
                les di, [bp+8]

		mov bl, 8
                
                repne scasb
                jne strchr_retzero
                mov dx, es
                mov ax, di
                dec ax
                jmp short strchr_found1

;**********************************************************************
%if 0
nix pascal - still untested
                global  _fstrcmp
_fstrcmp:
                call pascal_setup

                ; Get the source pointer, ss
                lds             si,[bp+4]

                ; and the destination pointer, d
                les             di,[bp+8]
                
                jmp short dostrcmp

;******
                global  _strcmp
_strcmp:
                call pascal_setup


                ; Get the source pointer, ss
                ; mov             si,[bp+4]

                ; and the destination pointer, d
                ; mov             di,[bp+6]
                xchg si,di

dostrcmp:                       
                                    ; replace strncmp(s1,s2)-->
                                    ;         strncmp(s1,s2,0xffff)
                mov cx,0xffff
                jmp short dostrncmp

                
;**********************************************************************
                global  FSTRNCMP
FSTRNCMP:
                call pascal_setup

                ; Get the source pointer, ss
                lds             si,[bp+4]

                ; and the destination pointer, d
                les             di,[bp+8]
                mov             cx,[bp+12]
                
                jmp short dostrncmp

;******
                global  _strncmp
_strncmp:
                call pascal_setup

                ; Get the source pointer, ss
                ;mov             si,[bp+4]

                ; and the destination pointer, d
                ;mov             di,[bp+6]
                ;mov             cx,[bp+8]
                xchg si,di

dostrncmp:
                jcxz strncmp_retzero

strncmp_loop:                
                lodsb
                scasb
                jne  strncmp_done
                test al,al
                loopne   strncmp_loop
%endif
strncmp_retzero:
                xor  ax, ax
                jmp  short strncmp_done2
strncmp_done:
                lahf
		ror  ah,1
strncmp_done2:  jmp  pascal_return


;**********************************************************************
; fmemcmp(BYTE FAR *s1 , BYTE FAR *s2, int count);
                global  FMEMCMP
FMEMCMP:
                call pascal_setup

                ; the length - preset above
                ; mov cx, [bp+4]
                
                ; Get the source pointer, ss
                les di,[bp+6]

                ; and the destination pointer, d
                lds si,[bp+10]

		mov bl,10

                jmp short domemcmp

;******
;  memcmp(BYTE     *s1 , BYTE     *s2, int count);        
                global  MEMCMP
MEMCMP:
                call pascal_setup

                ; all preset: Get the source pointer, ss
                ;mov             si,[bp+6]

                ; and the destination pointer, d
                ;mov             di,[bp+8]
                ;mov             cx,[bp+4]
		;mov		 bl,6
                xchg si,di

domemcmp:
                jcxz strncmp_retzero
                repe cmpsb
                jne  strncmp_done
                jmp short strncmp_retzero

; Log: asmsupt.asm,v 
;
; Revision 1.3  1999/08/10 17:57:12  jprice
; ror4 2011-02 patch
;
; Revision 1.2  1999/04/23 04:24:39  jprice
; Memory manager changes made by ska
;
; Revision 1.1.1.1  1999/03/29 15:40:41  jprice
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
;    Rev 1.4   06 Dec 1998  8:46:50   patv
; Bug fixes.
;
;    Rev 1.3   03 Jan 1998  8:36:44   patv
; Converted data area to SDA format
;
;    Rev 1.2   29 May 1996 21:03:38   patv
; bug fixes for v0.91a
;
;    Rev 1.1   01 Sep 1995 17:54:26   patv
; First GPL release.
;
;    Rev 1.0   05 Jul 1995 11:38:42   patv
; Initial revision.
; EndLog
;
