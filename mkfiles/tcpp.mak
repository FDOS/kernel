#
# TCPP.MAK - kernel compiler options for Turbo C++ 1.01
#

TARGET=KTP

CC=$(BINPATH)\tcc -c
CL=$(BINPATH)\tcc

# used for building the library

CLIB=$(LIBPATH)\cs.lib
MATH_EXTRACT=*H_LDIV *H_LLSH *H_LURSH *F_LXMUL
MATH_INSERT =+H_LDIV +H_LLSH +H_LURSH +F_LXMUL

#
# Compiler options for Turbo/Borland C
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
# Common options specified in turboc.cfg instead ALLCFLAGS
#

ALLCFLAGS=-I..\hdr;$(INCLUDEPATH) $(CPUOPT) $(ALLCFLAGS)
INITCFLAGS=@tci.cfg

CFLAGST=-I..\hdr;$(INCLUDEPATH) -L$(LIBPATH) -mt -lt
CFLAGSC=-I..\hdr;$(INCLUDEPATH) -L$(LIBPATH) -mc
