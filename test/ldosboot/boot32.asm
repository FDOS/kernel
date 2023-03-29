
%if 0

File system boot sector loader code for FAT32

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

	strdef OEM_NAME,	"    lDOS"
	strdef OEM_NAME_FILL,	'_'
	strdef DEFAULT_LABEL,	"lDOS"
	numdef VOLUMEID,	0
	strdef FSIBOOTNAME,	"FSIBOOT4"
	; used to set experimental name
	; strdef FSIBOOTNAME,	"FSIBEX02"

	strdef LOAD_NAME,	"LDOS"
	strdef LOAD_EXT,	"COM"	; name of file to load
	numdef LOAD_ADR,	02000h	; where to load
	numdef LOAD_MIN_PARA,	paras(1536)
	numdef EXEC_SEG_ADJ,	0	; how far cs will be from _LOAD_ADR
	numdef EXEC_OFS,	400h	; what value ip will be
	numdef CHECKOFFSET,	1020
	numdef CHECKVALUE,	"lD"
	numdef LOAD_DIR_SEG,	0	; => where to store dir entry (0 if nowhere)
	numdef ADD_SEARCH,	0	; whether to search second file
	strdef ADD_NAME,	""
	strdef ADD_EXT,		""	; name of second file to search
	numdef ADD_DIR_SEG,	0	; => where to store dir entry (0 if nowhere)

	numdef QUERY_GEOMETRY,	1	; query geometry via 13.08 (for CHS access)
	numdef QUERY_GEOMETRY_DISABLED, 0
	numdef USE_PART_INFO,	1	; use ds:si-> partition info from MBR, if any
	numdef USE_PART_INFO_DISABLED, 0
	numdef USE_AUTO_UNIT,	1	; use unit passed from ROM-BIOS in dl
	numdef RPL,		1	; support RPL and do not overwrite it
	numdef CHS,		1	; support CHS (if it fits)
	numdef LBA,		1	; support LBA (if available)
	numdef LBA_33_BIT,	1	; support 33-bit LBA
	numdef LBA_CHECK_NO_33,	1	; else: check that LBA doesn't carry

	numdef RELOCATE,	0	; relocate the loader to top of memory
	numdef SET_DL_UNIT,	0	; if to pass unit in dl
	numdef SET_BL_UNIT,	0	; if to pass unit in bl as well
	numdef SET_AXBX_DATA,	0	; if to pass first data sector in ax:bx
	numdef SET_DSSI_DPT,	0	; if to pass DPT address in ds:si
	numdef PUSH_DPT,	0	; if to push DPT address
	numdef MEMORY_CONTINUE,	1	; if to just execute when memory full
	numdef SET_SIDI_CLUSTER,0	; if to pass first load file cluster in si:di
	numdef TURN_OFF_FLOPPY,	0	; if to turn off floppy motor after loading
	numdef DATASTART_HIDDEN,0	; if to add hidden sectors to data_start
	numdef LBA_SET_TYPE,	0	; if to set third byte to LBA partition type
	numdef SET_LOAD_SEG,	1	; if to set load_seg (word [ss:bp - 6])
	numdef SET_FAT_SEG,	1	; if to set fat_seg (word [ss:bp - 8])
	numdef SET_CLUSTER,	1	; if to set first_cluster (dword [ss:bp - 16])
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

		; Unlike the 1440 KiB diskette image defaults for the FAT12
		;  loader we just fill the BPB with zeros by default.
	numdef MEDIAID, 0		; media ID
	numdef UNIT, 0			; load unit in BPB
	numdef CHS_SECTORS, 0		; CHS geometry field for sectors
	numdef CHS_HEADS, 0		; CHS geometry field for heads
	numdef HIDDEN, 0		; number of hidden sectors
	numdef SPI, 0			; sectors per image
	numdef BPS, 0			; bytes per sector
	numdef SPC, 0			; sectors per cluster
	numdef SPF, 0			; sectors per FAT
	numdef SECTOR_FSINFO, 0		; FSINFO sector
	numdef SECTOR_BACKUP, 0		; backup boot sector
	numdef CLUSTER_ROOT, 0		; root directory first cluster
	numdef NUMFATS, 0		; number of FATs
	numdef NUMRESERVED, 0		; number of reserved sectors

	numdef COMPAT_FREEDOS,	0	; partial FreeDOS load compatibility
	numdef COMPAT_IBM,	0	; partial IBMDOS load compatibility
	numdef COMPAT_MS7,	0	; partial MS-DOS 7 load compatibility
	numdef COMPAT_LDOS,	0	; lDOS load compatibility

%if (!!_COMPAT_FREEDOS + !!_COMPAT_IBM + !!_COMPAT_MS7 + !!_COMPAT_LDOS) > 1
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
	numdef CHECKVALUE,	0
	numdef LOAD_DIR_SEG,	50h
	numdef ADD_SEARCH,	1
	strdef ADD_NAME,	"IBMDOS"
	strdef ADD_EXT,		"COM"
	numdef ADD_DIR_SEG,	52h
 	; Note: The IBMBIO.COM directory entry must be stored at
	;  0:500h, and the IBMDOS.COM directory entry at 0:520h.

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

	numdef SET_DL_UNIT,	1
	numdef SET_DSSI_DPT,	0
	numdef PUSH_DPT,	1
	numdef MEMORY_CONTINUE,	1
	; 4 sectors * 512 BpS should suffice. We load into 700h--7A00h,
	;  ie >= 6000h bytes (24 KiB), <= 7300h bytes (28.75 KiB).
	numdef SET_SIDI_CLUSTER,1
	numdef DATASTART_HIDDEN,1
	numdef LBA_SET_TYPE,	1
