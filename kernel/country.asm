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

; $Id: country.asm 1637 2011-06-27 01:05:22Z perditionc $

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
ent dw 171; number of entries - don't forget to update when adding a new country

; entries
; (size, country, codepage, reserved(2), offset)

; Countries 0 - 999 (Standard)
;
__us_437 dw 12,	 1,437,0,0
	 dd _us_437
__us_850 dw 12,	 1,850,0,0
	 dd _us_850
__us_858 dw 12,	 1,858,0,0
	 dd _us_858
__ca_863 dw 12,	 2,863,0,0
	 dd _ca_863
__ca_850 dw 12,	 2,850,0,0
	 dd _ca_850
__ca_858 dw 12,	 2,858,0,0
	 dd _ca_858
__la_858 dw 12,	 3,858,0,0
	 dd _la_858
__la_850 dw 12,	 3,850,0,0
	 dd _la_850
__la_437 dw 12,	 3,437,0,0
	 dd _la_437
__ru_866 dw 12,	 7,866,0,0
	 dd _ru_866
__ru_808 dw 12,	 7,808,0,0
	 dd _ru_808
__ru_855 dw 12,	 7,855,0,0
	 dd _ru_855
__ru_872 dw 12,	 7,872,0,0
	 dd _ru_872
__ru_852 dw 12,	 7,852,0,0
	 dd _ru_852
__ru_850 dw 12,	 7,850,0,0
	 dd _ru_850
__ru_858 dw 12,	 7,858,0,0
	 dd _ru_858
__ru_437 dw 12,	 7,437,0,0
	 dd _ru_437
__gr_869 dw 12, 30,869,0,0
	 dd _gr_869
__gr_737 dw 12, 30,737,0,0
	 dd _gr_737
__gr_850 dw 12, 30,850,0,0
	 dd _gr_850
__gr_858 dw 12, 30,858,0,0
	 dd _gr_858
__nl_858 dw 12, 31,858,0,0
	 dd _nl_858
__nl_850 dw 12, 31,850,0,0
	 dd _nl_850
__nl_437 dw 12, 31,437,0,0
	 dd _nl_437
__be_858 dw 12, 32,858,0,0
	 dd _be_858
__be_850 dw 12, 32,850,0,0
	 dd _be_850
__be_437 dw 12, 32,437,0,0
	 dd _be_437
__fr_858 dw 12, 33,858,0,0
	 dd _fr_858
__fr_850 dw 12, 33,850,0,0
	 dd _fr_850
__fr_437 dw 12, 33,437,0,0
	 dd _fr_437
__es_858 dw 12, 34,858,0,0
	 dd _es_858
__es_850 dw 12, 34,850,0,0
	 dd _es_850
__es_437 dw 12, 34,437,0,0
	 dd _es_437
__hu_852 dw 12, 36,852,0,0
	 dd _hu_852
__hu_850 dw 12, 36,850,0,0
	 dd _hu_850
__hu_858 dw 12, 36,858,0,0
	 dd _hu_858
__yu_852 dw 12, 38,852,0,0
	 dd _yu_852
__yu_855 dw 12, 38,855,0,0
	 dd _yu_855
__yu_872 dw 12, 38,872,0,0
	 dd _yu_872
__yu_858 dw 12, 38,858,0,0
	 dd _yu_858
__yu_850 dw 12, 38,850,0,0
	 dd _yu_850
__it_858 dw 12, 39,858,0,0
	 dd _it_858
__it_850 dw 12, 39,850,0,0
	 dd _it_850
__it_437 dw 12, 39,437,0,0
	 dd _it_437
__ro_852 dw 12, 40,852,0,0
	 dd _ro_852
__ro_850 dw 12, 40,850,0,0
	 dd _ro_850
__ro_858 dw 12, 40,858,0,0
	 dd _ro_858
__ch_858 dw 12, 41,858,0,0
	 dd _ch_858
__ch_850 dw 12, 41,850,0,0
	 dd _ch_850
__ch_437 dw 12, 41,437,0,0
	 dd _ch_437
__cz_852 dw 12, 42,852,0,0
	 dd _cz_852
__cz_850 dw 12, 42,850,0,0
	 dd _cz_850
__cz_858 dw 12, 42,858,0,0
	 dd _cz_858
__at_858 dw 12, 43,858,0,0
	 dd _at_858
__at_850 dw 12, 43,850,0,0
	 dd _at_850
__at_437 dw 12, 43,437,0,0
	 dd _at_437
__uk_858 dw 12, 44,858,0,0
	 dd _uk_858
__uk_850 dw 12, 44,850,0,0
	 dd _uk_850
__uk_437 dw 12, 44,437,0,0
	 dd _uk_437
__dk_865 dw 12, 45,865,0,0
	 dd _dk_865
__dk_850 dw 12, 45,850,0,0
	 dd _dk_850
__dk_858 dw 12, 45,858,0,0
	 dd _dk_858
__se_858 dw 12, 46,858,0,0
	 dd _se_858
__se_850 dw 12, 46,850,0,0
	 dd _se_850
__se_437 dw 12, 46,437,0,0
	 dd _se_437
__no_865 dw 12, 47,865,0,0
	 dd _no_865
__no_850 dw 12, 47,850,0,0
	 dd _no_850
__no_858 dw 12, 47,858,0,0
	 dd _no_858
__pl_852 dw 12, 48,852,0,0
	 dd _pl_852
__pl_850 dw 12, 48,850,0,0
	 dd _pl_850
__pl_858 dw 12, 48,858,0,0
	 dd _pl_858
__de_858 dw 12, 49,858,0,0
	 dd _de_858
__de_850 dw 12, 49,850,0,0
	 dd _de_850
__de_437 dw 12, 49,858,0,0
	 dd _de_437
__ar_858 dw 12, 54,858,0,0
	 dd _ar_858
__ar_850 dw 12, 54,850,0,0
	 dd _ar_850
__ar_437 dw 12, 54,437,0,0
	 dd _ar_437
__br_858 dw 12, 55,858,0,0
	 dd _br_858
__br_850 dw 12, 55,850,0,0
	 dd _br_850
__br_437 dw 12, 55,437,0,0
	 dd _br_437
__my_437 dw 12, 60,437,0,0
	 dd _my_437
__au_437 dw 12, 61,437,0,0
	 dd _au_437
__au_850 dw 12, 61,850,0,0
	 dd _au_850
__au_858 dw 12, 61,858,0,0
	 dd _au_858
__sg_437 dw 12, 65,437,0,0
	 dd _sg_437
__jp_437 dw 12, 81,437,0,0
	 dd _jp_437
__jp_932 dw 12, 81,932,0,0
	 dd _jp_932
__kr_437 dw 12, 82,437,0,0
	 dd _kr_437
__kr_934 dw 12, 82,934,0,0
	 dd _kr_934
__cn_437 dw 12, 86,437,0,0
	 dd _cn_437
__cn_936 dw 12, 86,936,0,0
	 dd _cn_936
__tr_857 dw 12, 90,857,0,0
	 dd _tr_857
__tr_850 dw 12, 90,850,0,0
	 dd _tr_850
__in_437 dw 12, 91,437,0,0
	 dd _in_437
__pt_860 dw 12,351,860,0,0
	 dd _pt_860
__pt_850 dw 12,351,850,0,0
	 dd _pt_850
__pt_858 dw 12,351,858,0,0
	 dd _pt_858
__fi_858 dw 12,358,858,0,0
	 dd _fi_858
__fi_850 dw 12,358,850,0,0
	 dd _fi_850
__fi_437 dw 12,358,437,0,0
	 dd _fi_437
__bg_855 dw 12,359,855,0,0
	 dd _bg_855
__bg_872 dw 12,359,872,0,0
	 dd _bg_872
__bg_850 dw 12,359,850,0,0
	 dd _bg_850
__bg_858 dw 12,359,858,0,0
	 dd _bg_858
__bg_866 dw 12,359,866,0,0
	 dd _bg_866
__bg_808 dw 12,359,808,0,0
	 dd _bg_808
__bg_849 dw 12,359,849,0,0
	 dd _bg_849
__bg_1131 dw 12,359,1131,0,0
	 dd _bg_1131
__bg_30033 dw 12,359,30033,0,0
	 dd _bg_30033
__by_849 dw 12,375,849,0,0
	 dd _by_849
__by_1131 dw 12,375,1131,0,0
	 dd _by_1131
__by_850 dw 12,375,850,0,0
	 dd _by_850
__by_858 dw 12,375,858,0,0
	 dd _by_858
__ua_848 dw 12,380,848,0,0
	 dd _ua_848
__ua_1125 dw 12,380,1125,0,0
	 dd _ua_1125
__sr_855 dw 12, 381,855,0,0 ; Serbia and Montenegro, Serbian, Cyrillic
	 dd _sr_855
__sr_872 dw 12, 381,872,0,0
	 dd _sr_872
__sr_852 dw 12, 381,852,0,0 ; Serbia and Montenegro, Serbian, Latin
	 dd _sr_852
__sr_850 dw 12, 381,850,0,0
	 dd _sr_850
__sr_858 dw 12, 381,858,0,0
	 dd _sr_858
__hr_852 dw 12, 384,852,0,0 ; Croatia, Croatian
	 dd _hr_852
__hr_850 dw 12, 384,850,0,0
	 dd _hr_850
__hr_858 dw 12, 384,858,0,0
	 dd _hr_858
__si_852 dw 12, 386,852,0,0 ; Slovenia
	 dd _si_852
__si_850 dw 12, 386,850,0,0
	 dd _si_850
__si_858 dw 12, 386,858,0,0
	 dd _si_858
__ba_852 dw 12, 387,852,0,0 ; Bosnia Herzegovina
	 dd _ba_852
__ba_850 dw 12, 387,850,0,0
	 dd _ba_850
__ba_858 dw 12, 387,858,0,0
	 dd _ba_858
__ba_855 dw 12, 387,855,0,0 ; Bosnia Herzegovina, Cyrillic
	 dd _sr_855
__ba_872 dw 12, 387,872,0,0
	 dd _sr_872
__mk_855 dw 12, 389,855,0,0 ; Macedonia
	 dd _mk_855
__mk_872 dw 12, 389,872,0,0
	 dd _mk_872
__mk_850 dw 12, 389,850,0,0
	 dd _mk_850
__mk_858 dw 12, 389,858,0,0
	 dd _mk_858
__me_858 dw 12,785,858,0,0
	 dd _me_858
__me_850 dw 12,785,850,0,0
	 dd _me_850
__me_864 dw 12,785,864,0,0
	 dd _me_864
__il_858 dw 12,972,858,0,0
	 dd _il_858
__il_850 dw 12,972,850,0,0
	 dd _il_850
__il_862 dw 12,972,862,0,0
	 dd _il_862

; Countries 4x000 - 4x999 (Multilingual)
;
__nl_BE_850 dw 12, 40032,850,0,0
	 dd _nl_BE_850
__nl_BE_858 dw 12, 40032,858,0,0
	 dd _nl_BE_858
__nl_BE_437 dw 12, 40032,437,0,0
	 dd _nl_BE_437
__fr_BE_850 dw 12, 41032,850,0,0
	 dd _fr_BE_850
__fr_BE_858 dw 12, 41032,858,0,0
	 dd _fr_BE_858
__fr_BE_437 dw 12, 41032,437,0,0
	 dd _fr_BE_437
__de_BE_850 dw 12, 42032,850,0,0
	 dd _de_BE_850
__de_BE_858 dw 12, 42032,858,0,0
	 dd _de_BE_858
__de_BE_437 dw 12, 42032,437,0,0
	 dd _de_BE_437
__es_ES_850 dw 12, 40034,850,0,0
	 dd _es_ES_850
__es_ES_858 dw 12, 40034,858,0,0
	 dd _es_ES_858
__es_ES_437 dw 12, 40034,437,0,0
	 dd _es_ES_437
__ca_ES_850 dw 12, 41034,850,0,0
	 dd _ca_ES_850
