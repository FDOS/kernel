; File:
;			country.asm
; Description:
;	    The FreeDOS COUNTRY.SYS source code
;
;		     Copyleft (G) 2004
;		    The FreeDOS Project
;
; This file is part of FreeDOS.
;
; FreeDOS is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either
; version 2, or (at your option) any later version.
;
; FreeDOS is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty
; of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
; See the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with FreeDOS; see the file COPYING.  If not,
; write to the Free Software Foundation, Inc.,
; 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
; or go to http://www.gnu.org/licenses/gpl.html

; $Id$

; This COUNTRY.SYS is compatible with COUNTRY.SYS
; of MS-DOS, PC-DOS, PTS-DOS, OS/2, Win9x, WinNT
; File format described in RBIL tables 2619-2622
;
; Created as a kernel table by Tom Ehlert
; Reformatted and commented by Bernd Blaauw
; Separated from the kernel by Luchezar Georgiev
; Case/collate tables added by Eduardo Casino
; Yes/No and page 850 table by Steffen Kaiser
; Amended by many contributors (see names below)

; file header

db 0FFh,"COUNTRY",0,0,0,0,0,0,0,0,1,0,1 ; reserved and undocumented values
dd  ent	 ; first entry
ent dw 63; number of entries - don't forget to update when adding a new country

; entries
; (size, country, codepage, reserved(2), offset)

__us_437 dw 12,	 1,437,0,0
	 dd _us_437
__us_850 dw 12,	 1,850,0,0
	 dd _us_850
__us_858 dw 12,	 1,858,0,0
	 dd _us_858
__ca	 dw 12,	 2,863,0,0
	 dd _ca
__la_850 dw 12,	 3,850,0,0
	 dd _la_850
__la_858 dw 12,	 3,858,0,0
	 dd _la_858
__la_437 dw 12,	 3,437,0,0
	 dd _la_437
__ru	 dw 12,	 7,866,0,0
	 dd _ru
__gr	 dw 12, 30,869,0,0
	 dd _gr
__nl_850 dw 12, 31,850,0,0
	 dd _nl_850
__nl_858 dw 12, 31,858,0,0
	 dd _nl_858
__be_850 dw 12, 32,850,0,0
	 dd _be_850
__be_858 dw 12, 32,858,0,0
	 dd _be_858
__fr_850 dw 12, 33,850,0,0
	 dd _fr_850
__fr_858 dw 12, 33,858,0,0
	 dd _fr_858
__es_850 dw 12, 34,850,0,0
	 dd _es_850
__es_858 dw 12, 34,858,0,0
	 dd _es_858
__es_437 dw 12, 34,437,0,0
	 dd _es_437
__hu	 dw 12, 36,852,0,0
	 dd _hu
__yu	 dw 12, 38,852,0,0
	 dd _yu
__it_850 dw 12, 39,850,0,0
	 dd _it_850
__it_858 dw 12, 39,858,0,0
	 dd _it_858
__ro	 dw 12, 40,852,0,0
	 dd _ro
__ch_850 dw 12, 41,850,0,0
	 dd _ch_850
__ch_858 dw 12, 41,858,0,0
	 dd _ch_858
__cz	 dw 12, 42,852,0,0
	 dd _cz
__at_850 dw 12, 43,850,0,0
	 dd _at_850
__at_858 dw 12, 43,858,0,0
	 dd _at_858
__at_437 dw 12, 43,437,0,0
	 dd _at_437
__uk_850 dw 12, 44,850,0,0
	 dd _uk_850
__uk_858 dw 12, 44,858,0,0
	 dd _uk_858
__uk_437 dw 12, 44,437,0,0
	 dd _uk_437
__dk	 dw 12, 45,865,0,0
	 dd _dk
__se_850 dw 12, 46,850,0,0
	 dd _se_850
__se_858 dw 12, 46,858,0,0
	 dd _se_858
__no	 dw 12, 47,865,0,0
	 dd _no
__pl	 dw 12, 48,852,0,0
	 dd _pl
__de_850 dw 12, 49,850,0,0
	 dd _de_850
