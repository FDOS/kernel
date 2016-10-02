@echo off
:loop_commandline

if \%1 == \ goto done_with_commandline
if exist %1 del %1>nul
shift
goto loop_commandline

:done_with_commandline

