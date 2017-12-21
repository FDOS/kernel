#
# GCC.MAK - kernel compiler options for ia16-elf-gcc
#

#**********************************************************************
#* TARGET    : we create a %TARGET%.sys file
#* TARGETOPT : options, handled down to the compiler
#**********************************************************************

TARGET=KGC$(XCPU)$(XFAT)
TARGETOPT=-march=i8086

ifeq ($(XCPU),186)
TARGETOPT=march=i80186
ALLCFLAGS+=-DI186
endif
ifeq ($(XCPU),386)
TARGETOPT=-march=i80286
ALLCFLAGS+=-DI386
endif

ifeq ($(XFAT),32)
ALLCFLAGS+=-DWITHFAT32
NASMFLAGS+=-DWITHFAT32
endif

NASM=$(XNASM)
NASMFLAGS+=-i../hdr/ -DXCPU=$(XCPU) -felf -o $@

CC=ia16-elf-gcc -c
CL=ia16-elf-gcc
INCLUDEPATH=.

LIBUTIL=ar crs
LIBPLUS=
LIBTERM=

TINY=-mcmodel=tiny
CFLAGST=-Os -fpack-struct -fcall-used-es -w -o $@
CFLAGSC=

#
# heavy stuff - building  
#
# -mcmodel=small small memory model (small code/small data)
# -Os           -> favor code size over execution time in optimizations
# -fleading-underscore underscores leading field for DOS compiler compat
# -fno-common    no "common" variables, just BSS for uninitialized data
# -fpack-struct pack structure members
# -ffreestanding don't assume any headers
# -fcall-used-es es clobbered in function calls
# -mrtd         use stdcall calling convention
# -w            disable warnings for now
# -Werror       treat all warnings as errors

ALLCFLAGS+=-I../hdr $(TARGETOPT) -mcmodel=small -fleading-underscore -fno-common -fpack-struct -ffreestanding -fcall-used-es -mrtd -w -Werror -Os
INITCFLAGS=$(ALLCFLAGS) -o $@
CFLAGS=$(ALLCFLAGS) -o $@

DIRSEP=/
RM=rm -f
CP=cp
ECHOTO=echo>>
ifeq ($(LOADSEG)0, 0)
LOADSEG=0x60
endif

INITPATCH=objcopy --redefine-sym ___umodsi3=_init_umodsi3 --redefine-sym ___udivsi3=_init_udivsi3 --redefine-sym ___ashlsi3=_init_ashlsi3 --redefine-sym ___lshrsi3=_init_lshrsi3
CLDEF=1
CLT=gcc -DDOSC_TIME_H -I../hdr -o $@
CLC=$(CLT)
LINK=$(XLINK) -Tkernel.ld -Wl,-Map,kernel.map -o kernel.exe $(OBJS) -Wl,--whole-archive ../drivers/device.lib -Wl,--no-whole-archive \#

.SUFFIXES: .obj .asm

#               *Implicit Rules*
.asm.obj :
	$(NASM) -D$(COMPILER) $(NASMFLAGS) $*.asm

.c.obj :
	$(CC) $(CFLAGS) $*.c
