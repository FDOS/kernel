#
# MSCL8.MAK - kernel copiler options for MS CL8 = MSVC1.52
#

# Use these for MSCV 1.52
COMPILERPATH=$(MS_BASE)
COMPILERBIN=$(COMPILERPATH)\bin
INCLUDEPATH=$(COMPILERPATH)\include
CC=$(COMPILERBIN)\cl -c
CL=$(COMPILERBIN)\cl
TINY=
CFLAGST=/Fm /AT /Os /Zp1
CFLAGSC=/Fm /AL /Os /Zp1
LIBPATH=$(COMPILERPATH)\lib
LIB=$(COMPILERPATH)\lib
INCLUDE=$(COMPILERPATH)\include
LIBUTIL=$(COMPILERBIN)\lib /nologo
LIBPLUS=+
LIBTERM=;
INCLUDE=$(COMPILERPATH)\include
LIB=$(COMPILERPATH)\lib

# used for building the library

CLIB=$(COMPILERPATH)\lib\slibce.lib
MATH_EXTRACT=*aflmul *aFlshl *aFNaulsh *aFNauldi *aFulrem *aFulshr *aFuldiv *aFlrem *aFldiv
MATH_INSERT= +aflmul +aFlshl +aFNaulsh +aFNauldi +aFulrem +aFulshr +aFuldiv +aFlrem +aFldiv

TARGETOPT=
!if $(XCPU) == 186    
TARGETOPT=-G1
!endif
!if $(XCPU) == 386
TARGETOPT=-G3
!endif

TARGET=KMS

#
# heavy stuff - building


ALLCFLAGS=-I..\hdr $(TARGETOPT) $(ALLCFLAGS) -nologo -Zl -Fc -WX -Gr -f- -Os -Gs -Ob1 -OV4 -Gy -Oe -Zp1

INITCFLAGS=$(ALLCFLAGS) -NTINIT_TEXT
CFLAGS=$(ALLCFLAGS) -NTHMA_TEXT
INITPATCH = ..\utils\patchobj _DATA=IDATA DATA=ID BSS=ID DGROUP=I_GROUP CONST=IC
