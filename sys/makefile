#
# makefile for bin2c.com and sys.com
#
# $Id$
#

!include "../mkfiles/generic.mak"

SYS_C=sys.c fdkrncfg.c ..\kernel\prf.c talloc.c
DEPENDS=$(DEPENDS) *.cfg

########################################################################

all:		bin2c.com ..\bin\sys.com

bin2c.com:	bin2c.c $(DEPENDS)

fat12com.h:	..\boot\fat12.bin bin2c.com
		bin2c ..\boot\fat12.bin $*.h $*

fat16com.h:	..\boot\fat16.bin bin2c.com
		bin2c ..\boot\fat16.bin $*.h $*

fat32chs.h:	..\boot\fat32chs.bin bin2c.com
		bin2c ..\boot\$*.bin $*.h $*

fat32lba.h:	..\boot\fat32lba.bin bin2c.com
		bin2c ..\boot\$*.bin $*.h $*

oemfat12.h:	..\boot\oemfat12.bin bin2c.com
		bin2c ..\boot\$*.bin $*.h $*

oemfat16.h:	..\boot\oemfat16.bin bin2c.com
		bin2c ..\boot\$*.bin $*.h $*

..\bin\sys.com:	$(SYS_C) ..\hdr\*.h fat12com.h fat16com.h fat32chs.h fat32lba.h oemfat12.h oemfat16.h
		$(CL) $(CFLAGST) -DFORSYS -DWITHFAT32 $(SYS_C)
		copy sys.com ..\bin
		del sys.com

########################################################################

clean:
		-$(RM) *.bak *.cod *.crf *.err *.las *.lst *.map *.obj *.xrf

clobber:	clean
		-$(RM) bin2c.com fat*.h status.me
