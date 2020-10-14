#
# Makefile for kernel.sys  (originally for Borland C/C++ 3.1)
#

!include "../mkfiles/generic.mak"

LIBS=..$(DIRSEP)lib$(DIRSEP)device.lib ..$(DIRSEP)lib$(DIRSEP)libm.lib
HDR=../hdr/

# *List Macros*
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

OBJS1=kernel.obj entry.obj io.obj console.obj serial.obj printer.obj dsk.obj \
sysclk.obj
OBJS2=asmsupt.obj execrh.obj nlssupt.obj procsupt.obj dosidle.obj int2f.obj \
nls_hc.obj
OBJS3=apisupt.obj intr.obj irqstack.obj blockio.obj chario.obj systime.obj \
error.obj
OBJS4=break.obj dosfns.obj fatdir.obj fatfs.obj fattab.obj fcbfns.obj \
inthndlr.obj
OBJS5=ioctl.obj memmgr.obj task.obj newstuff.obj nls.obj network.obj
OBJS6=prf.obj misc.obj strings.obj syspack.obj lfnapi.obj iasmsupt.obj memdisk.obj
OBJS7=main.obj config.obj initoem.obj inithma.obj dyninit.obj iprf.obj
OBJS8=initdisk.obj initclk.obj cpu.obj
OBJS=$(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) $(OBJS7) $(OBJS8)

#               *Explicit Rules*

production:     ../bin/$(TARGET).sys ../bin/country.sys

../bin/$(TARGET).sys: kernel.sys
		$(CP) kernel.sys ..$(DIRSEP)bin
		$(CP) kernel.sys ..$(DIRSEP)bin$(DIRSEP)$(TARGET).sys
		$(CP) kernel.map ..$(DIRSEP)bin$(DIRSEP)$(TARGET).map

# -S to avoid showing expected relocations
# 0x10 & 0x78 or 0x79 depending on compilation options
kernel.sys:	kernel.exe ../utils/exeflat.exe
		..$(DIRSEP)utils$(DIRSEP)exeflat.exe kernel.exe kernel.sys $(LOADSEG) -S0x10 -S0x78 -S0x79 $(UPXOPT) $(XUPX)

kernel.exe:	$(TARGET).lnk $(OBJS) $(LIBS)
		$(LINK) @$(TARGET).lnk;

../bin/country.sys: country.asm
		$(NASM) -o ../bin/country.sys country.asm

clobber:        clean
		-$(RM) kernel.exe kernel.sys status.me

clean:
		-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err *.lnk

# XXX: This is a very ugly way of linking the kernel, forced upon us by the
# inability of Turbo `make' 2.0 to perform command line redirection. -- ror4

$(TARGET).lnk: turboc.cfg makefile ../mkfiles/generic.mak ../mkfiles/$(COMPILER).mak
		-$(RM) *.lnk
		$(ECHOTO) $(TARGET).lnk $(OBJS1)+
		$(ECHOTO) $(TARGET).lnk $(OBJS2)+
		$(ECHOTO) $(TARGET).lnk $(OBJS3)+
		$(ECHOTO) $(TARGET).lnk $(OBJS4)+
		$(ECHOTO) $(TARGET).lnk $(OBJS5)+
		$(ECHOTO) $(TARGET).lnk $(OBJS6)+
		$(ECHOTO) $(TARGET).lnk $(OBJS7)+
		$(ECHOTO) $(TARGET).lnk $(OBJS8)
		$(ECHOTO) $(TARGET).lnk kernel.exe
		$(ECHOTO) $(TARGET).lnk kernel.map
		$(ECHOTO) $(TARGET).lnk $(LIBS)