__de_858 dw 12, 49,858,0,0
	 dd _de_858
__de_437 dw 12, 49,858,0,0
	 dd _de_437
__ar_850 dw 12, 54,850,0,0
	 dd _ar_850
__ar_858 dw 12, 54,858,0,0
	 dd _ar_858
__ar_437 dw 12, 54,437,0,0
	 dd _ar_437
__br_850 dw 12, 55,850,0,0
	 dd _br_850
__br_858 dw 12, 55,858,0,0
	 dd _br_858
__my	 dw 12, 60,437,0,0
	 dd _my
__au_437 dw 12, 61,437,0,0
	 dd _au_437
__au_850 dw 12, 61,850,0,0
	 dd _au_850
__au_858 dw 12, 61,858,0,0
	 dd _au_858
__sg	 dw 12, 65,437,0,0
	 dd _sg
__jp	 dw 12, 81,932,0,0
	 dd _jp
__kr	 dw 12, 82,934,0,0
	 dd _kr
__cn	 dw 12, 86,936,0,0
	 dd _cn
__tr_857 dw 12, 90,857,0,0
	 dd _tr_857
__tr_850 dw 12, 90,850,0,0
	 dd _tr_850
__in	 dw 12, 91,437,0,0
	 dd _in
__pt	 dw 12,351,860,0,0
	 dd _pt
__fi_850 dw 12,358,850,0,0
	 dd _fi_850
__fi_858 dw 12,358,858,0,0
	 dd _fi_858
__bg	 dw 12,359,855,0,0
	 dd _bg
__ua	 dw 12,380,848,0,0
	 dd _ua
__me	 dw 12,785,864,0,0
	 dd _me
__il	 dw 12,972,862,0,0
	 dd _il

; subfunction headers
; (count, size, id, offset)
; add ofher subfunctions after each one

_us_437 dw 7,
	dw 6,1
	  dd us_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_us_850 dw 7,
	dw 6,1
	  dd us_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_us_858 dw 7,
	dw 6,1
	  dd us_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_ca	dw 1
	dw 6,1
	  dd ca
_la_850 dw 7,
	dw 6,1
	  dd la_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_la_858 dw 7,
	dw 6,1
	  dd la_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_la_437 dw 7,
	dw 6,1
	  dd la_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_ru	dw 1
	dw 6,1
	  dd ru
_gr	dw 1
	dw 6,1
	  dd gr
_nl_850 dw 1
	dw 6,1
	  dd nl_850
_nl_858 dw 1
	dw 6,1
	  dd nl_858
_be_850 dw 1
	dw 6,1
	  dd be_850
_be_858 dw 1
	dw 6,1
	  dd be_858
_fr_850 dw 1
	dw 6,1
	  dd fr_850
_fr_858 dw 1
	dw 6,1
	  dd fr_858
_es_850 dw 7,
	dw 6,1
	  dd es_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_es_858 dw 7,
	dw 6,1
	  dd es_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_es_437 dw 7,
	dw 6,1
	  dd es_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_hu	dw 1
	dw 6,1
	  dd hu
_yu	dw 1
	dw 6,1
	  dd yu
_it_850 dw 1
	dw 6,1
	  dd it_850
_it_858 dw 1
	dw 6,1
	  dd it_858
_ro	dw 1
	dw 6,1
	  dd ro
_ch_850 dw 1
	dw 6,1
	  dd ch_850
_ch_858 dw 1
	dw 6,1
	  dd ch_858
_cz	dw 1
	dw 6,1
	  dd cz
_at_850 dw 7,
	dw 6,1
	  dd at_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd de_yn
_at_858 dw 7,
	dw 6,1
	  dd at_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd de_yn
_at_437 dw 7,
	dw 6,1
	  dd at_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd de_yn
_uk_850 dw 7,
	dw 6,1
	  dd uk_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_uk_858 dw 7,
	dw 6,1
	  dd uk_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_uk_437 dw 7,
	dw 6,1
	  dd uk_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_dk	dw 1
	dw 6,1
	  dd dk
