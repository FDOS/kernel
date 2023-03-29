
%include "lmacros2.mac"

%ifdef _MAP
[map symbols brief _MAP]
%endif

	cpu 8086
	org 100h
start:
	and sp, ~15
	call fun
	mov word [first], ax

	sub sp, 16 - 4
	mov cx, 3
	mov dx, 4
	 push cx
	 push dx
	call fun2
	 pop ax

	mov word [fifth], ax

	mov ax, 4C00h
	int 21h

fun:
	lframe near
	lenter
	lvar word, foo
	lalign 16
%ifn %$alignment
 %error Should need alignment
%endif
%if %$alignment != 10
 %error Should need 10-byte alignment
%endif
	lreserve
	lalign 16
%if %$alignment
 %error Should not need alignment
%endif
	lalign 8
%if %$alignment
 %error Should not need alignment
%endif
	mov ax, 1
	test sp, 15
	jz @F
	neg ax
@@:
	lleave
	lret


fun2:
	lframe near
	lpar word, gamma
	lpar_return
	lpar word, delta
	lenter
	mov ax, [bp + ?gamma]
	mov word [third], ax
	mov ax, [bp + ?delta]
	mov word [fourth], ax
	mov word [bp + ?gamma], 5

	lalign 16
%ifn %$alignment
 %error Should need alignment
%endif
%if %$alignment != 12
 %error Should need 12-byte alignment
%endif
	lreserve
	call fun
	mov word [second], ax

	lleave
	lret

	align 256
first:	dw 0
second:	dw 0
third:	dw 0
fourth:	dw 0
fifth:	dw 0
