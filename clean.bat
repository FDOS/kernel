@echo off

rem batch file to clean everything

rem $Id$

if not exist config.bat goto noconfigbat
goto start

:noconfigbat
echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
goto end

:start
call config.bat
call getmake.bat

cd utils
%MAKE% clean

cd ..\lib
%MAKE% clean

cd ..\drivers
%MAKE% clean

cd ..\boot
%MAKE% clean

cd ..\sys
%MAKE% clean

cd ..\kernel
%MAKE% clean

cd ..\hdr
del *.bak

cd ..

del *.bak

:end
set MAKE=
set COMPILER=

rem Log: clean.bat,v 
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
