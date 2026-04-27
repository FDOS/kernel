# Makefile for INT 2F / AH=12h tests
# Open Watcom 16-bit, real-mode DOS, small memory model
#
# Directory layout assumed:
#   tests/
#       greatest.h
#       testlib.h
#       testlib.c
#       int2f-12/
#           tNNNN.c
#           Makefile        <- this file
#
# Usage:
#   wmake              build all .exe files
#   wmake clean        remove generated files
#   wmake ref          run all tests in reference mode (writes *.ref)
#   wmake test         run all tests in test mode     (writes *.tst)

CC      = wcl
CFLAGS  = -zq -q -ms -k4096 
INCLUDES= -i=..
TESTLIB = ../testlib.c
DEPS    = ../testlib.c ../testlib.h ../greatest.h

# List of test executables (add new ones here as you implement them)
EXES = t121f.com t121f.exe t1203.exe t1204.exe t1211.exe t1212.exe t1213.exe t1214.exe t121a.exe t121c.exe t121d.exe t1225.exe 

all: $(EXES)

# Implicit suffix rule: any .c -> .exe in this directory
# wmake uses this when no explicit rule exists for a target
.c.exe: $(DEPS) .AUTODEPEND
	$(CC) $(CFLAGS) $(INCLUDES) $[*.c $(TESTLIB)
	
# MSDOS requires SS=DOSSTACK so we need to build special version, FD kernel does not need SS=DOSSTACK so use .exe version
t121f.com:
	$(CC) -zq -q -mt -fpc $(INCLUDES) -dUSEDOSSTACK t121f.c $(TESTLIB)
	del testlib.obj

	
# Reference mode: run on MS-DOS to generate golden .ref files
ref: $(EXES)
	for %f in ($(EXES)) do %f ref

# Test mode: compare against .ref files, write .tst
test: $(EXES)
	for %f in ($(EXES)) do %f test

clean: .SYMBOLIC
	@if exist *.obj del *.obj
	@if exist *.map del *.map
	@if exist *.err del *.err

clobber: clean .SYMBOLIC
	@if exist *.exe del *.exe
	@if exist *.ref del *.ref
	@if exist *.tst del *.tst
	@if exist *.txt del *.txt
	@if exist *.out del *.out