%endif

%if _COMPAT_LDOS
	strdef LOAD_NAME,	"LDOS"
	strdef LOAD_EXT,	"COM"
	numdef LOAD_ADR,	02000h
	numdef LOAD_MIN_PARA,	paras(1536)
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

%if 0

Notes about partial load compatibilities

* FreeDOS:
 * Relocates to an address other than 27A00h (1FE0h:7C00h)
* IBMDOS:
 * Does not actually relocate DPT, just provide its address
* MS-DOS 7:
 * Does not actually relocate DPT, just provide its address
 * Does not contain message table used by loader

%endif

%if _SET_BL_UNIT && _SET_AXBX_DATA
 %error Cannot select both of these options!
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


%if (_LOAD_ADR & 0Fh)
 %error Load address must be on a paragraph boundary
%endif

%if _LOAD_ADR > LOADLIMIT
 %error Load address must be in LMA
%elif _LOAD_ADR < 00500h
 %error Load address must not overlap IVT or BDA
%endif
%if ! _RELOCATE
 %if _LOAD_ADR > (POSITION-512) && _LOAD_ADR < (POSITION+1024)
  %error Load address must not overlap loader
 %endif
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

; 512-byte stack (minus the variables).
ADR_STACK_LOW	equ	7C00h - 200h		; 07A00h

ADR_FSIBOOT	equ	end -start+7C00h	; 07E00h

%define _AFTER (ADR_FSIBOOT + 512)
%if _RELOCATE
	; dynamic allocation of FAT buffer
%else
 %if _LOAD_ADR < 7C00h
  ADR_FATBUF	equ	_AFTER
  %define _AFTER (ADR_FATBUF + 8192)
 %else
  ADR_FATBUF	equ	2000h
 %endif
%endif

%if _ADD_SEARCH
 %if _RELOCATE
	; dynamic allocation of dir buffer
 %elif _LOAD_ADR < 7C00h
  ADR_DIRBUF	equ	_AFTER
  %define _AFTER (ADR_DIRBUF + 8192)
 %else
  ADR_DIRBUF	equ	4000h
 %endif
%else
 %if _RELOCATE
  ADR_DIRBUF	equ	_LOAD_ADR		; 00600h
 %elif _LOAD_ADR < 7C00h
 ; one-sector directory buffer. Assumes sectors are no larger than 8 KiB
  ADR_DIRBUF	equ	_AFTER			; 0A000h
  %define _AFTER (ADR_DIRBUF + 8192)
 %else
  ADR_DIRBUF	equ	4000h
 %endif
%endif
ADR_FREE	equ	_AFTER			; 08000h

%if ! _RELOCATE
 %if ((ADR_FATBUF + 8192 - 1) & ~0FFFFh) != (ADR_FATBUF & ~0FFFFh)
  %warning Possibly crossing 64 KiB boundary while reading FAT
 %endif
%endif

%ifn _RELOCATE && _ADD_SEARCH
 %if ((ADR_DIRBUF + 8192 - 1) & ~0FFFFh) != (ADR_DIRBUF & ~0FFFFh)
  %warning Possibly crossing 64 KiB boundary while reading directory
 %endif
%endif

%if _LOAD_ADR >= ADR_FREE || _RELOCATE
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


	struc FSINFO	; FAT32 FSINFO sector layout
.signature1:	resd 1		; 41615252h ("RRaA") for valid FSINFO
.reserved1:			; former unused, initialized to zero by FORMAT
.fsiboot:	resb 480	; now used for FSIBOOT
.signature2:	resd 1		; 61417272h ("rrAa") for valid FSINFO
.numberfree:	resd 1		; FSINFO: number of free clusters or -1
.nextfree:	resd 1		; FSINFO: first free cluster or -1
.reserved2:	resd 3		; unused, initialized to zero by FORMAT
.signature3:	resd 1		; AA550000h for valid FSINFO or FSIBOOT
	endstruc

	struc FSIBOOTG	; FSIBOOT general layout
.signature:	resq 1		; 8 bytes that identify the FSIBOOT type
.fsicode:	resb 470	; 470 bytes FSIBOOT type specific data or code
.dirsearch:	resw 1		; 1 word -> directory search entrypoint
	endstruc

	struc DIRENTRY
deName:		resb 8
deExt:		resb 3
deAttrib:	resb 1
dePlusSize:	resb 1
		resb 7
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
	%macro nextvariable 2.nolist
%1	equ (_LASTVARIABLE - %2)
%define _LASTVARIABLE %1
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

; (dword for FAT32) currently loaded sector-in-FAT, -1 if none
	nextvariable fat_sector, 4

; (dword for FAT32) first cluster of load file
	nextvariable first_cluster, 4

