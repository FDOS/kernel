;
; File:
;                            boot.asm
; Description:
;                           DOS-C boot
;
;                       Copyright (c) 1997;
;                           Svante Frey
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
;
;	+--------+
;	|        |
;	|        |
;	|--------| 4000:0000
;	|        |
;	|  FAT   |
;	|        |
;	|--------| 2000:0000
;	|BOOT SEC|
;	|RELOCATE|
;	|--------| 1FE0:0000
;	|        |
;	|        |
;	|        |
;	|        |
;	|--------|
;	|BOOT SEC|
;	|ORIGIN  | 07C0:0000
;	|--------|
;	|        |
;	|        |
;	|        |
;	|--------|
;	|KERNEL  |
;	|LOADED  |
;	|--------| 0060:0000
;	|        |
;	+--------+


;%define ISFAT12         1
;%define ISFAT16         1
;%define CALCPARAMS      1
;%define MULTI_SEC_READ  1


segment	.text

%define BASE            0x7c00

                org     BASE

Entry:          jmp     short real_start
		nop

;       bp is initialized to 7c00h
%define bsOemName       bp+0x03      ; OEM label
%define bsBytesPerSec   bp+0x0b      ; bytes/sector
%define bsSecPerClust   bp+0x0d      ; sectors/allocation unit
%define bsResSectors    bp+0x0e      ; # reserved sectors
%define bsFATs          bp+0x10      ; # of fats
%define bsRootDirEnts   bp+0x11      ; # of root dir entries
%define bsSectors       bp+0x13      ; # sectors total in image
%define bsMedia         bp+0x15      ; media descrip: fd=2side9sec, etc...
%define sectPerFat      bp+0x16      ; # sectors in a fat
%define sectPerTrack    bp+0x18      ; # sectors/track
%define nHeads          bp+0x1a      ; # heads
%define nHidden         bp+0x1c      ; # hidden sectors
%define nSectorHuge     bp+0x20      ; # sectors if > 65536
%define drive           bp+0x24      ; drive number
%define extBoot         bp+0x26      ; extended boot signature
%define volid           bp+0x27
%define vollabel        bp+0x2b
%define filesys         bp+0x36

%define LOADSEG         0x0060

%define FATBUF          0x2000          ; offset of temporary buffer for FAT
                                        ; chain

;       Some extra variables

;%define StoreSI         bp+3h          ;temp store

;       To save space, functions that are just called once are
;       implemented as macros instead. Four bytes are saved by
;       avoiding the call / ret instructions.


;       GETDRIVEPARMS:  Calculate start of some disk areas.
;

%macro		GETDRIVEPARMS	0
                mov     si, word [nHidden]
                mov     di, word [nHidden+2]
                add     si, word [bsResSectors]
                adc     di, byte 0              ; DI:SI = first FAT sector

                mov     word [fat_start], si
                mov     word [fat_start+2], di

                mov     al, [bsFATs]
                xor     ah, ah
                mul     word [sectPerFat]       ; DX:AX = total number of FAT sectors

                add     si, ax
                adc     di, dx                  ; DI:SI = first root directory sector
                mov     word [root_dir_start], si
                mov     word [root_dir_start+2], di

                ; Calculate how many sectors the root directory occupies.
                mov     bx, [bsBytesPerSec]
                mov     cl, 5                   ; divide BX by 32
                shr     bx, cl                  ; BX = directory entries per sector

                mov     ax, [bsRootDirEnts]
                xor     dx, dx
                div     bx

                mov     word [RootDirSecs], ax  ; AX = sectors per root directory

                add     si, ax
                adc     di, byte 0              ; DI:SI = first data sector

                mov     [data_start], si
                mov     [data_start+2], di
%endmacro

;-----------------------------------------------------------------------

		times	0x3E-$+$$ db 0

%define tempbuf         bp+0x3E
                dw      LOADSEG

%ifdef CALCPARAMS
%define RootDirSecs     bp+0x27         ; # of sectors root dir uses

%define fat_start       bp+0x29         ; first FAT sector

%define root_dir_start  bp+0x2D         ; first root directory sector

%define data_start      bp+0x31         ; first data sector

%else
%define RootDirSecs     bp+0x40         ; # of sectors root dir uses
                dw      0

%define fat_start       bp+0x42         ; first FAT sector
                dd      0

%define root_dir_start  bp+0x46         ; first root directory sector
                dd      0

%define data_start      bp+0x4A         ; first data sector
                dd      0
%endif

;-----------------------------------------------------------------------
;   ENTRY
;-----------------------------------------------------------------------

