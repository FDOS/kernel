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

!if $(XFAT)0 == 0
XFAT=32
!endif
!if $(XFAT) == 32
ALLCFLAGS=-DWITHFAT32 $(ALLCFLAGS)
NASMFLAGS=-DWITHFAT32 $(NASMFLAGS)
!endif

NASMFLAGS=-fobj -i../hdr/ -D$(COMPILER) -DXCPU=$(XCPU) $(NASMFLAGS)

BINPATH=$(BASE)\bin
INCLUDEPATH=$(BASE)\include
LIBPATH=$(BASE)\lib
INITPATCH=@rem

!if $(LOADSEG)0 == 0
LOADSEG=0x60
!endif

UPXOPT=-U
!if $(__MAKE__)0 == 0	# NMAKE/WMAKE
!if "$(XUPX)" == ""	# TC doesn't supports this
XUPX=rem		# NMAKE doesn't supports @ in macro
UPXOPT=
!endif
!else			# TC/BC MAKE
!if !$d(XUPX)		# NMAKE/WMAKE doesn't supports $d()
XUPX=@rem
UPXOPT=
!endif
!endif

!include "..\mkfiles\$(COMPILER).mak"

TARGET=$(TARGET)$(XCPU)$(XFAT)
INITCFLAGS=$(INITCFLAGS) $(ALLCFLAGS)
CFLAGS=$(CFLAGS) $(ALLCFLAGS)
RM=..\utils\rmfiles
DEPENDS=makefile ..\*.bat ..\mkfiles\*.*

# Implicit Rules #######################################################

.asm.obj:
	$(NASM) $(NASMFLAGS) $<

.c.obj:
	$(CC) $(CFLAGS) $<

.cpp.obj:
	$(CC) $(CFLAGS) $<

.c.com:
	$(CL) $(CFLAGST) $<

.c.exe:
	$(CL) $(CFLAGSC) $<
