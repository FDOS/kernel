@if not "%MAKE%" == "" goto make_set

@if \%COMPILER% == \TC2 set MAKE=%TC2_BASE%\make
@if \%COMPILER% == \TURBOCPP set MAKE=%TP1_BASE%\bin\make
@if \%COMPILER% == \TC3 set MAKE=%TC3_BASE%\bin\make
@if \%COMPILER% == \BC5 set MAKE=%BC5_BASE%\bin\make
@if \%COMPILER% == \WATCOM set MAKE=wmake /ms
@if \%COMPILER% == \MSCL8 set MAKE=%MS_BASE%\bin\nmake /nologo

:make_set
