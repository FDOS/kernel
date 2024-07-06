#
# WATCOM.MAK - kernel compiler options for Open Watcom on Windows (cross-compile)
#

# Get definitions from watcom.mak, then override
include "../mkfiles/watcom.mak"

DIRSEP=\ 

INCLUDEPATH=$(COMPILERPATH)\h
#RM=del 2>nul
#CP=copy
#ECHOTO=echo>>
#INITPATCH=@echo > nul
CLDEF=1
CLT=wcl386 -zq -bcl=nt -DDOSC_TIME_H -I..\hdr -fe=$@ -I$(COMPILERPATH)\h -I$(COMPILERPATH)\h\nt
CLC=$(CLT)
CFLAGST=-fo=.obj $(CFLAGST)
ALLCFLAGS=-fo=.obj $(ALLCFLAGS) 
NASMFLAGS=-DWATCOM $(NASMFLAGS)
#XLINK=$(XLINK) debug all op symfile format dos option map,statics,verbose F { $(OBJS) } L ..$(DIRSEP)lib$(DIRSEP)device.lib N kernel.exe $#
