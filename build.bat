:-@echo off

:-  batch file to build everything

:-  $Id$

set XERROR=


@if not exist config.bat echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
@if not exist config.bat goto end

@if not \%1 == \-r goto norebuild
    call clobber
:norebuild    


call config.bat
call getmake.bat


@set XERROR=

:**********************************************************************
:* DONE with preferences - following is command line handling
:*
:* options on the commandline overwrite your default settings
:*
:* options handled ( case significant !! )
:*
:* BUILD [fat32|fat16] [msc|wc|tc|tcpp] [86|186|386]
:*
:**********************************************************************

:loop_commandline

if \%1 == \ goto done_with_commandline

if %1 == fat32 set XFAT=32
if %1 == fat16 set XFAT=16

if %1 == msc   set COMPILER=MSCL8
if %1 == wc    set COMPILER=WATCOM
if %1 == tc    set COMPILER=TC2
if %1 == tcpp  set COMPILER=TURBOCPP


if %1 == 86    set XCPU=86
if %1 == 186   set XCPU=186
if %1 == 386   set XCPU=386

shift
goto loop_commandline

:done_with_commandline   

if \%COMPILER%      == \ echo you MUST define a COMPILER     variable in CONFIG.BAT
if \%COMPILER%      == \ goto end


@if not "%XLINK%" == "" goto link_set

@if \%COMPILER% == \TC2 set XLINK=%TC2_BASE%\tlink /m/c
@if \%COMPILER% == \TURBOCPP set XLINK=%TP1_BASE%\bin\tlink /m/c
@if \%COMPILER% == \TC3 set XLINK=%TC3_BASE%\bin\tlink /m/c
@if \%COMPILER% == \BC5 set XLINK=%BC5_BASE%\bin\tlink /m/c
@if \%COMPILER% == \WATCOM set XLINK=..\utils\wlinker /ma/nologo
@if \%COMPILER% == \MSCL8 set XLINK=%MS_BASE%\bin\link /ONERROR:NOEXE /ma /nologo
goto link_set

:link_set   

echo linker is %XLINK%

@if not "%XUPX%" == "" goto upx_set
@set XUPX=@rem
@set UPXOPT=
goto compile

:upx_set
@set UPXOPT=-U

:compile

:************************************************************************
:* finally - we are going to compile
:************************************************************************

cd utils
%MAKE% production
if errorlevel 1 goto abort

cd ..\lib
%MAKE%
if errorlevel 1 goto abort

cd ..\drivers
%MAKE% production
if errorlevel 1 goto abort


cd ..\boot
%MAKE% production
if errorlevel 1 goto abort

cd ..\sys
%MAKE% production
if errorlevel 1 goto abort

:start

cd ..\kernel
%MAKE% production
if errorlevel 1 goto abort

cd ..

:- if you like, put some finalizing commands (like copy to floppy)
:- into build2.bat

if exist build2.bat call build2

@goto end

:abort
cd ..
set XERROR=1
:end
:***** cleanup ******
@echo off

@if "%OLDPATH%" == "" goto no_path_change
@set PATH=%OLDPATH%
@set OLDPATH=
:no_path_change

@set MAKE=
@set COMPILER=
@set XCPU=
@set XFAT=
@set XLINK=
@set TC2_BASE=
@set TP1_BASE=
@set TC3_BASE=
@set BC5_BASE=
@set MS_BASE=
@set XNASM=
@set XERROR=
@set XUPX=
@set UPXOPT=
