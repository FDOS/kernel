
%if 0

Create a boot image file (default: 1440 KiB diskette) in NASM
 2019, by C. Masloch

Usage of the works is permitted provided that this
instrument is retained with the works, so that any entity
that uses the works is notified of this instrument.

DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

%endif

%if 0

_PAYLOADFILE must be defined to a list of names and/or keywords.
The filenames and keywords are separated by commas.

Filenames are accepted as tokens or strings. A plain filename that
does not belong to a preceeding keyword is taken as a file to add
to the image in the image's current directory. The entire name is
interpreted as a source pathname, passed to the incbin directive
in full, which in turn searches NASM's working directory and the
directories specified to NASM with -I switches. The last component
of the name, after the last slash or backslash, is converted into
allcaps and taken as the target filename.

Keywords are accepted only as (non-string) tokens. The available
keywords are:

::empty		Ignored. Can be used to create an image without files.

::chdir		Change into subdirectory. May be followed by a ..
		 (dotdot) keyword to back out of a directory.
		 Otherwise, the filename of the subdirectory to
		 create (relative to the current directory),
		 as a string, is expected in the next list item.

::rename	Followed by source pathname, then a target filename.

::fill		Followed by length, byte value, target filename.

::endfill	Followed by length, byte value, target filename.
		 Filename is written to the directory as usual.
		 The FAT chain is allocated after all other
		 non-::endfill files however. Additionally, if the
		 byte value is zero, the chain is allocated after
		 all non-zero-value ::endfill files. If _FULL=0
		 then the zero-value ::endfill data will not be
		 written at all, only the FAT entries are allocated.
		If the value is given as 256 this is treated as
		 zero but not handled specifically to make use of
		 _FILL=0 like it is for the literal value 0.

::attrib	Followed by attribute byte value. Sets what
		 attributes to use for the following directory
		 entries. ATTR_DIRECTORY and ATTR_VOLLABEL are
		 invalid to use for this. The _ATTRIB def is
		 used to set the initial default.
		The valid values are: ATTR_READONLY,
		 ATTR_HIDDEN, ATTR_SYSTEM, ATTR_ARCHIVE, and
		 any combination of those.

::date		Followed by numeric date value, eg 20110308.
		 The _FILEDATE def is used to set the initial
		 default. If _USE_FILE_DATETIME is set, this
		 defaults to NASM's __DATE_NUM__, else it
		 defaults to 1980_00_00.

::time		Followed by numeric time value, eg 113842.
		 The _FILETIME def is used to set the initial
		 default. If _USE_FILE_DATETIME is set, this
		 defaults to NASM's __TIME_NUM__, else it
		 defaults to 0.

::setdirdatetime	Special case: Set root directory
			 datetime to current setting, instead
			 of using default _FILEDATE _FILETIME.
			 (Only affects subdir dotdot entries.)

File and directory names are checked for duplicates. If a
string like ::chdir,'foo',::chdir,..,::chdir,'foo' is used,
the subdirectory will be detected as a duplicate. That is,
it is not valid to create/enter the same subdirectory twice.

The order of the files' directory entries and cluster chains in
the file system is determined by the order they were specified
in the list. Particularly, in FAT12 and FAT16 images, the first
two files specified will be placed consecutively at the start
of the data area, which may be necessary for some loaders.
As this is not known to be required by any FAT32 loaders,
the FAT32 root directory counts as the first file.

Files and directories are always written into a single run of
clusters each, that is, there is no fragmentation.

Some things are not supported yet:

* A volume label directory entry.

* Automatic choice of a volume serial number.

* Long file names.

* Boot sector loaders that are not exactly 512 bytes long.

* Automatic calculation of some parameters such as SPF.


A directory is always terminated with one or more directory
entries that are filled with all-zeros.

If _BOOTFILE is specified (not the empty quoted string),
it is expected that the (E)BPB of that boot sector matches
the file system parameters specified to the bootimg.asm file.
If _BOOTFILE equals "" then a proper (E)BPB is created,
along with a default loader (which aborts with an error).
If _BOOTJUMPFILE and _BOOTCODEFILE are specified but
_BOOTFILE is not then these files are used to initialise the
first 512 bytes of the boot sector(s), excepting the (E)BPB.
The size of the jump file has to match 3 bytes or 11 bytes.
The size of the code file has to match the amount of bytes
remaining after the BPBN fields to fill up 512 bytes, or two
bytes less (for an 0AA55h signature), or four bytes less
(for an 0AA55_0000h signature).

If the FAT type is FAT32 and _FSINFO is set, then the def
_BOOTINFOFILE can be specified. The corresponding file must
be exactly 484 bytes long. It is expected that the first
4 bytes contain the FSINFO signature "RRaA". Otherwise the
480-bytes FSIBOOT area is zero-filled.

%endif


%include "lmacros3.mac"

	defaulting
	sectalign off

	numdef SPI, 2880	; sectors per image
	numdef BPS, 512		; bytes per sector
	numdef SPC, 1		; sectors per cluster
	numdef SPF, 9		; sectors per FAT
	numdef BPE, 12		; bits per entry
	numdef FSINFO, 1	; (only if FAT32) include an FSINFO sector
	numdef BACKUP, 1	; (only if FAT32) include a boot sector backup
	numdef FSI_INIT_FREE, 1	; initialise FSINFO amount free clusters
	numdef NUMFATS, 2	; number of FATs
	numdef NUMROOT, 224	; number of root directory entries
%assign _SP512 (512 + _BPS - 1) / _BPS
; (512 + 32 - 1) / 32 = 16
; (512 + 64 - 1) / 64 = 8
; (512 + 512 - 1) / 512 = 1
; (512 + 1024 - 1) / 1024 = 1
	numdef NUMRESERVED, _SP512
				; number of reserved sectors
%if _BPE == 32 && (_FSINFO || _BACKUP)
	numdef NUMRESERVED, _SP512 * 16
				; number of reserved sectors
%endif
	numdef ALIGNDATA, 0, 16	; align data to this sector boundary
				;  (done by increasing _NUMRESERVED)
	numdef FILLROOT, 1
	numdef WARN_TOOMANYFAT, 1
	numdef WARN_ALIGNDATA, 1
	numdef WARN_FILLROOT, 1
	numdef WARN_DEFAULT_OFF, 0
