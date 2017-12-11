#
# makefile for libm.lib
#
# $Id: makefile 688 2003-09-15 10:46:24Z bartoldeman $
#


!include "../mkfiles/generic.mak"


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

