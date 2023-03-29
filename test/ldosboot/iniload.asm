
%if 0

Loader for finishing file system booting
 by C. Masloch, 2017

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif


%assign __lMACROS1_MAC__DEBUG_DEFAULTS 1
%include "lmacros3.mac"
	numdef DEBUG5
%idefine d5 _d 5,

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
ldLoadingSeg:		; word
ldQueryPatchValue:	; word
lsvCommandLine:		; word
.start:		equ $ - lsvclBufferLength
.signature:	resw 1
ldLoadUntilSeg:		; word
lsvExtra:		; word
.partition:	resb 1	; byte
.flags:		resb 1	; byte
	endstruc

lsvefNoDataStart	equ 1
lsvefPartitionNumber	equ 2

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

	struc PARTINFO
piBoot:		resb 1
piStartCHS:	resb 3
piType:		resb 1
piEndCHS:	resb 3
piStart:	resd 1
piLength:	resd 1
	endstruc

ptEmpty:		equ 0
ptFAT12:		equ 1
ptFAT16_16BIT_CHS:	equ 4
ptExtendedCHS:		equ 5
ptFAT16_CHS:		equ 6
ptFAT32_CHS:		equ 0Bh
ptFAT32:		equ 0Ch
ptFAT16:		equ 0Eh
ptExtended:		equ 0Fh
ptLinux:		equ 83h
ptExtendedLinux:	equ 85h


query_no_geometry equ 4
query_no_chs equ 2
query_no_lba equ 1
query_fd_multiplier equ 1
query_hd_multiplier equ 256
query_all_multiplier equ query_fd_multiplier + query_hd_multiplier


%ifndef _MAP
%elifempty _MAP
%else	; defined non-empty, str or non-str
	[map all _MAP]
%endif

	defaulting

	numdef QUERY_PATCH,	1	; use new style patch of CHS/LBA/geometry
	numdef QUERY_DEFAULT,	0
	numdef QUERY_GEOMETRY,	1	; query geometry via 13.08 (for CHS access)
	numdef RPL,		1	; support RPL and do not overwrite it
	numdef CHS,		1	; support CHS (if it fits)
	numdef LBA,		1	; support LBA (if available)
	numdef LBA_33_BIT,	1	; support 33-bit LBA
	numdef LBA_CHECK_NO_33,	1	; else: check that LBA doesn't carry
	numdef MULTIBOOT1,	1	; use Multiboot specification loader
	numdef MULTIBOOT2,	1	; use Multiboot2 specification loader
	numdef LSVEXTRA,	1	; use lsvExtra field
		; (needed if to use partition scanner)

	numdef LBA_SKIP_CHECK,	0	; don't use proper LBA extensions check
	numdef LBA_RETRY,	1	; retry LBA reads
	numdef CHS_RETRY,	1	; retry CHS reads
	numdef STACKSIZE,	2048
%if _STACKSIZE < 256
 %error Too small stack size
%elif _STACKSIZE > 3 * 1024
		; Note that we use 8 KiB for SectorSeg, 8 KiB for FATSeg,
		; 512 bytes + (ebpbNew - bpbNew) for the boot sector,
		; and a few paragraphs left for MCBs and headers. As the
		; protocol is implemented with a 20 KiB reserved area (below
		; EBDA / RPL / end of low memory), this results in a maximum
		; stack size around 3 KiB (substantially below 4 KiB).
 %error Too large stack size
%endif
	numdef CHECKSUM,	0	; include checksumming of kernel image
%if _CHECKSUM
 %include "inicheck.mac"
%endif

	numdef PADDING, 0
	strdef PAYLOAD_FILE,	"lDOSLOAD.BIN"
	numdef EXEC_OFFSET,	0
	numdef EXEC_SEGMENT,	0
	strdef INILOAD_SIGNATURE,	"XX"

	numdef IMAGE_EXE,	0
	numdef IMAGE_EXE_CS,	-16	; relative-segment for CS
	numdef IMAGE_EXE_IP,	256 +64	; value for IP
		; The next two are only used if _IMAGE_EXE_AUTO_STACK is 0.
	numdef IMAGE_EXE_SS,	-16	; relative-segment for SS
	numdef IMAGE_EXE_SP,	0FFFEh	; value for SP (0 underflows)
	numdef IMAGE_EXE_AUTO_STACK,	0, 2048	; allocate stack behind image
	numdef IMAGE_EXE_MIN,	65536	; how much to allocate for the process
%ifndef _IMAGE_EXE_MIN_CALC
 %define _IMAGE_EXE_MIN_CALC	\
		(((_IMAGE_EXE_MIN \
		- (payload.actual_end - payload) \
		- 256 \
		+ _IMAGE_EXE_AUTO_STACK) + 15) & ~15)
%endif
	numdef IMAGE_EXE_MAX, -1

	numdef SECOND_PAYLOAD_EXE,	0
	numdef SECOND_PAYLOAD_EXE_CS,	-16
	numdef SECOND_PAYLOAD_EXE_IP,	256 +64
	numdef SECOND_PAYLOAD_EXE_SS,	-16
	numdef SECOND_PAYLOAD_EXE_SP,	0FFFEh
	numdef SECOND_PAYLOAD_EXE_AUTO_STACK,	0, 2048
	numdef SECOND_PAYLOAD_EXE_MIN,	65536
%ifndef _SECOND_PAYLOAD_EXE_MIN_CALC
 %define _SECOND_PAYLOAD_EXE_MIN_CALC	\
		(((_SECOND_PAYLOAD_EXE_MIN \
		- (second_payload.actual_end - second_payload) \
		- 256 \
		+ _SECOND_PAYLOAD_EXE_AUTO_STACK) + 15) & ~15)
%endif
	numdef SECOND_PAYLOAD_EXE_MAX, -1
	strdef SECOND_PAYLOAD_FILE,	"lDOSEXEC.COM"


	strdef INILOAD_CFG, ""
%ifnidn _INILOAD_CFG, ""
 %include _INILOAD_CFG
%endif


%if _IMAGE_EXE && _SECOND_PAYLOAD_EXE
 %error Cannot use both of these.
%endif

%push
%define %$string _INILOAD_SIGNATURE
%strlen %$length %$string
%if %$length != 2
 %error Invalid signature
%endif
%substr %$letter %$string 1
%if %$letter <= 32 || %$letter >= 127
 %error Invalid signature
%endif
%substr %$letter %$string 2
%if %$letter <= 32 || %$letter >= 127
 %error Invalid signature
%endif
%pop


	cpu 8086
	org 0
start:
	db "MZ"		; exeSignature
		; dec bp, pop dx
	jmp strict short ms6_entry	; exeExtraBytes
			; db 0EBh, 16h	; dw 16EBh
%if _IMAGE_EXE
		; For now hardcoded to carry a .COM-like executable.
		; Note: With _IMAGE_EXE_AUTO_STACK, the
		;	 stack segment will be behind the image.
	dw (payload.end - $$ + 511) / 512	; exePages
	dw 0		; exeRelocItems
	dw (payload -$$+0) >> 4	; exeHeaderSize
	dw (_IMAGE_EXE_MIN_CALC + 15) >> 4	; exeMinAlloc
%if _IMAGE_EXE_MAX
	dw _IMAGE_EXE_MAX	; exeMaxAlloc
%else
	dw (_IMAGE_EXE_MIN_CALC + 15) >> 4	; exeMaxAlloc
%endif
%if _IMAGE_EXE_AUTO_STACK
	dw ((payload.actual_end - payload) \
		+ _IMAGE_EXE_MIN_CALC \
		- _IMAGE_EXE_AUTO_STACK + 15) >> 4	; exeInitSS
		; ss: payload size minus 512 (conservative, assume DOS
		;  treats bogus exeExtraBytes as below 512 bytes.)
		; + exeMinAlloc
		; - auto stack size
	dw _IMAGE_EXE_AUTO_STACK		; exeInitSP
		; sp = auto stack size (eg 800h)
%else
	dw _IMAGE_EXE_SS	; exeInitSS
	dw _IMAGE_EXE_SP	; exeInitSP
%endif
	dw 0		; exeChecksum
	dw _IMAGE_EXE_IP, _IMAGE_EXE_CS	; exeInitCSIP
	dw 0		; exeRelocTable
