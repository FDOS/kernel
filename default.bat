@echo off
:- $Id: default.bat 1482 2009-07-11 16:59:43Z perditionc $

:- with option clearset, clears all config.bat-made environment variables
:- without options, MAKE / LINK / ... are set to defaults based on COMPILER ...

if "%1" == "clearset" goto clearset

:-----------------------------------------------------------------------

if not "%COMPILER%" == "" goto skip_cc

set COMPILER=WATCOM

echo No compiler specified, defaulting to Open Watcom

:skip_cc

:-----------------------------------------------------------------------

if not "%MAKE%" == "" goto skip_make

if "%COMPILER%" == "TC2"      set MAKE=%TC2_BASE%\make
if "%COMPILER%" == "TURBOCPP" set MAKE=%TP1_BASE%\bin\make
if "%COMPILER%" == "TC3"      set MAKE=%TC3_BASE%\bin\make
if "%COMPILER%" == "BC3"      set MAKE=%BC3_BASE%\bin\make
if "%COMPILER%" == "BC5"      set MAKE=%BC5_BASE%\bin\make
if "%COMPILER%" == "WATCOM"   set MAKE=wmake /ms /h
if "%COMPILER%" == "MSCL8"    set MAKE=%MS_BASE%\bin\nmake /nologo

echo Make is %MAKE%.

:skip_make

:-----------------------------------------------------------------------

if not "%XLINK%" == "" goto skip_xlink

if "%COMPILER%" == "TC2"      set XLINK=%TC2_BASE%\tlink /m/c
if "%COMPILER%" == "TURBOCPP" set XLINK=%TP1_BASE%\bin\tlink /m/c
if "%COMPILER%" == "TC3"      set XLINK=%TC3_BASE%\bin\tlink /m/c
if "%COMPILER%" == "BC3"      set XLINK=%BC3_BASE%\bin\tlink /m/c
if "%COMPILER%" == "BC5"      set XLINK=%BC5_BASE%\bin\tlink /m/c
if "%COMPILER%" == "WATCOM"   set XLINK=..\utils\wlinker /ma/nologo
if "%COMPILER%" == "MSCL8"    set XLINK=%MS_BASE%\bin\link /ONERROR:NOEXE /ma /nologo

echo Linker is %XLINK%.

:skip_xlink

:-----------------------------------------------------------------------

if not "%XUPX%" == "" set UPXOPT=-U
if     "%XUPX%" == "" set UPXOPT=

goto end

:-----------------------------------------------------------------------

:clearset

if not "%OLDPATH%" == "" set PATH=%OLDPATH%
if not "%OLDPATH%" == "" set OLDPATH=

set MAKE=
set COMPILER=
set ALLCFLAGS=
set CFLAGS=
set XCPU=
set XCPU_EX=
set XFAT=
set XLINK=
set TC2_BASE=
set TP1_BASE=
set TC3_BASE=
set BC3_BASE=
set BC5_BASE=
set MS_BASE=
set XNASM=
set NASMFLAGS=
set XUPX=
set UPXOPT=
set LOADSEG=

:end
