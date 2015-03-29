; Hardcoded DOS-NLS information for country = 1, codepage = 437
; This is an automatically generated file!
; Any modifications will be lost!

; Prerequisites:
;; ==> Assuming that data of tables remains constant all the time
;; ==> Reordering tables 1, 2, 4 and 5

	%include "segs.inc"
segment CONST2

	GLOBAL _nlsPackageHardcoded
_nlsPackageHardcoded:
	DB  000h, 000h, 000h, 000h, 001h, 000h, 0b5h, 001h
	DB  00fh, 000h, 059h, 000h, 04eh, 000h, 006h, 000h
	DB  002h
	DW ?table2, SEG ?table2
	DB  004h
	DW ?table4, SEG ?table4
	DB  005h
	DW ?table5, SEG ?table5
	DB  006h
	DW ?table6, SEG ?table6
	DB  007h
	DW ?table7, SEG ?table7
	GLOBAL _nlsCountryInfoHardcoded
_nlsCountryInfoHardcoded:
	DB  001h
	GLOBAL _nlsCntryInfoHardcoded
_nlsCntryInfoHardcoded:
?table1:
	DB  01ch, 000h, 001h, 000h, 0b5h, 001h, 000h, 000h
	DB  024h, 000h, 000h, 000h, 000h, 02ch, 000h, 02eh
	DB  000h, 02dh, 000h, 03ah, 000h, 000h, 002h, 000h
extern _CharMapSrvc:wrt DGROUP
        DW  _CharMapSrvc, SEG _CharMapSrvc
        DB  02ch, 000h
	GLOBAL _hcTablesStart
_hcTablesStart:
	GLOBAL _nlsUpcaseHardcoded
_nlsUpcaseHardcoded:
?table2:
	DB  080h, 000h, 080h, 09ah, 045h, 041h, 08eh, 041h
	DB  08fh, 080h, 045h, 045h, 045h, 049h, 049h, 049h
	DB  08eh, 08fh, 090h, 092h, 092h, 04fh, 099h, 04fh
	DB  055h, 055h, 059h, 099h, 09ah, 09bh, 09ch, 09dh
	DB  09eh, 09fh, 041h, 049h, 04fh, 055h, 0a5h, 0a5h
	DB  0a6h, 0a7h, 0a8h, 0a9h, 0aah, 0abh, 0ach, 0adh
	DB  0aeh, 0afh, 0b0h, 0b1h, 0b2h, 0b3h, 0b4h, 0b5h
	DB  0b6h, 0b7h, 0b8h, 0b9h, 0bah, 0bbh, 0bch, 0bdh
	DB  0beh, 0bfh, 0c0h, 0c1h, 0c2h, 0c3h, 0c4h, 0c5h
	DB  0c6h, 0c7h, 0c8h, 0c9h, 0cah, 0cbh, 0cch, 0cdh
	DB  0ceh, 0cfh, 0d0h, 0d1h, 0d2h, 0d3h, 0d4h, 0d5h
	DB  0d6h, 0d7h, 0d8h, 0d9h, 0dah, 0dbh, 0dch, 0ddh
	DB  0deh, 0dfh, 0e0h, 0e1h, 0e2h, 0e3h, 0e4h, 0e5h
	DB  0e6h, 0e7h, 0e8h, 0e9h, 0eah, 0ebh, 0ech, 0edh
	DB  0eeh, 0efh, 0f0h, 0f1h, 0f2h, 0f3h, 0f4h, 0f5h
	DB  0f6h, 0f7h, 0f8h, 0f9h, 0fah, 0fbh, 0fch, 0fdh
	DB  0feh, 0ffh
	GLOBAL _nlsFUpcaseHardcoded
_nlsFUpcaseHardcoded:
?table4:
	DB  080h, 000h, 080h, 09ah, 045h, 041h, 08eh, 041h
	DB  08fh, 080h, 045h, 045h, 045h, 049h, 049h, 049h
	DB  08eh, 08fh, 090h, 092h, 092h, 04fh, 099h, 04fh
	DB  055h, 055h, 059h, 099h, 09ah, 09bh, 09ch, 09dh
	DB  09eh, 09fh, 041h, 049h, 04fh, 055h, 0a5h, 0a5h
	DB  0a6h, 0a7h, 0a8h, 0a9h, 0aah, 0abh, 0ach, 0adh
	DB  0aeh, 0afh, 0b0h, 0b1h, 0b2h, 0b3h, 0b4h, 0b5h
	DB  0b6h, 0b7h, 0b8h, 0b9h, 0bah, 0bbh, 0bch, 0bdh
	DB  0beh, 0bfh, 0c0h, 0c1h, 0c2h, 0c3h, 0c4h, 0c5h
	DB  0c6h, 0c7h, 0c8h, 0c9h, 0cah, 0cbh, 0cch, 0cdh
	DB  0ceh, 0cfh, 0d0h, 0d1h, 0d2h, 0d3h, 0d4h, 0d5h
	DB  0d6h, 0d7h, 0d8h, 0d9h, 0dah, 0dbh, 0dch, 0ddh
	DB  0deh, 0dfh, 0e0h, 0e1h, 0e2h, 0e3h, 0e4h, 0e5h
	DB  0e6h, 0e7h, 0e8h, 0e9h, 0eah, 0ebh, 0ech, 0edh
	DB  0eeh, 0efh, 0f0h, 0f1h, 0f2h, 0f3h, 0f4h, 0f5h
	DB  0f6h, 0f7h, 0f8h, 0f9h, 0fah, 0fbh, 0fch, 0fdh
	DB  0feh, 0ffh
	GLOBAL _nlsFnameTermHardcoded
