;
; File:
;                            io.inc
; Description:
;     Segments and external common routines used by various device drivers
;
;                       Copyright (c) 1998
;                       Pasquale J. Villani
;                       All Rights Reserved
;
; This file is part of DOS-C.
;
; DOS-C is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version
; 2, or (at your option) any later version.
;
; DOS-C is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
; the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
; $Logfile:   C:/usr/patv/dos-c/src/kernel/io.inv  $
;
; $Header$
;
; $Log$
; Revision 1.2  2000/05/08 04:30:00  jimtabor
; Update CVS to 2020
;
; Revision 1.3  1999/08/10 17:57:12  jprice
; ror4 2011-02 patch
;
; Revision 1.2  1999/03/29 17:05:09  jprice
; ror4 changes
;
; Revision 1.1.1.1  1999/03/29 15:41:08  jprice
; New version without IPL.SYS
;
; Revision 1.1  1999/02/08 05:55:57  jprice
; Added Pat's 1937 kernel patches
;
; 
;    Rev 1.0   06 Dec 1998  8:13:02   patv
; Initial revision.
; $EndLog$
;

group	TGROUP		_TEXT _IO_TEXT _IO_FIXED_DATA
group	DGROUP		_FIXED_DATA _DATA _BSS _BSSEND

segment	_TEXT		class=CODE
segment	_IO_TEXT	class=CODE
segment	_IO_FIXED_DATA	class=CODE align=2
segment	_FIXED_DATA	class=DATA align=16
segment	_DATA		class=DATA align=2
segment	_BSS		class=BSS align=2
segment	_BSSEND		class=BSS

	extern	_IOExit:wrt TGROUP
	extern	_IOSuccess:wrt TGROUP
	extern	_IOErrorExit:wrt TGROUP
	extern	_IOErrCnt:wrt TGROUP
	extern	_IODone:wrt TGROUP
	extern	_IOCommandError:wrt TGROUP
	extern	GetUnitNum:wrt TGROUP
	extern	_ReqPktPtr:wrt TGROUP
