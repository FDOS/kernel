#
# WATCOM.MAK - kernel compiler options for Open Watcom on Windows (cross-compile)
#

# Get definitions from watcom.mak, then override
include "../mkfiles/watcom.mak"

DIRSEP=\ 

#RM=del 2>nul
#CP=copy
#ECHOTO=echo>>
#INITPATCH=@echo > nul
CLDEF=1
CLT=wcl386 -zq -bcl=nt -fe=$@ -I..\hdr -I$(COMPILERPATH)\h -I$(COMPILERPATH)\h\nt
CLC=$(CLT)
NASMFLAGS=-DWATCOM $(NASMFLAGS)
XLINK=$(XLINK) debug all format dos opt quiet,symfile,map,statics,verbose F { $(OBJS) } L ..$(DIRSEP)lib$(DIRSEP)device.lib N kernel.exe $#
