#
# makefile for bin2c.com and sys.com
#
# $Id$
#

!include "../mkfiles/generic.mak"

CFLAGS = -I$(INCLUDEPATH) -I..\hdr -DFORSYS -DWITHFAT32 $(CFLAGST)
NASMFLAGS = -DSYS=1

#		*List Macros*

SYS_EXE_dependencies =  \
 sys.obj \
 fdkrncfg.obj \
 prf.obj \
 talloc.obj

#		*Explicit Rules*
production:     bin2c.com ..\bin\sys.com

bin2c.com:      bin2c.c
		$(CL) $(CFLAGS) $(TINY) bin2c.c

..\bin\sys.com: sys.com
		copy sys.com ..\bin

fat12com.h:	..\boot\fat12com.bin bin2c.com
		.\bin2c ..\boot\fat12com.bin fat12com.h fat12com

fat16com.h:	..\boot\fat16com.bin bin2c.com
		.\bin2c ..\boot\fat16com.bin fat16com.h fat16com

fat32chs.h:	..\boot\fat32chs.bin bin2c.com
		.\bin2c ..\boot\fat32chs.bin fat32chs.h fat32chs

fat32lba.h:	..\boot\fat32lba.bin bin2c.com
		.\bin2c ..\boot\fat32lba.bin fat32lba.h fat32lba

prf.obj:	..\kernel\prf.c
		$(CC) $(CFLAGS) ..\kernel\prf.c

fdkrncfg.obj:   fdkrncfg.c ..\hdr\kconfig.h

talloc.obj:     talloc.c

sys.com:        $(SYS_EXE_dependencies)
		$(CL) $(CFLAGST) $(TINY) $(SYS_EXE_dependencies)

clobber:	clean
		-$(RM) bin2c.com sys.com fat*.h

clean:
		-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me

#		*Individual File Dependencies*
sys.obj: sys.c ..\hdr\portab.h ..\hdr\device.h fat12com.h fat16com.h fat32chs.h fat32lba.h
		$(CC) $(CFLAGS) $*.c

