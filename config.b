@echo off

:- batch file that is included in all other batch files for configuration
:- $Id$

:-----------------------------------------------------------------------
:- NOTICE! You must edit and rename this file to CONFIG.BAT!
:-----------------------------------------------------------------------

:- determine compiler(s) settings.
:-
:- you REQUIRED to
:-  search for NASM	- and set the path to NASM
:-  search for COMPILER	- and set the default compiler name
:-  search for ??_BASE	- and set the path to (all) compiler(s)

set LAST=

:-----------------------------------------------------------------------
:- define NASM executable. It should not be protected mode DJGPP
:- version if you're using Windows NT/2k/XP to compile. also:
:- NASM/DJGPP crashes when using protected mode Borland's make.

set NASM=c:\bin\nasm16
::set NASM=c:\bin\nasmw.exe

:-----------------------------------------------------------------------
:- define COMPILER name here, pick one of them.

:- Turbo C 2.01
:: set COMPILER=TC
:- Turbo C++ 1.01
::set COMPILER=TCPP
:- Turbo C++ 3.0
::set COMPILER=TCPP3
:- Borland C
::set COMPILER=BC
:- Microsoft C
::set COMPILER=MSC
:- Watcom C
set COMPILER=WATCOM

:-----------------------------------------------------------------------
:- define BASE dir of compiler;
:- may be defined for all installed compilers.

::set TC_BASE=c:\tc
::set TCPP_BASE=c:\tcpp
::set TCPP3_BASE=c:\tcpp3
::set BC_BASE=c:\bc
::set MSC_BASE=c:\msc
set WATCOM=c:\watcom

:-----------------------------------------------------------------------
:- define which linker to use OR it will be determined AUTOMATICALLY.

:- Turbo Link
::set LINK=tlink /c/m/s/l
:- Microsoft Link
::set LINK=link /ONERROR:NOEXE /nologo
:- WATCOM Link (wlinker is a batch file calling ms2wlink and wlink)
::set LINK=..\utils\wlinker /nologo

:-----------------------------------------------------------------------
:- define which librarian to use OR it will be determined AUTOMATICALLY.

:- Turbo Lib
::set LIBUTIL=tlib
::set LIBTERM=
:- Microsoft Lib
::set LIBUTIL=lib /nologo
::set LIBTERM=;
:- WATCOM Lib
::set LIBUTIL=wlib -q
::set LIBTERM=

:-----------------------------------------------------------------------
:- define which MAKE to use OR it will be determined AUTOMATICALLY.

:- Borland MAKE
::set MAKE=make
::set MAKE=maker -S
:- Watcom MAKE in MS mode
::set MAKE=wmake /ms
:- Microsoft MAKE
::set MAKE=nmake /nologo
::set MAKE=nmaker /nologo

:-----------------------------------------------------------------------
:- This section can still be used if you need special consideration
:- for UPX, such as it is not in your PATH or you do not want
:- 8086 compatible settings, otherwise the recommended use is now
:- to add 'upx' option to the build.bat command line

:- where is UPX and which options to use
:- (comment this out if you don't want to use it)

::set XUPX=upx --8086 --best

:-----------------------------------------------------------------------
:- select default target: CPU type (default is 86) and
:- what FAT system (default is 32) to support
:- NOTE: Turbo C doesn't support 386 CPU.

::set XCPU=86
::set XCPU=186
::set XCPU=386

::set XFAT=16
::set XFAT=32

:- Give extra compiler DEFINE flags here
:- such as -DDEBUG : extra DEBUG output
:-         -DDOSEMU : printf output goes to dosemu log
:-         -DWIN31SUPPORT : extra Win3.x API support
::set ALLCFLAGS=-DDEBUG

:-----------------------------------------------------------------------

set LAST=1
if not "%LAST%" == "1" defaults.bat clearset