%if _WARN_DEFAULT_OFF
	numdef WARN_TOOMANYFAT, 0
	numdef WARN_ALIGNDATA, 0
	numdef WARN_FILLROOT, 0
%endif
	numdef MEDIAID, 0F0h	; media ID
	numdef EOF, 15		; suffix of EOF entry marker

	numdef UNIT, 0		; load unit in BPB
	numdef CHS_SECTORS, 18	; CHS geometry field for sectors
	numdef CHS_HEADS, 2	; CHS geometry field for heads
	numdef HIDDEN, 0	; number of hidden sectors

	strdef BOOTFILE, ""
	strdef BOOTJUMPFILE, ""
	strdef BOOTCODEFILE, ""
	strdef BOOTINFOFILE, ""
	gendef PAYLOADFILE, ""
	gendef MAP, ""

	strdef OEM_NAME,	"    lDOS"
	strdef OEM_NAME_FILL,	'_'
	strdef DEFAULT_LABEL,	"NO NAME"
	numdef VOLUMEID,	0
	numdef ATTRIB, 0
	numdef FILEDATE, 1980_00_00
	numdef FILETIME, 0
	numdef USE_FILE_DATETIME, 0
%if _USE_FILE_DATETIME
	numdef FILEDATE, __DATE_NUM__
	numdef FILETIME, __TIME_NUM__
%endif

	numdef MBR,			0
	strdef MBRCODEFILE, ""
	gendef MBR_PART_TYPE,		fat %+ _BPE
	numdef MBR_DOSEMU_IMAGE_HEADER,	0
	numdef MBR_GAP_SIZE_SECTORS,	0, 2048
	numdef MBR_GAP_SIZE_CYLINDERS,	1
	numdef MBR_ADD_GAP_TO_HIDDEN,	1
	numdef MBR_ADD_GAP_TO_ALIGN,	1


	numdef FULL,			1
	numdef ZEROBYTES_RESB,		1
	numdef ZEROBYTES_MAX,		4000_0000h
		; Using the resb trick to zero takes about 1/40th
		;  the time of using times (for a 512 MiB image).
%if _ZEROBYTES_RESB
	%imacro zerobytes 1.nolist
%if %1 < 0
 %error Negative count given to zerobytes
%else
[warning -zeroing]
[warning -other]
 %rep (%1) / (_ZEROBYTES_MAX)
	resb (_ZEROBYTES_MAX)
 %endrep
	resb (%1) % (_ZEROBYTES_MAX)
[warning *other]
[warning *zeroing]
%endif
	%endmacro
%else
	%imacro zerobytes 1.nolist
%if %1 < 0
 %error Negative count given to zerobytes
%else
 %rep (%1) / (_ZEROBYTES_MAX)
	times (_ZEROBYTES_MAX) db 0
 %endrep
	times (%1) % (_ZEROBYTES_MAX) db 0
%endif
	%endmacro
%endif


	numdef FILLBYTES_MAX,		4000_0000h
	%imacro fillbytes 2.nolist
%ifn %2
	zerobytes %1
%else
 %rep (%1) / (_FILLBYTES_MAX)
	times (_FILLBYTES_MAX) db %2
 %endrep
	times (%1) % (_FILLBYTES_MAX) db %2
%endif
	%endmacro


	%imacro zerobytes_if_full 1.nolist
%if _FULL
zerobytes %1
%endif
	%endmacro


ptEmpty:		equ 0
ptFAT12:		equ 1
ptFAT16_16BIT_CHS:	equ 4
ptExtendedCHS:		equ 5
ptFAT16_CHS:		equ 6
ptFAT32_CHS:		equ 0Bh
ptFAT32:		equ 0Ch
ptFAT16:		equ 0Eh
ptExtended:		equ 0Fh
ptLinux:		equ 83h
ptExtendedLinux:	equ 85h

%if _MBR
 %if _MBR_GAP_SIZE_SECTORS == 0
  %if _MBR_GAP_SIZE_CYLINDERS == 0
   %error Invalid gap size specified
  %else
   %assign _MBR_GAP_SIZE_SECTORS _MBR_GAP_SIZE_CYLINDERS * _CHS_HEADS * _CHS_SECTORS
  %endif
 %endif
 %if _MBR_ADD_GAP_TO_HIDDEN
  %assign _HIDDEN _HIDDEN + _MBR_GAP_SIZE_SECTORS
 %endif
 %ifidni _MBR_PART_TYPE,	fat12
  %assign _MBR_PART_TYPE	ptFAT12
 %elifidni _MBR_PART_TYPE,	fat16_16bit_chs
  %assign _MBR_PART_TYPE	ptFAT16_16BIT_CHS
 %elifidni _MBR_PART_TYPE,	fat16_chs
  %assign _MBR_PART_TYPE	ptFAT16_CHS
 %elifidni _MBR_PART_TYPE,	fat32_chs
  %assign _MBR_PART_TYPE	ptFAT32_CHS
 %elifidni _MBR_PART_TYPE,	fat32
  %assign _MBR_PART_TYPE	ptFAT32
 %elifidni _MBR_PART_TYPE,	fat16
  %assign _MBR_PART_TYPE	ptFAT16
 %endif
 %assign _MBR_PART_TYPE _MBR_PART_TYPE
 %ifnnum _MBR_PART_TYPE
  %error Invalid partition type specified
 %endif
 %if _MBR_PART_TYPE == 0
  %error Invalid partition type specified
 %endif
%endif

%ifnidn _MAP, ""
 [map all _MAP]
%endif

%ifidn _PAYLOADFILE, ""
  %error No payload files specified!
%endif


%if _BPE == 12
%elif _BPE == 16
%elif _BPE == 32
%else
 %error Invalid BPE (_BPE)
%endif

%if _BPE != 32
 %if _FILLROOT
  %assign ii (_NUMROOT + _BPS / 32 - 1) & ~(_BPS / 32 - 1)
  %if _WARN_FILLROOT && ii != _NUMROOT
   %warning Expanding root to ii entries to fill last sector
  %endif
  %assign _NUMROOT ii
 %endif
 %assign ROOTSECTORS (_NUMROOT * 32 + _BPS - 1) / _BPS
%else
 %assign ROOTSECTORS 0
%endif
%assign FATSECTORS _NUMFATS * _SPF

