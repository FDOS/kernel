!include "..\mkfiles\generic.mak"

CFLAGS = -I$(INCLUDEPATH) -I..\hdr

production: patchobj.exe exeflat.exe

patchobj.exe: patchobj.c
	$(CC) $(CFLAGST) $(CFLAGS) patchobj.c

exeflat.exe: exeflat.c ..\hdr\exe.h
	$(CC) $(CFLAGSC) $(CFLAGS) exeflat.c


clobber: clean
	$(RM) bin2c.com exeflat.exe patchobj.exe

clean:
	$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod status.me

