
%if 0

Shut down machine
 by C. Masloch, 2020

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif


	cpu 8086
	org 256
quit:
	int3

	mov ax, 0F000h
	mov es, ax
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

	mov dx, msg.failed
	mov ah, 09h
	int 21h
	mov ax, 4C00h
	int 21h


	align 4
msg:
.dosemudate:	db "02/25/93"
.failed:	db "Quit failed.",13,10,36