%if _ALIGNDATA
 %assign ii 0
 %if _MBR && _MBR_ADD_GAP_TO_ALIGN
  %assign ii ii + _MBR_GAP_SIZE_SECTORS
 %endif
 %assign ii ii + _NUMRESERVED + ROOTSECTORS + FATSECTORS
 %assign ii (ii + (_ALIGNDATA)) % (_ALIGNDATA)
 %assign ii ((_ALIGNDATA) - ii) % (_ALIGNDATA)
 %if ii
  %if _WARN_ALIGNDATA
   %warning Adding ii reserved sectors for %[_ALIGNDATA]-sector alignment
  %endif
 %endif
 %assign _NUMRESERVED _NUMRESERVED + ii
%endif

%assign DATASECTORS (_SPI - _NUMRESERVED - FATSECTORS - ROOTSECTORS)
%assign DATACLUSTERS (DATASECTORS / _SPC)
%assign NUMENTRIES _SPF * _BPS * 8 / _BPE
%assign NUMSECTORS _SPF
%assign NUMNEEDED (DATACLUSTERS + 2)
%assign SECNEEDED (NUMNEEDED * _BPE / 8 + _BPS - 1) / _BPS
%if NUMENTRIES < NUMNEEDED
 %error Too few FAT sectors specified \
 (NUMENTRIES entries (NUMSECTORS sectors), NUMNEEDED needed (SECNEEDED sectors))
%elif NUMENTRIES >= (NUMNEEDED + _BPS * 8 / _BPE)
 %if _WARN_TOOMANYFAT
  %warning Too many FAT sectors specified \
 (NUMENTRIES entries (NUMSECTORS sectors), NUMNEEDED needed (SECNEEDED sectors))
 %endif
%endif

%macro detectionerror 1.nolist
 %assign len 8
 %assign ii 0
 %assign sum 0
 %define string ""
 %rep len
  %assign shift ((len - 1 - ii) * 4)
  %assign digit (DATACLUSTERS >> shift) & 15
  %assign sum sum + digit
  %if sum
   %substr digit "0123456789ABCDEF" digit + 1
   %strcat string string,digit
  %endif
  %assign ii ii + 1
 %endrep
 %ifn sum
  %define string "0"
 %endif
 %deftok string string
 %error FAT would be detected %1 (%[DATACLUSTERS] = %[string]h clusters)
%endmacro

%if (DATACLUSTERS + 2) > 0FFF0h
 %if _BPE == 12 || _BPE == 16
  detectionerror as FAT32
 %endif
%elif (DATACLUSTERS + 2) > 0FF0h
 %if _BPE == 12
  detectionerror as FAT16
 %endif
%endif
%if (DATACLUSTERS + 2) < 1000h
 %if _BPE == 16 || _BPE == 32
  detectionerror as FAT12
 %endif
%elif (DATACLUSTERS + 2) < 10000h
 %if _BPE == 32
  detectionerror as FAT16
 %endif
%endif

%if _EOF < 8 || _EOF > 15
 %error Invalid EOF value (_EOF)
%endif

%if _BPE != 32
 %assign MAXENTRY (1 << _BPE) - 1
%else
 %assign MAXENTRY (1 << 28) - 1
%endif
%assign EOFSTARTENTRY MAXENTRY - 7
%assign EOFENDENTRY MAXENTRY
%assign BADENTRY MAXENTRY - 8
%assign EOFENTRY MAXENTRY - 15 + _EOF


	struc PARTINFO_CHS_TUPLE
pictHead:		resb 1
pictSectorLow6:
pictCylinderHigh2:	resb 1
pictCylinderLow8:	resb 1
	endstruc

	%macro chs_tuple 1.nolist
%assign %%sector (%1) % _CHS_SECTORS
%assign %%i (%1) / _CHS_SECTORS
%assign %%head %%i % _CHS_HEADS
%assign %%cylinder %%i / _CHS_HEADS
%if %%cylinder >= 1024
 %assign %%cylinder 1023
%endif
		istruc PARTINFO_CHS_TUPLE
at pictHead,		db %%head
at pictSectorLow6
at pictCylinderHigh2,	db (%%sector + 1) | ((%%cylinder >> (8 - 6)) & 0C0h)
at pictCylinderLow8,	db (%%cylinder & 0FFh)
		iend
	%endmacro

	struc PARTINFO
piBoot:		resb 1
piStartCHS:	resb 3		; PARTINFO_CHS_TUPLE
piType:		resb 1
piEndCHS:	resb 3		; PARTINFO_CHS_TUPLE
piStart:	resd 1
piLength:	resd 1
	endstruc

	struc BS
bsJump:	resb 3
bsOEM:	resb 8
bsBPB:
	endstruc

	struc EBPB		;        BPB sec
bpbBytesPerSector:	resw 1	; offset 00h 0Bh
bpbSectorsPerCluster:	resb 1	; offset 02h 0Dh
bpbReservedSectors:	resw 1	; offset 03h 0Eh
bpbNumFATs:		resb 1	; offset 05h 10h
bpbNumRootDirEnts:	resw 1	; offset 06h 11h -- 0 for FAT32
bpbTotalSectors:	resw 1	; offset 08h 13h
bpbMediaID:		resb 1	; offset 0Ah 15h
bpbSectorsPerFAT:	resw 1	; offset 0Bh 16h -- 0 for FAT32
bpbCHSSectors:		resw 1	; offset 0Dh 18h
bpbCHSHeads:		resw 1	; offset 0Fh 1Ah
bpbHiddenSectors:	resd 1	; offset 11h 1Ch
bpbTotalSectorsLarge:	resd 1	; offset 15h 20h
bpbNew:				; offset 19h 24h

ebpbSectorsPerFATLarge:	resd 1	; offset 19h 24h
ebpbFSFlags:		resw 1	; offset 1Dh 28h
ebpbFSVersion:		resw 1	; offset 1Fh 2Ah
ebpbRootCluster:	resd 1	; offset 21h 2Ch
ebpbFSINFOSector:	resw 1	; offset 25h 30h
ebpbBackupSector:	resw 1	; offset 27h 32h
ebpbReserved:		resb 12	; offset 29h 34h
ebpbNew:			; offset 35h 40h
	endstruc

	struc BPBN		; ofs B16 S16 B32 S32
