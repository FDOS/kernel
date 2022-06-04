#
# WATCOM.MAK - kernel compiler options for Open Watcom on Windows (cross-compile)
#

# Get definitions from watcom.mak, then override
include "../mkfiles/watcom.mak"

DIRSEP=\ 

INCLUDEPATH=$(COMPILERPATH)$(DIRSEP)h
#RM=del 2>nul
#CP=copy
#ECHOTO=echo>>
#INITPATCH=@echo > nul
CLDEF=1
CLT=owcc -DDOSC_TIME_H -DBUILD_UTILS -I../hdr -o $@
CLC=$(CLT)
CFLAGST=-fo=.obj $(CFLAGST)
ALLCFLAGS=-fo=.obj $(ALLCFLAGS) 
NASMFLAGS=-DWATCOM $(NASMFLAGS)
#XLINK=$(XLINK) debug all op symfile format dos option map,statics,verbose F { $(OBJS) } L ..$(DIRSEP)lib$(DIRSEP)device.lib N kernel.exe $#
