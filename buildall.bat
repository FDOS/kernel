:-@echo off

:- 
:- Revision 1.0  2001/09/05  tomehlert
:-


:----------------------------------------------------------
:- batch file to build _many_ KERNELS, hope build works
:-
:- takes 3 minutes on my(TE) Win2K/P700. your milage may vary :-)
:----------------------------------------------------------

if \%1 == \$SUMMARY goto summary

:-goto xsummary

set onerror=if not \%XERROR% == \ goto daswarwohlnix

:***** some MSCL kernels

call config.bat

if \%MS_BASE% == \ goto no_ms
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

:***** some TC 2.01 kernels

if \%TC2_BASE% == \ goto no_tc
call build -r tc   186 fat16
%ONERROR%
call build -r tc    86 fat16
%ONERROR%
call build -r tc   186 fat32
%ONERROR%
call build -r tc    86 fat32
%ONERROR%
:no_tc

:wc

:***** some WATCOM kernels - just for fun !!!

:-
:- this is definitively only for fun - now
:- hope, this gets better
:- 
if \%WATCOM% == \ goto no_wc
call build -r wc    386 fat32
call build -r wc    386 fat16
call build -r wc     86 fat32
call build -r wc     86 fat16
:no_wc
    
:- the watcom executables will currently NOT RUN
@del bin\kwc*.sys >nul


:***** now rebuild the normal kernel !!
call build -r


:**************************************************************
:* now we build a summary of all kernels HMA size + total size
:* Yes, I know - "mit Linux waer das nicht passiert" :-)
:* at least, it's possible with standard DOS tools
:**************************************************************

:xsummary

set Sumfile=bin\ksummary.txt
set TempSumfile=bin\tsummary.txt

:****@echo  >%TempSumfile% Summary of all kernels build
:****@echo.|date  >>%TempSumfile% 
:****@echo.|time  >>%TempSumfile% 
:****for %%i in (bin\k*.map) do call %0 $SUMMARY %%i
:****for %%i in (bin\k*.map) do call %0 $SUMMARY %%i
:****for %%i in (bin\k*.map) do call %0 $SUMMARY %%i

del %Sumfile%
del %TempSumfile%
del ktemp.bat
for %%i in (bin\k*.map) do echo call %0 $SUMMARY %%i >>ktemp.bat
sort <ktemp.bat >ktemps.bat
call ktemps
del ktemp.bat
del ktemps.bat


@echo        >>%Sumfile% Summary of all kernels build
@echo.|date  >>%Sumfile% 
@echo.|time  >>%Sumfile% 
find <%TempSumfile% "H" >>%Sumfile%
del %TempSumfile% 

set TempSumfile=
set Sumfile=
goto end

:summary 
echo >>%TempSumfile% H*************************************************  %2 
type %2| find " HMA_TEXT" |find /V "HMA_TEXT_START" |find /V "HMA_TEXT_END" >>%TempSumfile%
type %2| find " STACK"    >>%TempSumfile%
goto end

:************* done with summary *********************************


:daswarwohlnix
@echo Sorry, something didn't work as expected :-(
@set ONERROR=
:end


