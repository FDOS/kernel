rem **********************************************************************
rem - define your MAKE type here, pick one of them
rem **********************************************************************

set MAKE=c:\tc201\make
rem set MAKE=c:\watcom\binw\wmake /ms
rem set MAKE=c:\msvc\bin\nmake /nologo

rem **********************************************************************
rem - define your COMPILER type here, pick one of them
rem **********************************************************************

set COMPILER=TC2
rem set COMPILER=TURBOCPP
rem set COMPILER=TC3
rem set COMPILER=BC5
rem set COMPILER=MSCL8

rem warning: watcom can compile but the result does not work yet.
rem set COMPILER=WATCOM

rem skip MS compiler in buildall.
rem set SKIPMS=yes
