%define _@@_check
%assign _@@_start 0
%include "lmacros2.mac"

@S
	nop
@@:
	nop
	jmp @F
	nop
	nop
	jmp @B

@@:
	nop
	nop
@I

	nop
	nop
@S
@@:
	nop
	nop
	jmp @B
@I
