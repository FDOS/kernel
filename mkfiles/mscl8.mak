#
# MSCL8.MAK - kernel copiler options for MS CL8 = MSVC1.52
#

# Use these for MSCV 1.52
COMPILERPATH=$(MS_BASE)
COMPILERBIN=$(COMPILERPATH)\bin
INCLUDEPATH=$(COMPILERPATH)\include
CC=$(COMPILERBIN)\cl
CFLAGST = /Fm /AS /Os ????
CFLAGSC=-a- -mc ????
LIBUTIL=$(COMPILERBIN)\lib /nologo
LIBPATH=$(COMPILERPATH)\lib
LIBUTIL=$(COMPILERBIN)\lib /nologo
LIBTERM=;


# used for building the library

CLIB=$(COMPILERPATH)\lib\slibce.lib
MATH_EXTRACT=*aflmul *aFlshl *aFNauldi *aFulrem *aFulshr *aFuldiv *aFlrem *aFldiv
MATH_INSERT= +aflmul +aFlshl +aFNauldi +aFulrem +aFulshr +aFuldiv +aFlrem +aFldiv

!if $(XCPU) == 186    
TARGETOPT=-G1
!end
!if $(XCPU) == 386
TARGETOPT=-G3
!end

TARGET=KMS

#
# heavy stuff - building


ALLCFLAGS = -I..\hdr $(TARGETOPT) $(ALLCFLAGS) -nologo -c -Zl -Fc -Zp1 -Gs -Os -WX
INITCFLAGS = $(ALLCFLAGS) -NTINIT_TEXT -AT
CFLAGS     = $(ALLCFLAGS) -NTHMA_TEXT
DYNCFLAGS = $(ALLCFLAGS) -NTHMA_TEXT 
IPRFCFLAGS = $(INITCFLAGS) -Foiprf.obj
PATCHOBJ = patchobj
