
%include "lmacros2.mac"

	numdef STANDALONE, 1

%if _STANDALONE
%ifdef _MAP
[map symbols brief _MAP]
%endif

	cpu 8086
	org 256
	sectalign off
	section lCode start=256 align=1

start:
	xor ax, ax
	 push ax
	call fun16
	mov ax, 1
	 push bx
	call fun16

	mov ax, 4C00h
	int 21h
%endif

fun16:
	mov bx, 0B3B3h
	mov dx, 0D3D3h
	lframe near
	lpar word, alpha
	lvar word, foo
	lvar word, bar
	lenter
	test ax, ax
	jnz .handler2

.handler1:
	lvar word, quux
	 push cx
	mov dx, word [bp + ?alpha]
	mov word [bp + ?foo], ax
	add ax, word [bp + ?quux]

	lleave
	lret

.handler2:
	lframe near
	lemit off
	lpar word, alpha
	lvar word, foo
	lvar word, bar
	lenter
	lemit
	lvar word, baz
	 push dx

	mov bx, word [bp + ?alpha]
	mov word [bp + ?foo], ax

	lleave
	lret
