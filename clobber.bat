@echo off
rem batch file to clobber everything

rem $Id$

rem $Log$
rem Revision 1.1  2000/05/06 19:34:02  jhall1
rem Initial revision
rem
rem Revision 1.3  1999/08/25 03:59:14  jprice
rem New build batch files.
rem
rem Revision 1.2  1999/08/10 18:34:06  jprice
rem case
rem
rem Revision 1.1  1999/04/23 03:47:19  jprice
rem Initial include
rem

if not exist config.bat goto noconfigbat
if not exist config.mak goto noconfigmak
goto start

:noconfigbat
echo You must copy CONFIG.B to CONFIG.BAT and edit it to reflect your setup!
goto end

:noconfigmak
echo You must copy CONFIG.M to CONFIG.MAK and edit it to reflect your setup!
goto end

:start
call config.bat

cd lib
%MAKE% -flibm.mak clobber

cd ..\drivers
%MAKE% -fdevice.mak clobber

cd ..\boot
%MAKE% -fboot.mak clobber

cd ..\sys
%MAKE% -fbin2c.mak clobber
%MAKE% -fsys.mak clobber

cd ..\kernel
%MAKE% -fkernel.mak clobber

cd ..\hdr
del *.bak

cd ..

del *.bak
del status.me

:end
set MAKE=