%elif _SECOND_PAYLOAD_EXE
		; For now hardcoded to carry a .COM-like executable.
		; Note: With _SECOND_PAYLOAD_EXE_AUTO_STACK, the
		;	 stack segment will be behind the image.
	dw (second_payload.end - $$ + 511) / 512	; exePages
	dw 0		; exeRelocItems
	dw (second_payload -$$+0) >> 4	; exeHeaderSize
	dw (_SECOND_PAYLOAD_EXE_MIN_CALC + 15) >> 4	; exeMinAlloc
%if _SECOND_PAYLOAD_EXE_MAX
	dw _SECOND_PAYLOAD_EXE_MAX	; exeMaxAlloc
%else
	dw (_SECOND_PAYLOAD_EXE_MIN_CALC + 15) >> 4	; exeMaxAlloc
%endif
%if _SECOND_PAYLOAD_EXE_AUTO_STACK
	dw ((second_payload.actual_end - second_payload) \
		+ _SECOND_PAYLOAD_EXE_MIN_CALC \
		- _SECOND_PAYLOAD_EXE_AUTO_STACK + 15) >> 4	; exeInitSS
	dw _SECOND_PAYLOAD_EXE_AUTO_STACK	; exeInitSP
%else
	dw _SECOND_PAYLOAD_EXE_SS	; exeInitSS
	dw _SECOND_PAYLOAD_EXE_SP	; exeInitSP
%endif
	dw 0		; exeChecksum
	dw _SECOND_PAYLOAD_EXE_IP, _SECOND_PAYLOAD_EXE_CS	; exeInitCSIP
	dw 0		; exeRelocTable
%else
	dw -1		; exePages
	dw 0		; exeRelocItems
	dw 0		; exeHeaderSize
	dw -1		; exeMinAlloc
	dw -1		; exeMaxAlloc
	dw -16, 0	; exeInitSS, exeInitSP
	dw 0		; exeChecksum
	dw 100h, -16	; exeInitCSIP
	dw 0		; exeRelocTable
%endif

