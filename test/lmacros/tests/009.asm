
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
	call fun9

	mov ax, 4C00h
	int 21h
%endif

fun9:
	lframe near
	lvar word, foo
	lvar word, bar
	lenter

	mov word [bp + ?foo], ax

	push cs
	push si

	lframe 0, inner
	lpar word, segment
	lpar word, offset
	lvar word, qux
	lvar word, foo
	lvar word, xyzzy
	lenter

	mov word [bp + ?foo], bx
	mov ax, word [bp + ?bar]
	mov bx, word [bp + ?segment]
	mov dx, word [bp + ?offset]

	lleave

	mov ax, word [bp + ?foo]

	lleave
	lret
