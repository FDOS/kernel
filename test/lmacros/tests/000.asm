
%assign _@@_check 2
%assign _@@_start 0
%include "lmacros2.mac"

%ifdef _MAP
[map symbols brief _MAP]
%endif

	cpu 8086
	org 256
	sectalign off
	section lCode start=256 align=1

start: @S
	jmp @F
	nop
	nop
@@:

;%fatal 1>@F1< 2>@F2< 3>@F3< 4>@F4< 
	jz @F
	nop
	nop
@@:

	jnz @F
	nop
	nop
@@:

	call @F
	nop
@@:

exit:
	mov ax, 4C00h
	int 21h
	align 16

second: @S 0
	jmp @F
	nop
	nop
@@:

;%fatal 1>@F1< 2>@F2< 3>@F3< 4>@F4< 
	jz @F
	nop
	nop
@@:

	jnz @F
	nop
	nop
@@:

	call @F
	nop
@@:
	jmp exit
	align 16

	jmp @B
	jmp @BB
	jmp @B3
	jmp @B4