ADR_STACK_START	equ	_LASTVARIABLE -start+POSITION

; (word) number of 16-byte paragraphs per sector
	nextvariable para_per_sector, 2

; (word) number of 32-byte directory entries per sector
	nextvariable entries_per_sector, 2

; (word) segment of last available memory for sector
	nextvariable last_available_sector, 2

; (word) actual sectors per cluster
	nextvariable adj_sectors_per_cluster, 2

; (word) paragraphs left to read
	nextvariable paras_left, 2

lowest_variable		equ _LASTVARIABLE


	jmp strict short skip_bpb
%if !_CHS && _LBA_SET_TYPE
	db 0Ch		; LBA-enabled FAT32 FS partition type
%else
	nop		; default: no LBA
%endif


; BIOS Parameter Block (BPB)
;
; Installation will use the BPB already present in your file system.
; These values must be initialised when installing the loader.

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
	dw 0
total_sectors:		; offset 13h (19) - not used by this code
%if _SPI < 1_0000h
	dw _SPI
%else
	dw 0
%endif
media_id:		; offset 15h (21) - not used by this code
	db _MEDIAID
sectors_per_fat:	; offset 16h (22)
	dw 0
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

sectors_per_fat_large:	; offset 24h (36)
	dd _SPF
fsflags:		; offset 28h (40)
	dw 0
fsversion:		; offset 2Ah (42)
	dw 0
root_cluster:		; offset 2Ch (44)
	dd _CLUSTER_ROOT
fsinfo_sector:		; offset 30h (48)
	dw _SECTOR_FSINFO
backup_sector:		; offset 32h (50)
	dw _SECTOR_BACKUP

	times 12 db 0	; offset 34h (52) reserved

; Extended BPB		; offset 40h (64)

boot_unit:		db _UNIT
			db 0
ext_bpb_signature:	db 29h
serial_number:		dd _VOLUMEID
volume_label:		fill 11,32,db _DEFAULT_LABEL
filesystem_identifier:	fill 8,32,db "FAT32"


; Initialised data

	align 2
fsiboot_table:		; this table is used by the FSIBOOT stage
.error:		dw error
			; INP: al = error condition letter
			; ('B' = bad chain / FS error, 'F' = file not found,
			;  'R' = disk read error, 'M' = not enough memory,
			;  'E' = not enough data in file)
.success:	dw load_finish
			; INP: dword [ss:sp] = first cluster
			; Note: The first cluster dword is always filled in
			;  by FSIBOOT; the option _SET_SIDI_CLUSTER only
			;  affects usage in the primary loader.
%if _MEMORY_CONTINUE
.memory_full:	dw load_finish_mc
			; INP: al = error condition letter ('M'),
			;  dword [ss:sp] = current cluster number,
			;  dword [ss:sp + 4] = first cluster, refer to .success
%else
.memory_full:	dw error
			; refer to previous .memory_full comment
%endif
.read_sector:	dw read_sector
			; INP: dx:ax = sector number within partition,
			;  bx => buffer, (_LBA) ds = ss
			; OUT: dx:ax incremented, bx => incremented,
			;  es = input bx, does not return if error occurred
			; CHG: none
%if _RELOCATE && _ADD_SEARCH
.dirbuf:	dw 8192 >> 4
			; initialised to segment displacement from FAT buffer
%else
.dirbuf:	dw ADR_DIRBUF>>4
			; => directory sector buffer (one sector)
%endif
.writedirentry:	dw writedirentry.loaddir
			; INP:	es:bx -> found dir entry in dir sector buffer
			;	si:di = loaded sector-in-FAT
			; CHG:	ax, cx, dx
			; STT:	ss:bp -> boot sector
			;	ds = ss
			;	UP
			; OUT:	directory entry copied if so desired
			;	(is a no-op if not to copy dir entry)
.filename:	dw .load_name
			; -> name to search
.minpara:	dw _LOAD_MIN_PARA
.loadseg:	dw _LOAD_ADR>>4
			; => where to load

.load_name:		; = blank-padded 11-byte filename to search for
	fill 8,32,db _LOAD_NAME
	fill 3,32,db _LOAD_EXT

fsiboot_name:
	fill 8, 32, db _FSIBOOTNAME

%if _WARN_PART_SIZE
 %assign num $ - start
 %warning BPB + data size is num bytes
%endif


; Code

		; Note that this may be entered with cs:ip = 07C0h:0 !
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


; 16-byte paragraphs per sector
	mov ax, [VAR(bytes_per_sector)]
	mov cl, 4
	shr ax, cl
	push ax			; push into word [VAR(para_per_sector)]

; 32-byte FAT directory entries per sector
	shr ax, 1		; /2 = 32-byte entries per sector
	push ax			; push into word [VAR(entries_per_sector)]

load_fsiboot:
	cmp ax, 1024 >> 5	; sector size is at least 1 KiB (large) ?
	jae .loaded		; already loaded as part of the boot sector -->

		; If this code runs, then ax was below 1024 >> 5,
		;  therefore this cwd instruction always zeros dx.
	cwd

	mov ax, [VAR(fsinfo_sector)]
;	inc ax
;	jz error_fsiboot	; was FFFFh, invalid -->
;	dec ax
		; FFFFh always fails the < num_reserved_sectors check
	test ax, ax
	jz @F			; is 0 ? invalid --> (ZR, NC)
	cmp ax, [VAR(num_reserved_sectors)]
@@:			; (ZR, NC if ax == 0)
	jae error_fsiboot	; (jump if NC)
				; dx:ax = FSINFO sector (dx = 0 from cwd)
	mov cx, 512
	mov bx, ADR_FSIBOOT >> 4
@@:
	call read_sector
	sub cx, [VAR(bytes_per_sector)]
	ja @B			; read 512 bytes -->

.loaded:
	push ds
	pop es
	mov di, fsiboot.signature
	mov si, fsiboot_name
	mov cx, 4		; size of fsiboot_name
	repe cmpsw
	jne error_fsiboot
		; Note that now es:di -> fsiboot.start

%if _LOAD_ADR >= ADR_FREE || _RELOCATE
; Get conventional memory size and store it
		int 12h
		mov cl, 6
		shl ax, cl
 %if _RPL		; +33 bytes
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
  %if _ADD_SEARCH
	sub ax, (20 * 1024 + 8192 + 8192 + (8192-16) + 1024 + 7C00h) >> 4
  %else
	sub ax, (20 * 1024 + 8192 + (8192-16) + 1024 + 7C00h) >> 4
  %endif
		; 20 KiB:	reserved for iniload
		; 8 KiB:	dir buffer (only if _ADD_SEARCH)
		; 8 KiB:	FAT buffer
		; 8 KiB - 16 B:	to allow rounding down position of buffers
		; 1 KiB:	sector + FSIBOOT
		; 7C00h:	stack and to allow addressing with 7C00h in bp
		;
		; Note also that by addressing the stack and sector and FSIBOOT
		;  with bp at 7C00h, and insuring this subtraction doesn't
		;  underflow, makes sure that we do not overwrite the IVT or
		;  BDA. (However, we assume that ax is at least 60h or so.)
		;
		; The FAT buffer segment is masked so that the actual buffer
		;  is stored on an 8 KiB boundary. This is to ensure that
		;  the buffer doesn't possibly cross a 64 KiB DMA boundary.
	jc error_fsiboot
	cmp ax, (end_after_fsiboot -start+7C00h - ADR_STACK_LOW + 15) >> 4
		; This check is to ensure that the start of the destination
		;  for the relocation (stack, sector, FSIBOOT) is
		;  above-or-equal the end of the source for the relocation.
		;  That is, to avoid overwriting any of the source with the
		;  string move instruction (which for simplicity is always UP).
	jb error_fsiboot

			; With _RELOCATE enabled, the FAT buffer segment
			;  is fixed up by the primary loader here before
			;  FSIBOOT is executed.
	mov bx, ((8192 - 16) + 1024 + 7C00h)>>4
; bx is initialised to the value
;  ((8192 - 16) + 1024 + 7C00h)>>4
;  so this is like calculating the following for its value:
;  ((LMA_top - 20 KiB - 8 KiB - 8 KiB{AS} - (8 KiB - 16 B) - 1 KiB - 7C00h) + \
;	((8 KiB - 16 B) + 1 KiB + 7C00h))>>4
;  == (LMA_top - 20 KiB - 8 KiB - 8 KiB{AS})>>4
; {AS} = only if _ADD_SEARCH
	add bx, ax
	and bx, ~ ((8192>>4) - 1)	; => FAT sector buffer (one sector)
  %if _ADD_SEARCH
; .dirbuf is initialised to 8192 >> 4
	add word [VAR(fsiboot_table.dirbuf)], bx
					; => dir sector buffer (one sector)
  %endif

	push ax
	push di				; -> reloc destination of fsiboot.start

	mov es, ax			; => destination
	mov si, sp			; ds:si = ss:entries_per_sector - 4
	mov di, si			; es:di -> destination for stack low
	mov cx, (end_after_fsiboot - (entries_per_sector - 4)) >> 1
		; end_after_fsiboot is the top of used memory
		; entries_per_sector is the lowest filled stack frame slot
		; 4 is for the additional slots taken by the return address
	rep movsw			; relocate stack, sector, and FSIBOOT
	mov ss, ax
	mov ds, ax			; relocate these
	add ax, (ADR_STACK_LOW) >> 4	; (rounding down) => behind available

	retf				; jump to relocated FSIBOOT
 %else
	sub ax, (20 * 1024) >> 4	; 20 KiB reserved for iniload
	jc error_fsiboot
	mov bx, ADR_FATBUF >> 4		; => FAT sector buffer (one sector)
	push es
	push di				; -> fsiboot.start
	retf
		; Do a far return here to ensure that cs is zero.
 %endif
%elif _LOAD_ADR < ADR_STACK_LOW
	mov ax, (ADR_STACK_LOW >> 4)
	mov bx, ADR_FATBUF >> 4		; => FAT sector buffer (one sector)
	push es
	push di				; -> fsiboot.start
	retf
		; Do a far return here to ensure that cs is zero.
%else
 %error Load address within used memory
%endif


%if _WARN_PART_SIZE
 %assign num $ - skip_bpb
 %warning init size is num bytes
%endif