ms6_entry:
		; This is the MS-DOS 6 / IBMDOS compatible entry point.
		;  Note that this supports FAT32 for PC-DOS 7.10!
		; cs:ip = 70h:0
		; ax:bx = first data sector of first cluster,
		;	including hidden sectors
		; 0:7C00h-> boot sector with BPB,
		;	    load unit field set, hidden sectors set
		; (actually boot unit in dl; because the "MZ" signature
		;  destroys dl we assume it's in the BPB too)
		; Either:
		;	dword [ss:sp] = 0:78h = 1Eh * 4 (IVT entry of int 1Eh)
		;	dword [ss:sp + 4] = old int 1Eh address
		; Or:
		;	ds:si = old int 1Eh address
		; 0:500h-> directory entry for BIO file
	cli
	cld
	push dx
	inc bp		; undo signature instructions

d3	call d3_display_two_characters
d3	test ax, "00"

	mov cx, cs
	cmp cx, 60h
	jne @F
.freedos_or_msdos1_com_entry:
	jmp freedos_or_msdos1_com_entry
@@:

;	xor cx, cx
;;	test dx, dx
;;	jnz @FF
		; Actual DOS will always put a zero word on top of
		;  the stack. But when the debugger loads us as
		;  a flat format binary it may set up another
		;  stack segment or not initialise the stack slot.
		;  (So as to avoid corrupting the binary.)
		; The offset check should suffice anyway.
	call @F
@@:
	pop cx
	sub cx, @B	; cx == 0 iff entered at offset 0
	jne .freedos_or_msdos1_com_entry
@@:
			; cx = 0

		; Note: It has been observed that some IBMBIO.COM / IO.SYS
		;	 boot sector loaders pass the int 1Eh address on the
		;	 stack (like MS-DOS 7 loading does). So we detect
		;	 whether the first dword (far pointer to IVT entry)
		;	 matches and then assume that the second dword has
		;	 the original int 1Eh address. Else, ds:si is used.
	mov di, 1Eh * 4	; -> IVT entry of int 1Eh
	cmp dx, di	; int 1Eh address on stack ?
	jne .dssi	; no -->
	mov bp, sp
	cmp word [bp + 2], cx	; segment 0 in next word ?
	jne .dssi	; no -->
	pop si
	pop ds		; discard
	pop si
	pop ds		; get old int 1Eh address from stack
.dssi:
	jmp ms6_continue1


error:
	push cs
	pop ds
	mov si, msg.error
	call disp_error
	pop si
	call disp_error
	xor ax, ax
	int 16h
	int 19h


disp_error.loop:
	mov ah, 0Eh
	mov bx, 7
	; push bp
		; (call may change bp, but it is not used here any longer.)
	int 10h
	; pop bp
disp_error:
	lodsb
	test al, al
	jnz .loop
	retn


query_geometry:
%if _QUERY_GEOMETRY || !_LBA_SKIP_CHECK
		; magic bytes start
	mov dl, [bp + bsBPB + ebpbNew + bpbnBootUnit]
				; magic bytes
 %if _QUERY_PATCH
	mov ax, _QUERY_DEFAULT	; magic bytes, checked by patch script
..@query_patch_site equ $ - 2
	test dl, dl		; hard disk unit ?
	jns @F			; no -->
	xchg al, ah		; get high byte into al
		; magic bytes end
@@:
 %endif
%endif

%if _QUERY_GEOMETRY	; +30 bytes
 %if !_LBA_SKIP_CHECK
	push dx
  %if _QUERY_PATCH
	push ax
  %endif
 %endif

  %if _QUERY_PATCH
	test al, 4		; don't query geometry ?
	jnz @F			; yes -->
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
 %if _QUERY_GEOMETRY
  %if _QUERY_PATCH
	pop ax			; restore query patch flags in al
  %endif
	pop dx			; restore unit number in dl
 %endif
 %if _QUERY_PATCH
	shr al, 1		; CY if force CHS
	jc @F			; if so -->
	and al, 1		; force LBA ?
	jnz .done_lba		; yes -->
 %endif
	mov ah, 41h
	mov bx, 55AAh
	stc
	int 13h		; 13.41.bx=55AA extensions installation check
@@:
	mov al, 0	; zero in case of no LBA support
	jc .no_lba
	cmp bx, 0AA55h
	jne .no_lba
	shr cl, 1	; support bitmap bit 0
	jnc .no_lba
	inc ax		; al = 1 to indicate LBA support
.no_lba:
.done_lba:
	mov byte [bp + ldHasLBA], al
%else
	mov byte [bp + ldHasLBA], 0
%endif

%if 1 || _QUERY_GEOMETRY || !_LBA_SKIP_CHECK
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
	jc .err_CY_2
  %if !_LBA
.err_CY_2: equ .err_CY_1
  %endif
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
	mov ah, 42h	; 13.42 extensions read
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

	; push word [si + 4 + 0]
	push es		; => user buffer
	 mov es, word [bp + ldSectorSeg]
	 mov word [si + 4 + 2], es
	; and word [si + 4 + 0], byte 0

	mov ah, 42h
%if _LBA_RETRY
	call .int13_retry
%else
	int 13h
		; (don't need .int13_preserve_lpcount as no further call)
%endif
.err_CY_2:
	jc .err_CY_1
%ifn _CHS
.err_CY_1: equ .err
%endif

	pop es
	; pop cx
	add sp, 10h
	jmp .sectorseg_helper_then_done

.lba_done:
	add sp, 10h
	jmp short .done

.lba_error: equ .err

 %if !_CHS
.no_lba_skip: equ .err
.no_lba_checked: equ .err
 %elif _LBA_SKIP_CHECK
.no_lba_skip:
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
	jnz .err_NZ_2
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
.err_NZ_2:
			 jnz .err_NZ_1	; error if cylinder >= 1024 -->
					; ! bx = 0 (for 13.02 call)

; we call INT 13h AH=02h once for each sector. Multi-sector reads
; may fail if we cross a track or 64K boundary

			mov ax, 0201h	; read one sector
%if _CHS_RETRY
			call .int13_retry
%else
			int 13h
%endif
			jnc .done

	cmp ah, 9	; data boundary error?
.err_NZ_1:
	jne .err

	push es		; user buffer
	 mov es, word [bp + ldSectorSeg]

	mov ax, 0201h
%if _CHS_RETRY
	call .int13_retry
%else
	int 13h
%endif
.err_CY_1:
	jnc .sectorseg_helper_es
%endif		; _CHS
.err:
error_diskaccess: equ $
	call error
	db "Disk read error.", 0


%if _CHS
.sectorseg_helper_es:
	pop es
%endif

.sectorseg_helper_then_done:
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

.done:
; increment segment
	mov bx, es
	add bx, word [bp + ldParaPerSector]

	pop si
	pop ax
	pop cx
	pop dx
.increment_sector_number:
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


error_shortfile:
	call error
	db "File is too short.", 0

error_badchain:
	call error
	db "Bad cluster chain.", 0

error_badclusters:
	call error
	db "Bad amount of clusters.", 0

error_outofmemory:
	call error
	db "Out of memory.", 0

%assign num 512-($-$$)
%if num >= 3
%assign num num - 3
 %warning num bytes in front of ms7_entry
	_fill 512 - 3,38,start
error_outofmemory_j1:
	jmp error_outofmemory
%else
error_outofmemory_j1: equ error_outofmemory
 %warning num bytes in front of ms7_entry
%endif
	_fill 512,38,start
ms7_entry:
		; This is the MS-DOS 7 compatible entry point.
		;  Supports FAT32 too.
		; cs:ip = 70h:200h
		; (si:)di = first cluster of load file
		; dwo [ss:bp - 4] = first data sector (with hidden sectors)
		; dwo [ss:sp] = 0:78h (IVT entry of int 1Eh)
		; dwo [ss:sp + 4] = old int 1Eh address
		; ss:bp -> boot sector with (E)BPB,
		;	    load unit field set, hidden sectors set
	inc dx
	dec dx		; "BJ" signature (apparently not about FAT32 support)

	jmp .continue	; jump to handler above 600h (sector loads 800h bytes)

.ms6_common:		; cx = 0
	mov ax, 70h + ((3 * 512) >> 4)	; MS6 entry has 3 sectors loaded
					;  (and is always segment 70h)

.continue2_set_extra_and_empty_cmdline:	; cx = 0, ax => behind loaded
%if _LSVEXTRA
	mov word [bp + lsvExtra], cx
%endif
	mov word [bp + lsvCommandLine], cx
.continue2:				; cx = 0, ax => behind loaded
	mov word [bp + lsvLoadSeg], ax

	mov word [bp + lsvFATSeg], cx	; initialise to zero (for FAT12)
	dec cx
	mov word [bp + lsvFATSector + 0], cx
	mov word [bp + lsvFATSector + 2], cx	; initialise to -1

		; Actually it seems that the MS-DOS 7 loaders load 4 sectors
		;  instead of only three (as the MS-DOS 6 loaders do).
		;  We use this to store specific handling in that last sector.

	jmp ldos_entry.ms7_common

msg:
.error:	db "Load error: ", 0


finish_continue:
	mov bx, cs
	add ax, bx	; = cs + rounded up length
	sub ax, word [bp + ldLoadTop]	; = paras to move down
	jbe short finish_load

	mov cx, word [bp + lsvLoadSeg]
			; => after end of loaded data
	sub word [bp + lsvLoadSeg], ax
			; relocate this pointer already
	neg ax
	add ax, bx	; ax = cs - paras to move down
			; want to relocate cs to this
	jnc short error_outofmemory_j1
	mov di, relocate_to
	push ax
	push di		; dword on stack: relocate_to
	cmp ax, 60h + 1
	jb short error_outofmemory_j1
	push ax		; word on stack => where to relocate to
	dec ax		; one less to allow relocator
	mov es, ax


finish_relocation:
	xor di, di	; es:di -> where to put relocator

	push es
	push di		; dword on stack: relocator destination

	mov ds, bx	; ds => unrelocated cs
	mov si, relocator	; ds:si -> relocator
relocator_size equ relocator.end - relocator
%rep (relocator_size + 1) / 2
	movsw		; place relocator
%endrep
%if relocator_size > 16
 %error Relocator is too large
%endif
	xor di, di	; word [ss:sp+4]:di -> where to relocate to
	xor si, si	; ds:si = cs:0

			; cx => after end of loaded data
	sub cx, bx	; length of currently loaded fragment
	mov bx, 1000h
	mov ax, cx
	cmp ax, bx	; > 64 KiB ?
	jbe @F
	mov cx, bx	; first relocate the first 64 KiB
@@:
	sub ax, cx	; how much to relocate later
	shl cx, 1
	shl cx, 1
	shl cx, 1	; how much to relocate first,
			;  << 3 == convert paragraphs to words
	retf		; jump to relocator


		; ds => first chunk of to be relocated data
		; es => first chunk of relocated data
		; bx = 1000h (64 KiB >> 4)
		; ax = number of paragraphs after first chunk (in next chunk)
relocate_to:
@@:
	mov dx, es
	add dx, bx
	mov es, dx	; next segment

	mov dx, ds
	add dx, bx
	mov ds, dx	; next segment

	sub ax, bx	; = how much to relocate after this round
	mov cx, 1000h << 3	; in case another full 64 KiB to relocate
	jae @F		; another full 64 KiB to relocate -->
	add ax, bx	; restore
	shl ax, 1
	shl ax, 1
	shl ax, 1	; convert paragraphs to words
	xchg cx, ax	; cx = that many words
	xor ax, ax	; no more to relocate after this round

@@:
	xor si, si
	xor di, di
	rep movsw	; relocate next chunk
	test ax, ax	; another round needed?
	jnz @BB		; yes -->

	push ss
	pop ds

		; ds = ss
		; cs = low enough to complete load
		; lsvLoadSeg => after last loaded fragment
		; ldLoadTop => after last available memory
		; ldParaPerSector = initialised
		; word [ss:sp] = payload.actual_end in paras
finish_load:
	pop ax
	mov bx, cs
	add ax, bx
	mov word [bp + ldLoadUntilSeg], ax
		; ldLoadUntilSeg => after last to-be-loaded paragraph

	mov bx, word [bp + lsvLoadSeg]
	cmp bx, ax
	jae short loaded_all_if_ae	; (for FreeDOS entrypoint) already loaded -->

	mov word [bp + ldLoadingSeg], cs

	mov ax, [bp + lsvFirstCluster]
	mov dx, [bp + lsvFirstCluster + 2]
	mov di, [bp + lsvFATSector]
	mov si, [bp + lsvFATSector + 2]
	call check_clust
	jc short error_badchain_j

skip_next_clust:
	push dx
	push ax
	call clust_to_first_sector
skip_next_sect:
	mov bx, [bp + ldLoadingSeg]
	cmp bx, [bp + ldLoadUntilSeg]
	jae loaded_all.2stack

	add bx, [bp + ldParaPerSector]
				; bx += paras per sector
	cmp bx, [bp + lsvLoadSeg]
	ja skipped_all
				; emulate read_sector:
	call read_sector.increment_sector_number
				; dx:ax += 1
	mov [bp + ldLoadingSeg], bx

	loop skip_next_sect
	pop ax
	pop dx
	call clust_next
	jnc skip_next_clust
end_of_chain:
	inc ax
	inc ax
	test al, 8	; set in 0FFF_FFF8h--0FFF_FFFFh,
			;  clear in 0, 1, and 0FFF_FFF7h
	jz short error_badchain_j
	mov bx, [bp + ldLoadingSeg]
	cmp bx, [bp + ldLoadUntilSeg]
loaded_all_if_ae:
	jae loaded_all
	jmp error_shortfile


skipped_all:
	sub bx, [bp + ldParaPerSector]
				; restore bx => next sector to read
	call read_sector
		; we can depend on the fact that at least
		;  up to end was already loaded, so this
		;  (successful) read_sector call loaded
		;  at least 32 bytes starting at end.
		; therefore, we can put part of the
		;  remaining handler into these 32 bytes.
	jmp skipped_all_continue


error_badchain_j:
	jmp error_badchain


		; ds => first chunk of to be relocated data
		; word [ss:sp] => first chunk of relocation destination
		; cx = number of words in first chunk
relocator:
	pop es		; => where to relocate to
	rep movsw
	retf		; jump to relocated relocate_to
.end:


		; INP:	dx:ax = cluster - 2 (0-based cluster)
		; OUT:	dx:ax = first sector of that cluster
		;	cx = adjusted sectors per cluster
		; CHG:	bx
clust_to_first_sector:
	mov cx, word [bp + ldClusterSize]
	 push dx
	mul cx
	xchg bx, ax
	 pop ax
	push dx
	mul cx
	test dx, dx
	jnz short error_badchain_j
	xchg dx, ax
	pop ax
	add dx, ax
.cy_error_badchain:
	jc short error_badchain_j
	xchg ax, bx

	add ax, [bp + lsvDataStart]
	adc dx, [bp + lsvDataStart + 2]
	jc short .cy_error_badchain
				; dx:ax = first sector in cluster
	retn


		; INP:	dx:ax = cluster (0-based)
		;	si:di = loaded FAT sector, -1 if none
		; OUT:	CY if no next cluster
		;	NC if next cluster found
		;	dx:ax = next cluster value (0-based)
		;	si:di = loaded FAT sector
		; CHG:	cx, bx
clust_next:
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


ms6_continue1:
	mov es, cx			; cx = 0
	mov bp, 7C00h			; 0:bp -> boot sector with BPB

	mov word [es:di], si
	mov word [es:di + 2], ds	; restore old int 1Eh address

	mov ss, cx			; = 0
	mov sp, 7C00h + lsvCommandLine

	push word [es:500h + 20]
	push word [es:500h + 26]
	pop word [bp + lsvFirstCluster + 0]
	pop word [bp + lsvFirstCluster + 2]

	sub bx, word [bp + bsBPB + bpbHiddenSectors + 0]
	sbb ax, word [bp + bsBPB + bpbHiddenSectors + 2]
	mov word [bp + lsvDataStart + 0], bx
	mov word [bp + lsvDataStart + 2], ax
	jmp ms7_entry.ms6_common	; passing cx = 0


%assign num 1020-($-$$)
%warning num bytes in front of ldos_entry
	_fill 1020,38,start
	dw "lD"		; always this signature (word [1020] == 446Ch)
	dw _INILOAD_SIGNATURE
			; two printable non-blank ASCII characters
			; (ie both bytes in the range 21h..7Eh)
			;  Rx = RxDOS kernel
			;  FD = FreeDOS kernel
			;  TP = TestPL
			;  (lD)eb = lDebug
			;  (lD)Db = lDDebug
%if ($ - $$) != 1024
 %error Invalid signature
%endif
ldos_entry:
	cli
	cld

		; ip = 400h
		; cs = arbitrary; typically 60h, 70h, or 200h
		; dwo [ss:bp - 4] = first data sector (without hidden sectors)
		; wo [ss:bp - 6] = load_seg, => after last loaded data
		; wo [ss:bp - 8] = fat_seg, 0 if invalid
		;  initialised to 0 by MS-DOS 6, 7, FreeDOS entrypoints
		;  fat_sector is not used for FAT12 !
		; wo [ss:bp - 12] = fat_sector, -1 if none (FAT16)
		; dwo [ss:bp - 12] = fat_sector, -1 if none (FAT32)
		;  initialised to -1 by MS-DOS 6, 7, FreeDOS entrypoints
		; wo [ss:bp - 16] = first_cluster (FAT16, FAT12)
		; dwo [ss:bp - 16] = first_cluster (FAT32)
		;  initialised to 0 by FreeDOS entrypoint
		;
		; Extension 1:
		; lsvExtra (word [ss:bp - 18]) may be set,
		;  not sure about interface yet. allows
		;  to not initialise data start, or to specify
		;  a partition number instead of offset
		;
		; Extension 2:
		; word [ss:bp - 20] = signature "CL" if valid
		; bp >= 20 + 256 if valid
		; 256bytes [ss:bp - 20 - 256] = ASCIZ command line string

	xor ax, ax
	push ax			; push into lsvExtra if sp -> LSV
%if _LSVEXTRA
	mov word [bp + lsvExtra], ax
		; byte [ss:bp - 18] = partition number
		; byte [ss:bp - 17] = flags for initialisation
%endif
	push ax			; push into lsvCommandLine if sp -> LSV

.ms7_common:
	mov ax, cs
	mov cx, word [bp + lsvLoadSeg]
	sub cx, ax
	cmp cx, (end -$$+0) >> 4
	jae @F
error_notfullyloaded:
	call error
	db "Initial loader not fully loaded.", 0
@@:

	mov bx, (payload.actual_end -$$+0 +15) >> 4
	cmp cx, bx
	jbe @F
	add bx, ax
	mov word [bp + lsvLoadSeg], bx
@@:

init_memory:
; Get conventional memory size and store it
		int 12h
		mov cl, 6
		shl ax, cl
%if _RPL
	xor si, si
	xchg dx, ax
	mov ds, si
	lds si, [4 * 2Fh]
	add si, 3
	lodsb
	cmp al, 'R'
	jne .no_rpl
	lodsb
	cmp al, 'P'
	jne .no_rpl
	lodsb
	cmp al, 'L'
	jne .no_rpl
	mov ax, 4A06h
	int 2Fh
.no_rpl:
	xchg ax, dx
%endif
	push ax
	; sub ax, 32 >> 4	; make space for two MCBs: top MCB, RPL MCB
	dec ax
	dec ax
	mov cx, ax
	sub ax, (8192 + 16) >> 4
	dec cx		; => last paragraph of higher buffer (16-byte trailer)
	mov dx, ax	; => first paragraph of higher buffer
	mov bx, cx
	and dh, 0F0h	; 64 KiB chunk of first paragraph of higher buffer
	and bh, 0F0h	; 64 KiB chunk of last paragraph of higher buffer
	cmp bh, dh	; in same chunk?
	mov bx, ax
	je .gotsectorseg	; yes, use higher buffer as sector buffer ->
			; bx = use higher buffer as FAT buffer
	inc bx		; => 8 KiB buffer (no 16-byte trailer)
	sub ax, (8192 + 32) >> 4
			; 32 = leave space for higher buffer MCB + header
			; +16 from the above calcs for 16-byte trailer
	mov cx, ax	; use lower buffer as sector buffer
	jmp short .gotsegs

.gotsectorseg:
			; ax = use higher buffer as sector buffer
	sub bx, (8192 + 32) >> 4	; use lower buffer as FAT buffer
			; 32 = leave space for higher buffer MCB + header
	mov cx, bx
		; ax = sector seg
		; bx = FAT seg
		; cx = the lower of the two
.gotsegs:
	sub cx, (+_STACKSIZE -LOADCMDLINE + 512 + (ebpbNew - bpbNew) + 32 + 15) >> 4
			; +_STACKSIZE = stack space
			; -LOADCMDLINE = load cmd line + data + lsv space
			; 512 = boot sector (allows finding filename)
			; (ebpbNew - bpbNew) = additional space for BPBN moving
			; 32 = leave space for lower buffer MCB + header
		; cx = stack seg

	dec cx		; leave space for stack + BPB buffer MCB
	cmp cx, word [bp + lsvLoadSeg]
	jnb @F
.error_outofmemory:
	jmp error_outofmemory
@@:

	push ax
	mov dx, ss
	mov ax, bp
	add ax, 512 + 15
	jnc @F
	mov ax, 1_0000h >> 1
	db __TEST_IMM16	; (skip one shr)
@@:
	shr ax, 1
	shr ax, 1
	shr ax, 1
	shr ax, 1
	add dx, ax
	cmp dx, cx
	ja .error_outofmemory

		; note that the next conditional doesn't jump for lsvFATSeg = 0
	mov dx, word [bp + lsvFATSeg]
	add dx, (8192) >> 4
	cmp dx, cx
	ja .error_outofmemory
	pop ax

	pop dx		; top of memory (=> start of RPL, EBDA, video memory)
	inc cx		; => stack + BPB buffer
	push ss
	pop ds
	mov es, cx
	push cx		; top of memory below buffers
	push ax		; => sector seg

	xor cx, cx
	lea si, [bp + lsvCommandLine.start]
	cmp bp, si	; can have command line ?
			;  (also makes sure movsw and lodsw never run
			;  with si = 0FFFFh which'd cause a fault.)
	jb .no_cmdline

	mov di, _STACKSIZE - LOADCMDLINE + ldCommandLine.start
			; -> cmd line target
	mov cl, (LOADCMDLINE_size + 1) >> 1
	rep movsw	; copy cmd line
%if lsvCommandLine.start + fromwords(words(LOADCMDLINE_size)) != lsvCommandLine.signature
 %error Unexpected structure layout
%endif
	lodsw
	cmp ax, lsvclSignature
	je @F		; if command line given -->
.no_cmdline:
	mov byte [es: _STACKSIZE - LOADCMDLINE + ldCommandLine.start ], cl
			; truncate as if empty line given
	dec cx		; cl = 0FFh
@@:
	mov byte [es: _STACKSIZE - LOADCMDLINE + ldCommandLine.start \
		+ fromwords(words(LOADCMDLINE_size)) - 1 ], cl
			; remember whether command line given
			;  = 0 if given (also truncates if too long)
			;  = 0FFh if not given

		; si happens to be already correct here if we didn't
		;  branch to .no_cmdline, however make sure to set
		;  it here to support this case.
	lea si, [bp + lsvExtra]
			; ds:si -> lsv + BPB
	mov di, _STACKSIZE - LOADCMDLINE + lsvExtra
			; es:di -> where to place lsv
	mov cx, (- lsvExtra + 512 + 1) >> 1
	rep movsw	; copy lsv (including lsvExtra) and BPB
	xor ax, ax
	mov cx, ((ebpbNew - bpbNew + 15) & ~15) >> 1
	rep stosw	; initialise area behind sector (left so for FAT32)
	pop ax
	pop cx
	mov ss, cx
	mov sp, _STACKSIZE
			; -> above end of stack space
	mov bp, _STACKSIZE - LOADCMDLINE
			; -> BPB, above end of lsv
	dec cx		; => space for stack + BPB buffer MCB
	sti

		; ax => sector buffer
		; bx => FAT buffer
		; cx => above end of memory available for load
		; dx => above end of memory used by us
	mov word [bp + ldMemoryTop], dx
	mov word [bp + ldLoadTop], cx
	mov word [bp + ldSectorSeg], ax

	mov ds, word [bp + lsvFATSeg]
	xor si, si	; ds:si -> FAT buffer
	mov es, bx
	xor di, di	; es:di -> where to move
	mov cx, 8192 >> 1
	rep movsw
	mov word [bp + lsvFATSeg], bx

	push ds		; to check for word [lsvFATSeg] == zero later on

	push ss
	pop es
	push ss
	pop ds

	mov bx, [bp + bsBPB + bpbSectorsPerFAT]
	test bx, bx
	jz .is_fat32

	; lea si, [bp + 510]			; -> last source word
	mov si, _STACKSIZE - LOADCMDLINE + 510
	lea di, [si + (ebpbNew - bpbNew)]	; -> last dest word
	mov cx, (512 - bsBPB - bpbNew + 1) >> 1
			; move sector up, except common BPB start part
%if ((512 - bsBPB - bpbNew + 1) >> 1) <= 20
 %fatal Need AMD erratum 109 workaround
%endif
	std		; AMD erratum 109 handling not needed
	rep movsw
	cld

	mov word [bp + lsvFirstCluster + 2], cx
	mov word [bp + lsvFATSector + 2], cx

	mov word [bp + bsBPB + ebpbSectorsPerFATLarge], bx
	mov word [bp + bsBPB + ebpbSectorsPerFATLarge + 2], cx
	mov word [bp + bsBPB + ebpbFSFlags], cx
	; FSVersion, RootCluster, FSINFOSector, BackupSector, Reserved:
	;  uninitialised here (initialised by loaded_all later)
.is_fat32:
%if 1 || _QUERY_GEOMETRY || !_LBA_SKIP_CHECK
	call query_geometry
		; The ebpbNew BPBN needs to be initialised
		;  to use this function. It must be called
		;  before using read_sector (used by the FAT12
		;  FAT loader, or by finish_load later).
%endif

%if _LSVEXTRA
	test byte [bp + lsvExtra.flags], -1
	jz @F

	mov cx, cs
	mov ax, word [bp + lsvLoadSeg]
	sub ax, cx
	cmp ax, (end_of_handle_lsv_extra_flags + 15 -$$+0) >> 4
	jb error_notfullyloaded

	call handle_lsv_extra_flags
@@:
%endif

; adjusted sectors per cluster (store in a word,
;  and decode EDR-DOS's special value 0 meaning 256)
	mov al, [bp + bsBPB + bpbSectorsPerCluster]
	dec ax
	mov ah, 0
	inc ax
	mov [bp + ldClusterSize], ax

; 16-byte paragraphs per sector
	mov ax, [bp + bsBPB + bpbBytesPerSector]
	mov cl, 4
	shr ax, cl
	mov [bp + ldParaPerSector], ax

; total sectors
		; After the prior shr instruction, ax is < 8000h,
		;  so the following cwd always zeros dx.
	cwd
	mov ax, [bp + bsBPB + bpbTotalSectors]
	test ax, ax
	jnz @F
	mov dx, [bp + bsBPB + bpbTotalSectorsLarge + 2]
	mov ax, [bp + bsBPB + bpbTotalSectorsLarge]

		; fall through and let it overwrite the field with the
		; already current contents. saves a jump.
@@:
	mov [bp + bsBPB + bpbTotalSectorsLarge + 2], dx
	mov [bp + bsBPB + bpbTotalSectorsLarge], ax

	; dx:ax = total sectors

	cmp word [bp + bsBPB + bpbSectorsPerFAT], 0
	mov byte [bp + ldFATType], 32
	je .got_fat_type

	; dx:ax = total amount of sectors
	sub ax, word [bp + lsvDataStart]
	sbb dx, word [bp + lsvDataStart + 2]

	; dx:ax = total amount of data sectors
	mov bx, ax
	xchg ax, dx
	xor dx, dx
	div word [bp + ldClusterSize]
	xchg bx, ax
	div word [bp + ldClusterSize]
	; bx:ax = quotient, dx = remainder
	; bx:ax = number of clusters
	test bx, bx
	jz @F
.badclusters:
	jmp error_badclusters

@@:
	cmp ax, 0FFF7h - 2
	ja .badclusters
	shr byte [bp + ldFATType], 1	; = 16
	cmp ax, 0FF7h - 2
	ja .got_fat_type

	mov byte [bp + ldFATType], 12
	pop ax
	test ax, ax
	jnz .got_fat12

; lsvFATSeg was zero! This means the FAT isn't loaded yet.

; Load the entire FAT into memory. This is easily feasible for FAT12,
;  as the FAT can only contain at most 4096 entries.
; (The exact condition should be "at most 4087 entries", or with a
;  specific FF7h semantic, "at most 4088 entries"; the more reliable
;  and portable alternative would be "at most 4080 entries".)
; Thus, no more than 6 KiB need to be read, even though the FAT size
;  as indicated by word[sectors_per_fat] could be much higher. The
;  first loop condition below is to correctly handle the latter case.
; (Sector size is assumed to be a power of two between 32 and 8192
;  bytes, inclusive. An 8 KiB buffer is necessary if the sector size
;  is 4 or 8 KiB, because reading the FAT can or will write to 8 KiB
;  of memory instead of only the relevant 6 KiB. This is always true
;  if the sector size is 8 KiB, and with 4 KiB sector size it is true
;  iff word[sectors_per_fat] is higher than one.)
		mov di, 6 << 10		; maximum size of FAT12 to load
		mov cx, [bp + bsBPB + bpbSectorsPerFAT]
					; maximum size of this FS's FAT
			; If we're here, then ax = 0 (jnz jumped if not),
			;  so this cwd always zeros dx.
		cwd
		mov ax, [bp + bsBPB + bpbReservedSectors]; = first FAT sector
		mov bx, [bp + lsvFATSeg]
@@:
		call read_sector	; read next FAT sector
		sub di, [bp + bsBPB + bpbBytesPerSector]
					; di = bytes still left to read
		jbe @F			; if none -->
					; (jbe means jump if CF || ZF)
		loop @B			; if any FAT sector still remains -->
@@:					; one of the limits reached; FAT read

.got_fat12:
	db __TEST_IMM8	; skip pop ax
.got_fat_type:
	pop ax

	mov ax, (payload.actual_end -$$+0 +15) >> 4
	push ax
		; on stack: payload.actual_end in paragraphs
	mov bx, [bp + ldParaPerSector]
	dec bx		; para per sector - 1
	add ax, bx	; round up
	not bx		; ~ (para per sector - 1)
	and ax, bx	; rounded up,
		; ((payload.actual_end -$$+0 +15) >> 4 + pps - 1) & ~ (pps - 1)

	jmp finish_continue


%assign num 1024+512-($-$$)
%warning num bytes in front of end
	_fill 1024+512,38,start
end:


load_next_clust:
	push dx
	push ax
	call clust_to_first_sector
load_next_sect:
	mov bx, [bp + ldLoadingSeg]
	cmp bx, [bp + ldLoadUntilSeg]
	jae loaded_all.2stack_j

	call read_sector
skipped_all_continue:
	mov [bp + ldLoadingSeg], bx
	loop load_next_sect
	pop ax
	pop dx
	call clust_next
	jnc load_next_clust
	jmp end_of_chain

%if ($ - end) > 32
 %error load_next part exceeds end+32
%endif

		; if we jump to here, then the whole file has
		;  been loaded, so this jump doesn't have to
		;  stay in the 32 bytes after the end label.
loaded_all.2stack_j:
	jmp loaded_all.2stack


ms7_entry.continue:
	cli
	cld
	pop bx
	pop es
	pop word [es:bx]
	pop word [es:bx + 2]

	lea bx, [bp + lsvCommandLine]
	cmp sp, bx
	jbe @F
	mov sp, bx
@@:
	mov word [bp + lsvFirstCluster + 0], di
	mov word [bp + lsvFirstCluster + 2], si

	mov ax, word [bp + bsBPB + bpbHiddenSectors + 0]
	mov dx, word [bp + bsBPB + bpbHiddenSectors + 2]
	sub word [bp + lsvDataStart + 0], ax
	sbb word [bp + lsvDataStart + 2], dx

	mov ax, cs
	add ax, (4 * 512) >> 4	; MS7 entry has 4 sectors loaded
	xor cx, cx		; cx = 0
	jmp ms7_entry.continue2_set_extra_and_empty_cmdline


%assign num 2046-($-$$)
%warning num bytes in front of end2
	_fill 2046,38,start
	dw "MS"			; signature of MS-DOS 7 load
	align 16, db 38
end2:


		; This handling is in the second header part,
		;  behind the needed part to finish loading.
		;  It is only used when the file is completely loaded.
loaded_all.2stack:
	pop ax
	pop ax
loaded_all:
	mov ax, word [bp + bsBPB + bpbSectorsPerFAT]
	test ax, ax
	jz .fat32

	xor ax, ax
	push ss
	pop es
	lea di, [bp + bsBPB + ebpbFSFlags]
	mov cx, (EBPB_size - ebpbFSFlags) / 2
	rep stosw
		; initialise ebpbFSFlags (reinit), ebpbFSVersion,
		;  ebpbRootCluster, ebpbFSINFOSector, ebpbBackupSector,
		;  ebpbReserved

.fat32:

%if _CHECKSUM
        push cs
        pop ds

        mov si, checksumheader
        mov cx, CHECKSUMHEADER_size / 2
        xor bx, bx
@@:
        cmp si, ..@checksumfield
         lodsw
        jne @F
         xor ax, ax
@@:
        add bx, ax
        loop @BB

        test bx, bx
        jnz error_header_checksum_failed

        testopt [..@checksumtype], 8000h
        jnz @F

        call checksum_crc16_6_paragraphs_start_cs
        int3

        push cs
        pop ds

        cmp ax, word [..@checksumfield]
	jne error_data_checksum_failed
..@data_checksum_ignore_failure_debugger:
@@:
%endif

	push ss
	pop es
	lea di, [bp + ldCommandLine.start]
	mov cx, lsvclBufferLength
	xor ax, ax
	push word [bp + ldCommandLine.start + lsvclBufferLength - 1]
				; get sentinel (whether command line given)
	repne scasb		; scan for terminator
	pop ax			; al = 0FFh if no command line given
				; al = 0 else
	rep stosb		; clear remainder of buffer

%if _QUERY_PATCH
	mov ax, word [cs:..@query_patch_site]
%else
	mov ax, _QUERY_DEFAULT
%endif
	mov word [bp + ldQueryPatchValue], ax

	mov ax, cs
	add ax, ((payload -$$+0) >> 4) + _EXEC_SEGMENT
	push ax
%if _EXEC_OFFSET
	mov ax, _EXEC_OFFSET
%else
	xor ax, ax
%endif
	push ax
		; cs:ip = xxxxh:_EXEC_OFFSET
		; entire payload loaded (payload -- payload.actual_end)
		; LOADSTACKVARS and LOADDATA and EBPB and ebpbNew BPBN set
		; LOADCMDLINE set (ASCIZ, up to 255 bytes + 1 byte terminator)
		; word [ldCommandLine.start] = 0FF00h if had invalid signature
	retf


%if _CHECKSUM
error_header_checksum_failed:
	call error
	db "Header checksum failed.", 0

error_data_checksum_failed:
	stc
	int3
	jnc ..@data_checksum_ignore_failure_debugger
	call error
	db "Data checksum failed.", 0
%endif


freedos_or_msdos1_com_entry:
	call @F
@@:
	pop cx
	cmp cx, @B
	jne msdos1_com_entry

freedos_entry:
		; This is the FreeDOS compatible entry point.
		;  Supports FAT32 too.
		; cs:ip = 60h:0
		; whole load file loaded
		; first cluster of load file: not given!
		; first data sector: not given!
		; int 1Eh not modified, original address: not given!
		; bl = load unit (not used by us)
		; ss:bp -> boot sector with (E)BPB,
		;	    load unit field set, hidden sectors set
		;  (usually at 1FE0h:7C00h)
		; NEW: word [ss:bp - 14h] = "CL" to indicate command line
		;	then ss:bp - 114h -> 256 byte ASCIZ string

	lea bx, [bp + lsvCommandLine.start]
				; ss:bx -> command line buffer, if any
	cmp bp, - lsvCommandLine.start
				; enough data below bp to hold buffer ?
	jb @F			; no -->
	cmp sp, bx		; sp below-or-equal would-be buffer ?
	jbe .canbevalid		; yes, can be valid --> (and word access valid)
@@:
	cmp bp, - lsvCommandLine.signature
				; enough data below bp to hold our lsv ?
	jae @F			; yes -->
	test bp, 1		; valid to access even-aligned words ?
	jnz .error		; maybe not -->
@@:
	and word [bp + lsvCommandLine.signature], 0
				; invalidate signature
.canbevalid:
	cmp word [bp + lsvCommandLine.signature], "CL"
				; valid signature ?
	je @F			; yes, keep bx pointing at buffer

	lea bx, [bp + lsvCommandLine.signature]
				; no, ss:bx -> lsv with signature
@@:
	cmp sp, bx		; sp below-or-equal needed stack frame ?
	jbe @F			; yes -->
	and bl, ~1		; make even-aligned stack (rounding down)
	mov sp, bx		; change sp
@@:


d3	call d3_display_two_characters
d3	test ax, "F0"

	xor cx, cx
	mov word [bp + lsvFirstCluster + 0], cx
	mov word [bp + lsvFirstCluster + 2], cx

%if _LSVEXTRA
	mov word [bp + lsvExtra], lsvefNoDataStart << 8
%else
	call calculate_data_start
%endif
.multiboot_entry:
	mov ax, cs
	add ax, (payload.actual_end -$$+0 +15) >> 4
				; Multiboot1/2 and FreeDOS have whole image
	xor cx, cx		; cx = 0
	jmp ms7_entry.continue2


.error:
	call error
	asciz "Invalid base pointer in FreeDOS entrypoint."


%if _LSVEXTRA
handle_lsv_extra_flags:
	test byte [bp + lsvExtra.flags], lsvefPartitionNumber
	jz @F
	call parse_partition_number
@@:
	test byte [bp + lsvExtra.flags], lsvefNoDataStart
	jz @F
	call calculate_data_start
@@:
	retn


parse_partition_number:
	xor ax, ax
	mov word [bp + bsBPB + bpbHiddenSectors], ax
	mov word [bp + bsBPB + bpbHiddenSectors + 2], ax
	cmp byte [bp + bsBPB + ebpbNew + bpbnBootUnit], -1
	jne @F
	mov byte [bp + bsBPB + ebpbNew + bpbnBootUnit], 80h
	mov word [bp + bsBPB + bpbCHSSectors], ax
	mov word [bp + bsBPB + bpbCHSHeads], ax
	call query_geometry
@@:

 %if !_LBA_SKIP_CHECK
	test byte [bp + ldHasLBA], 1
	jnz @F
 %endif

	mov ax, word [bp + bsBPB + bpbCHSSectors]
	mov dx, word [bp + bsBPB + bpbCHSHeads]

		; following is from lDebug 0c0930773929 boot.asm
	overridedef DEBUG5, 0
%define load_unit (bp + bsBPB + ebpbNew + bpbnBootUnit)
%define load_sectorsize (bp + bsBPB + bpbBytesPerSector)
%define load_sectorsizepara (bp + ldParaPerSector)

	test ax, ax
	jz .invalid_sectors
	cmp ax, 63
	ja .invalid_sectors
	test dx, dx
	jz .invalid_heads
	cmp dx, 100h
	ja .invalid_heads
@@:

	mov ax, word [bp + ldSectorSeg]	; ax => sector seg
	dec ax				; ax => sector seg - 16
	mov es, ax
	xor ax, ax
	mov bx, 16

d5	call d5dumpregs
d5	call d5message
d5	asciz 13,10,"In query_geometry 0",13,10

	mov di, bx
	mov cx, (8192 + 2) >> 1
					; es:bx -> auxbuff, es:di = same
	rep stosw			; fill buffer, di -> behind (auxbuff+8192+2)
	mov ax, 0201h			; read sector, 1 sector
	inc cx				; sector 1 (1-based!), cylinder 0 (0-based)
	mov dh, 0			; head 0 (0-based)
	mov dl, [load_unit]
	stc
	call .int13_retry
	jc .access_error

	std				; _AMD_ERRATUM_109_WORKAROUND does not apply
	mov word [es:bx - 2], 5E5Eh	; may overwrite last 2 bytes at line_out_end
	scasw				; -> auxbuff+8192 (at last word to sca)
d5	call d5dumpregs
d5	call d5message
d5	asciz 13,10,"In query_geometry 1",13,10
	mov cx, (8192 + 2) >> 1
	xor ax, ax
	repe scasw
	add di, 4			; di -> first differing byte (from top)
	cld
	push di

	mov di, bx
	mov cx, (8192 + 2) >> 1
	dec ax				; = FFFFh
	rep stosw

	mov ax, 0201h
	inc cx
	mov dh, 0
	mov dl, [load_unit]
	stc
	call .int13_retry
	jc .access_error

	std				; _AMD_ERRATUM_109_WORKAROUND does not apply
	scasw				; di -> auxbuff+8192 (last word to sca)
d5	call d5dumpregs
d5	call d5message
d5	asciz 13,10,"In query_geometry 2",13,10
	pop dx
	mov ax, -1
	mov cx, (8192 + 2) >> 1
	repe scasw
%if 0
AAAB
   ^
	sca B, match
  ^
	sca B, mismatch
 ^
	stop
%endif
	add di, 4			; di -> first differing byte (from top)
	cld

%if 0
0000000000000
AAAAAAAA00000
	^
FFFFFFFFFFFFF
AAAAAAAA00FFF
	  ^
%endif
	cmp dx, di			; choose the higher one
	jae @F
	mov dx, di
@@:
	sub dx, bx			; dx = sector size

d5	call d5dumpregs
d5	call d5message
d5	asciz 13,10,"In query_geometry 3",13,10

	cmp dx, 8192 + 2
	jae .sector_too_large
	mov ax, 32
	cmp dx, ax
	jb .sector_too_small
@@:
	cmp dx, ax
	je .got_match
	cmp ax, 8192
	jae .sector_not_power
	shl ax, 1
	jmp @B

.got_match:
	mov word [load_sectorsize], ax
	mov cl, 4
	shr ax, cl
	mov word [load_sectorsizepara], ax

	resetdef


	push cs
	pop ds
	push cs
	pop es

	mov cx, .per_partition
	call scan_partitions

	mov di, partition_offset
	mov dx, word [di + 2]
	mov ax, word [di]

	cmp ax, -1
	jne @F
	cmp dx, -1
	jne @F

	push ss
	pop es
	mov di, bp
	xor ax, ax
	mov cx, 512 / 2 + (((ebpbNew - bpbNew + 15) & ~15) >> 1)
	push word [bp + bsBPB + ebpbNew + bpbnBootUnit]
	push word [bp + bsBPB + bpbCHSSectors]
	push word [bp + bsBPB + bpbCHSHeads]
	rep stosw
	pop word [bp + bsBPB + bpbCHSHeads]
	pop word [bp + bsBPB + bpbCHSSectors]
	pop bx
	mov byte [bp + bsBPB + ebpbNew + bpbnBootUnit], bl
	mov word [bp + bsBPB + bpbHiddenSectors], dx
	mov word [bp + bsBPB + bpbHiddenSectors + 2], dx

	jmp .invalid_return


@@:
	push dx
	push ax
	mov bx, [bp + ldSectorSeg]
	call read_ae_512_bytes
	push es
	pop ds
	xor si, si
	push ss
	pop es
	mov di, bp
	mov cx, 512 / 2

	pop ax
	pop dx

	push word [bp + bsBPB + ebpbNew + bpbnBootUnit]
	push word [bp + bsBPB + bpbCHSSectors]
	push word [bp + bsBPB + bpbCHSHeads]


	rep movsw

	push ax
	xor ax, ax
	mov cx, ((ebpbNew - bpbNew + 15) & ~15) >> 1
	rep stosw	; initialise area behind sector (left so for FAT32)
	pop ax


	pop word [bp + bsBPB + bpbCHSHeads]
	pop word [bp + bsBPB + bpbCHSSectors]
	mov word [bp + bsBPB + bpbHiddenSectors], ax
	mov word [bp + bsBPB + bpbHiddenSectors + 2], dx


	push ss
	pop ds
	push ss
	pop es

	mov bx, [bp + bsBPB + bpbSectorsPerFAT]
	test bx, bx
	jz .not_fat32

	lea si, [bp + 510]			; -> last source word
	lea di, [si + (ebpbNew - bpbNew)]	; -> last dest word
	mov cx, (512 - bsBPB - bpbNew + 1) >> 1
			; move sector up, except common BPB start part
%if ((512 - bsBPB - bpbNew + 1) >> 1) <= 20
 %fatal Need AMD erratum 109 workaround
%endif
	std		; AMD erratum 109 handling not needed
	rep movsw
	cld

	mov word [bp + lsvFirstCluster + 2], cx
	mov word [bp + lsvFATSector + 2], cx

	mov word [bp + bsBPB + ebpbSectorsPerFATLarge], bx
	mov word [bp + bsBPB + ebpbSectorsPerFATLarge + 2], cx
	mov word [bp + bsBPB + ebpbFSFlags], cx
	; FSVersion, RootCluster, FSINFOSector, BackupSector, Reserved:
	;  uninitialised here (initialised by loaded_all later)
.not_fat32:

	pop bx
	mov byte [bp + bsBPB + ebpbNew + bpbnBootUnit], bl

	mov ah, lsvefNoDataStart
	mov al, byte [cs:partition_type]
	cmp al, ptFAT12
	je @F
	cmp al, ptFAT16_16BIT_CHS
	je @F
	cmp al, ptFAT16_CHS
	je @F
	cmp al, ptFAT32_CHS
	je @F
	cmp al, ptFAT32
	je @F
	cmp al, ptFAT16
	je @F

.invalid_return:
	xor ax, ax
	mov word [bp + lsvDataStart], ax
	mov word [bp + lsvDataStart + 2], ax
@@:
	mov byte [bp + lsvExtra.flags], ah

%if _CHECKSUM
	push cs
	pop es
	mov di, scanparttab_variables_start
	mov cx, scanparttab_variables_length_w
	xor ax, ax
	rep stosw	; clear variables for eventual checksum
	dec ax
	mov di, partition_offset
	stosw
	stosw		; reset this variable too
%endif

	push ss
	pop es
	push ss
	pop ds
	retn


.per_partition:
	push cx
	push si
	push di
	push bx

	mov ax, [es:si + piStart]
	mov dx, [es:si + piStart + 2]
	add ax, [ss:bx + di - 8]
	adc dx, [ss:bx + di - 8 + 2]	; = partition start

	mov cx, -1
	mov di, partition_offset
	cmp word [di], cx		; first one encountered ?
	jne @F
	cmp word [di + 2], cx
	jne @F				; no -->
	mov cl, byte [es:si + piType]
	mov byte [di - partition_offset + partition_type], cl
					; save type
	mov word [di], ax
	mov word [di + 2], dx		; yes, save offset
@@:

	mov cl, byte [load_current_partition]
					; which one ?
	cmp cl, byte [bp + lsvExtra.partition]
	jne @F				; not the sought one

	mov cl, byte [es:si + piType]
	mov byte [di - partition_offset + partition_type], cl
					; save type
	mov word [di], ax
	mov word [di + 2], dx		; save offset

	pop bx				; bx = base
	mov sp, bx			; reset sp
	pop ax				; pop dummy bp
	retn				; return to caller

@@:					; not yet found, continue
	pop bx
	pop di
	pop si
	pop cx
	retn


.int13_retry:
	pushf
	push ax
	int 13h		; first try
	jnc @F		; NC, success on first attempt -->

; reset drive
	xor ax, ax
	int 13h
	jc @F		; CY, reset failed, error in ah -->

; try read again
	pop ax		; restore function number
	popf
	int 13h		; retry, CF error status, ah error number
	retn

@@:			; NC or CY, stack has function number
	inc sp
	inc sp
	inc sp
	inc sp		; discard two words on stack, preserve CF
	retn


.access_error:
	jmp error_diskaccess

.sector_too_large:
.sector_too_small:
.sector_not_power:
	call error
	asciz "Invalid sector size."

.invalid_sectors:
.invalid_heads:
	call error
	asciz "Invalid geometry."

scan_logical.got_partition_cycle:
	call error
	asciz "Partition cycle detected."

scan_logical.error_too_many_partitions:
	call error
	asciz "Too many partitions detected."

read_partition_table.signature_fail:
	call error
	asciz "Invalid partition table detected."


		; INP:	dx:ax = first sector
		;	bx:0 -> buffer
		; OUT:	dx:ax = sector number after last read
		;	es = input bx
		;	bx:0 -> buffer after last written
		; CHG:	-
read_ae_512_bytes:
	push ds
	push cx
	push bx
	 push ss
	 pop ds
	mov cx, 512
.loop:
	call read_sector
	sub cx, word [bp + bsBPB + bpbBytesPerSector]
	ja .loop
	pop es
	pop cx
	pop ds
	retn


%assign _PARTITION_TABLE_IN_CS 1
%assign _BOOTCMD_FAIL_ERROR 0
%define _SCANPTAB_PREFIX
%define _SCANPTAB_DEBUG4_PREFIX
	overridedef DEBUG4, 0
%include "scanptab.asm"
	resetdef


	align 16
scanparttab_variables_start:
partition_table:
	times 16 * 4 db 0
.end:

partition_offset:
	dd -1

load_partition_cycle:
	dw 0
load_current_partition:
	db 0
partition_type:
	db 0

	align 2
scanparttab_variables_length_w: equ ($ - scanparttab_variables_start) / 2
%endif


		; INP:	ss:bp -> BPB
		;	ss:bp - LOADSTACKVARS -> lsv
		; OUT:	lsvDataStart set
		; CHG:	ax, bx, cx, dx, si, di
calculate_data_start:
	xor cx, cx			; ! ch = 0

		; Although this currently is unused, we calculate the
		;  first data sector (including root directory size)
		;  here to complete the LOADSTACKVARS.

; 32-byte FAT directory entries per sector
	mov ax, [bp + bsBPB + bpbBytesPerSector]
	mov cl, 5			; ! ch = 0
	shr ax, cl

; number of sectors used for root directory (store in CX)
		; After the prior shr instruction, ax is always < 8000h,
		;  so this cwd instruction always zeros dx.
	cwd
	mov si, [bp + bsBPB + bpbNumRootDirEnts]	; (0 iff FAT32)
	mov bx, ax
	dec ax				; rounding up
	js .error_badchain		; if >= 8000h (ie, 0FFFFh while bx = 0)
	add ax, si			; from BPB
	adc dx, dx			; account for overflow (dx was zero)
	div bx				; get number of root sectors
	xchg ax, cx			; cx = number of root secs, ! ah = 0

	push cx				; number of root secs
; first sector of root directory
	mov al, [bp + bsBPB + bpbNumFATs]
					; ! ah = 0, hence ax = number of FATs
	mov cx, word [bp + bsBPB + bpbSectorsPerFAT]
	xor di, di			; di:cx = sectors per FAT
					;  iff FAT12, FAT16
	test cx, cx			; is FAT32 ?
	jnz @F				; no -->
	mov cx, word [bp + bsBPB + ebpbSectorsPerFATLarge]
	mov di, word [bp + bsBPB + ebpbSectorsPerFATLarge + 2]	; for FAT32
@@:
	push ax
	mul cx
		; ax = low word SpF*nF
		; dx = high word
	xchg bx, ax
	xchg cx, dx
		; cx:bx = first mul
	pop ax
	mul di
		; ax = high word adjust
		; dx = third word
	test dx, dx
	jnz .error_badchain
	xchg dx, ax
		; dx = high word adjust
	add dx, cx
		; dx:bx = result
	xchg ax, bx
		; dx:ax = result
	jc .error_badchain

	add ax, [bp + bsBPB + bpbReservedSectors]
	adc dx, byte 0
	jc .error_badchain

	pop cx				; number of root sectors
	xor di, di

; first sector of disk data area:
	add cx, ax
	adc di, dx
	mov [bp + lsvDataStart], cx
	mov [bp + lsvDataStart + 2], di

	retn

.error_badchain:
	jmp error_badchain


end_of_handle_lsv_extra_flags:


%if _CHECKSUM
CHECKSUM_SIZE_P equ (payload.actual_end - $$ + 0) / 16

	overridedef STANDALONE, 0
 %include "inicheck.asm"
	resetdef

        align 16
checksumheader:
        istruc CHECKSUMHEADER
at cshSignature,                dw "CS"
at cshLengthBytesStructure,     dw CHECKSUMHEADER_size
at cshOffsetStructure,          dw paras(checksumheader - $$ + 0)
at cshChecksumStructure,        dw \
        10000h-(("CS" + CHECKSUMHEADER_size \
                 + paras(checksumheader - $$ + 0) \
                 + 8106h + CHECKSUM_SIZE_P \
                 + 0 + 0) & 0FFFFh)
at cshTypeChecksum
..@checksumtype:                dw 8106h
at cshAmountParagraphsData,     dw CHECKSUM_SIZE_P
at cshChecksumData
..@checksumfield:               dw 0
at cshReserved,                 dw 0
        iend
%endif

%if _MULTIBOOT1 || _MULTIBOOT2
 %include "multboot.asm"
%endif


%if _DEBUG3
		; INP:	word [cs:ip + 1] = two characters to display
		;		(second one may be NUL to skip)
		; OUT:	-
		; CHG:	-
d3_display_two_characters:
	lframe near
	lenter
	push ax
	push bx
	mov bx, word [bp + ?frame_ip]
	mov ax, [cs:bx + 1]

	push ax
	call d3_disp_al
	pop ax

	xchg al, ah
	test al, al
	jz @F

	call d3_disp_al
@@:

	pop bx
	pop ax
	lleave
	lret

		; INP:	al = to display
		; CHG:	ax, bx
d3_disp_al:
	push bp
	mov ah, 0Eh
	mov bx, 7
	int 10h
	pop bp
	retn
%endif


msdos1_com_entry:
	mov dx, .msg + 100h
	mov ah, 09h
	int 21h
	int 20h

.msg:
	ascic "86-DOS version 1 not supported, aborting.",13,10


	align 16, db 38
payload:
	incbin _PAYLOAD_FILE
	align 16, db 38
.actual_end:
%if _IMAGE_EXE
	align 512, db 38	; until end of page
	times 512 db 38		; a full additional page,
				; this is for the bogus exeExtraBytes
		; Note that the pages start counting within the EXE header!
		; Thus alignment to the file-level page boundary is correct.
%endif
.end:


%if _SECOND_PAYLOAD_EXE
	align 16, db 38
second_payload:
	incbin _SECOND_PAYLOAD_FILE
	align 16, db 38
.actual_end:
	align 512, db 38
	times 512 db 38
.end:
%endif

%if ($ - start) < 4096
	_fill 4096, 38, start	; fill to new minimum limit
%endif

%if _PADDING
 %if ($ - $$) > _PADDING
  %warning No padding needed
 %else
	times _PADDING - ($ - $$) db 0
 %endif
%endif
