#
# makefile for DOS-C boot
#
# $Id$
#

!include "../mkfiles/generic.mak"

########################################################################

all:		fat12.bin fat16.bin fat32chs.bin fat32lba.bin

fat12.bin:	boot.asm $(DEPENDS)
		$(NASM) -DISFAT12 boot.asm -o$*.bin

fat16.bin:	boot.asm $(DEPENDS)
		$(NASM) -DISFAT16 boot.asm -o$*.bin

fat32chs.bin:	boot32.asm $(DEPENDS)
		$(NASM) boot32.asm -o$*.bin

fat32lba.bin:	boot32lb.asm $(DEPENDS)
		$(NASM) boot32lb.asm -o$*.bin

########################################################################

clean:
		-$(RM) *.bak *.cod *.crf *.err *.las *.lst *.map *.obj *.xrf

clobber:	clean
		-$(RM) *.bin status.me
