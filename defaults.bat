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

set BINPATH=%BASE%\bin
if "%COMPILER%" == "TC"     set BINPATH=%BASE%
if "%COMPILER%" == "WATCOM" set BINPATH=%BASE%\binw
if "%COMPILER%" == "WATCOM" if "%OS%" == "Windows_NT" set BINPATH=%BASE%\binnt

echo Path to compiler programs (binaries) is %BINPATH%

:-----------------------------------------------------------------------
:- When compiling executable, compilers may invoke secondary programs
:- such as preprocessor, compiler component, or linker through PATH;

set OLDPATH=%PATH%
set PATH=%BINPATH%;%PATH%

:- MSC searches libraries only through LIB variable.
if "%COMPILER%" == "MSC" set LIB=%MSC_BASE%\lib

:-----------------------------------------------------------------------

if not "%LINK%" == "" goto skip_link

set LINK=%BINPATH%\tlink /c/m/s/l
if "%COMPILER%" == "WATCOM" set LINK=..\utils\wlinker /nologo
if "%COMPILER%" == "MSC"    set LINK=%BINPATH%\link /ONERROR:NOEXE /batch

echo Linker is %LINK%

:skip_link

:-----------------------------------------------------------------------

if not "%LIBUTIL%" == "" goto skip_lib

set LIBUTIL=%BINPATH%\tlib
set LIBTERM=
if "%COMPILER%" == "WATCOM" set LIBUTIL=%BINPATH%\wlib -q
if "%COMPILER%" == "MSC"    set LIBUTIL=%BINPATH%\lib /nologo
if "%COMPILER%" == "MSC"    set LIBTERM=;

echo Librarian is %LIBUTIL%

:skip_lib

:-----------------------------------------------------------------------

if not "%MAKE%" == "" goto skip_make

set MAKE=%BINPATH%\make
if "%COMPILER%" == "WATCOM" set MAKE=%BINPATH%\wmake /ms /h
if "%COMPILER%" == "MSC"    set MAKE=%BINPATH%\nmake /nologo

echo Make is %MAKE%

:skip_make

:-----------------------------------------------------------------------

set LAST=1
if "%LAST%" == "1" goto end

:-----------------------------------------------------------------------

:clearset

set NASM=
set NASMFLAGS=
set COMPILER=
set BASE=
set BINPATH=
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
set XCPU_EX=
set XFAT=
set ALLCFLAGS=
set LOADSEG=

if not "%OLDPATH%" == "" set PATH=%OLDPATH%
set OLDPATH=

:end