bpbnBootUnit:		resb 1	; 00h 19h 24h 35h 40h
			resb 1	; 01h 1Ah 25h 36h 41h
bpbnExtBPBSignature:	resb 1	; 02h 1Bh 26h 37h 42h -- 29h for valid BPBN
bpbnSerialNumber:	resd 1	; 03h 1Ch 27h 38h 43h
bpbnVolumeLabel:	resb 11	; 07h 20h 2Bh 3Ch 47h
bpbnFilesystemID:	resb 8	; 12h 2Bh 36h 47h 52h
	endstruc		; 1Ah 33h 3Eh 4Fh 5Ah

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

	struc DOSEMU_IMAGE_HEADER
; This structure is parsed when determining the geometry for
;  a hard disk image. It is documented in the dosemu2 sources at:
; https://github.com/dosemu2/dosemu2/blob/f41c0f267fec/src/include/disks.h#L132
dihSig:		resb 7		; "DOSEMU",0 or 0Eh,"DEXE"
dihCHSHeads:	resd 1		; (note the misalignment for these)
dihCHSSectors:	resd 1
dihCHSCylinders:resd 1
dihHeaderSize:	resd 1
dihDummy:	resb 1
dihDEXEFlags:	resd 1
	endstruc
dih_our_size:	equ 8192	; (16 * 512)

	%imacro checkattrib 0
%if _ATTRIB & (ATTR_VOLLABEL | ATTR_DIRECTORY)
 %error Invalid attribute: _ATTRIB
%endif
	%endmacro
	checkattrib

	%imacro convertdatetime 0
%push
%assign %$year _FILEDATE / 1_00_00
%assign %$month _FILEDATE / 1_00 % 100
%assign %$day _FILEDATE % 100
%if %$year < 1980
 %assign _FATFILEDATE 0 << 9 | 1 << 5 | 1
%elif (%$year - 1980) >= 128
 %assign _FATFILEDATE 127 << 9 | 12 << 5 | 31
%else
 %assign _FATFILEDATE (%$year - 1980) << 9 | %$month << 5 | %$day
%endif
%assign %$hour _FILETIME / 1_00_00
%assign %$minute _FILETIME / 1_00 % 100
%assign %$second _FILETIME % 100
%assign _FATFILETIME %$hour << 11 | %$minute << 5 | %$second >> 1
%pop
	%endmacro
	convertdatetime


%define STARTSFOLLOWS start=0

%if _MBR
CYLINDERS equ (_MBR_GAP_SIZE_SECTORS + _SPI + _CHS_HEADS * _CHS_SECTORS - 1) \
	/ (_CHS_HEADS * _CHS_SECTORS)


 %if _MBR_DOSEMU_IMAGE_HEADER
	addsection dosemu_image_header, STARTSFOLLOWS
  %define STARTSFOLLOWS follows=dosemu_image_header
	istruc DOSEMU_IMAGE_HEADER
at dihSig,		asciz "DOSEMU"
at dihCHSHeads,		dd _CHS_HEADS
at dihCHSSectors,	dd _CHS_SECTORS
at dihCHSCylinders,	dd CYLINDERS
at dihHeaderSize,	dd dih_our_size
	iend
	times dih_our_size - ($ - $$) db 0
 %endif


	addsection mbr, STARTSFOLLOWS vstart=7C00h
 %define STARTSFOLLOWS follows=mbr

mbr_start:
 %ifnidn _MBRCODEFILE, ""
	incbin _MBRCODEFILE
 %else
	cli
	cld
	xor ax, ax
	mov es, ax
	mov ds, ax
	mov ss, ax
	mov sp, 7C00h
	sti
	mov si, mbr_message
@@:
	lodsb
	test al, al
	jz @F
	mov ah, 0Eh
	mov bh, [462h]
	mov bl, 7
	int 10h
	jmp @B

@@:
	xor ax, ax
	int 13h
	xor ax, ax
	int 16h
	int 19h

mbr_message:
	db "Unable to boot, MBR loader not written.",13,10
	db 13,10
	db "Press any key to reboot.",13,10
	db 0
 %endif

	times (512 - 2 - 4 * 16) - ($ - $$) db 0

%assign TOTAL_MBR_SECTORS CYLINDERS * _CHS_HEADS * _CHS_SECTORS
mbr_partition_table:
	istruc PARTINFO
at piBoot,	db 80h			; active primary partition
at piStartCHS
		chs_tuple _MBR_GAP_SIZE_SECTORS
at piType,	db _MBR_PART_TYPE
at piEndCHS
		chs_tuple (TOTAL_MBR_SECTORS - 1)
at piStart,	dd _MBR_GAP_SIZE_SECTORS
at piLength,	dd TOTAL_MBR_SECTORS - _MBR_GAP_SIZE_SECTORS
	iend

	times (512 - 2) - ($ - $$) db 0
mbr_signature:
	dw 0AA55h
	align _BPS, db 0


	addsection gap, STARTSFOLLOWS
 %define STARTSFOLLOWS follows=gap

	zerobytes (_MBR_GAP_SIZE_SECTORS - 1) * _BPS
%endif


	%imacro emit_boot 2
	addsection %1_boot, STARTSFOLLOWS vstart=7C00h
 %define STARTSFOLLOWS follows=%1_boot

%1_boot_start:
%ifnidn _BOOTFILE, ""
	incbin _BOOTFILE
%else
 %ifnidn _BOOTJUMPFILE, ""
	incbin _BOOTJUMPFILE
 %else
	jmp strict short %1_boot_after_bpb
	nop
 %endif
 %if ($ - $$) == bsOEM
%1_oem_id:		; offset 03h (03)
	fill 8,_OEM_NAME_FILL,db _OEM_NAME
 %endif
 %if ($ - $$) != bsBPB
  %error Invalid boot jump file length, must be 11 or 3
 %endif
%1_bytes_per_sector:	; offset 0Bh (11)
	dw _BPS
%1_sectors_per_cluster:	; offset 0Dh (13)
	db _SPC & 255
%1_num_reserved_sectors:; offset 0Eh (14)
	dw _NUMRESERVED
%1_num_fats:		; offset 10h (16)
	db _NUMFATS
%1_num_root_dir_ents:	; offset 11h (17)
%if _BPE != 32
	dw _NUMROOT
%else
	dw 0
%endif
%1_total_sectors:	; offset 13h (19)
%if _SPI < 1_0000h
	dw _SPI
