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
; $Log$
; Revision 1.6  2001/09/23 20:39:44  bartoldeman
; FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
;
; Revision 1.5  2001/04/15 03:21:50  bartoldeman
; See history.txt for the list of fixes.
;
; Revision 1.4  2001/03/21 02:56:25  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
; Revision 1.3  2000/05/25 20:56:21  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/08 04:29:59  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
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
; $EndLog$
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
;  memcpy(void     *dest, void     *src, int count)
; fmemcpy(void FAR *dest, void FAR *src, int count)
; _fmemcpy(void FAR *dest, void FAR *src, int count)
; fmemset(void FAR *dest, int ch, int count);
; fstrncpy(void FAR*dest, void FAR *src, int count);
;  strcpy (void    *dest, void     *src);
; fstrcpy (void FAR*dest, void FAR *src, int count);
;  strlen (void    *dest);
; fstrlen (void FAR*dest);
;  strchr (BYTE     *src , BYTE ch);
; fstrcmp (BYTE FAR *s1 , BYTE FAR *s2);
;  strcmp (BYTE     *s1 , BYTE     *s2);
; fstrncmp(BYTE FAR *s1 , BYTE FAR *s2, int count);
;  strncmp(BYTE     *s1 , BYTE     *s2, int count);

;***********************************************
; common_setup - set up the standard calling frame for C-functions
;                and save registers needed later
;                also preload the args for the near functions
;                di=arg1
;                si=arg2
;                cx=arg3
;
common_setup:
                pop     bx                      ; get return address
                
                push    bp                      ; Standard C entry
                mov     bp,sp
                push    si
                push    di
                push    es
                push    ds
                ; Set both ds and es to same segment (for near copy)
                pop     es
                push    ds

                ; Set direction to autoincrement
                cld

                                ; to conserve even some more bytes,
                                ; the registers for the near routines
                                ; are preloaded here
                
                ; the destination pointer, d = arg1
                mov             di,[bp+4]

                ; Get the source pointer,  s = arg2
                mov             si,[bp+6]

                ; Get the repitition count, n = arg3
                mov             cx,[bp+8]

                jmp bx


;***********************************************
;
;       VOID memcpy(REG BYTE *s, REG BYTE *d, REG COUNT n);
;
                global  _memcpy
_memcpy:
                call common_setup


domemcpy:
                ; And do the built-in byte copy, but do a 16-bit transfer
                ; whenever possible.
                shr     cx,1
                rep     movsw
                jnc     common_return
                movsb

;
; common_return - pop saved registers and do return
;

common_return:
				pop	ds
		pop     es
                pop     di
                pop     si
                pop     bp
                ret



;************************************************************
;
;       VOID fmemcpy(REG BYTE FAR *d, REG BYTE FAR *s,REG COUNT n);
;
                global  __fmemcpy
                global  _fmemcpy
_fmemcpy:
__fmemcpy:
                call common_setup

                ; Get the far source pointer, s
                lds     si,[bp+8]

                ; Get the far destination pointer d
                les     di,[bp+4]

                ; Get the repetition count, n
                mov     cx,[bp+12]


                jmp short domemcpy

;***************************************************************
;
;       VOID fmemset(REG VOID FAR *d, REG BYTE ch, REG COUNT n);
;
                global  _fmemset
_fmemset:
                call common_setup

                ; Get the repetition count, n
                mov     cx,[bp+10]

                ; Get the far source pointer, s
                les     di,[bp+4]

                ; Get the far destination pointer ch
                mov     al,[bp+8]
                
domemset:                
                mov		ah, al

                shr    cx,1
                rep     stosw
                jnc     common_return
                stosb
                
                jmp  short common_return

;***************************************************************
;
;       VOID memset(REG VOID *d, REG BYTE ch, REG COUNT n);
;
                global  _memset
_memset:
                call common_setup
                
                                ; Get the far source pointer, s
                ; mov      di,[bp+4]

                ; Get the char ch
                mov     ax,si   ; mov al, [bp+6]

                ; Get the repititon count, n
                ; mov     cx,[bp+8]

                jmp short domemset


                
                
;***************************************************************
                
                global  _fstrncpy
_fstrncpy:
                call common_setup

                ; Get the source pointer, ss
                lds             si,[bp+8]

                ; and the destination pointer, d
                les             di,[bp+4]

                mov             cx,[bp+12]
                
                jcxz    common_return
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
                
                jmp  short common_return
                
;*****************************************************************
                


                global  _fstrcpy
_fstrcpy:
                call common_setup

                ; Get the source pointer, ss
                lds             si,[bp+8]

                ; and the destination pointer, d
                les             di,[bp+4]
                
                jmp short dostrcpy

;******
                global  _strcpy
_strcpy:
                call common_setup


                ; Get the source pointer, ss
                ;mov             si,[bp+6]

                ; and the destination pointer, d
                ;mov             di,[bp+4]

dostrcpy:

strcpy_loop:                
                lodsb
                stosb
                test al,al
                jne  strcpy_loop
                
				jmp  short common_return

;******************************************************************                
                
                global  _fstrlen
_fstrlen:
                call common_setup

                ; Get the source pointer, ss
                les             di,[bp+4]

                jmp short dostrlen

;**********************************************
                global  _strlen
_strlen:
                call common_setup

                ; The source pointer, ss, arg1 was loaded as di

dostrlen:           
                mov al,0
                mov cx,0xffff
                repne scasb

                mov ax,cx
                not ax                
                dec ax

                jmp short common_return

;************************************************************
                global  _strchr
_strchr:
                call common_setup

                ; Get the source pointer, ss
                ; mov             si,[bp+4]
                ; mov             bx,[bp+6]
                mov bx,si
                mov si,di

strchr_loop:                
                lodsb
                cmp  al,bl
                je   strchr_found
                test al,al
                jne  strchr_loop
                
                mov si,1                ; return NULL if not found
strchr_found:
                mov ax,si
                dec ax

                jmp common_return

;**********************************************************************
                global  _fstrcmp
_fstrcmp:
                call common_setup

                ; Get the source pointer, ss
                lds             si,[bp+4]

                ; and the destination pointer, d
                les             di,[bp+8]
                
                jmp dostrcmp

;******
                global  _strcmp
_strcmp:
                call common_setup


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
                global  _fstrncmp
_fstrncmp:
                call common_setup

                ; Get the source pointer, ss
                lds             si,[bp+4]

                ; and the destination pointer, d
                les             di,[bp+8]
                mov             cx,[bp+12]
                
                jmp short dostrncmp

;******
                global  _strncmp
_strncmp:
                call common_setup

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
strncmp_retzero:
                xor  ax, ax
                jmp  short strncmp_done2
strncmp_done:
                sbb  ax,ax
		or   al,1
strncmp_done2:  jmp  common_return