__ca_ES_858 dw 12, 41034,858,0,0
	 dd _ca_ES_858
__ca_ES_437 dw 12, 41034,437,0,0
	 dd _ca_ES_437
__gl_ES_850 dw 12, 42034,850,0,0
	 dd _gl_ES_850
__gl_ES_858 dw 12, 42034,858,0,0
	 dd _gl_ES_858
__gl_ES_437 dw 12, 42034,437,0,0
	 dd _gl_ES_437
__eu_ES_850 dw 12, 43034,850,0,0
	 dd _eu_ES_850
__eu_ES_858 dw 12, 43034,858,0,0
	 dd _eu_ES_858
__eu_ES_437 dw 12, 43034,437,0,0
	 dd _eu_ES_437
__de_CH_858 dw 12, 40041,858,0,0
	 dd _de_CH_858
__de_CH_850 dw 12, 40041,850,0,0
	 dd _de_CH_850
__de_CH_437 dw 12, 40041,437,0,0
	 dd _de_CH_437
__fr_CH_858 dw 12, 41041,858,0,0
	 dd _fr_CH_858
__fr_CH_850 dw 12, 41041,850,0,0
	 dd _fr_CH_850
__fr_CH_437 dw 12, 41041,437,0,0
	 dd _fr_CH_437
__it_CH_858 dw 12, 42041,858,0,0
	 dd _it_CH_858
__it_CH_850 dw 12, 42041,850,0,0
	 dd _it_CH_850
__it_CH_437 dw 12, 42041,437,0,0
	 dd _it_CH_437

; subfunction headers
; (count, size, id, offset)
; add ofher subfunctions after each one

_us_437 dw 7
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
	dw 6,35
	  dd en_yn
_us_850 dw 7
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
	dw 6,35
	  dd en_yn
_us_858 dw 7
	dw 6,1
	  dd us_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd en_yn
_ca_863 dw 7
	dw 6,1
	  dd ca_863
	dw 6,2
	  dd ucase_863
	dw 6,4
	  dd ucase_863
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_863
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_ca_850 dw 7
	dw 6,1
	  dd ca_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_ca_858 dw 7
	dw 6,1
	  dd ca_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_la_850 dw 7
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
	dw 6,35
	  dd es_yn
_la_858 dw 7
	dw 6,1
	  dd la_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd es_yn
_la_437 dw 7
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
	dw 6,35
	  dd es_yn
_ru_866 dw 8
	dw 6,1
	  dd ru_866
	dw 6,2
	  dd ucase_866
	dw 6,3
	  dd lcase_866
	dw 6,4
	  dd ucase_866
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_866
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn_866
_ru_808 dw 8
	dw 6,1
	  dd ru_808
	dw 6,2
	  dd ucase_808
	dw 6,3
	  dd lcase_808
	dw 6,4
	  dd ucase_808
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_808
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn_808
_ru_855 dw 7
	dw 6,1
	  dd ru_855
	dw 6,2
	  dd ucase_855
	dw 6,4
	  dd ucase_855
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_855
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn_855
_ru_872 dw 7
	dw 6,1
	  dd ru_872
	dw 6,2
	  dd ucase_872
	dw 6,4
	  dd ucase_872
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_872
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn_872
_ru_852 dw 7
	dw 6,1
	  dd ru_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn
_ru_850 dw 7
	dw 6,1
	  dd ru_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn
_ru_858 dw 7
	dw 6,1
	  dd ru_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn
_ru_437 dw 7
	dw 6,1
	  dd ru_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ru_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ru_yn
_gr_869	dw 7
	dw 6,1
	  dd gr_869
	dw 6,2
	  dd ucase_869
	dw 6,4
	  dd ucase_869
	dw 6,5
	  dd fchar
	dw 6,6
	  dd gr_collate_869
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd gr_yn_869
_gr_737	dw 7
	dw 6,1
	  dd gr_737
	dw 6,2
	  dd ucase_737
	dw 6,4
	  dd ucase_737
	dw 6,5
	  dd fchar
	dw 6,6
	  dd gr_collate_737
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd gr_yn_737
_gr_850	dw 7
	dw 6,1
	  dd gr_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd gr_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd gr_yn
_gr_858	dw 7
	dw 6,1
	  dd gr_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd gr_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd gr_yn
_nl_850 dw 7
	dw 6,1
	  dd nl_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd nl_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_nl_858 dw 7
	dw 6,1
	  dd nl_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd nl_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_nl_437 dw 7
	dw 6,1
	  dd nl_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd nl_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_be_850 dw 7
	dw 6,1
	  dd be_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd be_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_be_858 dw 7
	dw 6,1
	  dd be_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd be_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_be_437 dw 7
	dw 6,1
	  dd be_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd be_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_fr_850 dw 7
	dw 6,1
	  dd fr_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_fr_858 dw 7
	dw 6,1
	  dd fr_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_fr_437 dw 7
	dw 6,1
	  dd fr_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_es_850 dw 7
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
	dw 6,35
	  dd es_yn
_es_858 dw 7
	dw 6,1
	  dd es_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd es_yn
_es_437 dw 7
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
	dw 6,35
	  dd es_yn
_hu_852 dw 7
	dw 6,1
	  dd hu_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd hu_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd hu_yn
_hu_850 dw 7
	dw 6,1
	  dd hu_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd hu_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd hu_yn
_hu_858 dw 7
	dw 6,1
	  dd hu_850
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd hu_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd hu_yn
_yu_852 dw 7
	dw 6,1
	  dd yu_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_yu_855 dw 7
	dw 6,1
	  dd yu_855
	dw 6,2
	  dd ucase_855
	dw 6,4
	  dd ucase_855
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_855
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn_855
_yu_872 dw 7
	dw 6,1
	  dd yu_872
	dw 6,2
	  dd ucase_872
	dw 6,4
	  dd ucase_872
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_872
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn_872
_yu_850 dw 7
	dw 6,1
	  dd yu_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_yu_858 dw 7
	dw 6,1
	  dd yu_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_it_850 dw 7
	dw 6,1
	  dd it_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd it_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd it_yn
_it_858 dw 7
	dw 6,1
	  dd it_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd it_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd it_yn
_it_437 dw 7
	dw 6,1
	  dd it_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd it_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd it_yn
_ro_852 dw 7
	dw 6,1
	  dd ro_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ro_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ro_yn
_ro_850 dw 7
	dw 6,1
	  dd ro_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ro_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ro_yn
_ro_858 dw 7
	dw 6,1
	  dd ro_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ro_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ro_yn
_ch_850 dw 7
	dw 6,1
	  dd ch_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ch_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd de_yn
_ch_858 dw 7
	dw 6,1
	  dd ch_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ch_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd de_yn
_ch_437 dw 7
	dw 6,1
	  dd ch_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ch_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd de_yn
_cz_852 dw 7
	dw 6,1
	  dd cz_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd cz_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd cz_yn
_cz_850 dw 7
	dw 6,1
	  dd cz_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd cz_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd cz_yn
_cz_858 dw 7
	dw 6,1
	  dd cz_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd cz_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd cz_yn
_at_850 dw 7
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
	dw 6,35
	  dd de_yn
_at_858 dw 7
	dw 6,1
	  dd at_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd de_yn
_at_437 dw 7
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
	dw 6,35
	  dd de_yn
_uk_850 dw 7
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
	dw 6,35
	  dd en_yn
_uk_858 dw 7
	dw 6,1
	  dd uk_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd en_yn
_uk_437 dw 7
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
	dw 6,35
	  dd en_yn
_dk_865 dw 7
	dw 6,1
	  dd dk_865
	dw 6,2
	  dd ucase_865
	dw 6,4
	  dd ucase_865
	dw 6,5
	  dd fchar
	dw 6,6
	  dd dk_collate_865
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd dk_yn
_dk_850 dw 7
	dw 6,1
	  dd dk_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd dk_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd dk_yn
_dk_858 dw 7
	dw 6,1
	  dd dk_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd dk_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd dk_yn
_se_850 dw 7
	dw 6,1
	  dd se_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd se_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd se_yn
_se_858 dw 7
	dw 6,1
	  dd se_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd se_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd se_yn
_se_437 dw 7
	dw 6,1
	  dd se_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd se_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd se_yn
_no_865 dw 7
	dw 6,1
	  dd no_865
	dw 6,2
	  dd ucase_865
	dw 6,4
	  dd ucase_865
	dw 6,5
	  dd fchar
	dw 6,6
	  dd no_collate_865
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd no_yn
_no_850 dw 7
	dw 6,1
	  dd no_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd no_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd no_yn
_no_858 dw 7
	dw 6,1
	  dd no_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd no_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd no_yn
_pl_852 dw 7
	dw 6,1
	  dd pl_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pl_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pl_yn
_pl_850 dw 7
	dw 6,1
	  dd pl_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pl_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pl_yn
_pl_858 dw 7
	dw 6,1
	  dd pl_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pl_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pl_yn
_de_850 dw 7
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
	dw 6,35
	  dd de_yn
_de_858 dw 7
	dw 6,1
	  dd de_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd de_yn
_de_437 dw 7
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
	dw 6,35
	  dd de_yn
_ar_437 dw 7
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
	dw 6,35
	  dd es_yn
_ar_850 dw 7
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
	dw 6,35
	  dd es_yn
_ar_858 dw 7
	dw 6,1
	  dd ar_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd es_yn
_br_850 dw 7
	dw 6,1
	  dd br_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pt_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pt_yn
_br_858 dw 7
	dw 6,1
	  dd br_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pt_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pt_yn
_br_437 dw 7
	dw 6,1
	  dd br_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pt_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pt_yn
_my_437 dw 7
	dw 6,1
	  dd my_437
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
	dw 6,35
	  dd en_yn
_au_437 dw 7
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
	dw 6,35
	  dd en_yn
_au_850 dw 7
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
	dw 6,35
	  dd en_yn
_au_858 dw 7
	dw 6,1
	  dd au_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd en_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd en_yn
_sg_437 dw 7
	dw 6,1
	  dd sg_437
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
	dw 6,35
	  dd en_yn
_jp_437 dw 7
	dw 6,1
	  dd jp_437
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
	dw 6,35
	  dd en_yn	; Japanese MS-DOS uses "Y" and "N" - Yuki
_jp_932 dw 7
	dw 6,1
	  dd jp_932
	dw 6,2
	  dd ucase_932
	dw 6,4
	  dd ucase_932
	dw 6,5
	  dd fchar
	dw 6,6
	  dd jp_collate_932
	dw 6,7
	  dd jp_dbcs_932
	dw 6,35
	  dd en_yn
_kr_437 dw 7
	dw 6,1
	  dd kr_437
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
	dw 6,35
	  dd kr_yn
_kr_934 dw 7
	dw 6,1
	  dd kr_934
	dw 6,2
	  dd ucase_934
	dw 6,4
	  dd ucase_934
	dw 6,5
	  dd fchar
	dw 6,6
	  dd kr_collate_934
	dw 6,7
	  dd kr_dbcs_934
	dw 6,35
	  dd kr_yn
_cn_437 dw 7
	dw 6,1
	  dd cn_437
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
	dw 6,35
	  dd cn_yn
_cn_936 dw 7
	dw 6,1
	  dd cn_936
	dw 6,2
	  dd ucase_936
	dw 6,4
	  dd ucase_936
	dw 6,5
	  dd fchar
	dw 6,6
	  dd cn_collate_936
	dw 6,7
	  dd cn_dbcs_936
	dw 6,35
	  dd cn_yn_936
_tr_857 dw 7
	dw 6,1
	  dd tr_857
	dw 6,2
	  dd ucase_857
	dw 6,4
	  dd ucase_857
	dw 6,5
	  dd fchar
	dw 6,6
	  dd tr_collate_857
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd tr_yn
_tr_850 dw 7
	dw 6,1
	  dd tr_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd tr_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd tr_yn
