!include "..\config.mak"

CFLAGS = -I$(INCLUDEPATH) -I..\hdr

production: patchobj.exe exeflat.exe

patchobj.exe: patchobj.obj
	$(CC) -L$(LIBPATH) patchobj.obj

exeflat.exe: exeflat.obj
	$(CC) -L$(LIBPATH) $(CFLAGSC) exeflat.obj

patchobj.obj: patchobj.c ..\config.mak

exeflat.obj: exeflat.c ..\hdr\exe.h ..\config.mak
	$(CC) $(CFLAGSC) $(CFLAGS) -c exeflat.c

clobber: clean
	$(RM) bin2c.com exeflat.exe patchobj.exe

clean:
	$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las status.me

