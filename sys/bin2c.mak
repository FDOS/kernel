#
# makefile for bin2c.com
#
# $Id$
#
# $Log$
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

!include "..\config.mak"

CFLAGS = -mt -1- -v -vi- -k- -f- -ff- -O -Z -d -I$(INCLUDEPATH) -L$(LIBPATH)

#               *Implicit Rules*
.c.obj:
  $(CC) $(CFLAGS) -c $<

.cpp.obj:
  $(CC) $(CFLAGS) -c $<

#		*List Macros*

EXE_dependencies =  \
 bin2c.obj

#		*Explicit Rules*
production:     bin2c.com

bin2c.com:      $(EXE_dependencies)
                $(LINK) /m/t/c $(LIBPATH)\c0t.obj+bin2c.obj,bin2c,,\
		$(LIBS)+$(CLIB);


clobber:	clean
                $(RM) bin2c.com

clean:
        ..\utils\rm -f *.obj *.bak *.crf *.xrf *.map *.lst *.las status.me

#		*Individual File Dependencies*
bin2c.obj: bin2c.c
