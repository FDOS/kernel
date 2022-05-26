
%include "lmacros2.mac"

%ifdef _MAP
[map symbols brief _MAP]
%endif

	cpu 8086
	org 256
	sectalign off
	section lCode start=256 align=1

start:
	call fun1
	push ax
	call fun2
	push bx
	push cx
	call fun3
	pop bx
	call fun4
	call fun5
	call fun6
	call fun7
	call fun8
	call fun9
	call fun10
	call fun11
	call fun12
	call fun13
	call fun14

	mov ax, 4C00h
	int 21h

	overridedef STANDALONE, 0
%include "001.asm"
%include "002.asm"
%include "003.asm"
%include "004.asm"
%include "005.asm"
%include "006.asm"
%include "007.asm"
%include "008.asm"
%include "009.asm"
%include "010.asm"
%include "011.asm"
%include "012.asm"
%include "013.asm"
%include "014.asm"
	resetdef
