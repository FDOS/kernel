#
# Makefile for Borland C++ 3.1 for kernel.sys
#
# $Id$
#

# $Log$
# Revision 1.13  2001/07/24 16:56:29  bartoldeman
# fixes for FCBs, DJGPP ls, DBLBYTE, dyninit allocation (2024e).
#
# Revision 1.12  2001/07/22 01:58:58  bartoldeman
# Support for Brian's FORMAT, DJGPP libc compilation, cleanups, MSCDEX
#
# Revision 1.11  2001/07/09 22:19:33  bartoldeman
# LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
#
# Revision 1.10  2001/04/21 22:32:53  bartoldeman
# Init DS=Init CS, fixed stack overflow problems and misc bugs.
#
# Revision 1.9  2001/04/16 14:28:32  bartoldeman
# Kernel build 2024. Fixed critical error handler/config.sys/makefiles/UMBs
#
# Revision 1.8  2001/04/15 03:21:50  bartoldeman
# See history.txt for the list of fixes.
#
# Revision 1.7  2001/03/25 17:11:54  bartoldeman
# Fixed sys.com compilation. Updated to 2023. Also: see history.txt.
#
# Revision 1.6  2001/03/21 02:56:26  bartoldeman
# See history.txt for changes. Bug fixes and HMA support are the main ones.
#
# Revision 1.4  2000/08/06 05:50:17  jimtabor
# Add new files and update cvs with patches and changes
#
# Revision 1.3  2000/05/25 20:56:21  jimtabor
# Fixed project history
#
# Revision 1.2  2000/05/08 04:30:00  jimtabor
# Update CVS to 2020
#
# Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
# The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
# MS-DOS.  Distributed under the GNU GPL.
#
# Revision 1.14  2000/03/31 05:40:09  jtabor
# Added Eric W. Biederman Patches
#
# Revision 1.13  2000/03/17 22:59:04  kernel
# Steffen Kaiser's NLS changes
#
# Revision 1.12  2000/03/09 06:07:11  kernel
# 2017f updates by James Tabor
#
# Revision 1.11  1999/09/23 04:40:47  jprice
# *** empty log message ***
#
# Revision 1.8  1999/09/13 20:41:41  jprice
# Some clean up.
#
# Revision 1.7  1999/08/25 03:18:09  jprice
# ror4 patches to allow TC 2.01 compile.
#
# Revision 1.6  1999/08/10 17:57:13  jprice
# ror4 2011-02 patch
#
# Revision 1.5  1999/04/23 04:25:15  jprice
# no message
#
# Revision 1.4  1999/04/23 03:45:11  jprice
# Improved by jprice
#
# Revision 1.3  1999/04/16 12:21:22  jprice
# Steffen c-break handler changes
#
# Revision 1.2  1999/04/13 15:48:21  jprice
# no message
#
# Revision 1.1.1.1  1999/03/29 15:41:15  jprice
# New version without IPL.SYS
#
# Revision 1.7  1999/03/01 06:04:37  jprice
# Fixed so it'll work with config.mak
#
# Revision 1.6  1999/03/01 05:46:43  jprice
# Turned off DEBUG define.
#
# Revision 1.5  1999/02/09 04:49:43  jprice
# Make makefile use common config.mak file
#
# Revision 1.4  1999/02/08 05:55:57  jprice
# Added Pat's 1937 kernel patches
#
# Revision 1.3  1999/02/04 03:09:59  jprice
# Added option to share constants (-d).
#
# Revision 1.2  1999/01/22 04:13:26  jprice
# Formating
#
# Revision 1.1.1.1  1999/01/20 05:51:01  jprice
# Imported sources
#
#
#   Rev 1.8.1 10 Jan 1999            SRM
#Took out "/P-" from TLINK
#Changed "bcc" to "tcc"
#
#   Rev 1.9   06 Dec 1998  8:45:40   patv
#Added new files for I/O subsystem.
#
#   Rev 1.8   22 Jan 1998 14:50:06   patv
#Outdated stacks.asm.
#
#   Rev 1.6   03 Jan 1998  8:36:50   patv
#Converted data area to SDA format
#
#   Rev 1.5   30 Jan 1997  7:55:54   patv
#Added TSC flag for trace support.
#
#   Rev 1.4   16 Jan 1997 12:46:42   patv
#pre-Release 0.92 feature additions
#
#   Rev 1.3   29 Aug 1996 13:07:34   patv
#Bug fixes for v0.91b
#
#   Rev 1.2   29 May 1996 21:03:32   patv
#bug fixes for v0.91a
#
#   Rev 1.1   19 Feb 1996  3:35:38   patv
#Added NLS, int2f and config.sys processing
#
#   Rev 1.0   02 Jul 1995  8:30:22   patv
#Initial revision.
#
# $EndLog$
#