#               *Individual File Dependencies*
apisupt.obj:	apisupt.asm segs.inc			$(TARGET).lnk
asmsupt.obj:	asmsupt.asm segs.inc			$(TARGET).lnk
console.obj:	console.asm io.inc			$(TARGET).lnk
cpu.obj:	cpu.asm  segs.inc $(TARGET).lnk
dosidle.obj:	dosidle.asm segs.inc			$(TARGET).lnk
entry.obj:	entry.asm   segs.inc $(HDR)stacks.inc	$(TARGET).lnk
execrh.obj:	execrh.asm  segs.inc 			$(TARGET).lnk
int2f.obj:	int2f.asm   segs.inc $(HDR)stacks.inc	$(TARGET).lnk
intr.obj:	intr.asm    segs.inc			$(TARGET).lnk
io.obj:		io.asm	    segs.inc $(HDR)stacks.inc   $(TARGET).lnk
irqstack.obj:   irqstack.asm segs.inc			$(TARGET).lnk
kernel.obj:	kernel.asm  segs.inc ludivmul.inc 	$(TARGET).lnk
memdisk.obj:	memdisk.asm  segs.inc $(TARGET).lnk
nls_hc.obj:	nls_hc.asm  segs.inc			$(TARGET).lnk
nlssupt.obj:	nlssupt.asm segs.inc $(HDR)stacks.inc   $(TARGET).lnk
printer.obj:	printer.asm io.inc			$(TARGET).lnk
procsupt.obj:	procsupt.asm segs.inc $(HDR)stacks.inc  $(TARGET).lnk
serial.obj:	serial.asm  io.inc			$(TARGET).lnk

HDRS=\
    $(HDR)portab.h $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h \
    $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)time.h $(HDR)process.h \
    $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h     \
    $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h  \
    $(HDR)version.h dyndata.h
HEADERS=$(HDRS) globals.h proto.h
INITHEADERS=$(HDRS) init-mod.h init-dat.h

blockio.obj: blockio.c    $(HEADERS) $(TARGET).lnk  
break.obj: break.c        $(HEADERS) $(TARGET).lnk  
chario.obj: chario.c      $(HEADERS) $(TARGET).lnk  
dosfns.obj: dosfns.c      $(HEADERS) $(TARGET).lnk  
dsk.obj: dsk.c            $(HEADERS) $(TARGET).lnk  
error.obj: error.c        $(HEADERS) $(TARGET).lnk  
fatdir.obj: fatdir.c      $(HEADERS) $(TARGET).lnk  
fatfs.obj: fatfs.c        $(HEADERS) $(TARGET).lnk  
fattab.obj: fattab.c      $(HEADERS) $(TARGET).lnk  
fcbfns.obj: fcbfns.c      $(HEADERS) $(TARGET).lnk  
inthndlr.obj: inthndlr.c  $(HEADERS) $(TARGET).lnk  
ioctl.obj: ioctl.c        $(HEADERS) $(TARGET).lnk  
memmgr.obj: memmgr.c      $(HEADERS) $(TARGET).lnk  
misc.obj: misc.c          $(HEADERS) $(TARGET).lnk  
lfnapi.obj: lfnapi.c      $(HEADERS) $(TARGET).lnk
newstuff.obj: newstuff.c  $(HEADERS) $(TARGET).lnk  
network.obj: network.c    $(HEADERS) $(TARGET).lnk  
nls.obj: nls.c            $(HEADERS) $(TARGET).lnk  
prf.obj: prf.c  $(HDR)portab.h $(TARGET).lnk
strings.obj: strings.c  $(TARGET).lnk
sysclk.obj: sysclk.c    $(HEADERS) $(TARGET).lnk
syspack.obj: syspack.c  $(HEADERS) $(TARGET).lnk
systime.obj: systime.c  $(HEADERS) $(TARGET).lnk
task.obj: task.c        $(HEADERS) $(TARGET).lnk

# now the funny stuff :-)
# Files in the INIT segment

# XXX: Special handling for initialization modules -- this is required because
# TC 2.01 cannot handle `#pragma option' like TC 3 can. -- ror4

config.obj:   config.c  $(INITHEADERS) $(TARGET).lnk  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

initoem.obj:  initoem.c  $(INITHEADERS) $(TARGET).lnk  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

main.obj:     main.c  $(INITHEADERS) $(TARGET).lnk 
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

inithma.obj:  inithma.c  $(INITHEADERS) $(TARGET).lnk  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

dyninit.obj:  dyninit.c  $(INITHEADERS) $(TARGET).lnk  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

initdisk.obj: initdisk.c  $(INITHEADERS) $(TARGET).lnk  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

initclk.obj: initclk.c  $(INITHEADERS) $(TARGET).lnk  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

#the string functions for INIT_TEXT
iasmsupt.obj: asmsupt.asm $(TARGET).lnk
	$(NASM) -D$(COMPILER) -D_INIT -f obj $(NASMFLAGS) -o $@ asmsupt.asm

#the printf for INIT_TEXT - yet another special case, this file includes prf.c
iprf.obj: iprf.c prf.c $(HDR)portab.h $(TARGET).lnk
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj
