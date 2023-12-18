#
# makefile for DOS-C boot
#


!include "../mkfiles/generic.mak"

production:     fat12com.bin fat16com.bin fat32chs.bin fat32lba.bin oemfat12.bin oemfat16.bin

fat12com.bin:    boot.asm
		$(NASM) -dISFAT12 boot.asm -l$*.lst -ofat12com.bin

fat16com.bin:    boot.asm
		$(NASM) -dISFAT16 boot.asm -l$*.lst -ofat16com.bin

fat32chs.bin:    boot32.asm
		$(NASM) boot32.asm -l$*.lst -ofat32chs.bin

fat32lba.bin:    boot32lb.asm
		$(NASM) boot32lb.asm -l$*.lst -ofat32lba.bin

oemfat12.bin:   oemboot.asm
		$(NASM) -dISFAT12 oemboot.asm -l$*.lst -ooemfat12.bin

oemfat16.bin:   oemboot.asm
		$(NASM) -dISFAT16 oemboot.asm -l$*.lst -ooemfat16.bin

clobber:        clean
		-$(RM) *.bin status.me

clean:
		-$(RM) *.lst *.map *.bak *.obj