%else
	dw 0
%endif
%1_media_id:		; offset 15h (21)
	db _MEDIAID
%1_sectors_per_fat:	; offset 16h (22)
%if _BPE != 32
	dw _SPF
%else
	dw 0
%endif
%1_sectors_per_track:	; offset 18h (24)
	dw _CHS_SECTORS
%1_heads:		; offset 1Ah (26)
	dw _CHS_HEADS
%1_hidden_sectors:	; offset 1Ch (28)
	dd _HIDDEN
%1_total_sectors_large:	; offset 20h (32)
%if _SPI >= 1_0000h
	dd _SPI
%else
	dd 0
%endif

; Extended BPB		; offset 24h (36)

%if _BPE == 32
%1_sectors_per_fat_large:
			; offset 24h (36)
	dd _SPF
%1_fsflags:		; offset 28h (40)
	dw 0
%1_fsversion:		; offset 2Ah (42)
	dw 0
%1_root_cluster:	; offset 2Ch (44)
	dd rootcluster
%1_fsinfo_sector:	; offset 30h (48)
%if _FSINFO
	dw (%1_fsinfo - %1_boot_start) / _BPS
%else
	dw 0
%endif
%1_backup_sector:	; offset 32h (50)
%if _BACKUP
	dw (_boot_end_2 - _boot_start) / _BPS
%else
	dw 0
%endif

	times 12 db 0	; offset 34h (52) reserved

; Extended BPB		; offset 40h (64)
%endif

%1_boot_unit:		db _UNIT
			db 0
%1_ext_bpb_signature:	db 29h
%1_serial_number:	dd _VOLUMEID
%1_volume_label:	fill 11,32,db _DEFAULT_LABEL
%1_filesystem_identifier:
%if _BPE == 12
	fill 8,32,db "FAT12"
%elif _BPE == 16
	fill 8,32,db "FAT16"
%elif _BPE == 32
	fill 8,32,db "FAT32"
%else
 %error Invalid BPE
%endif


%1_boot_after_bpb:
 %ifnidn _BOOTCODEFILE, ""
	incbin _BOOTCODEFILE
 %else
	cli
	cld
	xor ax, ax
	mov es, ax
	mov ds, ax
	mov ss, ax
	mov sp, 7C00h
	sti
	mov si, %1_boot_message
@@:
	lodsb
	test al, al
	jz @F
	mov ah, 0Eh
	mov bh, [462h]
	mov bl, 7
	int 10h
	jmp @B

@@:
	xor ax, ax
	int 13h
	xor ax, ax
	int 16h
	int 19h

%1_boot_message:
	db "Unable to boot, loader not written.",13,10
	db 13,10
	db "Press any key to reboot.",13,10
	db 0

	times 508 - ($ - $$) db 0
 %endif
%endif
%if ($ - $$) == 508
	dw 0
%endif
%if ($ - $$) == 510
	dw 0AA55h
%endif
%1_boot_end:

%if (%1_boot_end - %1_boot_start) != 512
 %ifnidn _BOOTFILE, ""
  %error Boot file has wrong size, must be 512, 510, or 508
 %elifnidn _BOOTCODEFILE, ""
  %assign numberexpected1 512 - (%1_boot_after_bpb - $$)
  %assign numberexpected2 numberexpected1 - 2
  %assign numberexpected3 numberexpected1 - 4
  %error Boot code file has wrong size, must be numberexpected1, numberexpected2, or numberexpected3
 %else
  %error Internal error, default boot sector loader has wrong size
 %endif
%endif
%if _BPS > 512
	_fill _BPS - 4, 0, %1_boot_start
	dd 0AA55_0000h
%endif

%if _BPE == 32 && _FSINFO
%1_fsinfo:
	istruc FSINFO
 %ifnidn _BOOTINFOFILE, ""
	incbin _BOOTINFOFILE
 %else
at FSINFO.signature1
	dd "RRaA"
at FSINFO.reserved1
	_fill 480 + 4, 0, %1_fsinfo
 %endif
 %if $ - %1_fsinfo != FSINFO.signature2
  Boot info file has wrong size
 %endif
at FSINFO.signature2
	dd "rrAa"
at FSINFO.numberfree
%if _FSI_INIT_FREE && %2
	dd DATACLUSTERS - USEDCLUSTERS
%else
	dd -1		; number of free clusters
%endif
at FSINFO.nextfree
	dd -1		; first free cluster
at FSINFO.reserved2
	times 3 dd 0
at FSINFO.signature3
	dd 0_AA55_0000h
	iend
 %if _BPS > 512
	times (_BPS - 512 - 4) db 0
	dd 0_AA55_0000h
 %endif
%endif

%if _BACKUP && _BPE == 32
 %if _BPS < 512
	zerobytes (6 * 512) - ($ - $$)
 %else
	zerobytes (6 * _BPS) - ($ - $$)
 %endif
%endif
%1_boot_end_2:
%assign bootused bootused + ($ - $$)

	%endmacro


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

%assign bootused 0
	emit_boot ,1
%if _BACKUP && _BPE == 32
	emit_boot backup,0
%endif
	zerobytes (_NUMRESERVED * _BPS) - bootused


	addsection fat, STARTSFOLLOWS align=_BPS


		; %1 = name for the equ to receive the start cluster,
		;	_start_cluster will be appended
		; %2 = how many clusters/entries to allocate
		;	(if zero, %1_start_cluster equ will be zero)
	%macro addfatchain 2.nolist
%xdefine FATCHAINS FATCHAINS,%1,%2
	%endmacro
%define FATCHAINS secondentry,1


	addsection fixed_root, follows=fat align=_BPS


		; %1 = directory name ("" if is root)
		; %2 = data section name to use
		; %3 = data section name of dir to write to (none if is root)
		; %$parentcluster = data cluster number of parent dir (0 if root is parent)
		; %$parentattrib = attrib of parent dir (0 if root is parent)
		; %$parentfiletime = file time of parent dir
		; %$parentfiledate = file date of parent dir
	%macro incdir 3.nolist
	usesection %2
%2 %+ _start:
%ifnidni %1, ""
	istruc DIRENTRY