_nlsFnameTermHardcoded:
?table5:
	DB  016h, 000h, 08eh, 000h, 0ffh, 041h, 000h, 020h
	DB  0eeh, 00eh, 02eh, 022h, 02fh, 05ch, 05bh, 05dh
	DB  03ah, 07ch, 03ch, 03eh, 02bh, 03dh, 03bh, 02ch
	GLOBAL _nlsCollHardcoded
_nlsCollHardcoded:
?table6:
	DB  000h, 001h, 000h, 001h, 002h, 003h, 004h, 005h
	DB  006h, 007h, 008h, 009h, 00ah, 00bh, 00ch, 00dh
	DB  00eh, 00fh, 010h, 011h, 012h, 013h, 014h, 015h
	DB  016h, 017h, 018h, 019h, 01ah, 01bh, 01ch, 01dh
	DB  01eh, 01fh, 020h, 021h, 022h, 023h, 024h, 025h
	DB  026h, 027h, 028h, 029h, 02ah, 02bh, 02ch, 02dh
	DB  02eh, 02fh, 030h, 031h, 032h, 033h, 034h, 035h
	DB  036h, 037h, 038h, 039h, 03ah, 03bh, 03ch, 03dh
	DB  03eh, 03fh, 040h, 041h, 042h, 043h, 044h, 045h
	DB  046h, 047h, 048h, 049h, 04ah, 04bh, 04ch, 04dh
	DB  04eh, 04fh, 050h, 051h, 052h, 053h, 054h, 055h
	DB  056h, 057h, 058h, 059h, 05ah, 05bh, 05ch, 05dh
	DB  05eh, 05fh, 060h, 041h, 042h, 043h, 044h, 045h
	DB  046h, 047h, 048h, 049h, 04ah, 04bh, 04ch, 04dh
	DB  04eh, 04fh, 050h, 051h, 052h, 053h, 054h, 055h
	DB  056h, 057h, 058h, 059h, 05ah, 07bh, 07ch, 07dh
	DB  07eh, 07fh, 043h, 055h, 045h, 041h, 041h, 041h
	DB  041h, 043h, 045h, 045h, 045h, 049h, 049h, 049h
	DB  041h, 041h, 045h, 041h, 041h, 04fh, 04fh, 04fh
	DB  055h, 055h, 059h, 04fh, 055h, 024h, 024h, 024h
	DB  024h, 024h, 041h, 049h, 04fh, 055h, 04eh, 04eh
	DB  0a6h, 0a7h, 03fh, 0a9h, 0aah, 0abh, 0ach, 021h
	DB  022h, 022h, 0b0h, 0b1h, 0b2h, 0b3h, 0b4h, 0b5h
	DB  0b6h, 0b7h, 0b8h, 0b9h, 0bah, 0bbh, 0bch, 0bdh
	DB  0beh, 0bfh, 0c0h, 0c1h, 0c2h, 0c3h, 0c4h, 0c5h
	DB  0c6h, 0c7h, 0c8h, 0c9h, 0cah, 0cbh, 0cch, 0cdh
	DB  0ceh, 0cfh, 0d0h, 0d1h, 0d2h, 0d3h, 0d4h, 0d5h
	DB  0d6h, 0d7h, 0d8h, 0d9h, 0dah, 0dbh, 0dch, 0ddh
	DB  0deh, 0dfh, 0e0h, 053h, 0e2h, 0e3h, 0e4h, 0e5h
	DB  0e6h, 0e7h, 0e8h, 0e9h, 0eah, 0ebh, 0ech, 0edh
	DB  0eeh, 0efh, 0f0h, 0f1h, 0f2h, 0f3h, 0f4h, 0f5h
	DB  0f6h, 0f7h, 0f8h, 0f9h, 0fah, 0fbh, 0fch, 0fdh
	DB  0feh, 0ffh
	GLOBAL _nlsDBCSHardcoded
_nlsDBCSHardcoded:
?table7:
	DB  000h, 000h, 000h, 000h, 000h, 000h, 000h, 000h
	DB  000h, 000h
	GLOBAL _hcTablesEnd
_hcTablesEnd:
