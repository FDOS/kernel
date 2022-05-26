
%if 0

Multiboot header and loader
 2008--2019 by C. Masloch

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif

%macro mbootchecksummed 1-2.nolist 1BADB002h
	dd %2		; signature
	dd %1		; flags
	dd -((%2+%1) & 0FFFF_FFFFh)
			; checksum: dword sum of these three = 0
%endmacro

%macro mboot2checksummed 0-1.nolist 0E852_50D6h
	dd %1		; signature
	dd 0		; platform (i386)
	dd mboot2_header_end - mboot2_header
			; size of header, including header tags
	dd -((%1 + (mboot2_header_end - mboot2_header))) & 0FFFF_FFFFh
			; checksum
%endmacro


MULTIBOOT_CMDLINE_LENGTH equ lsvclBufferLength

MULTIBOOT_BASE equ (1024+64)*1024	; one paragraph above the HMA
MULTIBOOT_END equ MULTIBOOT_BASE + (payload.actual_end - $$ + 0)
MULTIBOOT_BSS_END equ MULTIBOOT_END \
		+ fromdwords(dwords(MULTIBOOT_CMDLINE_LENGTH))
MULTIBOOT_TARGET_SEGMENT equ 200h
MULTIBOOT_TARGET equ MULTIBOOT_TARGET_SEGMENT << 4
MULTIBOOT_BPB equ MULTIBOOT_TARGET - 512 - 16
MULTIBOOT_CMDLINE_START equ MULTIBOOT_BPB + lsvCommandLine.start
MULTIBOOT_STACK_TOP equ MULTIBOOT_CMDLINE_START

%if _MULTIBOOT1
		; Multiboot header
		; must be dword aligned and in first 8 KiB!
	align 4, db 0
mbootheader:
	mbootchecksummed 00010000h
			; flags: provides info where to load image
	dd MULTIBOOT_BASE + mbootheader	; load address of header
	dd MULTIBOOT_BASE		; load address
	dd MULTIBOOT_END		; load area end (0 = load whole file)
	dd MULTIBOOT_BSS_END		; uninitialised area end (0 = none)
	dd MULTIBOOT_BASE + mbootentry	; entry point
%endif

%if _MULTIBOOT2
		; Multiboot2 header
		; qword (64-bit) aligned and in first 32 KiB
	align 8, db 0
mboot2_header:
	mboot2checksummed
mboot2_tag_information_request:
.:
.type:	dw 1
.flags:	dw 0
.size:	dd .end - .
	dd 1		; command line
	dd 5		; ROM-BIOS boot device
.end:
	align 8, db 0
mboot2_tag_addresses:
.:
.type:	dw 2
.flags:	dw 0
.size:	dd .end - .
	dd MULTIBOOT_BASE + mboot2_header
					; load address of header
	dd MULTIBOOT_BASE		; load address
	dd MULTIBOOT_END		; load area end (0 = load whole file)
	dd MULTIBOOT_BSS_END		; uninitialised area end (0 = none)
.end:
	align 8, db 0
mboot2_tag_entrypoint:
.:
.type:	dw 3
.flags:	dw 0
.size:	dd .end - .
	dd MULTIBOOT_BASE + mboot2entry	; entry point
.end:
	align 8, db 0
mboot2_tag_end:
.:
.type:	dw 0
.flags:	dw 0
.size:	dd .end - .
.end:
mboot2_header_end:
%endif


%unmacro mbootchecksummed 1-2.nolist 1BADB002h
%unmacro mboot2checksummed 0-1.nolist 0E85250D6h

%if ($-$$) > 8192
 %fatal Multiboot header must be in the first 8 KiB
%endif

	struc MBI
mbiFlags:	resd 1
		resb 8
mbiBootDevice:	resd 1
mbiCmdLine:	resd 1
mbiModuleCount:	resd 1
mbiModuleTable:	resd 1
		; (More data follows but none which seems useful.)
	endstruc

[cpu 386]
[bits 32]

	numdef MULTIBOOT_CHECKS, 1
	numdef MULTIBOOT_DEBUG, 0
	numdef MULTIBOOT_HALT, 0
	numdef MULTIBOOT_PROGRESS, 0


 %if _MULTIBOOT_CHECKS || _MULTIBOOT_HALT
mbootentry.halt:
mboot2entry.halt:
		; Unfortunately, as the environment is unknown, we can only
		;  literally halt here, as we don't know how to do I/O.
	cli
