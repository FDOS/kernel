@echo off

:- batch file to clean and clobber everything
:- $Id$

if "%1" == "" %0 clean
goto %1
goto end

:clean
:clobber
if not exist config.bat echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
if not exist config.bat goto end

call config.bat
if not "%LAST%" == "" call defaults.bat
if     "%LAST%" == "" goto end

cd utils
call %MAKE% %1

cd ..\lib
call %MAKE% %1

cd ..\drivers
call %MAKE% %1

cd ..\boot
call %MAKE% %1

cd ..\sys
call %MAKE% %1

cd ..\kernel
call %MAKE% %1

cd ..\hdr
if exist *.bak del *.bak>nul

cd ..
if exist *.bak del *.bak>nul
if "%1"=="clobber" if exist status.me del status.me>nul

:end
defaults.bat clearset
