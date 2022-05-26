
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
	call fun18

	mov ax, 4C00h
	int 21h
%endif

fun18:
	lframe near
	lequ 16 * 3, 16tablesize
	lvar ?16tablesize, firsttable
	lvar fromparas(paras(26)), secondtable
	lenter			; push bp \ mov bp, sp \ lea sp, [bp - 50h]

	lea si, [bp + ?firsttable]
				; lea si, [bp - 30h]
	lea di, [bp + ?secondtable]
				; lea di, [bp - 50h]

	lleave			; mov sp, bp \ pop bp
	lret			; retn
