#
# makefile for libm.lib
#
# $Id$
#

!include "../mkfiles/generic.mak"

########################################################################

all:		libm.lib

libm.lib:	$(CLIB) $(DEPENDS)
		-$(RM) $*.lib
		$(LIBUTIL) $(CLIB) $(MATH_EXTRACT) $(LIBTERM)
		$(COMSPEC) /c for %i in (*.obj) do ..\utils\patchobj CODE=LCODE %i
		$(LIBUTIL) $* $(MATH_INSERT) $(LIBTERM)
		-$(RM) *.obj

########################################################################

clean:
		-$(RM) *.bak *.cod *.crf *.err *.las *.lst *.map *.obj *.xrf

clobber:	clean
		-$(RM) libm.lib status.me
