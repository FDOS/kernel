
%if 0

Loader adjustment to load FreeDOS kernel
 by C. Masloch, 2017

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif


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


%ifndef _MAP
%elifempty _MAP
%else	; defined non-empty, str or non-str
	[map all _MAP]
%endif

	strdef PAYLOAD_FILE,	"KERNEL.SYS"


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


	cpu 8086
	org 0
	addsection ENTRY, start=0 vstart=0
entry:

	mov ax, cs
	add ax, entry_size_p + payload_size_p
	xor bx, bx
	push ax
	push bx
	retf

	align 16
	endarea entry


	addsection PAYLOAD, follows=ENTRY
payload:
realpayload:
	incbin _PAYLOAD_FILE
	align 16, db 38
	endarea realpayload


	addsection STACKRELOCATE, follows=PAYLOAD vstart=0
stackrelocate:

	mov ax, 60h + payload_size_p
	mov es, ax
	xor di, di
	xor si, si
	lea cx, [ bp + 512 ]
	rep movsb
	mov ds, ax
	mov si, bp
	xor di, di
	cmp word [ds:bp + ldCommandLine], 0FF00h
	je @F
	lea si, [bp + ldCommandLine + lsvclBufferLength - 2]
	lea di, [bp + lsvCommandLine.start + lsvclBufferLength - 2]
	mov cx, words(lsvclBufferLength)
%if words(lsvclBufferLength) <= 20
 %error AMD erratum 109 workaround needed
%endif
	std
	rep movsw
	cld
	lea si, [bp + lsvCommandLine.start]
	mov di, lsvclSignature
@@:
	cli
	mov ss, ax
	mov sp, si
	mov word [bp + lsvCommandLine.signature], di
		; Note that this access uses the new ss.
		; Also note: If no command line is passed,
		;  si will equal bp. That means the word
		;  written here is technically below sp,
		;  that is it belongs to the unused stack.
		; This does not cause any problems however.
		; It hardens the next load stage against
		;  accidentally expecting a command line if
		;  it does not check the offsets properly.
	sti

	jmp 60h:0

	align 16
	endarea stackrelocate

payload_size equ realpayload_size + stackrelocate_size
	endarea payload, 1


	addsection RELOCATE, follows=STACKRELOCATE vstart=0
relocate:
	mov bx, 1000h
	mov ax, 60h
	mov es, ax
	mov cx, payload_size_p
	mov ax, cs
	sub ax, cx
	mov ds, ax
	xor si, si
	xor di, di

	mov ax, cx
	cmp ax, bx
	jbe @F
	mov cx, bx
@@:
	sub ax, cx
	shl cx, 1
	shl cx, 1
	shl cx, 1
	rep movsw

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
	mov cx, ax	; that many words
	xor ax, ax	; no more to relocate after this round

@@:
	xor si, si
	xor di, di
	rep movsw	; relocate next chunk
	test ax, ax	; another round needed?
	jnz @BB		; yes -->
			; ax = 0

	mov dl, [bp + bsBPB + ebpbNew + bpbnBootUnit]
	mov bl, dl

	 push ss
	 pop ds
	cmp word [bp + bsBPB + bpbSectorsPerFAT], ax
	je @F
	 push ss
	 pop es
	lea si, [bp + bsBPB + ebpbNew]
	lea di, [bp + bsBPB + bpbNew]
	mov cx, (512 - bsBPB - bpbNew + 1) >> 1
	rep movsw
@@:

	jmp 60h + realpayload_size_p:0
