
%if 0

File system boot sector loader code for FAT12 or FAT16

Adapted from 2002-11-26 fatboot.zip/fat12.asm,
 released as public domain by Chris Giese

Public domain by C. Masloch, 2012

%endif


%include "lmacros2.mac"

%ifndef _MAP
%elifempty _MAP
%else	; defined non-empty, str or non-str
	[map all _MAP]
%endif

	defaulting

	numdef FAT16,		0	; 0 = FAT12, 1 = FAT16
	strdef OEM_NAME,	"    lDOS"
	strdef OEM_NAME_FILL,	'_'
	strdef DEFAULT_LABEL,	"lDOS"
	numdef VOLUMEID,	0

	strdef LOAD_NAME,	"LDOS"
	strdef LOAD_EXT,	"COM"	; name of file to load
	numdef LOAD_ADR,	02000h	; where to load
	numdef LOAD_MIN_PARA,	paras(4096)
	numdef LOAD_NON_FAT,	0, 2048	; use FAT-less loading (value is amount bytes)
	numdef EXEC_SEG_ADJ,	0	; how far cs will be from _LOAD_ADR
	numdef EXEC_OFS,	400h	; what value ip will be
	numdef CHECKOFFSET,	1020
	numdef CHECKVALUE,	"lD"
	numdef LOAD_DIR_SEG,	0	; => where to store dir entry (0 if nowhere)
	numdef ADD_SEARCH,	0	; whether to search second file
	strdef ADD_NAME,	""
	strdef ADD_EXT,		""	; name of second file to search
	numdef ADD_DIR_SEG,	0	; => where to store dir entry (0 if nowhere)
	numdef CHECK_ATTRIB,	0	; check attribute for LFN, label, directory
	numdef ATTRIB_SAVE,	_CHECK_ATTRIB

	gendef _ADR_DIRBUF, end -start+7C00h	; 07E00h
	gendef _ADR_FATBUF, end -start+7C00h	; 07E00h

	numdef QUERY_GEOMETRY,	1	; query geometry via 13.08 (for CHS access)
	numdef QUERY_GEOMETRY_DISABLED, 0
	numdef USE_PART_INFO,	1	; use ds:si-> partition info from MBR, if any
	numdef USE_PART_INFO_DISABLED, 0
	numdef USE_AUTO_UNIT,	1	; use unit passed from ROM-BIOS in dl
	numdef RPL,		1	; support RPL and do not overwrite it
	numdef RPL_GRACE_AREA,	130 * 1024
					; alternative RPL support,
					;  assume RPL fits in this area
	numdef CHS,		1	; support CHS (if it fits)
	numdef LBA,		1	; support LBA (if available)
	numdef LBA_33_BIT,	1	; support 33-bit LBA
	numdef LBA_CHECK_NO_33,	1	; else: check that LBA doesn't carry

	numdef RELOCATE,	0	; relocate the loader to top of memory
	numdef SET_BL_UNIT,	0	; if to pass unit in bl as well
	numdef SET_DL_UNIT,	0	; if to pass unit in dl
	numdef SET_AXBX_DATA,	0	; if to pass first data sector in ax:bx
	numdef SET_DSSI_DPT,	0	; if to pass DPT address in ds:si
	numdef PUSH_DPT,	0	; if to push DPT address
	numdef MEMORY_CONTINUE,	1	; if to just execute when memory full
	numdef SET_DI_CLUSTER,	0	; if to pass first load file cluster in di
	numdef DIRBUF_500,	0	; if to load root dir sector(s) to 0:500h
	numdef DIR_ENTRY_500,	0	; if to copy directory entry to 0:500h
	numdef DIR_ENTRY_520,	0	; if to copy next directory entry to 0:520h
	numdef TURN_OFF_FLOPPY,	0	; if to turn off floppy motor after loading
	numdef DATASTART_HIDDEN,0	; if to add hidden sectors to data_start
	numdef LBA_SET_TYPE,	0	; if to set third byte to LBA partition type
	numdef SET_LOAD_SEG,	1	; if to set load_seg (word [ss:bp - 6])
	numdef SET_FAT_SEG,	1	; if to set fat_seg (word [ss:bp - 8])
	numdef SET_FAT_SEG_NORMAL, 1	; do not use aggressive optimisation
	numdef SET_CLUSTER,	1	; if to set first_cluster (word [ss:bp - 16])
	numdef ZERO_ES,		0	; if to set es = 0 before jump
	numdef ZERO_DS,		0	; if to set ds = 0 before jump

	numdef FIX_SECTOR_SIZE, 0	; fix sector size (0 = disable, else = sector size)
	numdef FIX_SECTOR_SIZE_SKIP_CHECK,	0	; don't check sector size
	numdef FIX_CLUSTER_SIZE,		0	; fix cluster size
	numdef FIX_CLUSTER_SIZE_SKIP_CHECK,	0	; don't check cluster size
	numdef NO_LIMIT,	0	; allow using more memory than a boot sector
					;  also will not write 0AA55h signature!
	numdef WARN_PART_SIZE,	0

	numdef LBA_SKIP_CHECK,	1	; don't use proper LBA extensions check
	numdef LBA_SKIP_CY,	1	; skip check: set up CY before 13.42
	numdef LBA_SKIP_ANY,	0	; skip check: try CHS on any error
	incdef _LBA_SKIP_ANY, LBA_SKIP_CY
	numdef LBA_RETRY,	0	; retry LBA reads one time
	numdef CHS_RETRY,	1	; retry CHS reads one time
	numdef CHS_RETRY_REPEAT,16	; retry CHS reads multiple times
					; (value of the def is used as count)
	numdef CHS_RETRY_NORMAL,1	; do not use aggressive optimisation
	numdef RETRY_RESET,	1	; call reset disk system 13.00 on retries

	numdef MEDIAID, 0F0h		; media ID
	numdef UNIT, 0			; load unit in BPB
	numdef CHS_SECTORS, 18		; CHS geometry field for sectors
	numdef CHS_HEADS, 2		; CHS geometry field for heads
	numdef HIDDEN, 0		; number of hidden sectors
	numdef SPI, 2880		; sectors per image
	numdef BPS, 512			; bytes per sector
	numdef SPC, 1			; sectors per cluster
	numdef SPF, 9			; sectors per FAT
	numdef NUMFATS, 2		; number of FATs
	numdef NUMROOT, 224		; number of root directory entries
	numdef NUMRESERVED, 1		; number of reserved sectors

%if _FAT16
		; Unlike the 1440 KiB diskette image defaults for the FAT12
		;  loader we just fill the FAT16 BPB with zeros by default.
	numdef MEDIAID, 0		; media ID
	numdef UNIT, 0			; load unit in BPB
	numdef CHS_SECTORS, 0		; CHS geometry field for sectors
	numdef CHS_HEADS, 0		; CHS geometry field for heads
	numdef HIDDEN, 0		; number of hidden sectors
	numdef SPI, 0			; sectors per image
	numdef BPS, 0			; bytes per sector
	numdef SPC, 0			; sectors per cluster
	numdef SPF, 0			; sectors per FAT
	numdef NUMFATS, 0		; number of FATs
	numdef NUMROOT, 0		; number of root directory entries
	numdef NUMRESERVED, 0		; number of reserved sectors
%endif

%if _DIRBUF_500
	gendef _ADR_DIRBUF, 500h
%endif


	numdef COMPAT_FREEDOS,	0	; partial FreeDOS load compatibility
	numdef COMPAT_IBM,	0	; partial IBMDOS load compatibility
	numdef COMPAT_MS7,	0	; partial MS-DOS 7 load compatibility
	numdef COMPAT_MS6,	0	; partial MS-DOS 6 load compatibility
	numdef COMPAT_LDOS,	0	; lDOS load compatibility
	numdef COMPAT_KERNEL7E, 0	; kernel at 0:7E00h load compatibility

%if (!!_COMPAT_FREEDOS + !!_COMPAT_IBM + \
	!!_COMPAT_MS7 + !!_COMPAT_MS6 + \
	!!_COMPAT_LDOS + !!_COMPAT_KERNEL7E) > 1
 %error At most one set must be selected.
%endif

%if _COMPAT_FREEDOS
	strdef LOAD_NAME,	"KERNEL"
	strdef LOAD_EXT,	"SYS"
	numdef LOAD_ADR,	00600h
	numdef LOAD_MIN_PARA,	paras(512)
	numdef EXEC_SEG_ADJ,	0
	numdef EXEC_OFS,	0

	numdef CHECKVALUE,	0
	numdef SET_LOAD_SEG,	0
	numdef SET_FAT_SEG,	0
	numdef SET_CLUSTER,	0

	numdef SET_BL_UNIT,	1
	numdef MEMORY_CONTINUE,	0
	numdef RELOCATE,	1
	; The FreeDOS load protocol mandates that the entire file be loaded.
%endif

