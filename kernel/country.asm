; A rudimentary COUNTRY.SYS for FreeDOS
; Handles only Date/Time/Currency, and NOT codepage things [yet]
; Compatible with COUNTRY.SYS of MS-DOS, PC-DOS, PTS-DOS, OS/2, Win9x, WinNT
; File format described in RBIL tables 2619-2622
;
; Created as a kernel table by Tom Ehlert
; Reformatted and commented by Bernd Blaauw
; Separated from the kernel by Luchezar Georgiev
; Amended by many contributors (see names below)

; file header

db 0FFh,"COUNTRY",0,0,0,0,0,0,0,0,1,0,1 ; reserved and undocumented values
dd  ent  ; first entry
ent dw 38; number of entries - don't forget to update when adding a new country

; entries
; (size, country, codepage, reserved(2), offset)

__us dw 12,  1,437,0,0
     dd _us
__ca dw 12,  2,863,0,0
     dd _ca
__la dw 12,  3,850,0,0
     dd _la
__ru dw 12,  7,866,0,0
     dd _ru
__gr dw 12, 30,869,0,0
     dd _gr
__nl dw 12, 31,850,0,0
     dd _nl
__be dw 12, 32,850,0,0
     dd _be
__fr dw 12, 33,850,0,0
     dd _fr
__es dw 12, 34,850,0,0
     dd _es
__hu dw 12, 36,852,0,0
     dd _hu
__yu dw 12, 38,852,0,0
     dd _yu
__it dw 12, 39,850,0,0
     dd _it
__ro dw 12, 40,852,0,0
     dd _ro
__ch dw 12, 41,850,0,0
     dd _ch
__cz dw 12, 42,852,0,0
     dd _cz
__at dw 12, 43,850,0,0
     dd _at
__uk dw 12, 44,850,0,0
     dd _uk
__dk dw 12, 45,865,0,0
     dd _dk
__se dw 12, 46,850,0,0
     dd _se
__no dw 12, 47,865,0,0
     dd _no
__pl dw 12, 48,852,0,0
     dd _pl
__de dw 12, 49,850,0,0
     dd _de
__ar dw 12, 54,850,0,0
     dd _ar
__br dw 12, 55,850,0,0
     dd _br
__my dw 12, 60,437,0,0
     dd _my
__au dw 12, 61,437,0,0
     dd _au
__sg dw 12, 65,437,0,0
     dd _sg
__jp dw 12, 81,932,0,0
     dd _jp
__kr dw 12, 82,934,0,0
     dd _kr
__cn dw 12, 86,936,0,0
     dd _cn
__tr dw 12, 90,850,0,0
     dd _tr
__in dw 12, 91,437,0,0
     dd _in
__pt dw 12,351,860,0,0
     dd _pt
__fi dw 12,358,850,0,0
     dd _fi
__bg dw 12,359,855,0,0
     dd _bg
__ua dw 12,380,848,0,0
     dd _ua
__me dw 12,785,864,0,0
     dd _me
__il dw 12,972,862,0,0
     dd _il

; subfunction headers (so far only for subfunction 1, "CTYINFO")
; (count, size, id, offset)
; add ofher subfunctions after each one

_us dw 1,6,1
 dd us
_ca dw 1,6,1
 dd ca
_la dw 1,6,1
 dd la
_ru dw 1,6,1
 dd ru
_gr dw 1,6,1
 dd gr
_nl dw 1,6,1
 dd nl
_be dw 1,6,1
 dd be
_fr dw 1,6,1
 dd fr
_es dw 1,6,1
 dd sn
_hu dw 1,6,1
 dd hu
_yu dw 1,6,1
 dd yu
_it dw 1,6,1
 dd it
_ro dw 1,6,1
 dd ro
_ch dw 1,6,1
 dd sw
_cz dw 1,6,1
 dd cz
_at dw 1,6,1
 dd as
_uk dw 1,6,1
 dd uk
_dk dw 1,6,1
 dd dk
_se dw 1,6,1
 dd se
_no dw 1,6,1
 dd no
_pl dw 1,6,1
 dd pl
_de dw 1,6,1
 dd de
_ar dw 1,6,1
 dd ar
_br dw 1,6,1
 dd br
_my dw 1,6,1
 dd my
_au dw 1,6,1
 dd au
_sg dw 1,6,1
 dd sg
_jp dw 1,6,1
 dd np
_kr dw 1,6,1
 dd kr
_cn dw 1,6,1
 dd cn
_tr dw 1,6,1
 dd tr
_in dw 1,6,1
 dd ia
_pt dw 1,6,1
 dd pt
_fi dw 1,6,1
 dd fi
_bg dw 1,6,1
 dd bg
_ua dw 1,6,1
 dd ua
_me dw 1,6,1
 dd me
_il dw 1,6,1
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
;               bit 2 = set if currency symbol replaces decimal point
;               bit 1 = number of spaces between value and currency symbol
;               bit 0 = 0 if currency symbol precedes value
;                       1 if currency symbol follows value
; Currencyp   : Currency precision
; Time format : 0=12 hour format (AM/PM), 1=24 hour format (4:12 PM is 16:12)

