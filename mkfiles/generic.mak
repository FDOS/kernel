# These are generic definitions

#**********************************************************************
#* TARGET    : we create a %TARGET%.sys file
#* TARGETOPT : options, handled down to the compiler
#**********************************************************************

TARGETOPT=-1-

!if $(XCPU) == 186
TARGETOPT=-1
ALLCFLAGS=$(ALLCFLAGS) -DI186
!endif
!if $(XCPU) == 386
TARGETOPT=-3
ALLCFLAGS=$(ALLCFLAGS) -DI386
!endif

!if $(XFAT) == 32
ALLCFLAGS=$(ALLCFLAGS) -DWITHFAT32
NASMFLAGS=$(NASMFLAGS) -DWITHFAT32
!endif

NASM=$(XNASM)
NASMFLAGS   = $(NASMFLAGS) -i../hdr/ -DXCPU=$(XCPU)

LINK=$(XLINK)

INITPATCH=@rem
DIRSEP=\ #a backslash
RM=..\utils\rmfiles
CP=copy
ECHOTO=..\utils\echoto
CLDEF=0

!if $(LOADSEG)0 == 0
LOADSEG=0x60
!endif

!include "../mkfiles/$(COMPILER).mak"

!if $(CLDEF) == 0
CLT=$(CL) $(CFLAGST) $(TINY) -I$(INCLUDEPATH)
CLC=$(CL) $(CFLAGSC) -I$(INCLUDEPATH)
!endif

TARGET=$(TARGET)$(XCPU)$(XFAT)

.asm.obj :
	$(NASM) -D$(COMPILER) -f obj $(NASMFLAGS) $*.asm

#               *Implicit Rules*
.c.obj :
	$(CC) $(CFLAGS) $*.c

.cpp.obj :
	$(CC) $(CFLAGS) $*.cpp

