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
; $Id$
;
; $Log$
; Revision 1.1  2000/05/06 19:34:52  jhall1
; Initial revision
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

segment	_TEXT
;
;       VOID bcopy(s, d, n)
;       REG BYTE *s, *d;
;       REG COUNT n;
;
;
                global	_bcopy
_bcopy:
                push    bp                      ; Standard C entry
                mov     bp,sp
                push    si
                push    di
                push ds
                push    es

                ; Get the repitition count, n
                mov     cx,[bp+8]
                jcxz      bcopy_exit

                ; Set both ds and es to same segment (for near copy)
                mov             ax,ds
                mov             es,ax

                ; Get the source pointer, ss
                mov             si,[bp+4]

                ; and the destination pointer, d
                mov             di,[bp+6]

?doIt:
                ; Set direction to autoincrement
                cld

                ; And do the built-in byte copy, but do a 16-bit transfer
                ; whenever possible.
                mov al, cl
                and     al,1            ; test for odd count
                jz      b_even
                movsb
b_even:         shr     cx,1
                rep     movsw

                ; Finally do a C exit to return
fbcopy_exit:
bcopy_exit:     pop     es
				pop	ds
                pop     di
                pop     si
                pop     bp
                ret


;
;       VOID fbcopy(s, d, n)
;
;       REG VOID FAR *s, FAR *d;
;       REG COUNT n;
                global  _fbcopy
_fbcopy:
                push    bp              ; Standard C entry
                mov     bp,sp
                push    si
                push    di

                ; Save ds, since we won't necessarily be within our
                ; small/tiny environment
                push    ds
                push    es

                ; Get the repititon count, n
                mov     cx,[bp+12]
                jcxz      fbcopy_exit

                ; Get the far source pointer, s
                lds     si,[bp+4]

                ; Get the far destination pointer d
                les     di,[bp+8]

                jmp short ?doIt

;
;       VOID fmemset(s, ch, n)
;
;       REG VOID FAR *s
;		REG int ch
;       REG COUNT n;
                global  _fmemset
_fmemset:
                push    bp              ; Standard C entry
                mov     bp,sp
                push    di

                ; Save ds, since we won't necessarily be within our
                ; small/tiny environment
                push    es

                ; Get the repititon count, n
                mov     cx,[bp+10]
                jcxz      fmemset_exit

                ; Get the far source pointer, s
                les     di,[bp+4]

				; Test if odd or even
				mov al, cl
				and al, 1

                ; Get the far destination pointer ch
                mov     al,[bp+8]
                mov		ah, al

                jz      m_even
                stosb
m_even:         shr     cx,1
                rep     stosw
                

fmemset_exit:	pop es
				pop di
				pop bp
				ret
