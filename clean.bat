@echo off

rem batch file to clean everything

rem $Id$

rem $Log$
rem Revision 1.2  2000/05/14 17:05:58  jimtabor
rem Cleanup CRs
rem
rem Revision 1.3  1999/08/25 03:59:14  jprice
rem New build batch files.
rem
rem Revision 1.2  1999/04/23 03:46:02  jprice
rem Improved by jprice
rem
rem Revision 1.1.1.1  1999/03/29 15:39:15  jprice
rem New version without IPL.SYS
rem
rem Revision 1.4  1999/02/09 04:47:54  jprice
rem Make makefile use common config.mak file
rem
rem Revision 1.3  1999/01/30 08:29:10  jprice
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
%MAKE% -flibm.mak clean

cd ..\drivers
%MAKE% -fdevice.mak clean

cd ..\boot
%MAKE% -fboot.mak clean

cd ..\sys
%MAKE% -fbin2c.mak clean
%MAKE% -fsys.mak clean

cd ..\kernel
%MAKE% -fkernel.mak clean

cd ..\hdr
del *.bak

cd ..

del *.bak

:end
set MAKE=