!include "..\config.mak"

RELEASE = 1.00

# Compiler and Options for Borland C++
# ------------------------------------
#
#  -zAname       ¦ ¦ Code class
#  -zBname       ¦ ¦ BSS class
#  -zCname       ¦ ¦ Code segment
#  -zDname       ¦ ¦ BSS segment
#  -zEname       ¦ ¦ Far segment
#  -zFname       ¦ ¦ Far class
#  -zGname       ¦ ¦ BSS group
#  -zHname       ¦ ¦ Far group
#  -zPname       ¦ ¦ Code group
#  -zRname       ¦ ¦ Data segment
#  -zSname       ¦ ¦ Data group
#  -zTname       ¦ ¦ Data class
#  -zX           ¦«¦ Use default name for "X"

#
# Compiler and Options for Borland C++
# ------------------------------------
LIBPATH = .
INCLUDEPATH = ..\HDR
#AFLAGS      = /Mx /DSTANDALONE=1 /I..\HDR
NASMFLAGS   = -i../hdr/
LIBS        =..\LIB\DEVICE.LIB ..\LIB\LIBM.LIB
INITCFLAGS =$(ALLCFLAGS) -zAINIT -zCINIT_TEXT -zDIB -zRID -zTID -zPIGROUP -zBIB \
-zGIGROUP -zSIGROUP
CFLAGS     =$(ALLCFLAGS) -zAHMA -zCHMA_TEXT
HDR=../hdr/

#               *Implicit Rules*
.c.obj:
	$(CC) $(CFLAGS) -c $<

# Borland make 3.6 does not like this rule.
#.c.asm:
#	$(CC) $(CFLAGS) -S $<

.cpp.obj:
	$(CC) $(CFLAGS) -c $<

.asm.obj:
	$(NASM) $(NASMFLAGS) -f obj $<

#               *List Macros*


EXE_dependencies =  \
 apisupt.obj  \
 asmsupt.obj  \
 blockio.obj  \
 break.obj    \
 chario.obj   \
 config.obj   \
 console.obj  \
 dosidle.obj  \
 dosfns.obj   \
 dosnames.obj \
 dsk.obj      \
 initdisk.obj      \
 entry.obj    \
 error.obj    \
 execrh.obj   \
 fatdir.obj   \
 fatfs.obj    \
 fattab.obj   \
 fcbfns.obj   \
 initoem.obj  \
 int2f.obj    \
 inthndlr.obj \
 io.obj       \
 intr.obj     \
 ioctl.obj    \
 irqstack.obj \
 kernel.obj   \
 main.obj     \
 memmgr.obj   \
 misc.obj     \
 newstuff.obj \
 network.obj  \
 nls.obj      \
 nls_hc.obj   \
 nlssupt.obj  \
 prf.obj      \
 initprf.obj  \
 printer.obj  \
 procsupt.obj \
 serial.obj   \
 strings.obj  \
 sysclk.obj   \
 syspack.obj  \
 systime.obj  \
 task.obj     \
 inithma.obj  \
 DynData.obj  \
 DynInit.obj

