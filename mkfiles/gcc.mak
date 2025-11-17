#
# GCC.MAK - kernel compiler options for ia16-elf-gcc
#

#**********************************************************************
#* TARGET    : we create a %TARGET%.sys file
#* TARGETOPT : options, handled down to the compiler
#**********************************************************************

TARGET=kgc$(XCPU)$(XFAT)
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
NASMFLAGS+=-i../hdr/ -DXCPU=$(XCPU) -felf

CC=ia16-elf-gcc -c
CL=ia16-elf-gcc
INCLUDEPATH=.

LIBUTIL=ar crs
LIBPLUS=
LIBTERM=

TINY=-mcmodel=tiny
CFLAGST=-Os -fno-strict-aliasing -fpack-struct -fcall-used-es -Wno-pointer-to-int-cast -Wno-pragmas -Wno-array-bounds -Werror -o $@
CFLAGSC=

#
# heavy stuff - building  
#
# -mcmodel=small small memory model (small code/small data)
# -Os           -> favor code size over execution time in optimizations
# -fno-strict-aliasing don't assume strict aliasing rules
# -fleading-underscore underscores leading field for DOS compiler compat
# -fno-common    no "common" variables, just BSS for uninitialized data
# -fpack-struct pack structure members
# -ffreestanding don't assume any headers
# -fcall-used-es es clobbered in function calls
# -mrtd         use stdcall calling convention
# -Wno-pointer-to-int-cast  do not warn about FP_OFF
# -Wno-pragmas  do not warn about #pragma pack
# -Werror       treat all warnings as errors
# -mfar-function-if-far-return-type treat `int __far f ();' as a far function

ALLCFLAGS+=-I../hdr $(TARGETOPT) -mcmodel=small -fleading-underscore -fno-common -fpack-struct -ffreestanding -fcall-used-es -mrtd -Wno-pointer-to-int-cast -Wno-pragmas -Werror -Os -fno-strict-aliasing -mfar-function-if-far-return-type
INITCFLAGS=$(ALLCFLAGS) -o $@
CFLAGS=$(ALLCFLAGS) -o $@

DIRSEP=/
RM=rm -f
CP=cp
ECHOTO=echo>>
ifeq ($(LOADSEG)0, 0)
LOADSEG=0x60
endif

INITPATCH=ia16-elf-objcopy --redefine-sym ___umodsi3=_init_umodsi3 --redefine-sym ___udivsi3=_init_udivsi3 --redefine-sym ___ashlsi3=_init_ashlsi3 --redefine-sym ___lshrsi3=_init_lshrsi3 --redefine-sym _printf=_init_printf --redefine-sym _sprintf=_init_sprintf --redefine-sym _execrh=_init_execrh --redefine-sym _memcpy=_init_memcpy --redefine-sym _fmemcpy=_init_fmemcpy --redefine-sym _fmemset=_init_fmemset --redefine-sym _fmemcmp=_init_fmemcmp --redefine-sym _memcmp=_init_memcmp --redefine-sym _memset=_init_memset --redefine-sym _strchr=_init_strchr --redefine-sym _strcpy=_init_strcpy --redefine-sym _fstrcpy=_init_fstrcpy --redefine-sym _strlen=_init_strlen --redefine-sym _fstrlen=_init_fstrlen --redefine-sym _open=_init_DosOpen
CLDEF=1
CLT=gcc -Wall -I../hdr -o $@
CLC=$(CLT)
LINK=$(XLINK) -Tkernel.ld -nostdlib -Wl,-Map,kernel.map -o kernel.exe $(OBJS) -Wl,--whole-archive ../drivers/device.lib -Wl,--no-whole-archive \#

.SUFFIXES: .obj .asm

#               *Implicit Rules*
.asm.obj :
	$(NASM) -D$(COMPILER) $(NASMFLAGS) -o $@ $<

.c.obj :
	$(CC) $(CFLAGS) $*.c
