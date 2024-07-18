#
# WATCOM.MAK - kernel compiler options for Open Watcom on Linux (cross-compile)
#

# Get definitions from watcom.mak, then override
include "../mkfiles/watcom.mak"

DIRSEP=/
CC=$(CC) -fo=.obj
CL=$(CL) -fo=.obj
INCLUDEPATH=$(COMPILERPATH)/h
RM=rm -f
CP=cp
ECHOTO=echo>>
INITPATCH=@echo > /dev/null
CLDEF=1
CLT=wcl386 -zq -fo=.obj -bcl=linux -I../hdr -fe=$@ -I$(COMPILERPATH)/lh
CLC=$(CLT)