%if _COMPAT_IBM
	strdef LOAD_NAME,	"IBMBIO"
	strdef LOAD_EXT,	"COM"
	numdef LOAD_ADR,	00700h
	numdef LOAD_MIN_PARA,	paras(512)
	numdef EXEC_SEG_ADJ,	0
	numdef EXEC_OFS,	0
	numdef LOAD_DIR_SEG,	50h
	numdef ADD_SEARCH,	1
	strdef ADD_NAME,	"IBMDOS"
	strdef ADD_EXT,		"COM"
	numdef ADD_DIR_SEG,	52h
 	; Note: The IBMBIO.COM directory entry must be stored at
	;  0:500h, and the IBMDOS.COM directory entry at 0:520h.

	numdef CHECKVALUE,	0
	numdef SET_LOAD_SEG,	0
	numdef SET_FAT_SEG,	0
	numdef SET_CLUSTER,	0

	numdef SET_DL_UNIT,	1
	numdef MEMORY_CONTINUE,	1
	; 3 sectors * 512 BpS should suffice. We load into 700h--7A00h,
	;  ie >= 6000h bytes (24 KiB), <= 7300h bytes (28.75 KiB).
	numdef SET_AXBX_DATA,	1
	numdef DATASTART_HIDDEN,1
	numdef SET_DSSI_DPT,	1
	numdef PUSH_DPT,	1
%endif

%if _COMPAT_MS7
	strdef LOAD_NAME,	"IO"
	strdef LOAD_EXT,	"SYS"
	numdef LOAD_ADR,	00700h
	numdef LOAD_MIN_PARA,	paras(1024)
	numdef EXEC_SEG_ADJ,	0
	numdef EXEC_OFS,	200h

	numdef CHECKVALUE,	0
	numdef SET_LOAD_SEG,	0
	numdef SET_FAT_SEG,	0
	numdef SET_CLUSTER,	0

	numdef SET_DL_UNIT,	1
	numdef SET_DSSI_DPT,	0
	numdef PUSH_DPT,	1
	numdef MEMORY_CONTINUE,	1
	; 4 sectors * 512 BpS should suffice. We load into 700h--7A00h,
	;  ie >= 6000h bytes (24 KiB), <= 7300h bytes (28.75 KiB).
	numdef SET_DI_CLUSTER,	1
	numdef DATASTART_HIDDEN,1
	numdef LBA_SET_TYPE,	1
%endif

%if _COMPAT_MS6
	strdef LOAD_NAME,	"IO"
	strdef LOAD_EXT,	"SYS"
	numdef LOAD_ADR,	00700h
	numdef LOAD_MIN_PARA,	paras(512)
	numdef EXEC_SEG_ADJ,	0
	numdef EXEC_OFS,	0
	numdef LOAD_DIR_SEG,	50h
	numdef ADD_SEARCH,	1
	strdef ADD_NAME,	"MSDOS"
	strdef ADD_EXT,		"SYS"
	numdef ADD_DIR_SEG,	52h
 	; Note: The IO.SYS directory entry must be stored at
	;  0:500h, and the MSDOS.SYS directory entry at 0:520h.

	numdef CHECKVALUE,	0
	numdef SET_LOAD_SEG,	0
	numdef SET_FAT_SEG,	0
	numdef SET_CLUSTER,	0

	numdef SET_DL_UNIT,	1
	numdef MEMORY_CONTINUE,	1
	; 3 sectors * 512 BpS should suffice. We load into 700h--7A00h,
	;  ie >= 6000h bytes (24 KiB), <= 7300h bytes (28.75 KiB).
	numdef SET_AXBX_DATA,	1
	numdef DATASTART_HIDDEN,1
	numdef SET_DSSI_DPT,	1
	numdef PUSH_DPT,	1
%endif

%if _COMPAT_LDOS
	strdef LOAD_NAME,	"LDOS"
	strdef LOAD_EXT,	"COM"
	numdef LOAD_ADR,	02000h
	numdef LOAD_MIN_PARA,	paras(4096)
	numdef EXEC_SEG_ADJ,	0
	numdef EXEC_OFS,	400h
	numdef CHECKOFFSET,	1020
	numdef CHECKVALUE,	"lD"

	numdef SET_DL_UNIT,	0
	numdef SET_CLUSTER,	1
	numdef SET_FAT_SEG,	1
	numdef SET_LOAD_SEG,	1
	numdef MEMORY_CONTINUE,	1
	numdef DATASTART_HIDDEN,0
%endif

%if _COMPAT_KERNEL7E
	strdef OEM_NAME,	"KERNEL7E"
	strdef LOAD_NAME,	"KERNEL7E"
	strdef LOAD_EXT,	"BIN"
	numdef LOAD_ADR,	07E00h
	numdef LOAD_MIN_PARA,	paras(512)
	numdef EXEC_SEG_ADJ,	-7E0h
	numdef EXEC_OFS,	7E00h
	numdef CHECKVALUE,	0

	numdef SET_DL_UNIT,	1
	numdef SET_BL_UNIT,	0
	numdef SET_CLUSTER,	0
	numdef SET_FAT_SEG,	0
	numdef SET_LOAD_SEG,	0
	numdef MEMORY_CONTINUE,	0
	numdef DATASTART_HIDDEN,0

	gendef _ADR_FATBUF,	4000h
	gendef _ADR_DIRBUF,	4000h
	numdef RPL,		0
	numdef ZERO_ES,		1
%endif


%if 0

Notes about partial load compatibilities

* FreeDOS:
 * Relocates to an address other than 27A00h (1FE0h:7C00h)
 * A lot of options between _USE_PART_INFO, _QUERY_GEOMETRY, _CHS, _LBA,
   and/or _RPL need to be disabled to make the loader fit
* IBMDOS:
* MS-DOS 6:
 * Does not actually relocate DPT, just provide its address
 * A lot of options between _USE_PART_INFO, _QUERY_GEOMETRY, _CHS,
   and/or _LBA need to be disabled to make the loader fit
* MS-DOS 7:
 * Does not actually relocate DPT, just provide its address
 * Does not contain message table used by loader

%endif

%if _SET_BL_UNIT && _SET_AXBX_DATA
 %error Cannot select both of these options!
%endif

%if _DIR_ENTRY_520
 %assign _DIR_ENTRY_500		1
%endif

%if _DIRBUF_500 && _ADD_SEARCH
 %error Cannot select both of these options!
%endif

%if _ADD_SEARCH
 %if _LOAD_DIR_SEG == 0 || _ADD_DIR_SEG == 0
  %error Assuming dir segs should be set if add search set
 %endif
%endif

%if _RPL
 %assign _RPL_GRACE_AREA 0
%endif


%assign LOADLIMIT 0A0000h
%assign POSITION   07C00h

%if _FIX_SECTOR_SIZE
 %assign i 5
 %rep 13-5
  %if (1 << i) != (_FIX_SECTOR_SIZE)
   %assign i i+1
  %endif
 %endrep
 %if (1 << i) != (_FIX_SECTOR_SIZE)
  %error Invalid sector size _FIX_SECTOR_SIZE
 %endif
%endif

%if _FIX_CLUSTER_SIZE
 %if _FIX_CLUSTER_SIZE > 256
  %error Invalid cluster size _FIX_CLUSTER_SIZE
 %endif
 %assign i 0
 %rep 8-0
  %if (1 << i) != (_FIX_CLUSTER_SIZE)
   %assign i i+1
  %endif
 %endrep
 %if (1 << i) != (_FIX_CLUSTER_SIZE)
  %warning Non-power-of-two cluster size _FIX_CLUSTER_SIZE
 %endif
%endif


; 512-byte stack (minus the variables).
ADR_STACK_LOW	equ	7C00h - 200h		; 07A00h

%if _DIRBUF_500
	gendef _ADR_DIRBUF,	500h
%elif _RELOCATE
	gendef _ADR_DIRBUF,	_LOAD_ADR
%endif

; one-sector directory buffer. Assumes sectors are no larger than 8 KiB
ADR_DIRBUF	equ	__ADR_DIRBUF

%if ! _RELOCATE
; this used to be a two-sector FAT buffer -- two sectors because FAT12
;  entries are 12 bits and may straddle a sector boundary.
; however, with the FAT12 loaded completely, the buffer only needs to hold
;  one 8 KiB sector, two 4 KiB sectors, three 2 KiB sectors, six 1 KiB sectors,
;  or twelve 512 byte sectors.
; this shares its area with the directory buffer as they
;  are not simultaneously used. (if not _DIRBUF_500.)
ADR_FATBUF	equ	__ADR_FATBUF
%endif

; start of unused memory after loader:
ADR_END		equ	end -start+7C00h
%if ! _RELOCATE
 %if (ADR_FATBUF + 8192) > ADR_END
  ADR_FREE_FROM	equ	(ADR_FATBUF + 8192)	; 09E00h
 %else
  ADR_FREE_FROM	equ	ADR_END			; 07E00h
 %endif

; end of unused memory before loader:
 %if ADR_FATBUF < ADR_STACK_LOW
ADR_FREE_UNTIL	equ	ADR_FATBUF
 %else
ADR_FREE_UNTIL	equ	ADR_STACK_LOW
 %endif

 %if ((ADR_FATBUF + 8192 - 1) & ~0FFFFh) != (ADR_FATBUF & ~0FFFFh)
  %warning Possibly crossing 64 KiB boundary while reading FAT
 %endif
%endif

