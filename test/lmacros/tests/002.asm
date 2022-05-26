
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
	push ax
	call fun2

	mov ax, 4C00h
	int 21h
%endif

fun2:
	lframe near
	lpar word, alpha
	lvar word, foo
	lvar word, bar
	lenter
	lvar word, quux
	 push ax

	mov word [bp + ?foo], 0F00h
	mov dx, word [bp + ?alpha]

	lleave
	lret
