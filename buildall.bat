@echo off

:----------------------------------------------------------
:- batch file to build _many_ KERNELS
:----------------------------------------------------------

if "%1" == "$SUMMARY" goto summary

call config.bat
if "%LAST%" == "" goto end

:***** MSVC kernels

if "%MSC_BASE%" == "" goto no_ms
		    call build.bat -r msc 386 fat16
if "%XERROR%" == "" call build.bat -r msc 186 fat16
if "%XERROR%" == "" call build.bat -r msc  86 fat16
if "%XERROR%" == "" call build.bat -r msc 386 fat32
if "%XERROR%" == "" call build.bat -r msc 186 fat32
if "%XERROR%" == "" call build.bat -r msc  86 fat32

if not "%XERROR%" == "" goto daswarwohlnix
:no_ms

:***** TC 2.01 kernels

if "%TC_BASE%" == "" goto no_tc
		    call build.bat -r tc 186 fat16
if "%XERROR%" == "" call build.bat -r tc  86 fat16
if "%XERROR%" == "" call build.bat -r tc 186 fat32
if "%XERROR%" == "" call build.bat -r tc  86 fat32

if not "%XERROR%" == "" goto daswarwohlnix
:no_tc

:***** TCPP kernels

if "%TCPP_BASE%" == "" goto no_tcpp
		    call build.bat -r tcpp 186 fat16
if "%XERROR%" == "" call build.bat -r tcpp  86 fat16
if "%XERROR%" == "" call build.bat -r tcpp 186 fat32
if "%XERROR%" == "" call build.bat -r tcpp  86 fat32

if not "%XERROR%" == "" goto daswarwohlnix
:no_tcpp

:***** BC kernels

if "%BC_BASE%" == "" goto no_bc
		    call build.bat -r bc 386 fat16
if "%XERROR%" == "" call build.bat -r bc 186 fat16
if "%XERROR%" == "" call build.bat -r bc  86 fat16
if "%XERROR%" == "" call build.bat -r bc 386 fat32
if "%XERROR%" == "" call build.bat -r bc 186 fat32
if "%XERROR%" == "" call build.bat -r bc  86 fat32

if not "%XERROR%" == "" goto daswarwohlnix
:no_bc

:***** (Open) Watcom kernels

if "%WATCOM%" == "" goto no_wc
		    call build.bat -r wc 386 fat32
if "%XERROR%" == "" call build.bat -r wc 386 fat16
if "%XERROR%" == "" call build.bat -r wc  86 fat32
if "%XERROR%" == "" call build.bat -r wc  86 fat16

if not "%XERROR%" == "" goto daswarwohlnix
:no_wc

:***** now rebuild the default kernel

call build.bat -r
if not "%XERROR%" == "" goto daswarwohlnix

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

if exist %Sumfile% del %Sumfile%>nul
if exist %TempSumfile% del %TempSumfile%>nul
>ktemp.bat
for %%i in (bin\k*.map) do echo call %0 $SUMMARY %%i >>ktemp.bat
sort <ktemp.bat >ktemps.bat
call ktemps.bat
del ktemp.bat>nul
del ktemps.bat>nul

echo        >>%Sumfile% Summary of all kernels build
echo.|date  >>%Sumfile%
echo.|time  >>%Sumfile%
find <%TempSumfile% "H" >>%Sumfile%
del %TempSumfile%>nul

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

:end
