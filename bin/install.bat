@echo off
rem
rem Create a distribution floppy
rem
rem $Header$

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

