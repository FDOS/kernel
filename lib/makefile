#
# makefile for libm.lib
#
# $Id$
#


!include "..\mkfiles\generic.mak"


libm.lib:	$(CLIB)
	-$(RM) libm.lib
	$(LIBUTIL) $(CLIB) $(MATH_EXTRACT) $(LIBTERM)
	$(LIBUTIL) libm $(MATH_INSERT)     $(LIBTERM)
	-$(RM) *.OBJ


clobber:        clean
	-$(RM) libm.lib status.me

clean:
	-$(RM) *.obj *.bak

# Log: makefile,v 
#
# Revision 1.6  1999/09/14 17:32:20  jprice
# no message
#
# Revision 1.5  1999/09/13 20:13:15  jprice
# Added !if so we can use TC2 or TC3 to compile.
#
# Revision 1.4  1999/08/25 03:19:22  jprice
# ror4 patches to allow TC 2.01 compile.
#
# Revision 1.3  1999/04/23 03:45:18  jprice
# Improved by jprice
#