%if ((ADR_DIRBUF + 8192 - 1) & ~0FFFFh) != (ADR_DIRBUF & ~0FFFFh)
 %warning Possibly crossing 64 KiB boundary while reading directory
%endif

%if _RELOCATE
 ADR_FREE_FROM	equ 0				; make next conditional true
 ADR_FREE_UNTIL	equ 0
%endif
%if _LOAD_ADR >= ADR_FREE_FROM
	; If reading on a sector size boundary, no crossing can occur.
	;  Check for all possible sector sizes (32 B to 8 KiB). If one
	;  of them fails display a warning, including the minimum size.
 %assign SECSIZECHECK 32
 %assign EXITREP 0
 %rep 256
  %ifn EXITREP
   %if _LOAD_ADR & (SECSIZECHECK - 1)
    %warning Possibly crossing 64 KiB boundary while reading file (sector size >= SECSIZECHECK)
    %assign EXITREP 1
    %exitrep
   %endif
   %if SECSIZECHECK == 8192
    %assign EXITREP 1
    %exitrep
   %endif
   %assign SECSIZECHECK SECSIZECHECK * 2
  %endif
 %endrep
%else
	; If loading below the boot sector, address 1_0000h is never reached.
%endif


%if (_LOAD_ADR & 0Fh)
 %error Load address must be on a paragraph boundary
%endif

%if _LOAD_ADR > LOADLIMIT
 %error Load address must be in LMA
%elif _LOAD_ADR < 00500h
 %error Load address must not overlap IVT or BDA
%endif

%if ! _RELOCATE
 %if _LOAD_ADR > (POSITION-512) && _LOAD_ADR < (POSITION+512)
  %error Load address must not overlap loader
 %endif

 %if ADR_FATBUF > LOADLIMIT
  %error FAT buffer address must be in LMA
 %elif ADR_FATBUF < 00500h
  %error FAT buffer address must not overlap IVT or BDA
 %elif (ADR_FATBUF + 8192) > (POSITION-512) && ADR_FATBUF < (POSITION+512)
  %error FAT buffer address must not overlap loader
 %endif
%endif

%if ADR_DIRBUF > LOADLIMIT
 %error Dir buffer address must be in LMA
%elif ADR_DIRBUF < 00500h
 %error Dir buffer address must not overlap IVT or BDA
%elif (ADR_DIRBUF + 8192) > (POSITION-512) && ADR_DIRBUF < (POSITION+512)
 %error Dir buffer address must not overlap loader at initial position
%endif

%if ((_EXEC_SEG_ADJ<<4)+_EXEC_OFS) < 0
 %error Execution address must be in loaded file
%elif ((_EXEC_SEG_ADJ<<4)+_EXEC_OFS+_LOAD_ADR) > LOADLIMIT
 %error Execution address must be in LMA
%endif

%if (_EXEC_OFS & ~0FFFFh)
 %error Execution offset must fit into 16 bits
%endif

%if (_EXEC_SEG_ADJ > 0FFFFh || _EXEC_SEG_ADJ < -0FFFFh)
 %error Execution segment adjustment must fit into 16 bits
%endif


%if !_CHS && _QUERY_GEOMETRY
 %warning No CHS support but querying geometry anyway
%endif

%if !_CHS && !_LBA
 %error Either CHS or LBA or both must be enabled
%endif


%if 0

There is some logic inside MS-DOS's hard disk partition initialisation
code that sets up several requirements for us to fulfil. Otherwise,
it will not accept the information given in the BPB (using default
information based on the length as specified by MBR/EPBR instead) or
make the whole file system inaccessible except for formatting. Both of
those are undesirable of course. Some/all(?) checks are documented on
pages 601,602 in "DOS Internals", Geoff Chappell 1994, as follows:

* First three bytes contain either "jmp sho xx\ nop" or "jmp ne xx".
* Media descriptor field >= 0F0h.
* Bytes per sector field == 512.
* Sectors per cluster field a power of 2 (1,2,4,8,16,32,64,128).
* OEM name "version" (last three to four characters)
 * must be "20.?", "10.?" (? = wildcard), but no other with "0.?",
 * otherwise, must be "2.0", or
  * 2nd-to-last,3rd-to-last character codes together > "3.", or
   * those == "3.", last character code > "0"

To stay compatible to those, display a warning here if the name
itself would disqualify our boot sector already.

%endif

%push
%strlen %$len _OEM_NAME_FILL
%if %$len != 1
 %error Specified OEM name fill must be 1 character
%endif
%strlen %$len _OEM_NAME
%define %$nam _OEM_NAME
%if %$len > 8
 %error Specified OEM name is too long
%else
 %assign %$warn 0
 %rep 8 - %$len
  %strcat %$nam %$nam,_OEM_NAME_FILL
 %endrep
 %substr %$prefix %$nam	5	; "wvxyZa.b", get "Z"
 %substr %$major %$nam 6,7	; "wvxyzA.b", get "A."
 %substr %$minor %$nam 8	; "wvxyza.B", get "B"
 %if %$major == "0."
  %ifn %$prefix == "1" || %$prefix == "2"
   %assign %$warn 1
  %endif
 %elifn %$major == "2." && %$minor == "0"
  %if %$major < "3."
   %assign %$warn 1
  %elif %$major == "3." && %$minor < "1"
   %assign %$warn 1
  %endif
 %endif
 %if %$warn
  %warning Specified OEM name fails MS-DOS's validation
 %endif
%endif
%pop


	struc DIRENTRY
deName:		resb 8
deExt:		resb 3
deAttrib:	resb 1
		resb 8
deClusterHigh:	resw 1
deTime:		resw 1
deDate:		resw 1
deClusterLow:	resw 1
deSize:		resd 1
	endstruc

ATTR_READONLY	equ 1
ATTR_HIDDEN	equ 2
ATTR_SYSTEM	equ 4
ATTR_VOLLABEL	equ 8
ATTR_DIRECTORY	equ 10h
ATTR_ARCHIVE	equ 20h


; use byte-offset addressing from BP for smaller code
%define	VAR(x)	((x) - start) + bp


	cpu 8086
; bootsector loaded at address 07C00h, addressable using 0000h:7C00h
	org POSITION
start:


%define _LASTVARIABLE start
	%macro nextvariable 2-3.nolist
%1	equ (_LASTVARIABLE - %2)
%define _LASTVARIABLE %1
%ifidn %3, relocatestart
 %define _RELOCATESTART %1
%elifempty %3
%else
 %error Invalid third parameter
%endif
	%endmacro

; Variables

; (dword) sector where the first cluster's data starts
	nextvariable data_start, 4

; (word) current load segment (points behind last loaded data)
	nextvariable load_seg, 2

; (word) segment of FAT buffer
; for FAT12 this holds the entire FAT
; for FAT16 this holds the sector given by wo[fat_sector]
; for FAT32 this holds the sector given by dwo[fat_sector]
	nextvariable fat_seg, 2

; (word for FAT16) currently loaded sector-in-FAT, -1 if none
	nextvariable fat_sector, 4

; (word for FAT12/FAT16) first cluster of load file
	nextvariable first_cluster, 4, relocatestart

ADR_STACK_START	equ	_LASTVARIABLE -start+POSITION

%ifn _FIX_SECTOR_SIZE
; (word) number of 16-byte paragraphs per sector
	nextvariable para_per_sector, 2, relocatestart
%endif

%assign DIRSEARCHSTACK_CL_FIRST 0
%assign DIRSEARCHSTACK_CL_SECOND 0
%assign PLACEHOLDER 0

 %if _ATTRIB_SAVE && ! (_ADD_SEARCH || _LOAD_DIR_SEG)
   %if _LASTVARIABLE == start - 12h
    %assign DIRSEARCHSTACK_CL_FIRST 1
   %elif _LASTVARIABLE == start - 10h
    %assign DIRSEARCHSTACK_CL_SECOND 1
   %endif
   %ifn _DIR_ENTRY_500
; three words left on the stack after directory search
	nextvariable dirsearchstack, 6, relocatestart
   %else
; two words left on the stack after directory search
	nextvariable dirsearchstack, 4, relocatestart
   %endif
 %elifn !_RELOCATE && _LOAD_ADR < ADR_FREE_UNTIL
  %if _LASTVARIABLE == start - 12h
	nextvariable cmdline_signature_placeholder, 2, relocatestart
   %assign PLACEHOLDER 1
  %elif _LASTVARIABLE == start - 10h
   %if _PUSH_DPT
	nextvariable cmdline_signature_placeholder, 4, relocatestart
    %assign PLACEHOLDER 2
	; In this case, part of the original DPT pointer may
	;  overlap the CL signature word. Therefore allocate
	;  two placeholder words to insure no CL match.
   %else
	; In this case the last_available_sector variable
	;  will be at word [ss:bp - 12h] (or none) and the
	;  stack pointer will be equal to bp - 12h (or - 10h)
	;  at handover time. Thus no placeholder is needed.
   %endif
  %else
   %error Placeholder not placed
  %endif
		; This stack slot is used to insure that
		;  the "CL" signature is not present at this
		;  location. If not relocate and load address
		;  is below loader then the next variable
		;  (last_available_sector) will always receive
		;  a value < 7C0h so cannot hold "CL".
		; If _ATTRIB_SAVE is in use and neither the
		;  _ADD_SEARCH nor the _LOAD_DIR_SEG options
		;  are set, the first word of dirsearchstack
		;  will be at word [ss:bp - 14h].
 %endif

