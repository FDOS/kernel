
%if 0

Create test data in NASM
 2022, by C. Masloch

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif


%ifndef SIZE
 %define SIZE 512*16
%endif
%ifndef CONTENT
 %define CONTENT "a"
%endif

times SIZE db CONTENT
