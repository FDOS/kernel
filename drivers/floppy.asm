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

     %include "../kernel/segs.inc"
     segment HMA_TEXT
;
;
; Reset both the diskette and hard disk system
;
; BOOL fl_reset(WORD drive)
;
;       returns TRUE if successful
;

		global	FL_RESET
FL_RESET:
		pop	ax		; return address
		pop	dx		; drive
		push	ax		; restore address
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

		global	FL_DISKCHANGED
FL_DISKCHANGED:
		pop	ax		; return address
		pop	dx		; get the drive number
		push	ax		; restore stack
		push	si		; restore stack

                mov     ah,16h          ;  read change status type
		xor	si,si		; RBIL: avoid crash on AT&T 6300
                int     13h

		mov	al,1
                jnc	fl_dc_ret1	; cy==1 is error or disk has changed

		cmp     ah,6		; ah=6: disk has changed
		je      fl_dc_ret
		dec	ax		; 0xFF on error

fl_dc_ret1:	dec	ax
fl_dc_ret:	cbw
                pop	si
                ret

;
; Format Sectors
;
; COUNT fl_format(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Formats one or more tracks, sector should be 0.
;
; Returns 0 if successful, error code otherwise.
		global	FL_FORMAT
FL_FORMAT:
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
		global	FL_READ
FL_READ:
                mov     ah,2            ; cmd READ
                jmp short fl_common
                
		global	FL_VERIFY
FL_VERIFY:
                mov     ah,4            ; cmd verify
                jmp short fl_common
                
		global	FL_WRITE
FL_WRITE:
                mov     ah,3            ; cmd WRITE

fl_common:                
                push    bp              ; C entry
                mov     bp,sp

                mov     cx,[bp+0Ch]     ; cylinder number (lo only if hard)

                mov     al,1            ; this should be an error code                     
                cmp     ch,3            ; this code can't write above 3ff=1023
                ja      fl_error

                xchg    ch,cl           ; ch=low 8 bits of cyl number
                ror     cl,1		; extract bits 8+9 to cl
                ror     cl,1
                or      cl,[bp+0Ah]	; or in the sector number (bits 0-5)

                mov     al,[bp+08h]     ; count to read/write
                les     bx,[bp+04h]     ; Load 32 bit buffer ptr

                mov     dl,[bp+10h]     ; get the drive (if or'ed 80h its
                                        ; hard drive.
                mov     dh,[bp+0Eh]     ; get the head number

                int     13h             ;  write sectors from mem es:bx

		sbb	al,al		; carry: al=ff, else al=0
		and	al,ah		; carry: error code, else 0
					; (Zero transfer count)
fl_error:
                mov     ah,0            ; force into < 255 count
                pop     bp
                ret     14


; COUNT fl_lba_ReadWrite(BYTE drive, UWORD mode, VOID FAR *dap_p)
;
; Returns 0 if successful, error code otherwise.
;
		global  FL_LBA_READWRITE
FL_LBA_READWRITE:
		push    bp              ; C entry
		mov     bp,sp
		
		push    ds
		push    si              ; wasn't in kernel < KE2024Bo6!!

		mov     dl,[bp+10]      ; get the drive (if ored 80h harddrive)
		mov     ax,[bp+8]       ; get the command
		lds     si,[bp+4]       ; get far dap pointer
		int     13h             ; read from/write to drive
		
                pop     si
		pop     ds

		pop     bp
ret_AH:
		mov     al,ah           ; place any error code into al
		mov     ah,0            ; zero out ah           
		ret     8

;
; void fl_readkey (void);
;

global FL_READKEY
FL_READKEY:     xor	ah, ah
		int	16h
		ret

global FL_SETDISKTYPE
FL_SETDISKTYPE:
		pop	bx		; return address
		pop	ax		; disk type (al)
		pop	dx		; drive number (dl)
		push	bx		; restore stack
                mov     ah,17h
                int     13h
		jmp	short ret_AH
                        
;
; COUNT fl_setmediatype (WORD drive, WORD tracks, WORD sectors);
;
global FL_SETMEDIATYPE
FL_SETMEDIATYPE:
		pop	ax		; return address
                pop     bx		; sectors/track
		pop	cx		; number of tracks
		pop	dx		; drive number
		push	ax		; restore stack
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
                