real_start:	cli
		cld
		xor	ax, ax
		mov     ss, ax          ; initialize stack
		mov	ds, ax
		mov     bp, 0x7c00
		lea     sp, [bp-0x20]
		sti
		cmp     byte [drive], 0xff ; BIOS bug ??
		jne	dont_use_dl
                mov     [drive], dl	; BIOS passes drive number in DL
                			; a reset should not be needed here
dont_use_dl:    
;		int     0x13            ; reset drive

;		int	0x12		; get memory available in AX
;		mov	ax, 0x01e0
;		mov	cl, 6		; move boot sector to higher memory
;		shl	ax, cl
;		sub	ax, 0x07e0

		mov	ax, 0x1FE0
		mov	es, ax
		mov	si, bp
		mov	di, bp
		mov	cx, 0x0100
		rep	movsw
                jmp     word 0x1FE0:cont

cont:           mov     ds, ax
		mov	ss, ax

                call    print
                db      "FreeDOS",0

%ifdef CALCPARAMS
                GETDRIVEPARMS
%endif


;       FINDFILE: Searches for the file in the root directory.
;
;       Returns:
;                               AX = first cluster of file

                ; First, read the whole root directory
                ; into the temporary buffer.

                mov     ax, word [root_dir_start]
                mov     dx, word [root_dir_start+2]
                mov     di, word [RootDirSecs]
                xor     bx, bx
                mov     word [tempbuf], LOADSEG
                mov     es, [tempbuf]
                call    readDisk
                jc      jmp_boot_error

                xor     di, di

		; Search for KERNEL.SYS file name, and find start cluster.

next_entry:     mov     cx, 11
                mov     si, filename
                push    di
                repe    cmpsb
                pop     di
                mov     ax, [es:di+0x1A]; get cluster number from directory entry
                je      ffDone

                add     di, byte 0x20   ; go to next directory entry
                cmp     byte [es:di], 0	; if the first byte of the name is 0,
                jnz     next_entry	; there is no more files in the directory

                jc      boot_error	; fail if not found
ffDone:
                push    ax              ; store first cluster number

                call    print
                db      " FAT",0



;       GETFATCHAIN:
;
;       Reads the FAT chain and stores it in a temporary buffer in the first
;       64 kb.  The FAT chain is stored an array of 16-bit cluster numbers,
;       ending with 0.
;
;       The file must fit in conventional memory, so it can't be larger than
;       640 kb. The sector size must be at least 512 bytes, so the FAT chain
;       can't be larger than around 3 kb.
;
;       Call with:      AX = first cluster in chain

                ; Load the complete FAT into memory. The FAT can't be larger
                ; than 128 kb, so it should fit in the temporary buffer.

                mov     es, [tempbuf]
                xor     bx, bx
                mov     di, [sectPerFat]
                mov     ax, word [fat_start]
                mov     dx, word [fat_start+2]
                call    readDisk
                pop     ax                      ; restore first cluster number
jmp_boot_error: jc      boot_error

                ; Set ES:DI to the temporary storage for the FAT chain.
                push    ds
                push    es
                pop     ds
                pop     es
                mov     di, FATBUF

next_clust:     stosw                           ; store cluster number
                mov     si, ax                  ; SI = cluster number

%ifdef ISFAT12
                ; This is a FAT-12 disk.

fat_12:         add     si, si          ; multiply cluster number by 3...
                add     si, ax
                shr     si, 1           ; ...and divide by 2
                lodsw

                ; If the cluster number was even, the cluster value is now in
                ; bits 0-11 of AX. If the cluster number was odd, the cluster
                ; value is in bits 4-15, and must be shifted right 4 bits. If
                ; the number was odd, CF was set in the last shift instruction.

                jnc     fat_even
                mov     cl, 4
                shr     ax, cl          ; shift the cluster number

fat_even:       and     ah, 0x0f        ; mask off the highest 4 bits
                cmp     ax, 0x0ff8      ; check for EOF
                jb      next_clust      ; continue if not EOF

%endif
%ifdef ISFAT16
                ; This is a FAT-16 disk. The maximal size of a 16-bit FAT
                ; is 128 kb, so it may not fit within a single 64 kb segment.

fat_16:         mov     dx, [tempbuf]
                add     si, si          ; multiply cluster number by two
                jnc     first_half      ; if overflow...
                add     dh, 0x10        ; ...add 64 kb to segment value

first_half:     mov     ds, dx          ; DS:SI = pointer to next cluster
                lodsw                   ; AX = next cluster

                cmp     ax, 0xfff8      ; >= FFF8 = 16-bit EOF
                jb      next_clust      ; continue if not EOF
%endif

finished:       ; Mark end of FAT chain with 0, so we have a single
                ; EOF marker for both FAT-12 and FAT-16 systems.

                xor     ax, ax
                stosw

                push    cs
                pop     ds

                call    print
                db      " Kernel",0			; "KERNEL"
                