_in_437 dw 7
	dw 6,1
	  dd in_437
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
	dw 6,35
	  dd en_yn
_pt_860 dw 7
	dw 6,1
	  dd pt_860
	dw 6,2
	  dd ucase_860
	dw 6,4
	  dd ucase_860
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pt_collate_860
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pt_yn
_pt_850 dw 7
	dw 6,1
	  dd pt_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pt_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pt_yn
_pt_858 dw 7
	dw 6,1
	  dd pt_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd pt_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd pt_yn
_fi_850 dw 7
	dw 6,1
	  dd fi_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fi_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fi_yn
_fi_858 dw 7
	dw 6,1
	  dd fi_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fi_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fi_yn
_fi_437 dw 7
	dw 6,1
	  dd fi_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fi_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fi_yn
_bg_855 dw 7
	dw 6,1
	  dd bg_855
	dw 6,2
	  dd ucase_855
	dw 6,4
	  dd ucase_855
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_855
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn_855
_bg_872 dw 7
	dw 6,1
	  dd bg_872
	dw 6,2
	  dd ucase_872
	dw 6,4
	  dd ucase_872
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_872
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn_872
_bg_850 dw 7
	dw 6,1
	  dd bg_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn
_bg_858 dw 7
	dw 6,1
	  dd bg_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn
_bg_866 dw 8
	dw 6,1
	  dd bg_866
	dw 6,2
	  dd ucase_866
	dw 6,3
	  dd lcase_866
	dw 6,4
	  dd ucase_866
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_866
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn_866
_bg_808 dw 8
	dw 6,1
	  dd bg_808
	dw 6,2
	  dd ucase_808
	dw 6,3
	  dd lcase_808
	dw 6,4
	  dd ucase_808
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_808
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn_808
_bg_849 dw 8
	dw 6,1
	  dd bg_849
	dw 6,2
	  dd ucase_849
	dw 6,3
	  dd lcase_849
	dw 6,4
	  dd ucase_849
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_849
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn_849
_bg_1131 dw 8
	dw 6,1
	  dd bg_1131
	dw 6,2
	  dd ucase_1131
	dw 6,3
	  dd lcase_1131
	dw 6,4
	  dd ucase_1131
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_1131
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn_1131
_bg_30033 dw 8
	dw 6,1
	  dd bg_30033
	dw 6,2
	  dd ucase_30033
	dw 6,3
	  dd lcase_30033
	dw 6,4
	  dd ucase_30033
	dw 6,5
	  dd fchar
	dw 6,6
	  dd bg_collate_30033
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd bg_yn_30033
_by_849 dw 8
	dw 6,1
	  dd by_849
	dw 6,2
	  dd ucase_849
	dw 6,3
	  dd lcase_849
	dw 6,4
	  dd ucase_849
	dw 6,5
	  dd fchar
	dw 6,6
	  dd by_collate_849
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd by_yn_849
_by_1131 dw 8
	dw 6,1
	  dd by_1131
	dw 6,2
	  dd ucase_1131
	dw 6,3
	  dd lcase_1131
	dw 6,4
	  dd ucase_1131
	dw 6,5
	  dd fchar
	dw 6,6
	  dd by_collate_1131
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd by_yn_1131
_by_850 dw 7
	dw 6,1
	  dd by_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd by_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd by_yn
_by_858 dw 7
	dw 6,1
	  dd by_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd by_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd by_yn
_ua_848 dw 8
	dw 6,1
	  dd ua_848
	dw 6,2
	  dd ucase_848
	dw 6,3
	  dd lcase_848
	dw 6,4
	  dd ucase_848
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ua_collate_848
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ua_yn_848
_ua_1125 dw 8
	dw 6,1
	  dd ua_1125
	dw 6,2
	  dd ucase_1125
	dw 6,3
	  dd lcase_1125
	dw 6,4
	  dd ucase_1125
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ua_collate_1125
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ua_yn_1125
_sr_852 dw 7
	dw 6,1
	  dd sr_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_sr_855 dw 7
	dw 6,1
	  dd sr_855
	dw 6,2
	  dd ucase_855
	dw 6,4
	  dd ucase_855
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_855
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn_855
_sr_872 dw 7
	dw 6,1
	  dd sr_872
	dw 6,2
	  dd ucase_872
	dw 6,4
	  dd ucase_872
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_872
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn_872
_sr_850 dw 7
	dw 6,1
	  dd sr_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_sr_858 dw 7
	dw 6,1
	  dd sr_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_hr_852 dw 7
	dw 6,1
	  dd hr_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd hr_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd hr_yn
_hr_850 dw 7
	dw 6,1
	  dd hr_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd hr_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd hr_yn
_hr_858 dw 7
	dw 6,1
	  dd hr_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd hr_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd hr_yn
_si_852 dw 7
	dw 6,1
	  dd si_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd si_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd si_yn
_si_850 dw 7
	dw 6,1
	  dd si_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd si_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd si_yn
_si_858 dw 7
	dw 6,1
	  dd si_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd si_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd si_yn
_ba_852 dw 7
	dw 6,1
	  dd ba_852
	dw 6,2
	  dd ucase_852
	dw 6,4
	  dd ucase_852
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_852
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_ba_850 dw 7
	dw 6,1
	  dd ba_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_ba_858 dw 7
	dw 6,1
	  dd ba_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn
_ba_855 dw 7
	dw 6,1
	  dd ba_855
	dw 6,2
	  dd ucase_855
	dw 6,4
	  dd ucase_855
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_855
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn_855
_ba_872 dw 7
	dw 6,1
	  dd ba_872
	dw 6,2
	  dd ucase_872
	dw 6,4
	  dd ucase_872
	dw 6,5
	  dd fchar
	dw 6,6
	  dd sh_collate_872
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd sh_yn_872
_mk_855 dw 7
	dw 6,1
	  dd mk_855
	dw 6,2
	  dd ucase_855
	dw 6,4
	  dd ucase_855
	dw 6,5
	  dd fchar
	dw 6,6
	  dd mk_collate_855
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd mk_yn_855
_mk_872 dw 7
	dw 6,1
	  dd mk_872
	dw 6,2
	  dd ucase_872
	dw 6,4
	  dd ucase_872
	dw 6,5
	  dd fchar
	dw 6,6
	  dd mk_collate_872
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd mk_yn_872
_mk_850 dw 7
	dw 6,1
	  dd mk_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd mk_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd mk_yn
_mk_858 dw 7
	dw 6,1
	  dd mk_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd mk_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd mk_yn
_me_850	dw 7
	dw 6,1
	  dd me_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd me_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd me_yn
_me_858	dw 7
	dw 6,1
	  dd me_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd me_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd me_yn
_me_864	dw 7
	dw 6,1
	  dd me_864
	dw 6,2
	  dd ucase_864
	dw 6,4
	  dd ucase_864
	dw 6,5
	  dd fchar
	dw 6,6
	  dd me_collate_864
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd me_yn_864
_il_850	dw 7
	dw 6,1
	  dd il_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd il_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd il_yn
_il_858	dw 7
	dw 6,1
	  dd il_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd il_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd il_yn
_il_862	dw 7
	dw 6,1
	  dd il_862
	dw 6,2
	  dd ucase_862
	dw 6,4
	  dd ucase_862
	dw 6,5
	  dd fchar
	dw 6,6
	  dd il_collate_862
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd il_yn_862
_nl_BE_850 dw 7
	dw 6,1
	  dd nl_BE_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd nl_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_nl_BE_858 dw 7
	dw 6,1
	  dd nl_BE_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd nl_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_nl_BE_437 dw 7
	dw 6,1
	  dd nl_BE_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd nl_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd nl_yn
_fr_BE_850 dw 7
	dw 6,1
	  dd fr_BE_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_fr_BE_858 dw 7
	dw 6,1
	  dd fr_BE_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_fr_BE_437 dw 7
	dw 6,1
	  dd fr_BE_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_de_BE_850 dw 7
	dw 6,1
	  dd de_BE_850
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
	dw 6,35
	  dd de_yn
_de_BE_858 dw 7
	dw 6,1
	  dd de_BE_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd de_yn
_de_BE_437 dw 7
	dw 6,1
	  dd de_BE_437
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
	dw 6,35
	  dd de_yn
_es_ES_850 dw 7
	dw 6,1
	  dd es_ES_850
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
	dw 6,35
	  dd es_yn
_es_ES_858 dw 7
	dw 6,1
	  dd es_ES_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd es_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd es_yn
_es_ES_437 dw 7
	dw 6,1
	  dd es_ES_437
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
	dw 6,35
	  dd es_yn
_ca_ES_850 dw 7
	dw 6,1
	  dd ca_ES_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ca_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ca_yn
_ca_ES_858 dw 7
	dw 6,1
	  dd ca_ES_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ca_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ca_yn
_ca_ES_437 dw 7
	dw 6,1
	  dd ca_ES_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd ca_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd ca_yn
_gl_ES_850 dw 7
	dw 6,1
	  dd gl_ES_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd gl_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd gl_yn
_gl_ES_858 dw 7
	dw 6,1
	  dd gl_ES_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd gl_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd gl_yn
_gl_ES_437 dw 7
	dw 6,1
	  dd gl_ES_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd gl_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd gl_yn
_eu_ES_850 dw 7
	dw 6,1
	  dd eu_ES_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd eu_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd eu_yn
_eu_ES_858 dw 7
	dw 6,1
	  dd eu_ES_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd eu_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd eu_yn
_eu_ES_437 dw 7
	dw 6,1
	  dd eu_ES_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd eu_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd eu_yn
_de_CH_850 dw 7
	dw 6,1
	  dd de_CH_850
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
	dw 6,35
	  dd de_yn
_de_CH_858 dw 7
	dw 6,1
	  dd de_CH_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd de_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd de_yn
_de_CH_437 dw 7
	dw 6,1
	  dd de_CH_437
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
	dw 6,35
	  dd de_yn
_fr_CH_850 dw 7
	dw 6,1
	  dd fr_CH_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_fr_CH_858 dw 7
	dw 6,1
	  dd fr_CH_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_fr_CH_437 dw 7
	dw 6,1
	  dd fr_CH_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd fr_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd fr_yn