finish_start:
		; INP:	es:bx -> found dir entry in dir sector buffer
		;	si:di = loaded sector-in-FAT
		; CHG:	ax, cx, dx
		; STT:	ss:bp -> boot sector
		;	ds = ss
		;	UP
		; OUT:	directory entry copied if so desired
		;	(is a no-op if not to copy dir entry)
writedirentry:
%if _LOAD_DIR_SEG
.loaddir:
	mov ax, _LOAD_DIR_SEG
%else
.loaddir: equ read_sector.retn
%endif
%if _LOAD_DIR_SEG || (_ADD_DIR_SEG && _ADD_SEARCH)
		; INP:	ax => where to store directory entry
.ax:
	push ds
	push si
	push di
	 push es
	 pop ds
	mov si, bx	; ds:si -> directory entry
	mov cx, DIRENTRY_size >> 1
	mov es, ax
	xor di, di	; es:di -> where to store directory entry
	rep movsw	; move to here (one directory entry)
	 push ds
	 pop es		; es:bx -> dir entry in dir sector buffer
	pop di
	pop si
	pop ds
	retn
%endif


%if _MEMORY_CONTINUE
load_finish_mc:
		pop bx
		pop cx
%endif
load_finish:

%if _ADD_SEARCH
		mov word [VAR(fsiboot_table.filename)], add_name
		call near word [dirsearch_entrypoint]
 %if _ADD_DIR_SEG
		mov ax, _ADD_DIR_SEG
		call writedirentry.ax
 %endif
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
 %if CHECKLINEAR <= (64 * 1024 - 2) && ! _RELOCATE
		cmp word [CHECKLINEAR], _CHECKVALUE
 %else
		mov ax, CHECKLINEAR >> 4
		mov es, ax
		cmp word [es:CHECKLINEAR & 15], _CHECKVALUE
 %endif
		mov al, 'V'		; check 'V'alue mismatch
		jne error
%endif
%if _SET_SIDI_CLUSTER && (_PUSH_DPT || _SET_DSSI_DPT)
		pop cx
		pop dx
%endif
%if _PUSH_DPT || _SET_DSSI_DPT
 %ifn _SET_DSSI_DPT		; (implying that only _PUSH_DPT is set)
  %if _RELOCATE
		xor ax, ax
		mov es, ax		; => IVT
		mov di, 1Eh*4
		les si, [es:di]		; -> original (also current) DPT
		push es
		push si			; original (also current) DPT address
		push ax
		push di			; 0000h:0078h (address of 1Eh IVT entry)
  %else
			; If not _RELOCATE, ds = 0000h (=> IVT)
		mov di, 1Eh*4
		les si, [di]		; -> original (also current) DPT
		push es
		push si			; original (also current) DPT address
			; If not _RELOCATE, ss = 0000h (=> IVT)
		push ss
		push di			; 0000h:0078h (address of 1Eh IVT entry)
  %endif
 %else
  %if _RELOCATE
		xor ax, ax
		mov ds, ax		; => IVT
  %endif
			; If not _RELOCATE, ds = 0000h (=> IVT)
		mov di, 1Eh*4
		lds si, [di]		; -> original (also current) DPT
  %if _PUSH_DPT
		push ds
		push si			; original (also current) DPT address
   %if _RELOCATE
		push ax
		push di			; 0000h:0078h (address of 1Eh IVT entry)
   %else
			; If not _RELOCATE, ss = 0000h (=> IVT)
		push ss
		push di			; 0000h:0078h (address of 1Eh IVT entry)
   %endif
  %endif
 %endif
%endif
%if _ZERO_ES || _ZERO_DS
 %if _RELOCATE
  %ifn _PUSH_DPT || _SET_DSSI_DPT
		xor ax, ax
  %endif
  %if _ZERO_ES
		mov es, ax
  %endif
  %if _ZERO_DS
		mov ds, ax
  %endif
 %else
  %if _ZERO_ES
		push ss
		pop es
  %endif
  %if _ZERO_DS && _SET_DSSI_DPT
		push ss
		pop ds
  %endif
 %endif
%endif
%if _DATASTART_HIDDEN
		mov bx, [VAR(hidden_sectors + 0)]
		mov ax, [VAR(hidden_sectors + 2)]
		add word [VAR(data_start + 0)], bx
		adc word [VAR(data_start + 2)], ax
%endif
%if _SET_AXBX_DATA
		mov bx, [VAR(data_start)]
		mov ax, [VAR(data_start+2)]
%endif
%if _SET_SIDI_CLUSTER
 %if _SET_DSSI_DPT
  %error Cannot select both of these.
 %endif
 %if _PUSH_DPT || _SET_DSSI_DPT
		mov di, cx
		mov si, dx
 %else
		pop di
		pop si
 %endif
%endif
%if _SET_DL_UNIT
		mov dl, [VAR(boot_unit)]; set dl to unit
 %if _SET_BL_UNIT
		mov bl, dl		; set bl to unit, too
 %endif
%elif _SET_BL_UNIT
		mov bl, [VAR(boot_unit)]; set bl to unit
%endif
			; ss:bp-> boot sector with BPB
		jmp (_LOAD_ADR>>4)+_EXEC_SEG_ADJ:_EXEC_OFS