@@:
	hlt
	jmp short @B
 %endif

%if _MULTIBOOT1
		; INP:	eax = 2BADB002h
		;	ebx-> multiboot info structure
		; STT:	loaded and executed according to Multiboot header, ie
		;	 loaded at MULTIBOOT_BASE
		;	 eip = MULTIBOOT_BASE+mbootentry
		;	in PM32 with paging disabled
		;	CPL 0
		;	cs,ds,es,fs,gs,ss = flat 32-bit code/data selectors
		;	DI
		;	! stack not set up
		;	! GDT might not be set up
		;	! IDT might not be set up
		;	A20 on
		;
		; Note:	Although A20 is on, HMA access without an XMM
		;	 to manage it might be undesirable. Multiboot
		;	 also doesn't tell us whether and if so then
		;	 how we can switch A20.
mbootentry:
 %if _MULTIBOOT_HALT
	jmp .halt
 %endif
 %if _MULTIBOOT_CHECKS
[bits 16]
	test ax, 0			; (test eax if in 32-bit segment)
	jmp short .halt			; (skipped if in 32-bit segment)
[bits 32]
	cmp eax, 2BADB002h		; signature ?
	jne short .halt			; no -->
	smsw eax
	rol eax, 1			; 2 = protection, 1 = paging
	and al, 3			; mask off others
	cmp al, 2			; PE but not PG ?
	jne short .halt			; no -->
	mov eax, cs
	and al, 3			; CPL 0 ?
	jnz short .halt			; no -->
 %endif

 %if _MULTIBOOT_PROGRESS
	mov ebp, 0B8000h + 2 * 80 * 20
	mov word [ebp], 5000h | '1'
	inc ebp
	inc ebp
 %endif

		; prepare this and that
	or edx, -1
		; locate boot drive in Multiboot info structure
	test byte [ ebx + mbiFlags ], 2	; boot device info valid ?
	jz @F
	mov eax, [ ebx + mbiBootDevice ]; get the info
	rol eax, 16
	xchg al, ah
	mov edx, eax			; dl = boot load unit (or 0FFh),
					;  dh = partition (or 0FFh)
					;  (edxh = subpartition numbers)
@@:

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | '2'
	inc ebp
	inc ebp
 %endif

	xor eax, eax
	mov edi, MULTIBOOT_END
	mov ecx, MULTIBOOT_CMDLINE_LENGTH / 4
	test byte [ ebx + mbiFlags ], 4	; command line valid ?
	jz @F
	mov esi, [ ebx + mbiCmdLine ]
	rep movsd
	mov byte [ edi - 1 ], 0		; insure it is terminated
					; (this may truncate the line)
@@:
	rep stosd

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | '3'
	inc ebp
	inc ebp
 %endif

 %if _MULTIBOOT2
	jmp multiboot_1_2_common
 %endif
%endif

%if _MULTIBOOT2
		; INP:	eax = 36D7_6289h
		;	ebx-> multiboot2 info structure
		; STT:	loaded and executed according to Multiboot2 header, ie
		;	 loaded at MULTIBOOT_BASE
		;	 eip = MULTIBOOT_BASE+mboot2entry
		;	in PM32 with paging disabled
		;	CPL 0
		;	cs,ds,es,fs,gs,ss = flat 32-bit code/data selectors
		;	DI
		;	! stack not set up
		;	! GDT might not be set up
		;	! IDT might not be set up
		;	A20 on
		;
		; Note:	Although A20 is on, HMA access without an XMM
		;	 to manage it might be undesirable. Multiboot
		;	 also doesn't tell us whether and if so then
		;	 how we can switch A20.
mboot2entry:
 %if _MULTIBOOT_HALT
	jmp .halt
 %endif
 %if _MULTIBOOT_CHECKS
[bits 16]
	test ax, 0			; (test eax if in 32-bit segment)
	jmp short .halt			; (skipped if in 32-bit segment)
