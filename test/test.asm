
%if 0

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif

%include "lmacros3.mac"

	cpu 8086
	org 256
start:
	mov ah, 3Ch
	xor cx, cx
	mov dx, filename
	int 21h
	mov dx, msg.creating
	jc error.nofile
	xchg bx, ax
	mov ah, 40h
	mov dx, buffer
	mov cx, buffer.size
	int 21h
	mov dx, msg.writing
	jc error
	cmp ax, cx
	mov dx, msg.full
	jne error
	mov ah, 3Eh
	int 21h
	xor ax, ax
	jmp quit

error:
	mov ah, 3Eh
	int 21h
.nofile:
	push ax
	mov ah, 09h
	int 21h
	pop ax
	call disp_ax_hex
	mov al, 13
	call disp_al
	mov al, 10
	call disp_al
	mov ax, -1
	jmp quit


		; INP:	al = character to display
		; CHG:	-
		; STT:	ds, es don't care
disp_al:
	push ax
	push dx
	xchg ax, dx
	mov ah, 02h
	int 21h
	pop dx
	pop ax
	retn


		; Display number in ax hexadecimal, always 4 digits
		;
		; INP:	ax = number
		; OUT:	displayed using disp_al
		; CHG:	none
disp_ax_hex:
	xchg al, ah
	call disp_al_hex
	xchg al, ah
disp_al_hex:
	push cx
	mov cl, 4
	rol al, cl
	call disp_al_nybble_hex
	rol al, cl
	pop cx
disp_al_nybble_hex:
	push ax
	and al, 0Fh
	add al, '0'
	cmp al, '9'
	jbe @F
	add al, -'9' -1 +'A'
@@:
	call disp_al
	pop ax
	retn


%if 0

Shut down machine
 by C. Masloch, 2020

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif


	cpu 8086
	; org 256
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
.creating:	ascic "Creating failed, code="
.writing:	ascic "Writing failed, code="
.full:		ascic "Writing failed, full. AX="
filename:
	asciz "result.txt"
buffer:
.:		db "success"
.size: equ $ - .
