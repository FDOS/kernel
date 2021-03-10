#
# makefile for setver.sys
#

!include "../mkfiles/generic.mak"


production:     setver.sys

setver.sys:     setver.asm
		$(NASM) -o ..$(DIRSEP)bin$(DIRSEP)setver.sys setver.asm

clobber:	clean

clean:
		-$(RM) *.obj *.map *.lst *.err

