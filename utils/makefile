!include "../mkfiles/generic.mak"

DEPENDS=$(DEPENDS) *.cfg

########################################################################

all:		patchobj.com exeflat.exe

patchobj.com:	patchobj.c $(DEPENDS)

exeflat.exe:	exeflat.c ../hdr/exe.h $(DEPENDS)

########################################################################

clean:
		-$(RM) *.bak *.cod *.crf *.err *.las *.lst *.map *.obj *.xrf

clobber:	clean
		-$(RM) patchobj.com exeflat.exe status.me
