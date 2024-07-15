!include "../mkfiles/generic.mak"

CFLAGS = -I..$(DIRSEP)hdr

production: patchobj.com bin2c.exe exeflat.exe upxentry.bin upxdevic.bin

patchobj.com: patchobj.c
	$(CLT) $(CFLAGS) patchobj.c

upxentry.bin: upxentry.asm
	$(NASM) -f bin upxentry.asm -o upxentry.bin

upxdevic.bin: upxdevic.asm
	$(NASM) -f bin upxdevic.asm -o upxdevic.bin

exeflat.exe: exeflat.c ../hdr/exe.h
	$(CLC) $(CFLAGS) exeflat.c

bin2c.exe: bin2c.c
	$(CLC) $(CFLAGS) bin2c.c


clobber: clean

clean:
	$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.las *.cod *.err status.me
	$(RM) bin2c.exe exeflat.exe patchobj.com upxentry.bin upxdevic.bin

