:-@echo off

:-  batch file to build everything

:-  $Id$

:-  $Log$
:-  Revision 1.6  2001/11/04 19:47:37  bartoldeman
:-  kernel 2025a changes: see history.txt
:-
:-  Revision 1.5  2001/07/09 22:19:30  bartoldeman
:-  LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
:- 
:-  Revision 1.4  2001/03/22 04:13:30  bartoldeman
:-  Change LF to CR/LF in batch files.
:- 
:-  Revision 1.3  2000/05/25 20:56:19  jimtabor
:-  Fixed project history
:- 
:-  Revision 1.2  2000/05/14 17:05:39  jimtabor
:-  Cleanup CRs
:- 
:-  Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
:-  The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
:-  MS-DOS.  Distributed under the GNU GPL.
:- 
:-  Revision 1.5  1999/08/25 03:59:14  jprice
:-  New build batch files.
:- 
:-  Revision 1.4  1999/08/25 03:38:16  jprice
:-  New build config
:- 
:-  Revision 1.3  1999/04/23 03:46:02  jprice
:-  Improved by jprice
:- 
:-  Revision 1.2  1999/04/17 19:13:29  jprice
:-  ror4 patches
:- 
:-  Revision 1.1.1.1  1999/03/29 15:39:13  jprice
:-  New version without IPL.SYS
:- 
:-  Revision 1.5  1999/02/09 04:47:54  jprice
:-  Make makefile use common config.mak file
:- 
:-  Revision 1.4  1999/01/30 08:29:10  jprice
:-  Clean up
:- 
:-  Revision 1.3  1999/01/30 07:49:16  jprice
:-  Clean up
:- 

set XERROR=


if not exist config.bat echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
if not exist config.bat goto end

if not \%1 == \-r goto norebuild
    del kernel\*.obj 
    del lib\libm.lib
:norebuild    


call config.bat

set XERROR=

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

cd..

:- if you like, put some finalizing commands (like copy to floppy)
:- into build2.bat

if exist build2.bat call build2

@goto end

:abort
cd ..
set XERROR=1
:end
:***** cleanup ******
@set MAKE=
@set COMPILER=
@set XCPU=
@set XFAT=
