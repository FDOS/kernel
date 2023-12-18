#
# makefile for DOS-C boot
#


!include "../mkfiles/generic.mak"

production:     fat12com.bin fat16com.bin fat32chs.bin fat32lba.bin oemfat12.bin oemfat16.bin

fat12com.bin:    boot.asm
		$(NASM) -dISFAT12 $(NASMFLAGS) boot.asm -ofat12com.bin

fat16com.bin:    boot.asm
		$(NASM) -dISFAT16 $(NASMFLAGS) boot.asm -ofat16com.bin

fat32chs.bin:    boot32.asm
		$(NASM) $(NASMFLAGS) boot32.asm -ofat32chs.bin

fat32lba.bin:    boot32lb.asm
		$(NASM) $(NASMFLAGS) boot32lb.asm -ofat32lba.bin

oemfat12.bin:   oemboot.asm
		$(NASM) -dISFAT12 $(NASMFLAGS) oemboot.asm -ooemfat12.bin

oemfat16.bin:   oemboot.asm
		$(NASM) -dISFAT16 $(NASMFLAGS) oemboot.asm -ooemfat16.bin

clobber:        clean
		-$(RM) *.bin status.me

clean:
		-$(RM) *.lst *.map *.bak *.obj

