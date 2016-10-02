@echo off
rem IF NOTHING COMPILES, CHECK IF YOUR CVS CHECKOUT USES CORRECT DOS LINEBREAKS

:- $Id: buildall.bat 1305 2006-10-31 21:13:02Z bartoldeman $

:----------------------------------------------------------
:- batch file to build _many_ KERNELS, hope build works.
:- takes 3 minutes on my(TE) Win2K/P700. your milage may vary :-)
:----------------------------------------------------------

if "%1" == "$SUMMARY" goto summary

set onerror=if not "%XERROR%" == "" goto daswarwohlnix

:***** MSCL kernels

call config.bat
set dos4g=quiet

if "%MS_BASE%" == "" goto no_ms
call build -r msc 386 fat16
%ONERROR%
call build -r msc 186 fat16
%ONERROR%
call build -r msc  86 fat16
%ONERROR%
call build -r msc 386 fat32
%ONERROR%
call build -r msc 186 fat32
%ONERROR%
call build -r msc  86 fat32
%ONERROR%
:no_ms

:***** TC 2.01 kernels

if "%TC2_BASE%" == "" goto no_tc
call build -r tc   186 fat16
%ONERROR%
call build -r tc    86 fat16
%ONERROR%
call build -r tc   186 fat32
%ONERROR%
call build -r tc    86 fat32
%ONERROR%
:no_tc

:***** (Open) Watcom kernels

if not "%COMPILER%" == "WATCOM" goto no_wc
call build -r wc    386 fat32
%ONERROR%
call build -r wc    386 fat16
%ONERROR%
call build -r wc     86 fat32
%ONERROR%
call build -r wc     86 fat16
%ONERROR%
:no_wc
    
:***** now rebuild the default kernel

call build -r

:**************************************************************
:* now we build a summary of all kernels HMA size + total size
:* Yes, I know - "mit Linux waer das nicht passiert" :-)
:* at least, it's possible with standard DOS tools
:**************************************************************

set Sumfile=bin\ksummary.txt
set TempSumfile=bin\tsummary.txt

:****echo  >%TempSumfile% Summary of all kernels build
:****echo.|date  >>%TempSumfile% 
:****echo.|time  >>%TempSumfile% 
:****for %%i in (bin\k*.map) do call %0 $SUMMARY %%i

if exist %Sumfile% del %Sumfile%
if exist %TempSumfile% del %TempSumfile%
>ktemp.bat
for %%i in (bin\k*.map) do echo call %0 $SUMMARY %%i >>ktemp.bat
sort <ktemp.bat >ktemps.bat
call ktemps.bat
del ktemp.bat
del ktemps.bat

echo        >>%Sumfile% Summary of all kernels build
echo.|date  >>%Sumfile% 
echo.|time  >>%Sumfile% 
find <%TempSumfile% "H" >>%Sumfile%
del %TempSumfile% 

set TempSumfile=
set Sumfile=
goto end

:summary 
echo H*************************************************  %2 >>%TempSumfile%
find<%2 " HMA_TEXT"|find/V "HMA_TEXT_START"|find/V "HMA_TEXT_END">>%TempSumfile%
find<%2 " STACK">>%TempSumfile%
goto end

:************* done with summary *********************************

:daswarwohlnix
echo Sorry, something didn't work as expected :-(
set ONERROR=

:end