[bits 32]
	cmp eax, 36D7_6289h		; signature ?
	jne .halt			; no -->
	smsw eax
	rol eax, 1			; 2 = protection, 1 = paging
	and al, 3			; mask off others
	cmp al, 2			; PE but not PG ?
	jne .halt			; no -->
	mov eax, cs
	and al, 3			; CPL 0 ?
	jnz .halt			; no -->
 %endif

 %if _MULTIBOOT_PROGRESS
	mov ebp, 0B8000h + 2 * 80 * 20
	mov word [ebp], 5000h | 'A'
	inc ebp
	inc ebp
 %endif

	or edx, -1	; initialise boot partition info

	xor eax, eax
	mov edi, MULTIBOOT_END
	mov ecx, MULTIBOOT_CMDLINE_LENGTH / 4
	rep stosd	; initialise command line buffer

	add ebx, 8	; skip fixed header (size, reserved)
.tags_loop:

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 'B'
	inc ebp
	inc ebp
 %endif

	mov eax, dword [ebx]
	test eax, eax	; end of tags ?
	jz .tags_end	; yes -->

	cmp eax, 1
	je .cmdline

	cmp eax, 5
	je .partition

.tags_next:
 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 'C'
	inc ebp
	inc ebp
 %endif

	add ebx, dword [ebx + 4]
			; -> after end of tag
			;  (at padding or next tag)
	add ebx, 7
	and ebx, ~7	; skip any padding
	jmp .tags_loop

.cmdline:

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 'D'
	inc ebp
	inc ebp
 %endif

	lea esi, [ebx + 8]
	mov edi, MULTIBOOT_END
	mov ecx, MULTIBOOT_CMDLINE_LENGTH / 4
	rep movsd	; copy command line
	mov byte [ edi - 1 ], 0
			; insure it is terminated
			;  (this may truncate the line)
	jmp .tags_next

.partition:

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 'E'
	inc ebp
	inc ebp
 %endif

	mov eax, dword [ebx + 8]
	cmp eax, 100h
	jae @F
	mov dl, al	; get ROM-BIOS unit
@@:

	mov eax, dword [ebx + 12]
	cmp eax, 100h
	jae @F
	mov dh, al	; get partition number (0 = first primary)
@@:
	jmp .tags_next

.tags_end:
	xor eax, eax

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 'F'
	inc ebp
	inc ebp
 %endif

%endif

multiboot_1_2_common:
	mov esp, MULTIBOOT_STACK_TOP	; set a valid stack, at 8 KiB
					;  (also required by 86M entry later)

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 's'
	inc ebp
	inc ebp
 %endif

		; A20 is on. set up some things
	xor ecx, ecx
	mov edi, 1024*1024
	mov cl, 10h			; (ecx = 10h)
	rep stosd			; clear this area so A20 tests succeed
; The above writes to 10_0000h..10_003Fh, leaving edi = 10_0040h
	mov al, 0C0h			; (eax = C0h)
	mov byte [ edi-40h+eax ], 0EAh
	mov [ edi-40h+eax+1 ], eax	; write jump for CP/M entry hack here
; The above writes to 10_00C0h..10_00C4h

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 't'
	inc ebp
	inc ebp
 %endif

		; relocate image to a good location, 2000h
	mov esi, MULTIBOOT_BASE		; -> iniload image
	mov edi, MULTIBOOT_TARGET
	mov ecx, (payload.actual_end - $$ + 0 + 3) / 4
	rep movsd

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 'u'
	inc ebp
	inc ebp
 %endif

MULTIBOOT_ESI_AFTER equ MULTIBOOT_BASE + (payload.actual_end - $$ + 0 + 3) / 4 * 4
%if MULTIBOOT_ESI_AFTER != MULTIBOOT_END
	mov esi, MULTIBOOT_END
%endif
	mov edi, MULTIBOOT_CMDLINE_START
	mov ecx, MULTIBOOT_CMDLINE_LENGTH / 4
	rep movsd

 %if _MULTIBOOT_PROGRESS
	mov word [ebp], 5000h | 'v'
	inc ebp
	inc ebp
 %endif

		; now back to real mode
	o32 lgdt [ MULTIBOOT_BASE+mbootgdtdesc ]
					; set our GDT
	mov ax, 10h
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax			; set 64 KiB segment limits
	o32 push byte 0
	o32 popf			; reset all flags
	jmp 08h:.pm16			; use 16-bit selector

[bits 16]
		; now really switch to real mode
		; (already executing relocated code)
.pm16:
	mov eax, cr0
	dec ax
	mov cr0, eax				; clear PE bit

	jmp dword MULTIBOOT_TARGET_SEGMENT:.rm	; reload cs

		; Set up registers and the environment for
		; the kernel entry point. It doesn't need initialised
		; segment registers but we'll initialise them in case
		; they still contain selector content.