%ifn ! _RELOCATE && _LOAD_ADR < ADR_FREE_UNTIL && _FIX_SECTOR_SIZE
; (word) segment of last available memory for sector
	nextvariable last_available_sector, 2
%else
  %if _LASTVARIABLE == start - 12h
	nextvariable cmdline_signature_placeholder, 2, relocatestart
   %assign PLACEHOLDER 1
  %elif _LASTVARIABLE == start - 10h
   %if _PUSH_DPT
	nextvariable cmdline_signature_placeholder, 4, relocatestart
    %assign PLACEHOLDER 2
   %endif
  %endif
%endif

lowest_variable		equ _LASTVARIABLE


	jmp strict short skip_bpb
%if !_CHS && _LBA_SET_TYPE
	db 0Eh		; LBA-enabled FAT16 FS partition type
%else
	nop		; default: no LBA
%endif


; BIOS Parameter Block (BPB)
;
; Installation will use the BPB already present in your file system.
; These values must be initialised when installing the loader.
;
; The values shown here work only with 1440 KiB disks (CHS=80:2:18)

oem_id:			; offset 03h (03) - not used by this code
	fill 8,_OEM_NAME_FILL,db _OEM_NAME
bytes_per_sector:	; offset 0Bh (11) - refer to _FIX_SECTOR_SIZE !
	dw _BPS
sectors_per_cluster:	; offset 0Dh (13) - refer to _FIX_CLUSTER_SIZE !
	db _SPC & 255
fat_start:
num_reserved_sectors:	; offset 0Eh (14)
	dw _NUMRESERVED
num_fats:		; offset 10h (16)
	db _NUMFATS
num_root_dir_ents:	; offset 11h (17)
	dw _NUMROOT
total_sectors:		; offset 13h (19) - not used by this code
%if _SPI < 1_0000h
	dw _SPI
%else
	dw 0
%endif
media_id:		; offset 15h (21) - not used by this code
	db _MEDIAID
sectors_per_fat:	; offset 16h (22)
	dw _SPF
sectors_per_track:	; offset 18h (24)
	dw _CHS_SECTORS
heads:			; offset 1Ah (26)
	dw _CHS_HEADS
hidden_sectors:		; offset 1Ch (28)
	dd _HIDDEN
total_sectors_large:	; offset 20h (32) - not used by this code
%if _SPI >= 1_0000h
	dd _SPI
%else
	dd 0
%endif

; Extended BPB

boot_unit:		db _UNIT
			db 0
ext_bpb_signature:	db 29h
serial_number:		dd _VOLUMEID
volume_label:		fill 11,32,db _DEFAULT_LABEL
filesystem_identifier:	fill 8,32,db "FAT1",'2'+4*!!_FAT16


; Initialised data

load_name:
	fill 8,32,db _LOAD_NAME
	fill 3,32,db _LOAD_EXT
%if _ADD_SEARCH
add_name:
	fill 8,32,db _ADD_NAME
	fill 3,32,db _ADD_EXT

	; align 2
	;  This happens to be aligned anyway. But even if
	;  it didn't, we'd rather save that byte than use
	;  it to align these fields. So comment this out.
dirseg:
	dw _ADD_DIR_SEG
%endif


	numdef TMPINC, 0

%if _TMPINC
 [list -]
%endif
		%imacro errorhandler 0
%if _TMPINC
 %include "error.tmp"
 [list -]
%else
; === error.tmp ===
error_start:

read_sector.err:
	mov al, 'R'	; Disk 'R'ead error
%if ! _MEMORY_CONTINUE || _RELOCATE || _LOAD_ADR >= ADR_FREE_FROM
	db __TEST_IMM16	; (skip mov)
error_filetoobig:
error_memory:
	mov al,'M'	; Not enough 'M'emory
%endif

error:
%if _RELOCATE
	mov bx, 7
	mov ds, bx
	mov bh, [462h - 70h]
%else
	mov bh, [462h]
	mov bl, 7
%endif
	mov ah, 0Eh
	int 10h		; display character
	mov al, 07h
	int 10h		; beep!

	xor ax, ax	; await key pressed
	int 16h

	int 19h		; re-start the boot process

%if _WARN_PART_SIZE
 %assign num $ - error_start
 %warning error size is num bytes
%endif
; === eof ===
%endif
%if _TMPINC
 [list +]
%endif
		%endmacro


		%imacro readhandler 0
%if _TMPINC
 %include "read.tmp"
 [list -]
%else
; === read.tmp ===
read_sector_start:
		; INP:	dx:ax = sector
		; OUT:	only if successful
		;	dx:ax = incremented
		;	bx => behind read sector
		;	es = ADR_FATBUF>>4 = ADR_DIRBUF>>4
		; CHG:	-
%if ! _RELOCATE
 %if ADR_DIRBUF == ADR_FATBUF
read_sector_dirbuf:
 %endif
 %if _FAT16 && ! _LOAD_NON_FAT
read_sector_fatbuf:
 %endif
 %if (ADR_DIRBUF == ADR_FATBUF) || (_FAT16 && ! _LOAD_NON_FAT)
	mov bx, ADR_FATBUF>>4
  %if _FAT16 && _SET_FAT_SEG && ! _LOAD_NON_FAT
	mov word [VAR(fat_seg)], bx
		; Optimisation: Set FAT buffer segment here where
		;  we have it ready in a register, instead of
		;  wasting a word immediate on it. If the FAT is
		;  never read then we do not need to set the
		;  variable anyway, only the sector variable has
		;  to contain a -1 to indicate it's uninitialised.
		; If we get here from read_sector_dirbuf we will
		;  also initialise this variable but that does not
		;  cause any problems.
  %endif
 %endif
%endif

		; Read a sector using Int13.02 or Int13.42
		;
		; INP:	dx:ax = sector number within partition
		;	bx => buffer
		;	(_LBA) ds = ss
		; OUT:	If unable to read,
		;	 ! jumps to error instead of returning
		;	If sector has been read,
		;	 dx:ax = next sector number (has been incremented)
		;	 bx => next buffer (bx = es+word[para_per_sector])
		;	 es = input bx
		; CHG:	-
read_sector:
	push dx
	push cx
	push ax
	push si

	mov es, bx	; => buffer

; DX:AX==LBA sector number
; add partition start (= number of hidden sectors)
		add ax,[VAR(hidden_sectors + 0)]
		adc dx,[VAR(hidden_sectors + 2)]
 %if (!_LBA || !_LBA_33_BIT) && _LBA_CHECK_NO_33
	jc .err
 %endif
%if _LBA		; +70 bytes (with CHS, +63 bytes without CHS)
 %if _LBA_33_BIT
	sbb si, si	; -1 if was CY, 0 else
	neg si		; 1 if was CY, 0 else
 %endif
	xor cx, cx
	push cx		; highest word = 0
 %if _LBA_33_BIT
	push si		; bit 32 = 1 if operating in 33-bit space
 %else
	push cx		; second highest word = 0
 %endif
	push dx
	push ax		; = qword sector number
	push bx
	push cx		; bx => buffer
	inc cx
	push cx		; word number of sectors to read
	mov cl, 10h
	push cx		; word size of disk address packet
	mov si, sp	; ds:si -> disk address packet (on stack)

 %if _LBA_SKIP_CHECK		; -14 bytes
	mov dl, [VAR(boot_unit)]
 %else
	mov ah, 41h
	mov dl, [VAR(boot_unit)]
	mov bx, 55AAh
	stc
	int 13h		; 13.41.bx=55AA extensions installation check
	jc .no_lba
	cmp bx, 0AA55h
	jne .no_lba
	shr cl, 1	; support bitmap bit 0
	jnc .no_lba
 %endif

%if _LBA_RETRY
 %if _LBA_SKIP_CHECK && _LBA_SKIP_CY
	stc
 %endif
	mov ah, 42h
	int 13h		; 13.42 extensions read
	jnc .lba_done

 %if _RETRY_RESET
	xor ax, ax
	int 13h		; reset disk
 %endif

		; have to reset the LBAPACKET's lpCount, as the handler may
		;  set it to "the number of blocks successfully transferred".
		; (in any case, the high byte is still zero.)
	mov byte [si + 2], 1
%endif

 %if _LBA_SKIP_CHECK && _LBA_SKIP_CY
	stc
 %endif
	mov ah, 42h
	int 13h
 %if _LBA_SKIP_CHECK && _CHS
  %if _LBA_SKIP_ANY
	jc .no_lba
  %else
	jc .lba_check_error_1
  %endif
 %else
.cy_err:
	jc .lba_error
 %endif

.lba_done:
%if _CHS && _LBA_SET_TYPE
	mov byte [bp + 2], 0Eh	; LBA-enabled FAT16 FS partition type
%endif
	add sp, 10h
%if _CHS
	jmp short .done
