@echo off

rem batch file to build everything

rem $Id$

rem $Log$
rem Revision 1.5  2001/07/09 22:19:30  bartoldeman
rem LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
rem
rem Revision 1.4  2001/03/22 04:13:30  bartoldeman
rem Change LF to CR/LF in batch files.
rem
rem Revision 1.3  2000/05/25 20:56:19  jimtabor
rem Fixed project history
rem
rem Revision 1.2  2000/05/14 17:05:39  jimtabor
rem Cleanup CRs
rem
rem Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
rem The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
rem MS-DOS.  Distributed under the GNU GPL.
rem
rem Revision 1.5  1999/08/25 03:59:14  jprice
rem New build batch files.
rem
rem Revision 1.4  1999/08/25 03:38:16  jprice
rem New build config
rem
rem Revision 1.3  1999/04/23 03:46:02  jprice
rem Improved by jprice
rem
rem Revision 1.2  1999/04/17 19:13:29  jprice
rem ror4 patches
rem
rem Revision 1.1.1.1  1999/03/29 15:39:13  jprice
rem New version without IPL.SYS
rem
rem Revision 1.5  1999/02/09 04:47:54  jprice
rem Make makefile use common config.mak file
rem
rem Revision 1.4  1999/01/30 08:29:10  jprice
rem Clean up
rem
rem Revision 1.3  1999/01/30 07:49:16  jprice
rem Clean up
rem

if not exist config.bat goto noconfigbat
if not exist config.mak goto noconfigmak
goto start

:noconfigbat
echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
goto end

:noconfigmak
echo You must copy CONFIG.M to CONFIG.MAK and edit it to reflect your setup!
goto end

:start
call config.bat

cd lib
%MAKE% -flibm.mak
if errorlevel 1 goto abort

cd ..\drivers
%MAKE% -fdevice.mak production
if errorlevel 1 goto abort

cd ..\boot
%MAKE% -fboot.mak production
if errorlevel 1 goto abort

cd ..\sys
%MAKE% -fbin2c.mak production
if errorlevel 1 goto abort
%MAKE% -fsys.mak production
if errorlevel 1 goto abort

cd ..\kernel
%MAKE% -fkernel.mak production
if errorlevel 1 goto abort

cd..

:- if you like, put some finalizing commands (like copy to floppy)
:- into build2.bat

if exist build2.bat call build2

goto end

:abort
cd ..
:end
set MAKE=
