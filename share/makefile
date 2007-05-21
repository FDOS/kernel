
# nmake makefile
# share must be linked as COM file
# best is to use TC 2.01 which is freely available

USETC2=1
COM=1

!if $(USETC2)
TCC=d:\alt\tc201\tcc
TLINK=d:\alt\tc201\tlink
LIBS=d:\alt\tc201
COPT=-c -mt -1 -Id:\alt\tc201
LOPT=/m /s /c /t
!else
TCC=d:\alt\tc30\bin\tcc
TLINK=d:\alt\tc30\bin\tlink
LIBS=d:\alt\tc30\lib
!if $(COM)
COPT=-c -mt -1 -Id:\alt\tc30\Include
LOPT=-m -s -c -t
!else
COPT=-c -ms -1 -Id:\alt\tc30\Include
LOPT=-m -s -c
!endif
!endif

SHARE.COM: SHARE.OBJ
	$(TLINK) $(LOPT) $(LIBS)\c0t.obj share.obj,share.com,,$(LIBS)\cs.lib

SHARE.OBJ: SHARE.C    
	$(TCC) $(COPT) share.c

