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
; :patch_iniload_no_query_geometry      (new or old style)
; :patch_iniload_no_lba                 (new or old style)
; :patch_iniload_no_query_geometry_old  (old style)
; :patch_iniload_no_lba_old             (old style)
; :patch_iniload_no_chs                 (new style only)
; :patch_iniload_detect_lba             (new style only)
; :patch_iniload_detect_geometry        (new style only)
;
; Inputs: vef = nonzero to debug
;         cs:ve7 -> iniload
; Output: vee = nonzero if error
;         iniload patched if successful
; Change: ve0 to vef, src, sro, aao, stack
;
; :query_patch_iniload                  (new style)
;
; Input:  ve8 = what to set for new style query patch
;               0 = default, -1 = none (do old style patch)
;         ve9 = what to clear for new style query patch
; Requires lDebug release 3 or later
@goto :eof


:query_patch_iniload
@if not (vef) then r ysf |= C000
@:query_patch_common
r ve0 word (sp - 100)
a ss:ve0
 mov dl, byte [bp + 40]
 mov ax, 0
 .
r ve1 := aao - 2
r vec := ve1 - ve0
s cs:ve7 l #1536 range ss:ve0 l vec
if (src == 0 && vee == -1) then goto :eof
if not (src) then goto :query_patch_error
a ss:ve0
 test dl, dl
 jns (ve0 + 6)
 db 86,C4	; xchg al, ah
 .
if (word [srs:sro + vec + 2] != word [ss:ve0 + 0]) then goto :query_patch_error
if (word [srs:sro + vec + 4] != word [ss:ve0 + 2]) then goto :query_patch_error
if (word [srs:sro + vec + 6] != word [ss:ve0 + 4]) then goto :query_patch_error
if not (ve8 or ve9) then goto :query_patch_found
r word [srs:sro + vec] or:= ve8
r word [srs:sro + vec] and:= ~ve9
:query_patch_success
r ve8 := 0
r ve9 := 0
r vee := 0
r ysf &= ~C000
; Patched successfully (new style)
@goto :eof

:query_patch_found
r ve8 := 0
r ve9 := 0
r vee := 0
r ysf &= ~C000
; Patch site found, no patch requested (new style)
@goto :eof

:query_patch_error
r ve8 := 0
r ve9 := 0
r vee := 1
r ysf &= ~C000
; Patch failed (new style)
@goto :eof


:patch_iniload_no_query_geometry
@if not (vef) then r ysf |= C000
r vee := -1
r ve8 := 404
r ve9 := 0
y :query_patch_common
@if not (vee == -1) then goto :eof

:patch_iniload_no_query_geometry_old
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
s cs:ve7 l #1536 range ss:ve0 l vec
if not (src) then goto :error
r word [srs:sro + vec - 3] := 9090
goto :success


:patch_iniload_no_lba
@if not (vef) then r ysf |= C000
r vee := -1
r ve8 := 101
r ve9 := 0
y :query_patch_common
@if not (vee == -1) then goto :eof

:patch_iniload_no_lba_old
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
s cs:ve7 l #1536 range ss:ve0 l vec
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
s cs:ve7 l #1536 range ss:ve0 l vec
if (src) then goto :patch_iniload_no_lba.success
a ss:ve0
 mov ah, 41
 mov bx, 55AA
 stc
 int 13
 mov al, 0
 jc (ve0)
 .
r ve1 := aao - 1
r vec := ve1 - ve0
s cs:ve7 l #1536 range ss:ve0 l vec
if not (src) then goto :error

:patch_iniload_no_lba.success
r word [srs:sro + vec - 5] := 9090
goto :success

:success
r ve8 := 0
r ve9 := 0
r vee := 0
r ysf &= ~C000
; Patched successfully
@goto :eof


:patch_iniload_no_chs
@if not (vef) then r ysf |= C000
r vee := -1
r ve9 := 0
r ve8 := 202
y :query_patch_common
@if not (vee == -1) then goto :eof
goto :error

:patch_iniload_detect_lba
@if not (vef) then r ysf |= C000
r vee := -1
r ve8 := 0
r ve9 := 303
y :query_patch_common
@if not (vee == -1) then goto :eof
goto :error

:patch_iniload_detect_geometry
@if not (vef) then r ysf |= C000
r vee := -1
r ve8 := 0
r ve9 := 404
y :query_patch_common
@if not (vee == -1) then goto :eof
goto :error

:error
r ve8 := 0
r ve9 := 0
r vee := 1
r ysf &= ~C000
; Patch failed
@goto :eof
