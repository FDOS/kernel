#
# GCC.MAK - kernel compiler options for ia16-elf-gcc
#

CC=ia16-elf-gcc -c
CL=echo ia16-elf-gcc
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
# -fleading-underscore underscores leading field for DOS compiler compat
# -fno-common    no "common" variables, just BSS for uninitialized data
# -fpack-struct pack structure members
# -ffreestanding don't assume any headers
# -fcall-used-es es clobbered in function calls
# -mrtd         use stdcall calling convention
# -w            disable warnings for now
# -Werror       treat all warnings as errors

ALLCFLAGS=-I../hdr $(TARGETOPT) $(ALLCFLAGS) -mcmodel=small -fleading-underscore -fno-common -fpack-struct -ffreestanding -fcall-used-es -mrtd -w -Werror
INITCFLAGS=$(ALLCFLAGS) -o $@
CFLAGS=$(ALLCFLAGS) -o $@

DIRSEP=/
RM=rm -f
CP=echo cp
ECHOTO=echo>>
INITPATCH=@echo > /dev/null
CLDEF=1
CLT=gcc -DDOSC_TIME_H -I../hdr -o $@
CLC=$(CLT)
XLINK=echo $(XLINK) debug all op symfile format dos option map,statics,verbose F { $(OBJS) } L ../lib/device.lib N kernel.exe $#
