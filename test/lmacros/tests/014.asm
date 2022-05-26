
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
	call fun14

	mov ax, 4C00h
	int 21h
%endif

fun14:
	lframe near
	lvar word, foo
	lvar word, bar
	lenter
	lvar word, baz
	 push bx

	mov word [bp + ?foo], ax

	push cs
	push si
	lframe 0, inner
	lpar word, alpha
	lpar word, beta
	lpar_return
	lenter
	lvar word, quux
	lvar word, foo
	lreserve

	mov word [bp + ?foo], bx
	mov ax, word [bp + ?bar]
	mov bx, word [bp + ?quux]
	mov dx, word [bp + ?baz]
	mov si, word [bp + ?beta]
	inc word [bp + ?alpha]

	lleave

	mov ax, word [bp + ?foo]

	lleave
	lret