#               *Explicit Rules*

production:     ..\bin\kernel.sys

..\bin\kernel.sys: kernel.sys
                copy kernel.sys ..\bin

kernel.sys:	kernel.exe
                ..\utils\exeflat kernel.exe kernel.sys 0x60

clobber:        clean
                $(RM) kernel.exe kernel.sys status.me

clean:
                $(RM) *.obj *.bak *.crf *.xrf *.map *.lst

# XXX: This is a very ugly way of linking the kernel, forced upon us by the
# inability of Turbo `make' 2.0 to perform command line redirection. -- ror4
kernel.exe: $(EXE_dependencies) $(LIBS)
    $(RM) kernel.lib
    $(LIBUTIL) kernel +entry +io +blockio +chario +dosfns +console
    $(LIBUTIL) kernel +printer +serial +dsk +initdisk+error +fatdir +fatfs
    $(LIBUTIL) kernel +fattab +fcbfns +initoem +initHMA+inthndlr +ioctl +nls_hc
    $(LIBUTIL) kernel +main +config +memmgr +misc +newstuff +nls +intr
    $(LIBUTIL) kernel +dosnames +prf +initprf +strings +network +sysclk +syspack
    $(LIBUTIL) kernel +systime +task +int2f +irqstack +apisupt
    $(LIBUTIL) kernel +asmsupt +execrh +nlssupt +procsupt +break
    $(LIBUTIL) kernel +dosidle+dyndata+dyninit
    $(RM) kernel.bak
    $(LINK) /m/c/L$(LIBPATH) kernel,kernel,kernel,kernel+$(LIBS);
    $(RM) kernel.lib

#               *Individual File Dependencies*
kernel.obj: kernel.asm segs.inc

console.obj: console.asm io.inc

printer.obj: printer.asm io.inc

serial.obj: serial.asm io.inc

entry.obj: entry.asm segs.inc $(HDR)stacks.inc

apisupt.obj: apisupt.asm segs.inc

asmsupt.obj: asmsupt.asm segs.inc

execrh.obj: execrh.asm segs.inc

int2f.obj: int2f.asm segs.inc

intr.obj: intr.asm segs.inc intr.h

io.obj: io.asm segs.inc

irqstack.obj: irqstack.asm

nls_hc.obj: nls_hc.asm segs.inc

nlssupt.obj: nlssupt.asm segs.inc

procsupt.obj: procsupt.asm segs.inc $(HDR)stacks.inc

dosidle.obj: dosidle.asm segs.inc

# XXX: Special handling for initialization modules -- this is required because
# TC 2.01 cannot handle `#pragma option' like TC 3 can. -- ror4
config.obj: config.c init-mod.h $(HDR)portab.h globals.h \
 $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h \
 $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h \
 $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h \
 $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h \
 $(HDR)error.h $(HDR)version.h proto.h turboc.cfg  dyndata.h
	$(CC) $(INITCFLAGS) -c config.c

initoem.obj: initoem.c init-mod.h $(HDR)portab.h globals.h \
 $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h \
 $(HDR)fat.h $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h \
 $(HDR)sft.h $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h \
 $(HDR)dirmatch.h $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h \
 $(HDR)error.h $(HDR)version.h proto.h turboc.cfg  dyndata.h
	$(CC) $(INITCFLAGS) -c initoem.c

main.obj: main.c init-mod.h $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg dyndata.h
	$(CC) $(INITCFLAGS) -c main.c

initHMA.obj: initHMA.c init-mod.h $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg  dyndata.h
	$(CC) $(INITCFLAGS) -c initHMA.c

DynInit.obj: DynInit.c init-mod.h $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg  dyndata.h
	$(CC) $(INITCFLAGS) -c DynInit.c

