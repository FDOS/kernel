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
; $Header$
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


	extern	_IOExit
	extern	_IOSuccess
	extern	_IOErrorExit
	extern	_IOErrCnt
	extern	_IODone
	extern	_IOCommandError
	extern	GetUnitNum
        extern  _ReqPktPtr

