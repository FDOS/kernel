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
