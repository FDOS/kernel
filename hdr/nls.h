/****************************************************************/
/*                                                              */
/*                            NLS.H                             */
/*                           FreeDOS                            */
/*                                                              */
/*    National Language Support data structures                 */
/*                                                              */
/*                   Copyright (c) 2000                         */
/*                         Steffen Kaiser                       */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of FreeDOS.                                */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

/* one byte alignment */
#include <algnbyte.h>

/*
 *	Description of the organization of NLS information -- 2000/02/13 ska
 *
 *	Glossar:
 *	NLS package -- NLS information incl. any code required to access or
 *		correctly interprete this particular information
 *
 *	Abbreviation:
 *	(NLS) pkg -- NLS package
 *
 *	The code included into the kernel does "only" support NLS packages
 *	structurally compatible with the one of the U.S.A. / CP437.
 *	I guess that most NLS packages has been tweaked to be compatible,
 *	so that this is not a real limitation, but for all other packages
 *	the external NLSFUNC can supply every piece of code necessary.
 *	To allow this the interface between the kernel and NLSFUNC has been
 *	extended; at the same time the interface has been reduced, because some
 *	of the API functions do not seem to offer any functionality required
 *	for now. This, however, may be a misinterpretation because of
 *	lack of understanding.
 *
 *	The supported structure consists of the following assumptions:
 *	1) The pkg must contain the tables 2 (Upcase character), 4
 *		(Upcase filename character) and 5 (filename termination
 *		characters); because they are used internally.
 *	2) The tables 2 and 4 must contain exactly 128 (0x80) characters.
 *		The character at index 0 corresponses to character 128 (0x80).
 *		The characters in the range of 0..0x7f are constructed out of
 *		the 7-bit US-ASCII (+ control characters) character set and are
 *		upcased not through the table, but by the expression:
 *			(ch >= 'a' && ch <= 'z')? ch - 'a' + 'A': ch
 *			with: 'a' == 97; 'z' == 122; 'A' == 65
 *	3) The data to be returned by DOS-65 is enlisted in the
 *		nlsPointer[] array of the nlsPackage structure, including
 *		the DOS-65-01 data, which always must be last entry of the
 *		array.
 *	4) DOS-38 returns the 34 bytes beginning with the byte at offset
 *		4 behind the size field of DOS-65-01.
 *
 *	It seems that pure DOS can internally maintain two NLS pkgs:
 *	NLS#1: The hardcoded pkg of U.S.A. on CP437, and
 *	NLS#2: the pkg loaded via COUNTRY= from within CONFIG.SYS.
 *	I do interprete this behaviour as follows:
 *	CONFIG.SYS is read in more passes; before COUTRY= can be evaluated,
 *	many actions must be performed, e.g. to load kernel at all, open
 *	CONFIG.SYS and begin reading. The kernel requires at least two
 *	NLS information _before_ COUNTRY= has been evaluated - both upcase
 *	tables. To not implement the same function multiple times, e.g.
 *	to upcase with and without table, the kernel uses the default
 *	NLS pkg until a more appropriate one can be loaded and hopes that
 *	the BIOS (and the user) can live with its outcome.
 *	Though, theoretically, the hardcoded NLS pkg could be purged
 *	or overwritten once the COUNTRY= statement has been evaluated.
 *	It would be possible that this NLS pkg internally performs different
 *	purposes, for now this behaviour will be kept.
 *
 *	The current implementation extends the above "two maintained
 *	NLS pkgs" into that the kernel chains all NLS pkgs loaded in
 *	memory into one single linked list. When the user does neither
 *	wants to load other NLS pkgs without executing NLSFUNC and the
 *	loaded NLS pkgs do not contain code themselves, no other code is
 *	required, but some memory to store the NLS pkgs into.
 *
 *	Furthermore, because the kernel needs to include the code for the
 *	hardcoded NLS pkg anyway, every NLS pkg can use it; so only
 *	NLS pkgs that structurally differ from U.S.A./CP437 actually need
 *	to add any code and residently install the MUX handler for NLSFUNC.
 *	This technique reduces the overhead calling the MUX handler, when
 *	it is not needed.
 *	However, NLSFUNC is always required if the user wants to return
 *	information about NLS pkgs _not_ loaded into memory.
 *
 *=== Attention: Because the nlsInfoBlock structure differs from the
 *===	the "traditional" (aka MS) implementation, the MUX-14 interface
 *===	is _not_ MS-compatible, although all the registers etc.
 *===	do conform. -- 2000/02/26 ska
 *
 *	Previous failed attempts to implement NLS handling and a full-
 *	featured MUX-14 supporting any-structured NLS pkgs suggest
 *	to keep the implement as simple as possible and keep the
 *	optimization direction off balance and to tend toward either
 *	an optimization for speed or size.
 *
 *	The most problem is that the MUX interrupt chain is considered
 *	highly overcrowded, so if the kernels invokes it itself, the
 *	performance might decrease dramatically; on the other side, the
 *	more complex the interface between kernel and a _probably_ installed
 *	external NLSFUNC becomes the more difficult all the stuff is becoming
 *	and, most importantly, the size grows unnecessarily, because many
 *	people don't use NLSFUNC at all.
 *
 *	The kernel uses the NLS pkg itself for two operations:
 *	1) DOS-65-2x and DOS-65-Ax: Upcase character, string, memory area, &
 *	2) whenever a filename is passed into the kernel, its components
 *		must be identified, invalid characters must be detected
 *		and, finally, all letters must be uppercased.
 *	I do not consider operation 1) an action critical for performance,
 *	because traditional DOS programming praxis says: Do it Yourself; so
 *	one can consider oneself lucky that a program aquires the upcase
 *	table once in its life time (I mean: lucky the program calls NLS at all).
 *	Operation 2), in opposite, might dramatically reduce performance, if
 *	it lacks proper implementations.
 *
 *	Straight forward implementation:
 *	The basic implementation of the NLS channels all requests of DOS-65,
 *	DOS-66, and DOS-38 through MUX-14. Hereby, any external program, such
 *	as NLSFUNC, may (or may not) install a piece of code to filter
 *	one, few, or all requests in order to perform them itself, by default
 *	all requests will end within the root of the MUX interrupt, which is
 *	located within the kernel itself. An access path could look like this:
 *	1. Call to DOS-65-XX, DOS-66-XX, or DOS-38.
 *	2. The kernel is enterred through the usual INT-21 API handler.
 *	3. The request is decoded and one of the NLS.C function is called.
 *	4. This function packs a new request and calls MUX-14.
 *	5. Every TSR/driver hooking INT-2F will check, if the request is
 *		directed for itself;
 *	5.1. If not, the request is passed on to the next item of the MUX
 *		interrupt chain;
 *	5.2. If so, the TSR, e.g. NLSFUNC, tests if the request is to be
 *		performed internally;
 *	5.2.1. If so, the request is performed and the MUX-14 call is
 *		terminated (goto step 8.)
 *	5.2.2. If not, the request is passed on (see step 5.1.)
 *	6. If all TSRs had their chance to filter requests, but none decided
 *		to perform the request itself, the kernel is (re-)enterred
 *		through its INT-2F (MUX) API handler.
 *	7. Here the request is decoded again and performed with the kernel-
 *		internal code; then the MUX-14 call is terminated.
 *	8. When the MUX-14 call returns, it has setup all return parameters
 *		already, so the INT-21 call is terminated as well.
 *
 *	Note: The traditional MUX-14 is NOT supported to offer functionality
 *	to the kernel at the first place, but to let the kernel access and
 *	return any values they must be loaded into memory, but the user may
 *	request information through the DOS-65 interface of NLS pkgs _not_
 *	already loaded. Theoretically, NLSFUNC needs not allocate any internal
 *	buffer to load the data into, because the user already supplied one;
 *	also if the kernel would instruct NLSFUNC to load the requested
 *	NLS pkg, more memory than necessary would be allocated. However, all
 *	except subfunction 1 return a _pointer_ to the data rather than the
 *	data itself; that means that NLSFUNC must cache the requested data
 *	somewhere, but how long?
 *
 *	Performance tweaks:
 *	When the system -- This word applies to the combination of kernel and
 *	any loaded MUX-14 extension á la NLSFUNC here. -- uppercases
 *	_filenames_, it must perform a DOS-65-A2 internally. In the basic
 *	implementation this request would be channeled through MUX-14, even
 *	if there is no external NLSFUNC at all. Also, when a NLS pkg had
 *	been loaded by the kernel itself, it complies to above mentioned
 *	rules and it is very unlikely that it is necessary to probe if
 *	a MUX-14 TSR might want to perform the request itself. Therefore
 *	each NLS pkg contains some flags that allow the kernel to bypass
 *	the MUX-14 request and invoke the proper function directly. Both
 *	default NLS pkgs will have those flags enabled, because they are
 *	already loaded into memory and must comply to the rules.
 *
 *	Note: Those flags do not alter the way the request is actually
 *	performed, but the MUX-14 call is omitted only (steps 4. through 6.).
 *
 *	======= Description of the API
 *
 *	There are three APIs to be supported by NLS:
 *	1) DOS API: DOS-38, DOS-65, DOS-66;
 *	2) MUX-14, and
 *	3) internal: upcasing filenames.
 *
 *	1) and 2) address the used NLS pkg by the country code / codepage pair.
 *	3) uses the currently active NLS pkg only; furthermore, these functions
 *	more or less match DOS-64-A*. Therefore, the NLS system merges the
 *	interfaces 1) and 3) and offers function suitable for both ones.
 *
 *	Both 1) and 3) must channel the request through the MUX chain, if
 *	appropriate, whereas 2) is the back-end and does natively process the
 *	request totally on its own.
 *
 *	The API of 1) and 3) consists of:
 *		+ DosUpChar(), DosUpString(), and DosUpMem(): to upcase an object
 *	(DOS-65-2[0-2]);
 *		+ DosYesNo(): to check a character, if it is the yes or no prompt
 *	(DOS-65-23);
 *		+ DosUpFChar(), DosUpFString(), and DosUpFMem(): to upcase an object
 *	for filenames (DOS-65-A[0-2]);
 *		+ DosGetData(): to retreive certain information (DOS-38, all the
 *	other DOS-65-** subfunctions);
 *		+ DosSetCountry(): to change the currently active country code
 *	(DOS-38);
 *		+ DosSetCodepage(): to change the currently active codepage (DOS-66).
 *
 *	The API of 2) consists of:
 *		+ syscall_MUX14().
 *	This function is invoked for all MUX-14 requests and recieves the
 *	registers of the particular INT-2F call, it will then decode the
 *	registers and pass the request forth to a NLS-internal interface
 *	consisting of the following "static" functions:
 *		+ nlsUpMem(): called for DosUp*(),
 *		+ nlsUpFMem(): called for DosUpF*(),
 *		+ nlsYesNo(): called for DosYesNo(),
 *		+ nlsGetData(): called for DosGetData(),&
 *		+ nlsSetPackage(): called for DosSetCountry() and DosSetCodepage().
 *	In opposite of the APIs 1) through 3) the NLS-internal functions address
 *	the NLS pkg to operate upon by a (struct nlsInfoBlock *) pointer.
 *
 *	This designs supports to easily implement to bypass the MUX chain to
 *	speed up especially the internal API to upcase filenames, because
 *	the Dos*() functions can decide do not pass the request through MUX,
 *	but directly call the nls*() function instead. This way it is ensured
 *	that the performed actions are the same in both cases and, with repect
 *	to the functions that operate with the currently active NLS pkg, the
 *	performance is rather high, because one can use the globally available
 *	pointer to the current NLS pkg and need not search for a country code/
 *	codepage pair.
 *
 *	======== Compile-time options
 *
 *	Win9x supports to change the individual portions of a NLS pkg
 *	through DOS-65-00; also there are no references what happens when
 *	a program changes the areas addressed by returned pointers. The
 *	current implementation does _not_ support changes of the NLS pkg
 *	except by invoking DOS-38 (Set Country Code) or DOS-66 (Set Codepage).
 *	Future implementations might offer this ability; to reduce the
 *	overhead introduced by this feature, the macro NLS_MODIFYABLE_DATA
 *	enables the appropriate code.
 *	NLS_MODIFYABLE_DATA is *disabled* by default.
 *
 *	The tables 2 and 4 (upcase tables) are accessed relatively often,
 *	but theoretically these tables could be located at any position
 *	of the pointer array. If the macro NLS_REORDER_POINTERS is enabled,
 *	both NLSFUNC and the internal loader will reorder the pointers
 *	array so that mandatory tables are located at predictable indexes.
 *	This removes that the kernel must search for the table when
 *	one of the DOS-65-[2A]x functions is called or a filename has been
 *	passed in (which must be uppercased to be suitable for internal
 *	purpose). However, when some program try to tweak the internal
 *	tables this assumption could be wrong.
 *	This setting has any effect only, if the kernel tries to access
 *	information itself; it is ignored when the user calls DOS-65-0x
 *	to return such pointer.
 *	NLS_REORDER_POINTERS is *enabled* by default.
 */

