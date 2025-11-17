#
# WATCOM.MAK - kernel copiler options for WATCOM C 11.0c
#

# Use these for WATCOM 11.0c
COMPILERPATH=$(WATCOM)
CC=*wcc -zq
CL=wcl -zq
INCLUDEPATH=$(COMPILERPATH)\H
INCLUDE=$(COMPILERPATH)\h 
EDPATH=$(COMPILERPATH)\EDDAT

!if $(XCPU) != 186
!if $(XCPU) != 386
TARGETOPT=-0
!endif
!endif

LIBPATH=$(COMPILERPATH)\lib286
LIBUTIL=wlib -q 
LIBPLUS=
LIBTERM=

TINY=-mt
CFLAGST=-zp1-os-s-we-e3-wx-bt=DOS
CFLAGSC=-mc-zp1-os-s-we-e3-wx-bt=DOS

TARGET=kwc

# used for building the library

CLIB=$(COMPILERPATH)\lib286\dos\clibm.lib

# we use our own ones, which override these ones when linking.
#  

MATH_EXTRACT=
MATH_INSERT=


#
# heavy stuff - building  
#
# -e=<num>      set limit on number of error messages
# -ms           small memory model (small code/small data)
# -j            change char default from unsigned to signed   
#-nc=<id>      set code class name
#-nd=<id>      set data segment name
#-nm=<file>    set module name
#-nt=<id>      set name of text segment  
# -g=<id>       set code group name
# -os           -> favor code size over execution time in optimizations
# -s            remove stack overflow checks  
# -w=<num>      set warning level number 
# -we           treat all warnings as errors   
# -ze           enable extensions (i.e., near, far, export, etc.)
# -zl           remove default library information
# -zp=<num>     pack structure members with alignment {1,2,4,8,16}
# -zq           operate quietly
#
# -3		optimization for 386 - given in CONFIG.MAK, not here
#

ALLCFLAGS=-I..$(DIRSEP)hdr $(TARGETOPT) $(ALLCFLAGS) -os-s-e5-j-zl-zp1-wx-we-zgf-zff-r
INITCFLAGS=$(ALLCFLAGS)-ntINIT_TEXT-gTGROUP-ndI
CFLAGS=$(ALLCFLAGS)-ntHMA_TEXT

