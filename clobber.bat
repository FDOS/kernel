@echo off
rem batch file to clobber everything

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
%MAKE% clobber

cd ..\lib
%MAKE% clobber

cd ..\drivers
%MAKE% clobber

cd ..\boot
%MAKE% clobber

cd ..\sys
%MAKE% clobber
%MAKE% clobber

cd ..\kernel
%MAKE% clobber

cd ..\hdr
del *.bak

cd ..

del *.bak
del status.me

:end
set MAKE=
set COMPILER=

rem Log: clobber.bat,v 
rem
rem Revision 1.3  1999/08/25 03:59:14  jprice
rem New build batch files.
rem
rem Revision 1.2  1999/08/10 18:34:06  jprice
rem case
rem
rem Revision 1.1  1999/04/23 03:47:19  jprice
rem Initial include
rem