_it_CH_850 dw 7
	dw 6,1
	  dd it_CH_850
	dw 6,2
	  dd ucase_850
	dw 6,4
	  dd ucase_850
	dw 6,5
	  dd fchar
	dw 6,6
	  dd it_collate_850
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd it_yn
_it_CH_858 dw 7
	dw 6,1
	  dd it_CH_858
	dw 6,2
	  dd ucase_858
	dw 6,4
	  dd ucase_858
	dw 6,5
	  dd fchar
	dw 6,6
	  dd it_collate_858
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd it_yn
_it_CH_437 dw 7
	dw 6,1
	  dd it_CH_437
	dw 6,2
	  dd ucase_437
	dw 6,4
	  dd ucase_437
	dw 6,5
	  dd fchar
	dw 6,6
	  dd it_collate_437
	dw 6,7
	  dd dbcs_empty
	dw 6,35
	  dd it_yn

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
ca_863 cnf   2,863,YMD,"$",    0,0,0,0," ",",","-",":",3,2,_24; Canada-French
ca_850 cnf   2,850,YMD,"$",    0,0,0,0," ",",","-",":",3,2,_24; Canada-French
ca_858 cnf   2,858,YMD,"$",    0,0,0,0," ",",","-",":",3,2,_24; Canada-French
la_850 cnf   3,850,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Latin America
la_858 cnf   3,858,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Latin America
la_437 cnf   3,437,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Latin America
ru_866 cnf   7,866,DMY,0E0h,".", 0,0,0," ",",",".",":",3,2,_24; Russia	 Arkady
ru_808 cnf   7,808,DMY,0E0h,".", 0,0,0," ",",",".",":",3,2,_24; Russia
ru_855 cnf   7,855,DMY,0E1h,".", 0,0,0," ",",",".",":",3,2,_24; Russia
ru_872 cnf   7,872,DMY,0E1h,".", 0,0,0," ",",",".",":",3,2,_24; Russia
ru_852 cnf   7,852,DMY,"R","U","B",0,0," ",",",".",":",3,2,_24; Russia
ru_850 cnf   7,850,DMY,"R","U","B",0,0," ",",",".",":",3,2,_24; Russia
ru_858 cnf   7,858,DMY,"R","U","B",0,0," ",",",".",":",3,2,_24; Russia
ru_437 cnf   7,437,DMY,"R","U","B",0,0," ",",",".",":",3,2,_24; Russia
gr_869 cnf  30,869,DMY,0A8h,0D1h,0C7h,0,0,".",",","/",":",1,2,_12; Greece
gr_737 cnf  30,737,DMY,84h,93h,90h,0,0,".",",","/",":",1,2,_12; Greece
gr_850 cnf  30,850,DMY,"E","Y","P",0,0,".",",","/",":",1,2,_12; Greece
gr_858 cnf  30,858,DMY,0D5h,   0,0,0,0,".",",","/",":",1,2,_12; Greece
nl_850 cnf  31,850,DMY,"E","U","R",0,0,".",",","-",":",0,2,_24; Netherlands Bart
nl_858 cnf  31,858,DMY,0D5h,   0,0,0,0,".",",","-",":",0,2,_24; Netherlands
nl_437 cnf  31,437,DMY,"E","U","R",0,0,".",",","-",":",0,2,_24; Netherlands
be_850 cnf  32,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Belgium
be_858 cnf  32,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24; Belgium
be_437 cnf  32,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Belgium
fr_850 cnf  33,850,DMY,"E","U","R",0,0," ",",",".",":",0,2,_24; France
fr_858 cnf  33,858,DMY,0D5h,   0,0,0,0," ",",",".",":",0,2,_24; France
fr_437 cnf  33,437,DMY,"E","U","R",0,0," ",",",".",":",0,2,_24; France
es_850 cnf  34,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Spain	  Aitor
es_858 cnf  34,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24; Spain
es_437 cnf  34,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Spain
hu_852 cnf  36,852,YMD,"F","t",	 0,0,0," ",",",".",":",3,2,_24; Hungary
hu_850 cnf  36,850,YMD,"F","t",	 0,0,0," ",",",".",":",3,2,_24; Hungary
hu_858 cnf  36,858,YMD,"F","t",	 0,0,0," ",",",".",":",3,2,_24; Hungary
yu_852 cnf  38,852,YMD,"D","i","n",0,0,".",",","-",":",2,2,_24; Yugoslavia
yu_855 cnf  38,855,YMD,0A7h,0B7h,0D4h,0,0,".",",","-",":",2,2,_24; Yugoslavia
yu_872 cnf  38,872,YMD,0A7h,0B7h,0D4h,0,0,".",",","-",":",2,2,_24; Yugoslavia
yu_850 cnf  38,850,YMD,"D","i","n",0,0,".",",","-",":",2,2,_24; Yugoslavia
yu_858 cnf  38,858,YMD,"D","i","n",0,0,".",",","-",":",2,2,_24; Yugoslavia
it_850 cnf  39,850,DMY,"E","U","R",0,0,".",",","/",".",0,2,_24; Italy
it_858 cnf  39,858,DMY,0D5h,   0,0,0,0,".",",","/",".",0,2,_24; Italy
it_437 cnf  39,437,DMY,"E","U","R",0,0,".",",","/",".",0,2,_24; Italy
ro_852 cnf  40,852,YMD,"L","e","i",0,0,".",",","-",":",0,2,_24; Romania
ro_850 cnf  40,850,YMD,"L","e","i",0,0,".",",","-",":",0,2,_24; Romania
ro_858 cnf  40,858,YMD,"L","e","i",0,0,".",",","-",":",0,2,_24; Romania
ch_850 cnf  41,850,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24; Switzerland
ch_858 cnf  41,858,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24; Switzerland
ch_437 cnf  41,437,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24; Switzerland
cz_852 cnf  42,852,YMD,"K","C","s",0,0,".",",","-",":",2,2,_24; Czechoslovakia
cz_850 cnf  42,850,YMD,"K","C","s",0,0,".",",","-",":",2,2,_24; Czechoslovakia
cz_858 cnf  42,858,YMD,"K","C","s",0,0,".",",","-",":",2,2,_24; Czechoslovakia
at_850 cnf  43,850,DMY,"E","U","R",0,0,".",",",".",".",0,2,_24; Austria
at_858 cnf  43,858,DMY,0D5h,   0,0,0,0,".",",",".",".",0,2,_24; Austria
at_437 cnf  43,437,DMY,"E","U","R",0,0,".",",",".",".",0,2,_24; Austria
uk_850 cnf  44,850,DMY,9Ch,    0,0,0,0,",",".","/",":",0,2,_24; United Kingdom
uk_858 cnf  44,858,DMY,9Ch,    0,0,0,0,",",".","/",":",0,2,_24; United Kingdom
uk_437 cnf  44,437,DMY,9Ch,    0,0,0,0,",",".","/",":",0,2,_24; United Kingdom
dk_865 cnf  45,865,DMY,"k","r",	 0,0,0,".",",","-",".",2,2,_24; Denmark
dk_850 cnf  45,850,DMY,"k","r",	 0,0,0,".",",","-",".",2,2,_24; Denmark
dk_858 cnf  45,858,DMY,"k","r",	 0,0,0,".",",","-",".",2,2,_24; Denmark
se_850 cnf  46,850,YMD,"K","r",	 0,0,0," ",",","-",".",3,2,_24; Sweden
se_858 cnf  46,858,YMD,"K","r",	 0,0,0," ",",","-",".",3,2,_24; Sweden
se_437 cnf  46,437,YMD,"K","r",	 0,0,0," ",",","-",".",3,2,_24; Sweden
no_865 cnf  47,865,DMY,"K","r",	 0,0,0,".",",",".",":",2,2,_24; Norway
no_850 cnf  47,850,DMY,"K","r",	 0,0,0,".",",",".",":",2,2,_24; Norway
no_858 cnf  47,858,DMY,"K","r",	 0,0,0,".",",",".",":",2,2,_24; Norway
pl_852 cnf  48,852,YMD,"Z",88h,	 0,0,0,".",",","-",":",0,2,_24; Poland	 Michal
pl_850 cnf  48,850,YMD,"P","L","N",0,0,".",",","-",":",0,2,_24; Poland
pl_858 cnf  48,858,YMD,"P","L","N",0,0,".",",","-",":",0,2,_24; Poland
de_850 cnf  49,850,DMY,"E","U","R",0,0,".",",",".",".",1,2,_24; Germany	    Tom
de_858 cnf  49,850,DMY,0D5h,   0,0,0,0,".",",",".",".",1,2,_24; Germany
de_437 cnf  49,437,DMY,"E","U","R",0,0,".",",",".",".",1,2,_24; Germany
ar_850 cnf  54,850,DMY,"$",    0,0,0,0,".",",","/",".",0,2,_24; Argentina
ar_858 cnf  54,858,DMY,"$",    0,0,0,0,".",",","/",".",0,2,_24; Argentina
ar_437 cnf  54,437,DMY,"$",    0,0,0,0,".",",","/",".",0,2,_24; Argentina
br_850 cnf  55,850,DMY,"C","r","$",0,0,".",",","/",":",2,2,_24; Brazil
br_858 cnf  55,858,DMY,"C","r","$",0,0,".",",","/",":",2,2,_24; Brazil
br_437 cnf  55,437,DMY,"C","r","$",0,0,".",",","/",":",2,2,_24; Brazil
my_437 cnf  60,437,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Malaysia
au_437 cnf  61,437,DMY,"$",    0,0,0,0,",",".","-",":",0,2,_12; Australia
au_850 cnf  61,850,DMY,"$",    0,0,0,0,",",".","-",":",0,2,_12; Australia
au_858 cnf  61,858,DMY,"$",    0,0,0,0,",",".","-",":",0,2,_12; Australia
sg_437 cnf  65,437,DMY,"$",    0,0,0,0,",",".","/",":",0,2,_12; Singapore
jp_932 cnf  81,932,YMD,5Ch,    0,0,0,0,",",".","-",":",0,0,_24; Japan	   Yuki
jp_437 cnf  81,437,YMD,9Dh,    0,0,0,0,",",".","-",":",0,0,_24; Japan
kr_934 cnf  82,934,YMD,5Ch,    0,0,0,0,",",".",".",":",0,0,_24; Korea
kr_437 cnf  82,437,YMD,"K","R","W",0,0,",",".",".",":",0,0,_24; Korea
cn_936 cnf  86,936,YMD,5Ch,    0,0,0,0,",",".",".",":",0,2,_12; China
cn_437 cnf  86,437,YMD,9Dh,    0,0,0,0,",",".",".",":",0,2,_12; China
tr_857 cnf  90,857,DMY,"T","L",	 0,0,0,".",",","/",":",4,2,_24; Turkey
tr_850 cnf  90,850,DMY,"T","L",	 0,0,0,".",",","/",":",4,2,_24; Turkey
in_437 cnf  91,437,DMY,"R","s",	 0,0,0,".",",","/",":",0,2,_24; India
pt_860 cnf 351,860,DMY,"E","U","R",0,0,".",",","-",":",0,2,_24; Portugal
pt_850 cnf 351,850,DMY,"E","U","R",0,0,".",",","-",":",0,2,_24; Portugal
pt_858 cnf 351,858,DMY,0D5h,   0,0,0,0,".",",","-",":",0,2,_24; Portugal
fi_850 cnf 358,850,DMY,"E","U","R",0,0," ",",",".",".",3,2,_24; Finland	   Wolf
fi_858 cnf 358,858,DMY,0D5h,   0,0,0,0," ",",",".",".",3,2,_24; Finland
fi_437 cnf 358,437,DMY,"E","U","R",0,0," ",",",".",".",3,2,_24;
bg_855 cnf 359,855,DMY,0D0h,0EBh,".",0,0," ",",",".",",",3,2,_24; Bulgaria  Lucho&RDPK7
bg_872 cnf 359,872,DMY,0D0h,0EBh,".",0,0," ",",",".",",",3,2,_24; Bulgaria  Lucho&RDPK7
bg_850 cnf 359,850,DMY,"B","G","N",0,0," ",",",".",",",3,2,_24; Bulgaria  RDPK7
bg_858 cnf 359,858,DMY,"B","G","N",0,0," ",",",".",",",3,2,_24; Bulgaria  RDPK7
bg_866 cnf 359,866,DMY,0ABh,0A2h,".",0,0," ",",",".",",",3,2,_24; Bulgaria
bg_808 cnf 359,808,DMY,0ABh,0A2h,".",0,0," ",",",".",",",3,2,_24; Bulgaria
bg_849 cnf 359,849,DMY,0ABh,0A2h,".",0,0," ",",",".",",",3,2,_24; Bulgaria
bg_1131 cnf 359,1131,DMY,0ABh,0A2h,".",0,0," ",",",".",",",3,2,_24; Bulgaria
bg_30033 cnf 359,30033,DMY,0ABh,0A2h,".",0,0," ",",",".",",",3,2,_24; Bulgaria  RDPK7
by_849 cnf 375,849,DMY,0E0h,0E3h,0A1h,".",0," ",",",".",":",3,2,_24;Belarus
by_1131 cnf 375,1131,DMY,0E0h,0E3h,0A1h,".",0," ",",",".",":",3,2,_24; Belarus
by_850 cnf 375,850,DMY,"B","Y","R",0,0," ",",",".",",",3,2,_24; Belarus
by_858 cnf 375,858,DMY,"B","Y","R",0,0," ",",",".",",",3,2,_24; Belarus
ua_848 cnf 380,848,DMY,0A3h,0E0h,0ADh,".",0," ",",",".",":",3,2,_24;Ukraine Oleg
ua_1125 cnf 380,1125,DMY,0A3h,0E0h,0ADh,".",0," ",",",".",":",3,2,_24; Ukraine
sr_855 cnf 381,855,DMY,0A7h,0B7h,0D4h,0,0,".",",",".",":",3,2,_24; Serbia
sr_872 cnf 381,872,DMY,0A7h,0B7h,0D4h,0,0,".",",",".",":",3,2,_24; Serbia
sr_852 cnf 381,852,DMY,"D","i","n",0,0,".",",",".",":",3,2,_24; Serbia
sr_850 cnf 381,850,DMY,"D","i","n",0,0,".",",",".",":",3,2,_24; Serbia
sr_858 cnf 381,858,DMY,"D","i","n",0,0,".",",",".",":",3,2,_24; Serbia
hr_852 cnf 384,852,DMY,"k","n",  0,0,0,".",",",".",".",3,2,_24; Croatia
hr_850 cnf 384,850,DMY,"k","n",  0,0,0,".",",",".",".",3,2,_24; Croatia
hr_858 cnf 384,858,DMY,"k","n",  0,0,0,".",",",".",".",3,2,_24; Croatia
si_852 cnf 386,852,DMY,"S","I","T",0,0,".",",",".",":",3,2,_24; Slovenia
si_850 cnf 386,850,DMY,"S","I","T",0,0,".",",",".",":",3,2,_24; Slovenia
si_858 cnf 386,858,DMY,"S","I","T",0,0,".",",",".",":",3,2,_24; Slovenia
ba_852 cnf 387,852,DMY,"K","M",  0,0,0,".",",",".",".",1,2,_24; Bosnia
ba_850 cnf 387,850,DMY,"K","M",  0,0,0,".",",",".",".",1,2,_24; Bosnia
ba_858 cnf 387,858,DMY,"K","M",  0,0,0,".",",",".",".",1,2,_24; Bosnia
ba_855 cnf 387,855,DMY,"K","M",  0,0,0,".",",",".",":",1,2,_24; Bosnia
ba_872 cnf 387,872,DMY,"K","M",  0,0,0,".",",",".",":",1,2,_24; Bosnia
mk_855 cnf 389,855,DMY,0A7h,0A8h,0D4h,0,0,".",",",".",":",3,2,_24; Macedonia
mk_872 cnf 389,872,DMY,0A7h,0A8h,0D4h,0,0,".",",",".",":",3,2,_24; Macedonia
mk_850 cnf 389,850,DMY,"D","e","n",0,0,".",",",".",":",3,2,_24; Macedonia
mk_858 cnf 389,858,DMY,"D","e","n",0,0,".",",",".",":",3,2,_24; Macedonia
me_864 cnf 785,864,DMY,0A4h,   0,0,0,0,".",",","/",":",1,3,_12; Middle East
me_850 cnf 785,850,DMY,0CFh,   0,0,0,0,".",",","/",":",3,3,_12; Middle East
me_858 cnf 785,858,DMY,0CFh,   0,0,0,0,".",",","/",":",3,3,_12; Middle East
il_862 cnf 972,862,DMY,99h,    0,0,0,0,",","."," ",":",2,2,_24; Israel
il_850 cnf 972,850,DMY,"N","I","S",0,0,",","."," ",":",2,2,_24; Israel
il_858 cnf 972,858,DMY,"N","I","S",0,0,",","."," ",":",2,2,_24; Israel
es_ES_850 cnf 40034,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Spain:
es_ES_858 cnf 40034,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24;  Spanish
es_ES_437 cnf 40034,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24
ca_ES_850 cnf 41034,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24;  Catalan
ca_ES_858 cnf 41034,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24
ca_ES_437 cnf 41034,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24
gl_ES_850 cnf 42034,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24;  Galician
gl_ES_858 cnf 42034,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24
gl_ES_437 cnf 42034,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24
eu_ES_850 cnf 43034,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24;  Basque
eu_ES_858 cnf 43034,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24
eu_ES_437 cnf 43034,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24
nl_BE_850 cnf 40032,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24; Belgium:
nl_BE_858 cnf 40032,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24;  Dutch
nl_BE_437 cnf 40032,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24
fr_BE_850 cnf 41032,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24;  French
fr_BE_858 cnf 41032,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24
fr_BE_437 cnf 41032,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24
de_BE_850 cnf 42032,850,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24;  German
de_BE_858 cnf 42032,858,DMY,0D5h,   0,0,0,0,".",",","/",":",0,2,_24
de_BE_437 cnf 42032,437,DMY,"E","U","R",0,0,".",",","/",":",0,2,_24
de_CH_850 cnf 40041,850,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24; Switzerland
de_CH_858 cnf 40041,858,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24;  German
de_CH_437 cnf 40041,437,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24
fr_CH_850 cnf 41041,850,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24;  French
fr_CH_858 cnf 41041,858,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24
fr_CH_437 cnf 41041,437,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24
it_CH_850 cnf 42041,850,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24;  Italian
it_CH_858 cnf 42041,858,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24
it_CH_437 cnf 42041,437,DMY,"F","r",".",0,0,"'",".",".",",",2,2,_24

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
	  dw 128		; small fix: 'ÿ' -> 'Y' -- eca
