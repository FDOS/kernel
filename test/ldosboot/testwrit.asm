
%if 0

Write success indicator after file system booting
 by C. Masloch, 2020

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif


%assign __lMACROS1_MAC__DEBUG_DEFAULTS 1
%include "lmacros3.mac"

	struc BS
bsJump:	resb 3
bsOEM:	resb 8
bsBPB:
	endstruc

	struc EBPB		;        BPB sec
bpbBytesPerSector:	resw 1	; offset 00h 0Bh
bpbSectorsPerCluster:	resb 1	; offset 02h 0Dh
bpbReservedSectors:	resw 1	; offset 03h 0Eh
bpbNumFATs:		resb 1	; offset 05h 10h
bpbNumRootDirEnts:	resw 1	; offset 06h 11h -- 0 for FAT32
bpbTotalSectors:	resw 1	; offset 08h 13h
bpbMediaID:		resb 1	; offset 0Ah 15h
bpbSectorsPerFAT:	resw 1	; offset 0Bh 16h -- 0 for FAT32
bpbCHSSectors:		resw 1	; offset 0Dh 18h
bpbCHSHeads:		resw 1	; offset 0Fh 1Ah
bpbHiddenSectors:	resd 1	; offset 11h 1Ch
bpbTotalSectorsLarge:	resd 1	; offset 15h 20h
bpbNew:				; offset 19h 24h

ebpbSectorsPerFATLarge:	resd 1	; offset 19h 24h
ebpbFSFlags:		resw 1	; offset 1Dh 28h
ebpbFSVersion:		resw 1	; offset 1Fh 2Ah
ebpbRootCluster:	resd 1	; offset 21h 2Ch
ebpbFSINFOSector:	resw 1	; offset 25h 30h
ebpbBackupSector:	resw 1	; offset 27h 32h
ebpbReserved:		resb 12	; offset 29h 34h
ebpbNew:			; offset 35h 40h
	endstruc

	struc BPBN		; ofs B16 S16 B32 S32
bpbnBootUnit:		resb 1	; 00h 19h 24h 35h 40h
			resb 1	; 01h 1Ah 25h 36h 41h
bpbnExtBPBSignature:	resb 1	; 02h 1Bh 26h 37h 42h -- 29h for valid BPBN
bpbnSerialNumber:	resd 1	; 03h 1Ch 27h 38h 43h
bpbnVolumeLabel:	resb 11	; 07h 20h 2Bh 3Ch 47h
bpbnFilesystemID:	resb 8	; 12h 2Bh 36h 47h 52h
	endstruc		; 1Ah 33h 3Eh 4Fh 5Ah

	struc LOADSTACKVARS, -10h
lsvFirstCluster:	resd 1
lsvFATSector:		resd 1
lsvFATSeg:		resw 1
lsvLoadSeg:		resw 1
lsvDataStart:		resd 1
	endstruc

lsvclSignature		equ "CL"
lsvclBufferLength	equ 256

	struc LOADDATA, LOADSTACKVARS - 10h
ldMemoryTop:	resw 1
ldLoadTop:	resw 1
ldSectorSeg:	resw 1
ldFATType:	resb 1
ldHasLBA:	resb 1
ldClusterSize:	resw 1
ldParaPerSector:resw 1
ldLoadingSeg:	resw 1
ldLoadUntilSeg:	resw 1
	endstruc

	struc LOADCMDLINE, LOADDATA - lsvclBufferLength
ldCommandLine:
.start:		resb lsvclBufferLength
	endstruc

	struc LBAPACKET
lpSize:		resw 1
lpCount:	resw 1
lpBuffer:	resd 1
lpSector:	resq 1
	endstruc

	struc DIRENTRY
deName:		resb 8
deExt:		resb 3
deAttrib:	resb 1
		resb 8
deClusterHigh:	resw 1
deTime:		resw 1
deDate:		resw 1
deClusterLow:	resw 1
deSize:		resd 1
	endstruc

ATTR_READONLY	equ 1
ATTR_HIDDEN	equ 2
ATTR_SYSTEM	equ 4
ATTR_VOLLABEL	equ 8
ATTR_DIRECTORY	equ 10h
ATTR_ARCHIVE	equ 20h


%ifndef _MAP
%elifempty _MAP
%else	; defined non-empty, str or non-str
	[map all _MAP]
