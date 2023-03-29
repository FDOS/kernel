
%if 0

Loader test payload
 by C. Masloch, 2017

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif


%include "lmacros2.mac"

		numdef LARGE, 1
		numdef PADDING, 0

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


%ifndef _MAP
%elifempty _MAP
%else	; defined non-empty, str or non-str
	[map all _MAP]
%endif

	cpu 8086
	org 0
payload:
		; The device header is of a fixed format.
		;  For our purposes, the 4-byte code for
		;  each the strategy entry and the
		;  interrupt entry is part of this format.
		; (DOS may read the attributes or entrypoint
		;  offsets before calling either, so the
		;  inicomp stage needs to recreate in its
		;  entrypoints part exactly what we have here.)
device_header:
.next:
	fill 2, -1, jmp strict short j_zero_entrypoint
	dw -1
.attributes:
	dw 8000h			; character device
.strategy:
	dw .strategy_entry		; -> strategy entry
.interrupt:
	dw .interrupt_entry		; -> interrupt entry
.name:
	fill 8, 32, db "TESTPL$$"	; character device name
.strategy_entry:
	fill 4, 90h, jmp device_entrypoint
.interrupt_entry:
	fill 4, 90h, retf


j_zero_entrypoint:
	jmp zero_entrypoint


	nop
	align 32, nop
kernel_entrypoint:
		; cs:ip = load seg : 32 here
%if ($ - $$) != 32
 %error Wrong kernel mode entrypoint
%endif

		; S0 +28
	pushf	; +26
	push ax	; +24
	push cs	; +22
	call .push_ip
.push_ip:
	pop ax
	sub ax, .push_ip - kernel_entrypoint
	push ax	; +20 IP

common_entrypoint:
	push es	; +18 ES
	push bx	; +16 BX
	cmp word [cs:signature], 2638h
	je sig1_valid
	jmp sig_invalid

	align 64, nop
dos_exe_entrypoint:
		; cs:ip = PSP : 256 + 64 here
		;
		; Code must be position independent enough.
%if ($ - $$) != 64
 %error Wrong EXE mode entrypoint
%endif

		; S0 +28
	pushf	; +26
	push ax	; +24
	push cs	; +22
	call .push_ip
.push_ip:
	pop ax
	sub ax, .push_ip - dos_exe_entrypoint
	push ax	; +20 IP

dos_com_entrypoint:
	mov byte [cs:100h + loadmode], 1
	mov ax, cs
	add ax, 10h	; simulate kernel loading
	push ax
jump_common_entrypoint:
	mov ax, common_entrypoint
	push ax
	retf		; jump to cs + 10h : common_entrypoint


zero_entrypoint:
		; S0 +28
	pushf	; +26
	push ax	; +24
	push cs	; +22
	call .push_ip
.push_ip:
	pop ax
	sub ax, .push_ip
	push ax	; +20 IP

	cmp word [cs:0], 20CDh
	jne @F
	cmp ax, 100h
	je dos_com_entrypoint
@@:

	push cx
	mov cl, 4
	shr ax, cl
	mov cx, cs
	add ax, cx
	pop cx
	push ax
	jmp jump_common_entrypoint


device_entrypoint:
	pushf
	push ax
	push cs
	call .push_ip
.push_ip:
	pop ax
	sub ax, .push_ip - device_header.strategy_entry
	push ax
	mov byte [cs:loadmode], 2
	mov word [cs:dev_sp], sp
	mov word [cs:dev_exit], dev_exit.1
	jmp common_entrypoint


sig_invalid:
	call error
	db "Signature invalid.", 0

sig1_valid:
	push ds
	push bx
	mov bx, cs
	add bx, (signature2 -$$+0) >> 4
	mov ds, bx
	cmp word [(signature2 -$$+0) & 0Fh], 2638h
	pop bx
	pop ds
	jne sig_invalid
	jmp sig_valid

msg:
.error:	db "Test payload error: ", 0
.test:	db "Test payload loaded.", 13, 10, 0

.psp_and_size_before:	asciz "PSP at "
.psp_and_size_between:	asciz "h, size of memory block is "
.psp_and_size_after:	asciz "h paragraphs.",13,10
.cmdline.kern.none:	asciz "No kernel command line given!",13,10
.cmdline_before.kern:	asciz "Kernel command line = ",'"'
.cmdline_before.app:	asciz "Application command line = ",'"'
.cmdline_before.device:	asciz "Device command line = ",'"'
.cmdline_after:		asciz '"',13,10

	align 4
.foundname:
	times 8+1+3+1 db 0
		; buffer for base name (8) + dot (1) + ext (3) + NUL (1)
	align 2
.foundname_none:
	asciz "(None)"
.foundname_none_size: equ $ - .foundname_none
	align 2
