# These are generic definitions

#**********************************************************************
#* TARGET    : we create a %TARGET%.sys file
#* TARGETOPT : options, handled down to the compiler
#**********************************************************************

TARGETOPT=-1-

!if $(XCPU) == 186
TARGETOPT=-1
!endif
!if $(XCPU) == 386
TARGETOPT=-3
!endif

!if $(XFAT) == 32
ALLCFLAGS=$(ALLCFLAGS) -DWITHFAT32
NASMFLAGS=$(NASMFLAGS) -DWITHFAT32
!endif

NASM=$(XNASM)
NASMFLAGS   = $(NASMFLAGS) -i../hdr/ -DXCPU=$(XCPU)

LINK=$(XLINK)

INITPATCH=@rem

!include "..\mkfiles\$(COMPILER).mak"

TARGET=$(TARGET)$(XCPU)$(XFAT)
RM=..\utils\rmfiles

.asm.obj :
	$(NASM) -D$(COMPILER) $(NASMFLAGS) -f obj $*.asm

#               *Implicit Rules*
.c.obj :
	$(CC) $(CFLAGS) $*.c

.cpp.obj :
	$(CC) $(CFLAGS) $*.cpp

