@echo off

:- $Id$

set LAST=
if "%1" == "clearset" goto clearset

:-----------------------------------------------------------------------

if "%COMPILER%" == "TC"     set BASE=%TC_BASE%
if "%COMPILER%" == "TCPP"   set BASE=%TCPP_BASE%
if "%COMPILER%" == "TCPP3"  set BASE=%TCPP3_BASE%
if "%COMPILER%" == "BC"     set BASE=%BC_BASE%
if "%COMPILER%" == "WATCOM" set BASE=%WATCOM%
if "%COMPILER%" == "MSC"    set BASE=%MSC_BASE%
if "%BASE%"     == ""       goto clearset

:-----------------------------------------------------------------------

if not "%LINK%" == "" goto skip_link

set LINK=%BASE%\bin\tlink /c/m
if "%COMPILER%" == "TC"     set LINK=%BASE%\tlink /c/m
if "%COMPILER%" == "WATCOM" set LINK=..\utils\wlinker /nologo
if "%COMPILER%" == "MSC"    set LINK=%BASE%\bin\link /ONERROR:NOEXE /batch

echo Linker is %LINK%

:skip_link

:-----------------------------------------------------------------------

if not "%LIBUTIL%" == "" goto skip_lib

set LIBUTIL=%BASE%\bin\tlib
set LIBTERM=
if "%COMPILER%" == "TC"     set LIBUTIL=%BASE%\tlib
if "%COMPILER%" == "WATCOM" set LIBUTIL=%BASE%\binw\wlib -q
if "%COMPILER%" == "MSC"    set LIBUTIL=%BASE%\bin\lib /nologo
if "%COMPILER%" == "MSC"    set LIBTERM=;

echo Librarian is %LIBUTIL%

:skip_lib

:-----------------------------------------------------------------------

if not "%MAKE%" == "" goto skip_make

set MAKE=%BASE%\bin\make
if "%COMPILER%" == "TC"     set MAKE=%BASE%\make
if "%COMPILER%" == "WATCOM" set MAKE=%BASE%\binw\wmake /ms /h
if "%COMPILER%" == "MSC"    set MAKE=%BASE%\bin\nmake /nologo

echo Make is %MAKE%

:skip_make

:-----------------------------------------------------------------------

set LAST=1
if "%LAST%" == "1" goto end

:-----------------------------------------------------------------------

:clearset

set NASM=
set COMPILER=
set BASE=
set TC_BASE=
set TCPP_BASE=
set TCPP3_BASE=
set BC_BASE=
set MSC_BASE=
set LINK=
set LIBUTIL=
set LIBTERM=
set MAKE=
set XUPX=
set XCPU=
set XFAT=
set ALLCFLAGS=
set LOADSEG=

if not "%OLDPATH%" == "" set PATH=%OLDPATH%
set OLDPATH=

:end
