#
# MSCL8.MAK - kernel copiler options for MS CL8 = MSVC1.52
#

# Use these for MSCV 1.52
COMPILERPATH=$(MS_BASE)
COMPILERBIN=$(COMPILERPATH)\bin
INCLUDEPATH=$(COMPILERPATH)\include
CC=$(COMPILERBIN)\cl
CFLAGST=/Fm /AT /Os
CFLAGSC=/Fm /AL /Os
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
MATH_EXTRACT=*aflmul *aFlshl *aFNauldi *aFulrem *aFulshr *aFuldiv *aFlrem *aFldiv
MATH_INSERT= +aflmul +aFlshl +aFNauldi +aFulrem +aFulshr +aFuldiv +aFlrem +aFldiv

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


ALLCFLAGS=-I..\hdr $(TARGETOPT) $(ALLCFLAGS) -nologo -Zl -Fc -Zp1 -WX -Gr -f- -Os -Gs -Ob1 -OV4 -Gy -Oe

INITCFLAGS=$(ALLCFLAGS) -NTINIT_TEXT -AT
CFLAGS=$(ALLCFLAGS) -NTHMA_TEXT
DYNCFLAGS=$(ALLCFLAGS) -NTHMA_TEXT 
PATCHOBJ=patchobj
