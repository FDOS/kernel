#
# WATCOM.MAK - kernel compiler options for Open Watcom on Linux (cross-compile)
#

# Get definitions from watcom.mak, then override
include "../mkfiles/watcom.mak"

DIRSEP=/
CC=$(CC) -fo=.obj
CL=$(CL) -fo=.obj
INCLUDEPATH=-I$(COMPILERPATH)/h
EDPATH=$(COMPILERPATH)/eddat
RM=rm -f
CP=cp
ECHOTO=echo>>
INITPATCH=@echo > /dev/null
CLDEF=1
CLT=wcl386 -zq -fo=.obj -bcl=linux -fe=$@ -I../hdr -I$(COMPILERPATH)/lh
CLC=$(CLT)
XLINK=$(XLINK) debug all format dos opt quiet,symfile,map,statics,verbose F { $(OBJS) } L ../lib/device.lib N kernel.exe $#