at deName,		fill 8,32,db "."
at deExt,		fill 3,32,db 32
at deAttrib,		db ATTR_DIRECTORY | _ATTRIB
at deClusterHigh,	dw %%file_start_cluster >> 16
at deTime,		dw _FATFILETIME
at deDate,		dw _FATFILEDATE
at deClusterLow,	dw %%file_start_cluster & 0FFFFh
at deSize,		dd 0
	iend
	istruc DIRENTRY
at deName,		fill 8,32,db ".."
at deExt,		fill 3,32,db 32
at deAttrib,		db ATTR_DIRECTORY | %$$parentattrib
at deClusterHigh,	dw %$$parentcluster >> 16
at deTime,		dw %$$parentfiletime
at deDate,		dw %$$parentfiledate
at deClusterLow,	dw %$$parentcluster & 0FFFFh
at deSize,		dd 0
	iend
%endif

%xdefine CHAINLIST CHAINLIST,%%file

%ifnidni %3, none

%define string %1
%ifnstr string
 %defstr string string
%endif
	parsename %1, 1

	usesection %3
	istruc DIRENTRY
at deName,		fill 8,32,db %$$name
at deExt,		fill 3,32,db %$$ext
at deAttrib,		db ATTR_DIRECTORY | _ATTRIB
at deClusterHigh,	dw %%file_start_cluster >> 16
at deTime,		dw _FATFILETIME
at deDate,		dw _FATFILEDATE
at deClusterLow,	dw %%file_start_cluster & 0FFFFh
at deSize,		dd 0
	iend
%else
 rootcluster equ %%file_start_cluster
%endif
%2 %+ _start_cluster equ %%file_start_cluster
	%endmacro


		; %1 = source filename, may include a directory
		;	(the directory is used to locate
		;	the file on the host system)
		; %2 = target filename, may include a directory
		;	(directory is stripped, base name is
		;	used in all-caps to store to the image)
		; %3 = data section name to use
		; %4 = data section name of directory to write to
		; %5 = nonzero if filling data
		; %6 = size of data to fill
		; %7 = value to fill
		; isrename = nonzero if ::rename
		; isfill = nonzero if ::fill
	%macro incfile 4-7.nolist 0
	usesection %3
%3 %+ _start:
%if %5
 fillbytes %6, %7
%else
 %define string %1
 %ifnstr string
  %defstr string string
 %endif
 incbin string
%endif
%assign filesize ($ - %3 %+ _start)

%xdefine CHAINLIST CHAINLIST,%%file

%define string %2
%ifnstr string
 %defstr string string
%endif
	parsename %2, !!(isrename || isfill)

	usesection %4
	istruc DIRENTRY
at deName,		fill 8,32,db %$$name
at deExt,		fill 3,32,db %$$ext
at deAttrib,		db _ATTRIB
at deClusterHigh,	dw %%file_start_cluster >> 16
at deTime,		dw _FATFILETIME
at deDate,		dw _FATFILEDATE
at deClusterLow,	dw %%file_start_cluster & 0FFFFh
at deSize,		dd filesize
	iend
	%endmacro


		; %1 = filename
		; %2 = data section name of directory to write to
		; %3 = fill length
		; %4 = fill value
	%imacro incendfill 4.nolist
%if %4
 %if %4 == 256
  %xdefine LISTENDFILL LISTENDFILL, %%file, %3, 0
 %else
  %xdefine LISTENDFILL LISTENDFILL, %%file, %3, %4
 %endif
%else
 %xdefine LISTENDFILLZEROS LISTENDFILLZEROS, %%file, %3, %4
%endif

%define string %1
%ifnstr string
 %defstr string string
%endif
	parsename %1, 1

	usesection %2
	istruc DIRENTRY
at deName,		fill 8,32,db %$$name
at deExt,		fill 3,32,db %$$ext
at deAttrib,		db _ATTRIB
at deClusterHigh,	dw %%file_start_cluster >> 16
at deTime,		dw _FATFILETIME
at deDate,		dw _FATFILEDATE
at deClusterLow,	dw %%file_start_cluster & 0FFFFh
at deSize,		dd %3
	iend
	%endmacro


		; %1 = filename, with directory as on host side
		; string = stringified filename, still with host path
		; %2 = if slash disallowed in pathname
		; returns result in %$name, %$ext, and %$namelist
	%macro parsename 1-2.nolist 0
%strlen length string
%assign ii 1
%rep length
 %substr cc string length - ii + 1
 %ifidn cc,"/"
  %if %2
   %if ischdir
    %error Subsubdirectory not supported, use ::chdir twice
   %elif isrename
    %error Subdirectory not supported in ::rename target
   %elif isfill
    %error Subdirectory not supported in ::fill target
   %else
    %error Subdirectory not supported
   %endif
  %endif
  %substr string string length -ii + 2, -1
  %exitrep
 %endif
 %ifidn cc,"\"
  %if %2
   %if ischdir
    %error Subsubdirectory not supported, use ::chdir twice
   %elif isrename
    %error Subdirectory not supported in ::rename target
   %elif isfill
    %error Subdirectory not supported in ::fill target
   %else
    %error Subdirectory not supported
   %endif
  %endif
  %substr string string length -ii + 2, -1
  %exitrep
 %endif
 %assign ii ii + 1
%endrep
%strlen length string
%assign ii 1
%define %$name ""
%define %$ext ""
%assign dotyet 0
%rep length
 %substr cc string ii
 %assign ii ii + 1
 %if cc >= 'a' && cc <= 'z'
  %substr cc "ABCDEFGHIJKLMNOPQRSTUVWXYZ" (cc - 'a' + 1)
 %endif
 %ifn dotyet
  %ifidn cc,"."
   %assign dotyet 1
  %else
   %strlen ll %$name
   %if ll >= 8
    %error Too long name part in %1
    %exitrep
   %endif
   checkchar %1,cc
   %strcat %$name %$name,cc
  %endif
 %else
  %strlen ll %$ext
  %if ll >= 3
   %error Too long ext part in %1
   %exitrep
  %else
   checkchar %1,cc,"."
   %strcat %$ext %$ext,cc
  %endif
 %endif
%endrep
%ifidn %$name,""
 %error Invalid empty name part in %1
%endif

	addnametonamelist %$namelist
	%endmacro

	%macro checkchar 2-3.nolist 0
%if %2 <= ' ' || %2 >= 128 || \
	%2 == '/' || %2 == '\' || \
	%2 == '"' || %2 == %3
 %error Invalid character (%2) in name (%1)