;       loadFile: Loads the file into memory, one cluster at a time.

                mov     es, [tempbuf]   ; set ES:BX to load address
                xor     bx, bx

                mov     si, FATBUF      ; set DS:SI to the FAT chain

cluster_next:   lodsw                           ; AX = next cluster to read
                or      ax, ax                  ; if EOF...
                je      boot_success            ; ...boot was successful

                dec     ax                      ; cluster numbers start with 2
                dec     ax

                mov     di, word [bsSecPerClust]
                and     di, 0xff                ; DI = sectors per cluster
                mul     di
                add     ax, [data_start]
                adc     dx, [data_start+2]      ; DX:AX = first sector to read
                call    readDisk
                jnc     cluster_next


boot_error:     call    print
                db      13,10,"BOOT error!",13,10,0

		xor	ah,ah
		int	0x16			; wait for a key
		int	0x19			; reboot the machine

boot_success:   call    print
                db      " GO! ",0
                mov     bl, [drive]
		jmp	word LOADSEG:0


; prints text after call to this function.

print_1char:        
                xor   bx, bx                   ; video page 0
                mov   ah, 0x0E                 ; else print it
                int   0x10                     ; via TTY mode
print:          pop   si                       ; this is the first character
print1:         lodsb                          ; get token
                push  si                       ; stack up potential return address
                cmp   al, 0                    ; end of string?
                jne   print_1char              ; until done
                ret                            ; and jump to it


;       readDisk:       Reads a number of sectors into memory.
;
;       Call with:      DX:AX = 32-bit DOS sector number
;                       DI = number of sectors to read
;                       ES:BX = destination buffer
;                       ES must be 64k aligned (1000h, 2000h etc).
;
;       Returns:        CF set on error
;                       ES:BX points one byte after the last byte read.

readDisk:       push    si
read_next:      push    dx
                push    ax

;******************** LBA_READ *******************************

						; check for LBA support
		push 	bx
										
  		mov 	ah,041h		;
        	mov 	bx,055aah	;
                mov     dl, [drive]
                int     0x13
                jc	read_normal_BIOS

                sub	bx,0aa55h
                jne	read_normal_BIOS
                
                shr     cx,1			; CX must have 1 bit set
                jnc	read_normal_BIOS
  				
						; OK, drive seems to support LBA addressing

		lea	si,[LBA_DISK_PARAMETER_BLOCK]
                            
						; setup LBA disk block                            	
		mov	[si+12],bx
		mov	[si+14],bx
	
		pop	bx
		
		pop	ax
		pop	dx
		push	dx
		push	ax
		mov	[si+ 8],ax
		mov	[si+10],dx
        	mov	[si+4],bx
		mov	[si+6],es


		mov	ah,042h
                jmp short    do_int13_read

LBA_DISK_PARAMETER_BLOCK:
		db 10h		; constant size of block
		db  0
		dw  1		; 1 sector read
							; and overflow into code !!!
							


read_normal_BIOS:      
                pop 	bx

		pop	ax
		pop	dx
		push	dx
		push	ax
;******************** END OF LBA_READ ************************


                ;
                ; translate sector number to BIOS parameters
                ;

                ;
                ; abs = sector                          offset in track
                ;     + head * sectPerTrack             offset in cylinder
                ;     + track * sectPerTrack * nHeads   offset in platter
                ;
                xchg    ax, cx
                mov     al, [sectPerTrack]
                mul     byte [nHeads]
                xchg    ax, cx
                ; cx = nHeads * sectPerTrack <= 255*63
                ; dx:ax = abs
                div     cx
                ; ax = track, dx = sector + head * sectPertrack
                xchg    ax, dx
                ; dx = track, ax = sector + head * sectPertrack
                div     byte [sectPerTrack]
                ; dx =  track, al = head, ah = sector
                mov     cx, dx
                ; cx =  track, al = head, ah = sector

                ; the following manipulations are necessary in order to
                ; properly place parameters into registers.
                ; ch = cylinder number low 8 bits
                ; cl = 7-6: cylinder high two bits
                ;      5-0: sector
                mov     dh, al                  ; save head into dh for bios
                xchg    ch, cl                  ; set cyl no low 8 bits
                ror     cl, 1                   ; move track high bits into
                ror     cl, 1                   ; bits 7-6 (assumes top = 0)
                mov     al, byte [sectPerTrack]
                sub     al, ah                  ; al has # of sectors left
                inc     ah                      ; sector offset from 1
                or      cl, ah                  ; merge sector into cylinder

%ifdef MULTI_SEC_READ
                ; Calculate how many sectors can be transfered in this read
                ; due to dma boundary conditions.
                push    dx

                mov     si, di                  ; temp register save
                ; this computes remaining bytes because of modulo 65536
                ; nature of dma boundary condition
                mov     ax, bx                  ; get offset pointer
                neg     ax                      ; and convert to bytes
                jz      ax_min_1                ; started at seg:0, skip ahead

                xor     dx, dx                  ; convert to sectors
                div     word [bsBytesPerSec]

                cmp     ax, di                  ; check remainder vs. asked
                jb      ax_min_1                ; less, skip ahead
                mov     si, ax                  ; transfer only what we can

ax_min_1:       pop     dx

                ; Check that request sectors do not exceed track boundary
                mov     si, [sectPerTrack]
                inc     si
                mov     ax, cx                  ; get the sector/cyl byte
                and     ax, 0x3f                ; and mask out sector
                sub     si, ax                  ; si has how many we can read
                mov     ax, di
                cmp     si, di                  ; see if asked <= available
                jge     ax_min_2
                mov     ax, si                  ; get what can be xfered

ax_min_2:       push    ax
                mov     ah, 2
                mov     dl, [drive]
                int     0x13
                pop     ax
%else
                mov     ax, 0x0201
do_int13_read:                
                mov     dl, [drive]
                int     0x13
%endif

read_finished:
                jnc     read_ok                 ; jump if no error
                xor     ah, ah                  ; else, reset floppy
                int     0x13
                pop     ax
                pop     dx                      ; and...
read_next_chained:                   
                jmp     short read_next         ; read the same sector again

read_ok:
%ifdef MULTI_SEC_READ
                mul     word [bsBytesPerSec]    ; add number of bytes read to BX
                add     bx, ax
%else
                add     bx, word [bsBytesPerSec]
%endif
                jnc     no_incr_es              ; if overflow...

                mov     ax, es
                add     ah, 0x10                ; ...add 1000h to ES
                mov     es, ax

no_incr_es:     pop     ax
                pop     dx                      ; DX:AX = last sector number

%ifdef MULTI_SEC_READ
                add     ax, si
                adc     dx, byte 0              ; DX:AX = next sector to read
                sub	di,si                   ; if there is anything left to read,
                jg      read_next               ; continue
%else
                add     ax, byte 1
                adc     dx, byte 0              ; DX:AX = next sector to read
                dec     di                      ; if there is anything left to read,
                jnz     read_next_chained       ; continue
%endif

                clc
                pop     si
                ret

filename        db      "KERNEL  SYS"

		times	0x01fe-$+$$ db 0

sign            dw      0xAA55

; Log: boot.asm,v 
;
; Revision 1.12  1999/09/25 06:42:18  jprice
; Optimize boot loader.  Documentation.
;
; Revision 1.11  1999/09/24 19:04:55  jprice
; Added changes recommended by Jens Horstmeier

; to make their bootable CD work.
;
; Revision 1.10  1999/09/23 04:39:02  jprice
; *** empty log message ***
;
; Revision 1.7  1999/04/23 03:43:46  jprice
; Ported to NASM by ror4
;
; Revision 1.6  1999/04/17 19:14:03  jprice
; Fixed multi-sector code
;
; Revision 1.5  1999/04/17 06:23:26  jprice
; Changed so multi-sector IO is optional.
;
; Revision 1.4  1999/04/13 15:52:22  jprice
; Moves boot sector to top of mem
;
; Revision 1.3  1999/04/06 22:53:36  jprice
; Put back code to read multiple sectors at a time.
;
; Revision 1.2  1999/04/01 07:23:20  jprice
; New boot loader
;
; Revision 1.1.1.1  1999/03/29 15:39:39  jprice
; New version without IPL.SYS
;
; Revision 1.3  1999/03/02 06:57:14  jprice
; Added entry address for more recent versions of TLINK
;
; Revision 1.2  1999/01/21 05:03:58  jprice
; Formating.
;
; Revision 1.1.1.1  1999/01/20 05:51:00  jprice
; Imported sources
;
;
;          Rev 1.5   10 Jan 1997  4:58:06   patv
;       Corrected copyright
;
;          Rev 1.4   10 Jan 1997  4:52:50   patv
;       Re-written to support C drive and eliminate restrictions on IPL.SYS
;
;          Rev 1.3   29 Aug 1996 13:06:50   patv
;       Bug fixes for v0.91b
;
;          Rev 1.2   01 Sep 1995 17:56:44   patv
;       First GPL release.
;
;          Rev 1.1   30 Jul 1995 20:37:38   patv
;       Initialized stack before use.
;
;          Rev 1.0   02 Jul 1995 10:57:52   patv
;       Initial revision.
;