db 128, 154, 144, 182, 142, 183, 143, 128
db 210, 211, 212, 216, 215, 222, 142, 143
db 144, 146, 146, 226, 153, 227, 234, 235
db  99, 153, 154, 157, 156, 157, 158, 159
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

ucase_858 db 0FFh,"UCASE  "
	  dw 128
db 128, 154, 144, 182, 142, 183, 143, 128
db 210, 211, 212, 216, 215, 222, 142, 143
db 144, 146, 146, 226, 153, 227, 234, 235
db  99, 153, 154, 157, 156, 157, 158, 159
db 181, 214, 224, 233, 165, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 199, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 209, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 229, 229, 230, 232
db 232, 233, 234, 235, 237, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

ucase_860 db 0FFh,"UCASE  "
          dw 128		; Derived from ucase_437
db 128, 154, 144, 143, 142, 145, 134, 128
db 137, 137, 146, 139, 140, 152, 142, 143
db 144, 145, 146, 140, 153, 169, 150, 157
db 152, 153, 154, 155, 156, 157, 158, 159
db 134, 139, 159, 150, 165, 165, 166, 167
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

ucase_857 db 0FFh,"UCASE  "	; Turkish. Needs NLSFUNC for proper uppercasing
          dw 128		; of letter "i" (dotted i)
db 128, 154, 144, 182, 142, 183, 143, 128
db 210, 211, 212, 216, 215,  73, 142, 143
db 144, 146, 146, 226, 153, 227, 234, 235
db 152, 153, 154, 157, 156, 157, 158, 158
db 181, 214, 224, 233, 165, 165, 166, 166
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 199, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 229, 229, 230, 231
db 232, 233, 234, 235, 222,  89, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

ucase_863 db 0FFh,"UCASE  "
          dw 128		; Derived from ucase_437
db 128, 154, 144, 132, 132, 142, 134, 128
db 146, 148, 145, 149, 168, 141, 142, 143
db 144, 145, 146, 153, 148, 149, 158, 157
db 152, 153, 154, 155, 156, 157, 158, 159
db 160, 161,  79,  85, 164, 165, 166, 167
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

ucase_865 db 0FFh,"UCASE  "
          dw 128
db 128, 154, 144,  65, 142,  65, 143, 128
db  69,  69,  69,  73,  73,  73, 142, 143
db 144, 146, 146,  89, 153,  89,  85,  85
db  89, 153, 154, 157, 156, 157, 158, 159
db  65,  73,  79,  85, 165, 165, 166, 167
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

ucase_866 db 0FFh,"UCASE  "
          dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 240, 240, 242, 242, 244, 244, 246, 246
db 248, 249, 250, 251, 252, 253, 254, 255

ucase_808 equ ucase_866

ucase_852 db 0FFh,"UCASE  "
          dw 128
db 128, 154, 144, 182, 142, 222, 143, 128
db 157, 211, 138, 138, 215, 141, 142, 143
db 144, 145, 145, 226, 153, 149, 149, 151
db 151, 153, 154, 155, 155, 157, 158, 172
db 181, 146, 224, 233, 164, 164, 166, 166
db 168, 168, 170, 141, 172, 184, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 189, 191
db 192, 193, 194, 195, 196, 197, 198, 198
db 200, 201, 202, 203, 204, 205, 206, 207
db 209, 209, 210, 211, 210, 213, 214, 215
db 183, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 227, 213, 230, 230
db 232, 233, 232, 235, 237, 237, 221, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 235, 252, 252, 254, 255

ucase_855 db 0FFh,"UCASE  "
          dw 128
db 129, 129, 131, 131, 133, 133, 135, 135
db 137, 137, 139, 139, 141, 141, 143, 143
db 145, 145, 147, 147, 149, 149, 151, 151
db 153, 153, 155, 155, 157, 157, 159, 159
db 161, 161, 163, 163, 165, 165, 167, 167
db 169, 169, 171, 171, 173, 173, 174, 175
db 176, 177, 178, 179, 180, 182, 182, 184
db 184, 185, 186, 187, 188, 190, 190, 191
db 192, 193, 194, 195, 196, 197, 199, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 209, 209, 211, 211, 213, 213, 215, 215
db 221, 217, 218, 219, 220, 221, 224, 223
db 224, 226, 226, 228, 228, 230, 230, 232
db 232, 234, 234, 236, 236, 238, 238, 239
db 240, 242, 242, 244, 244, 246, 246, 248
db 248, 250, 250, 252, 252, 253, 254, 255

ucase_872 equ ucase_855

ucase_30033 db 0FFh,"UCASE  " ; MIK codepage
	  dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

ucase_869 db 0FFh,"UCASE  "
          dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 134, 155, 141, 143, 144
db 145, 145, 146, 149, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 164, 165
db 166, 217, 218, 219, 220, 167, 168, 223
db 169, 170, 172, 173, 181, 182, 183, 184
db 189, 190, 198, 199, 207, 207, 208, 239
db 240, 241, 209, 210, 211, 245, 212, 247
db 248, 249, 213, 150, 150, 152, 254, 255

ucase_737 db 0FFh,"UCASE  "
          dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 145, 146, 147, 148, 149, 150
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 151, 234, 235, 236, 244, 237, 238, 239
db 245, 240, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

ucase_932 db 0FFh,"UCASE  "
          dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 160, 161, 162, 163, 164, 165, 166, 167
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

ucase_934 equ ucase_932
ucase_936 equ ucase_932

ucase_848 db 0FFh,"UCASE  "
          dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 240, 240, 242, 242, 244, 244, 246, 246
db 248, 248, 250, 251, 252, 253, 254, 255

ucase_1125 equ ucase_848

ucase_849 db 0FFh,"UCASE  "
          dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 240, 240, 242, 242, 244, 244, 246, 246
db 248, 248, 250, 251, 252, 252, 254, 255

ucase_1131 equ ucase_849

ucase_862 db 0FFh,"UCASE  "
	  dw 128
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
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

ucase_864 equ ucase_932

; Lowercase equivalents of chars 00h to FFh
;------------------------------------------------------------------------------
lcase_866 db 0FFh,"LCASE  "
          dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122,  91,  92,  93,  94,  95
db  96,  97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122, 123, 124, 125, 126, 127
db 160, 161, 162, 163, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 160, 161, 162, 163, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 241, 241, 243, 243, 245, 245, 247, 247
db 248, 249, 250, 251, 252, 253, 254, 255

lcase_808 equ lcase_866

lcase_848 db 0FFh,"LCASE  "
          dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122,  91,  92,  93,  94,  95
db  96,  97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122, 123, 124, 125, 126, 127
db 160, 161, 162, 163, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 160, 161, 162, 163, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 241, 241, 243, 243, 245, 245, 247, 247
db 249, 249, 250, 251, 252, 253, 254, 255

lcase_1125 equ lcase_848

lcase_849 db 0FFh,"LCASE  "
          dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122,  91,  92,  93,  94,  95
