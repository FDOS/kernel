#
# makefile for libm.lib
#
# $Id$
#


!include "..\mkfiles\generic.mak"


libm.lib:	$(CLIB)
	-$(RM) libm.lib
	$(LIBUTIL) $(CLIB) $(MATH_EXTRACT) $(LIBTERM)
	$(COMSPEC) /c for %i in (*.obj) do ..\utils\patchobj CODE=LCODE %i
	$(LIBUTIL) libm $(MATH_INSERT)     $(LIBTERM)
	-$(RM) *.OBJ


clobber:        clean
	-$(RM) status.me

clean:
	-$(RM) *.obj *.bak libm.lib