_se_850 dw 1
	dw 6,1
	  dd se_850
_se_858 dw 1
	dw 6,1
	  dd se_858
_no	dw 1
	dw 6,1
	  dd no
_pl	dw 1
	dw 6,1
	  dd pl
_de_850 dw 7,
	dw 6,1
	  dd de_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd de_yn
_de_858 dw 7,
	dw 6,1
	  dd de_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd de_yn
_de_437 dw 7,
	dw 6,1
	  dd de_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd de_yn
_ar_437 dw 7,
	dw 6,1
	  dd ar_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_ar_850 dw 7,
	dw 6,1
	  dd ar_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_ar_858 dw 7,
	dw 6,1
	  dd ar_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd es_yn
_br_850 dw 1
	dw 6,1
	  dd br_850
_br_858 dw 1
	dw 6,1
	  dd br_858
_my	dw 1
	dw 6,1
	  dd my
_au_437 dw 7,
	dw 6,1
	  dd au_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_au_850 dw 7,
	dw 6,1
	  dd au_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_au_858 dw 7,
	dw 6,1
	  dd au_858
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,23
	  dd en_yn
_sg	dw 1
	dw 6,1
	  dd sg
_jp	dw 1
	dw 6,1
	  dd np
_kr	dw 1
	dw 6,1
	  dd kr
_cn	dw 1
	dw 6,1
	  dd cn
_tr_857 dw 1
	dw 6,1
	  dd tr_857
_tr_850 dw 1
	dw 6,1
	  dd tr_850
_in	dw 1
	dw 6,1
	  dd ia
_pt	dw 1
	dw 6,1
	  dd pt
_fi_850 dw 1
	dw 6,1
	  dd fi_850
_fi_858 dw 1
	dw 6,1
	  dd fi_858
_bg	dw 1
	dw 6,1
	  dd bg
_ua	dw 1
	dw 6,1
	  dd ua
_me	dw 1
	dw 6,1
	  dd me
_il	dw 1
	dw 6,1
	  dd il

%define MDY 0 ; month/day/year
%define DMY 1 ; day/month/year
%define YMD 2 ; year/month/day

%define _12 0 ; time as AM/PM
%define _24 1 ; 24-hour format

; Country ID  : international numbering
; Codepage    : codepage to use by default
; Date format : M = Month, D = Day, Y = Year (4digit); 0=USA, 1=Europe, 2=Japan
; Currency    : $ = dollar, EUR = EURO (ALT-128), UK uses the pound sign
; Thousands   : separator for 1000s (1,000,000 bytes; Dutch: 1.000.000 bytes)
; Decimals    : separator for decimals (2.5 KB; Dutch: 2,5 KB)
; Datesep     : Date separator (2/4/2004 or 2-4-2004 for example)
; Timesep     : usually ":" is used to separate hours, minutes and seconds
; Currencyf   : Currency format (bit array)
;		bit 2 = set if currency symbol replaces decimal point
;		bit 1 = number of spaces between value and currency symbol
;		bit 0 = 0 if currency symbol precedes value
;			1 if currency symbol follows value
; Currencyp   : Currency precision
; Time format : 0=12 hour format (AM/PM), 1=24 hour format (4:12 PM is 16:12)

%macro cnf 15
   db 0FFh,"CTYINFO"
   dw 22; length
dw	     %1,%2,%3					     ; ID,CP,DF
		     db %4,%5,%6,%7,%8			     ; currency
				    dw %9,%10,%11,%12	     ; 1000, 0.1, DS,TS
						 db %13,%14,%15 ; CF,Pr,TF