db  96,  97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122, 123, 124, 125, 126, 127
db 160, 161, 162, 163, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 160, 161, 162, 163, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 241, 241, 243, 243, 245, 245, 247, 247
db 249, 249, 250, 251, 253, 253, 254, 255

lcase_1131 equ lcase_849

lcase_30033 db 0FFh,"LCASE  "
          dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,	 97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122,  91,  92,  93,  94,  95
db  96,  97,  98,  99, 100, 101, 102, 103
db 104, 105, 106, 107, 108, 109, 110, 111
db 112, 113, 114, 115, 116, 117, 118, 119
db 120, 121, 122, 123, 124, 125, 126, 127
db 160, 161, 162, 163, 164, 165, 166, 167
db 168, 169, 170, 171, 172, 173, 174, 175
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 160, 161, 162, 163, 164, 165, 166, 167
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
db  64,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  80
db  81,	 82,  83,  84,	85,  86,  87,  88
db  89,	 90,  91,  92,	93,  94,  95,  96
db  97,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  80
db  81,	 82,  83,  84,	85,  86,  87,  88
db  89,	 90,  91, 123, 124, 125, 126, 127
db  68,	 86,  69,  65,	65,  65,  65,  68
db  69,	 69,  69,  73,	73,  73,  65,  65
db  69,	 65,  65,  80,	80,  80,  86,  86
db  90,	 80,  86,  80,	36,  80, 158,  36
db  65,	 73,  80,  86,	79,  79,  65,  80
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,	 68,  69,  69,	69,  36,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  80,	225,  80,  80,	80,  80, 230, 231
db 232,	 86,  86,  86,	90,  90, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,	51,  50, 254, 255

ca_collate_850 equ en_collate_850	; Catalan, CP850
ca_collate_858 equ en_collate_858	; Catalan, CP858
ca_collate_437 equ en_collate_437	; Catalan, CP437
gl_collate_850 equ en_collate_850	; Gallegan, CP850
gl_collate_858 equ en_collate_858	; Gallegan, CP858
gl_collate_437 equ en_collate_437	; Gallegan, CP437
eu_collate_850 equ en_collate_850	; Basque, CP850
eu_collate_858 equ en_collate_858	; Basque, CP858
eu_collate_437 equ en_collate_437	; Basque, CP437

de_collate_850 equ en_collate_850	; German, CP850
de_collate_858 equ en_collate_858	; German, CP858
de_collate_437 equ en_collate_437	; German, CP437

pt_collate_860 db 0FFh,"COLLATE"	; Portuguese, CP860
	       dw 256			; Derived from English CP437
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
db  69,	 69,  69,  73,	79,  73,  65,  65
db  69,	 65,  69,  79,	79,  79,  85,  85
db  73,	 79,  85,  36,	36,  85,  36,  79
db  65,	 73,  79,  85,	78,  78, 166, 167
db  63,  79, 170, 171, 172,  33,  34,  34
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

pt_collate_850 equ en_collate_850	; Portuguese CP850
pt_collate_858 equ en_collate_858	; Portuguese CP858
pt_collate_437 equ en_collate_437	; Portuguese CP437


fr_collate_863 db 0FFh,"COLLATE"	; French, CP863
	       dw 256			; Derived from English CP437
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
db  67,	 85,  69,  65,	65,  65, 134,  67
db  69,	 69,  69,  73,	73, 141,  65, 143
db  69,	 69,  69,  79,	69,  73,  85,  85
db  36,	 79,  85,  36,	36,  85,  85,  36
db 160,	161,  79,  85, 164, 165, 166, 167
db  63, 169, 170, 171, 172, 173,  34,  34
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

fr_collate_850 equ en_collate_850	; French, CP850
fr_collate_858 equ en_collate_858	; French, CP858
fr_collate_437 equ en_collate_437	; French, CP437

it_collate_850 equ en_collate_850	; Italian, CP850
it_collate_858 equ en_collate_858	; Italian, CP858
it_collate_437 equ en_collate_437	; Italian, CP437

nl_collate_850 equ en_collate_850	; Dutch, CP850
nl_collate_858 equ en_collate_858	; Dutch, CP858
nl_collate_437 equ en_collate_437	; Dutch, CP437

be_collate_850 equ en_collate_850	; Belgium, CP850
be_collate_858 equ en_collate_858	; Belgium, CP858
be_collate_437 equ en_collate_437	; Belgium, CP437

tr_collate_857 db 0FFh,"COLLATE"	; Turkish, CP857 (with Euro)
	       dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,  65,  66,  67,  69,  70,  71,  72
db  74,  75,  77,  78,  79,  80,  81,  82
db  84,  85,  86,  87,  89,  90,  92,  93
db  94,  95,  96,  97,  98,  99, 100, 101
db 102,  65,  66,  67,  69,  70,  71,  72
db  74,  76,  77,  78,  79,  80,  81,  82
db  84,  85,  86,  87,  89,  90,  92,  93
db  94,  95,  96, 123, 124, 125, 126, 127
db  68,  91,  70,  65,  65,  65,  65,  68
db  70,  70,  70,  76,  76,  75,  65,  65
db  70, 145, 145,  82,  83,  82,  90,  90
db  76,  83,  91, 155,  36, 155,  88,  88
db  65,  76,  82,  90,  81,  81,  73,  73
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  82,  65,  70,  70,  70,  36,  76,  76
db  76, 217, 218, 219, 220, 221,  76, 223
db  82, 225,  82,  82,  82,  82, 230,  32
db 232,  90,  90,  90,  76,  95, 238, 239
db 240, 241,  32, 243, 244, 245, 246, 247
db 248, 249, 250,  49,  51,  50, 254, 255

tr_collate_850 db 0FFh,"COLLATE"	; Turkish, CP850
        dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  69,  70,  71,  72
db  74,  75,  77,  78,  79,  80,  81,  82
db  84,  85,  86,  87,  89,  90,  92,  93
db  94,  95,  96,  97,  98,  99, 100, 101
db 102,  65,  66,  67,  69,  70,  71,  72
db  74,  76,  77,  78,  79,  80,  81,  82
db  84,  85,  86,  87,  89,  90,  92,  93
db  94,  95,  96, 123, 124, 125, 126, 127
db  68,  91,  70,  65,  65,  65,  65,  68
db  70,  70,  70,  76,  76,  76,  65,  65
db  70, 145, 145,  82,  83,  82,  90,  90
db  95,  83,  91, 155,  36, 155,  36,  36
db  65,  76,  82,  90,  81,  81,  65,  82
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 184, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db 209, 209,  70,  70,  70,  75,  76,  76
db  76, 217, 218, 219, 220, 221,  76, 223
db  82, 225,  82,  82,  82,  82, 230, 231
db 231,  90,  90,  90,  95,  95, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,  51,  50, 254, 255

dk_collate_865 db 0FFh,"COLLATE"		; Danish, CP865
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89,  93,  94,  95,  96,  97
db  98,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89, 123, 124, 125, 126, 127
db  67,  85,  69,  65,  65,  65,  92,  67
db  69,  69,  69,  73,  73,  73,  65,  92
db  69,  90,  90,  79,  79,  79,  85,  85
db  88,  79,  85,  91,  36,  91,  36,  36
db  65,  73,  79,  85,  78,  78,  65,  79
db  63, 169, 170, 171, 172,  33,  34,  36
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224,  83, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251,  78,  50, 254, 255

dk_collate_850 db 0FFh,"COLLATE"		; Danish, CP850
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89,  93,  94,  95,  96,  97
db  98,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89, 123, 124, 125, 126, 127
db  67,  85,  69,  65,  65,  65,  92,  67
db  69,  69,  69,  73,  73,  73,  65,  92
db  69,  90,  90,  79,  79,  79,  85,  85
db  88,  79,  85,  91,  36,  91,  36,  36
db  65,  73,  79,  85,  78,  78,  65,  79
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 169, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,  68,  69,  69,  69,  73,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,  83,  79,  79,  79,  79, 230, 231
db 232,  85,  85,  85,  88,  88, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,  51,  50, 254, 255

dk_collate_858 db 0FFh,"COLLATE"		; Danish, CP858
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89,  93,  94,  95,  96,  97
db  98,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89, 123, 124, 125, 126, 127
db  67,  85,  69,  65,  65,  65,  92,  67
db  69,  69,  69,  73,  73,  73,  65,  92
db  69,  90,  90,  79,  79,  79,  85,  85
db  88,  79,  85,  91,  36,  91,  36,  36
db  65,  73,  79,  85,  78,  78,  65,  79
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 169, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,  68,  69,  69,  69,  36,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,  83,  79,  79,  79,  79, 230, 231
db 232,  85,  85,  85,  88,  88, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,  51,  50, 254, 255

no_collate_865 equ dk_collate_865	; Norwegian CP865
no_collate_850 equ dk_collate_850	; Norwegian CP850
no_collate_858 equ dk_collate_858	; Norwegian CP858

ru_collate_866 db 0FFh,"COLLATE"	; Russian, CP866
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
db 128, 129, 130, 131, 132, 135, 137, 138
db 140, 143, 145, 146, 148, 149, 151, 152
db 153, 154, 155, 158, 160, 161, 162, 163
db 165, 166, 167, 168, 169, 170, 171, 172
db 128, 129, 130, 131, 132, 135, 137, 138
db 140, 143, 145, 146, 148, 149, 151, 152
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 153, 154, 155, 158, 160, 161, 162, 163
db 165, 166, 167, 168, 169, 170, 171, 172
db 135, 135, 136, 136, 142, 142, 159, 159
db 248, 249, 250, 251, 252,  36, 254, 255

ru_collate_808 equ ru_collate_866	; Russian, CP808

ru_collate_852 db 0FFh,"COLLATE" 	; Russian, CP852 (with Euro)
               dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,  65,  68,  69,  72,  74,  76,  77
db  78,  79,  81,  82,  83,  85,  86,  88
db  92,  93,  94,  96, 100, 102, 105, 106
db 107, 108, 109, 114, 115, 116, 117, 118
db 119,  65,  68,  69,  72,  74,  76,  77
db  78,  79,  81,  82,  83,  85,  86,  88
db  92,  93,  94,  96, 100, 102, 105, 106
db 107, 108, 109, 123, 124, 125, 126, 127
db  69, 103,  74,  65,  65, 102,  71,  69
db  84,  74,  91,  91,  80, 111,  65,  71
db  74,  83,  83,  88,  90,  83,  83,  98
db  98,  90, 103, 101, 101,  84,  36,  70
db  65,  79,  88, 102,  67,  67, 110, 110
db  75,  75,  36, 111,  70,  99,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  74
db  99, 185, 186, 187, 188, 112, 112, 191
db 192, 193, 194, 195, 196, 197,  66,  66
db 200, 201, 202, 203, 204, 205, 206,  36
db  73,  73,  72,  74,  72,  86,  79,  80
db  74, 217, 218, 219, 220, 113, 102, 223
db  88, 225,  88,  87,  87,  86,  97,  97
db  94, 102,  94, 104, 108, 108, 113, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 104,  95,  95, 254, 255

pl_collate_852 db 0FFh,"COLLATE" 	; Polish, CP852 (with Euro)
               dw 256
