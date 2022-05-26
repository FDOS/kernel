
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
	call fun1

	mov ax, 4C00h
	int 21h
%endif

fun1:
	lframe near
	lvar word, foo
	lvar word, bar
	lenter
	lvar word, quux
	 push ax

	lleave
	lret
