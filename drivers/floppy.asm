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
; int ASMPASCAL fl_reset(UBYTE drive);
;
; Reset both the diskette and hard disk system.
; returns TRUE if successful.
;

		global	FL_RESET
FL_RESET:
		pop	ax		; return address
		pop	dx		; drive
		push	ax		; restore address
		mov	ah,0		; reset disk
		int	13h
		sbb	ax,ax		; CF=1: error
		inc	ax		; ...return TRUE (1) on success,
		ret			;  FALSE (0) on error

;
; int ASMPASCAL fl_diskchanged(UBYTE drive);
;
; Read disk change line status.
; returns 1 if disk has changed, 0 if not, 0xFF if error.
;

		global	FL_DISKCHANGED
FL_DISKCHANGED:
		pop	ax		; return address
		pop	dx		; drive
		push	ax		; restore stack

		push	si
		mov	ah,16h		; read change status type
		xor	si,si		; RBIL: avoid crash on AT&T 6300
		int	13h
		pop	si

		sbb	al,al		; CF=0 (disk has not changed)
		jnc	ret_AH_0	; ...return 0
		cmp	ah,6		; ah!=6 (error)
		jne	ret_AH_0	; ...return 0xFF
		mov	al,1		; ah=6 (disk has changed)
		jmp	short ret_AH_0	; ...return 1

;
; int ASMPASCAL fl_read  (UBYTE drive, WORD head, WORD track, WORD sector, WORD count, void FAR *buffer);
; int ASMPASCAL fl_write (UBYTE drive, WORD head, WORD track, WORD sector, WORD count, void FAR *buffer);
; int ASMPASCAL fl_verify(UBYTE drive, WORD head, WORD track, WORD sector, WORD count, void FAR *buffer);
; int ASMPASCAL fl_format(UBYTE drive, WORD head, WORD track, WORD sector, WORD count, void FAR *buffer);
;

; Format tracks (sector should be 0).

		global	FL_FORMAT
FL_FORMAT:
		mov	ah,5		; format track
		jmp	short fl_common

		global	FL_VERIFY
FL_VERIFY:
		mov	ah,4		; verify sector(s)
		jmp	short fl_common

		global	FL_READ
FL_READ:
		mov	ah,2		; read sector(s)
		jmp	short fl_common

		global	FL_WRITE
FL_WRITE:
		mov	ah,3		; write sector(s)

fl_common:
		push	bp
		mov	bp,sp

		mov	cx,[bp+12]	; cylinder number
		mov	al,1		; error code
		cmp	ch,3
		ja	fl_error	; can't write above 3FFh=1023

		xchg	ch,cl		; ch=low 8 bits of cylinder number
		mov	dh,[bp+14]	; head number
		ror	cl,1		; bits 8-9 of cylinder number...
		 ror	cl,1		; ...to bits 6-7 in CL
		or	cl,[bp+10]	; sector number (bits 0-5)

		mov	al,[bp+8]	; number of sectors
		les	bx,[bp+4]	; 32-bit buffer ptr
		mov	dl,[bp+16]	; drive (if or'ed 80h its hard drive)
		int	13h		; process sectors

		sbb	al,al		; carry: al=ff, else al=0
		and	al,ah		; carry: error code, else 0
fl_error:
		mov	ah,0
		pop	bp
		ret	14

;
; int ASMPASCAL fl_lba_ReadWrite(UBYTE drive, WORD mode, void FAR * dap);
;
; Returns 0 if successful, error code otherwise.
;

		global	FL_LBA_READWRITE
FL_LBA_READWRITE:
		push	bp
		mov	bp,sp
		push	si
		push	ds
		mov	dl,[bp+10]	; drive (if or'ed 80h its hard drive)
		mov	ax,[bp+8]	; command
		lds	si,[bp+4]	; far dap pointer
		int	13h		; process sectors
		pop	ds
		pop	si
		pop	bp
		mov	al,ah		; place error code into al
		mov	ah,0
		ret	8

;
; void ASMPASCAL fl_readkey (void);
;

		global	FL_READKEY
FL_READKEY:
		mov	ah,0
		int	16h
		ret

;
; int ASMPASCAL fl_setdisktype (UBYTE drive, WORD type);
;

		global	FL_SETDISKTYPE
FL_SETDISKTYPE:
		pop	bx		; return address
		pop	ax		; disk type
		pop	dx		; drive
		push	bx		; restore stack
		mov	ah,17h		; set disk type for format
		int	13h
ret_AH:
		mov	al,ah		; place error code into al
ret_AH_0:
		mov	ah,0
		ret

;
; int ASMPASCAL fl_setmediatype (UBYTE drive, WORD tracks, WORD sectors);
;

		global	FL_SETMEDIATYPE
FL_SETMEDIATYPE:
		pop	ax		; return address
		pop	bx		; sectors/track
		pop	cx		; number of tracks
		pop	dx		; drive
		push	ax		; restore stack
		push	di

		dec	cx		; last cylinder number
		xchg	ch,cl		; CH=low 8 bits of last cyl number
		ror	cl,1		; bits 8-9 of cylinder number...
		 ror	cl,1		; ...to bits 6-7 in CL
		or	cl,bl		; sectors/track (bits 0-5)
		mov	ah,18h		; set media type for format
		int	13h
		jc	skipint1e

		push	es
		xor	dx,dx
		mov	es,dx
		cli
		mov	[es:0x1e*4],di
		pop	word [es:0x1e*4+2] ; set int 0x1e table to es:di
		sti
skipint1e:
		pop	di
		jmp	short ret_AH
