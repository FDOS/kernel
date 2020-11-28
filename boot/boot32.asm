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
%define xsectPerFat     bp+0x24      ; Sectors/Fat
%define xrootClst       bp+0x2c      ; Starting cluster of root directory
%define drive           bp+0x40      ; Drive number

		times   52h - ($ - $$) db 0
		; The filesystem ID is used by lDOS's instsect (by ecm)
		;  by default to validate that the filesystem matches.
		db "FAT32"
		times   5Ah - ($ - $$) db 32


%define LOADSEG         0x0060

%define FATSEG          0x2000

%define fat_sector      bp+0x48         ; last accessed sector of the FAT

%define loadsegoff_60	bp+loadseg_off-Entry ; FAR pointer = 60:0
%define loadseg_60	bp+loadseg_seg-Entry

%define fat_start       bp+0x5e         ; first FAT sector
%define data_start      bp+0x62         ; first data sector
%define fat_secmask     bp+0x66		; number of clusters in a FAT sector - 1
%define fat_secshift    bp+0x68         ; fat_secmask+1 = 2^fat_secshift

;-----------------------------------------------------------------------
;   ENTRY
;-----------------------------------------------------------------------

real_start:     cld
		cli
                sub	ax, ax
		mov	ds, ax
                mov     bp, 0x7c00

		mov	ax, 0x1FE0
		mov	es, ax
		mov	si, bp
		mov	di, bp
		mov	cx, 0x0100
		rep	movsw           ; move boot code to the 0x1FE0:0x0000
		jmp     word 0x1FE0:cont

loadseg_off	dw	0
loadseg_seg	dw	LOADSEG

cont:           mov     ds, ax
                mov     ss, ax
                lea     sp, [bp-0x20]
		sti
                mov     [drive], dl     ; BIOS passes drive number in DL

;                call    print
;                db      "Loading ",0

;      Calc Params
;      Fat_Start
		mov	si, word [nHidden]
		mov	di, word [nHidden+2]
		add	si, word [bsResSectors]
		adc	di, byte 0

		mov	word [fat_start], si
		mov	word [fat_start+2], di
 ;	Data_Start
		mov	al, [bsFATs]
		cbw
		push	ax
		mul	word [xsectPerFat+2]
		add	di, ax
		pop	ax
		mul	word [xsectPerFat]
		add	ax, si
		adc	dx, di
		mov	word[data_start], ax
		mov	word[data_start+2], dx
;      fat_secmask
		mov	ax, word[bsBytesPerSec]
		shr	ax, 1
		shr	ax, 1
		dec	ax
		mov	word [fat_secmask], ax
;      fat_secshift
; cx = temp
; ax = fat_secshift
		xchg	ax, cx ; cx = 0 after movsw
		inc	cx
secshift:	inc	ax
		shr	cx, 1
		cmp	cx, 1
		jne	secshift
		mov	byte [fat_secshift], al
		dec	cx
 
;       FINDFILE: Searches for the file in the root directory.
;
;       Returns:
;            DX:AX = first cluster of file

                mov     word [fat_sector], cx           ; CX is 0 after "dec"
                mov     word [fat_sector + 2], cx

                mov     ax, word [xrootClst]
                mov     dx, word [xrootClst + 2]
ff_next_cluster:
                push    dx                              ; save cluster
                push    ax
                call    convert_cluster
                jc      boot_error                      ; EOC encountered
                                
ff_next_sector:
                push    bx                              ; save sector count

                les     bx, [loadsegoff_60]
                call    readDisk
                push    dx                              ; save sector
                push    ax

                mov     ax, [bsBytesPerSec]

		; Search for KERNEL.SYS file name, and find start cluster.
ff_next_entry:  mov     cx, 11
                mov     si, filename
                mov     di, ax
                sub     di, 0x20
                repe    cmpsb
                jz      ff_done

                sub     ax, 0x20
                jnz     ff_next_entry
                pop     ax                      ; restore  sector
                pop     dx
                pop     bx                      ; restore sector count
                dec     bx
                jnz     ff_next_sector
ff_find_next_cluster:
                pop     ax                      ; restore current cluster
                pop     dx
                call    next_cluster
                jmp     short ff_next_cluster
ff_done:
                
                mov     ax, [es:di+0x1A-11]        ; get cluster number
                mov     dx, [es:di+0x14-11]
c4:
                sub     bx, bx                  ; ES points to LOADSEG      
c5:             push    dx
                push    ax
                push    bx
                call    convert_cluster
                jc      boot_success
                mov     di, bx
                pop     bx
c6:
                call    readDisk
                dec     di
                jnz     c6
                pop     ax
                pop     dx
                call    next_cluster
                jmp     short c5
                
boot_error:
		xor	ah,ah
		int	0x16			; wait for a key
		int	0x19			; reboot the machine

; input: 
;    DX:AX - cluster
; output:
;    DX:AX - next cluster
;    CX = 0
; modify:
;    DI
next_cluster:  
                push    es
                mov     di, ax
                and     di, [fat_secmask]
                
                mov     cx, [fat_secshift]
cn_loop:
                shr     dx,1
                rcr     ax,1
                dec     cx
                jnz     cn_loop                ; DX:AX fat sector where our
                                               ; cluster resides
                                               ; DI - cluster index in this
                                               ; sector
                                               
                shl     di,1                   ; DI - offset in the sector
                shl     di,1
                add     ax, [fat_start]
                adc     dx, [fat_start+2]      ; DX:AX absolute fat sector

                push    bx
                mov     bx, FATSEG
                mov     es, bx
                sub     bx, bx

                cmp     ax, [fat_sector]
                jne     cn1                    ; if the last fat sector we
                                               ; read was this, than skip
                cmp     dx,[fat_sector+2]
                je      cn_exit
cn1:
                mov     [fat_sector],ax        ; save the fat sector number,
                mov     [fat_sector+2],dx      ; we are going to read
                call    readDisk
cn_exit:
                pop     bx
                mov     ax, [es:di]             ; DX:AX - next cluster
                mov     dx, [es:di + 2]         ;
                pop     es
                ret


boot_success:   
                mov     bl, [drive]
		jmp	far [loadsegoff_60]

; Convert cluster to the absolute sector
;input:
;    DX:AX - target cluster
;output:
;    DX:AX - absoulute sector
;    BX - [bsSectPerClust]
;modify:
;    CX
convert_cluster:
                cmp     dx,0x0fff
                jne     c3
                cmp     ax,0xfff8
                jb      c3              ; if cluster is EOC (carry is set), do ret
                stc
                ret
c3:
                mov     cx, dx          ; sector = (cluster - 2)*clussize +
                                        ; + data_start
                sub     ax, 2
                sbb     cx, byte 0           ; CX:AX == cluster - 2
                mov     bl, [bsSecPerClust]
                sub     bh, bh
                xchg    cx, ax          ; AX:CX == cluster - 2
                mul     bx              ; first handle high word
                                        ; DX must be 0 here
                xchg    ax, cx          ; then low word
                mul     bx
                add     dx, cx                          ; DX:AX target sector
                add     ax, [data_start]
                adc     dx, [data_start + 2]
                ret

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
                
;input:
;   DX:AX - 32-bit DOS sector number
;   ES:BX - destination buffer
;output:
;   ES:BX points one byte after the last byte read.
;   DX:AX - next sector
;modify:
;   ES if DI * bsBytesPerSec >= 65536, CX

readDisk:
read_next:      push    dx
                push    ax
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
                inc     ah                      ; sector offset from 1
                or      cl, ah                  ; merge sector into cylinder

                mov     ax, 0x0201
                mov     dl, [drive]
                int     0x13

                pop     ax
                pop     dx         
                jnc     read_ok                 ; jump if no error
                xor     ah, ah                  ; else, reset floppy
                int     0x13
                jmp     short read_next
read_ok:
                add     bx, word [bsBytesPerSec]

                jnc     no_incr_es              ; if overflow...

                mov     cx, es
                add     ch, 0x10                ; ...add 1000h to ES
                mov     es, cx

no_incr_es:
                add     ax,byte 1
                adc     dx,byte 0
                ret

       times   0x01f1-$+$$ db 0

filename        db      "KERNEL  SYS",0,0

sign            dw      0xAA55
