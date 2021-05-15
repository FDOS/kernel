@ECHO OFF
IF "%1"=="" GOTO USAGE
REM assumes ran in same directory as this file, ie base of kernel source
SET BASEPATH=%CD%
CD ..
REM setup command aliases
SET ZIPIT=7z.exe a -tzip -mx9 -mm=deflate -mpass15 -r
REM when called from RELEASE.BAT assume clean source exported from version control
IF "%2"=="RELEASE" GOTO PACK

ECHO create source copy
if EXIST SOURCE RMDIR /S /Q SOURCE > NUL
MKDIR SOURCE
ECHO \.svn>SOURCE\SKIPLIST
ECHO .git>>SOURCE\SKIPLIST
ECHO .yml>>SOURCE\SKIPLIST
ECHO CNAME>>SOURCE\SKIPLIST
XCOPY %BASEPATH% SOURCE\ke%1 /S /V /I /Q /G /H /R /Y /EXCLUDE:SOURCE\SKIPLIST
DEL SOURCE\SKIPLIST > NUL
ECHO ensuring clean
PUSHD .
CD SOURCE\ke%1
CALL clobber.bat
POPD

:PACK
SET VERSION=%1
SET LSMRET=SRC
SET LSMFILE=SOURCE\ke%1\docs\fdkernel.lsm
GOTO LSM
:SRC
REM remove CONFIG.BAT if exists, such as building packages from develoment directory
IF EXIST SOURCE\ke%1\CONFIG.BAT DEL SOURCE\ke%1\CONFIG.BAT > NUL
ECHO zipping source
%ZIPIT% ke%1s.zip SOURCE\*
ECHO gathering source and creating APPINFO and expected packaging dir structure
REM FreeDOS package format, has APPINFO, BIN, DOC\KERNEL, SOURCE\KERNEL\
MKDIR PACKAGE
MKDIR PACKAGE\SOURCE
XCOPY /S  /I /Q SOURCE\ke%1 PACKAGE\SOURCE\KERNEL
MKDIR PACKAGE\DOC
MKDIR PACKAGE\DOC\KERNEL
COPY /N SOURCE\ke%1\DOCS\* PACKAGE\DOC\KERNEL\
COPY SOURCE\ke%1\COPYING PACKAGE\DOC\KERNEL\COPYING
MKDIR PACKAGE\APPINFO
MOVE PACKAGE\DOC\KERNEL\fdkernel.lsm PACKAGE\APPINFO\KERNEL.LSM
MKDIR PACKAGE\BIN
ECHO using working configuration file
COPY %BASEPATH%\CONFIG.BAT SOURCE\ke%1\ > NUL
CD SOURCE\ke%1

ECHO build and packaging
SET VERSION=%1 (FAT12/FAT16)
SET FAT=16
SET CPU=86
SET BZKRET=F16
GOTO BZK
:F16
SET VERSION=%1 (FAT12/FAT16/FAT32)
SET FAT=32
SET CPU=386
SET BZKRET=F32
GOTO BZK
:F32
ECHO building FreeDOS package
CD ..\..\PACKAGE
%ZIPIT% ..\kernel.zip APPINFO BIN DOC SOURCE
ECHO clean up
CD ..
RMDIR /S /Q SOURCE > NUL
RMDIR /S /Q PACKAGE > NUL
ECHO Done.
SET BZKRET=
GOTO DONE


:BZK
ECHO build kernel %VERSION%
CALL build.bat /D KERNEL_VERSION /V "%1 " %CPU% win upx fat%FAT%
DEL BIN\K*86??.sys
SET LSMRET=BZK_2
SET LSMFILE=docs\fdkernel.lsm
GOTO LSM
:BZK_2
SET LSMRET=
ECHO zipping FAT%FAT% release version
%ZIPIT% ..\..\ke%1_%CPU%f%FAT%.zip BIN\* DOCS\*
ECHO storing binaries for FreeDOS package
COPY BIN\KERNEL.SYS ..\..\PACKAGE\BIN\KERNL%CPU%.SYS
COPY /B /Y BIN\country.sys  ..\..\PACKAGE\BIN\COUNTRY.SYS
COPY /B /Y BIN\setver.sys  ..\..\PACKAGE\BIN\SETVER.SYS
COPY /B /Y BIN\sys.com  ..\..\PACKAGE\BIN\SYS.COM
IF EXIST BIN\share.com COPY /B /Y BIN\share.com  ..\..\PACKAGE\BIN\SHARE.COM
ECHO cleaning up between builds
CALL clobber.bat
GOTO %BZKRET%

:LSM
ECHO Begin3>%LSMFILE%
ECHO Title:          The FreeDOS Kernel>>%LSMFILE%
ECHO Version:        %VERSION%>>%LSMFILE%
ECHO Entered-date:   %DATE%>>%LSMFILE%
ECHO Description:    The FreeDOS Kernel>>%LSMFILE%
ECHO Keywords:       kernel, FreeDOS, DOS, MSDOS>>%LSMFILE%
ECHO Author:         (developers: can be reached on the freedos-kernel mailing list)>>%LSMFILE%
ECHO Maintained-by:  freedos-kernel@lists.sourceforge.net>>%LSMFILE%
ECHO Primary-site:   http://github.com/fdos/kernel>>%LSMFILE%
ECHO Alternate-site: http://www.fdos.org/kernel/>>%LSMFILE%
ECHO Alternate-site: http://freedos.sourceforge.net/kernel/>>%LSMFILE%
ECHO Original-site:  http://www.gcfl.net/pub/FreeDOS/kernel>>%LSMFILE%
ECHO Platforms:      DOS, FreeDOS, DOSEMU (OpenWatcom/Borland/GCC-ia16, NASM, UPX)>>%LSMFILE%
ECHO Copying-policy: GPL2>>%LSMFILE%
ECHO End>>%LSMFILE%
SET LSMFILE=
SET VERSION=
GOTO %LSMRET%

:USAGE
ECHO Build kernel packages (interim builds or release without tagging) - usage: %0 {VERSION} e.g. %0 2039 or %0 git
:DONE
CD %BASEPATH%
SET BASEPATH=
SET ZIPIT=