%endif

.lba_error: equ .err

 %if !_CHS
.no_lba: equ .err
 %else
 %if _LBA_SKIP_CHECK
  %if ! _LBA_SKIP_ANY
.lba_check_error_1:
	cmp ah, 1	; invalid function?
	jne .lba_error	; no, other error -->
			; try CHS instead
  %endif
.cy_err: equ .err
 %endif
.no_lba:
	add sp, 8
	pop cx		; cx = low word of sector
	pop ax
	pop dx		; dx:ax = middle two words of sector
		; Here dx <= 1 if _LBA_33_BIT, else zero.
		;  If dx is nonzero then the CHS calculation
		;  should fail. If CHS sectors is equal to 1
		;  (very unusual) then the div may fail. Else,
		;  we will detect a cylinder > 1023 eventually.
	pop si		; discard highest word of qword
 %endif
%endif

%if !_LBA
.cy_err:	equ .err
%endif

%if _CHS		; +70 bytes
; dx:ax = LBA sector number, (if _LBA) cx = 0
; divide by number of sectors per track to get sector number
; Use 32:16 DIV instead of 64:32 DIV for 8088 compatability
; Use two-step 32:16 divide to avoid overflow
 %if !_LBA
			xchg cx, ax	; cx = low word of sector, clobbers ax
			xchg ax, dx	; ax = high word of sector, clobbers dx
			xor dx, dx	; dx:ax = high word of sector
 %else
	; from the .no_lba popping we already have:
	;  cx = low word of sector
	;  dx:ax = high word of sector
 %endif
			div word [VAR(sectors_per_track)]
			xchg cx,ax
			div word [VAR(sectors_per_track)]
			xchg cx,dx

; DX:AX=quotient, CX=remainder=sector (S) - 1
; divide quotient by number of heads
			xchg bx, ax	; bx = low word of quotient, clobbers ax
			xchg ax, dx	; ax = high word of quotient, clobbers dx
			xor dx, dx	; dx = 0
			div word [VAR(heads)]
					; ax = high / heads, dx = high % heads
			xchg bx, ax	; bx = high / heads, ax = low quotient
			div word [VAR(heads)]

; bx:ax=quotient=cylinder (C), dx=remainder=head (H)
; move variables into registers for INT 13h AH=02h
			mov dh, dl	; dh = head
			inc cx		; cl5:0 = sector
			xchg ch, al	; ch = cylinder 7:0, al = 0
			shr ax, 1
			shr ax, 1	; al7:6 = cylinder 9:8
	; bx has bits set iff it's > 0, indicating a cylinder >= 65536.
			 or bl, bh	; collect set bits from bh
			or cl, al	; cl7:6 = cylinder 9:8
	; ah has bits set iff it was >= 4, indicating a cylinder >= 1024.
			 or bl, ah	; collect set bits from ah
			mov dl,[VAR(boot_unit)] ; dl = drive
.nz_err:
			 jnz .err	; error if cylinder >= 1024 -->
					; ! bx = 0 (for 13.02 call)

; we call INT 13h AH=02h once for each sector. Multi-sector reads
; may fail if we cross a track or 64K boundary
%if _CHS_RETRY_REPEAT
			mov si, _CHS_RETRY_REPEAT + 1
 %if _CHS_RETRY_NORMAL && _RETRY_RESET
			db __TEST_IMM16	; (skip int 13h)
.loop_chs_retry_repeat:
			int 13h		; reset disk
 %elif _RETRY_RESET
.loop_chs_retry_repeat:
			xor ax, ax
			int 13h		; reset disk
 %else
.loop_chs_retry_repeat:
 %endif
			dec si		; another attempt ?
			js .nz_err	; no -->
			mov ax, 0201h
			int 13h		; read one sector
 %if _CHS_RETRY_NORMAL && _RETRY_RESET
			mov ax, bx	; ax = 0
 %endif
			jc .loop_chs_retry_repeat
	; fall through to .done
%else
			mov ax, 0201h
 %if _CHS_RETRY
  %if _RETRY_RESET
	; In this case we cannot store to the stack and
	;  pop the value at the right moment for both
	;  cases of the "jnc .done" branch. So use the
	;  original code to re-init ax to 0201h.
			int 13h		; read one sector
			jnc .done
; reset drive
			xor ax, ax
			int 13h
			mov ax, 0201h
  %else
			push ax
			int 13h		; read one sector
			pop ax		; restore ax = 0201h
			jnc .done
  %endif
 %endif
; try read again
			int 13h
 %if _LBA_SKIP_CHECK
			inc bx
			jc .nz_err
 %else
			jc .cy_err
 %endif
%endif

%endif		; _CHS

.done:
; increment segment
	mov bx, es
%if _FIX_SECTOR_SIZE
	add bx, _FIX_SECTOR_SIZE >> 4
%else
	add bx, [VAR(para_per_sector)]
%endif

	pop si
	pop ax
	pop cx
	pop dx
; increment LBA sector number
	inc ax
	jne @F
	inc dx
@@:

	retn

%if _WARN_PART_SIZE
 %assign num $ - read_sector_start
 %warning read_sector size is num bytes
%endif
; === eof ===
%endif
%if _TMPINC
 [list +]
%endif
		%endmacro
%if _TMPINC
 [list +]
%endif

%if _WARN_PART_SIZE
 %assign num $ - start
 %warning BPB + data size is num bytes
%endif


; Code

skip_bpb:
	cli
	cld
	 xor cx, cx
	mov bp, start		; magic bytes - checked by instsect
	 mov ss, cx
	mov sp, ADR_STACK_START
%if _USE_AUTO_UNIT
	mov [VAR(boot_unit)], dl; magic bytes - checked by instsect
%else
	mov dl, [VAR(boot_unit)]; magic bytes - checked by instsect
%endif

	; Note:	es is left uninitialised here until the first call to
	;	 read_sector if the below conditional is false.
%if _USE_PART_INFO	; +19 bytes
	 mov es, cx
; Note:	Award Medallion BIOS v6.0 (ASUS MED 2001 ACPI BIOS Revision 1009)
;	 loads from a floppy disk drive with ds:si = 0F000h:0A92Dh ->
;	 FF FF FF FF 08 00 08 01 FF FF FF FF FF FF FF FF, which was detected
;	 as a valid partition table entry by this handling. Therefore, we
;	 only accept partition information when booting from a hard disk now.

		; start of magic byte sequence for instsect
	test dl, dl		; floppy ?
	jns @F			; don't attempt detection -->
; Check whether an MBR left us partition information.
; byte[ds:si] bit 7 means active and must be set if valid.
	cmp byte [si], cl	; flags for xx-00h (result is xx), SF = bit 7
	jns @F			; xx < 80h, ie info invalid -->
; byte[ds:si+4] is the file system type. Check for valid one.
	cmp byte [si+4], cl	; is it zero?
	je @F			; yes, info invalid -->
; Info valid, trust their hidden sectors over hardcoded.
; Assume the movsw instructions won't run with si = FFFFh.
	mov di, hidden_sectors	; -> BPB field
	add si, 8		; -> partition start sector in info
 %if _USE_PART_INFO_DISABLED
	nop
	nop			; size has to match enabled code
 %else
	movsw
	movsw			; overwrite BPB field with value from info
 %endif
@@:
		; end of magic byte sequence for instsect
%endif
	mov ds, cx
	sti


%if _QUERY_GEOMETRY	; +27 bytes

		; start of magic byte sequence for instsect
;	test dl, dl		; floppy?
;	jns @F			; don't attempt query, might fail -->
	; Note that while the original PC BIOS doesn't support this function
	;  (for its diskettes), it does properly return the error code 01h.
	; https://sites.google.com/site/pcdosretro/ibmpcbios (IBM PC version 1)
	mov ah, 08h
	; xor cx, cx		; initialise cl to 0
	; Already from prologue cx = 0.
	stc			; initialise to CY
 %if _QUERY_GEOMETRY_DISABLED
	nop
	nop			; size has to match enabled code
 %else
	int 13h			; query drive geometry
 %endif
	jc @F			; apparently failed -->
	and cx, 3Fh		; get sectors
	jz @F			; invalid (S is 1-based), don't use -->
	mov [VAR(sectors_per_track)], cx
	mov cl, dh		; cx = maximum head number
	inc cx			; cx = number of heads (H is 0-based)
	mov [VAR(heads)], cx
@@:
		; end of magic byte sequence for instsect
%endif

%if _FIX_SECTOR_SIZE
 %if !_FIX_SECTOR_SIZE_SKIP_CHECK
	cmp word [VAR(bytes_per_sector)], _FIX_SECTOR_SIZE
	mov al, 'S'
	jne error
 %endif
	mov bx, _FIX_SECTOR_SIZE >> 5
 %if _FIX_CLUSTER_SIZE
  %if !_FIX_CLUSTER_SIZE_SKIP_CHECK
	cmp byte [VAR(sectors_per_cluster)], _FIX_CLUSTER_SIZE & 0FFh
	mov al, 'C'
	jne error
  %endif
 %endif
	mov ch, 0			; ! ch = 0
