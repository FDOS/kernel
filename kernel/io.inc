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
; Revision 1.5  2001/09/23 20:39:44  bartoldeman
; FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
;
; Revision 1.4  2001/03/21 02:56:26  bartoldeman
; See history.txt for changes. Bug fixes and HMA support are the main ones.
;
; Revision 1.3  2000/05/25 20:56:21  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/08 04:30:00  jimtabor
; Update CVS to 2020
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
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

    %include "segs.inc"

;
;      Error Return Codes
;

%define E_WRPRT         0       ; Write Protect
%define E_UNIT          1       ; Unknown Unit
%define E_NOTRDY        2       ; Device Not Ready
%define E_CMD           3       ; Unknown Command
%define E_CRC           4       ; Crc Error
%define E_LENGTH        5       ; Bad Length
%define E_SEEK          6       ; Seek Error
%define E_MEDIA         7       ; Unknown MEDIA
%define E_NOTFND        8       ; Sector Not Found
%define E_PAPER         9       ; No Paper
%define E_WRITE         10      ; Write Fault
%define E_READ          11      ; Read Fault
%define E_FAILURE       12      ; General Failure


	extern	_IOExit:wrt TGROUP
	extern	_IOSuccess:wrt TGROUP
	extern	_IOErrorExit:wrt TGROUP
	extern	_IOErrCnt:wrt TGROUP
	extern	_IODone:wrt TGROUP
	extern	_IOCommandError:wrt TGROUP
	extern	GetUnitNum:wrt TGROUP
	extern	_ReqPktPtr:wrt TGROUP
