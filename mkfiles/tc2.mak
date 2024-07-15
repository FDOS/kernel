#
# TURBOC.MAK - kernel copiler options for TURBOC
#

# Use these for Turbo C 2.01

#**********************************************************************
#* TARGET    : we create a %TARGET%.sys file
#* TARGETOPT : options, handled down to the compiler
#**********************************************************************

TARGET=KTC

TARGETOPT=-1-
!if $(XCPU) == 186
TARGETOPT=-1
ALLCFLAGS=$(ALLCFLAGS) -DI186
!endif
!if $(XCPU) == 386
TARGETOPT=-3
ALLCFLAGS=$(ALLCFLAGS) -DI386
!endif

COMPILERPATH=$(TC2_BASE)
COMPILERBIN=$(COMPILERPATH)
CC=$(COMPILERBIN)\tcc -c
CL=$(COMPILERBIN)\tcc
INCLUDEPATH1=-I$(COMPILERPATH)\include
LIBUTIL=$(COMPILERBIN)\tlib
LIBPATH=$(COMPILERPATH)\lib
LIBTERM=  
LIBPLUS=+

TINY=-lt
CFLAGST=-L$(LIBPATH) -mt -a- -k- -f- -ff- -O -Z -d -w
CFLAGSC=-L$(LIBPATH) -a- -mc

# used for building the library

CLIB=$(COMPILERPATH)\lib\cs.lib
MATH_EXTRACT=*LDIV *LXMUL *LURSH *LLSH *LRSH
MATH_INSERT=+LDIV +LXMUL  +LURSH +LLSH +LRSH

#
# heavy stuff - building the kernel
# Compiler and Options for Borland C++
# ------------------------------------
#
#  -zAname       � � Code class
#  -zBname       � � BSS class
#  -zCname       � � Code segment
#  -zDname       � � BSS segment
#  -zEname       � � Far segment
#  -zFname       � � Far class
#  -zGname       � � BSS group
#  -zHname       � � Far group
#  -zPname       � � Code group
#  -zRname       � � Data segment
#  -zSname       � � Data group
#  -zTname       � � Data class
#  -zX           ��� Use default name for "X"

#
# ALLCFLAGS specified by turbo.cfg and config.mak
#
ALLCFLAGS=$(TARGETOPT) -zCHMA_TEXT $(ALLCFLAGS)
INITCFLAGS=$(ALLCFLAGS) -zCINIT_TEXT -zDIB -zRID -zTID -zBIB -zGI_GROUP -zSI_GROUP
CFLAGS=$(ALLCFLAGS)
