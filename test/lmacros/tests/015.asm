
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
	call fun15

	mov ax, 4C00h
	int 21h
%endif

fun15:
.1:
	lframe near
	lpar word, alpha
	lvar word, foo
	lvar word, bar
	lenter
	lvar word, baz
	 push bx
	jmp .common

.2:
%ifdef FAIL3
bits 32
%endif
%ifdef FAIL4
	lframe far, nested
%else
	lframe near, nested
%endif
	lpar word, alpha
%ifdef FAIL2
	lpar_return
%endif
	lvar word, foo
%ifdef FAIL5
	lvar word, foobar
%else
	lvar word, bar
%endif
%ifdef FAIL1
	lvar word, quux
%endif
	lenter
	lvar word, baz
	 push bx

	ldup

	lleave ctx

.common:
	mov dx, word [bp + ?alpha]
	mov word [bp + ?foo], ax

	lleave
	lret