%endif
	%endmacro

		; %$name = (string) filename part, 1 to 8 characters, all caps
		; %$ext = (string) file extension part, 0 to 3 characters, all caps
		; %3 = first saved name part in name list
		; %4 = first saved extension part in name list
		; %5, %6 = next filename in name list
		; %$namelist = name list (pairs of name parts, extension parts)
	%macro addnametonamelist 2-*.nolist
%ifnidn %1, ""
 %error Expected first list entry to be empty
%elifnidn %2, ""
 %error Expected first list entry to be empty
%elif %0 & 1
 %error Expected list to contain even number of entries
%endif
%rotate 2
%rep (%0 - 2) / 2
 %ifidn %1, %$name
  %ifidn %2, %$ext
   %error Duplicate filename %$name %+ . %+ %$ext in directory %$dirname
   %exitrep
  %endif
 %endif
 %rotate 2
%endrep
%xdefine %$namelist %$namelist, %$name, %$ext
	%endmacro


	%macro setup_a_section 0.nolist
	addsection data %+ filescount, follows= %+ ff vstart=0
  %xdefine ff data %+ filescount
  %assign filescount filescount + 1
	%endmacro


%define DIRLIST none
%define CHAINLIST none
%define LISTENDFILL none, none, none
%define LISTENDFILLZEROS none, none, none
	%macro multiincfile 1-*.nolist
%assign filescount 0
%define ff fixed_root
%assign chainindex 0
%assign ischdir 0
%assign isrename 0
%assign isfill 0
%assign isattrib 0
%assign isdatetime 0
%assign isdate 0
%assign istime 0
%push ROOT
%define %$namelist "", ""
%define %$dirname "/"
%if _BPE == 32
	setup_a_section		; add a section for root directory
	incdir "", data %+ chainindex, none
 %xdefine nextdir data %+ chainindex
 %xdefine DIRLIST DIRLIST,nextdir
 %xdefine %$parent nextdir
; %xdefine %$parentcluster %$parent %+ _start_cluster
 %define %$parentcluster 0
 %assign chainindex chainindex + 1
%else
 %define %$parent fixed_root
 %define %$parentcluster 0
%endif
%assign %$parentattrib 0
%assign %$parentfiletime _FATFILETIME
%assign %$parentfiledate _FATFILEDATE
%rep %0
 %if ischdir
  %ifidni %1, ..
   %ifctx ROOT
    %error Cannot back out over root
   %else
    %pop
   %endif
  %else
	setup_a_section		; add a section for a directory
	incdir %1, data %+ chainindex, %$parent
   %push SUB
   %define %$namelist "", ""
   %strcat %$dirname %$$dirname, %$$name, ".", %$$ext, "/"
   %xdefine nextdir data %+ chainindex
   %xdefine DIRLIST DIRLIST,nextdir
   %xdefine %$parent nextdir
   %xdefine %$parentcluster %$parent %+ _start_cluster
   %assign %$parentattrib _ATTRIB
   %assign %$parentfiletime _FATFILETIME
   %assign %$parentfiledate _FATFILEDATE
   %assign chainindex chainindex + 1
  %endif
  %assign ischdir 0
 %elif isrename == 1
  %xdefine renamesource %1
  %assign isrename 2
 %elif isrename == 2
	setup_a_section		; add a section for a file
	incfile renamesource, %1, data %+ chainindex, %$parent
  %assign chainindex chainindex + 1
  %assign isrename 0
 %elif isfill == 1 || isfill == 4
  %xdefine filllength %1
  %assign isfill isfill + 1
 %elif isfill == 2 || isfill == 5
  %xdefine fillvalue %1
  %assign isfill isfill + 1
 %elif isfill == 3
	setup_a_section		; add a section for a file
	incfile "", %1, data %+ chainindex, %$parent, 1, filllength, fillvalue
  %assign chainindex chainindex + 1
  %assign isfill 0
 %elif isfill == 6
	incendfill %1, %$parent, filllength, fillvalue
  %assign isfill 0
 %elif isattrib
  %assign _ATTRIB %1
	checkattrib
  %assign isattrib 0
 %elif isdatetime == 1
  %assign _FILEDATE %1
  %assign isdatetime 2
 %elif isdatetime == 2
  %assign _FILETIME %1
	convertdatetime
  %assign isdatetime 0
 %elif isdate
  %assign _FILEDATE %1
	convertdatetime
  %assign isdate 0
 %elif istime
  %assign _FILETIME %1
	convertdatetime
  %assign istime 0
 %elifidni %1, ::chdir
  %assign ischdir 1
 %elifidni %1, ::rename
  %assign isrename 1
 %elifidni %1, ::fill
  %assign isfill 1
 %elifidni %1, ::endfill
  %assign isfill 4
 %elifidni %1, ::attrib
  %assign isattrib 1
 %elifidni %1, ::datetime
  %assign isdatetime 1
 %elifidni %1, ::date
  %assign isdate 1
 %elifidni %1, ::time
  %assign istime 1
 %elifidni %1, ::setdirdatetime
  %assign %$parentfiletime _FATFILETIME
  %assign %$parentfiledate _FATFILEDATE
 %elifnidni %1, ::empty
  %ifidn %1, ""
   %error Invalid filename
  %elifempty %1
   %error Invalid filename
  %else
	setup_a_section		; add a section for a file
	incfile %1, %1, data %+ chainindex, %$parent
   %assign chainindex chainindex + 1
  %endif
 %endif
 %rotate 1
%endrep
%if ischdir
 %error No directory to create specified
%endif
%if isrename == 1
 %error No rename source specified
%endif
%if isrename == 2
 %error No rename target specified
%endif
%if isfill == 1 || isfill == 4
 %error No fill length specified
%endif
%if isfill == 2 || isfill == 5
 %error No fill value specified
%endif
%if isfill == 3 || isfill == 6
 %error No fill target specified
%endif
%if isattrib
 %error No attributes value specified
%endif
%if isdatetime == 1
 %error No datetime date value specified
%endif
%if isdatetime == 2
 %error No datetime time value specified
%endif
%if isdate
 %error No date value specified
%endif
%if istime
 %error No time value specified
%endif
%rep 1024
 %ifctx ROOT
  %exitrep
 %else
  %pop
 %endif
