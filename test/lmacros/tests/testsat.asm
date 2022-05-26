%include "lmacros2.mac"

	[map all testsat.map]
	cpu 8086
	org 0

struc TESTSTRUC
	resb 1
struc_at 1, at_1:	resb 3
struc_at 4, at_4:
	resw 1
exact_struc_at 6
at_6:	resb 1
struc_at 32
at_32:	resb 10
warn_struc_at 64
at_64:	resb 1
endstruc