/* Define if some user program possibly modifies the value of the internal
	tables or the DOS-65-00 (Set Country Information) API function
	is to be supported. */
/* Currently unimplemented! -- 2000/02/13 ska*/
/* #define NLS_MODIFYABLE_DATA */

/* Define if the pointer array shall be reordered to allow a quick
	access to often used and mandatoryly present tables. */
#define NLS_REORDER_POINTERS

/*
 *	How the kernel and NLSFUNC communicate with each other
 */
        /* Must be pased to and returned by NLSFUNC upon MUX-14-00 */
#define NLS_FREEDOS_NLSFUNC_ID	0x534b
		/* What version of nlsInfo and accompanying associations
		   Must be passed to NLSFUNC upon MUX-14-00 to identify the
		   correct kernel to the tools. */
#define NLS_FREEDOS_NLSFUNC_VERSION 0xFD01
        /* Represents a call to DOS-38 within DOS-65 handlers.
           Current implementation relys on 0x101! */
#define NLS_DOS_38 0x101
        /* NLSFUNC may return NLS_REDO to instruct the kernel to
           try to perform the same action another time. This is most
           useful if the kernel only loads the NLS pkg into memory so
           the kernel will find it and will process the request internally
           now. */
#define NLS_REDO 353

/* Codes of the subfunctions of external NLSFUNC */
#define NLSFUNC_INSTALL_CHECK	0
#define NLSFUNC_DOS38			4
#define NLSFUNC_GETDATA			2
#define NLSFUNC_DRDOS_GETDATA	0xfe
#define NLSFUNC_LOAD_PKG		3
#define NLSFUNC_LOAD_PKG2		1
#define NLSFUNC_UPMEM			0x22
#define NLSFUNC_YESNO			0x23
#define NLSFUNC_FILE_UPMEM		0xa2

/* The NLS implementation flags encode what feature is in effect;
	a "1" in the bitfield means that the feature is active.
	All currently non-defined bits are to be zero to allow future
	useage. */
#define NLS_CODE_MODIFYABLE_DATA	0x0001
#define NLS_CODE_REORDER_POINTERS 	0x0002

/* NLS package useage flags encode what feature is in effect for this
	particular package:
	a "1" in the bitfield means that the feature is active/enabled.
	All currently non-defined bits are to be zero to allow future
	useage. */
#define NLS_FLAG_DIRECT_UPCASE		0x0001  /* DOS-65-2[012], */
#define NLS_FLAG_DIRECT_FUPCASE		0x0002  /* DOS-65-A[012], internal */
#define NLS_FLAG_DIRECT_YESNO		0x0004  /* DOS-65-23 */
#define	NLS_FLAG_DIRECT_GETDATA		0x0008  /* DOS-65-XX, DOS-38 */

#define NLS_FLAG_HARDCODED (NLS_FLAG_DIRECT_UPCASE		\
							| NLS_FLAG_DIRECT_FUPCASE	\
							| NLS_FLAG_DIRECT_YESNO		\
							| NLS_FLAG_DIRECT_GETDATA)

        /* No codepage / country code given */
#define NLS_DEFAULT ((UWORD)-1)

struct CountrySpecificInfo {
  short CountryID;    /*  = W1 W437   # Country ID & Codepage */
  short CodePage;
  short DateFormat;           /*    Date format: 0/1/2: U.S.A./Europe/Japan */
  char  CurrencyString[5];    /* '$' ,'EUR'   */
  char  ThousandSeparator[2]; /* ','          # Thousand's separator */
  char  DecimalPoint[2];      /* '.'        # Decimal point        */
  char  DateSeparator[2];     /* '-'  */
  char  TimeSeparator[2];     /* ':'  */
  char  CurrencyFormat;       /* = 0  # Currency format (bit array) 
                                 0Fh    BYTE    currency format
                                 bit 2 = set if currency symbol replaces decimal point
                                 bit 1 = number of spaces between value and currency symbol
                                 bit 0 = 0 if currency symbol precedes value
                                 1 if currency symbol follows value    
                              */
  char  CurrencyPrecision;    /* = 2  # Currency precision           */
  char  TimeFormat;           /* = 0  # time format: 0/1: 12/24 houres */
};

/*
 *	This is the data in the exact order returned by DOS-65-01
 */
struct nlsExtCntryInfo {
  UBYTE subfct;                 /* always 1 */
  WORD size;                    /* size of this structure
                                   without this WORD itself */
  WORD countryCode;             /* current country code */
  WORD codePage;                /* current code page (CP) */

  /*
   *      This is the data in the exact order as to return on
   *      DOS-38; it is also the most (important) part of DOS-65-01
   */
  /* Note: The ASCIZ strings might become
     a totally different understanding with
     DBCS (Double Byte Character Support) */
  WORD dateFmt;                 /* order of portions of date
                                   0: mm/dd/yyyy (USA)
                                   1: dd/mm/yyyy (Europe)
                                   2: yyyy/mm/dd (Japan)
                                 */
  char curr[5];                 /* ASCIZ of currency string */
  char thSep[2];                /* ASCIZ of thousand's separator */
  char point[2];                /* ASCIZ of decimal point */
  char dateSep[2];              /* ASCIZ of date separator */
  char timeSep[2];              /* ASCIZ of time separator */
  BYTE currFmt;                 /* format of currency:
                                   bit 0: currency string is placed
                                   0: before number
                                   1: behind number
                                   bit 1: currency string and number are
                                   separated by a space
                                   0: No
                                   1: Yes
                                   bit 2: currency string replaces decimal
                                   sign
                                   0: No
                                   1: Yes
                                 */
  BYTE prescision;              /* of monetary numbers */
  BYTE timeFmt;                 /* time format:
                                   0: 12 hours (append AM/PM)
                                   1: 24 houres
                                 */
    VOID(FAR * upCaseFct) (VOID);       /* far call to a function upcasing the
                                           character in register AL */
  char dataSep[2];              /* ASCIZ of separator in data records */
};

struct nlsPointer {             /* Information of DOS-65-0X is addressed
                                   by a pointer */
  UBYTE subfct;                 /* number of the subfunction */
  VOID FAR *pointer;            /* the pointer to be returned when the subfunction
                                   of DOS-65 is called (Note: won't work for
                                   subfunctions 0, 1, 0x20, 0x21, 0x22, 0x23,
                                   0xA0, 0xA1,& 0xA2 */
};

struct nlsPackage {             /* the contents of one chain item of the
                                   list of NLS packages */
  struct nlsPackage FAR *nxt;   /* next item in chain */
  UWORD cntry, cp;              /* country ID / codepage of this NLS pkg */
  int flags;                    /* direct access and other flags */
  /* Note: Depending on the flags above all remaining
     portions may be omitted, if the external NLSFUNC-like
     MUX-14 processor does not require them and performs
     all actions itself, so that the kernel never tries to
     fetch this information itself. */
  UBYTE yeschar;                /* yes / no character DOS-65-23 */
  UBYTE nochar;
  unsigned numSubfct;           /* number of supported sub-functions */
  struct nlsPointer nlsPointers[1];     /* grows dynamically */
};

struct nlsDBCS {                /* The internal structure is unknown to me */
  UWORD numEntries;
  UWORD dbcsTbl[1];
};

struct nlsCharTbl {
  /* table containing a list of characters */
  UWORD numEntries;             /* number of entries of this table.
                                   If <= 0x80, the first element of
                                   the table corresponse to character 0x80 */
  unsigned char tbl[1];         /* grows dynamically */
};
#define nlsChBuf(len)		struct nlsCharTbl##len {		\
			UWORD numEntries;							\
			unsigned char tbl[len];						\
		}
nlsChBuf(128);
nlsChBuf(256);

/* in file names permittable characters for DOS-65-05 */
struct nlsFnamTerm {
  WORD size;                    /* size of this structure */
  BYTE dummy1;
  char firstCh, lastCh;         /* first, last permittable character */
  BYTE dummy2;
  char firstExcl, lastExcl;     /* first, last excluded character */
  BYTE dummy3;
  BYTE numSep;                  /* number of file name separators */
  char separators[1];           /* grows dynamically */
};

struct nlsInfoBlock {           /* This block contains all information
                                   shared by the kernel and the external NLSFUNC program */
  char FAR *fname;              /* filename from COUNTRY=;
                                   maybe tweaked by NLSFUNC */
  UWORD sysCodePage;            /* system code page */
  unsigned flags;               /* implementation flags */
  struct nlsPackage FAR *actPkg;        /* current NLS package */
  struct nlsPackage FAR *chain; /* first item of info chain --
                                   hardcoded U.S.A./CP437 */
};

extern struct nlsInfoBlock nlsInfo;
extern struct nlsPackage ASM nlsPackageHardcoded;
        /* These are the "must have" tables within the hard coded NLS pkg */
extern struct nlsFnamTerm nlsFnameTermHardcoded;
extern struct nlsDBCS ASM nlsDBCSHardcoded;
extern struct nlsCharTbl nlsUpcaseHardcoded;
extern struct nlsCharTbl nlsFUpcaseHardcoded;
extern struct nlsCharTbl nlsCollHardcoded;
extern struct nlsExtCntryInfo nlsCntryInfoHardcoded;
extern BYTE FAR hcTablesStart[], hcTablesEnd[];

/***********************************************************************
 ***** Definitions & Declarations for COUNTRY.SYS **********************
 ***********************************************************************/

/* Note: These definitions are shared among all tools accessing the
	COUNTRY.SYS file as well -- 2000/06/11 ska*/

/* File structure:
	S0: Base (Primary) structure -- file header
	Offset	Size	Meaning
	0		array	ID string "FreeDOS COUNTRY.SYS v1.0\r\n"
	26		array	Copyright etc. (plain 7bit ASCII text)
	26+N	2byte	\x1a\0
	26+N+2	array	padded with \0 upto next offset
	128		word	number of country/codepage pairs	(N1)
	130		8byte	country code / codepage entries	(S1)
	130+8*N1	end of array
	===
	S1: structure of country/codepage pair
	Offset	Size	Meaning
	0		dword	relative position of table definition	(S2)
	4		word	codepage ID
	6		word	country code
	8		end of structure
	===
	S2: table definition of one country/codepage pair
	Offset	Size	Meaning
	0		word	number of function entries	(N2)
	2		8byte	function definition	(S3)
	2+8*N2	end of array
	===
	S3: function definition
	Offset	Size	Meaning
	0		dword	relative position of function data (see S4)
	4		word	number of bytes of data
	6		byte	function ID (same as passed to DOS-65-XX)
	7		byte	reserved for future use (currently 0 (zero))
	8		end of structure
	===
	S4: function data
	In opposite of the structures and arrays, the function data
	is just a structure-less stream of bytes, which is used as it is.
	Currently no validation check is performed over this data.
	That means, for instance, that a definition of function 2 (upcase
	table) has length 130 and the data consists of a word value with
	the length (128) and 128 bytes individual information.
	That way the DBCS is implemented exactly the same way as all the
	other tables; the only exception is pseudo-table 0x23.
	===
	"relative position" means this DWord specifies the amount of bytes
	between end of the current structure and the data the pointer is
	referring to. This shall enable future implementations to embed
	COUNTRY.SYS into other files.
*/

#define CSYS_FD_IDSTRING "FreeDOS COUNTRY.SYS v1.0\r\n\x1a"

#if 0
struct csys_function {       /* S3: function definition */
  UDWORD csys_rpos;             /* relative position to actual data */
  UWORD csys_length;
  UBYTE csys_fctID;             /* As passed to DOS-65-XX */
  UBYTE csys_reserved1;         /* always 0, reserved for future use */
};

struct csys_ccDefinition {   /* S1: country/codepage reference */
  UDWORD csys_rpos;             /* moving the 4byte value to the front
                                   can increase performance */
  UWORD csys_cp;
  UWORD csys_cntry;
};
#endif

struct csys_ccDefinition {   /* country/codepage reference */
  UDWORD csys_pos; /* moving the 4byte value to the front
                                   can increase performance */
  UWORD csys_cntry;
  UWORD csys_cp;
  UWORD csys_size1;			/* size of nlsPackage struct rpos is pointing to */

/* initially the object rpos is pointing to conforms to a
	struct nlsPackage, where:
	  struct nlsPackage FAR *nxt;   is missing
	  UWORD cntry, cp;              is missing
	  int flags;                    is NLS_FLAG_HARDCODED, if the
	  									kernel is to handle the data of its own
	  UBYTE yeschar;                is filled
	  UBYTE nochar;					is filled
	  unsigned numSubfct;           is filled
	  struct nlsPointer nlsPointers[1];   is filled
	  									the pointer member is the absolute
	  									position of the data within the file
	  									of this structure:
	  										UWORD count
	  										count bytes
	  										The "count" value is not a part
	  										of the data itself.
			Also: The data must be ordered corresponding to
			NLS_CODE_REORDER_POINTERS.
			Also: The last nlsPointer is subfct #1 _incl_ all its
			data [is the extended country information: struct nlsExtCntryInfo]
*/
};

struct csys_numEntries {     /* helper structure for "number of entries" */
  UWORD csys_entries;
};

/* Header of the COUNTRY.SYS file */
struct nlsCSys_fileHeader {     /* COUNTRY.SYS header */
  unsigned char csys_idstring[sizeof(CSYS_FD_IDSTRING)];
  UWORD csys_maxTotalSize;	/* maximal size of the total amount of
  								any individual definition, that includes
  								the nlsPackage skeleton and the sum of
  								all bytes required to load all the
  								subfunctions individually.
  								--> The code is to allocate maxTotalSize
  								and load any country definition of this
  								file into this buffer without any
  								overflow. */
	DWORD csys_posIndex;	/* absolute position of index table */
};

/* Structure created by CountryInfoLoad() */
struct nlsCSys_loadPackage {
	UWORD csys_size;
	struct nlsPackage csys_pkg;
};

/* standard alignment */
#include <algndflt.h>

#ifdef DEBUG
        /* Enable debugging of NLS part */

        /* Caution: Enabling NLS debugging usually generates
           _a_lot_ of noise. */
/*& #define NLS_DEBUG */

#endif