%if _WARN_PART_SIZE
 %assign num $ - finish_start
 %warning finish size is num bytes
%endif


error_start:

error_fsiboot:
	mov al,'I'

	db __TEST_IMM16	; (skip mov)
read_sector.err:
	mov al, 'R'	; Disk 'R'ead error

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
.err_CY: equ .err
.err_2: equ .err
  %else
	jnc .lba_done
	cmp ah, 1	; invalid function?
	je .no_lba	; try CHS instead -->
.err_CY:
.err_2:
	jmp .lba_error
  %endif
 %else
.err_CY:
	jc .lba_error
.err_2: equ .err
 %endif

.lba_done:
%if _CHS && _LBA_SET_TYPE
	mov byte [bp + 2], 0Ch	; LBA-enabled FAT32 FS partition type
%endif
	add sp, 10h
%if _CHS
	jmp short .done
%endif

.lba_error: equ .err

 %if !_CHS
.no_lba: equ .err
 %else
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
%else
.err_2: equ .err
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
			 jnz .err_2	; error if cylinder >= 1024 -->
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
			jc .err_CY
%endif
%if ! _LBA
.err_CY:	equ .err
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

.retn:
	retn

%if _WARN_PART_SIZE
 %assign num $ - read_sector
 %warning read_sector size is num bytes
%endif


%if _ADD_SEARCH
add_name:		; = blank-padded 11-byte filename to search for
	fill 8,32,db _ADD_NAME
	fill 3,32,db _ADD_EXT
%endif


%if !_NO_LIMIT
available:
	_fill 508,38,start

signatures:
	dw 0
; 2-byte magic bootsector signature
	dw 0AA55h

%assign num signatures-available
%warning FAT32: num bytes still available.
%else	; for testing
	align 4
signatures:
	dw 0
; 2-byte magic bootsector signature
	dw 0AA55h
%endif

	align 16, nop
end:

fsiboot:
	dd "RRaA"
.signature:
	fill 8, 32, db _FSIBOOTNAME

%if ($ - .signature) != 8
 %error Unexpected name size
%endif
.start:
		; INP:	ax => after last segment to be used for loading
		;	bx => FAT buffer (8 KiB)
		;	ss:bp -> boot sector, with EBPB
		;	dwo [ss:bp -  4] = data_start (uninit)
		;	wo [ss:bp -  6] = load_seg (uninit)
		;	wo [ss:bp -  8] = fat_seg (uninit)
		;	dwo [ss:bp - 12] = fat_sector (uninit)
		;	dwo [ss:bp - 16] = first_cluster (uninit)
		;	wo [ss:bp - 18] = para_per_sector
		;	wo [ss:bp - 20] = entries_per_sector
		;	ss:sp = ss:bp - 20
		;	(Note:	The following stack frame entries are currently
		;		 only used by FSIBOOT itself, so they may be
		;		 considered implementation detail instead of
		;		 part of the FSIBOOT protocol.)
		;	wo [ss:bp - 22] = last_available_sector (uninit)
		;	wo [ss:bp - 24] = adj_sectors_per_cluster (uninit)
		;	wo [ss:bp - 26] = paras_left (uninit)
		;	Stack layout has to match!
		;	ss = ds
		;	ss:bp + ((11 + ebpbNew + BPBN_size + 1) & ~1)
		;	 = ss:fsiboot_table, refer to its comments
		;	cs set to address jump table offsets in fsiboot_table
		;	ds set to address fsiboot_table.load_name
		; OUT:	Jumps to fsiboot_table.error if error occurs:
		;	 al = error condition letter
		;	Else:
		;	data_start (within partition) initialised
		;	load_seg => behind last loaded data
		;	fat_seg initialised (from bx)
		;	fat_sector initialised
		;	 (-1 initially, loaded sector-in-FAT later)
		;	first_cluster initialised
		;	last_available_sector initialised
		;	 (from input ax minus word [para_per_sector])
		;	adj_sectors_per_cluster initialised (from BPB)
		;	paras_left initialised
		;	Jumps to fsiboot_table.success if loaded entirely:
		;	 dword [ss:sp] = first cluster
		;	Jumps to fsiboot_table.memory_full if loaded partially:
		;	 al = error condition letter ('M')
		;	 dword [ss:sp] = current cluster number
		;	 dword [ss:sp + 4] = first cluster

	mov word [VAR(fat_seg)], bx	; initialise => FAT buffer

	sub ax, word [VAR(para_per_sector)]
	jc .CY_fsiboot_error_badchain
	push ax			; push into word [VAR(last_available_sector)]

	mov bx, word [VAR(fsiboot_table.loadseg)]
	mov word [VAR(load_seg)], bx	; initialise => load address
	cmp ax, bx
	jb .CY_fsiboot_error_badchain