%endmacro;    |	 |  |	  |		|   |	|  |  |	 |   |
;	     ID CP DF  currency	      1000 0.1	DS TS CF Pr TF Country Contrib
;-----------------------------------------------------------------------------
us_437 cnf   1,437,MDY,"$",    0,0,0,0,",",".","-",":",0,2,_12; United States
us_850 cnf   1,850,MDY,"$",    0,0,0,0,",",".","-",":",0,2,_12; United States
us_858 cnf   1,858,MDY,"$",    0,0,0,0,",",".","-",":",0,2,_12; United States
ca     cnf   2,863,YMD,"$",    0,0,0,0," ",",","-",":",3,2,_24; French Canada
la_850 cnf   3,850,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Latin America
la_858 cnf   3,858,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Latin America
la_437 cnf   3,437,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Latin America
ru     cnf   7,866,DMY,"R","U","B",0,0," ",",",".",":",3,2,_24; Russia	 Arkady
gr     cnf  30,869,DMY,"E","Y","P",0,0,".",",","/",":",1,2,_12; Greece
nl_850 cnf  31,850,DMY,"E","U","R",0,0,".",",","-",":",0,2,_24; Netherlands Bart
nl_858 cnf  31,858,DMY,0D5h,   0,0,0,0,".",",","-",":",0,2,_24; Netherlands
be_850 cnf  32,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Belgium
be_858 cnf  32,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24; Belgium
fr_850 cnf  33,850,DMY,"E","U","R",0,0," ",",",".",":",0,2,_24; France
fr_858 cnf  33,858,DMY,0D5h,   0,0,0,0," ",",",".",":",0,2,_24; France
es_850 cnf  34,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Spain	  Aitor
es_858 cnf  34,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24; Spain
es_437 cnf  34,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Spain
hu     cnf  36,852,YMD,"F","t",	 0,0,0," ",",",".",":",3,2,_24; Hungary
yu     cnf  38,852,YMD,"D","i","n",0,0,".",",","-",":",2,2,_24; Yugoslavia
it_850 cnf  39,850,DMY,"E","U","R",0,0,".",",","/",".",0,2,_24; Italy
it_858 cnf  39,858,DMY,0D5h,   0,0,0,0,".",",","/",".",0,2,_24; Italy
ro     cnf  40,852,YMD,"L","e","i",0,0,".",",","-",":",0,2,_24; Romania
ch_850 cnf  41,850,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24; Switzerland
ch_858 cnf  41,858,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24; Switzerland
cz     cnf  42,852,YMD,"K","C","s",0,0,".",",","-",":",2,2,_24; Czechoslovakia
at_850 cnf  43,850,DMY,"E","U","R",0,0,".",",",".",".",0,2,_24; Austria
at_858 cnf  43,858,DMY,0D5h,   0,0,0,0,".",",",".",".",0,2,_24; Austria
at_437 cnf  43,437,DMY,"E","U","R",0,0,".",",",".",".",0,2,_24; Austria
uk_850 cnf  44,850,DMY,9Ch,    0,0,0,0,",",".","/",":",0,2,_24; United Kingdom
uk_858 cnf  44,858,DMY,9Ch,    0,0,0,0,",",".","/",":",0,2,_24; United Kingdom
uk_437 cnf  44,437,DMY,9Ch,    0,0,0,0,",",".","/",":",0,2,_24; United Kingdom
dk     cnf  45,865,DMY,"k","r",	 0,0,0,".",",","-",".",2,2,_24; Denmark
se_850 cnf  46,850,YMD,"K","r",	 0,0,0," ",",","-",".",3,2,_24; Sweden
se_858 cnf  46,858,YMD,"K","r",	 0,0,0," ",",","-",".",3,2,_24; Sweden
no     cnf  47,865,DMY,"K","r",	 0,0,0,".",",",".",":",2,2,_24; Norway
pl     cnf  48,852,YMD,"Z",88h,	 0,0,0,".",",","-",":",0,2,_24; Poland	 Michal
de_850 cnf  49,850,DMY,"E","U","R",0,0,".",",",".",".",1,2,_24; Germany	    Tom
de_858 cnf  49,850,DMY,0D5h,   0,0,0,0,".",",",".",".",1,2,_24; Germany
de_437 cnf  49,437,DMY,"E","U","R",0,0,".",",",".",".",1,2,_24; Germany
ar_850 cnf  54,850,DMY,"$",    0,0,0,0,".",",","/",".",0,2,_24; Argentina
ar_858 cnf  54,858,DMY,"$",    0,0,0,0,".",",","/",".",0,2,_24; Argentina
ar_437 cnf  54,437,DMY,"$",    0,0,0,0,".",",","/",".",0,2,_24; Argentina
br_850 cnf  55,850,DMY,"C","r","$",0,0,".",",","/",":",2,2,_24; Brazil
br_858 cnf  55,858,DMY,"C","r","$",0,0,".",",","/",":",2,2,_24; Brazil
my     cnf  60,437,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Malaysia
au_437 cnf  61,437,DMY,"$",    0,0,0,0,",",".","-",":",0,2,_12; Australia
au_850 cnf  61,850,DMY,"$",    0,0,0,0,",",".","-",":",0,2,_12; Australia
au_858 cnf  61,858,DMY,"$",    0,0,0,0,",",".","-",":",0,2,_12; Australia
sg     cnf  65,437,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Singapore
np     cnf  81,932,YMD,81h,8fh,	 0,0,0,",",".","-",":",0,0,_24; Japan	   Yuki
kr     cnf  82,934,YMD,5Ch,    0,0,0,0,",",".",".",":",0,0,_24; Korea
cn     cnf  86,936,YMD,0A3h,0A4h,0,0,0,",",".",".",":",0,2,_12; China
tr_857 cnf  90,857,DMY,"T","L",	 0,0,0,".",",","/",":",4,2,_24; Turkey
tr_850 cnf  90,850,DMY,"T","L",	 0,0,0,".",",","/",":",4,2,_24; Turkey
ia     cnf  91,437,DMY,"R","s",	 0,0,0,".",",","/",":",0,2,_24; India
pt     cnf 351,860,DMY,"E","U","R",0,0,".",",","-",":",0,2,_24; Portugal
fi_850 cnf 358,850,DMY,"E","U","R",0,0," ",",",".",".",3,2,_24; Finland	   Wolf
fi_858 cnf 358,858,DMY,0D5h,   0,0,0,0," ",",",".",".",3,2,_24; Finland
bg     cnf 359,855,DMY,"B","G","L",0,0," ",",",".",",",3,2,_24; Bulgaria  Lucho
ua     cnf 380,848,DMY,"U","A","H",0,0," ",",",".",":",3,2,_24; Ukraine	   Oleg
me     cnf 785,864,DMY,0A4h,   0,0,0,0,".",",","/",":",3,3,_12; Middle East
il     cnf 972,862,DMY,99h,    0,0,0,0,",","."," ",":",2,2,_24; Israel