initdisk.obj: initdisk.c $(HDR)portab.h globals.h $(HDR)device.h $(HDR)mcb.h \
 $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h $(HDR)fcb.h \
 $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h \
 $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h $(HDR)file.h \
 $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h $(HDR)version.h proto.h \
 turboc.cfg  dyndata.h
	$(CC) $(INITCFLAGS) -c initDISK.c
	

#  -zBname       ¦ ¦ BSS class
#  -zDname       ¦ ¦ BSS segment
#  -zEname       ¦ ¦ Far segment
#  -zFname       ¦ ¦ Far class
#  -zGname       ¦ ¦ BSS group
#  -zHname       ¦ ¦ Far group
#  -zRname       ¦ ¦ Data segment
#  -zSname       ¦ ¦ Data group
#  -zTname       ¦ ¦ Data class

dynDATA.obj: dynDATA.c dynDATA.H
	$(CC) -c -zRDYN_DATA -zTDYN_DATA -zDDYN_DATA -zBDYN_DATA dynDATA.c

	
#the printf for INIT_TEXT:
initprf.obj: prf.c $(HDR)portab.h turboc.cfg
	$(CC) -DFORINIT $(INITCFLAGS) -oinitprf.obj -c prf.c 

# XXX: I generated these using `gcc -MM' and `sed', so they may not be
# completely correct... -- ror4
blockio.obj: blockio.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

break.obj: break.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

chario.obj: chario.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

dosfns.obj: dosfns.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

dosnames.obj: dosnames.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

dsk.obj: dsk.c $(HDR)portab.h globals.h $(HDR)device.h $(HDR)mcb.h \
 $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h $(HDR)fcb.h \
 $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h \
 $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h $(HDR)file.h \
 $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h $(HDR)version.h proto.h \
 turboc.cfg


error.obj: error.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

fatdir.obj: fatdir.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

fatfs.obj: fatfs.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

fattab.obj: fattab.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

fcbfns.obj: fcbfns.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

inthndlr.obj: inthndlr.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

ioctl.obj: ioctl.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

memmgr.obj: memmgr.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

misc.obj: misc.c $(HDR)portab.h globals.h $(HDR)device.h $(HDR)mcb.h \
 $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h $(HDR)fcb.h \
 $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h \
 $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h $(HDR)file.h \
 $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h $(HDR)version.h proto.h \
 turboc.cfg

newstuff.obj: newstuff.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

network.obj: network.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

nls.obj: nls.c $(HDR)portab.h globals.h $(HDR)device.h $(HDR)mcb.h \
 $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h $(HDR)fcb.h \
 $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h \
 $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h $(HDR)file.h \
 $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h $(HDR)version.h proto.h \
 turboc.cfg

# \
# 001-437.nls

prf.obj: prf.c $(HDR)portab.h turboc.cfg

strings.obj: strings.c $(HDR)portab.h turboc.cfg

sysclk.obj: sysclk.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

syspack.obj: syspack.c $(HDR)portab.h globals.h $(HDR)device.h \
 $(HDR)mcb.h $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

systime.obj: systime.c $(HDR)portab.h $(HDR)time.h $(HDR)date.h \
 globals.h $(HDR)device.h $(HDR)mcb.h $(HDR)pcb.h $(HDR)fat.h \
 $(HDR)fcb.h $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h \
 $(HDR)cds.h $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h \
 $(HDR)file.h $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h \
 $(HDR)version.h proto.h turboc.cfg

task.obj: task.c $(HDR)portab.h globals.h $(HDR)device.h $(HDR)mcb.h \
 $(HDR)pcb.h $(HDR)date.h $(HDR)time.h $(HDR)fat.h $(HDR)fcb.h \
 $(HDR)tail.h $(HDR)process.h $(HDR)dcb.h $(HDR)sft.h $(HDR)cds.h \
 $(HDR)exe.h $(HDR)fnode.h $(HDR)dirmatch.h $(HDR)file.h \
 $(HDR)clock.h $(HDR)kbd.h $(HDR)error.h $(HDR)version.h proto.h \
 turboc.cfg


