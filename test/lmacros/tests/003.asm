
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
	push bx
	push cx
	call fun3
	pop bx

	mov ax, 4C00h
	int 21h
%endif

fun3:
	lframe near
	lpar word, beta
	lpar_return
	lpar word, alpha
	lvar word, foo
	lvar word, bar
	lenter
	lvar word, quux
	 push ax

	mov word [bp + ?beta], 0BE7Ah

	lleave
	lret
