@echo off

:- batch file to build everything
:- $Id$

:-----------------------------------------------------------------------
:- Syntax: BUILD [-r] [fat32|fat16] [msc|wc|tc|tcpp] [86|186|386] [debug] [lfnapi] [/L #]
:- option case is significant !!
:-----------------------------------------------------------------------

set XERROR=1
if "%XERROR%" == "" goto noenv

if "%1" == "-r" call clobber.bat
if "%1" == "-r" shift

if not exist config.bat echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
if not exist config.bat goto abort

call config.bat
if "%LAST%" == "" goto noenv

:-----------------------------------------------------------------------
:- following is command line handling
:- options on the commandline overwrite default settings
:-----------------------------------------------------------------------

:loop_commandline

if "%1" == "fat32" set XFAT=32
if "%1" == "fat16" set XFAT=16

if "%1" == "msc"   set COMPILER=MSC
if "%1" == "wc"    set COMPILER=WATCOM
if "%1" == "tc"    set COMPILER=TC
if "%1" == "tcpp"  set COMPILER=TCPP

if "%1" == "86"    set XCPU=86
if "%1" == "186"   set XCPU=186
if "%1" == "386"   set XCPU=386

if "%1" == "debug" set ALLCFLAGS=%ALLCFLAGS% -DDEBUG
if "%1" == "lfnapi" set ALLCFLAGS=%ALLCFLAGS% -DWITHLFNAPI

if "%1" == "/L"    goto setLoadSeg

:nextOption
shift
if not "%1" == "" goto loop_commandline

if "%COMPILER%" == "" echo you MUST define a COMPILER variable in CONFIG.BAT
if "%COMPILER%" == "" goto abort

call defaults.bat
if "%LAST%" == "" goto noenv

:-----------------------------------------------------------------------
:- finally - we are going to compile
:-----------------------------------------------------------------------

echo.
echo Process UTILS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd utils
call %MAKE% all
if errorlevel 1 goto abort-cd

echo.
echo Process LIB ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\lib
call %MAKE% all
if errorlevel 1 goto abort-cd

echo.
echo Process DRIVERS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\drivers
call %MAKE% all
if errorlevel 1 goto abort-cd

echo.
echo Process BOOT +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\boot
call %MAKE% all
if errorlevel 1 goto abort-cd

echo.
echo Process SYS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\sys
call %MAKE% all
if errorlevel 1 goto abort-cd

echo.
echo Process KERNEL +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\kernel
call %MAKE% all
if errorlevel 1 goto abort-cd

cd ..

:- if you like, put finalizing commands (like copy to floppy) into build2.bat

set XERROR=

if exist build2.bat call build2.bat

echo.
echo Processing is done.
goto end

:-----------------------------------------------------------------------

:setLoadSeg
shift
if "%1" == "" echo you MUST specify load segment eg 0x60 with /L option
if "%1" == "" goot abort
set LOADSEG=%1
goto nextOption

:noenv
echo Unable to set necessary environment variables!
goto abort

:abort-cd
cd ..

:abort
echo Compilation was aborted!

:end
call defaults.bat clearset