; Uppercase equivalents of chars 80h to FFh
;------------------------------------------------------------------------------
ucase_437 db 0FFh,"UCASE  "	; Same as kernel's harcoded
	  dw 128
db 128, 154,  69,  65, 142,  65, 143, 128
db  69,	 69,  69,  73,	73,  73, 142, 143
db 144, 146, 146,  79, 153,  79,  85,  85
db  89, 153, 154, 155, 156, 157, 158, 159
db  65,	 73,  79,  85, 165, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

ucase_850 db 0FFh,"UCASE  "	; From Steffen Kaiser's UNF package
	  dw 128
db 128, 154, 144, 182, 142, 183, 143, 128
db 210, 211, 212, 216, 215, 222, 142, 143
db 144, 146, 146, 226, 153, 227, 234, 235
db  95, 153, 154, 157, 156, 157, 158, 159
db 181, 214, 224, 233, 165, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 199, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 209, 209, 210, 211, 212,  73, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 229, 229, 230, 232
db 232, 233, 234, 235, 237, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

; Filename terminator table
;------------------------------------------------------------------------------
fchar db 0FFh,"FCHAR  "		; Same as kernel's hardcoded
      dw 22			; Comments from RBIL
db 142	  ; ??? (01h for MS-DOS 3.30-6.00)
db   0	  ; lowest permissible character value for filename
db 255	  ; highest permissible character value for filename
db  65	  ; ??? (00h for MS-DOS 3.30-6.00)
db   0	  ; first excluded character in range \ all characters in this
db  32	  ; last excluded character in range  / range are illegal
db 238	  ; ??? (02h for MS-DOS 3.30-6.00)
db  14	  ; number of illegal (terminator) characters
; characters which terminate a filename:
db  46,	 34,  47,  92,	91,  93,  58, 124 ; ."/\[]:|
db  60,	 62,  43,  61,	59,  44		  ; <>+=;,