; adjusted sectors per cluster (store in a word,
;  and decode EDR-DOS's special value 0 meaning 256)
	mov al, [VAR(sectors_per_cluster)]
	dec ax
	mov ah, 0
	inc ax
	push ax			; push into word [VAR(adj_sectors_per_cluster)]
	dec ax				; ! ah = 0
	mov al,[VAR(num_fats)]		; ! ah = 0, hence ax = number of FATs
	push ax
	mul word [VAR(sectors_per_fat_large)]
		; ax = low word SpF*nF
		; dx = high word
	xchg bx, ax
	xchg cx, dx
		; cx:bx = first mul
	pop ax
	mul word [VAR(sectors_per_fat_large + 2)]
		; ax = high word adjust
		; dx = third word
	test dx, dx
	stc
	jnz .CY_fsiboot_error_badchain
		; dx = zero
	xchg dx, ax
		; dx = high word adjust
		; ax = zero
	add dx, cx
	jc .CY_fsiboot_error_badchain
		; dx:bx = result
	xchg ax, bx
		; dx:ax = result
		; bx = zero

	add ax,[VAR(num_reserved_sectors)]
	adc dx, bx		; bx is zero here
.CY_fsiboot_error_badchain:
	jc ..@CY_2_fsiboot_error_badchain

; first sector of disk data area:
	mov [VAR(data_start)], ax
	mov [VAR(data_start+2)], dx

	mov di, -1
	mov si, di
	mov word [VAR(fat_sector + 2)], si
	mov word [VAR(fat_sector + 0)], di
	call dirsearch


found_load_file:
	call near word [VAR(fsiboot_table.writedirentry)]

; get starting cluster of file
		push word [es:bx + deClusterHigh]
		push word [es:bx + deClusterLow]

					; check FAT+ size bits
		test byte [es:bx + dePlusSize], 0E7h
					; test whether bits 7-5 and 2-0 NZ
; https://web.archive.org/web/20150219123449/http://www.fdos.org/kernel/fatplus.txt
		jnz .large_file		; yes, clamp to maximum paras -->
		mov ax, [es:bx + deSize + 2]
		mov bx, [es:bx + deSize]
					; ax:bx = file size (non-FAT+)

		mov cx, [VAR(bytes_per_sector)]
		dec cx			; BpS - 1
		add bx, cx
		adc ax, 0		; large ?
		jc .large_file		; yes, clamp to maximum paras -->

		not cx			; ~ (BpS - 1)
		and bx, cx		; mask to limit to rounded-up sector
				; (this also rounds up paragraphs)
		mov cx, 4
@@:
		shr ax, 1
		rcr bx, 1
		loop @B
			; ax:bx = size in paragraphs
			; bx = size in paragraphs if < 1_0000h
		test ax, ax		; > 0FFFFh paras ?
		jz @F			; no, take actual size -->
.large_file:
		mov bx, 0FFFFh		; cx = clamp size to 0FFFFh paras
@@:
		call check_enough.in_bx	; (CHG ax)

		pop ax
		pop dx			; dx:ax = first cluster

		push bx			; push into word [VAR(paras_left)]

		mov word [VAR(first_cluster + 0)], ax
		mov word [VAR(first_cluster + 2)], dx
		push dx
		push ax			; remember cluster for later

		call check_clust
..@CY_2_fsiboot_error_badchain:
		jc ..@CY_3_fsiboot_error_badchain

next_load_cluster:
		push dx
		push ax			; preserve cluster number for later
		call clust_to_first_sector
			; dx:ax = first sector of cluster
			; cx = adjusted sectors per cluster

; xxx - this will always load an entire cluster (e.g. 64 sectors),
; even if the file is shorter than this
@@:
		mov bx, [VAR(load_seg)]	; => where to read next sector
		cmp bx, [VAR(last_available_sector)]
		jbe @F
		call check_enough
		mov al, 'M'		; (! _MEMORY_CONTINUE: error code)
		jmp near word [VAR(fsiboot_table.memory_full)]

@@:
		call near word [VAR(fsiboot_table.read_sector)]
		mov [VAR(load_seg)], bx	; => after last read data

		mov bx, [VAR(para_per_sector)]
		sub word [VAR(paras_left)], bx
		jbe @F		; read enough -->

		loop @BB
		pop ax
		pop dx

		call clust_next
		jnc next_load_cluster
		inc ax
		inc ax
		test al, 8	; set in 0FFF_FFF8h--0FFF_FFFFh,
				;  clear in 0, 1, and 0FFF_FFF7h
		jz fsiboot_error_badchain
		db __TEST_IMM16
@@:
		pop bx
		pop cx
		call check_enough
		jmp near word [VAR(fsiboot_table.success)]


dirsearch:
	mov ax, [VAR(root_cluster)]
	mov dx, [VAR(root_cluster + 2)]
	call check_clust
..@CY_3_fsiboot_error_badchain:
	jc fsiboot_error_badchain

next_root_clust:
	push dx
	push ax
	call clust_to_first_sector
			; dx:ax = first sector of cluster
			; cx = adjusted sectors per cluster
next_root_sect:
	push cx
	mov cx, [VAR(entries_per_sector)]

; Scan root directory for file. We don't bother to check for deleted
;  entries (E5h) or entries that mark the end of the directory (00h).
	mov bx, [VAR(fsiboot_table.dirbuf)]
	call near word [VAR(fsiboot_table.read_sector)]

	push di
	xor di, di		; es:di-> first entry in this sector
next_ent:
	test byte [es:di + deAttrib], ATTR_DIRECTORY | ATTR_VOLLABEL
	jnz @F			; directory, label, or LFN entry --> (NZ)
	push si
	push di
	push cx
	mov si, [VAR(fsiboot_table.filename)]
				; ds:si-> name to match
	mov cx, 11		; length of padded 8.3 FAT filename
	repe cmpsb		; check entry
	pop cx
	pop di
	pop si
@@:				; ZR = match, NZ = mismatch
	je found_it		; found entry -->
	lea di, [di + DIRENTRY_size]

	loop next_ent		; count down sector's entries (jumps iff cx >0)
	pop di
	pop cx
	loop next_root_sect
	pop ax
	pop dx
	call clust_next
	jnc next_root_clust
file_not_found:
	mov al, 'F'
	db __TEST_IMM16
fsiboot_error_badchain:
	mov al, 'B'

fsiboot_error:
	jmp near word [VAR(fsiboot_table.error)]


		; INP:	dx:ax = cluster - 2 (0-based cluster)
		; OUT:	dx:ax = first sector of that cluster
		;	cx = adjusted sectors per cluster
		; CHG:	bx
clust_to_first_sector:
	mov cx, word [VAR(adj_sectors_per_cluster)]
	 push dx
	mul cx
	xchg bx, ax
	 pop ax
	push dx
	mul cx
	test dx, dx
	jnz fsiboot_error_badchain
	xchg dx, ax
	pop ax
	add dx, ax
	jc ..@CY_fsiboot_error_badchain
	xchg ax, bx

	add ax, [VAR(data_start)]
	adc dx, [VAR(data_start + 2)]
..@CY_fsiboot_error_badchain:
	jc fsiboot_error_badchain
				; dx:ax = first sector in cluster
	retn


check_enough:
		mov bx, [VAR(load_seg)]
				; => behind last read sector
		sub bx, word [VAR(fsiboot_table.loadseg)]
.in_bx:
		cmp bx, word [VAR(fsiboot_table.minpara)]
		mov al, 'E'
		jb fsiboot_error
		retn


found_it:
			; es:di -> dir entry in dir sector buffer
	mov bx, di
		pop di			; restore si:di = loaded FAT sector
		pop cx			; (discard sectors per cluster counter)
		pop cx
		pop cx			; (discard current cluster number)
			; es:bx -> dir entry
	retn


		; INP:	dx:ax = cluster (0-based)
		;	si:di = loaded FAT sector, -1 if none
		; OUT:	CY if no next cluster
		;	NC if next cluster found
		;	dx:ax = next cluster value (0-based)
		;	si:di = loaded FAT sector
		; CHG:	cx, bx, es
clust_next:
	add ax, 2
	adc dx, 0

	add ax, ax
	adc dx, dx
	add ax, ax
	adc dx, dx		; * 4 = byte offset into FAT (0--4000_0000h)
	 push ax
	xchg ax, dx
	xor dx, dx		; dx:ax = high word
	div word [VAR(bytes_per_sector)]
	xchg bx, ax		; bx = result high word, clobbers ax
	 pop ax			; dx = remainder, ax = low word
	div word [VAR(bytes_per_sector)]
	xchg dx, bx		; dx:ax = result, bx = remainder
				; dx:ax = sector offset into FAT (0--200_0000h)
				; bx = byte offset into FAT sector (0--8190)
	cmp dx, si
	jne @F		; read sector
	cmp ax, di
	je @FF		; sector is already buffered
@@:
	mov si, dx
	mov di, ax
	mov word [VAR(fat_sector + 2)], dx
	mov word [VAR(fat_sector + 0)], ax

	push bx
	add ax, [VAR(fat_start)]
	adc dx, 0
	mov bx, [VAR(fat_seg)]
	call near word [VAR(fsiboot_table.read_sector)]
	pop bx
@@:
	mov es, [VAR(fat_seg)]
	mov ax, [es:bx]
	mov dx, [es:bx + 2]

		; INP:	dx:ax = cluster value, 2-based
		; OUT:	dx:ax -= 2 (makes it 0-based)
		;	CY iff invalid cluster
check_clust:
	and dh, 0Fh
	sub ax, 2
	sbb dx, 0
	cmp dx, 0FFFh
	jb @F		; CY here means valid ...-
	cmp ax, 0FFF7h - 2
@@:			;  -... or if NC first, CY here also
	cmc		; NC if valid
	retn


fsinfo_available:
	_fill 480 + 4 - 2,38,fsiboot

%assign num $-fsinfo_available
%warning FSINFO: num bytes still available.


		; INP:	si:di = loaded sector-in-FAT
		;	word [fsiboot_table.filename] -> 8.3 filename
		; CHG:	ax, cx, dx
		; OUT:	si:di updated, if so
		;	es:bx -> found directory entry
		;	jumps to error handler if an error occurs
		; STT:	stack variables as set up by main FSIBOOT entry
dirsearch_entrypoint:
	dw dirsearch

	dd "rrAa"
	dd -1		; number of free clusters
	dd -1		; first free cluster
	times 3 dd 0	; FSINFO.reserved2
	dd 0_AA55_0000h	; FSINFO.signature3

end_after_fsiboot:
%if $ - fsiboot != 512
 %error Wrong FSIBOOT layout
%endif
