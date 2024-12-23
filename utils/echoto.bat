@echo off
if "%2%3%4%5%6%7%8%9" == "" goto nothing
echo %2 %3 %4 %5 %6 %7 %8 %9 >>%1
shift
if not "%9" == "" echo echoto.bat arguments overflow
:nothing