.names:
	dw .name_first, 0
	dw .name_second, 0
	dw .name_third, 0
	dw .name_fourth, 0
	dw 0
.name_first:	asciz "1st name"
.name_second:	asciz "2nd name"
.name_third:	asciz "3rd name"
.name_fourth:	asciz "4th name"
.name_before:	asciz ": "
.name_quote:	asciz '"'
.name_after:	asciz 13,10

	align 4
dev_request_header:
		dd 0
loadmode:	dw 0		; 0 = loaded as boot payload,
				; 1 = loaded as DOS application,
				; 2 = loaded as DOS device driver
dev_sp:		dw 0
dev_exit:	dw 0
.1:
	mov sp, word [cs:dev_sp]
	jmp .common_1

.2:	mov sp, word [cs:dev_sp]
	jmp .common_2

.common_2:
	pop ax			; ss
	pop ds
	pop ax			; sp
	pop bp
	pop di
	pop si
	pop dx
	pop cx

.common_1:
	pop bx			; bx
	pop es			; es
	pop ax			; (IP)
	pop ax			; (CS)
	pop ax			; ax
	mov word [es:bx + 3], 8103h
				; error, done, error code: unknown command
	popf			; flags
	retf			; far return to DOS


error:
	push cs
	pop ds
	mov si, msg.error
	call disp_msg
	pop si
	call disp_msg
	test byte [cs:loadmode], 2
	jz .dos_or_bios
	jmp near [cs:dev_exit]

.dos_or_bios:
	test byte [cs:loadmode], 1
	jz .bios

	mov ax, 4C01h
	int 21h

.bios:
	xor ax, ax
	int 16h
	int 19h

disp_msg_asciz:
	push ds
	push si
	push ax
	 push cs
	 pop ds
	mov si, dx
	call disp_msg
	pop ax
	pop si
	pop ds
	retn

disp_msg:
@@:
	lodsb
	test al, al
	jz @F
	call disp_al
	jmp short @B

disp_al:
	push ax
	push bx
	push dx
	push bp
	test byte [cs:loadmode], 1 | 2
	jz .bios

	mov dl, al
	mov ah, 02h
	int 21h
	jmp .common

.bios:
	mov ah, 0Eh
	mov bx, 7
	int 10h

.common:
	pop bp
	pop dx
	pop bx
	pop ax
@@:
	retn

disp_msg_length:
	push cx
	jcxz .ret
.loop:
	lodsb
	call disp_al
	loop .loop
.ret:
	pop cx
	retn


sig_valid:
		; S0 +28
;	pushf	; +26
;	push ax	; +24
;	push cs	; +22
;	call .push_ip
;.push_ip:
;	pop ax
;	sub ax, .push_ip
;	push ax	; +20 IP
;	push es	; +18
;	push bx	; +16
	sti
	cld
	push cx	; +14
	push dx	; +12
	push si	; +10
	push di	; +8
	push bp	; +6
	mov ax, sp
	add ax, 22
	push ax	; +4 SP
	push ds	; +2
	push ss	; +0

	test byte [cs:loadmode], 2
	jz @F

	mov word [cs:dev_sp], sp
	mov word [cs:dev_exit], dev_exit.2
	mov word [cs:dev_request_header], bx
	mov word [cs:dev_request_header + 2], es

	cmp byte [es:bx + 2], 0		; command code 0 (init) ?
	jne dev_exit.2			; else immediately return -->

	mov byte [es:bx + 13], 0	; number of units = 0
	mov word [es:bx + 14 + 2], cs
	and word [es:bx + 14], 0	; -> after end of memory to allocate
	or word [cs:device_header.next], -1
					; fill in offset of device header link
@@:

	mov si, sp
	 push ss
	 pop ds
	mov di, table
	 push cs
	 pop es
loop_table:
	mov bx, [es:di + 0]
	mov al, 32
	call disp_al
	mov ax, [es:di + 2]
	call disp_al
	xchg al, ah
	call disp_al
	cmp bx, -1
	je @F
	mov al, '='
	call disp_al
	mov ax, [si + bx]
	call disp_ax_hex
@@:
	add di, 4
	cmp di, table.end
	jb loop_table

listnames:
	test byte [cs:loadmode], 1 | 2
	jnz .skip

	mov bx, msg.names
	push ss
	pop ds
	lea si, [bp + bsBPB + ebpbNew + BPBN_size]
	mov cx, (512 - (bsBPB + ebpbNew + BPBN_size)) - 2
						; -2 = AA55h sig
	cmp word [bp + bsBPB + bpbSectorsPerFAT], 0
	je @F
	mov cx, (512 + (ebpbNew - bpbNew) - (bsBPB + ebpbNew + BPBN_size)) - 2
