!include "../mkfiles/generic.mak"

CFLAGS = -I$(INCLUDEPATH) -I..\hdr

production: patchobj.com exeflat.exe

patchobj.com: patchobj.c
	$(CL) $(CFLAGST) $(TINY) $(CFLAGS) patchobj.c

exeflat.exe: exeflat.c ../hdr/exe.h
	$(CL) $(CFLAGSC) $(CFLAGS) exeflat.c


clobber: clean

clean:
	$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me
	$(RM) exeflat.exe patchobj.com

