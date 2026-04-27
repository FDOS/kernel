@ECHO OFF
ECHO Running test %1
%1 ref > %2.txt
REM above create a .REF and .TXT file
%1 test > %2.out
IF [%ERRORLEVEL%]==1 GOTO failed
ECHO Success (all tests passed)
GOTO done
REM above creates a .TST and .OUT file
REM 1st arg is test.exe 2nd arg is function
:failed
ECHO at least one test failed
:done