%endif

	defaulting

	numdef QUERY_GEOMETRY,	1	; query geometry via 13.08 (for CHS access)
	numdef CHS,		1	; support CHS (if it fits)
	numdef LBA,		1	; support LBA (if available)
	numdef LBA_33_BIT,	1	; support 33-bit LBA
	numdef LBA_CHECK_NO_33,	1	; else: check that LBA doesn't carry
	numdef LBA_SKIP_CHECK,	0	; don't use proper LBA extensions check
	numdef LBA_RETRY,	1	; retry LBA reads
	numdef CHS_RETRY,	1	; retry CHS reads

	numdef PADDING, 0
	strdef FILE_NAME,	"RESULT"
	strdef FILE_EXT,	"TXT"	; name of file to write
	strdef FILE_SUCCESS_MSG,"success"


	cpu 8086
	org 0
start:
	push ss
	pop ds
	mov word [cs:dirsp], sp

	mov ax, cs
	cmp ax, 200h + 200h		; loaded high ?
	mov dx, 200h			; yes, use low buffer
	jae @F
	add ax, paras(end - $$)		; => behind used load image
	mov dx, ax
	neg ax				; - behind used
	add ax, word [bp + ldLoadTop]	; top - used = length of free
	cmp ax, 200h			; enough ?
	jb error_outofmemory		; no -->
		; (For simplicity we do not attempt to relocate our load image.)
@@:
	mov word [cs:dirbuf], dx

%if _QUERY_GEOMETRY || !_LBA_SKIP_CHECK
	call query_geometry
		; The ebpbNew BPBN needs to be initialised
		;  to use this function. It must be called
		;  before using read_sector.
%endif

	cmp byte [bp + ldFATType], 16
	ja search32
	cmp byte [bp + ldFATType], 0
	ja search1216
	call error
	asciz "Unknown file system (ldFATType = 0)."

search1216:
	xor cx, cx
	mov bx, word [bp + ldParaPerSector]
	shr bx, 1			; = entries per sector

; number of sectors used for root directory (store in CX)
	mov si, [bp + bsBPB + bpbNumRootDirEnts]
	mov ax, bx
		; The ax value here is the last value of bx, which is set
		;  by the shr instruction. Therefore, it cannot be higher
		;  than 7FFFh, so this cwd instruction always zeros dx.
	cwd
	dec ax				; rounding up
	add ax, si			; from BPB
	adc dx, dx			; account for overflow (dx was zero)
	div bx				; get number of root sectors
	xchg ax, cx			; cx = number of root secs, ! ah = 0

; first sector of root directory
	mov al, [bp + bsBPB + bpbNumFATs]; ! ah = 0, hence ax = number of FATs
	mul word [bp + bsBPB + bpbSectorsPerFAT]
	add ax, [bp + bsBPB + bpbReservedSectors]
	adc dl, dh			; account for overflow (dh was and is 0)

; Scan root directory for file. We don't bother to check for deleted
;  entries (E5h) or entries that mark the end of the directory (00h).
		; number of root entries in si here
.next_sect:
	mov cx, bx		; entries per sector as loop counter
	call read_sector_to_dirbuf
	mov bx, cx		; restore bx for next iteration later

	xor di, di		; es:di-> first entry in this sector
.next_ent:
	call check_entry
	dec si			; count down entire root's entries
	loopnz .next_ent	; count down sector's entries (jumps iff si >0 && cx >0)
	jnz .next_sect		; (jumps iff si >0 && cx ==0)
				; ends up here iff si ==0
				;  ie all root entries checked unsuccessfully
	jmp error_filenotfound


search32:
	mov ax, [bp + bsBPB + ebpbRootCluster]
	mov dx, [bp + bsBPB + ebpbRootCluster + 2]
	mov si, word [bp + lsvFATSector + 2]
	mov di, word [bp + lsvFATSector]
	call check_clust
	jc error_filenotfound

.next_root_clust:
	call clust_to_first_sector
	push cx
	push bx
	mov cx, [bp + ldClusterSize]
.next_root_sect:
	push cx
	mov cx, [bp + ldParaPerSector]
	shr cx, 1

; Scan root directory for file. We don't bother to check for deleted
;  entries (E5h) or entries that mark the end of the directory (00h).
	call read_sector_to_dirbuf

	push di
	xor di, di		; es:di-> first entry in this sector
