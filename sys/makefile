#
# makefile for bin2c.com and sys.com
#
# $Id$
#

!include "..\mkfiles\generic.mak"

CFLAGS = -I$(INCLUDEPATH) -I..\hdr -DFORSYS -DWITHFAT32 $(CFLAGST)
NASMFLAGS = -DSYS=1

#		*List Macros*

SYS_EXE_dependencies =  \
 sys.obj \
 fdkrncfg.obj \
 prf.obj

#		*Explicit Rules*
production:     bin2c.com ..\bin\sys.com

bin2c.com:      bin2c.c
		$(CL) $(CFLAGST) $(TINY) bin2c.c

..\bin\sys.com: sys.com
		copy sys.com ..\bin

b_fat12.h:	..\boot\b_fat12.bin bin2c.com
		.\bin2c ..\boot\b_fat12.bin b_fat12.h b_fat12

b_fat16.h:	..\boot\b_fat16.bin bin2c.com
		.\bin2c ..\boot\b_fat16.bin b_fat16.h b_fat16

b_fat32.h:	..\boot\b_fat32.bin bin2c.com
		.\bin2c ..\boot\b_fat32.bin b_fat32.h b_fat32

prf.obj:	..\kernel\prf.c
		$(CC) $(CFLAGS) ..\kernel\prf.c

fdkrncfg.obj:   fdkrncfg.c ..\hdr\kconfig.h

sys.com:        $(SYS_EXE_dependencies)
		$(CL) $(CFLAGST) $(TINY) $(SYS_EXE_dependencies)

clobber:	clean
		-$(RM) bin2c.com sys.com b_fat12.h b_fat16.h b_fat32.h

clean:
		-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me

#		*Individual File Dependencies*
sys.obj: sys.c ..\hdr\portab.h ..\hdr\device.h b_fat12.h b_fat16.h b_fat32.h
		$(CC) $(CFLAGS) $*.c

# Log: makefile,v 
#
# Revision 1.6  1999/09/20 18:34:40  jprice
# *** empty log message ***
#
# Revision 1.5  1999/08/25 03:19:51  jprice
# ror4 patches to allow TC 2.01 compile.
#
# Revision 1.4  1999/05/03 05:01:38  jprice
# no message
#
# Revision 1.3  1999/04/23 03:45:33  jprice
# Improved by jprice
#
