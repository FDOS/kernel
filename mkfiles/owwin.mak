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
CLT=wcl386 -zq -bcl=nt -I..\hdr -fe=$@ -I$(COMPILERPATH)\h -I$(COMPILERPATH)\h\nt
CLC=$(CLT)
NASMFLAGS=-DWATCOM $(NASMFLAGS)
#linker arguments are passed via response file, so don't modify XLINK
#XLINK=$(XLINK) debug all format dos opt quiet,symfile,map,statics,verbose F { $(OBJS) } L ..$(DIRSEP)lib$(DIRSEP)device.lib N kernel.exe $#