db   0,	  1,   2,   3,	 4,   5,   6,	7
db   8,	  9,  10,  11,	12,  13,  14,  15
db  16,	 17,  18,  19,	20,  21,  22,  23
db  24,	 25,  26,  27,	28,  29,  30,  31
db  32,	 33,  34,  35,	36,  37,  38,  39
db  40,	 41,  42,  43,	44,  45,  46,  47
db  48,	 49,  50,  51,	52,  53,  54,  55
db  56,	 57,  58,  59,	60,  61,  62,  63
db  64,  65,  68,  69,  72,  74,  76,  77
db  78,  79,  81,  82,  83,  85,  86,  88
db  92,  93,  94,  96, 100, 102, 105, 106
db 107, 108, 109, 114, 115, 116, 117, 118
db 119,  65,  68,  69,  72,  74,  76,  77
db  78,  79,  81,  82,  83,  85,  86,  88
db  92,  93,  94,  96, 100, 102, 105, 106
db 107, 108, 109, 123, 124, 125, 126, 127
db  69, 103,  74,  65,  65, 102,  71,  69
db  84,  74,  91,  91,  80, 111,  65,  71
db  74,  83,  83,  88,  90,  83,  83,  98
db  98,  90, 103, 101, 101,  84,  36,  70
db  65,  79,  89, 102,  67,  67, 110, 110
db  75,  75,  36, 111,  70,  99,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  74
db  99, 185, 186, 187, 188, 112, 112, 191
db 192, 193, 194, 195, 196, 197,  66,  66
db 200, 201, 202, 203, 204, 205, 206,  36
db  73,  73,  72,  74,  72,  86,  79,  80
db  74, 217, 218, 219, 220, 113, 102, 223
db  89, 225,  88,  87,  87,  86,  97,  97
db  94, 102,  94, 104, 108, 108, 113, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 104,  95,  95, 254, 255

pl_collate_850 db 0FFh,"COLLATE"		; Polish, CP850
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
db  79,	 66,  79,  79,	79,  79, 230,  97
db  97,	 85,  85,  85,	89,  89, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,	51,  50, 254, 255

pl_collate_858 db 0FFh,"COLLATE"		; Polish, CP858
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
db  79,	 66,  79,  79,	79,  79, 230,  97
db  97,	 85,  85,  85,	89,  89, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,	51,  50, 254, 255

ru_collate_855 db 0FFh,"COLLATE"	; Russian, CP855
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
db 133, 133, 134, 134, 135, 135, 136, 136
db 139, 139, 141, 141, 142, 142, 144, 144
db 147, 147, 150, 150, 156, 156, 157, 157
db 159, 159, 164, 164, 171, 171, 167, 167
db 128, 128, 129, 129, 162, 162, 132, 132
db 135, 135, 160, 160, 131, 131,  34,  34
db 176, 177, 178, 179, 180, 161, 161, 140
db 140, 185, 186, 187, 188, 143, 143, 191
db 192, 193, 194, 195, 196, 197, 145, 145
db 200, 201, 202, 203, 204, 205, 206,  36
db 146, 146, 148, 148, 149, 149, 151, 151
db 152, 217, 218, 219, 220, 152, 172, 223
db 172, 153, 153, 154, 154, 155, 155, 158
db 158, 137, 137, 130, 130, 169, 169, 239
db 240, 168, 168, 138, 138, 165, 165, 170
db 170, 166, 166, 163, 163, 253, 254, 255

ru_collate_872 equ ru_collate_855	; Russian CP872
ru_collate_850 equ en_collate_850	; Russian CP850
ru_collate_858 equ en_collate_858	; Russian CP858
ru_collate_437 equ en_collate_437	; Russian CP437

gr_collate_869 db 0FFh,"COLLATE"	; Greek, CP869 (with Euro)
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64, 117, 118, 119, 120, 121, 122, 123
db 124, 125, 126, 127, 128, 129, 130, 131
db 132, 133, 134, 135, 136, 137, 138, 139
db 140, 141, 142,  65,  66,  67,  68,  69
db  70, 117, 118, 119, 120, 121, 122, 123
db 124, 125, 126, 127, 128, 129, 130, 131
db 132, 133, 134, 135, 136, 137, 138, 139
db 140, 141, 142,  71,  72,  73,  74,  75
db  76,  77,  78,  79,  80,  81,  89,  36
db  88,  83,  84,  85,  86,  93,  87,  95
db  97,  97, 103, 147, 148, 108, 108, 151
db 112,  50,  51,  89,  36,  93,  95,  97
db  97,  97, 103, 108,  89,  90,  91,  92
db  93,  94,  95, 171,  96,  97, 174, 175
db 176, 177, 178, 179, 180,  98,  99, 100
db 101, 185, 186, 187, 188, 102, 103, 191
db 192, 193, 194, 195, 196, 197, 104, 105
db 200, 201, 202, 203, 204, 205, 206, 106
db 107, 108, 109, 110, 111, 112,  89,  90
db  91, 217, 218, 219, 220,  92,  93, 223
db  94,  95,  96,  97,  98,  99, 100, 101
db 102, 103, 104, 105, 106, 106, 107, 113
db 240, 241, 108, 109, 110, 245, 111, 114
db 115, 116, 112, 108, 108, 112, 254, 255

gr_collate_737 db 0FFh,"COLLATE"	; Greek, CP737
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64, 100, 101, 102, 103, 104, 105, 106
db 107, 108, 109, 110, 111, 112, 113, 114
db 115, 116, 117, 118, 119, 120, 121, 122
db 123, 124, 125,  65,  66,  67,  68,  69
db  70, 100, 101, 102, 103, 104, 105, 106
db 107, 108, 109, 110, 111, 112, 113, 114
db 115, 116, 117, 118, 119, 120, 121, 122
db 123, 124, 125,  71,  72,  73,  74,  75
db  76,  77,  78,  79,  80,  81,  82,  83
db  84,  85,  86,  87,  88,  89,  90,  91
db  92,  93,  94,  95,  96,  97,  98,  99
db  76,  77,  78,  79,  80,  81,  82,  83
db  84,  85,  86,  87,  88,  89,  90,  91
db  92,  93,  93,  94,  95,  96,  97,  98
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db  99,  76,  80,  82,  84,  84,  90,  95
db  95,  99,  76,  80,  82,  84,  90,  95
db  99, 241, 242, 243,  84,  95, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

gr_collate_850 equ pl_collate_850	; Polish, CP850
gr_collate_858 equ pl_collate_858	; Polish, CP858

hu_collate_852 equ ru_collate_852	; Hungarian, CP852
hu_collate_850 equ pl_collate_850	; Hungarian, CP850
hu_collate_858 equ gr_collate_858	; Hungarian, CP858

sh_collate_852 equ ru_collate_852	; Serbo-Croatian, CP852
sh_collate_855 equ ru_collate_855	; Serbo-Croatian, CP855
sh_collate_872 equ ru_collate_872	; Serbo-Croatian, CP872
sh_collate_850 equ pl_collate_850	; Serbo-Croatian, CP850
sh_collate_858 equ gr_collate_858	; Serbo-Croatian, CP858

ro_collate_852 equ ru_collate_852	; Romanian, CP852
ro_collate_850 equ pl_collate_850	; Romanian, CP850
ro_collate_858 equ gr_collate_858	; Romanian, CP858

ch_collate_850 equ en_collate_850	; Switzerland, CP850
ch_collate_858 equ en_collate_858	; Switzerland, CP858
ch_collate_437 equ en_collate_437	; Switzerland, CP437

cz_collate_852 equ ru_collate_852	; Czech, CP852
cz_collate_850 equ pl_collate_850	; Czech, CP850
cz_collate_858 equ gr_collate_858	; Czech, CP858

se_collate_850 db 0FFh,"COLLATE"	; Swedish, CP850
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89,  93,  94,  95,  96,  97
db  98,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89, 123, 124, 125, 126, 127
db  67,  88,  69,  65,  91,  65,  90,  67
db  69,  69,  69,  73,  73,  73,  91,  90
db  69,  65,  65,  79,  92,  79,  85,  85
db  88,  92,  88,  79,  36,  79,  36,  36
db  65,  73,  79,  85,  78,  78,  65,  79
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 169, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,  68,  69,  69,  69,  73,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,  83,  79,  79,  79,  79, 230, 231
db 232,  85,  85,  85,  88,  88, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,  51,  50, 254, 255

se_collate_858 db 0FFh,"COLLATE"	; Swedish, CP858
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89,  93,  94,  95,  96,  97
db  98,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89, 123, 124, 125, 126, 127
db  67,  88,  69,  65,  91,  65,  90,  67
db  69,  69,  69,  73,  73,  73,  91,  90
db  69,  65,  65,  79,  92,  79,  85,  85
db  88,  92,  88,  79,  36,  79,  36,  36
db  65,  73,  79,  85,  78,  78,  65,  79
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180,  65,  65,  65
db 169, 185, 186, 187, 188,  36,  36, 191
db 192, 193, 194, 195, 196, 197,  65,  65
db 200, 201, 202, 203, 204, 205, 206,  36
db  68,  68,  69,  69,  69,  36,  73,  73
db  73, 217, 218, 219, 220, 221,  73, 223
db  79,  83,  79,  79,  79,  79, 230, 231
db 232,  85,  85,  85,  88,  88, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250,  49,  51,  50, 254, 255

se_collate_437 db 0FFh,"COLLATE"	; Swedish, CP437
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89,  93,  94,  95,  96,  97
db  98,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  86
db  87,  88,  89, 123, 124, 125, 126, 127
db  67,  88,  69,  65,  91,  65,  90,  67
db  69,  69,  69,  73,  73,  73,  91,  90
db  69,  65,  65,  79,  92,  79,  85,  85
db  88,  92,  88,  36,  36,  36,  36,  36
db  65,  73,  79,  85,  78,  78,  65,  79
db  63, 169, 170, 171, 172,  33,  34,  34
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224,  83, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251,  78,  50, 254, 255

fi_collate_850 equ se_collate_850	; Finnish, CP850
fi_collate_858 equ se_collate_858	; Finnish, CP858
fi_collate_437 equ se_collate_437	; Finnish, CP437

jp_collate_932 db 0FFh,"COLLATE"	; Japanese, CP932
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
db  88,	 89,  90,  91,	36,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db 128, 183, 184, 185, 186, 187, 188, 189
db 190, 191, 192, 193, 194, 195, 196, 197
db 198, 199, 200, 201, 202, 203, 204, 205
db 206, 207, 208, 209, 210, 211, 212, 213
db 129, 130, 131, 132, 133, 136, 182, 138
db 139, 140, 141, 142, 173, 174, 175, 155
db 137, 138, 139, 140, 141, 142, 143, 144
db 145, 146, 147, 148, 149, 150, 151, 152
db 153, 154, 155, 156, 157, 158, 159, 160
db 161, 162, 163, 164, 165, 166, 167, 168
db 168, 170, 171, 172, 173, 174, 175, 176
db 177, 178, 179, 180, 181, 182, 134, 135
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

kr_collate_934 db 0FFh,"COLLATE"	; Korean, CP934
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
db  88,	 89,  90,  91,	36,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db 128, 181, 182, 183, 184, 185, 186, 187
db 188, 189, 190, 191, 192, 193, 194, 195
db 196, 197, 198, 199, 200, 201, 202, 203
db 204, 205, 206, 207, 208, 209, 210, 211
db 212, 213, 214, 215, 216, 217, 218, 219
db 220, 221, 222, 223, 224, 225, 226, 227
db 228, 229, 230, 231, 232, 233, 234, 235
db 236, 237, 238, 239, 240, 241, 242, 243
db 129, 130, 131, 172, 132, 173, 174, 133
db 134, 135, 175, 176, 177, 178, 179, 180
db 149, 136, 137, 138, 150, 139, 140, 141
db 142, 143, 144, 145, 146, 147, 148, 244
db 245, 246, 151, 152, 153, 154, 155, 156
db 247, 248, 157, 158, 159, 160, 161, 162
db 249, 250, 163, 164, 165, 166, 167, 168
db 251, 252, 169, 170, 171, 253, 254, 255

cn_collate_936 db 0FFh,"COLLATE"	; Chinese, CP936
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
db  88,	 89,  90,  91,	36,  93,  94,  95
db  96,	 65,  66,  67,	68,  69,  70,  71
db  72,	 73,  74,  75,	76,  77,  78,  79
db  80,	 81,  82,  83,	84,  85,  86,  87
db  88,	 89,  90, 123, 124, 125, 126, 127
db  36, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 160, 161, 162, 163, 164, 165, 166, 167
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

