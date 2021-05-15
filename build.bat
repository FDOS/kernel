@echo off
rem batch file to build everything
rem IF NOTHING COMPILES, CHECK IF YOUR CVS CHECKOUT USES CORRECT DOS LINEBREAKS

if NOT "%1" == "/?" goto start
echo ":-----------------------------------------------------------------------"
echo ":- Syntax: BUILD [-r] [fat32|fat16] [msc|wc|tc|tcpp|bc] [86|186|386]    "
echo ":-               [debug] [lfnapi] [/L #] [/D value] [list] [upx] [win]  "
echo ":- option case is significant !!                                        "
echo ":- Note: Open Watcom (wc) is the preferred compiler                     "
echo ":-----------------------------------------------------------------------"
goto end

:start

:- assume an error until successful build
set XERROR=1
if "%XERROR%" == "" goto noenv

if "%1" == "-r" call clobber.bat
if "%1" == "-r" shift

if not exist config.bat echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
if not exist config.bat goto abort

call config.bat
:-if "%LAST%" == "" goto noenv
set dos4g=quiet

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
if "%1" == "bc"    set COMPILER=BC

if "%1" == "86"    set XCPU=86
if "%1" == "186"   set XCPU=186
if "%1" == "386"   set XCPU=386
if "%1" == "x86"   goto setCPU

if "%1" == "upx"   set XUPX=upx --8086 --best

if "%1" == "debug" set ALLCFLAGS=%ALLCFLAGS% -DDEBUG
if "%1" == "lfnapi" set ALLCFLAGS=%ALLCFLAGS% -DWITHLFNAPI

if "%1" == "win"   set ALLCFLAGS=%ALLCFLAGS% -DWIN31SUPPORT
if "%1" == "win"   set NASMFLAGS=%NASMFLAGS% -DWIN31SUPPORT

if "%1" == "list"  set NASMFLAGS=%NASMFLAGS% -l$*.lst

if "%1" == "/L"    goto setLoadSeg
if "%1" == "/D"    goto setDefine

:nextOption
shift
if not "%1" == "" goto loop_commandline

call default.bat
:-if "%LAST%" == "" goto noenv

:-----------------------------------------------------------------------
:- finally - we are going to compile
:-----------------------------------------------------------------------

echo USING OPTIONS of C=[%ALLCFLAGS%] ASM=[%NASMFLAGS%]

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
if NOT "%XUPX%" == "" %XUPX% ..\bin\sys.com

echo.
echo Process KERNEL +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\kernel
%MAKE% production
if errorlevel 1 goto abort-cd

echo.
echo Process SETVER +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
echo.
cd ..\setver
%MAKE% production
if errorlevel 1 goto abort-cd

cd ..

set XERROR=

:- if you like, put finalizing commands (like copy to floppy) into build2.bat
if exist build2.bat call build2.bat

echo.
echo Processing is done.
goto end

:-----------------------------------------------------------------------

:setLoadSeg
shift
if "%1" == "" echo you MUST specify load segment eg 0x60 with /L option
if "%1" == "" goto abort
set LOADSEG=%1
goto nextOption

:setCPU
shift
if "%1" == "" echo you MUST specify compiler's cpu cmd line argument, eg -5
if "%1" == "" goto abort
set XCPU_EX=%1
goto nextOption

:setDefine
shift
:- Give extra compiler DEFINE flags here
if "%1" == "" echo you MUST specify value to define with /D option
if "%1" == "" echo such as /D DEBUG : extra DEBUG output
if "%1" == "" echo or      /D DOSEMU : printf output goes to dosemu log
if "%1" == "" echo or      /D WIN31SUPPORT : enable Win 3.x hooks
if "%1" == "" goto abort
if "%2" == "/V" goto :setDefineWithValue
set ALLCFLAGS=%ALLCFLAGS% -D%1
set NASMFLAGS=%NASMFLAGS% -D%1
goto nextOption

:setDefineWithValue
set ALLCFLAGS=%ALLCFLAGS% -D%1=%3
set NASMFLAGS=%NASMFLAGS% -D%1=%3
shift
shift
goto nextOption

:noenv
echo Unable to set necessary environment variables!
goto abort

:abort-cd
cd ..

:abort
echo Compilation was aborted!

:end
call default.bat clearset
