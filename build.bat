@echo off

:- batch file to build everything
:- $Id$

:- Syntax: BUILD [-r] [fat32|fat16] [msc|wc|tc|tcpp] [86|186|386]
:- option case is significant !!

if "%1" == "-r" call clobber
if "%1" == "-r" shift

set XERROR=

if not exist config.bat echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
if not exist config.bat goto abort

call config

:-----------------------------------------------------------------------
:- following is command line handling
:- options on the commandline overwrite default settings
:-----------------------------------------------------------------------

:loop_commandline

if "%1" == "fat32" set XFAT=32
if "%1" == "fat16" set XFAT=16

if "%1" == "msc"   set COMPILER=MSCL8
if "%1" == "wc"    set COMPILER=WATCOM
if "%1" == "tc"    set COMPILER=TC2
if "%1" == "tcpp"  set COMPILER=TURBOCPP

if "%1" == "86"    set XCPU=86
if "%1" == "186"   set XCPU=186
if "%1" == "386"   set XCPU=386

shift
if not "%1" == "" goto loop_commandline

if "%COMPILER%" == "" echo you MUST define a COMPILER variable in CONFIG.BAT
if "%COMPILER%" == "" goto abort

call default

:-----------------------------------------------------------------------
:- finally - we are going to compile
:-----------------------------------------------------------------------

echo.
echo Process UTILS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd utils
%MAKE% production
if errorlevel 1 goto abort-cd

echo.
echo Process LIB ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\lib
%MAKE%
if errorlevel 1 goto abort-cd

echo.
echo Process DRIVERS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\drivers
%MAKE% production
if errorlevel 1 goto abort-cd

echo.
echo Process BOOT +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\boot
%MAKE% production
if errorlevel 1 goto abort-cd

echo.
echo Process SYS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\sys
%MAKE% production
if errorlevel 1 goto abort-cd

echo.
echo Process KERNEL +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\kernel
%MAKE% production
if errorlevel 1 goto abort-cd

cd ..

:- if you like, put some finalizing commands (like copy to floppy)
:- into build2.bat

if exist build2.bat call build2

echo.
echo Processing is done.
goto end

:-----------------------------------------------------------------------

:abort-cd
cd ..
:abort
echo Compilation was aborted!
set XERROR=1

:end
default clearset