%else
; 16-byte paragraphs per sector
	mov bx,[VAR(bytes_per_sector)]
	mov cx,4			; ! ch = 0
	shr bx,cl
 %if _FIX_CLUSTER_SIZE
  %if !_FIX_CLUSTER_SIZE_SKIP_CHECK
	cmp byte [VAR(sectors_per_cluster)], _FIX_CLUSTER_SIZE & 0FFh
	mov al, 'C'
	jne error
  %endif
 %else
				; ! ch = 0
 %endif
	push bx				; push into word [VAR(para_per_sector)]

; 32-byte FAT directory entries per sector
	shr bx, 1			; /2 = 32-byte entries per sector
%endif

%if _WARN_PART_SIZE
 %assign num $ - skip_bpb
 %warning init size is num bytes
%endif


dirsearch_start:

; number of sectors used for root directory (store in CX)
	mov si, [VAR(num_root_dir_ents)]
	mov ax, bx
		; The ax value here is the last value of bx, which is set
		;  by the shr instruction. Therefore, it cannot be higher
		;  than 7FFFh, so this cwd instruction always zeros dx.
	cwd
	dec ax				; rounding up
	add ax, si			; from BPB
	adc dx, dx			; account for overflow (dx was zero)
	div bx				; get number of root sectors
	xchg ax, cx			; cx = number of root secs, ! ah = 0

; first sector of root directory
	mov al,[VAR(num_fats)]		; ! ah = 0, hence ax = number of FATs
	mul word [VAR(sectors_per_fat)]
	add ax,[VAR(num_reserved_sectors)]
	adc dl, dh			; account for overflow (dh was and is 0)

	xor di, di

; first sector of disk data area:
	add cx, ax
	adc di, dx
	mov [VAR(data_start)], cx
	mov [VAR(data_start+2)], di

next_dir_search:
%if _ADD_SEARCH
	push dx
	push ax
	push si
%endif

; Scan root directory for file. We don't bother to check for deleted
;  entries (E5h) or entries that mark the end of the directory (00h).
		; number of root entries in si here
next_sect:
	mov cx, bx		; entries per sector as loop counter
%if ! _RELOCATE
 %if ADR_DIRBUF == ADR_FATBUF
	call read_sector_dirbuf
 %else
	mov bx, ADR_DIRBUF>>4
	call read_sector
 %endif
%else
	mov bx, ADR_DIRBUF>>4
	call read_sector
%endif
	mov bx, cx		; restore bx for next iteration later

	xor di, di		; es:di-> first entry in this sector
next_ent:
 %if DIRSEARCHSTACK_CL_FIRST
	push cx			; first dirsearchstack word = entries-in-sector
	push si			; other: entries total
 %else
	push si
	push cx			; second dirsearchstack word = entries-in-sector
 %endif
	push di			; dirsearchstack
%if _CHECK_ATTRIB && ! _ATTRIB_SAVE
	test byte [es:di + deAttrib], ATTR_DIRECTORY | ATTR_VOLLABEL
	jnz @F			; directory, label, or LFN entry --> (NZ)
%endif
%if _ADD_SEARCH
	mov si, add_name
filename equ $ - 2		; SMC to update to load_name later
%else
	mov si, load_name	; ds:si-> name to match
%endif
	mov cx, 11		; length of padded 8.3 FAT filename
	repe cmpsb		; check entry
%if _ATTRIB_SAVE
 %if _CHECK_ATTRIB
	jnz @F
		; deAttrib == 11, right after the 11-byte name
	test byte [es:di], ATTR_DIRECTORY | ATTR_VOLLABEL
				; directory, label, or LFN entry ?
 %endif
	jz found_it		; found entry -->
%endif
@@:
	pop di
 %if DIRSEARCHSTACK_CL_FIRST
	pop si
	pop cx			; pop from dirsearchstack
 %else
	pop cx
	pop si			; pop from dirsearchstack
 %endif
	lea di, [di + DIRENTRY_size]
%if ! _ATTRIB_SAVE
	jz found_it		; found entry -->
%endif

	dec si			; count down entire root's entries
	loopnz next_ent		; count down sector's entries (jumps iff si >0 && cx >0)
	jnz next_sect		; (jumps iff si >0 && cx ==0)
				; ends up here iff si ==0
				;  ie all root entries checked unsuccessfully
%if 0

qemu prior to 2020-08 has a bug which affects the above
conditionals. The bug is that if NZ is set (like when the
branch to the label found_it is not taken) and then another
instruction sets ZR (like the dec si at the end of the root
directory) and then loopnz is used which sets cx to zero
then after the loopnz FL will be NZ leading to the jnz branch
to be taken. Eventually the entire load unit is traversed and
qemu returns error 01h when trying to read past the end of
the unit (at least for 1440 KiB diskettes).

The bug can be worked around in two ways as done by lDebug:

https://hg.pushbx.org/ecm/ldebug/rev/c95e2955bbca

https://hg.pushbx.org/ecm/ldebug/rev/c84047f15d9c

However, both cost a few bytes each. Therefore the proper
fix is considered to be updating qemu. Error behaviour occurs
when a file is not found. In an unlikely case, another sector
in the data area may hold a match for the searched entry.
Otherwise, the read eventually fails and the loader aborts
with an R error (instead of the expected F error).

Reference: https://bugs.launchpad.net/qemu/+bug/1888165

%endif

	mov al,'F'	; File not 'F'ound
	jmp error

found_it:
%if _ADD_SEARCH || _LOAD_DIR_SEG
 %if _ATTRIB_SAVE
	pop di			; es:di -> dir entry (pop from dirsearchstack)
 %endif
	mov cx, 32
	mov ax, _LOAD_DIR_SEG
 %if ! _ATTRIB_SAVE
	sub di, cx		; es:di -> dir entry
 %endif
 %if _ADD_SEARCH
	xchg ax, word [VAR(dirseg)]
 %endif
	push ds
	mov si, di
	push di
	 push es
	 pop ds			; ds:si -> dir entry
	mov es, ax
	xor di, di		; es:di -> destination
	rep movsb		; store dir entry
	 push ds
	 pop es
	pop di			; restore es:di -> dir entry
	pop ds
 %if _ADD_SEARCH
%if ((load_name - start) & 0FF00h) == ((add_name - start) & 0FF00h)
	mov byte [VAR(filename)], (load_name - start + 7C00h) & 255
%else
	mov word [VAR(filename)], load_name
%endif
				; update name to second iteration's
%if (_LOAD_DIR_SEG & 255) != (_ADD_DIR_SEG & 255)
	cmp al, _ADD_DIR_SEG & 255
%elif (_LOAD_DIR_SEG) != (_ADD_DIR_SEG)
	cmp ax, _ADD_DIR_SEG	; was first iteration ?
%else
 %error Must not store directory entries to same segment
%endif
 %if _ATTRIB_SAVE
	pop si			; discard cx/si
	pop si			; discard si/cx (dirsearchstack)
 %endif
	pop si
	pop ax
	pop dx			; restore root start and count
				;  (bx still holds entries per sector)
	je next_dir_search	; jump to search load file next -->
 %endif
	times PLACEHOLDER push bx
			; push into cmdline_signature_placeholder
 %if _RELOCATE
	push word [es:di + deClusterLow]
			; (word on stack) = first cluster number
 %endif
%else
 %if _DIR_ENTRY_500	; +24 bytes, probably
	mov cx, 32
 %if _ATTRIB_SAVE
	pop si		; es:si -> dir entry (pop from dirsearchstack)
 %else
	xchg si, di
	sub si, cx
 %endif
	 push ds
	 push es
	push es
	pop ds		; ds:si -> directory entry
	xor ax, ax
	mov es, ax
	mov di, 500h	; es:di -> 0:500h
  %if _DIR_ENTRY_520
	rep movsw	; move to here (two directory entries)
  %else
	rep movsb	; move to here
  %endif
	 pop es
	 pop ds
	xchg si, di	; es:di -> behind (second) directory entry
 %endif
	times PLACEHOLDER push bx
			; push into cmdline_signature_placeholder
		; Push the entries per sector value into this
		;  stack slot to ensure that it does not hold "CL".
 %if _RELOCATE
  %if _DIR_ENTRY_500 || !_ATTRIB_SAVE
	push word [es:di + deClusterLow - DIRENTRY_size \
		- (DIRENTRY_size * !!_DIR_ENTRY_520)]
			; (word on stack) = first cluster number
  %else
	push word [es:di + deClusterLow - (deName + 11)]
			; (word on stack) = first cluster number
  %endif
 %endif
%endif

%if _WARN_PART_SIZE
 %assign num $ - dirsearch_start
 %warning dirsearch size is num bytes
%endif


%if _RELOCATE || _LOAD_ADR >= ADR_FREE_FROM
memory_start:
; Get conventional memory size and store it
		int 12h
		mov cl, 6
		shl ax, cl
 %if _RPL		; +31 bytes
	xchg dx, ax
	lds si, [4 * 2Fh]
	add si, 3
	lodsb
	cmp al, 'R'
	jne .no_rpl
	lodsb
	cmp al, 'P'
	jne .no_rpl
	lodsb
	cmp al, 'L'
	jne .no_rpl
	mov ax, 4A06h
	int 2Fh
