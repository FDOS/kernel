#
# TC.MAK - kernel compiler options for Turbo C 2.01
#

BINPATH=$(BASE)

!include "..\mkfiles\tcpp.mak"

TARGET=KTC

MATH_EXTRACT=*LDIV *LXMUL *LURSH *LLSH *LRSH
MATH_INSERT =+LDIV +LXMUL +LURSH +LLSH +LRSH

# TCC doesn't support responce file

INITCFLAGS=-zCINIT_TEXT -zRID -zTID -zSI_GROUP -zDIB -zBIB -zGI_GROUP
