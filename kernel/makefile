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

production:     ../bin/$(TARGET).sys

../bin/$(TARGET).sys: kernel.sys
		$(CP) kernel.sys ..$(DIRSEP)bin
		$(CP) kernel.sys ..$(DIRSEP)bin$(DIRSEP)$(TARGET).sys
		$(CP) kernel.map ..$(DIRSEP)bin$(DIRSEP)$(TARGET).map

kernel.sys:	kernel.exe ../utils/exeflat.exe ../utils/upxentry.bin ../utils/upxdevic.bin exeflat.rsp
		..$(DIRSEP)utils$(DIRSEP)exeflat.exe kernel.exe kernel.sys $(LOADSEG) @exeflat.rsp

kernel.exe:	$(XLINKFILE) $(OBJS) $(LIBS)
		$(XLINK)

clobber:        clean
		-$(RM) kernel.exe kernel.sys status.me

clean:
		-$(RM) *.obj *.bak *.crf *.xrf *.map *.lst *.cod *.err *.lnk *.rsp

# XXX: This is a very ugly way of linking the kernel, forced upon us by the
# inability of Turbo `make' 2.0 to perform command line redirection. -- ror4

# -S to avoid showing expected relocations
# 0x10 & 0x78 or 0x79 depending on compilation options
exeflat.rsp: makefile
		-$(RM) exeflat.rsp
		$(ECHOTO) exeflat.rsp -S0x10
		$(ECHOTO) exeflat.rsp -S0x78
		$(ECHOTO) exeflat.rsp -S0x79
		$(ECHOTO) exeflat.rsp -E..$(DIRSEP)utils$(DIRSEP)upxentry.bin
		$(ECHOTO) exeflat.rsp -D..$(DIRSEP)utils$(DIRSEP)upxdevic.bin
		$(ECHOTO) exeflat.rsp $(UPXOPT)
		$(ECHOTO) exeflat.rsp $(XUPX)

kernel.lnk: turboc.cfg makefile ../mkfiles/generic.mak ../mkfiles/$(COMPILER).mak
		-$(RM) kernel.lnk
		$(ECHOTO) kernel.lnk $(OBJS1)+
		$(ECHOTO) kernel.lnk $(OBJS2)+
		$(ECHOTO) kernel.lnk $(OBJS3)+
		$(ECHOTO) kernel.lnk $(OBJS4)+
		$(ECHOTO) kernel.lnk $(OBJS5)+
		$(ECHOTO) kernel.lnk $(OBJS6)+
		$(ECHOTO) kernel.lnk $(OBJS7)+
		$(ECHOTO) kernel.lnk $(OBJS8)
		$(ECHOTO) kernel.lnk kernel.exe
		$(ECHOTO) kernel.lnk kernel.map
		$(ECHOTO) kernel.lnk $(LIBS)

#               *Individual File Dependencies*
apisupt.obj:	apisupt.asm segs.inc			$(XLINKFILE)
asmsupt.obj:	asmsupt.asm segs.inc			$(XLINKFILE)
console.obj:	console.asm io.inc			$(XLINKFILE)
cpu.obj:	cpu.asm  segs.inc $(XLINKFILE)
dosidle.obj:	dosidle.asm segs.inc			$(XLINKFILE)
entry.obj:	entry.asm   segs.inc $(HDR)stacks.inc	$(XLINKFILE)
execrh.obj:	execrh.asm  segs.inc 			$(XLINKFILE)
int2f.obj:	int2f.asm   segs.inc $(HDR)stacks.inc	$(XLINKFILE)
intr.obj:	intr.asm    segs.inc			$(XLINKFILE)
io.obj:		io.asm	    segs.inc $(HDR)stacks.inc   $(XLINKFILE)
irqstack.obj:   irqstack.asm segs.inc			$(XLINKFILE)
kernel.obj:	kernel.asm  segs.inc ludivmul.inc 	$(XLINKFILE)
memdisk.obj:	memdisk.asm  segs.inc $(XLINKFILE)
nls_hc.obj:	nls_hc.asm  segs.inc			$(XLINKFILE)
nlssupt.obj:	nlssupt.asm segs.inc $(HDR)stacks.inc   $(XLINKFILE)
printer.obj:	printer.asm io.inc			$(XLINKFILE)
procsupt.obj:	procsupt.asm segs.inc $(HDR)stacks.inc  $(XLINKFILE)
serial.obj:	serial.asm  io.inc			$(XLINKFILE)

