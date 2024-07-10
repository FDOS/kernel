# These are generic definitions

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
INCLUDEPATH2=$(INCLUDEPATH1)
CLT=$(CL) $(CFLAGST) $(TINY)
CLC=$(CL) $(CFLAGSC)
!endif

TARGET=$(TARGET)$(XCPU)$(XFAT)

.asm.obj :
	$(NASM) -D$(COMPILER) -f obj $(NASMFLAGS) $*.asm

#               *Implicit Rules*
.c.obj :
	$(CC) $(CFLAGS) $*.c

.cpp.obj :
	$(CC) $(CFLAGS) $*.cpp

