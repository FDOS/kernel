;
; File:
;                         rdpcclk.asm
; Description:
;                 read the PC style clock from bios
;
;                       Copyright (c) 1995
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
; $Logfile:   C:/dos-c/src/drivers/rdpcclk.asv  $
;
; $Header$
;
; $Log$
; Revision 1.3  2000/05/25 20:56:19  jimtabor
; Fixed project history
;
; Revision 1.2  2000/05/11 03:56:20  jimtabor
; Clean up and Release
;
; Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
; The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
; MS-DOS.  Distributed under the GNU GPL.
;
; Revision 1.4  1999/08/10 17:21:08  jprice
; ror4 2011-01 patch
;
; Revision 1.3  1999/04/12 03:19:44  jprice
; more ror4 patches
;
; Revision 1.2  1999/03/29 17:08:31  jprice
; ror4 changes
;
; Revision 1.1.1.1  1999/03/29 15:40:32  jprice
; New version without IPL.SYS
;
; Revision 1.2  1999/01/22 04:16:39  jprice
; Formating
;
; Revision 1.1.1.1  1999/01/20 05:51:00  jprice
; Imported sources
;
;
;   Rev 1.2   29 Aug 1996 13:07:10   patv
;Bug fixes for v0.91b
;
;   Rev 1.1   01 Sep 1995 18:50:40   patv
;Initial GPL release.
;
;   Rev 1.0   02 Jul 1995  8:00:26   patv
;Initial revision.
;

group	TGROUP	_TEXT

segment	_TEXT	class=CODE

;
;       BOOL ReadPCClock(Ticks)
;       ULONG *Ticks;
;
                global  _ReadPCClock
_ReadPCClock:
                xor     ah,ah
                int     26
		mov	bx,sp
		mov	bx,[bx+2]
                mov     [bx],dx
                mov     [bx+2],cx
		cbw
                ret
