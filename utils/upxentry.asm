
%if 0

UPX preparation stub for DOS/EXE format compressed FreeDOS kernel
 Public Domain by C. Masloch, 2022

%endif

	cpu 8086
	org 0

bootloadunit:		; (byte of short jump re-used)
start:
	jmp strict short entry
	times (32 - 4) - ($ - $$) db 0
			; area for CONFIG block

bootloadstack:		; (dword re-used for original ss:sp)
entry:
		; common setup (copied from kernel.asm)
	push cs
	pop ds
	xor di, di
	mov byte [di + bootloadunit - $$], bl
	push bp
	mov word [di + bootloadstack - $$], sp
	mov word [di + bootloadstack + 2 - $$], ss

		; the UPX DOS/EXE depacker needs a certain ss:sp
	cli
	mov ax, 0
patchstacksegment:	equ $ - 2
	mov ss, ax
	mov sp, 0
patchstackpointer:	equ $ - 2
	sti

	mov ax, -10h
patchpspsegment:	equ $ - 2
	mov ds, ax
	mov es, ax

	jmp 0:0
patchcsip:		equ $ - 4
end:

	times 0C0h - ($ - $$) nop
entry_common:

	times 100h - ($ - $$) db 0
	dw patchstackpointer
	dw patchstacksegment
	dw patchpspsegment
	dw patchcsip
	dw end
