@echo off
ms2wlink %1 %2 %3 %4 %5 %6 %7 %8 %9 ,,,, > kernel.lnk
echo op map,statics,verbose >> kernel.lnk
call wlink @kernel.lnk