.next_ent:
	call check_entry
	loop .next_ent		; count down sector's entries (jumps iff cx >0)
	pop di
	pop cx
	loop .next_root_sect
	pop bx
	pop cx
	call clust_next
	jnc .next_root_clust
	jmp error_filenotfound


read_sector_to_dirbuf:
	mov bx, word [cs:dirbuf]
	mov word [cs:dirsec], ax
	mov word [cs:dirsec + 2], dx
	jmp read_sector

check_entry:
	test byte [es:di + deAttrib], ATTR_DIRECTORY | ATTR_VOLLABEL
	jnz @F			; directory, label, or LFN entry --> (NZ)
	push si
	push di
	push cx
	push ds
	 push cs
	 pop ds
	mov si, msg.filename	; ds:si-> name to match
	mov cx, 11		; length of padded 8.3 FAT filename
	repe cmpsb		; check entry
	pop ds
	pop cx
	pop di
	pop si
@@:				; ZR = match, NZ = mismatch
	je found_it		; found entry -->
	lea di, [di + DIRENTRY_size]
	retn

found_it:
	mov sp, word [cs:dirsp]
	mov word [cs:dirofs], di

	cmp word [es:di + deSize], 0
	jne @F
	cmp word [es:di + deSize + 2], 0
	jne @F

error_emptyfile: equ $
	call error
	asciz "File is empty."

@@:
; get starting cluster of file
	mov ax, [es:di + deClusterLow]
	mov dx, [es:di + deClusterHigh]	; dx:ax = first cluster
	cmp byte [bp + ldFATType], 16
	ja @F
	xor dx, dx
@@:

	mov di, [bp + lsvFATSector]
	mov si, [bp + lsvFATSector + 2]
	call check_clust
	jc error_emptyfile

	call clust_to_first_sector
	mov bx, cs
	add bx, paras(filbuf - $$)
	call write_sector

	mov es, word [cs:dirbuf]
	mov bx, word [cs:dirofs]
	mov word [es:bx + deSize], filbuf.size
	and word [es:bx + deSize + 2], 0
	mov bx, es
	mov ax, word [cs:dirsec]
	mov dx, word [cs:dirsec + 2]
	call write_sector

	push cs
	pop ds
	mov si, msg.success
	call disp_error

quit:
	int3

	mov ax, 0F000h
	mov es, ax
	 push cs
	 pop ds				; avoid "repe cs cmpsw" (8086 bug)
	mov di, 0FFF5h
	mov si, msg.dosemudate
	mov cx, 4
	repe cmpsw			; running in DosEmu?
	jne .quit_not_dosemu

	xor bx, bx
	mov ax, -1
	int 0E6h			; dosemu quit

.quit_not_dosemu:

; from https://stackoverflow.com/a/5240330/738287
	mov ax, 5301h
	xor bx, bx
	int 15h				; connect to APM API

	mov ax, 530Eh
	xor bx, bx
	mov cx, 0102h
	int 15h				; set APM version to 1.02

	mov ax, 5307h
	mov bx, 1
	mov cx, 3
	int 15h				; shut down system

	; setopt [cs:quitrecurse], 1
	call error
	asciz "Quit failed."


error:
	push cs
	pop ds
	mov si, msg.error
	call disp_error
	pop si
	call disp_error
	mov si, msg.error.trailer
	call disp_error
%if 0
	testopt [cs:quitrecurse], 1
	jz quit
%endif
	int3
	xor ax, ax
	int 16h
	int 19h

disp_error:
.:
	lodsb
	test al, al
	jz .ret
	mov ah, 0Eh
	mov bx, 7
	push bp
		; (call may change bp)
	int 10h
	pop bp
	jmp short .


	align 4
dirsec:		dd 0
dirofs:		dw 0
dirbuf:		dw 0
dirsp:		dw 0
; quitrecurse:	db 0

msg:
.error:		asciz "Load error: "
.success:	db "Test writer loaded successfully. Quitting the machine."
.error.trailer:	asciz 13,10
.filename:	fill 8, 32, db _FILE_NAME
		fill 3, 32, db _FILE_EXT
	align 4
.dosemudate:	db "02/25/93"

	align 16, db 38
filbuf:
.:
	db _FILE_SUCCESS_MSG,13,10
.size: equ $ - .
%if .size > 32
 %error File success message is too long
%endif
	_fill 8192, 38, .


query_geometry:
%if _QUERY_GEOMETRY	; +30 bytes
	mov dl, [bp + bsBPB + ebpbNew + bpbnBootUnit]
 %if !_LBA_SKIP_CHECK
	push dx
 %endif