by_collate_849 db 0FFh,"COLLATE"	; Belarusian, CP849
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  87
db  88,  89,  90,  91,  92,  93,  94,  95
db  96,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  87
db  88,  89,  90, 123, 124, 125, 126, 127
db 128, 129, 130, 131, 133, 134, 137, 138
db 141, 142, 143, 144, 145, 146, 147, 148
db 149, 150, 151, 152, 154, 155, 156, 157
db 158, 159, 160, 161, 162, 163, 164, 165
db 128, 129, 130, 131, 133, 134, 137, 138
db 141, 142, 143, 144, 145, 146, 147, 148
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 201, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 149, 150, 151, 152, 154, 155, 156, 157
db 158, 159, 160, 161, 162, 163, 164, 165
db 136, 136, 135, 135, 140, 140, 153, 153
db 139, 139, 250, 251, 132, 132, 254, 255

by_collate_1131 equ by_collate_849	; Belarusian, CP1131
by_collate_850 equ en_collate_850	; Belarusian CP850
by_collate_858 equ en_collate_858	; Belarusian CP858

bg_collate_30033 db 0FFh,"COLLATE"	; Bulgarian, MIK codepage
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  87
db  88,  89,  90,  91,  92,  93,  94,  95
db  96,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  87
db  88,  89,  90, 123, 124, 125, 126, 127
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 153, 154, 155, 156, 157, 158, 159
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 202, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 224, 225, 226, 227, 228, 229, 230, 231
db 232, 233, 234, 235, 236, 237, 238, 239
db 240, 241, 242, 243, 244, 245, 246, 247
db 248, 249, 250, 251, 252, 253, 254, 255

bg_collate_855 equ ru_collate_855	; Bulgarian, CP855
bg_collate_872 equ ru_collate_872	; Bulgarian, CP872
bg_collate_850 equ en_collate_850	; Bulgarian CP850
bg_collate_858 equ en_collate_858	; Bulgarian CP858
bg_collate_866 equ ru_collate_866	; Bulgarian CP866
bg_collate_808 equ ru_collate_808	; Bulgarian CP808
bg_collate_849 equ by_collate_849	; Bulgarian CP849
bg_collate_1131 equ by_collate_1131	; Bulgarian CP1131

ua_collate_848 db 0FFh,"COLLATE"	; Ukrainian, CP848
	       dw 256
db   0,   1,   2,   3,   4,   5,   6,   7
db   8,   9,  10,  11,  12,  13,  14,  15
db  16,  17,  18,  19,  20,  21,  22,  23
db  24,  25,  26,  27,  28,  29,  30,  31
db  32,  33,  34,  35,  36,  37,  38,  39
db  40,  41,  42,  43,  44,  45,  46,  47
db  48,  49,  50,  51,  52,  53,  54,  55
db  56,  57,  58,  59,  60,  61,  62,  63
db  64,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  87
db  88,  89,  90,  91,  92,  93,  94,  95
db  96,  65,  66,  67,  68,  69,  70,  71
db  72,  73,  74,  75,  76,  77,  78,  79
db  80,  81,  82,  83,  84,  85,  86,  87
db  88,  89,  90, 123, 124, 125, 126, 127
db 128, 129, 130, 131, 133, 135, 137, 138
db 140, 143, 145, 146, 148, 149, 151, 152
db 153, 154, 155, 158, 160, 161, 162, 163
db 165, 166, 167, 168, 169, 170, 171, 172
db 128, 129, 130, 131, 133, 135, 137, 138
db 140, 143, 145, 146, 148, 149, 151, 152
db 176, 177, 178, 179, 180, 181, 182, 183
db 184, 185, 186, 187, 188, 189, 190, 191
db 192, 193, 194, 195, 196, 197, 198, 199
db 200, 201, 201, 203, 204, 205, 206, 207
db 208, 209, 210, 211, 212, 213, 214, 215
db 216, 217, 218, 219, 220, 221, 222, 223
db 153, 154, 155, 158, 160, 161, 162, 163
db 165, 166, 167, 168, 169, 170, 171, 172
db 135, 135, 132, 132, 136, 136, 141, 141
db 142, 142, 250, 251, 252, 36,  254, 255

ua_collate_1125 equ ua_collate_848	; Ukrainian, CP1125

hr_collate_852 equ ru_collate_852	; Croatian, CP852
hr_collate_850 equ pl_collate_850	; Croatian, CP850
hr_collate_858 equ gr_collate_858	; Croatian, CP858

si_collate_852 equ ru_collate_852	; Slovenian, CP852
si_collate_850 equ pl_collate_850	; Slovenian, CP850
si_collate_858 equ gr_collate_858	; Slovenian, CP858

mk_collate_855 equ ru_collate_855	; Macedonian, CP855
mk_collate_872 equ ru_collate_872	; Macedonian, CP872
mk_collate_850 equ pl_collate_850	; Macedonian, CP850
mk_collate_858 equ gr_collate_858	; Macedonian, CP858

il_collate_862 db 0FFh,"COLLATE"		; Hebrew, CP862
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
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 138, 139, 140, 140, 141
db 141, 142, 143, 144, 144, 145, 145, 146
db 147, 148, 149,  36,	36,  36,  36,  36 
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

il_collate_850 equ en_collate_850
il_collate_858 equ en_collate_858

me_collate_864 db 0FFh,"COLLATE"		; Arabic, CP864
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
db 128, 129, 130, 131, 132, 133, 134, 135
db 136, 137, 138, 139, 140, 141, 142, 143
db 144, 145, 146, 147, 148, 149, 150, 151
db 152, 202, 202, 153, 154, 202, 202, 155
db 156, 157, 175,  36,  36, 176, 158, 159
db 179, 180, 182, 183, 160, 184, 185, 186
db 164, 165, 166, 167, 168, 169, 170, 171
db 172, 173, 199, 161, 191, 192, 193, 162
db  36, 174, 175, 176, 177, 197, 178, 179
db 180, 181, 182, 183, 184, 185, 186, 187
db 188, 189, 190, 191, 192, 193, 194, 195
db 196, 197, 198, 219, 220, 221, 222, 197
db 163, 167, 200, 201, 202, 203, 204, 205
db 206, 207, 208, 194, 197, 198, 198, 203
db 209, 209, 204, 205, 205, 207, 208, 198
db 200, 202, 202, 202, 201, 208, 254, 255

me_collate_850 equ en_collate_850
me_collate_858 equ en_collate_858

; Dual Byte Character Sets
;   lead-byte ranges
;------------------------------------------------------------------------------
dbcs_empty db 0FFh,"DBCS   "
      dw 0			; Table length
      db 0, 0			; Table terminator (even if length == 0)

; Japan, CP932
; Source: http://www.microsoft.com/globaldev/reference/dbcs/932.htm
jp_dbcs_932 db 0FFh,"DBCS   "
      dw 6
      db 081h, 09Fh
      db 0E0h, 0FCh
      db 000h, 000h

; Korean, CP934
kr_dbcs_934 db 0FFh,"DBCS   "
      dw 4
      db 081h, 0BFh
      db 000h, 000h

; Chinese, CP936
cn_dbcs_936 db 0FFh,"DBCS   "
      dw 4
      db 081h, 0FCh
      db 000h, 000h

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

fr_yn db 0FFh,"YESNO  "
      dw 4
      db  'O',0,'N',0 ; French

pt_yn equ es_yn       ; Portuguese

fi_yn db 0FFh,"YESNO  "
      dw 4
      db  'K',0,'E',0 ; Finnish


it_yn equ es_yn       ; Italian

gr_yn db 0FFh,"YESNO  "
      dw 4
      db  'N',0,'O',0 ; Greek, latin alphabet

gr_yn_869 db 0FFh,"YESNO  "
      dw 4
      db  0B8h,0,0BEh,0 ; Greek, codepage 869

gr_yn_737 db 0FFh,"YESNO  "
      dw 4
      db  8Ch,0,8Eh,0 ; Greek, codepage 737

nl_yn equ de_yn       ; Dutch

tr_yn db 0FFh,"YESNO  "
      dw 4
      db  'E',0,'H',0 ; Turkish

ru_yn_866 db 0FFh,"YESNO  "
      dw 4
      db  84h,0,8Dh,0   ; Russian CP866

ru_yn_808 equ ru_yn_866

ru_yn_855 db 0FFh,"YESNO  "
      dw 4
      db 0A7h,0,0D5h,0  ; Russian CP855

ru_yn_872 equ ru_yn_855

ru_yn db 0FFh,"YESNO  "
      dw 4
      db 'D',0,'N',0    ; Russian Latin

by_yn_849 equ ru_yn_866 ; Belarusian
by_yn_1131 equ ru_yn_866
by_yn equ ru_yn         ; Belarusian Latin

ua_yn_848 equ ru_yn_866 ; Ukrainian
ua_yn_1125 equ ru_yn_866

bg_yn_855 equ ru_yn_855 ; Bulgarian CP855
bg_yn_872 equ ru_yn_872 ; Bulgarian CP872
bg_yn_866 equ ru_yn_866 ; Bulgarian CP866
bg_yn_808 equ ru_yn_808 ; Bulgarian CP808
bg_yn_849 equ by_yn_849 ; Bulgarian CP849
bg_yn_1131 equ by_yn_1131 ; Bulgarian CP1131
bg_yn_30033 equ bg_yn_866 ; Bulgarian MIK
bg_yn equ ru_yn         ; Bulgarian Latin

hu_yn db 0FFh,"YESNO  "
      dw 4
      db 'I',0,'N',0    ; Hungarian


sh_yn_855 equ ru_yn_855 ; Serbo-Croatian CP855
sh_yn_872 equ ru_yn_872 ; Serbo-Croatian CP872
sh_yn equ ru_yn    	; Serbo-Croatian, latin alphabet

hr_yn equ ru_yn    	; Croatian, latin alphabet

mk_yn_855 equ ru_yn_855 ; Macedonian CP855
mk_yn_872 equ ru_yn_872 ; Macedonian CP872
mk_yn equ ru_yn    	; Macedonian latin alphabet

ro_yn equ ru_yn		; Romanian

cz_yn db 0FFh,"YESNO  "
      dw 4
      db 'A','0','N',0	; Czech

pl_yn db 0FFh,"YESNO  "
      dw 4
      db 'T','0','N',0	; Polish

dk_yn equ de_yn    	; Danish

se_yn equ de_yn    	; Swedish

no_yn equ de_yn    	; Norwegian

si_yn equ ru_yn		; Slovenian

kr_yn db 0FFh,"YESNO  "
      dw 4
      db 'Y','0','A',0	; Korean, latin alphabet (Yeh, Anio)

kr_yn_934 db 0FFh,"YESNO  "
      dw 4
      db 0BFh,0B9h,0BEh,0C6h	; Korean, CP934 (Hangul syllables "Ye" and "A")

cn_yn db 0FFh,"YESNO  "
      dw 4
      db 'S','0','B',0	; Chinese (Mandrin), latin alphabet (Shi, Bushi)

cn_yn_936 db 0FFh,"YESNO  "
      dw 4
      db 0CAh,0C7h,0B2h,0BBh; Chinese (Mandrin), CP936

il_yn db 0FFh,"YESNO  "
      dw 4
      db 'K','0','L',0	; Hebrew, latin alphabet (Ken, Lo)

il_yn_862 db 0FFh,"YESNO  "
      dw 4
      db 8Bh,'0',8Ch,0	; Hebrew, CP862

me_yn db 0FFh,"YESNO  "
      dw 4
      db 'N','0','L',0	; Arabic, latin alphabet (Nam, La)

me_yn_864 db 0FFh,"YESNO  "
      dw 4
      db 0F2h,'0',9Dh,0	; Arabic, CP864

ca_yn equ es_yn		; Catalan

gl_yn equ es_yn		; Gallegan

eu_yn db 0FFh,"YESNO  "
      dw 4
      db 'B','0','E',0	; Basque

db "FreeDOS" ; Trailing - as recommended by the Ralf Brown Interrupt List
