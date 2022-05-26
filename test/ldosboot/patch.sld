@goto :help

Patch iniload script
 by C. Masloch, 2021

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.


@:help
; Available patches:
;
; :patch_iniload_no_query_geometry
; :patch_iniload_no_lba
;
; Inputs: vef = nonzero to debug
;         cs:0 -> iniload
; Output: vee = nonzero if error
;         iniload patched
; Change: ve0 to vef, src, sro, aao, stack
;
; Requires lDebug release 3 or later
@goto :eof

:patch_iniload_no_query_geometry
@if not (vef) then r ysf |= C000
r ve0 word (sp - 100)
a ss:ve0
 mov ah, 08
 xor cx, cx
 stc
 int 13
 jc (ve0)
 .
r ve1 := aao - 1
r vec := ve1 - ve0
s cs:0 l #8192 range ss:ve0 l vec
if not (src) then goto :error
r byte [srs:sro + vec - 1] := EB
goto :success

:patch_iniload_no_lba
@if not (vef) then r ysf |= C000
r ve0 word (sp - 100)
a ss:ve0
 mov ah, 41
 mov dl, byte [bp + 40]
 mov bx, 55AA
 stc
 int 13
 mov al, 0
 jc (ve0)
 .
r ve1 := aao - 1
r vec := ve1 - ve0
s cs:0 l #8192 range ss:ve0 l vec
if (src) then goto :patch_iniload_no_lba.success
a ss:ve0
 mov ah, 41
 pop dx
 mov bx, 55AA
 stc
 int 13
 mov al, 0
 jc (ve0)
 .
r ve1 := aao - 1
r vec := ve1 - ve0
s cs:0 l #8192 range ss:ve0 l vec
if not (src) then goto :error

:patch_iniload_no_lba.success
r byte [srs:sro + vec - 1] := EB
goto :success

:success
r vee := 0
r ysf &= ~C000
; Patched successfully
@goto :eof

:error
r vee := 1
r ysf &= ~C000
; Patch failed
@goto :eof
