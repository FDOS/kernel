#
# makefile for DOS-C boot
#
# $Id$
#


!include "..\mkfiles\generic.mak"

production:     b_fat12.bin b_fat16.bin b_fat32.bin

b_fat12.bin:    boot.asm
		$(NASM) -dISFAT12 boot.asm -ob_fat12.bin

b_fat16.bin:    boot.asm
		$(NASM) -dISFAT16 boot.asm -ob_fat16.bin

b_fat32.bin:    boot32.asm
		$(NASM) boot32.asm -ob_fat32.bin

clobber:        clean
		-$(RM) b_fat12.bin b_fat16.bin b_fat32.bin status.me

clean:
		-$(RM) *.lst *.map *.bak *.obj

