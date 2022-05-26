
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
	mov cx, 1234h
	xor ax, ax
	 push ax
	call fun17

	mov ax, 4C00h
	int 21h
%endif

fun17:
	lframe near
	lpar word, alpha
	lvar word, foo
	lvar word, bar
	lenter		; push bp \ mov bp, sp \ push ax \ push ax

	lvar word, quux
	 push cx	; push cx
	lvar word, baz
	lvar dword, xyzzy
	lreserve	; lea sp, [bp - 0Ch]
	mov dx, word [bp + ?alpha]
			; [bp + 4]
	mov word [bp + ?foo], ax
			; [bp - 2]
	add ax, word [bp + ?quux]
			; [bp - 6]
	mov word [bp + ?xyzzy], dx
			; [bp - 0Ch]

	lframe 0, inner
	lvar word, e
	lvar word, f
	lvar word, g
	lenter		; lea sp, [bp - 12h]
	lvar word, h
	 push ax	; push ax
	lvar dword, i
	lreserve	; lea sp, [bp - 18h]

	lleave		; lea sp, [bp - 0Ch]

	lleave		; mov sp, bp \ pop bp
	lret		; retn 2
