#
# makefile for DOS-C boot
#
# $Id$
#


!include "..\mkfiles\generic.mak"

production:     fat12com.bin fat16com.bin fat32chs.bin fat32lba.bin

fat12com.bin:    boot.asm
		$(NASM) -dISFAT12 boot.asm -ofat12com.bin

fat16com.bin:    boot.asm
		$(NASM) -dISFAT16 boot.asm -ofat16com.bin

fat32chs.bin:    boot32.asm
		$(NASM) boot32.asm -ofat32chs.bin

fat32lba.bin:    boot32lb.asm
		$(NASM) boot32lb.asm -ofat32lba.bin

clobber:        clean
		-$(RM) *.bin status.me

clean:
		-$(RM) *.lst *.map *.bak *.obj

