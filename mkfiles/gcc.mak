#
# GCC.MAK - kernel compiler options for ia16-elf-gcc
#

CC=ia16-elf-gcc -c
CL=ia16-elf-gcc
INCLUDEPATH=.

!if $(XCPU) != 186
!if $(XCPU) != 386
TARGETOPT=-march=i8086
!endif
!endif

LIBUTIL=wlib -q
LIBPLUS=
LIBTERM=

TINY=-mcmodel=tiny
CFLAGST=-w -o $@
CFLAGSC=

TARGET=KGC

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

ALLCFLAGS=-I../hdr $(TARGETOPT) $(ALLCFLAGS) -mcmodel=small -fleading-underscore -fno-common -fpack-struct -ffreestanding -fcall-used-es -mrtd -w -Werror -Os
INITCFLAGS=$(ALLCFLAGS) -o $@
CFLAGS=$(ALLCFLAGS) -o $@
NASMFLAGS=$(NASMFLAGS) -f elf -o $@

DIRSEP=/
RM=rm -f
CP=cp
ECHOTO=echo>>
INITPATCH=objcopy --redefine-sym ___umodsi3=_init_umodsi3 --redefine-sym ___udivsi3=_init_udivsi3 --redefine-sym ___ashlsi3=_init_ashlsi3 --redefine-sym ___lshrsi3=_init_lshrsi3
CLDEF=1
CLT=gcc -DDOSC_TIME_H -I../hdr -o $@
CLC=$(CLT)
XLINK=$(CL) -Tkernel.ld -Wl,-Map,kernel.map -o kernel.exe $(OBJS) -Wl,--whole-archive ../drivers/device.lib -Wl,--no-whole-archive $#
