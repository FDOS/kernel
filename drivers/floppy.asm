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
; $Id: floppy.asm 980 2004-06-19 19:41:47Z perditionc $
;

%include "../kernel/segs.inc"
%include "../hdr/stacks.inc"
segment HMA_TEXT

;
; BOOL ASMPASCAL fl_reset(WORD drive);
;
; Reset both the diskette and hard disk system.
; returns TRUE if successful.
;

		global	FL_RESET
FL_RESET:
		pop	ax		; return address
		pop	dx		; drive (DL only)
		push	ax		; restore address
		mov	ah,0		; BIOS reset diskette & fixed disk
		int	13h

		sbb	ax,ax		; carry set indicates error, AX=-CF={-1,0}
		inc	ax		; ...return TRUE (1) on success,
		ret			; else FALSE (0) on failure

;
; COUNT ASMPASCAL fl_diskchanged(WORD drive);
;
; Read disk change line status.
; returns 1 if disk has changed, 0 if not, 0xFFFF if error.
;

		global	FL_DISKCHANGED
FL_DISKCHANGED:
		pop	ax		; return address
		pop	dx		; drive (DL only, 00h-7Fh)
		push	ax		; restore stack

		push	si		; preserve value
		mov	ah,16h	; read change status type
		xor	si,si		; RBIL: avoid crash on AT&T 6300
		int	13h
		pop	si		; restore

		sbb	al,al		; AL=-CF={-1,0} where 0==no change
		jnc   fl_dc		; carry set on error or disk change
		cmp	ah,6		; if AH==6 then disk change, else error
		jne	fl_dc		; if error, return -1
		mov	al, 1		; set change occurred
fl_dc:	cbw			; extend AL into AX, AX={1,0,-1}
		ret			; note: AH=0 on no change, AL set above

;
; Format tracks (sector should be 0).
; COUNT ASMPASCAL fl_format(WORD drive, WORD head, WORD track, WORD sector, WORD count, UBYTE FAR *buffer);
; Reads one or more sectors.
; COUNT ASMPASCAL fl_read  (WORD drive, WORD head, WORD track, WORD sector, WORD count, UBYTE FAR *buffer);
; Writes one or more sectors.
; COUNT ASMPASCAL fl_write (WORD drive, WORD head, WORD track, WORD sector, WORD count, UBYTE FAR *buffer);
; COUNT ASMPASCAL fl_verify(WORD drive, WORD head, WORD track, WORD sector, WORD count, UBYTE FAR *buffer);
;
; Returns 0 if successful, error code otherwise.
;

		global	FL_FORMAT
FL_FORMAT:
                mov     ah,5            ; format track
                jmp     short fl_common

		global	FL_READ
FL_READ:
                mov     ah,2            ; read sector(s)
                jmp short fl_common
                
		global	FL_VERIFY
FL_VERIFY:
                mov     ah,4            ; verify sector(s)
                jmp short fl_common
                
		global	FL_WRITE
FL_WRITE:
                mov     ah,3            ; write sector(s)

fl_common:                
                push    bp              ; setup stack frame
                mov     bp,sp

arg drive, head, track, sector, count, {buffer,4}
                mov     cx,[.track]     ; cylinder number

                mov     al,1            ; this should be an error code                     
                cmp     ch,3            ; this code can't write above 3FFh=1023
                ja      fl_error        ; as cylinder # is limited to 10 bits.

                xchg    ch,cl           ; ch=low 8 bits of cyl number
                ror     cl,1            ; bits 8-9 of cylinder number...
                ror     cl,1            ; ...to bits 6-7 in CL
                or      cl,[.sector]	; or in the sector number (bits 0-5)

                mov     al,[.count]     ; count of sectors to read/write/...
                les     bx,[.buffer]    ; Load 32 bit buffer ptr into ES:BX

                mov     dl,[.drive]     ; drive (if or'ed 80h its a hard drive)
                mov     dh,[.head]      ; get the head number

                int     13h             ; process sectors

		sbb	al,al		; carry: al=ff, else al=0
		and	al,ah		; carry: error code, else 0
fl_error:
                mov     ah,0            ; extend AL into AX without sign extension
                pop     bp
                ret     14

;
; COUNT ASMPASCAL fl_lba_ReadWrite(BYTE drive, WORD mode, void FAR * dap_p);
;
; Returns 0 if successful, error code otherwise.
;

		global  FL_LBA_READWRITE
FL_LBA_READWRITE:
		push    bp              ; setup stack frame
		mov     bp,sp
		
		push    ds
		push    si              ; wasn't in kernel < KE2024Bo6!!

arg drive, mode, {dap_p,4}
		mov     dl,[.drive]     ; drive (if or'ed with 80h a hard drive)
		mov     ax,[.mode]      ; get the command
		lds     si,[.dap_p]     ; get far dap pointer
		int     13h             ; read from/write to drive
		
		pop     si
		pop     ds

		pop     bp

		mov     al,ah           ; place any error code into al
		mov     ah,0            ; zero out ah           
		ret     8

;
; void ASMPASCAL fl_readkey (void);
;

		global	FL_READKEY
FL_READKEY:     xor	ah, ah
		int	16h
		ret

;
; COUNT ASMPASCAL fl_setdisktype (WORD drive, WORD type);
;

		global	FL_SETDISKTYPE
FL_SETDISKTYPE:
		pop	bx		; return address
		popargs dx,ax		; drive number(dl),disk format type(al)
		push	bx		; restore stack
		mov	ah,17h	; floppy set disk type for format
		int	13h
ret_AH:
		mov     al,ah           ; place any error code into al
		mov     ah,0            ; zero out ah           
		ret
                        
;
; COUNT ASMPASCAL fl_setmediatype (WORD drive, WORD tracks, WORD sectors);
;
		global	FL_SETMEDIATYPE
FL_SETMEDIATYPE:
		pop	ax		; return address
		popargs dx,cx,bx	; drive number,#tracks,sectors/track
		push	ax		; restore stack
		push	di

		dec	cx		; number of cylinders - 1 (last cyl number)
		xchg	ch,cl		; CH=low 8 bits of last cyl number
               
		ror	cl,1		; extract bits 8-9 of cylinder number...
		ror	cl,1		; ...into cl bit 6-7
                
		or	cl,bl		; sectors/track (bits 0-5) or'd with high cyl bits 7-6

		mov	ah,18h	; disk set media type for format
		int	13h
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
                
