#
# makefile for sys.com
#
# $Id$
#
# $Log$
# Revision 1.4  2001/03/21 02:56:26  bartoldeman
# See history.txt for changes. Bug fixes and HMA support are the main ones.
#
# Revision 1.3  2000/05/25 20:56:23  jimtabor
# Fixed project history
#
# Revision 1.2  2000/05/15 05:28:09  jimtabor
# Cleanup CRs
#
# Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
# The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
# MS-DOS.  Distributed under the GNU GPL.
#
# Revision 1.10  1999/09/23 04:41:43  jprice
# *** empty log message ***
#
# Revision 1.9  1999/09/14 17:30:44  jprice
# Added debug log creation to sys.com.
#
# Revision 1.8  1999/08/25 03:19:51  jprice
# ror4 patches to allow TC 2.01 compile.
#
# Revision 1.7  1999/05/03 05:01:54  jprice
# no message
#
# Revision 1.6  1999/04/23 03:45:33  jprice
# Improved by jprice
#


!include "..\config.mak"

#CFLAGS = -mt -1- -v -vi- -k- -f- -ff- -O -Z -d -I$(INCLUDEPATH);..\hdr \
#	 -L$(LIBPATH) -DI86;PROTO;DEBUG
CFLAGS = -mt -1- -v -vi- -k- -f- -ff- -O -Z -d -I$(INCLUDEPATH);..\hdr \
	 -L$(LIBPATH) -DI86;PROTO -zAHMA -zCHMA_TEXT -zDHMA_TEXT

#               *Implicit Rules*
.c.obj:
  $(CC) $(CFLAGS) -c $<

.cpp.obj:
  $(CC) $(CFLAGS) -c $<

#		*List Macros*

LIBS =  ..\lib\device.lib

EXE_dependencies =  \
 sys.obj \
 $(LIBS)

#		*Explicit Rules*
production:     ..\bin\sys.com

..\bin\sys.com: sys.com
                copy sys.com ..\bin

b_fat12.h:      ..\boot\b_fat12.bin bin2c.com
                bin2c ..\boot\b_fat12.bin b_fat12.h b_fat12

b_fat16.h:      ..\boot\b_fat16.bin bin2c.com
                bin2c ..\boot\b_fat16.bin b_fat16.h b_fat16

sys.com:        $(EXE_dependencies)
#		 $(LINK) /m/t/c $(LIBPATH)\c0t.obj+sys.obj,sys,,\
		$(LINK) /m/c $(LIBPATH)\c0s.obj+sys.obj,sys.com,,\
                $(LIBS)+$(CLIB);

clobber:	clean
                $(RM) sys.com b_fat12.h b_fat16.h

clean:
                $(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las status.me

#		*Individual File Dependencies*
sys.obj: sys.c ..\hdr\portab.h ..\hdr\device.h b_fat12.h b_fat16.h