@@:
.nextname:
	call findname
	 lahf
	mov dx, [cs:bx]
	call disp_msg_asciz
	mov dx, msg.name_before
	call disp_msg_asciz
	 sahf
	 jc @F		; skip quote if no name -->
	 mov dx, msg.name_quote
	 call disp_msg_asciz
@@:
	mov dx, msg.foundname
	call disp_msg_asciz
	 sahf
	 jc @F		; skip quote if no name -->
	 mov dx, msg.name_quote
	 call disp_msg_asciz
@@:
	mov dx, msg.name_after
	call disp_msg_asciz
	 sahf
	 mov ax, 0
	 jc @F		; set to zero if no name -->
	 lea ax, [si - 11]	; -> name in buffer
@@:
	 mov word [cs:bx + 2], ax	; -> name in buffer, or 0
	add bx, 4
	cmp word [cs:bx], 0
	jne .nextname

.skip:

	push cs
	pop ds
	mov si, msg.test
	call disp_msg

	test byte [cs:loadmode], 1
	jz .skip_psp_dos

	mov si, msg.psp_and_size_before
	call disp_msg
	mov ah, 51h
	int 21h
	mov ax, bx
	call disp_ax_hex
	mov si, msg.psp_and_size_between
	call disp_msg
	dec bx
	mov es, bx
	mov ax, word [es:3]
	call disp_ax_hex
	mov si, msg.psp_and_size_after
	call disp_msg


	mov si, msg.cmdline_before.app
	call disp_msg

	mov ah, 51h
	int 21h
	mov ds, bx
	mov si, 81h
	xor cx, cx
	mov cl, byte [si - 1]
	call disp_msg_length

	push cs
	pop ds
	mov si, msg.cmdline_after
	call disp_msg

	jmp .after_cmdline

.skip_psp_dos:
	test byte [cs:loadmode], 2
	jz .skip_device

	mov si, msg.cmdline_before.device
	call disp_msg

	les bx, [cs:dev_request_header]
	les di, [es:bx + 18]
	push es
	pop ds
	mov si, di

		; Writing MS-DOS Device Drivers, second edition, page 349
		;  specifies the following as to the command line termination:
		; "Note that the DEVICE= command string is terminated by an
		;  Ah when there are no arguments. When there are arguments,
		;  the string is terminated with the following sequence:
		;  0h, Dh, Ah."
	db __TEST_IMM8
@@:
	inc di
	cmp byte [di], 0
	je @F
	cmp byte [di], 13
	je @F
	cmp byte [di], 10
	jne @B
@@:					; di -> at terminator
	sub di, si
	mov cx, di
	call disp_msg_length

	push cs
	pop ds
	mov si, msg.cmdline_after
	call disp_msg

	jmp .after_cmdline

.skip_device:
	mov si, msg.cmdline.kern.none
	cmp word [bp + ldCommandLine], 0FF00h
	je .no_kernel_cmdline

	mov si, msg.cmdline_before.kern
	call disp_msg

	push ss
	pop ds
	lea si, [bp + ldCommandLine]
	call disp_msg

	push cs
	pop ds
	mov si, msg.cmdline_after
.no_kernel_cmdline:
	call disp_msg

.after_cmdline:
	int3

	test byte [cs:loadmode], 2
	jz .dos_or_bios

	jmp near [cs:dev_exit]

.dos_or_bios:
	test byte [cs:loadmode], 1
	jz .bios

	mov ax, 4C00h
	int 21h

.bios:
	xor ax, ax
	int 16h
	int 19h


disp_ax_hex:			; ax
		xchg al,ah
		call disp_al_hex		; display former ah
		xchg al,ah			;  and fall trough for al
disp_al_hex:			; al
		push cx
		mov cl,4
		ror al,cl
		call disp_al_lownibble_hex	; display former high-nibble
		rol al,cl
		pop cx
						;  and fall trough for low-nibble
disp_al_lownibble_hex:
		push ax			 ; save ax for call return
		and al,00001111b		; high nibble must be zero
		add al,'0'			; if number is 0-9, now it's the correct character
		cmp al,'9'
		jna .decimalnum		 ; if we get decimal number with this, ok -->
		add al,7			;  otherwise, add 7 and we are inside our alphabet
 .decimalnum:
		call disp_al
		pop ax
		retn


		; INP:	ds:si -> first byte to check for name
		;	cx = number of bytes left
		; OUT:	(8+1+3+1)bytes[es:msg.foundname] = found name,
		;	 converted to 8.3 ASCIZ format,
		;	 "(None)" if none
		;	CY if no filename found,
		;	 si = INP:si + INP:cx
		;	 cx = 0
		;	NC if filename found,
		;	 ds:si -> byte behind the name, thus ds:(si-11)-> name
		;	 cx = number of bytes left
		; CHG:	di, ax
findname:
.:
	cmp cx, 11		; enough for another name ?
	jb .none		; no -->
				; (cx == 0 jumps here too)
