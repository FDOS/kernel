# These are generic definitions

# TARGET : we create a $(TARGET).sys file

!if $(XCPU)0 == 0
XCPU=86
!endif
CPUOPT=
!if $(XCPU) == 186
CPUOPT=-1
!endif
!if $(XCPU) == 386
CPUOPT=-3
!endif
# extension, if compiler supports it, specify compiler switch in XCPU_EX and set XCPU to 386
!if $(XCPU_EX)0 != 0
XCPU=386
CPUOPT=$(XCPU_EX)
!endif

!if $(XFAT)0 == 0
XFAT=32
!endif
!if $(XFAT) == 32
ALLCFLAGS=-DWITHFAT32 $(ALLCFLAGS)
NASMFLAGS=-DWITHFAT32 $(NASMFLAGS)
!endif

NASMFLAGS=-fobj -i../hdr/ -D$(COMPILER) -DXCPU=$(XCPU) $(NASMFLAGS)

#BINPATH=$(BASE)\bin
INCLUDEPATH=$(BASE)\include
LIBPATH=$(BASE)\lib
INITPATCH=@rem

!if $(LOADSEG)0 == 0
LOADSEG=0x60
!endif

!include "..\mkfiles\$(COMPILER).mak"

TARGET=$(TARGET)$(XCPU)$(XFAT)
INITCFLAGS=$(INITCFLAGS) $(ALLCFLAGS)
CFLAGS=$(CFLAGS) $(ALLCFLAGS)
RM=..\utils\rmfiles
DEPENDS=makefile ..\*.bat ..\mkfiles\*.*

# Implicit Rules #######################################################

.asm.obj:
	$(NASM) $(NASMFLAGS) -l$*.lst $<

.c.obj:
	$(CC) $(CFLAGS) $<

.cpp.obj:
	$(CC) $(CFLAGS) $<

.c.com:
	$(CL) $(CFLAGST) $<

.c.exe:
	$(CL) $(CFLAGSC) $<
