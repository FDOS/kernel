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
; $Logfile:   C:/usr/patv/dos-c/src/drivers/floppy.asv  $
;
; $Id$
;
; $Log$
; Revision 1.3  2000/05/25 20:56:19  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/11 03:56:20  jimtabor
; Clean up and Release
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.4  1999/08/10 17:21:08  jprice
; ror4 2011-01 patch
;
; Revision 1.3  1999/04/16 21:29:17  jprice
; ror4 multi-sector IO
;
; Revision 1.2  1999/03/29 17:08:31  jprice
; ror4 changes
;
; Revision 1.1.1.1  1999/03/29 15:40:24  jprice
; New version without IPL.SYS
;
; Revision 1.4  1999/02/14 04:25:16  jprice
; Added functions to check if a floppy disk has been changed.
;
; Revision 1.3  1999/02/08 05:49:47  jprice
; Added Pat's 1937 kernel patches
;
; Revision 1.2  1999/01/22 04:16:39  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:00  jprice
; Imported sources
;
;   Rev 1.3   06 Dec 1998  8:43:00   patv
;New floppy support functions.
;
;   Rev 1.2   29 Aug 1996 13:07:14   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:34   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  7:57:02   patv
;Initial revision.
;

group	TGROUP	_TEXT

segment _TEXT	class=CODE

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
		mov	bx,sp
                mov     ah,0            ; BIOS reset disketter & fixed disk
		mov	dl,[bx+2]
                int     13h

                jc      fl_rst1         ; cy==1 is error
                mov     ax,1            ; TRUE on success
                ret

fl_rst1:        xor     ax,ax           ; FALSE on error
                ret


;
;
; Read DASD Type
;
; COUNT fl_readdasd(WORD drive)
;
;       returns 0-3 if successful, 0xFF if error
;
; Code   Meaning
;  0     The drive is not present
;  1     Drive present, cannot detect disk change
;  2     Drive present, can detect disk change
;  3     Fixed disk
;

		global	_fl_readdasd
_fl_readdasd:
                push    bp
                mov     bp,sp

                mov     dl,[bp+4]       ; get the drive number
                mov     ah,15h          ;  read DASD type
                int     13h

                jc      fl_rdasd1       ; cy==1 is error
                mov     al,ah           ; for the return code
                xor     ah,ah
                pop     bp              ; C exit
                ret

fl_rdasd1:      mov     ah,0            ; BIOS reset disketter & fixed disk
                int     13h
                mov     ax,0FFh         ; 0xFF on error
                pop     bp              ; C exit
                ret


;
;
; Read disk change line status
;
; COUNT fl_diskchanged(WORD drive)
;
;       returns 1 if disk has changed, 0 if not, 0xFF if error
;

		global	_fl_diskchanged
_fl_diskchanged:
                push    bp              ; C entry
                mov     bp,sp

                mov     dl,[bp+4]       ; get the drive number
                mov     ah,16h          ;  read change status type
                int     13h

                jc      fl_dchanged1    ; cy==1 is error or disk has changed
                xor     ax,ax           ; disk has not changed
                pop     bp              ; C exit
                ret

fl_dchanged1:   cmp     ah,6
                jne     fl_dc_error
                mov     ax,1
                pop     bp              ; C exit
                ret

fl_dc_error:    mov     ax,0FFh         ; 0xFF on error
                pop     bp              ; C exit
                ret


;
; Read the disk system status
;
; COUNT fl_rd_status(WORD drive)
;
; Returns error codes
;
; See Phoenix Bios Book for error code meanings
;

		global	_fl_rd_status
_fl_rd_status:
                push    bp              ; C entry
                mov     bp,sp

                mov     dl,[bp+4]       ; get the drive number
                mov     ah,1            ;  read status
                int     13h

                mov     al,ah           ; for the return code
                xor     ah,ah

                pop     bp              ; C exit
                ret


;
; Read Sectors
;
; COUNT fl_read(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Reads one or more sectors.
;
; Returns 0 if successful, error code otherwise.
;
		global	_fl_read
_fl_read:
                push    bp              ; C entry
                mov     bp,sp

                mov     dl,[bp+4]       ; get the drive (if or'ed 80h its
                                        ; hard drive.
                mov     dh,[bp+6]       ; get the head number
                mov     ch,[bp+8]       ; cylinder number (lo only if hard)
                mov     al,[bp+9h]      ; get the top of cylinder
                xor     ah,ah
                mov     cl,6            ; form top of cylinder for sector
                shl     ax,cl
                mov     cl,[bp+0Ah]     ; sector number
                and     cl,03fh         ; mask to sector field bits 5-0
                or      cl,al           ; or in bits 7-6
                mov     al,[bp+0Ch]
                les     bx,[bp+0Eh]   ; Load 32 bit buffer ptr

                mov     ah,2
                int     13h             ; read sectors to memory es:bx

                mov     al,ah
                jc      fl_rd1          ; error, return error code
                xor     al,al           ; Zero transfer count
fl_rd1:
                xor     ah,ah           ; force into < 255 count
                pop     bp
                ret


;
; Write Sectors
;
; COUNT fl_write(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR *buffer);
;
; Writes one or more sectors.
;
; Returns 0 if successful, error code otherwise.
;
		global	_fl_write
_fl_write:
                push    bp              ; C entry
                mov     bp,sp

                mov     dl,[bp+4]       ; get the drive (if or'ed 80h its
                                        ; hard drive.
                mov     dh,[bp+6]       ; get the head number
                mov     ch,[bp+8]       ; cylinder number (lo only if hard)
                mov     al,[bp+9h]      ; get the top of cylinder
                xor     ah,ah
                mov     cl,6            ; form top of cylinder for sector
                shl     ax,cl
                mov     cl,[bp+0Ah]     ; sector number
                and     cl,03fh         ; mask to sector field bits 5-0
                or      cl,al           ; or in bits 7-6
                mov     al,[bp+0Ch]
                les     bx,[bp+0Eh]   ; Load 32 bit buffer ptr

                mov     ah,3
                int     13h             ;  write sectors from mem es:bx

                mov     al,ah
                jc      fl_wr1          ; error, return error code
                xor     al,al           ; Zero transfer count
fl_wr1:
                xor     ah,ah           ; force into < 255 count
                pop     bp
                ret


;
;                              SUBROUTINE
;

		global	_fl_verify
_fl_verify:
                push    bp
                mov     bp,sp
                mov     dl,[bp+4]
                mov     dh,[bp+6]
                mov     ch,[bp+8]
                mov     cl,[bp+0Ah]
                mov     al,[bp+0Ch]
                mov     ah,4
                int     13h                     ; Disk  dl=drive a: ah=func 04h
                                                ;  verify sectors with mem es:bx
                mov     al,ah
                jc      fl_ver1                 ; Jump if carry Set
                xor     al,al                   ; Zero register
fl_ver1:
                xor     ah,ah                   ; Zero register
                pop     bp
                ret


		global	_fl_format
_fl_format:

                xor     ax,ax
                ret


;
;
; Get number of disks
;
; COUNT fl_nrdrives(VOID)
;
;       returns AX = number of harddisks
;

		global	_fl_nrdrives
_fl_nrdrives:
                push    di              ; di reserved by C-code ???

                mov     ah,8            ; DISK, get drive parameters
                mov     dl,80h
                int     13h
                mov     ax,0            ; fake 0 drives on error
                jc      fl_nrd1
                mov     al,dl
fl_nrd1:
                pop     di
                ret