; Collating sequence
;------------------------------------------------------------------------------
en_collate_437 db 0FFh,"COLLATE"		; English, CP437
	       dw 256				; Same as kernel's harcoded
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90,  91,	92,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db  67,	 85,  69,  65,	65,  65,  65,  67
db  69,	 69,  69,  73,	73,  73,  65,  65
db  69,	 65,  65,  79,	79,  79,  85,  85
db  89,	 79,  85,  36,	36,  36,  36,  36
db  65,	 73,  79,  85,	78,  78, 166, 167
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224,	 83, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

en_collate_850 db 0FFh,"COLLATE"		; English, CP850
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90,  91,	92,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db  67,	 85,  69,  65,	65,  65,  65,  67
db  69,	 69,  69,  73,	73,  73,  65,  65
db  69,	 65,  65,  79,	79,  79,  85,  85
db  89,	 79,  85,  36,	36,  36,  36,  36
db  65,	 73,  79,  85,	78,  78, 166, 167
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,	 68,  69,  69,	69,  73,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,	 83,  79,  79,	79,  79, 230, 231
db 232,	 85,  85,  85,	89,  89, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,	51,  50, 254, 255

en_collate_858 db 0FFh,"COLLATE"		; English, CP858
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90,  91,	92,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db  67,	 85,  69,  65,	65,  65,  65,  67
db  69,	 69,  69,  73,	73,  73,  65,  65
db  69,	 65,  65,  79,	79,  79,  85,  85
db  89,	 79,  85,  36,	36,  36,  36,  36
db  65,	 73,  79,  85,	78,  78, 166, 167
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,	 68,  69,  69,	69,  36,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,	 83,  79,  79,	79,  79, 230, 231
db 232,	 85,  85,  85,	89,  89, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,	51,  50, 254, 255

es_collate_437 db 0FFh,"COLLATE"		; Spanish, CP437
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	69,  70,  71,  72
db  73,	 74,  75,  76,	77,  78,  79,  81
db  82,	 83,  84,  85,	87,  88,  89,  90
db  91,	 92,  93,  94,	95,  96,  97,  98
db  99,	 65,  66,  67,	69,  70,  71,  72
db  73,	 74,  75,  76,	77,  78,  79,  81
db  82,	 83,  84,  85,	87,  88,  89,  90
db  91,	 92,  93, 123, 124, 125, 126, 127
db  68,	 88,  70,  65,	65,  65,  65,  68
db  70,	 70,  70,  74,	74,  74,  65,  65
db  70,	 65,  65,  81,	81,  81,  88,  88
db  92,	 81,  88,  36,	36,  36,  36,  36
db  65,	 74,  81,  88,	80,  80,  65,  81
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224,	 86, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251,	79,  50, 254, 255

es_collate_850 db 0FFh,"COLLATE"		; Spanish, CP850
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	69,  70,  71,  72
db  73,	 74,  75,  76,	77,  78,  79,  81
db  82,	 83,  84,  85,	87,  88,  89,  90
db  91,	 92,  93,  94,	95,  96,  97,  98
db  99,	 65,  66,  67,	69,  70,  71,  72
db  73,	 74,  75,  76,	77,  78,  79,  81
db  82,	 83,  84,  85,	87,  88,  89,  90
db  91,	 92,  93, 123, 124, 125, 126, 127
db  68,	 87,  70,  65,	65,  65,  65,  68
db  70,	 70,  70,  74,	74,  74,  65,  65
db  70,	 65,  65,  81,	81,  81,  88,  88
db  92,	 81,  88,  81,	36,  81, 158,  36
db  65,	 74,  81,  88,	80,  80,  65,  81
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  69,	 69,  70,  70,	70,  74,  74,  74
db  74, 217, 218, 219, 220, 221,  74, 223
db  81,	 86,  81,  81,	81,  81, 230, 231
db 232,	 88,  88,  88,	92,  92, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,	51,  50, 254, 255

