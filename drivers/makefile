#
# makefile for device.lib
#
# $Id$
#

!include "../mkfiles/generic.mak"

OBJS=floppy.obj rdpcclk.obj wrpcclk.obj wratclk.obj
LIBOBJS=+floppy +rdpcclk +wrpcclk +wratclk

# Build the LIBRARY ####################################################

all:		..\lib\device.lib

..\lib\device.lib: $(OBJS)
		-$(RM) $*.lib
		$(LIBUTIL) $(LIBFLAGS) $* $(LIBOBJS) $(LIBTERM)

$(OBJS):	$(DEPENDS)

########################################################################

clean:
		-$(RM) *.bak *.cod *.crf *.err *.las *.lst *.map *.obj *.xrf

clobber:	clean
		-$(RM) ..\lib\device.lib status.me
