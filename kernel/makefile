#
# makefile for kernel.sys
#
# $Id$
#

!include "../mkfiles/generic.mak"

LIBS=..\lib\device.lib ..\lib\libm.lib
HDR=../hdr/

# List Macros ##########################################################
# Only 8 files per definition; this is limitation of DOS batch
# files (only 9 directly accessible parameters).

# Order of linking is important: first kernel.asm, last INIT code.
OBJS1=kernel.obj entry.obj intr.obj irqstack.obj apisupt.obj io.obj console.obj
OBJS2=printer.obj serial.obj dosidle.obj execrh.obj asmsupt.obj int2f.obj \
nlssupt.obj
OBJS3=nls_hc.obj procsupt.obj dsk.obj error.obj blockio.obj chario.obj break.obj
OBJS4=fatfs.obj fatdir.obj fattab.obj dosfns.obj fcbfns.obj inthndlr.obj
OBJS5=ioctl.obj memmgr.obj task.obj newstuff.obj dosnames.obj nls.obj
OBJS6=prf.obj network.obj sysclk.obj lfnapi.obj systime.obj initclk.obj
OBJS7=initdisk.obj inithma.obj main.obj config.obj iprf.obj dyninit.obj \
iasmsupt.obj
OBJS=$(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) $(OBJS7)

# Explicit Rules #######################################################

all:		..\bin\$(TARGET).sys ..\bin\country.sys

..\bin\$(TARGET).sys: $(TARGET).lnk $(OBJS) $(LIBS) ..\utils\exeflat.exe
		$(LINK) @$(TARGET).lnk;
		..\utils\exeflat kernel.exe $*.sys $(LOADSEG) -S0x10 -S0x8B
		copy $*.sys ..\bin\kernel.sys

..\bin\country.sys: country.asm
		$(NASM) -o $*.sys country.asm

# XXX: This is a very ugly way of linking the kernel, forced upon us by the
# inability of Turbo `make' 2.0 to perform command line redirection. --ror4

ECHOTO=..\utils\echoto

$(TARGET).lnk:	$(DEPENDS) *.cfg
		-$(RM) *.lnk *.obj
		$(ECHOTO) $*.lnk $(OBJS1)+
		$(ECHOTO) $*.lnk $(OBJS2)+
		$(ECHOTO) $*.lnk $(OBJS3)+
		$(ECHOTO) $*.lnk $(OBJS4)+
		$(ECHOTO) $*.lnk $(OBJS5)+
		$(ECHOTO) $*.lnk $(OBJS6)+
		$(ECHOTO) $*.lnk $(OBJS7)
		$(ECHOTO) $*.lnk kernel
		$(ECHOTO) $*.lnk ..\bin\$*
		$(ECHOTO) $*.lnk $(LIBS)

# Individual File Dependencies #########################################

apisupt.obj:	apisupt.asm  segs.inc
asmsupt.obj:	asmsupt.asm  segs.inc
console.obj:	console.asm  io.inc
dosidle.obj:	dosidle.asm  segs.inc
entry.obj:	entry.asm    segs.inc $(HDR)stacks.inc
execrh.obj:	execrh.asm   segs.inc
int2f.obj:	int2f.asm    segs.inc $(HDR)stacks.inc
intr.obj:	intr.asm     segs.inc
io.obj:		io.asm       segs.inc $(HDR)stacks.inc
irqstack.obj:	irqstack.asm segs.inc
kernel.obj:	kernel.asm   segs.inc ludivmul.inc
nls_hc.obj:	nls_hc.asm   segs.inc
nlssupt.obj:	nlssupt.asm  segs.inc $(HDR)stacks.inc
printer.obj:	printer.asm  io.inc
procsupt.obj:	procsupt.asm segs.inc $(HDR)stacks.inc
serial.obj:	serial.asm   io.inc

HDRS=\
    $(HDR)portab.h $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h \
    $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)time.h $(HDR)process.h \
    $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h \
    $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
    $(HDR)version.h dyndata.h
HEADERS=$(HDRS) globals.h proto.h
INITHEADERS=$(HDRS) init-mod.h init-dat.h

blockio.obj:	blockio.c	$(HEADERS)
break.obj:	break.c		$(HEADERS)
chario.obj:	chario.c	$(HEADERS)
dosfns.obj:	dosfns.c	$(HEADERS)
dosnames.obj:	dosnames.c	$(HEADERS)
dsk.obj:	dsk.c		$(HEADERS)
error.obj:	error.c		$(HEADERS)
fatdir.obj:	fatdir.c	$(HEADERS)
fatfs.obj:	fatfs.c		$(HEADERS)
fattab.obj:	fattab.c	$(HEADERS)
fcbfns.obj:	fcbfns.c	$(HEADERS)
inthndlr.obj:	inthndlr.c	$(HEADERS)
ioctl.obj:	ioctl.c		$(HEADERS)
lfnapi.obj:	lfnapi.c	$(HEADERS)
memmgr.obj:	memmgr.c	$(HEADERS)
misc.obj:	misc.c		$(HEADERS)
network.obj:	network.c	$(HEADERS)
newstuff.obj:	newstuff.c	$(HEADERS)
nls.obj:	nls.c		$(HEADERS)
prf.obj:	prf.c		$(HDR)portab.h
strings.obj:	strings.c
sysclk.obj:	sysclk.c	$(HEADERS)
syspack.obj:	syspack.c	$(HEADERS)
systime.obj:	systime.c	$(HEADERS)
task.obj:	task.c		$(HEADERS)

# now the funny stuff :-)
# Files in the INIT segment

# XXX: Special handling for initialization modules -- this is required because
# TC 2.01 cannot handle `#pragma option' like TC 3 can. -- ror4

config.obj:	config.c	$(INITHEADERS)
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

dyninit.obj:	dyninit.c	$(INITHEADERS)
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

initclk.obj:	initclk.c	$(INITHEADERS)
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

initdisk.obj:	initdisk.c	$(INITHEADERS)
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

inithma.obj:	inithma.c	$(INITHEADERS)
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

initoem.obj:	initoem.c	$(INITHEADERS)
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

main.obj:	main.c		$(INITHEADERS)
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

# the string functions for INIT_TEXT

iasmsupt.obj:	asmsupt.asm
		$(NASM) $(NASMFLAGS) -D_INIT asmsupt.asm -o$*.obj

# the printf for INIT_TEXT - yet another special case, this file includes prf.c

iprf.obj:	iprf.c prf.c	$(HDR)portab.h
		$(CC) $(INITCFLAGS) $*.c
		$(INITPATCH) $*.obj

########################################################################

clean:
		-$(RM) *.bak *.cod *.crf *.err *.lnk *.lst *.map *.obj *.xrf

clobber:	clean
		-$(RM) kernel.exe status.me
