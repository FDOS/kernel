#
# WATCOM.MAK - kernel copiler options for WATCOM C 11.0c
#

# Use these for WATCOM 11.0c
COMPILERPATH=$(WATCOM)
COMPILERBIN=$(WATCOM)\binw
CC=$(COMPILERBIN)\wcl
INCLUDEPATH=$(COMPILERPATH)\H
INCLUDE=$(COMPILERPATH)\h 
EDPATH=$(COMPILERPATH)\EDDAT

!if $(XCPU) != 186
!if $(XCPU) != 386
TARGETOPT=-0
!endif
!endif

LIBPATH=$(COMPILERPATH)\lib286
LIBUTIL=$(COMPILERBIN)\wlib 
LIBPLUS=
LIBTERM=

CFLAGST=-mt
CFLAGSC=-zp=1 -mc

TARGET=KWC

# used for building the library

CLIB=$(COMPILERPATH)\lib286\dos\clibs.lib

#
#MATH_EXTRACT=*i4d *i4m
#MATH_INSERT= +i4d +i4m
#
# these are NOT usable, as they are called NEAR, and are in TEXT segment.
# so we can't use them, when moving the kernel. called ~15 times
# 
#  so I include 1 dummy library routine (stridup()), to make lib happy
#  

MATH_EXTRACT=*_icstrdu
MATH_INSERT= +_icstrdu
MATH_EXTRACT=*i4d *i4m
MATH_INSERT= +i4d +i4m


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

ALLCFLAGS=-I..\hdr $(TARGETOPT) $(ALLCFLAGS) -zq -os -ms -s -e=5 -j -zl -zp=1
INITCFLAGS=$(ALLCFLAGS) -nt=INIT_TEXT -nc=INIT -g=IGROUP
CFLAGS=$(ALLCFLAGS) -nt=HMA_TEXT -nc=HMA -g=HGROUP
DYNCFLAGS=$(ALLCFLAGS)