HDRS=\
    $(HDR)portab.h $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h \
    $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)dtime.h $(HDR)process.h \
    $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h     \
    $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h  \
    $(HDR)version.h dyndata.h
HEADERS=$(HDRS) globals.h proto.h
INITHEADERS=$(HDRS) init-mod.h init-dat.h

blockio.obj: blockio.c    $(HEADERS) $(XLINKFILE)  
break.obj: break.c        $(HEADERS) $(XLINKFILE)  
chario.obj: chario.c      $(HEADERS) $(XLINKFILE)  
dosfns.obj: dosfns.c      $(HEADERS) $(XLINKFILE)  
dsk.obj: dsk.c            $(HEADERS) $(XLINKFILE)  
error.obj: error.c        $(HEADERS) $(XLINKFILE)  
fatdir.obj: fatdir.c      $(HEADERS) $(XLINKFILE)  
fatfs.obj: fatfs.c        $(HEADERS) $(XLINKFILE)  
fattab.obj: fattab.c      $(HEADERS) $(XLINKFILE)  
fcbfns.obj: fcbfns.c      $(HEADERS) $(XLINKFILE)  
inthndlr.obj: inthndlr.c  $(HEADERS) $(XLINKFILE)  
ioctl.obj: ioctl.c        $(HEADERS) $(XLINKFILE)  
memmgr.obj: memmgr.c      $(HEADERS) $(XLINKFILE)  
misc.obj: misc.c          $(HEADERS) $(XLINKFILE)  
lfnapi.obj: lfnapi.c      $(HEADERS) $(XLINKFILE)
newstuff.obj: newstuff.c  $(HEADERS) $(XLINKFILE)  
network.obj: network.c    $(HEADERS) $(XLINKFILE)  
nls.obj: nls.c            $(HEADERS) $(XLINKFILE)  
prf.obj: prf.c  $(HDR)portab.h $(XLINKFILE)
strings.obj: strings.c  $(XLINKFILE)
sysclk.obj: sysclk.c    $(HEADERS) $(XLINKFILE)
syspack.obj: syspack.c  $(HEADERS) $(XLINKFILE)
systime.obj: systime.c  $(HEADERS) $(XLINKFILE)
task.obj: task.c        $(HEADERS) $(XLINKFILE)

# now the funny stuff :-)
# Files in the INIT segment

# XXX: Special handling for initialization modules -- this is required because
# TC 2.01 cannot handle `#pragma option' like TC 3 can. -- ror4

config.obj:   config.c  $(INITHEADERS) $(XLINKFILE)  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

initoem.obj:  initoem.c  $(INITHEADERS) $(XLINKFILE)  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

main.obj:     main.c  $(INITHEADERS) $(XLINKFILE) 
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

inithma.obj:  inithma.c  $(INITHEADERS) $(XLINKFILE)  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

dyninit.obj:  dyninit.c  $(INITHEADERS) $(XLINKFILE)  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

initdisk.obj: initdisk.c  $(INITHEADERS) $(XLINKFILE)  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

initclk.obj: initclk.c  $(INITHEADERS) $(XLINKFILE)  
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj

#the string functions for INIT_TEXT
iasmsupt.obj: asmsupt.asm $(XLINKFILE)
	$(NASM) -D$(COMPILER) -D_INIT -f obj $(NASMFLAGS) -o iasmsupt.obj asmsupt.asm

#the printf for INIT_TEXT - yet another special case, this file includes prf.c
iprf.obj: iprf.c prf.c $(HDR)portab.h $(XLINKFILE)
	$(CC) $(INITCFLAGS) $*.c
	$(INITPATCH) $*.obj