;	test dl, dl		; floppy?
;	jns @F			; don't attempt query, might fail -->
	; Note that while the original PC BIOS doesn't support this function
	;  (for its diskettes), it does properly return the error code 01h.
	; https://sites.google.com/site/pcdosretro/ibmpcbios (IBM PC version 1)
	mov ah, 08h
	xor cx, cx		; initialise cl to 0
	stc			; initialise to CY
	int 13h			; query drive geometry
	jc @F			; apparently failed -->
	and cx, 3Fh		; get sectors
	jz @F			; invalid (S is 1-based), don't use -->
	mov [bp + bsBPB + bpbCHSSectors], cx
	mov cl, dh		; cx = maximum head number
	inc cx			; cx = number of heads (H is 0-based)
	mov [bp + bsBPB + bpbCHSHeads], cx
@@:
%endif

%if !_LBA_SKIP_CHECK
	mov ah, 41h
 %if _QUERY_GEOMETRY
	pop dx
 %else
	mov dl, [bp + bsBPB + ebpbNew + bpbnBootUnit]
 %endif
	mov bx, 55AAh
	stc
	int 13h		; 13.41.bx=55AA extensions installation check
	mov al, 0	; zero in case of no LBA support
	jc .no_lba
	cmp bx, 0AA55h
	jne .no_lba
	test cl, 1	; support bitmap bit 0
	jz .no_lba
	inc ax		; al = 1 to indicate LBA support
.no_lba:
	mov byte [bp + ldHasLBA], al
%endif

%if 1 || _QUERY_GEOMETRY || !_LBA_SKIP_CHECK
disp_error.ret:
	retn
%endif


		; Read a sector using Int13.02 or Int13.42
		;
		; INP:	dx:ax = sector number within partition
		;	bx:0-> buffer
		;	(_LBA) ds = ss
		; OUT:	If unable to read,
		;	 ! jumps to error instead of returning
		;	If sector has been read,
		;	 dx:ax = next sector number (has been incremented)
		;	 bx:0-> next buffer (bx = es+word[para_per_sector])
		;	 es = input bx
		; CHG:	-
		; STT:	ds = ss
		;
		; Note:	If error 09h (data boundary error) is returned,
		;	 the read is done into the ldSectorSeg buffer,
		;	 then copied into the user buffer.
read_sector:
	clropt [bp + ldHasLBA], 2
	jmp @F

write_sector:
	setopt [bp + ldHasLBA], 2
@@:

	push dx
	push cx
	push ax
	push si

	mov es, bx

; DX:AX==LBA sector number
; add partition start (= number of hidden sectors)
		add ax,[bp + bsBPB + bpbHiddenSectors + 0]
		adc dx,[bp + bsBPB + bpbHiddenSectors + 2]

 %if (!_LBA || !_LBA_33_BIT) && _LBA_CHECK_NO_33
	jc .err_CY
 %endif
%if _LBA		; +70 bytes (with CHS, +63 bytes without CHS)
 %if _LBA_33_BIT
	sbb si, si	; -1 if was CY, 0 else
	neg si		; 1 if was CY, 0 else
 %endif
	xor cx, cx	; cx = 0 (needed if jumping to .no_lba_checked)
 %if !_LBA_SKIP_CHECK
	test byte [bp + ldHasLBA], 1
	jz .no_lba_checked
 %endif
	push cx
 %if _LBA_33_BIT
	push si		; bit 32 = 1 if operating in 33-bit space
 %else
	push cx		; second highest word = 0
 %endif
	push dx
	push ax		; qword sector number (lpSector)
	push bx
	push cx		; bx:0 -> buffer (lpBuffer)
	inc cx
	push cx		; word number of sectors to read (lpCount)
	mov cl, 10h
	push cx		; word size of disk address packet (lpSize)
	mov si, sp	; ds:si -> disk address packet (on stack)

	mov dl, [bp + bsBPB + ebpbNew + bpbnBootUnit]
	call .get_ah_3_write_2_read
	or ah, 40h	; 42h extensions read or 43h extensions write
%if _LBA_RETRY
	call .int13_retry
%else
	call .int13_preserve_lpcount
%endif
	jnc .lba_done

%if _LBA_SKIP_CHECK
	cmp ah, 1	; invalid function?
	je .no_lba_skip	; try CHS instead -->