%macro cnf 15
   db 0FFh,"CTYINFO"
   dw 22; length
dw       %1,%2,%3                                            ; ID,CP,DF
                 db %4,%5,%6,%7,%8                           ; currency
                                dw %9,%10, %11,%12           ; 1000, 0.1, DS,TS
                                              db %13,%14,%15 ; CF,Pr,TF
%endmacro;|  |  |     |             |   |    |  |  |  |   |
;        ID CP DF  currency       1000 0.1   DS TS CF Pr TF Country Contributor
;------------------------------------------------------------------------------
us cnf   1,437,MDY,"$",    0,0,0,0,",",".", "-",":",0,2,_12; United States
ca cnf   2,863,YMD,"$",    0,0,0,0," ",",", "-",":",3,2,_24; French Canada
la cnf   3,850,DMY,"$",    0,0,0,0,",",".", "/",":",0,2,_12; Latin America
ru cnf   7,866,DMY,"R","U","B",0,0," ",",", ".",":",3,2,_24; Russia      Arkady
gr cnf  30,869,DMY,"E","Y","P",0,0,".",",", "/",":",1,2,_12; Greece
nl cnf  31,850,DMY,"E","U","R",0,0,".",",", "-",":",0,2,_24; Netherlands   Bart
be cnf  32,850,DMY,"E","U","R",0,0,".",",", "/",":",0,2,_24; Belgium
fr cnf  33,850,DMY,"E","U","R",0,0," ",",", ".",":",0,2,_24; France
sn cnf  34,850,DMY,"E","U","R",0,0,".",",", "/",":",0,2,_24; Spain        Aitor
hu cnf  36,852,YMD,"F","t",  0,0,0," ",",", ".",":",3,2,_24; Hungary
yu cnf  38,852,YMD,"D","i","n",0,0,".",",", "-",":",2,2,_24; Yugoslavia
it cnf  39,850,DMY,"E","U","R",0,0,".",",", "/",".",0,2,_24; Italy
ro cnf  40,852,YMD,"L","e","i",0,0,".",",", "-",":",0,2,_24; Romania
sw cnf  41,850,DMY,"F","r",".",0,0,"'",".", ".",",",2,2,_24; Switzerland
cz cnf  42,852,YMD,"K","C","s",0,0,".",",", "-",":",2,2,_24; Czechoslovakia
as cnf  43,850,DMY,"E","U","R",0,0,".",",", ".",".",0,2,_24; Austria
uk cnf  44,850,DMY,9Ch,    0,0,0,0,",",".", "/",":",0,2,_24; United Kingdom
dk cnf  45,865,DMY,"k","r",  0,0,0,".",",", "-",".",2,2,_24; Denmark
se cnf  46,850,YMD,"K","r",  0,0,0," ",",", "-",".",3,2,_24; Sweden
no cnf  47,865,DMY,"K","r",  0,0,0,".",",", ".",":",2,2,_24; Norway
pl cnf  48,852,YMD,"Z",88h,  0,0,0,".",",", "-",":",0,2,_24; Poland      Michal
de cnf  49,850,DMY,"E","U","R",0,0,".",",", ".",".",1,2,_24; Germany        Tom
ar cnf  54,850,DMY,"$",    0,0,0,0,".",",", "/",".",0,2,_24; Argentina
br cnf  55,850,DMY,"C","r","$",0,0,".",",", "/",":",2,2,_24; Brazil
my cnf  60,437,DMY,"$",    0,0,0,0,",",".", "/",":",0,2,_12; Malaysia
au cnf  61,437,DMY,"$",    0,0,0,0,",",".", "-",":",0,2,_12; Australia
sg cnf  65,437,DMY,"$",    0,0,0,0,",",".", "/",":",0,2,_12; Singapore
np cnf  81,932,YMD,81h,8fh,  0,0,0,",",".", "-",":",0,0,_24; Japan         Yuki
kr cnf  82,934,YMD,5Ch,    0,0,0,0,",",".", ".",":",0,0,_24; Korea
cn cnf  86,936,YMD,0A3h,0A4h,0,0,0,",",".", ".",":",0,2,_12; China
tr cnf  90,850,DMY,"T","L",  0,0,0,".",",", "/",":",4,2,_24; Turkey
ia cnf  91,437,DMY,"R","s",  0,0,0,".",",", "/",":",0,2,_24; India
pt cnf 351,860,DMY,"E","U","R",0,0,".",",", "-",":",0,2,_24; Portugal
fi cnf 358,850,DMY,"E","U","R",0,0," ",",", ".",".",3,2,_24; Finland       Wolf
bg cnf 359,855,DMY,"B","G","L",0,0," ",",", ".",",",3,2,_24; Bulgaria  Luchezar
ua cnf 380,848,DMY,"U","A","H",0,0," ",",", ".",":",3,2,_24; Ukraine       Oleg
me cnf 785,864,DMY,0A4h,   0,0,0,0,".",",", "/",":",3,3,_12; Middle East
il cnf 972,862,DMY,99h,    0,0,0,0,",",".", " ",":",2,2,_24; Israel

db "FreeDOS" ; Trailing - as recommended by the Ralf Brown Interrupt List