es_collate_858 db 0FFh,"COLLATE"		; Spanish, CP858
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	69,  70,  71,  72
db  73,	 74,  75,  76,	77,  78,  79,  81
db  82,	 83,  84,  85,	87,  88,  89,  90
db  91,	 92,  93,  94,	95,  96,  97,  98
db  99,	 65,  66,  67,	69,  70,  71,  72
db  73,	 74,  75,  76,	77,  78,  79,  81
db  82,	 83,  84,  85,	87,  88,  89,  90
db  91,	 92,  93, 123, 124, 125, 126, 127
db  68,	 87,  70,  65,	65,  65,  65,  68
db  70,	 70,  70,  74,	74,  74,  65,  65
db  70,	 65,  65,  81,	81,  81,  88,  88
db  92,	 81,  88,  81,	36,  81, 158,  36
db  65,	 74,  81,  88,	80,  80,  65,  81
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  69,	 69,  70,  70,	70,  36,  74,  74
db  74, 217, 218, 219, 220, 221,  74, 223
db  81,	 86,  81,  81,	81,  81, 230, 231
db 232,	 88,  88,  88,	92,  92, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,	51,  50, 254, 255

de_collate_850 db 0FFh,"COLLATE"	; German CP850
	       dw 256			; From Steffen Kaiser's UNF package
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90,  91,	92,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db  67,	 85,  69,  65,	65,  65,  65,  67
db  69,	 69,  69,  73,	73,  73,  65,  65
db  69,	 65,  65,  79,	79,  79,  85,  85
db  89,	 79,  85,  79,	36,  79, 158,  36
db  65,	 73,  79,  85,	78,  78, 166, 167
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,	 68,  69,  69,	69,  73,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,	 83,  79,  79,	79,  79, 230, 232
db 232,	 85,  85,  85,	89,  89, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

de_collate_858 db 0FFh,"COLLATE"	; German CP858
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90,  91,	92,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db  67,	 85,  69,  65,	65,  65,  65,  67
db  69,	 69,  69,  73,	73,  73,  65,  65
db  69,	 65,  65,  79,	79,  79,  85,  85
db  89,	 79,  85,  79,	36,  79, 158,  36
db  65,	 73,  79,  85,	78,  78, 166, 167
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,	 68,  69,  69,	69,  36,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,	 83,  79,  79,	79,  79, 230, 232
db 232,	 85,  85,  85,	89,  89, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

de_collate_437 db 0FFh,"COLLATE"	; German CP437
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90,  91,	92,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db  67,	 85,  69,  65,	65,  65,  65,  67
db  69,	 69,  69,  73,	73,  73,  65,  65
db  69,	 65,  65,  79,	79,  79,  85,  85
db  89,	 79,  85,  36,	36,  36,  36,  36
db  65,	 73,  79,  85,	78,  78, 166, 167
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224,	 83, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

; Dual Byte Character Sets
;   lead-byte ranges
;------------------------------------------------------------------------------
dbcs_empty db 0FFh,"DBCS   "
      dw 0			; Table length
      db 0, 0			; Table terminator (even if length == 0)

; Yes/No table
; yes_l : Character (single byte) or leadbyte (DBCS) for YES
; yes_h : trailbyte for YES
; no_l	: Character (single byte) or leadbyte (DBCS) for NO
; no_h	: trailbyte for NO
;------------------------------------------------------------------------------
es_yn db 0FFh,"YESNO  "
      dw 4
      db  'S',0,'N',0 ; Spanish

de_yn db 0FFh,"YESNO  "
      dw 4
      db  'J',0,'N',0 ; German

en_yn db 0FFh,"YESNO  "
      dw 4
      db  'Y',0,'N',0 ; English

db "FreeDOS" ; Trailing - as recommended by the Ralf Brown Interrupt List
