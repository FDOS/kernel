/****************************************************************/
/*                                                              */
/*                            nls.c                             */
/*                            DOS-C                             */
/*                                                              */
/*    National Languge Support functions and data structures    */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
/*                         Steffen Kaiser                       */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
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

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.1  2000/05/06 19:35:29  jhall1
 * Initial revision
 *
 * Revision 1.7  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.6  1999/09/23 04:40:48  jprice
 * *** empty log message ***
 *
 * Revision 1.4  1999/08/25 03:18:09  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.3  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.2  1999/04/16 00:53:33  jprice
 * Optimized FAT handling
 *
 * Revision 1.1.1.1  1999/03/29 15:41:24  jprice
 * New version without IPL.SYS
 *
 * Revision 1.6  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.5  1999/02/04 03:12:08  jprice
 * Removed extra text.  Made .exe smaller.
 *
 * Revision 1.4  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.3  1999/01/30 08:28:12  jprice
 * Clean up; Fixed bug with set attribute function.
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.4   04 Jan 1998 23:15:16   patv
 * Changed Log for strip utility
 *
 *    Rev 1.3   16 Jan 1997 12:46:54   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.2   29 May 1996 21:03:46   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.1   19 Feb 1996  4:34:46   patv
 * Corrected typo
 *
 *    Rev 1.0   19 Feb 1996  3:21:46   patv
 * Added NLS, int2f and config.sys processing
 */

extern UWORD internalUpcase(UWORD c);

#ifdef __TURBOC__
/* TC 2.01 require these. :( -- ror4 */
void __int__(int);
void __emit__();
#endif

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

struct ctryInfo
{                               /* Country Information DOS-38 */
  WORD dateFmt;                 /* 0: USA, 1: Europe, 2: Japan */
  char curr[5];                 /* ASCIZ of currency string */
  char thSep[2];                /* ASCIZ of thousand's separator */
  char point[2];                /* ASCIZ of decimal point */
  char dateSep[2];              /* ASCIZ of date separator */
  char timeSep[2];              /* ASCIZ of time separator */
  BYTE currFmt;                 /* format of currency:
                                   bit 0: currency string is placed 0: before, 1: behind number
                                   bit 1: currency string and number are separated by a space; 0: No, 1: Yes
                                 */
  BYTE prescision;              /* */
  BYTE timeFmt;                 /* time format: 0: 12 hours; 1: 24 houres */
    VOID(FAR * upCaseFct) (VOID);	/* far call to a function mapping character in register AL */
  char dataSep[2];              /* ASCIZ of separator in data records */
};

struct _VectorTable
{
  VOID FAR *Table;
  BYTE FnCode;
};

struct _NlsInfo
{
  struct extCtryInfo
  {
    BYTE reserved[8];
    BYTE countryFname[64];
    WORD sysCodePage;
    WORD nFnEntries;
    struct _VectorTable VectorTable[6];

    /* Extended Country Information DOS-65-01 */
    WORD countryCode;           /* current COUNTRY= code */
    WORD codePage;              /* current code page (CP) */

    struct ctryInfo nlsCtryInfo;
  }
  nlsExtCtryInfo;

/* characters of Yes/No prompt for DOS-65-23 */
  char yesCharacter;
  char noCharacter;

/* upcased characters for ECS-ASCII > 0x7f for DOS-65-02 */
  WORD upNCsize;                /* number of entries in the following array */
  char upNormCh[128];

/* upcased characters for ECS-ASCII > 0x7f for file names for DOS-65-04 */
  WORD upFCsize;                /* number of entries in the following array */
  char upFileCh[128];

/* collating sequence for ECS-ASCII 0..0xff for DOS-65-06 */
  WORD collSize;                /* number of entries in the following array */
  char collSeq[256];

/* DBC support for DOS-65-07 */
  WORD dbcSize;                 /* number of entries in the following array */
  /* char dbcTable[1024]; no DBC support */
  WORD dbcEndMarker;            /* contains always 0 */

/* in file names permittable characters for DOS-65-05 */
  struct chFileNames
  {
    WORD fnSize;                /* size of this structure */
    BYTE dummy1;
    char firstCh,
      lastCh;                   /* first, last permittable character */
    BYTE dummy2;
    char firstExcl,
      lastExcl;                 /* first, last excluded character */
    BYTE dummy3;
    BYTE numSep;                /* number of file name separators */
    char fnSeparators[14];
  }
  nlsFn;
}
nlsInfo
#ifdef INIT_NLS_049
=                               /* let's initialize it with values for Germany */
#include "049-437.nls"
#else
=                               /* let's initialize it with default values (USA) */
#include "001-437.nls"
#endif
 ;

#define normalCh nlsInfo.upNormCh
#define fileCh nlsInfo.upFileCh
#define yesChar nlsInfo.yesCharacter
#define noChar nlsInfo.noCharacter

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))

/*  COUNTRY.SYS structures */
struct CpiHeader
{
  BYTE name[8];                 /* signature */
  BYTE reserved[8];
  WORD nPointers;               /* size of following array */

  struct
  {
    BYTE pointerType;           /* always 1 */
    DWORD offset;               /* offset to data */
  }
  pointer[1];
};

struct CountryRecord
{
  WORD length;                  /* size of record */
  WORD country;                 /* country code */
  WORD codePage;                /* code page */
  WORD reserved[2];
  DWORD subCountryOffset;       /* offset to data record */
};

struct CountryTableDescr
{
  WORD length;                  /* size of structure */
  WORD id;                      /* table type id */
  DWORD offset;                 /* offset to table data */
};

/* standard alignment */

#if defined (_MSC_VER) || defined(_QC) || defined(__WATCOMC__)
#pragma pack()
#elif defined (__ZTC__)
#pragma ZTC align
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a.
#endif

COUNT NlsFuncInst(VOID)
{
  BYTE cNlsRet;

#ifndef __TURBOC__
  asm
  {
    xor bx,
      bx
      mov ax,
      0x1400
    int 0x2F
      mov cNlsRet,
      al
  }
#else
  _BX = 0;
  _AX = 0x1400;
  __int__(0x2f);
  cNlsRet = _AL;
#endif

  /* Return the al register as sign extended:                     */
  /*               0 == not installed, ok to install              */
  /*               1 == not installed, not ok to install          */
  /*              -1 == installed                                 */
  return cNlsRet;
}

BOOL
GetGlblCodePage(UWORD FAR * ActvCodePage, UWORD FAR * SysCodePage)
{
  *ActvCodePage = nlsInfo.nlsExtCtryInfo.codePage;
  *SysCodePage = nlsInfo.nlsExtCtryInfo.sysCodePage;
  return TRUE;
}

BOOL
SetGlblCodePage(UWORD FAR * ActvCodePage, UWORD FAR * SysCodePage)
{
  nlsInfo.nlsExtCtryInfo.codePage = *ActvCodePage;
  nlsInfo.nlsExtCtryInfo.sysCodePage = *SysCodePage;
  return TRUE;
}

UWORD SetCtryInfo(UBYTE FAR * lpShrtCode, UWORD FAR * lpLongCode,
                  BYTE FAR * lpTable, UBYTE * nRetCode)
{
  UWORD CntryCode;
  UBYTE nNlsEntry;
  UWORD uSegTable,
    uOffTable;
  UBYTE nLclRet;

  /* Get the Country Code according to the DOS silly rules.       */
  if (0xff != *lpShrtCode)
    CntryCode = *lpShrtCode;
  else
    CntryCode = *lpLongCode;

  /* If it's the same country code as what's installed, just      */
  /* return because there's nothing to do.                        */
  if (CntryCode == nlsInfo.nlsExtCtryInfo.countryCode)
  {
    *nRetCode = 0;
    return CntryCode;
  }

  /* Check if nlsfunc is installed                                */
  if (NlsFuncInst() >= 0)
  {
    *nRetCode = 0xff;
    return 0xffff;
  }

  /* Get the country information from nlsfunc                     */
  uSegTable = FP_SEG(lpTable);
  uOffTable = FP_OFF(lpTable);

#ifndef __TURBOC__
  asm
  {
    push ds
      mov bx,
      CntryCode
      mov ax,
      uSegTable
      mov dx,
      uOffTable
      mov ds,
      ax
      mov ax,
      0x1404
    int 0x2F
      pop ds
      mov CntryCode,
      bx
      mov nLclRet,
      al
  }
#else
  /* XXX: this is ugly... but needed on `tcc' 2.01 without `tasm'. -- ror4 */
  __emit__(0x1e);               /* push ds */
  _BX = CntryCode;
  _AX = uSegTable;
  _DX = uOffTable;
  _DS = _AX;
  _AX = 0x1404;
  __int__(0x2f);
  __emit__(0x1f);               /* pop ds */
  CntryCode = _BX;
  nLclRet = _AL;
#endif
  *nRetCode = nLclRet;
  return CntryCode;
}

UWORD GetCtryInfo(UBYTE FAR * lpShrtCode, UWORD FAR * lpLongCode,
                  BYTE FAR * lpTable)
{
  fbcopy((BYTE FAR *) & nlsInfo.nlsExtCtryInfo.nlsCtryInfo,
         lpTable, sizeof(struct ctryInfo));
  return nlsInfo.nlsExtCtryInfo.countryCode;
}

BOOL ExtCtryInfo(UBYTE nOpCode, UWORD CodePageID, UWORD InfoSize, VOID FAR * Information)
{
  VOID FAR *lpSource;
  COUNT nIdx;

  if (0xffff != CodePageID)
  {
    UBYTE nNlsEntry;

    if (NlsFuncInst() >= 0)
      return FALSE;

#ifndef __TURBOC__
    asm
    {
      mov bp,
        word ptr nOpCode
        mov bx,
        CodePageID
        mov si,
        word ptr Information + 2
        mov ds,
        si
        mov si,
        word ptr Information
        mov ax,
        0x1402
      int 0x2F
        cmp al,
        0
        mov nNlsEntry,
        al
    }
#else
    /* XXX: again, this is ugly... -- ror4 */
    __emit__(0x1e, 0x55, 0x56); /* push ds; push bp; push si */
    _BX = CodePageID;
    _SI = ((WORD *) & Information)[1];
    _DS = _SI;
    _SI = *(WORD *) & Information;
    _BP = *(WORD *) & nOpCode;
    _BP &= 0x00ff;
    _AX = 0x1402;
    __int__(0x2f);
    nNlsEntry = _AL;
    __emit__(0x5e, 0x5d, 0x1f); /* pop si; pop bp; pop ds */
#endif

    if (0 != nNlsEntry)
      return FALSE;

    return TRUE;
  }

  CodePageID = nlsInfo.nlsExtCtryInfo.codePage;

  for (nIdx = 0; nIdx < nlsInfo.nlsExtCtryInfo.nFnEntries; nIdx++)
  {
    if (nlsInfo.nlsExtCtryInfo.VectorTable[nIdx].FnCode == nOpCode)
    {
      BYTE FAR *bp = Information;
      lpSource = nlsInfo.nlsExtCtryInfo.VectorTable[nIdx].Table;

      if (nOpCode == 1)
      {
        bp++;                   /* first byte unused */

        *bp = (BYTE) (sizeof(struct ctryInfo) + 4);
        bp += 2;

        fbcopy(lpSource, bp, InfoSize > 3 ? InfoSize - 3 : 0);
      }
      else
      {
        *bp++ = nOpCode;
        *((VOID FAR **) bp) = lpSource;
      }
      return TRUE;
    }
  }

  return FALSE;
}

UWORD internalUpcase(UWORD c)
{
  if (!(c & 0x80))
    return c;

  return (c & 0xff00) | (nlsInfo.upNormCh[c & 0x7f] & 0xff);
}

char upMChar(UPMAP map, char ch)
/* upcase character ch according to the map */
{
  return (ch >= 'a' && ch <= 'z') ? ch + 'A' - 'a' :
      ((unsigned)ch > 0x7f ? map[ch & 0x7f] : ch);
}

VOID upMMem(UPMAP map, char FAR * str, unsigned len)
{
  REG unsigned c;

  if (len)
    do
    {
      if ((c = *str) >= 'a' && c <= 'z')
        *str += 'A' - 'a';
      else if (c > 0x7f)
        *str = map[c & 0x7f];
      ++str;
    }
    while (--len);
}

BYTE yesNo(char ch)             /* returns: 0: ch == "No", 1: ch == "Yes", 2: ch crap */
{
  ch = upMChar(normalCh, ch);
  if (ch == noChar)
    return 0;
  if (ch == yesChar)
    return 1;
  return 2;
}

char upChar(char ch)            /* upcase a single character */
{
  return upMChar(normalCh, ch);
}

VOID upString(char FAR * str)   /* upcase a string */
{
  upMMem(normalCh, str, fstrlen(str));
}

VOID upMem(char FAR * str, unsigned len)	/* upcase a memory area */
{
  upMMem(normalCh, str, len);
}

char upFChar(char ch)           /* upcase a single character for file names */
{
  return upMChar(fileCh, ch);
}

VOID upFString(char FAR * str)  /* upcase a string for file names */
{
  upMMem(fileCh, str, fstrlen(str));
}

VOID upFMem(char FAR * str, unsigned len)	/* upcase a memory area for file names */
{
  upMMem(fileCh, str, len);
}

/*      ReadCountryTable():

 *      Loads a country information table.
 */

static BOOL ReadCountryTable(COUNT file, WORD id, ULONG offset)
{
  VOID *buf;                    /* where to load the information */
  UWORD maxSize;                /* max number of bytes to read   */
  UWORD length;                 /* length of table in file       */
  BOOL rc = TRUE;

  switch (id)
  {
    case 1:                    /* extended country information */
      buf = &nlsInfo.nlsExtCtryInfo.countryCode;
      maxSize = sizeof(struct ctryInfo) + sizeof(WORD) * 2;
      break;

    case 2:                    /* uppercase table              */
      buf = &normalCh[0];
      maxSize = sizeof normalCh;
      break;

    case 4:                    /* filename uppercase table     */
      buf = &fileCh[0];
      maxSize = sizeof fileCh;
      break;

    case 5:                    /* filename terminator table    */
      buf = &nlsInfo.nlsFn.dummy1;
      maxSize = sizeof(struct chFileNames) - sizeof(WORD);
      break;

    case 6:                    /* collating sequence table     */
      buf = &nlsInfo.collSeq[0];
      maxSize = sizeof nlsInfo.collSeq;
      break;

    default:                   /* unknown or unsupported table - ignore  */
      buf = 0;
      break;
  }

  if (buf)
  {
    dos_lseek(file, offset, 0);
    dos_read(file, &length, sizeof(length));

    if (length > maxSize)
      length = maxSize;

    if (dos_read(file, buf, length) != length)
      rc = FALSE;

    if (id == 1)
      nlsInfo.nlsExtCtryInfo.nlsCtryInfo.upCaseFct = CharMapSrvc;
  }

  return rc;
}

/*      LoadCountryInfo():

 *      Searches a file in the COUNTRY.SYS format for an entry
 *      matching the specified code page and country code, and loads
 *      the corresponding information into memory. If code page is 0,
 *      the default code page for the country will be used.
 *
 *      Returns TRUE if successful, FALSE if not.
 */

/* XXX: This function should be placed in `INIT_TEXT'. -- ror4 */
BOOL FAR LoadCountryInfo(char FAR * filename, WORD ctryCode, WORD codePage)
{
  struct CpiHeader hdr;
  struct CountryRecord ctry;
  struct CountryTableDescr ct;
  COUNT i,
    nCountries,
    nSubEntries;
  ULONG currpos;
  int rc = FALSE;
  COUNT file;

  if ((file = dos_open(filename, 0)) < 0)
    return rc;

  if (dos_read(file, &hdr, sizeof(hdr)) == sizeof(hdr))
  {
    /* check signature */
    if (!fstrncmp(hdr.name, "\377COUNTRY", 8))
    {
      dos_lseek(file, hdr.pointer[0].offset, 0);
      dos_read(file, &nCountries, sizeof(nCountries));

      /* search for matching country record */
      for (i = 0; i < nCountries; i++)
      {
        if (dos_read(file, &ctry, sizeof(ctry)) != sizeof(ctry))
          break;

        if (ctry.country == ctryCode && (!codePage || ctry.codePage == codePage))
        {
          /* found country - now load the tables */
          dos_lseek(file, ctry.subCountryOffset, 0);
          dos_read(file, &nSubEntries, sizeof(nSubEntries));
          currpos = ctry.subCountryOffset + sizeof(nSubEntries);

          for (i = 0; i < nSubEntries; i++)
          {
            dos_lseek(file, currpos, 0);
            if (dos_read(file, &ct, sizeof(ct)) != sizeof(ct))
              break;

            currpos += ct.length + sizeof(ct.length);
            ReadCountryTable(file, ct.id, ct.offset + 8);
          }

          if (i == nSubEntries)
            rc = TRUE;

          break;
        }
      }
    }
  }
  dos_close(file);
  return rc;
}
