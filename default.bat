@echo off

:- $Id$

if "%1" == "clearset" goto clearset

:-----------------------------------------------------------------------

if not "%MAKE%" == "" goto skip_make

if "%COMPILER%" == "TC2"      set MAKE=%TC2_BASE%\make
if "%COMPILER%" == "TURBOCPP" set MAKE=%TP1_BASE%\bin\make
if "%COMPILER%" == "TC3"      set MAKE=%TC3_BASE%\bin\make
if "%COMPILER%" == "BC5"      set MAKE=%BC5_BASE%\bin\make
if "%COMPILER%" == "WATCOM"   set MAKE=wmake/ms /h
if "%COMPILER%" == "MSCL8"    set MAKE=%MS_BASE%\bin\nmake/nologo

echo Make is %MAKE%.

:skip_make

:-----------------------------------------------------------------------

if not "%XLINK%" == "" goto skip_xlink

if "%COMPILER%" == "TC2"      set XLINK=%TC2_BASE%\tlink/m/c
if "%COMPILER%" == "TURBOCPP" set XLINK=%TP1_BASE%\bin\tlink/m/c
if "%COMPILER%" == "TC3"      set XLINK=%TC3_BASE%\bin\tlink/m/c
if "%COMPILER%" == "BC5"      set XLINK=%BC5_BASE%\bin\tlink/m/c
if "%COMPILER%" == "WATCOM"   set XLINK=..\utils\wlinker/ma/nologo
if "%COMPILER%" == "MSCL8"    set XLINK=%MS_BASE%\bin\link/ONERROR:NOEXE /ma /nologo

echo Linker is %XLINK%.

:skip_xlink

:-----------------------------------------------------------------------

if not "%XUPX%" == "" set UPXOPT=-U
if     "%XUPX%" == "" set UPXOPT=
if     "%XUPX%" == "" set XUPX=@rem

goto end

:-----------------------------------------------------------------------

:clearset

if not "%OLDPATH%" == "" set PATH=%OLDPATH%
if not "%OLDPATH%" == "" set OLDPATH=

set MAKE=
set COMPILER=
set XCPU=
set XFAT=
set XLINK=
set TC2_BASE=
set TP1_BASE=
set TC3_BASE=
set BC5_BASE=
set MS_BASE=
set XNASM=
set XUPX=
set UPXOPT=

:end
