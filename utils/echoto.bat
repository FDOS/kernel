@echo off
set FILE=%1
set STR=%2

:loop
shift
if "%2" == "" goto end
set STR=%STR% %2
goto loop
:end

echo %STR%>> %FILE%
