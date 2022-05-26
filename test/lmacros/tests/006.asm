
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
	call fun6

	mov ax, 4C00h
	int 21h
%endif

fun6:
	lframe near
	lvar word, foo
	lvar word, bar
	lenter

	mov word [bp + ?foo], ax

	lframe 0, nested
	lvar word, qux
	lvar word, foo
	lenter

	mov word [bp + ?foo], bx

	lleave

	mov ax, word [bp + ?foo]

	lleave
	lret
