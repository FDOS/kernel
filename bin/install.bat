@echo off
rem
rem Create a distribution floppy
rem
rem $Header$
rem $Log$
rem Revision 1.2  2000/05/11 03:51:37  jimtabor
rem Clean up and Release
rem
rem Revision 1.3  1999/08/25 03:15:33  jprice
rem ror4 patches to allow TC 2.01 compile.
rem
rem Revision 1.2  1999/04/01 07:22:58  jprice
rem no message
rem
rem Revision 1.1.1.1  1999/03/29 15:40:21  jprice
rem New version without IPL.SYS
rem
rem

set D=A:
if "%1" == "b:" set D=B:
if "%1" == "B:" set D=B:
if "%1" == "b" set D=B:
if "%1" == "B" set D=B:

echo This utility will create a distribution floppy on the disk in drive %D%
pause

rem try to transfer system files -- abort if it cannot.
sys %D%
if errorlevel 1 goto out

rem copy remaining files
echo copying remaining files...
echo copying autoexec.bat...
copy autoexec.bat %D%
echo copying config.sys..
copy config.sys %D%
echo copying sys.com..
copy sys.com %D%
label %D% freedos

rem exit methods
goto done
:out
echo Floppy creation aborted
:done
set D=
