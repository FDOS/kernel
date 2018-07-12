/* Linker script for DOS executables with separate data and text segments.
   Partly derived from dos-exe-small.ld in newlib-ia16 and elks-separate.ld. */

OUTPUT_FORMAT(binary)

DOS_PSP = 0x60;
MEMOFS = DOS_PSP * 16 - SIZEOF(.msdos_mz_hdr);

/* these GROUPs play the same role as GROUPS (segments) in OMF */
PGROUP = (MEMOFS + LOADADDR(.ptext)) / 16;
LGROUP = (MEMOFS + LOADADDR(.ltext)) / 16;
DGROUP = (MEMOFS + LOADADDR(.data)) / 16;
_DosDataSeg = DGROUP;
TGROUP = (MEMOFS + LOADADDR(.text)) / 16;
IGROUP = (MEMOFS + LOADADDR(.itext)) / 16;
I_GROUP = (MEMOFS + LOADADDR(.idata)) / 16;

INITSIZE = SIZEOF(.itext) + SIZEOF(.idata) + SIZEOF(.ibss);

SECTIONS
  {
    /* Fabricate a .exe header here.  Although libbfd does have an
       "i386msdos" back-end which produces an "MZ" exe header, it cannot do
       certain things (yet). */
    .msdos_mz_hdr : {
		/* Signature.  */
		SHORT (0x5a4d)
		/* Bytes in last 512-byte page.  */
		SHORT (LOADADDR (.ibss) % 512)
		/* Total number of 512-byte pages.  */
		SHORT (ALIGN(LOADADDR (.ibss), 512) / 512)
		/* Relocation entries.  */
		SHORT ((__msdos_mz_rel_end - __msdos_mz_rel_start) / 4)
		/* Header size in paragraphs.  */
		SHORT (SIZEOF(.msdos_mz_hdr) / 16)
		/* Minimum extra paragraphs.  */
		SHORT (ALIGN(SIZEOF (.ibss) + SIZEOF(.istack), 16) / 16)
		/* Maximum extra paragraphs.  */
		SHORT (0xffff)
		/* Initial %ss.  */
		SHORT (LOADADDR (.istack) / 16 )
		/* Initial %sp. */
		SHORT (LOADADDR (.istack) % 16 + SIZEOF(.istack))
		/* Padding for Checksum (unused) and initial cs:ip (0:0) */
		. = 0x18;
		/* Relocation table offset.  */
		SHORT (. + 4)
		/* Overlay number  */
		SHORT (0)
		/* Relocations  */
		HIDDEN (__msdos_mz_rel_start = .);
		*(.msdos_mz_reloc .msdos_mz_reloc.*)
		HIDDEN (__msdos_mz_rel_end = .);
		/* Padding */
                . = ALIGN (16);
    }

    /* Target PSP section.  */
    .ptext 0 : AT (SIZEOF(.msdos_mz_hdr)) {
		*(PSP)
    }

    /* Target low data+text sections.  */
    .ltext 0 : AT (LOADADDR(.ptext) + SIZEOF(.ptext)) {
		*(_IRQTEXT)
		*(_LOWTEXT)
		*(_IO_TEXT)
		*(_IO_FIXED_DATA)
    }

    /* Target data sections.  */
    .data 0 : AT (ALIGN(LOADADDR(.ltext) + SIZEOF(.ltext), 16)) {
		*(_FIXED_DATA)
		*(_BSS)
		*(EXCLUDE_FILE (config.obj iasmsupt.obj *init*.obj iprf.obj main.obj) .bss)
		*(_DATA)
		*(EXCLUDE_FILE (config.obj iasmsupt.obj *init*.obj iprf.obj main.obj) .data)
		*(_DATAEND)
		*(CONST)
		*(CONST2)
		*(EXCLUDE_FILE (config.obj iasmsupt.obj *init*.obj iprf.obj main.obj) .rodata)
		*(EXCLUDE_FILE (config.obj iasmsupt.obj *init*.obj iprf.obj main.obj) .rodata.*)
		*(DYN_DATA)
		ASSERT(. <= 0xfff8,
		    "Error: too large for a small-model .exe file.");
	}

    /* Target text sections.  */
    .text 0 : AT (ALIGN(LOADADDR(.data) + SIZEOF(.data), 16)) {
		*(HMA_TEXT_START)
		*(HMA_TEXT)
		_res_DosExec = RES_DOSEXEC;
		_res_read = RES_READ;
		_strlen = STRLEN;
		_fstrlen = FSTRLEN;
		_strlen = STRLEN;
		_fstrcmp = FSTRCMP;
		_strchr = STRCHR;
		_fstrchr = FSTRCHR;
		_fmemchr = FMEMCHR;
		_strcpy = STRCPY;
		_fmemcpy = FMEMCPY;
		_fstrcpy = FSTRCPY;
		_memcpy = MEMCPY;
		_fmemset = FMEMSET;
		_memset = MEMSET;
		_memcmp = MEMCMP;
		_fmemcmp = FMEMCMP;
		_network_redirector_mx = NETWORK_REDIRECTOR_MX;
		_execrh = EXECRH;
		_share_check = SHARE_CHECK;
		_share_open_check = SHARE_OPEN_CHECK;
		_share_close_file = SHARE_CLOSE_FILE;
		_share_access_check = SHARE_ACCESS_CHECK;
		_share_lock_unlock = SHARE_LOCK_UNLOCK;
		_call_nls = CALL_NLS;
		_fl_reset = FL_RESET;
		_fl_diskchanged = FL_DISKCHANGED;
		_fl_format = FL_FORMAT;
		_fl_read = FL_READ;
		_fl_write = FL_WRITE;
		_fl_verify = FL_VERIFY;
		_fl_setdisktype = FL_SETDISKTYPE;
		_fl_setmediatype = FL_SETMEDIATYPE;
		_fl_readkey = FL_READKEY;
		_fl_lba_ReadWrite = FL_LBA_READWRITE;
		_floppy_change = FLOPPY_CHANGE;
		_ReadPCClock = READPCCLOCK;
		_WritePCClock = WRITEPCCLOCK;
		_WriteATClock = WRITEATCLOCK;
		*(EXCLUDE_FILE (config.obj iasmsupt.obj *init*.obj iprf.obj main.obj) .text)
		*(HMA_TEXT_END)
		ASSERT(. <= 0x10000,
		    "Error: too large for a small-model .exe file.");
	}

    /* Target init text sections. */
    .itext 0 : AT (LOADADDR(.text) + SIZEOF(.text)) {
		*(INIT_TEXT_START)
		*(INIT_TEXT)
                _init_execrh = INIT_EXECRH;
                _init_memset = INIT_MEMSET;
                _init_fmemset = INIT_FMEMSET;
                _init_memcmp = INIT_MEMCMP;
                _init_fmemcmp = INIT_FMEMCMP;
                _init_memcpy = INIT_MEMCPY;
                _init_fmemcpy = INIT_FMEMCPY;
                _init_strcpy = INIT_STRCPY;
                _init_strlen = INIT_STRLEN;
                _init_strchr = INIT_STRCHR;
                _UMB_get_largest = UMB_GET_LARGEST;
		_init_call_intr = INIT_CALL_INTR;
		_read = READ;
		_init_DosOpen = INIT_DOSOPEN;
		_close = CLOSE;
		_dup2 = DUP2;
		_lseek = LSEEK;
		_allocmem = ALLOCMEM;
		_init_PSPSet = INIT_PSPSET;
		_init_DosExec = INIT_DOSEXEC;
		_init_setdrive = INIT_SETDRIVE;
		_init_switchar = INIT_SWITCHAR;
		_keycheck = KEYCHECK;
		_set_DTA = SET_DTA;
		_DetectXMSDriver = DETECTXMSDRIVER;
		_init_call_XMScall = INIT_CALL_XMSCALL;
		__EnableA20 = _ENABLEA20;
		__DisableA20 = _DISABLEA20;
		*(.text)
		*(INIT_TEXT_END)
		ASSERT(. <= 0x10000,
		    "Error: too large for a small-model .exe file.");
	}

    /* Target init data sections.  */
    .idata 0 : AT (LOADADDR(.itext) + SIZEOF(.itext)) {
		*(ID_B)
		*(.data)
		*(.rodata) *(.rodata.*)
		*(ID_E)
	}
    .ibss (NOLOAD) : AT (LOADADDR(.idata) + SIZEOF(.idata)) {
		*(IB_B)
		*(.bss)
		*(IB_E)
	}
    .istack 0 (NOLOAD) : AT (LOADADDR(.ibss) + SIZEOF(.ibss)) {
		*(_STACK)
		. = 0x1000;
	}
    /DISCARD/ : { *(.*) }
  }