.no_rpl:
	push ss
	pop ds
	xchg ax, dx
 %endif
 %if _RELOCATE
  %if _LOAD_NON_FAT
	sub ax, (_RPL_GRACE_AREA + 20 * 1024 \
		+ 512 + 7C00h) >> 4
  %else
	sub ax, (_RPL_GRACE_AREA + 20 * 1024 \
		+ 8192 + (8192-16) + 512 + 7C00h) >> 4
  %endif
		; RPL grace	area preserved for RPL
		; 20 KiB:	reserved for iniload
		; 8 KiB:	FAT buffer
		; 8 KiB - 16 B:	to allow rounding down FAT buffer position
		; 512:		sector
		; 7C00h:	stack and to allow addressing with 7C00h in bp
		;
		; Note also that by addressing the stack and sector
		;  with bp at 7C00h, and insuring this subtraction doesn't
		;  underflow, makes sure that we do not overwrite the IVT or
		;  BDA. (However, we assume that ax is at least 60h or so.)
		;
		; The FAT buffer segment is masked so that the actual buffer
		;  is stored on an 8 KiB boundary. This is to ensure that
		;  the buffer doesn't possibly cross a 64 KiB DMA boundary.
	jc .error_memory_j_CY
	cmp ax, (end -start+7C00h - ADR_STACK_LOW + 15) >> 4
		; This check is to ensure that the start of the destination
		;  for the relocation (stack, sector) is
		;  above-or-equal the end of the source for the relocation.
		;  That is, to avoid overwriting any of the source with the
		;  string move instruction (which for simplicity is always UP).
.error_memory_j_CY:
	jb error_memory
  %if ! _LOAD_NON_FAT
	mov bx, ((8192 - 16) + 512 + 7C00h)>>4
	add bx, ax
;  this is like calculating the following for the bx value:
;  ((LMA_top - 20 KiB - 8 KiB - (8 KiB - 16 B) - 512 - 7C00h) + \
;	((8 KiB - 16 B) + 512 + 7C00h))>>4
;  == (LMA_top - 20 KiB - 8 KiB)>>4
	and bx, ~ ((8192>>4) - 1)
	mov word [VAR(fat_seg)], bx
  %endif

	mov di, relocated
	push ax
	push di				; -> relocation target label relocated
		; (We cannot use a near call here to push the target IP
		;  because we do not control whether the ROM-BIOS loader
		;  entered us at 0:7C00h or 7C0h:0. So we need to create
		;  the correct offset manually here.)

	mov es, ax			; => destination
	mov si, sp			; ds:si = ss:_RELOCATESTART - 2 - 4
	mov di, si			; es:di -> destination for stack low
	mov cx, (end - (_RELOCATESTART - 2 - 4)) >> 1
		; end is the top of used memory
		; _RELOCATESTART is the lowest filled stack frame slot
		; 2 is for the first cluster word on the stack
		; 4 is for the additional slots taken by the return address
	rep movsw			; relocate stack, sector
	retf				; jump to relocated code

%if _WARN_PART_SIZE
 %assign num $ - memory_start
 %warning memory size is num bytes
%endif


	readhandler

	errorhandler


relocated:
	mov ss, ax
	mov ds, ax			; relocate these
	add ax, (ADR_STACK_LOW) >> 4	; (rounding down) => behind available

	pop si
 %else
	sub ax, (_RPL_GRACE_AREA + 20 * 1024) >> 4
					; RPL grace area, plus
					;  20 KiB reserved for iniload
	jb error_memory
 %endif
%elif _LOAD_ADR < ADR_FREE_UNTIL
 %if !_FIX_SECTOR_SIZE
	mov ax, ADR_FREE_UNTIL >> 4	; rounding *down*
 %endif
%else
 %error Load address within used memory
%endif
%if ! _RELOCATE && _LOAD_ADR < ADR_FREE_UNTIL && _FIX_SECTOR_SIZE
	; user of last_available_sector will hardcode the value!
%else
 %if _FIX_SECTOR_SIZE
		sub ax, _FIX_SECTOR_SIZE >> 4
 %else
		sub ax, [VAR(para_per_sector)]
 %endif
		push ax		; push into word [VAR(last_available_sector)]
%endif

read_fat_start:
; get starting cluster of file
%if ! _RELOCATE
 %if _ADD_SEARCH || _LOAD_DIR_SEG
		mov si,[es:di + deClusterLow]
 %elif _ATTRIB_SAVE && ! _DIR_ENTRY_500
		mov si,[es:di - deAttrib + deClusterLow]
 %else
		mov si,[es:di + deClusterLow - DIRENTRY_size \
			- (DIRENTRY_size * !!_DIR_ENTRY_520)]
 %endif
%endif
%if _SET_CLUSTER
		mov word [VAR(first_cluster)], si
%endif
%if _SET_DI_CLUSTER
		push si			; remember cluster for later
%endif

%if !_FAT16 && !_LOAD_NON_FAT
; Load the entire FAT into memory. This is easily feasible for FAT12,
;  as the FAT can only contain at most 4096 entries.
; (The exact condition should be "at most 4087 entries", or with a
;  specific FF7h semantic, "at most 4088 entries"; the more reliable
;  and portable alternative would be "at most 4080 entries".)
; Thus, no more than 6 KiB need to be read, even though the FAT size
;  as indicated by word[sectors_per_fat] could be much higher. The
;  first loop condition below is to correctly handle the latter case.
; (Sector size is assumed to be a power of two between 32 and 8192
;  bytes, inclusive. An 8 KiB buffer is necessary if the sector size
;  is 4 or 8 KiB, because reading the FAT can or will write to 8 KiB
;  of memory instead of only the relevant 6 KiB. This is always true
;  if the sector size is 8 KiB, and with 4 KiB sector size it is true
;  iff word[sectors_per_fat] is higher than one.)
		mov di, 6 << 10		; maximum size of FAT12 to load
		mov cx, [VAR(sectors_per_fat)]
					; maximum size of this FS's FAT
%if ! _RELOCATE && _LOAD_ADR < ADR_FREE_UNTIL && !_FIX_SECTOR_SIZE
			; Under these conditions, ax here is
			;  below ADR_FREE_UNTIL >> 4 so the
			;  following cwd instruction zeros dx.
		cwd
%else
		xor dx, dx
%endif
		mov ax, [VAR(fat_start)]; = first FAT sector
 %if _RELOCATE
			; bx already = FAT buffer segment here
 %else
		mov bx, ADR_FATBUF>>4
  %if _SET_FAT_SEG
		mov word [VAR(fat_seg)], bx
  %endif
 %endif
@@:
		call read_sector	; read next FAT sector
%if _FIX_SECTOR_SIZE
		sub di, _FIX_SECTOR_SIZE
%else
		sub di, [VAR(bytes_per_sector)]
%endif
					; di = bytes still left to read
		jbe @F			; if none -->
					; (jbe means jump if CF || ZF)
		loop @B			; if any FAT sector still remains -->
@@:					; one of the limits reached; FAT read
%endif

		mov bx, _LOAD_ADR>>4	; => load address
%if _FAT16 && !_LOAD_NON_FAT
		mov di, -1		; = no FAT sector read yet
 %if _SET_FAT_SEG && _SET_FAT_SEG_NORMAL
	; This is not strictly needed because a FAT sector is
	;  read in any case, initialising this variable later.
		mov word [VAR(fat_sector)], di
 %endif
%endif
%if _LOAD_NON_FAT
 %if _SET_FAT_SEG
  %if _FAT16
		or word [VAR(fat_sector)], -1
  %else
		and word [VAR(fat_seg)], 0
  %endif
 %endif
%endif

next_cluster:
; convert 16-bit cluster value (in SI) to 32-bit LBA sector value (in DX:AX)
; and get next cluster in SI


		; Converts cluster number to sector number
		;  and finds next cluster in chain
		;
		; INP:	si = valid cluster number
		;	(!_FAT16) [ADR_FATBUF] = entire FAT as read from FS
		;	(_FAT16) di = currently buffered FAT sector, -1 if none
		; OUT:	If unable to read a FAT sector,
		;	 ! jumps to error instead of returning
		;	If everything is okay,
		;	 si = next cluster number (or EOC value)
		;	 dx:ax = sector number
		;	 (_FAT16) di = currently buffered FAT sector
		; CHG:	cx

%if ! _LOAD_NON_FAT
; prepare to load entry from FAT

%if _FAT16
		push si			; preserve cluster number for later

; Multiply cluster number by 2.
		xchg ax, si
		; xor dx, dx		; dx:ax = entry to load (0..FFF6h)
		; add ax, ax
		; adc dx, dx		; dx:ax = byte offset into FAT (0..131052)
		cwd			; dx = FFFFh if ax >= 8000h, else = 0
		add ax, ax		; ax = (2 * ax) & FFFFh
		neg dx			; dx = 1 if 2 * ax overflowed, else = 0

; Divide by sector size to split dx:ax into the remainder as the byte offset
;  into the FAT sector, and quotient as the sector offset into the FAT.
		div word [VAR(bytes_per_sector)]
		mov si, dx		; = byte offset in sector
			; dx = byte offset into sector (0..8190)
			; ax = sector offset into FAT (0..4095)