%endrep
%pop
	%endmacro
	multiincfile _PAYLOADFILE


	%macro extenddirectories 1-*.nolist
%ifnidni %1, none
 %error Expected a none entry to start dir list
%endif
%rotate 1
%rep %0 - 1
	usesection %1
	times 32 db 0
		; Insure at least one entry. Makes things
		;  easier to handle for the loader too.
		;  (Can depend on a zero-filled entry
		;  that terminates the directory.)
 %rotate 1
%endrep
	%endmacro

	extenddirectories DIRLIST


	%macro allocatechains 1-*.nolist
%ifnidni %1, none
 %error Expected a none entry to start chain list
%endif
%rotate 1
%assign ii 0
%rep %0 - 1
	usesection data %+ ii
	align _BPS * _SPC, db 0
 %assign clusters ($ - data %+ ii %+ _start) / (_BPS * _SPC)
	addfatchain %1,clusters
 %assign ii ii + 1
 %rotate 1
%endrep
	%endmacro

	%macro allocateendfill 3-*.nolist
%ifnidni %1, none
 %error Expected a none entry to start chain list
%endif
%if %0 % 3
 %error Expected an amount of chain list entries divisible by 3
%endif
	; ii as left from allocatechains
%rep (%0 - 3) / 3
 %rotate 3
	setup_a_section
	usesection data %+ ii
data %+ ii %+ _start:
 %if isendfillzeros
	zerobytes_if_full %2
 %else
	fillbytes %2, %3
 %endif
	align _BPS * _SPC, db 0
 %assign clusters (%2 + _BPS * _SPC - 1) / (_BPS * _SPC)
 %assign data_used data_used + (clusters * _BPS * _SPC)
	addfatchain %1,clusters
 %assign ii ii + 1
%endrep
	%endmacro

	allocatechains CHAINLIST

%assign data_used 0
%assign ii 0
%rep filescount
	usesection data %+ ii
	align _BPS * _SPC, db 0
data %+ ii %+ _end:
 %assign data_used data_used \
	+ (data %+ ii %+ _end - data %+ ii %+ _start)
 %assign ii ii + 1
%endrep

%assign isendfillzeros 0
	allocateendfill LISTENDFILL
%assign isendfillzeros 1
	allocateendfill LISTENDFILLZEROS
	addsection empty, follows= %+ ff vstart=0


	usesection fixed_root
%if _BPE != 32
	times 32 db 0
%endif
	zerobytes ROOTSECTORS * _BPS - ($ - $$)


USEDCLUSTERS equ data_used / (_BPS * _SPC)
	usesection empty
%if data_used > (DATASECTORS * _BPS)
 %error Too much data used
%endif
	zerobytes_if_full DATASECTORS * _BPS - data_used


%if _MBR
	addsection partition_end_padding, follows=empty
	zerobytes_if_full (CYLINDERS * _CHS_HEADS * _CHS_SECTORS \
			- _MBR_GAP_SIZE_SECTORS - _SPI) \
		* _BPS
%endif


%if _BPE == 12
	%macro dumpfatentry 1.nolist
%ifempty EVENENTRY
 %xdefine EVENENTRY %1
%else
	db (EVENENTRY) & 0FFh
	db ((EVENENTRY) >> 8) \
		| (((%1) & 0Fh) << 4)
	db ((%1) >> 4)
 %define EVENENTRY
%endif
	%endmacro
%elif _BPE == 16
 %define dumpfatentry dw
%elif _BPE == 32
 %define dumpfatentry dd
%else
 %error Invalid bpe
%endif

	%macro expandfatchains 2-*.nolist
%define EVENENTRY
dumpfatentry (_MEDIAID | (MAXENTRY - 15))
				; first entry has media ID byte
%assign ii 1			; first entry is at index 0,
				;  next is index 1
%rep %0 / 2			; for each pair
 %if %2
  %1_start_cluster equ ii	; equ for start cluster
  %rep %2 - 1			; as many as are non-EOF entries
   %assign ii ii + 1
   dumpfatentry ii		; link to next cluster
  %endrep
  %assign ii ii + 1
  dumpfatentry EOFENTRY		; EOF entry
 %else
  %1_start_cluster equ 0	; empty chain, start cluster is 0
 %endif
 %rotate 2			; rotate to next pair
%endrep
%ifnempty EVENENTRY
	; dumpfatentry 0
	dw EVENENTRY
 %if 0

Edge case result with dumpfatentry 0 here:

bootimg$ nasm bootimg.asm \
 -D_PAYLOADFILE=::endfill,"(_SPI - 1 - 1 - 1) * 512",0,zeros.bin \
 -I ../lmacros/ -D_FULL=0 -D_BPE=12 -D_SPI=342 -D_SPF="1" \
 -D_NUMFATS=1 -D_NUMROOT=16
bootimg.asm:1201: error: Negative count given to zerobytes
bootimg.asm:1205: error: Internal error in FAT filling

Correct behaviour with dw EVENENTRY:

bootimg$ nasm bootimg.asm \
 -D_PAYLOADFILE=::endfill,"(_SPI - 1 - 1 - 1) * 512",0,zeros.bin \
 -I ../lmacros/ -D_FULL=0 -D_BPE=12 -D_SPI=342 -D_SPF="1" \
 -D_NUMFATS=1 -D_NUMROOT=16

Correctly rejecting too small FAT:

bootimg$ time nasm bootimg.asm \
 -D_PAYLOADFILE=::endfill,"(_SPI - 1 - 1 - 1) * 512",0,zeros.bin \
 -I ../lmacros/ -D_FULL=0 -D_BPE=12 -D_SPI=343 -D_SPF="1" \
 -D_NUMFATS=1 -D_NUMROOT=16
bootimg.asm:272: error: Too few FAT sectors specified (341 entries, 342 needed)
bootimg.asm:1221: error: Negative count given to zerobytes
bootimg.asm:1225: error: Internal error in FAT filling

 %endif
%endif
	%endmacro

	usesection fat
%assign jj 0
%rep _NUMFATS
fat%[jj]:
	expandfatchains FATCHAINS
	zerobytes _SPF * _BPS - ($ - fat%[jj])
%assign jj jj+1
%endrep
%if FATSECTORS * _BPS - ($ - $$)
 %error Internal error in FAT filling
%endif
