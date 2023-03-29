
%if 0

FreeDOS kernel executable MZ header shim
 by C. Masloch, 2022

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif

%include "lmacros2.mac"

	defaulting

	strdef FILE, ""
%ifidn _FILE,""
 %fatal Has to specify a file!
%endif


	org 0
header:
	db "MZ"		; exeSignature
	dw (payload.end - $$) % 512		; exeExtraBytes
	dw (payload.end - $$ + 511) / 512	; exePages
	dw 0		; exeRelocItems
	dw (payload -$$+0) >> 4	; exeHeaderSize
	dw 0		; exeMinAlloc
	dw -1		; exeMaxAlloc
	dw (payload.end + 15 - payload) / 16	; exeInitSS
	dw 512		; exeInitSP
	dw 0		; exeChecksum
	dw 0, 0		; exeInitCSIP
	dw 0		; exeRelocTable
	endarea header


	align 16, db 0
payload:
	jmp strict short entry
	db "CONFIG"
	dw 1
	db -1

	times 32 - ($ - payload) db 0
entry: equ $
	jmp entry_common

	times 0xC0 - ($ - payload) nop
entry_common: equ $

	incbin _FILE
.actual_end:
.end:
