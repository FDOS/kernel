/****************************************************************/
/*                                                              */
/*                            NLS.H                             */
/*                           FreeDOS                            */
/*                                                              */
/*    National Language Support data structures                 */
/*                                                              */
/*                   Copyright (c) 1995, 1996, 2000             */
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

#if defined(_MSC_VER)
#define asm __asm
#pragma pack(1)
#elif defined(_QC) || defined(__WATCOM__)
#pragma pack(1)
#elif defined(__ZTC__)
#pragma ZTC align 1
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a-
#endif

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
 *	I guess that most NLS packages has been tweaked to be compatible
 *	so that this is not a real limitation, but for all other packages
 *	the external NLSFUNC can supply every piece of code necessary.
 *	To allow this the interface between the kernel and NLSFUNC has been
 *	extended; at same time the interface has been reduced, because some
 *	of the API functions do not seem to offer any functionality required
 *	for now. This, however, may be a misinterpretation because of
 *	lack of understanding.
 *	
 *	The supported structure consists of the following assumptions:
 *	1) The pkg must contain the tables 2 (Upcase character), 4
 *		(Upcase filename character) and 5 (filename termination
 *		characters); because they are internally used.
 *	2) The tables 2 and 4 must contain exactly 128 (0x80) characters.
 *		The character at index 0 corresponses to character 128 (0x80).
 *		The characters in the range of 0..0x7f are constructed out of
 *		the 7-bit US-ASCII (+ control characters) character set and are
 *		upcased not through the table, but by the expression:
 *			(ch >= 'a' && ch <= 'z')? ch - 'a' + 'A': ch
 *			with: 'a' == 97; 'z' == 122; 'A' == 65
 *
 *	It seems that pure DOS can internally maintain two NLS pkgs:
 *	NLS#1: The hardcoded pkg of U.S.A. on CP437, and
 *	NLS#2: the pkg loaded via COUNTRY= from within CONFIG.SYS.
 *	I do interprete this behaviour as follows:
 *	CONFIG.SYS is read in more passes; before COUTRY= can be evaluated,
 *	many actions must be performed, e.g. to load kernel at all, open
 *	CONFIG.SYS and begin reading. The kernel requires at least one
 *	NLS information _before_ COUNTRY= has been evaluated - the upcase
 *	table. To not implement the same function multiple times, e.g.
 *	to upcase with and without table, the kernel uses the default
 *	NLS pkg until a more appropriate can be loaded and hopes that
 *	the BIOS (and the user) can live with its outcome.
 *	Though, theoretically, the hardcoded NLS pkg could be purged
 *	or overwritten once the COUNTRY= statement has been evaluated.
 *	It would be possible that this NLS pkg internally performs different
 *	purposes, for now this behaviour will be kept.
 *
 *	The current implementation extendeds the above "two maintained
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
 *	
 *	The kernel can be instructed to pass any subfunction of DOS-65 to
 *	MUX-14-02, including the character upcase subfunctions 0x20-0x22 and
 *	0xA0-0xA2 as well as 0x23 (yes/no response). That way upcase table can
 *	be supported (by reducing performance) that do not contain exactly 128
 *	characters or where the lower portion is not constructed from the 7-bit
 *	US-ASCII character set.
 *	To do so, each NLS pkg contains some flags specifying if to pass a
 *	set of subfunctions to MUX-14-02, the sets include:
 *		set#1: filename character upcase 0xA0-0xA2
 *		set#2: character upcase 0x20-0x22
 *		set#3: yes/no response 0x23
 *		set#4: Extended Country Information (Includes DOS-38)
 *		set#5: Anything else (usually picks a pointer from an array)
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
 *	The tables 2 and 4 (upcase tables) are relatively accessed often,
 *	but theoretically these tables could be loacted at any position
 *	of the pointer array. If the macro NLS_REORDER_POINTERS is enabled,
 *	both NLSFUNC and the internal loader will reorder the pointers
 *	array so that mandatory tables are located at predictable indexes.
 *	This removes that the kernel must search for the table when
 *	one of the DOS-65-[2A]x function is called or a filename has been
 *	passed in (which must be uppercased to be suitable for internal
 *	purpose). However, when some program try to tweak the internal
 *	tables this assumption could be wrong.
 *	NLS_REORDER_POINTERS is *enabled* by default.
 *
 *	A second performance boost can be achieved, if the kernel shall
 *	support *only* NLS pkgs that apply to the structure mentioned above,
 *	thus, contain only characters 0x80-0xFF and the range 0x00-0x7F
 *	is upcased as 7-bit US-ASCII. In this case when upcasing the
 *	NLS pkg is bypassed at all, but cached pointers are used, which
 *	point directly to the upcased characters. Because I don't know
 *	existing NLS pkgs, this feature may be not very trustworthy; also
 *	when the NLS pkg is switched bypassing DOS, the cached pointers
 *	won't be updated, also by enabling this macro the MUX-flags are
 *	ignored for the sub-functions DOS-65-[2A][0-2], therefore:
 *	NLS_CACHE_POINTERS is *disabled* by default.
 */

/* Define if some user program possibly modifies the value of the internal
	tables or the DOS-65-00 (Set Country Information) API function
	is to be supported. */
/* Currently unimplemented! -- 2000/02/13 ska*/
/* #define NLS_MODIFYABLE_DATA */

/* Define if the pointer array shall be reordered to allow a quick
	access to often used and mandatoryly present tables. */
#define NLS_REORDER_POINTERS

/* Define if the kernel is to cache the time-consuming search results.
	Doing so could lead to imporper functionality, if the active
	codepage or country ID is changed bypassing the DOS API. */
/* #define NLS_CACHE_POINTERS */


/*
 *	How the kernel and NLSFUNC communicate with each other
 */
 	/* Must be returned by NLSFUNC upon MUX-14-00 */
#define NLS_FREEDOS_NLSFUNC_ID	0x534b
	/* MUX-14 subfunction called by the kernel to load a specific
		NLS package */
#define NLS_NLSFUNC_LOAD_PKG	0x4b
	/* MUX-14 subfunction called when to externally upcase */
#define NLS_NLSFUNC_UP			0x61
	/* MUX-14 subfunction called when to externally upcase filenames */
#define NLS_NLSFUNC_FUP			0x69
	/* Internally used to represent DOS-38 */
#define NLS_DOS_38				0x7365
	/* MUX-14 subfunction called when to check yes/nochar */
#define NLS_NLSFUNC_YESNO		0x72

	/* Flags for the communication with NLSFUNC */
#define NLS_FLAG_INFO		0x001
#define NLS_FLAG_POINTERS	0x002
#define NLS_FLAG_YESNO		0x004
#define NLS_FLAG_UP			0x008
#define NLS_FLAG_FUP		0x010

/* To ease the maintainance this header file is included to
	a) define the "normal" structures, where all the non-fixed size
		arrays are noted with length "1", and
	b) define the hardcoded NLS package for U.S.A. -- CP437
	If the macro NLS_HARDCODED is defined, the structures are modifed
	to result into structures with the correct length.

	When NLS_NO_VARS is defined, no prototypes of the global
	variables are included, useful in sources defining the hardcoded
	information, but require the normal types, too.
*/
#ifndef NLS_HARDCODED
	/* Use the default of length == 1 */
#define NLS_POINTERS 1
#define NLS_FNAMSEPS 1
#define NLS_DBCSENTR 1
#define __join(a,b) a
#define mkName(a) a

#else

#define __join(a,b) a##b
#define mkName(a)	__join(a,NLS_HARDCODED)

#endif

	/* No codepage / country code given */
#define NLS_DEFAULT ((UWORD)-1)

#ifndef NLS_HARDCODED
/*
 *	This is the data in the exact order returned by DOS-65-01
 */
struct nlsExtCtryInfo
{
	UBYTE subfct;				/* always 1 */
	WORD size;					/* size of this structure 
									without this WORD itself */
	WORD countryCode;           /* current country code */
	WORD codePage;              /* current code page (CP) */

	/*
	 *	This is the data in the exact order as to return on
	 *	DOS-38; it is also the most (important) part of DOS-65-01
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
  VOID(FAR * upCaseFct) (VOID);	/* far call to a function mapping the
  				character in register AL */
  char dataSep[2];              /* ASCIZ of separator in data records */
};

struct nlsPointerInf {	/* Information of DOS-65-0X is usually addressed
							by a pointer */
	UBYTE subfct;		/* number of the subfunction */
	VOID FAR *pointer;	/* the pointer to be returned when the subfunction
							of DOS-65 is called (Note: won't work for
							subfunctions 0, 1, 0x20, 0x21, 0x22, 0x23,
							0xA0, 0xA1,& 0xA2 */
};
#endif

struct mkName(nlsPackage) {	/* the contents of one chain item of the
							list of NLS packages */
	struct nlsPackage FAR *nxt;	/* next item in chain */
	unsigned muxCallingFlags;	/* combination of NLS_FLAGS-* */
	struct nlsExtCtryInfo cntryInfo; 
	char yeschar, nochar;	/* yes / no character DOS-65-23 */
	unsigned numSubfct;		/* number of supported sub-functions */
	struct nlsPointerInf nlsPointer[NLS_POINTERS];	/* grows dynamically */
};

struct mkName(nlsDBCS) {
	UWORD numEntries;
	UWORD dbcsTbl[NLS_DBCSENTR];
};

#ifndef NLS_HARDCODED
struct nlsCharTbl {
	/* table containing a list of characters */
	WORD numEntries;			/* number of entries of this table.
									If <= 0x80, the first element of
									the table corresponse to character 0x80 */
	unsigned char tbl[1];		/* grows dynamically */
};
struct nlsCharTbl128{
	WORD numEntries;
	unsigned char tbl[128];
};
struct nlsCharTbl256{
	WORD numEntries;
	unsigned char tbl[256];
};
#endif

/* in file names permittable characters for DOS-65-05 */
struct mkName(nlsFnamTerm) {
	WORD size;                /* size of this structure */
	BYTE dummy1;
	char firstCh,
	  lastCh;                   /* first, last permittable character */
	BYTE dummy2;
	char firstExcl,
	  lastExcl;                 /* first, last excluded character */
	BYTE dummy3;
	BYTE numSep;                /* number of file name separators */
	char separators[NLS_FNAMSEPS];		/* grows dynamically */
};

#ifndef NLS_NO_VARS
struct mkName(nlsInfoBlock) {		/* This block contains all information
					shared by the kernel and the external NLSFUNC program */
	char FAR *fname;	/* filename from COUNTRY= */
	UWORD sysCodePage;	/* system code page */
	struct nlsPackage FAR *actPkg;	/* current NLS package */
#ifdef NLS_CACHE_POINTERS
	unsigned char FAR *fnamUpTable;	/* upcase table for filenames */
	unsigned char FAR *upTable;		/* normal upcase table */
#endif
	struct mkName(nlsPackage) chain;	/* first item of info chain -- 
									hardcoded U.S.A. */
};

extern struct mkName(nlsInfoBlock) nlsInfo;
extern struct mkName(nlsFnamTerm) nlsFnameTermHardcodedTable;
extern struct mkName(nlsDBCS) nlsDBCSHardcodedTable;
extern struct __join(nlsCharTbl,128) nlsUpHardcodedTable;
extern struct __join(nlsCharTbl,128) nlsFnameUpHardcodedTable;
extern struct __join(nlsCharTbl,256) nlsCollHardcodedTable;
#endif

#undef NLS_POINTERS
#undef NLS_FNAMSEPS
#undef NLS_DBCSENTR
#undef __join(a,b)
#undef mkName(a)

/* standard alignment */

#if defined (_MSC_VER) || defined(_QC) || defined(__WATCOMC__)
#pragma pack()
#elif defined (__ZTC__)
#pragma ZTC align
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a.
#endif
