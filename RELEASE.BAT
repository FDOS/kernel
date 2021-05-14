@ECHO OFF
IF "%1"=="" GOTO USAGE
REM assume ran in root directory of kernel checkout, e.g. C:\fdos\source\kernel\

ECHO tag git with release version - 
git tag -a -m "Tag kernel release %1" ke%1 HEAD
ECHO get a clean tree
if EXIST ..\SOURCE RMDIR /S /Q ..\SOURCE > NUL
::git clone -v --local --branch ke%1 . ..\SOURCE\ke%1\
git clone -v --local . ..\SOURCE\ke%1\

REM delete files to exclude
RD /S /Q ..\SOURCE\ke%1\.git > NUL
RD /S /Q ..\SOURCE\ke%1\.github > NUL
DEL /Q ..\SOURCE\ke%1\.git* > NUL
DEL /Q ..\SOURCE\ke%1\*.yml > NUL
DEL /Q ..\SOURCE\ke%1\ci*.sh > NUL
DEL /Q ..\SOURCE\ke%1\docs\*.yml > NUL
DEL /Q ..\SOURCE\ke%1\docs\CNAME > NUL
pause

ECHO %CD%
CALL MAKEPKGS.BAT %1 RELEASE
GOTO DONE

:USAGE
ECHO Tag and build release kernels - usage: RELEASE {VERSION} e.g. RELEASE 2039
:DONE
ECHO Please git push the tag to origin and upload the archives.
ECHO E.g. git push origin ke%1
