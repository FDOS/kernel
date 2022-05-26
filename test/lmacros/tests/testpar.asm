
%include "lmacros2.mac"

	cpu 8086
	org 100h
start:
	mov ax, 1
	mov bx, 2
	 push ax
	 push bx
	call fun

	mov cx, 3
	mov dx, 4
	 push cx
	 push dx
	call fun2
	 pop ax

	mov word [fifth], ax

	mov ax, 4C00h
	int 21h

fun:
	lframe near
	lpar word, alpha
	lpar word, beta
	lenter
	mov ax, [bp + ?alpha]
	mov word [first], ax
	mov ax, [bp + ?beta]
	mov word [second], ax
	lleave
	lret


fun2:
	lframe near
	lpar word, gamma
	lpar_return
	lpar word, delta
	lenter
	mov ax, [bp + ?gamma]
	mov word [third], ax
	mov ax, [bp + ?delta]
	mov word [fourth], ax
	mov word [bp + ?gamma], 5
	lleave
	lret

	align 256
first:	dw 0
second:	dw 0
third:	dw 0
fourth:	dw 0
fifth:	dw 0