%endif
	cmp ah, 9	; data boundary error?
	jne .lba_error

	testopt [bp + ldHasLBA], 2
	jz @F

			; es => user buffer
	push es
	call .sectorseg_helper_write
	mov word [si + 4 + 2], es	; => sector buffer
	mov ah, 43h
%if _LBA_RETRY
	call .int13_retry
%else
	int 13h
		; (don't need .int13_preserve_lpcount as no further call)
%endif
	pop es		; ! restore es => user buffer
	jc .lba_error
	jmp .lba_done

@@:
	; push word [si + 4 + 0]
	push es		; => user buffer
	 mov es, word [bp + ldSectorSeg]
	 mov word [si + 4 + 2], es
			; => sector buffer
	; and word [si + 4 + 0], byte 0

	mov ah, 42h
%if _LBA_RETRY
	call .int13_retry
%else
	int 13h
		; (don't need .int13_preserve_lpcount as no further call)
%endif
	jc .lba_error

	pop es		; => user buffer
	; pop cx
	call .sectorseg_helper_read

.lba_done:
	add sp, 10h
	jmp short .done

.lba_error: equ .err

 %if !_CHS
.no_lba_skip: equ .err
.no_lba_checked: equ .err
 %elif _LBA_SKIP_CHECK
.no_lba_skip:
		; si == sp
	add sp, 8
	pop ax
	pop dx
  %if _LBA_33_BIT
	pop si
	pop cx		; cx = 0 (needed as input for next cwd instruction)
	test si, si
	mov si, sp	; si == sp
  %else
	pop cx
	pop cx
		; si == sp - 16
  %endif
 %else
.no_lba_checked:
  %if _LBA_33_BIT
	test si, si
  %endif
	mov si, sp	; si == sp
 %endif
%endif

%if _CHS		; +70 bytes
 %if _LBA && _LBA_33_BIT
	jnz .err
 %endif
; dx:ax = LBA sector number, (if _LBA) cx = 0
; divide by number of sectors per track to get sector number
; Use 32:16 DIV instead of 64:32 DIV for 8088 compatability
; Use two-step 32:16 divide to avoid overflow
 %if !_LBA
			xchg cx, ax	; cx = low word of sector, clobbers ax
			xchg ax, dx	; ax = high word of sector, clobbers dx
			xor dx, dx	; dx:ax = high word of sector
 %else
			xchg cx, ax	; cx = low word of sector, ax = 0
			push dx		; stack = high word of sector
			cwd		; dx = 0 (because ax was 0)
			pop ax		; ax = high word of sector
					; dx:ax = high word of sector
 %endif
			div word [bp + bsBPB + bpbCHSSectors]
			xchg cx,ax
			div word [bp + bsBPB + bpbCHSSectors]
			xchg cx,dx

; DX:AX=quotient, CX=remainder=sector (S) - 1
; divide quotient by number of heads
			xchg bx, ax	; bx = low word of quotient, clobbers ax
			xchg ax, dx	; ax = high word of quotient, clobbers dx
			xor dx, dx	; dx = 0
			div word [bp + bsBPB + bpbCHSHeads]
					; ax = high / heads, dx = high % heads
			xchg bx, ax	; bx = high / heads, ax = low quotient
			div word [bp + bsBPB + bpbCHSHeads]

; bx:ax=quotient=cylinder (C), dx=remainder=head (H)
; move variables into registers for INT 13h AH=02h
			mov dh, dl	; dh = head
			inc cx		; cl5:0 = sector
			xchg ch, al	; ch = cylinder 7:0, al = 0
			shr ax, 1
			shr ax, 1	; al7:6 = cylinder 9:8
	; bx has bits set iff it's > 0, indicating a cylinder >= 65536.
			 or bl, bh	; collect set bits from bh
			or cl, al	; cl7:6 = cylinder 9:8
	; ah has bits set iff it was >= 4, indicating a cylinder >= 1024.
			 or bl, ah	; collect set bits from ah
			mov dl, [bp + bsBPB + ebpbNew + bpbnBootUnit]
					; dl = drive
			 jnz .err	; error if cylinder >= 1024 -->
					; ! bx = 0 (for 13.02 call)

; we call INT 13h AH=02h once for each sector. Multi-sector reads
; may fail if we cross a track or 64K boundary

	call .get_ah_3_write_2_read
	mov al, 01h	; access one sector

%if _CHS_RETRY
			call .int13_retry
%else
			int 13h
%endif
			jnc .done

	cmp ah, 9	; data boundary error?
	jne .err

	testopt [bp + ldHasLBA], 2
	jz @F

	push es		; user buffer
	call .sectorseg_helper_write
	mov ax, 0301h
%if _LBA_RETRY
	call .int13_retry
%else
	int 13h
%endif
	jc .err
	pop es
	jmp .done

@@:
	push es		; user buffer
	 mov es, word [bp + ldSectorSeg]

	mov ax, 0201h
%if _CHS_RETRY
	call .int13_retry
%else
	int 13h
%endif
	jc .err

	pop es
	call .sectorseg_helper_read
%endif		; _CHS

.done:
; increment segment
	mov bx, es
	add bx, word [bp + ldParaPerSector]

	pop si
	pop ax
	pop cx
	pop dx
; increment LBA sector number
	inc ax
	jne @F
	inc dx
@@:
	retn


%if (_LBA && _LBA_RETRY) || (_CHS && _CHS_RETRY)
.int13_retry:
	push ax
%if _LBA
	call .int13_preserve_lpcount
%else
	int 13h		; first try
%endif
	jnc @F		; NC, success on first attempt -->

; reset drive
	xor ax, ax
	int 13h
	jnc @FF		; NC, reset succeeded -->
			; CY, reset failed, error in ah

@@:			; NC or CY, stack has function number
	inc sp
	inc sp		; discard word on stack, preserve CF
	retn

@@:
; try read again
	pop ax		; restore function number
%if ! _LBA
	int 13h		; retry, CF error status, ah error number
	retn
%endif		; else: fall through to .int13_preserve_lpcount
%endif

%if _LBA
		; have to reset the LBAPACKET's lpCount, as the handler may
		;  set it to "the number of blocks successfully transferred".

		; hack: si points into unclaimed stack space
		;  when this is called from the CHS handler.
		;  this should not cause any issues however.
		; actually, if !_LBA_SKIP_CHECK, then si is set
		;  to point to claimed stack space. also legal.
.int13_preserve_lpcount:
	push word [si + lpCount]
	int 13h
	pop word [si + lpCount]
	retn
%endif

		; INP:	word [bp + ldSectorSeg] => source buffer with read data
		;	es => destination buffer
		; CHG:	si, cx
		; OUT:	ss = ds
		;	data copied to destination buffer
		; STT:	UP
.sectorseg_helper_read:
	xor si, si
	mov ds, word [bp + ldSectorSeg]
	 push di
	; mov di, cx
	xor di, di
	mov cx, word [bp + bsBPB + bpbBytesPerSector]
	rep movsb
	 pop di

	push ss
	pop ds
	retn

		; INP:	es => source buffer with data to read
		;	word [bp + ldSectorSeg] => destination buffer
		; CHG:	-
		; OUT:	ss = ds
		;	es => destination buffer
		;	data copied to destination buffer
		; STT:	UP
.sectorseg_helper_write:
	push es
	pop ds
	 push si
	 push di
	 push cx
	xor di, di
	mov es, word [bp + ldSectorSeg]
	xor si, si
	mov cx, word [bp + bsBPB + bpbBytesPerSector]
	rep movsb
	 pop cx
	 pop di
	 pop si

	push ss
	pop ds
	retn

.get_ah_3_write_2_read:
	mov ah, 02h	; read
	testopt [bp + ldHasLBA], 2
	jz @F
	mov ah, 03h	; write
@@:
	retn

.err:
error_diskaccess:
	testopt [bp + ldHasLBA], 2
	jz @F
	call error
	db "Disk write error.", 0

@@:
	call error
	db "Disk read error.", 0

error_badchain:
	call error
	db "Bad cluster chain.", 0

error_badclusters:
	call error
	db "Bad amount of clusters.", 0

error_outofmemory:
	call error
	db "Out of memory.", 0

error_filenotfound:
	call error
	db "File not found.", 0


		; INP:	dx:ax = cluster - 2 (0-based cluster)
		; OUT:	cx:bx = input dx:ax
		;	dx:ax = first sector of that cluster
		; CHG:	-
clust_to_first_sector:
	push dx
	push ax
	 push dx
	mul word [bp + ldClusterSize]
	xchg bx, ax
	xchg cx, dx
	 pop ax
	mul word [bp + ldClusterSize]
	test dx, dx
	jnz short error_badchain
	xchg dx, ax
	add dx, cx
.cy_error_badchain:
	jc short error_badchain
	xchg ax, bx

	add ax, [bp + lsvDataStart]
	adc dx, [bp + lsvDataStart + 2]
	jc short .cy_error_badchain
				; dx:ax = first sector in cluster
	pop bx
	pop cx			; cx:bx = cluster
	retn


		; INP:	cx:bx = cluster (0-based)
		;	si:di = loaded FAT sector, -1 if none
		; OUT:	CY if no next cluster
		;	NC if next cluster found,
		;	 dx:ax = next cluster value (0-based)
		;	si:di = loaded FAT sector
		; CHG:	cx, bx
clust_next:
	mov ax, bx
	mov dx, cx
	add ax, 2
	adc dx, 0

	push es
	cmp byte [bp + ldFATType], 16
	je .fat16
	ja .fat32

.fat12:
; FAT12 entries are 12 bits, bytes are 8 bits. Ratio is 3 / 2,
;  so multiply cluster number by 3 first, then divide by 2.
					; ax = cluster number (up to 12 bits set)
		mov dx, ax
		shl ax, 1		; = 2n (up to 13 bits set)
		add ax, dx		; = 2n+n = 3n (up to 14 bits set)
		shr ax, 1		; ax = byte offset into FAT (0..6129)
					; CF = whether to use high 12 bits
		sbb cx, cx		; = -1 iff CY, else 0

; Use the calculated byte offset as an offset into the FAT
;  buffer, which holds all of the FAT's relevant data.
		mov es, [bp + lsvFATSeg]
		xchg bx, ax		; bx -> 16-bit word in FAT to load

; get 16 bits from FAT
		mov ax, [es:bx]

		and cl, 4	; = 4 iff CY after shift, else 0
		shr ax, cl	; shift down iff odd entry, else unchanged
		and ax, 0FFFh	; insure it's only 12 bits
	jmp short .gotvalue_zero_dx

.fat32:
		; * 4 = byte offset into FAT (0--4000_0000h)
	add ax, ax
	adc dx, dx
.fat16:
		; * 2 = byte offset into FAT (0--2_0000h)
	add ax, ax
	adc dx, dx

	 push ax
	xchg ax, dx
	xor dx, dx		; dx:ax = high word
	div word [bp + bsBPB + bpbBytesPerSector]
	xchg bx, ax		; bx = high word / divisor
	 pop ax			; dx = remainder, ax = low word
	div word [bp + bsBPB + bpbBytesPerSector]
	xchg dx, bx		; dx:ax = result, bx = remainder
				; dx:ax = sector offset into FAT (0--200_0000h)
				; bx = byte offset into FAT sector (0--8190)
	cmp dx, si
	jne @F		; read sector
	cmp ax, di
	je @FF		; sector is already buffered
@@:
	mov si, dx
	mov di, ax
	mov word [bp + lsvFATSector + 2], dx
	mov word [bp + lsvFATSector + 0], ax

	push bx
	add ax, [bp + bsBPB + bpbReservedSectors]
	adc dx, 0
	mov bx, [bp + lsvFATSeg]
	call read_sector
	pop bx
@@:
	mov es, [bp + lsvFATSeg]
	mov dx, [es:bx + 2]
	mov ax, [es:bx]		; dx:ax = FAT32 entry

	cmp byte [bp + ldFATType], 16	; is it FAT32 ?
	jne @F			; yes -->
.gotvalue_zero_dx:
	xor dx, dx		; no, clear high word
@@:
	pop es

		; INP:	dx:ax = cluster value, 2-based
		; OUT:	dx:ax -= 2 (makes it 0-based)
		;	CY iff invalid cluster
check_clust:
	and dh, 0Fh
	sub ax, 2
	sbb dx, 0

	cmp byte [bp + ldFATType], 16
	ja .fat32
	je .fat16

.fat12:
	cmp ax, 0FF7h - 2
	jmp short .common

.fat32:
	cmp dx, 0FFFh
	jb @F		; CY here means valid ...-

.fat16:
	cmp ax, 0FFF7h - 2
@@:			;  -... or if NC first, CY here also
.common:
	cmc		; NC if valid
	retn


	align 16, db 38
end:

%if _PADDING
 %if ($ - $$) > _PADDING
  %warning No padding needed
 %else
	times _PADDING - ($ - $$) db 0
 %endif
%endif
