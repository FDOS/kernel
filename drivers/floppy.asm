;
; File:
;                         floppy.asm
; Description:
;                   floppy disk driver primitives
;
;                       Copyright (c) 1995
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

%ifndef SYS
     %include "../kernel/segs.inc"
     segment HMA_TEXT
%else
     segment _TEXT class=CODE
%endif
;
;
; Reset both the diskette and hard disk system
;
; BOOL fl_reset(WORD drive)
;
;       returns TRUE if successful
;

		global	_fl_reset
_fl_reset:
		pop	ax		; return address
		pop	dx		; drive
		push	dx		; restore stack
		push	ax		;
                mov     ah,0            ; BIOS reset diskette & fixed disk
                int     13h

                sbb	ax,ax		; cy==1 is error
		inc	ax		; TRUE on success, FALSE on failure
                ret


;
;
; Read disk change line status
;
; COUNT fl_diskchanged(WORD drive)
;
;       returns 1 if disk has changed, 0 if not, 0xFFFF if error
;

		global	_fl_diskchanged
_fl_diskchanged:
		pop	ax		; return address
		pop	dx		; get the drive number
		push	dx		; restore stack
		push	ax		;

                mov     ah,16h          ;  read change status type
                int     13h

		mov	al,1
                jnc	fl_dc_ret1	; cy==1 is error or disk has changed

		cmp     ah,6		; ah=6: disk has changed
		je      fl_dc_ret
		dec	ax		; 0xFF on error

fl_dc_ret1:	dec	ax
fl_dc_ret:	cbw
                ret

;
; Format Sectors
;
; COUNT fl_format(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Formats one or more tracks, sector should be 0.
;
; Returns 0 if successful, error code otherwise.
		global	_fl_format
_fl_format:
                mov     ah, 5
                jmp     short fl_common
;
; Read Sectors
;
; COUNT fl_read(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Reads one or more sectors.
;
; Returns 0 if successful, error code otherwise.
;
;
; Write Sectors
;
; COUNT fl_write(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Writes one or more sectors.
;
; Returns 0 if successful, error code otherwise.
;
		global	_fl_read
_fl_read:
                mov     ah,2            ; cmd READ
                jmp short fl_common
                
		global	_fl_verify
_fl_verify:
                mov     ah,4            ; cmd verify
                jmp short fl_common
                
		global	_fl_write
_fl_write:
                mov     ah,3            ; cmd WRITE

fl_common:                
                push    bp              ; C entry
                mov     bp,sp

                mov     cx,[bp+8]       ; cylinder number (lo only if hard)

                mov     al,1            ; this should be an error code                     
                cmp     ch,3            ; this code can't write above 3ff=1023
                ja      fl_error

                xchg    ch,cl           ; ch=low 8 bits of cyl number
                ror     cl,1		; extract bits 8+9 to cl
                ror     cl,1
                or      cl,[bp+0Ah]	; or in the sector number (bits 0-5)

                mov     al,[bp+0Ch]     ; count to read/write
                les     bx,[bp+0Eh]   ; Load 32 bit buffer ptr

                mov     dl,[bp+4]       ; get the drive (if or'ed 80h its
                                        ; hard drive.
                mov     dh,[bp+6]       ; get the head number

                int     13h             ;  write sectors from mem es:bx

		sbb	al,al		; carry: al=ff, else al=0
		and	al,ah		; carry: error code, else 0
					; (Zero transfer count)
fl_error:
                mov     ah,0            ; force into < 255 count
                pop     bp
                ret


; COUNT fl_lba_ReadWrite(BYTE drive, UWORD mode, VOID FAR *dap_p)
;
; Returns 0 if successful, error code otherwise.
;
		global  _fl_lba_ReadWrite
_fl_lba_ReadWrite:
		push    bp              ; C entry
		mov     bp,sp
		
		push    ds
		push    si              ; wasn't in kernel < KE2024Bo6!!

		mov     dl,[bp+4]       ; get the drive (if ored 80h harddrive)
		mov     ax,[bp+6]       ; get the command
		lds     si,[bp+8]       ; get far dap pointer
		int     13h             ; read from/write to drive
		
                pop     si
		pop     ds

		pop     bp
ret_AH:
		mov     al,ah           ; place any error code into al
		mov     ah,0            ; zero out ah           
		ret

;
; void fl_readkey (void);
;

global _fl_readkey
_fl_readkey:    xor	ah, ah
		int	16h
		ret

global _fl_setdisktype
_fl_setdisktype:
		pop	bx		; return address
		pop	dx		; drive number (dl)
		pop	ax		; disk type (al)
		push	ax		; restore stack
		push	dx
		push	bx
                mov     ah,17h
                int     13h
		jmp	short ret_AH
                        
;
; COUNT fl_setmediatype (WORD drive, WORD tracks, WORD sectors);
;
global _fl_setmediatype
_fl_setmediatype:
		pop	ax		; return address
		pop	dx		; drive number
		pop	cx		; number of tracks
                pop     bx		; sectors/track
		push	bx		; restore stack
		push	cx		
		push	dx
		push	ax
                push    di

		dec	cx		; should be highest track
                xchg    ch,cl           ; low 8 bits of cyl number
                
                ror     cl,1		; extract bits 8+9 to cl bit 6+7
                ror     cl,1
                
                or      cl,bl           ; or in bits 7-6

                mov     ah,18h
                int     13h
		jc	skipint1e
		push	es
                xor     dx,dx
                mov     es,dx
		cli
                pop     word [es:0x1e*4+2] ; set int 0x1e table to es:di
                mov     [es:0x1e*4  ], di
		sti
skipint1e:		
                pop     di
		jmp	short ret_AH
                