; quotient in AX is FAT sector.
; check the FAT buffer to see if this sector is already loaded
; (simple disk cache; speeds things up a little --
; actually, it speeds things up a lot)
		cmp ax, di
		je @F
		mov di, ax
 %if _SET_FAT_SEG
		mov word [VAR(fat_sector)], ax
 %endif

; read the target FAT sector.
		push bx
			; As noted above, the sector offset in ax here is
			;  0..4095 (ie, 4 Ki S * 32 B/S = 128 KiB of FAT data),
			;  therefore this cwd instruction always zeros dx.
		cwd
		add ax, [VAR(fat_start)]
		adc dx, dx		; (account for overflow)
 %if _RELOCATE
		mov bx, word [VAR(fat_seg)]
		call read_sector
 %else
		call read_sector_fatbuf
 %endif
		pop bx
@@:

; get 16 bits from FAT
		es lodsw
		xchg ax, si
		pop ax		; restore cluster number
%else
; FAT12 entries are 12 bits, bytes are 8 bits. Ratio is 3 / 2,
;  so multiply cluster number by 3 first, then divide by 2.
		mov ax, si		; = cluster number (up to 12 bits set)
			; (remember cluster number in ax)
		shl si, 1		; = 2n (up to 13 bits set)
		add si, ax		; = 2n+n = 3n (up to 14 bits set)
		shr si, 1		; si = byte offset into FAT (0..6129)
					; CF = whether to use high 12 bits
 %if _RELOCATE
		mov es, word [VAR(fat_seg)]
		; es lodsw
		; xchg ax, si
		mov si, word [es:si]
 %else
; Use the calculated byte offset as an offset into the FAT
;  buffer, which holds all of the FAT's relevant data.
		; lea si, [ADR_FATBUF + si]
				; -> 16-bit word in FAT to load
; get 16 bits from FAT
		; lodsw
		; xchg ax, si
		mov si, [ADR_FATBUF + si]
				; = 16-bit word in FAT to load
 %endif

		mov cl, 4
		jc @F		; iff CY after shift -->
		shl si, cl	; shift up iff even entry
@@:
		shr si, cl	; shift down (clears top 4 bits)

			; (ax holds cluster number)
%endif

%else ; _LOAD_NON_FAT
		mov di, _LOAD_NON_FAT
	mov al, [VAR(sectors_per_cluster)]
	dec ax
	mov ah, 0
	inc ax
		mov cx, ax
		mul word [VAR(bytes_per_sector)]
		test dx, dx
		jnz @F
		cmp ax, di
		mov al, 'N'
		jb error
@@:
		xchg ax, si
%endif
; adjust cluster number to make it 0-based
		dec ax
		dec ax

%if ! _LOAD_NON_FAT
%if _FIX_CLUSTER_SIZE
		mov cx, _FIX_CLUSTER_SIZE
%else
; adjusted sectors per cluster
; decode EDR-DOS's special value 0 meaning 256
	mov cl, [VAR(sectors_per_cluster)]
	dec cx
	mov ch, 0
	inc cx
%endif
%endif

; convert from clusters to sectors
		mul cx
		add ax, [VAR(data_start)]
		adc dx, [VAR(data_start)+2]
				; dx:ax = sector number

; xxx - this will always load an entire cluster (e.g. 64 sectors),
; even if the file is shorter than this
@@:
 %if ! _RELOCATE && _LOAD_ADR < ADR_FREE_UNTIL && _FIX_SECTOR_SIZE
		cmp bx, (ADR_FREE_UNTIL >> 4) - (_FIX_SECTOR_SIZE >> 4)
 %else
		cmp bx, [VAR(last_available_sector)]
 %endif
%if _MEMORY_CONTINUE
		ja @F
%else
		ja error_filetoobig
%endif
%if _FAT16 && ! _LOAD_NON_FAT
		push es		; (must preserve ADR_FATBUF reference)
%endif
		call read_sector
%if _FAT16 && ! _LOAD_NON_FAT
		pop es
%endif
%if _SET_LOAD_SEG
		mov [VAR(load_seg)], bx	; => after last read data
%endif
%if _LOAD_NON_FAT
		sub di, word [VAR(bytes_per_sector)]
		ja @B
%else
		loop @B

%if _FAT16
; FFF7h: bad cluster
; FFF8h-FFFFh: end of cluster chain
		cmp si, 0FFF7h
%else
; 0FF7h: bad cluster
; 0FF8h-0FFFh: end of cluster chain
		cmp si, 0FF7h
%endif
		jb next_cluster
%endif	; _LOAD_NON_FAT
@@:

%if _WARN_PART_SIZE
 %assign num $ - read_fat_start
 %warning read_fat size is num bytes
%endif


finish_start:
%if _LOAD_MIN_PARA
 %if ((_LOAD_ADR >> 4) + _LOAD_MIN_PARA) & 255 == 0
	; If the value is divisible by 256 we can compare only the
	;  high byte for the same CF result: NC iff bx >= limit.
		cmp bh, ((_LOAD_ADR >> 4) + _LOAD_MIN_PARA) >> 8
 %else
		cmp bx, (_LOAD_ADR >> 4) + _LOAD_MIN_PARA
 %endif
		mov al, 'E'
		jb error
%endif

%if _TURN_OFF_FLOPPY
; turn off floppy motor
		mov dx,3F2h
		mov al,0
		out dx,al
%endif

; Set-up registers for and jump to loaded file
; Already: ss:bp-> boot sector containing BPB
%if _CHECKVALUE
CHECKLINEAR equ _LOAD_ADR + _CHECKOFFSET
 %if ! _RELOCATE && CHECKLINEAR <= (64 * 1024 - 2)
		cmp word [CHECKLINEAR], _CHECKVALUE
 %else
		mov ax, CHECKLINEAR >> 4
		mov es, ax
		cmp word [es:CHECKLINEAR & 15], _CHECKVALUE
 %endif
		mov al, 'V'		; check 'V'alue mismatch
		jne error
%endif
%if _SET_DL_UNIT
		mov dl, [VAR(boot_unit)]; set dl to unit
%endif
%if _SET_DI_CLUSTER && (_PUSH_DPT || _SET_DSSI_DPT)
		pop cx
%endif
%if _DATASTART_HIDDEN
		mov bx, [VAR(hidden_sectors + 0)]
		mov ax, [VAR(hidden_sectors + 2)]
		add word [VAR(data_start + 0)], bx
		adc word [VAR(data_start + 2)], ax
%endif
%if _SET_BL_UNIT
 %if _SET_DL_UNIT
		mov bl, dl		; set bl to unit, too
 %else
		mov bl, [VAR(boot_unit)]; set bl to unit
 %endif
%endif
%if _PUSH_DPT || _SET_DSSI_DPT
 %ifn _SET_DSSI_DPT		; (implying that only _PUSH_DPT is set)
  %if _RELOCATE
		xor ax, ax
		mov es, ax
		mov di, 1Eh * 4
		les si, [es:di]
		push es
		push si
		push ax
		push di
  %else
		mov di, 1Eh*4
		les si, [di]		; -> original (also current) DPT
		push es
		push si			; original (also current) DPT address
		push ss
		push di			; 0000h:0078h (address of 1Eh IVT entry)
  %endif
 %else
  %if _RELOCATE
		xor ax, ax
		mov ds, ax
  %endif
		mov di, 1Eh*4
		lds si, [di]		; -> original (also current) DPT
  %if _PUSH_DPT
		push ds
		push si			; original (also current) DPT address
   %if _RELOCATE
		push ax
		push di
   %else
		push ss
		push di			; 0000h:0078h (address of 1Eh IVT entry)
   %endif
  %endif
 %endif
%else
 %if _RELOCATE && (_ZERO_ES || _ZERO_DS)
		xor ax, ax
 %endif
%endif
%if _RELOCATE
 %if _ZERO_ES
		mov es, ax
 %endif
 %if _ZERO_DS
		mov ds, ax
 %endif
%endif

%if _SET_AXBX_DATA
		mov bx, [VAR(data_start)]
		mov ax, [VAR(data_start+2)]
%endif
%if _SET_DI_CLUSTER
 %if _PUSH_DPT || _SET_DSSI_DPT
		mov di, cx
 %else
		pop di
 %endif
%endif
%if ! _RELOCATE
 %if _ZERO_ES
		push ss
		pop es
 %endif
%endif
			; ss:bp-> boot sector with BPB
		jmp (_LOAD_ADR>>4)+_EXEC_SEG_ADJ:_EXEC_OFS


%if _WARN_PART_SIZE
 %assign num $ - finish_start
 %warning finish size is num bytes
%endif


%if ! _RELOCATE
	errorhandler

	readhandler
%endif


%if !_NO_LIMIT
available:
	_fill 508,38,start

signatures:
	dw 0
; 2-byte magic bootsector signature
	dw 0AA55h

%assign num signatures-available
%assign fatbits 12
%if _FAT16
 %assign fatbits 16
%endif
%warning FAT%[fatbits]: num bytes still available.
%endif

end:
