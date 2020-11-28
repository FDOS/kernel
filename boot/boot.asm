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
;       +--------+ 1FE0:7E00
;       |BOOT SEC|
;       |RELOCATE|
;       |--------| 1FE0:7C00
;       |LBA PKT |
;       |--------| 1FE0:7BC0
;       |--------| 1FE0:7BA0
;       |BS STACK|
;       |--------|
;       |4KBRDBUF| used to avoid crossing 64KB DMA boundary
;       |--------| 1FE0:63A0
;       |        |
;       |--------| 1FE0:3000
;       | CLUSTER|
;       |  LIST  |
;       |--------| 1FE0:2000
;       |        |
;       |--------| 0000:7E00
;       |BOOT SEC| overwritten by max 128k FAT buffer
;       |ORIGIN  | and later by max 134k loaded kernel
;       |--------| 0000:7C00
;       |        |
;       |--------|
;       |KERNEL  | also used as max 128k FAT buffer
;       |LOADED  | before kernel loading starts
;       |--------| 0060:0000
;       |        |
;       +--------+


;%define ISFAT12         1
;%define ISFAT16         1


segment .text

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

;-----------------------------------------------------------------------

                times   36h - ($ - $$) db 0
                ; The filesystem ID is used by lDOS's instsect (by ecm)
                ;  by default to validate that the filesystem matches.
%ifdef ISFAT12
                db "FAT12"
 %ifdef ISFAT16
 %error Must select one FS
 %endif
%elifdef ISFAT16
                db "FAT16"
%else
 %error Must select one FS
%endif
                times   3Eh - ($ - $$) db 32

; using bp-Entry+loadseg_xxx generates smaller code than using just
; loadseg_xxx, where bp is initialized to Entry, so bp-Entry equals 0
%define loadsegoff_60   bp-Entry+loadseg_off
%define loadseg_60      bp-Entry+loadseg_seg

%define LBA_PACKET       bp-0x40
%define LBA_SIZE       word [LBA_PACKET]    ; size of packet, should be 10h
%define LBA_SECNUM     word [LBA_PACKET+2]  ; number of sectors to read
%define LBA_OFF        LBA_PACKET+4         ; buffer to read/write to
%define LBA_SEG        LBA_PACKET+6
%define LBA_SECTOR_0   word [LBA_PACKET+8 ] ; LBA starting sector #
%define LBA_SECTOR_16  word [LBA_PACKET+10]
%define LBA_SECTOR_32  word [LBA_PACKET+12]
%define LBA_SECTOR_48  word [LBA_PACKET+14]

%define READBUF 0x63A0 ; max 4KB buffer (min 2KB stack), == stacktop-0x1800
%define READADDR_OFF   BP-0x60-0x1804    ; pointer within user buffer
%define READADDR_SEG   BP-0x60-0x1802

%define PARAMS LBA_PACKET+0x10
;%define RootDirSecs     PARAMS+0x0         ; # of sectors root dir uses

%define fat_start       PARAMS+0x2         ; first FAT sector

%define root_dir_start  PARAMS+0x6         ; first root directory sector

%define data_start      PARAMS+0x0a        ; first data sector


;-----------------------------------------------------------------------
;   ENTRY
;-----------------------------------------------------------------------

real_start:
                cli
                cld
                xor     ax, ax
                mov     ds, ax
                mov     bp, BASE


                                        ; a reset should not be needed here
;               int     0x13            ; reset drive

;               int     0x12            ; get memory available in AX
;               mov     ax, 0x01e0
;               mov     cl, 6           ; move boot sector to higher memory
;               shl     ax, cl
;               sub     ax, 0x07e0

                mov     ax, 0x1FE0
                mov     es, ax
                mov     si, bp
                mov     di, bp
                mov     cx, 0x0100
                rep     movsw
                jmp     word 0x1FE0:cont

loadseg_off     dw      0
loadseg_seg     dw      LOADSEG

cont:
                mov     ds, ax
                mov     ss, ax
                lea     sp, [bp-0x60]
                sti
;
; Note: some BIOS implementations may not correctly pass drive number
; in DL, however we work around this in SYS.COM by NOP'ing out the use of DL
; (formerly we checked for [drive]==0xff; update sys.c if code moves)
;
                mov     [drive], dl     ; rely on BIOS drive number in DL

                mov     LBA_SIZE, 10h
                mov     LBA_SECNUM,1    ; initialise LBA packet constants
                mov     word [LBA_SEG],ds
                mov     word [LBA_OFF],READBUF


;       GETDRIVEPARMS:  Calculate start of some disk areas.
;
                mov     si, word [nHidden]
                mov     di, word [nHidden+2]
                add     si, word [bsResSectors]
                adc     di, byte 0              ; DI:SI = first FAT sector

                mov     word [fat_start], si
                mov     word [fat_start+2], di

                mov     al, [bsFATs]
                cbw
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

;               mov     word [RootDirSecs], ax  ; AX = sectors per root directory
                push    ax

                add     si, ax
                adc     di, byte 0              ; DI:SI = first data sector

                mov     [data_start], si
                mov     [data_start+2], di


;       FINDFILE: Searches for the file in the root directory.
;
;       Returns:
;                               AX = first cluster of file

                ; First, read the whole root directory
                ; into the temporary buffer.

                mov     ax, word [root_dir_start]
                mov     dx, word [root_dir_start+2]
                pop     di                      ; mov     di, word [RootDirSecs]
                les     bx, [loadsegoff_60] ; es:bx = 60:0
                call    readDisk
                les     di, [loadsegoff_60] ; es:di = 60:0


                ; Search for KERNEL.SYS file name, and find start cluster.

next_entry:     mov     cx, 11
                mov     si, filename
                push    di
                repe    cmpsb
                pop     di
                mov     ax, [es:di+0x1A]; get cluster number from directory entry
                je      ffDone

                add     di, byte 0x20   ; go to next directory entry
                cmp     byte [es:di], 0 ; if the first byte of the name is 0,
                jnz     next_entry      ; there is no more files in the directory

                jc      boot_error      ; fail if not found
ffDone:
                push    ax              ; store first cluster number


;       GETFATCHAIN:
;
;       Reads the FAT chain and stores it in a temporary buffer in the first
;       64 kb.  The FAT chain is stored an array of 16-bit cluster numbers,
;       ending with 0.
;
;       The file must fit in conventional memory, so it can't be larger than
;       640 kb. The sector size must be at least 512 bytes, so the FAT chain
;       can't be larger than 2.5 KB (655360 / 512 * 2 = 2560).
;
;       Call with:      AX = first cluster in chain

                les     bx, [loadsegoff_60]     ; es:bx=60:0
                mov     di, [sectPerFat]
                mov     ax, word [fat_start]
                mov     dx, word [fat_start+2]
                call    readDisk
                pop     ax                      ; restore first cluster number

                ; Set ES:DI to the temporary storage for the FAT chain.
                push    ds
                pop     es
                mov     ds, [loadseg_60]
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
                shr     ax, cl

fat_even:       and     ah, 0x0f        ; mask off the highest 4 bits
                cmp     ax, 0x0ff8      ; check for EOF
                jb      next_clust      ; continue if not EOF

%endif
%ifdef ISFAT16
                ; This is a FAT-16 disk. The maximal size of a 16-bit FAT
                ; is 128 kb, so it may not fit within a single 64 kb segment.

fat_16:         mov     dx, [loadseg_60]
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


;       loadFile: Loads the file into memory, one cluster at a time.

                les     bx, [loadsegoff_60]   ; set ES:BX to load address 60:0

                mov     si, FATBUF      ; set DS:SI to the FAT chain

cluster_next:   lodsw                           ; AX = next cluster to read
                or      ax, ax                  ; EOF?
                jne     load_next               ; no, continue
                mov     bl,dl ; drive (left from readDisk)
                jmp     far [loadsegoff_60]     ; yes, pass control to kernel

load_next:      dec     ax                      ; cluster numbers start with 2
                dec     ax

                mov     di, word [bsSecPerClust]
                and     di, 0xff                ; DI = sectors per cluster
                mul     di
                add     ax, [data_start]
                adc     dx, [data_start+2]      ; DX:AX = first sector to read
                call    readDisk
                jmp     short cluster_next

; shows text after the call to this function.

show.do_show:
                mov     ah, 0Eh                 ; show character
                int     10h                     ; via "TTY" mode
show:           pop     si
                lodsb                           ; get character
                push    si                      ; stack up potential return address
                cmp     al, 0                   ; end of string?
                jne     .do_show                ; until done
                ret

boot_error:     call    show
;                db      "Error! Hit a key to reboot.",0
                db      "Error!",0

                xor     ah,ah
                int     0x13                    ; reset floppy
                int     0x16                    ; wait for a key
                int     0x19                    ; reboot the machine


;       readDisk:       Reads a number of sectors into memory.
;
;       Call with:      DX:AX = 32-bit DOS sector number
;                       DI = number of sectors to read
;                       ES:BX = destination buffer
;
;       Returns:        CF set on error
;                       ES:BX points one byte after the last byte read.

readDisk:       push    si

                mov     LBA_SECTOR_0,ax
                mov     LBA_SECTOR_16,dx
                mov     word [READADDR_SEG], es
                mov     word [READADDR_OFF], bx

                call    show
                db      ".",0
read_next:

;******************** LBA_READ *******************************

                                                ; check for LBA support
                                                                                
                mov     ah,041h                 ;
                mov     bx,055aah               ;
                mov     dl, [drive]

                ; NOTE: sys must be updated if location changes!!!
                test    dl,dl                   ; don't use LBA addressing on A:
                jz      read_normal_BIOS        ; might be a (buggy)
                                                ; CDROM-BOOT floppy emulation

                int     0x13
                jc      read_normal_BIOS

                shr     cx,1                    ; CX must have 1 bit set

                sbb     bx,0aa55h - 1           ; tests for carry (from shr) too!
                jne     read_normal_BIOS
                
                                
                                                ; OK, drive seems to support LBA addressing

                lea     si,[LBA_PACKET]
                            
                                                ; setup LBA disk block                                  
                mov     LBA_SECTOR_32,bx        ; bx is 0 if extended 13h mode supported
                mov     LBA_SECTOR_48,bx
        
                mov     ah,042h
                jmp short    do_int13_read

                                                        

read_normal_BIOS:      

;******************** END OF LBA_READ ************************
                mov     cx,LBA_SECTOR_0
                mov     dx,LBA_SECTOR_16


                ;
                ; translate sector number to BIOS parameters
                ;

                ;
                ; abs = sector                          offset in track
                ;     + head * sectPerTrack             offset in cylinder
                ;     + track * sectPerTrack * nHeads   offset in platter
                ;
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
                or      cl, ah                  ; merge sector into cylinder
                inc     cx                      ; make sector 1-based (1-63)

                les     bx,[LBA_OFF]
                mov     ax, 0x0201
do_int13_read:                
                mov     dl, [drive]
                int     0x13
                jc      boot_error              ; exit on error

                mov     ax, word [bsBytesPerSec]  

                push    di
                mov     si,READBUF              ; copy read in sector data to
                les     di,[READADDR_OFF]       ; user provided buffer
                mov     cx, ax
;                shr     cx, 1                   ; convert bytes to word count
;                rep     movsw
                rep     movsb
                pop     di

;               div     byte[LBA_PACKET]        ; luckily 16 !!
                mov     cl, 4
                shr     ax, cl                  ; adjust segment pointer by increasing
                add     word [READADDR_SEG], ax ; by paragraphs read in (per sector)

                add     LBA_SECTOR_0,  byte 1
                adc     LBA_SECTOR_16, byte 0   ; DX:AX = next sector to read
                dec     di                      ; if there is anything left to read,
                jnz     read_next               ; continue

                les     bx, [READADDR_OFF]
                ; clear carry: unnecessary since adc clears it
                pop     si
                ret

       times   0x01f1-$+$$ db 0

filename        db      "KERNEL  SYS",0,0

sign            dw      0xAA55

%ifdef DBGPRNNUM
; DEBUG print hex digit routines
PrintLowNibble:         ; Prints low nibble of AL, AX is destroyed
        and  AL, 0Fh    ; ignore upper nibble
        cmp  AL, 09h    ; if greater than 9, then don't base on '0', base on 'A'
        jbe .printme
        add  AL, 7      ; convert to character A-F
        .printme:
        add  AL, '0'    ; convert to character 0-9
        mov  AH,0x0E    ; show character
        int  0x10       ; via "TTY" mode
        retn
PrintAL:                ; Prints AL, AX is preserved
        push AX         ; store value so we can process a nibble at a time
        shr  AL, 4              ; move upper nibble into lower nibble
        call PrintLowNibble
        pop  AX         ; restore for other nibble
        push AX         ; but save so we can restore original AX
        call PrintLowNibble
        pop  AX         ; restore for other nibble
        retn
PrintNumber:            ; Prints (in Hex) value in AX, AX is preserved
        xchg AH, AL     ; high byte 1st
        call PrintAL
        xchg AH, AL     ; now low byte
        call PrintAL
        retn
%endif