.check:
	push cx
	push si
	mov cx, 11
	lodsb
	mov ah, al		; check for same char in all 11 places
	cmp al, 32		; first character must not be blank
	je .check_fail		; if it is -->
;	cmp al, 5		; first character may be 05h to indicate 0E5h
;	je .check_pass
	db __TEST_IMM8		; (skip lodsb)
.check_loop_same:
	lodsb
	cmp ah, al
	jne .check_loop_differs
	call .check_character
	jc .check_fail
	loop .check_loop_same
		; if we arrive here, all characters (while valid) are the
		;  same character repeated 11 times. we disallow this in case
		;  that the padding character is an allowed one (eg '&' 26h).
.check_fail:
	pop si
	pop cx
	dec cx			; lessen the counter
	inc si			; -> next position to check
	jmp .

.check_character:
	cmp al, 32
	jb .check_character_fail
	cmp al, 127
;	je .check_character_fail
	jae .check_character_fail
		; note: with all characters >= 128 allowed,
		;  we get false positives in our sectors.
	cmp al, '.'
	je .check_character_fail
	cmp al, '/'
	je .check_character_fail
	cmp al, '\'
	je .check_character_fail
	cmp al, 'a'
	jb .check_character_pass
	cmp al, 'z'
	ja .check_character_pass
.check_character_fail:
	stc
	retn

.check_character_pass:
	clc
	retn

.check_loop:
	lodsb
.check_loop_differs:
	call .check_character
	jc .check_fail
.check_pass:
	loop .check_loop

	pop ax			; (discard si)
	sub si, 11		; -> at name

	call convert_name_to_asciz
				; si -> behind name
	pop cx
	sub cx, 11		; lessen the counter
	clc
	retn

.none:
	 add si, cx
	mov di, msg.foundname
	 push si
	 push ds
	push cs
	pop ds
	mov si, msg.foundname_none
	mov cx, (msg.foundname_none_size + 1) >> 1
	rep movsw
	 pop ds
	 pop si
	 xor cx, cx
	stc
	retn


		; INP:	ds:si -> 11-byte blank-padded name
		;	es:msg.foundname -> (8+1+3+1)-byte buffer
		; OUT:	ds:si -> behind 11-byte blank-padded name
		;	es:msg.foundname filled
		; CHG:	cx, di, ax
convert_name_to_asciz:
	mov di, msg.foundname
	mov cx, 8
	rep movsb		; copy over base name, si -> extension
	cmp byte [es:di - 8], 05h	; is it 05h ?
	jne @F			; no -->
	mov byte [es:di - 8], 0E5h	; yes, convert to 0E5h
@@:

	db __TEST_IMM8		; (skip dec)
@@:
	dec di			; decrement -> at previous trailing blank
	cmp byte [es:di - 1], 32	; trailing blank ?
	je @B			; yes -->

	mov al, '.'
	stosb			; store dot (if needed)
	mov cl, 3
	rep movsb		; copy over extension, si -> behind name

	db __TEST_IMM8		; (skip dec)
@@:
	dec di			; decrement -> at previous trailing blank
	cmp byte [es:di - 1], 32	; trailing blank ?
	je @B			; yes -->

	cmp byte [es:di - 1], '.'	; trailing dot ? (only occurs if all-blank ext)
	jne @F			; no -->
	dec di			; -> at the dot
@@:
	mov al, 0
	stosb			; store filename terminator
	retn


	align 4
table:
	dw  +0, "SS"
	dw  +6, "BP"
	dw  +4, "SP"
	dw +22, "CS"
	dw +20, "IP"
	dw +26, "FL"
	db -1, -1, 13,10
	dw  +2, "DS"
	dw +10, "SI"
	dw +18, "ES"
	dw  +8, "DI"
	db -1, -1, 13,10
	dw +24, "AX"
	dw +16, "BX"
	dw +14, "CX"
	dw +12, "DX"
	db -1, -1, 13,10
	dw +28, "S0"
	dw +30, "S1"
	dw +32, "S2"
	dw +34, "S3"
	dw +36, "S4"
	dw +38, "S5"
	dw +40, "S6"
	dw +42, "S7"
	db -1, -1, 13,10
	dw +44, "S8"
	dw +46, "S9"
	dw +48, "SA"
	dw +50, "SB"
	dw +52, "SC"
	dw +54, "SD"
	dw +56, "SE"
	dw +58, "SF"
	db -1, -1, 13,10
.end:


signature:
	dw 2638h
	align 16, db 0

%if _LARGE
	times 64 * 1024 db 0
%endif

signature2:
	dw 2638h

%if _PADDING
 %if ($ - $$) > _PADDING
  %warning No padding needed
 %else
	times _PADDING - ($ - $$) db 0
 %endif
%endif
