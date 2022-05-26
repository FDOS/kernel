
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
	call fun5

	mov ax, 4C00h
	int 21h
%endif

fun5:
	lframe near
	lenter
	lvar word, foo
	 push bx
	lvar word, bar
	lreserve
	lvar word, quux
	 push ax

	lleave
	lret
