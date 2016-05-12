#
# makefile for bin2c.exe and sys.com
#
# $Id: makefile 1482 2009-07-11 16:59:43Z perditionc $
#

!include "../mkfiles/generic.mak"

CFLAGS = -I$(INCLUDEPATH) -I..$(DIRSEP)hdr -DFORSYS -DWITHFAT32 $(CFLAGST)
NASMFLAGS = -DSYS=1

#		*List Macros*

SYS_EXE_dependencies =  \
 sys.obj \
 fdkrncfg.obj \
 prf.obj \
 talloc.obj

#		*Explicit Rules*
production:     bin2c.exe ../bin/sys.com

bin2c.exe:      bin2c.c
		$(CLC) bin2c.c

../bin/sys.com: sys.com
		$(CP) sys.com ..$(DIRSEP)bin

fat12com.h:	../boot/fat12com.bin bin2c.exe
		.$(DIRSEP)bin2c.exe ../boot/fat12com.bin fat12com.h fat12com

fat16com.h:	../boot/fat16com.bin bin2c.exe
		.$(DIRSEP)bin2c.exe ../boot/fat16com.bin fat16com.h fat16com

fat32chs.h:	../boot/fat32chs.bin bin2c.exe
		.$(DIRSEP)bin2c.exe ../boot/fat32chs.bin fat32chs.h fat32chs

fat32lba.h:	../boot/fat32lba.bin bin2c.exe
		.$(DIRSEP)bin2c.exe ../boot/fat32lba.bin fat32lba.h fat32lba

oemfat12.h:	../boot/oemfat12.bin bin2c.exe
		.$(DIRSEP)bin2c.exe ../boot/oemfat12.bin oemfat12.h oemfat12

oemfat16.h:	../boot/oemfat16.bin bin2c.exe
		.$(DIRSEP)bin2c.exe ../boot/oemfat16.bin oemfat16.h oemfat16

prf.obj:	../kernel/prf.c
		$(CC) $(CFLAGS) ..$(DIRSEP)kernel$(DIRSEP)prf.c

fdkrncfg.obj:   fdkrncfg.c ../hdr/kconfig.h

talloc.obj:     talloc.c

sys.com:        $(SYS_EXE_dependencies)
		$(CL) $(CFLAGST) $(TINY) $(SYS_EXE_dependencies)

clobber:	clean
		-$(RM) bin2c.exe bin2c.com sys.com fat*.h oemfat*.h

clean:
		-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me

#		*Individual File Dependencies*
sys.obj: sys.c ../hdr/portab.h ../hdr/device.h fat12com.h fat16com.h fat32chs.h fat32lba.h oemfat12.h oemfat16.h
		$(CC) $(CFLAGS) $*.c

