:-
:- batch file that is included in all other batch files for configuration
:-

:-****************************************************************
:-  NOTICE!  You must edit and rename this file to CONFIG.BAT!   *
:-****************************************************************

:-*********************************************************************
:- determine your compiler settings
:- 
:- you have to
:-   search for XNASM    - and set the path for NASM
:-   search for COMPILER - and set your compiler
:-   search for ??_BASE  - and set the path to your compiler
:- 
:-*********************************************************************

:-**********************************************************************
:-- define where to find NASM - remember - it should not be protected
:-  mode DJGPP version if you're using Windows NT/2k/XP to compile
:-  also: DJGPP-nasm crashes when using protected mode Borland's make
:-**********************************************************************

set XNASM=c:\bin\nasm16

:**********************************************************************
:- define your COMPILER type here, pick one of them
:**********************************************************************

:- Turbo C 2.01
set COMPILER=TC2
:- Turbo C++ 1.01
:- set COMPILER=TURBOCPP
:- Turbo C 3.0
:- set COMPILER=TC3
:- Borland C
:- set COMPILER=BC5
:- Microsoft C
:- set COMPILER=MSCL8

:- warning: watcom can compile but the result does not work yet.
:- set COMPILER=WATCOM

:-**********************************************************************
:-- where is the BASE dir of your compiler(s) ??
:-**********************************************************************
						
set TC2_BASE=c:\tc201
:- set TP1_BASE=c:\tcpp
:- set TC3_BASE=c:\tc3
:- set BC5_BASE=c:\bc5
:- set MS_BASE=c:\msvc

:- if WATCOM maybe you need to set your WATCOM environment variables 
:- and path
:- if not %WATCOM% == \ goto watcom_defined
:- set WATCOM=c:\watcom
:- set PATH=%PATH%;%WATCOM%\binw
:watcom_defined

:-**********************************************************************
:- (optionally) which linker to use:
:- (otherwise will be determined automatically)
:- WATCOM wlink is not (yet) suitable for linking
:- (the map file and syntax are not compatible)
:- Turbo C 2.01 TLINK 2.0 can't link WATCOM (but can link TC2) 
:- Turbo C++ 1.01 and higher TLINK 3.01+ are ok
:- or get TLINK 4 (creates nice map file) from simtel at
:- ftp://ftp.simtel.net/pub/simtelnet/msdos/borland/tlink4.zip 
:-
:- WARNING TLINK needs to be in your PATH!
:-**********************************************************************

:- Turbo Link
:- set XLINK=%TC2_BASE%\tlink /m/c
:- Microsoft Link
:- set XLINK=%MS_BASE%\bin\link /ONERROR:NOEXE /ma /nologo

:- set path for Turbo Link - use OLDPATH to restore normal path
:- set OLDPATH=%PATH%
:- set PATH=%PATH%;%TC2_BASE%

:**********************************************************************
:* optionally define your MAKE type here, if not then
:* it will be automatically determined, pick one of them
:* use MS nmake if you want to compile with MSCL
:**********************************************************************

:- Borland MAKE
:- set MAKE=%TC2_BASE%\make
:- Watcom MAKE in MS mode
:- set MAKE=%WATCOM%\binw\wmake /ms
:- Microsoft MAKE
:- set MAKE=%MS_BASE%\bin\nmake /nologo

:**********************************************************************
:* select your default target: required CPU and what FAT system to support
:**********************************************************************

set XCPU=86
:- set XCPU=186
:- set XCPU=386

set XFAT=16
:- set XFAT=32

:- Give extra compiler DEFINE flags here
:- such as -DDEBUG : extra DEBUG output
:-         -DDOSEMU : printf output goes to dosemu log
:- set ALLCFLAGS=-DDEBUG


:-
:- $Id$
:-
:- Log: config.b,v 
:-
:- Revision 1.3  1999/09/13 20:40:17  jprice
:- Added COMPILER variable
:-
:- Revision 1.2  1999/08/25 03:59:14  jprice
:- New build batch files.
:-
:- Revision 1.1  1999/08/25 03:20:39  jprice
:- ror4 patches to allow TC 2.01 compile.
