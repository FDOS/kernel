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

		; the UPX DOS/SYS depacker does not need a certain ss:sp
		;  however it appears to need cs == ds

	mov ds, word [di + patchcsip + 2 - $$]
	jmp 0:0
patchcsip:		equ $ - 4
end:

	times 0C0h - ($ - $$) nop
entry_common:

	times 100h - ($ - $$) db 0
	dw 0
	dw 0
	dw 0
	dw patchcsip
	dw end