.rm:
		; some sources indicate we should set the IDT
	o32 lidt [cs:mbootidtdesc]		; set IDT to RM IVT
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	; (cs should be = 200h now, need no far jump)
		; cs = 200h, ip = mboot_86m_entry
		; ds = es = fs = gs = ss = 0
		; sp = 2000h - 512 - 16 + lsvCommandLine.start
		; dl = boot unit (or -1)
		; dh = boot partition (0..3 = primary, 4+ = logical, or -1)

	mov bp, MULTIBOOT_BPB			; -> pseudo BPB
	mov di, MULTIBOOT_BPB + lsvCommandLine.signature
	mov ax, lsvclSignature
	stosw					; store signature
	mov cx, (- (lsvCommandLine.signature + 2) + 512 + 3) / 4
	xor eax, eax
	rep stosd				; clear
		; lsvFirstCluster = 0 (same as for FreeDOS entrypoint)
		; bsBPB + bpbHiddenSectors = 0 (invalid or unpartitioned)
		; bsBPB + bpbCHSSectors = 0
		; bsBPB + bpbCHSHeads = 0

	inc ax
	mov dword [bp + lsvDataStart], eax
		; lsvDataStart = 1 (placeholder)
__CPU__
	mov word [bp + bsBPB + bpbNumRootDirEnts], ax
	mov word [bp + bsBPB + bpbSectorsPerFAT], ax
			; do not detect as FAT32
	mov byte [bp + bsBPB + bpbSectorsPerCluster], al
	mov word [bp + bsBPB + bpbBytesPerSector], 512
	mov word [bp + bsBPB + bpbTotalSectors], 8192
			; 1 C/s * 8 Ki sectors = 8 Ki clusters, detect as FAT16
			;  that is, do not load FAT at all (would load a FAT12)

	mov byte [bp + bsBPB + bpbNew + bpbnBootUnit], dl
			; set boot load unit

%if _LSVEXTRA
 %if _MULTIBOOT_DEBUG
	mov ax, dx
	call mb_disp_ax_hex
 %endif
	xchg dl, dh	; dl = Multiboot spec partition number
			; (0..3 primary, 4+ logical, -1 invalid)
	inc dx		; 0 invalid, 1..4 primary, 5+ logical
	mov dh, lsvefPartitionNumber
	mov word [bp + lsvExtra], dx
%endif
	jmp freedos_entry.multiboot_entry


%if _MULTIBOOT_DEBUG
mb_disp_ax_hex:			; ax
		xchg al,ah
		call mb_disp_al_hex		; display former ah
		xchg al,ah			;  and fall trough for al
mb_disp_al_hex:			; al
		push cx
		mov cl,4
		ror al,cl
		call mb_disp_al_lownibble_hex	; display former high-nibble
		rol al,cl
		pop cx
						;  and fall trough for low-nibble
mb_disp_al_lownibble_hex:
		push ax			 ; save ax for call return
		and al,00001111b		; high nibble must be zero
		add al,'0'			; if number is 0-9, now it's the correct character
		cmp al,'9'
		jna .decimalnum		 ; if we get decimal number with this, ok -->
		add al,7			;  otherwise, add 7 and we are inside our alphabet
 .decimalnum:
		call mb_disp_al
		pop ax
		retn

mb_disp_al:
	push ax
	push bx
	push bp
	mov ah, 0Eh
	mov bx, 7
	int 10h
	pop bp
	pop bx
	pop ax
	retn
%endif


	align 16, db 0
mbootgdt:
	dw 0,0
	db 0,0,0,0

		; selector 8: 16-bit real mode CS
		; base = 00002000h, limit 0FFFFh (1 B Granularity), present
		; type = 16-bit code execute/read only/conforming, DPL = 0
	dw 0FFFFh,MULTIBOOT_TARGET
	db 0,9Eh,0,0

		; selector 10h: 16-bit real mode DS
		; base = 00000000h, limit 0FFFFh (1 B Granularity), present
		; type = 16-bit data read/write, DPL = 0
	dw 0FFFFh,0
	db 0,92h,0,0
	endarea mbootgdt

mbootgdtdesc:
	dw mbootgdt_size-1	; limit
	dd MULTIBOOT_BASE+mbootgdt	; address

mbootidtdesc:
	dw 400h-1		; limit
	dd 0			; address (86M IVT)
