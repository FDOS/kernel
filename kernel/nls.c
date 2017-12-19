/****************************************************************/
/*                                                              */
/*                            nls.c                             */
/*                           FreeDOS                            */
/*                                                              */
/*    National Languge Support functions and data structures    */
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

/*
 *	Note 1: Some code assume certains prerequisites to be matched,
 *	e.g. character tables exactly 128 bytes long; I try to keep\
 *	track of these conditions within comments marked with:
 *		==ska*/

#include "portab.h"
#include "globals.h"
#include "pcb.h"
#include <nls.h>

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id: nls.c 1491 2009-07-18 20:48:44Z bartoldeman $";
#endif

#ifdef NLS_DEBUG
#define log(a)	printf a
#define log1(a)	printf a
#else
#define log(a)
#ifdef NDEBUG
#define log1(a)
#else
#define log1(a)	printf a
#endif
#endif

struct nlsInfoBlock ASM nlsInfo = {
  (char FAR *)0                 /* filename to COUNTRY.SYS */
      , 437                     /* system code page */
      /* Implementation flags */
      , 0
#ifdef NLS_MODIFYABLE_DATA
      | NLS_CODE_MODIFYABLE_DATA
#endif
#ifdef NLS_REORDER_POINTERS
      | NLS_CODE_REORDER_POINTERS
#endif
#ifdef __GNUC__
      , {.seg=DosDataSeg, .off=&nlsPackageHardcoded} /* hardcoded first package */
      , {.seg=DosDataSeg, .off=&nlsPackageHardcoded} /* first item in chain */
#else
      , &nlsPackageHardcoded    /* hardcoded first package */
      , &nlsPackageHardcoded    /* first item in chain */
#endif
};

        /* getTableX return the pointer to the X'th table; X==subfct */
        /* subfct 2: normal upcase table; 4: filename upcase table */
#ifdef NLS_REORDER_POINTERS
#define getTable2(nls)	((nls)->nlsPointers[0].pointer)
#define getTable4(nls)	((nls)->nlsPointers[1].pointer)
#define getTable7(nls)	((nls)->nlsPointers[4].pointer)
#else
#define getTable2(nls)	getTable(2, (nls))
#define getTable4(nls)	getTable(4, (nls))
#define getTable7(nls)	getTable(7, (nls))
#define NEED_GET_TABLE
#endif
        /*== both chartables must be 128 bytes long and lower range is
		identical to 7bit-US-ASCII ==ska*/
#define getCharTbl2(nls)			\
	 (((struct nlsCharTbl FAR*)getTable2(nls))->tbl - 0x80)
#define getCharTbl4(nls)			\
	 (((struct nlsCharTbl FAR*)getTable4(nls))->tbl - 0x80)

/********************************************************************
 ***** MUX calling functions ****************************************
 ********************************************************************/

#ifdef __GNUC__
#define call_nls(a,b,c,d,e,f) call_nls(f,e,d,c,b,a)
#endif
extern long ASMPASCAL call_nls(UWORD, VOID FAR *, UWORD, UWORD, UWORD, UWORD);
/*== DS:SI _always_ points to global NLS info structure <-> no
 * subfct can use these registers for anything different. ==ska*/
STATIC long muxGo(int subfct, UWORD bp, UWORD cp, UWORD cntry, UWORD bufsize,
		  void FAR *buf)
{
  long ret;
  log(("NLS: muxGo(): subfct=%x, cntry=%u, cp=%u, ES:DI=%p\n",
       subfct, cntry, cp, buf));
  ret = call_nls(bp, buf, subfct, cp, cntry, bufsize);
  log(("NLS: muxGo(): return value = %lx\n", ret));
  return ret;
}

/*
 *	Call NLSFUNC to load the NLS package
 */
STATIC COUNT muxLoadPkg(int subfct, UWORD cp, UWORD cntry)
{
  long ret;

  /*          0x1400 == not installed, ok to install              */
  /*          0x1401 == not installed, not ok to install          */
  /*          0x14FF == installed                                 */

#if NLS_FREEDOS_NLSFUNC_VERSION == NLS_FREEDOS_NLSFUNC_ID
  /* make sure the NLSFUNC ID is updated */
#error "NLS_FREEDOS_NLSFUNC_VERSION == NLS_FREEDOS_NLSFUNC_ID"
#endif
  /* Install check must pass the FreeDOS NLSFUNC version as codepage (cp) and
     the FreeDOS NLSFUNC ID as buffer size (bufsize).  If they match the
     version in NLSFUNC, on return it will set BX (cp on entry) to FreeDOS
     NLSFUNC ID.  call_nls will set the high word = BX on return.
  */
  ret = muxGo(0, 0, NLS_FREEDOS_NLSFUNC_VERSION, 0, NLS_FREEDOS_NLSFUNC_ID, 0);
  if ((int)ret != 0x14ff)
    return DE_FILENOTFND;       /* No NLSFUNC --> no load */
  if ((int)(ret >> 16) != NLS_FREEDOS_NLSFUNC_ID) /* FreeDOS NLSFUNC will return */
    return DE_INVLDACC;         /* This magic number */

  /* OK, the correct NLSFUNC is available --> load pkg */
  /* If cp == -1 on entry, NLSFUNC updates cp to the codepage loaded
     into memory. The system must then change to this one later */
  return (int)muxGo(subfct, 0, cp, cntry, 0, 0);
}

STATIC int muxBufGo(int subfct, int bp, UWORD cp, UWORD cntry,
                    UWORD bufsize, VOID FAR * buf)
{
  log(("NLS: muxBufGo(): subfct=%x, BP=%u, cp=%u, cntry=%u, len=%u, buf=%p\n",
       subfct, bp, cp, cntry, bufsize, buf));

  return (int)muxGo(subfct, bp, cp, cntry, bufsize, buf);
}

#define mux65(s,cp,cc,bs,b)	muxBufGo(2, (s), (cp), (cc), (bs), (b))
#define mux38(cp,cc,bs,b)	muxBufGo(4, 0, (cp), (cc), (bs), (b))
#define muxYesNo(ch)		muxBufGo(NLSFUNC_YESNO,0, NLS_DEFAULT, NLS_DEFAULT, (ch), 0)
#define muxUpMem(s,b,bs)	muxBufGo((s),0, NLS_DEFAULT,NLS_DEFAULT, (bs), (b))

/********************************************************************
 ***** Helper functions**********************************************
 ********************************************************************/

/*
 *	Search for the NLS package within the chain
 *	Also resolves the default values (-1) into the currently
 *	active codepage/country code.
 */
STATIC struct nlsPackage FAR *searchPackage(UWORD cp, UWORD cntry)
{
  struct nlsPackage FAR *nls;

  if (cp == NLS_DEFAULT)
    cp = nlsInfo.actPkg->cp;
  if (cntry == NLS_DEFAULT)
    cntry = nlsInfo.actPkg->cntry;

  nls = nlsInfo.chain;
  while ((nls->cp != cp || nls->cntry != cntry)
         && (nls = nls->nxt) != NULL) ;

  return nls;
}

/* For various robustnesses reasons and to simplify the implementation
	at other places, locateSubfct() returns NULL (== "not found"),
	if nls == NULL on entry. */
STATIC VOID FAR *locateSubfct(struct nlsPackage FAR * nls, int subfct)
{
  int cnt;
  struct nlsPointer FAR *p;

  if (nls)
    for (cnt = nls->numSubfct, p = &nls->nlsPointers[0]; cnt--; ++p)
      if (p->subfct == (UBYTE) subfct)
        return p;

  return NULL;
}

#ifdef NEED_GET_TABLE
/*	search the table (from a subfct) from the active package */
/* Note: Because this table returns the pointers for stuff of
	*internal* purpose, it seems to be more comfortable that this
	function is guaranteed to return valid pointers, rather than
	to let the user (some kernel function) deal with non-existing
	tables -- 2000/02/26 ska*/
STATIC VOID FAR *getTable(UBYTE subfct, struct nlsPackage FAR * nls)
{
  struct nlsPointer FAR *poi;

  if ((poi = locateSubfct(nls, subfct)) != NULL)
    return poi;

  /* Failed --> return the hardcoded table */
  switch (subfct)
  {
    case 2:
      return &nlsUpcaseHardcoded;
    case 4:
      return &nlsFUpcaseHardcoded;
      /* case 5:                                                                      return &nlsFnameTermHardcodedTable; */
      /* case 6: return &nlsCollHardcodedTable; */
    case 7:
      return &nlsDBCSHardcoded;
  }
  return NULL;
}
#endif

/*
 *	Copy a buffer and test the size of the buffer
 *	Returns SUCCESS on success; DE_INVLDFUNC on failure
 *
 *	Efficiency note: This function is used as:
 *		return cpyBuf(buf, bufsize, ...)
 *	three times. If the code optimizer is some good, it can re-use
 *	the code to push bufsize, buf, call cpyBuf() and return its result.
 *	The parameter were ordered to allow this code optimization.
 */
STATIC COUNT cpyBuf(VOID FAR * dst, UWORD dstlen, VOID FAR * src,
                    UWORD srclen)
{
  if (srclen <= dstlen)
  {
    fmemcpy(dst, src, srclen);
    return SUCCESS;
  }
  return DE_INVLDFUNC;          /* buffer too small */
}

/*
 *	This function assumes that 'map' is adjusted such that
 *	map[0x80] is the uppercase of character 0x80.
 *== 128 byte chartables, lower range conform to 7bit-US-ASCII ==ska*/
STATIC VOID upMMem(UBYTE FAR * map, UBYTE FAR * str, unsigned len)
{
  REG unsigned c;

#ifdef NLS_DEBUG
  UBYTE FAR *oldStr;
  unsigned oldLen;

  oldStr = str;
  oldLen = len;
  log(("NLS: upMMem(): len=%u, %04x:%04x=\"", len, FP_SEG(str),
       FP_OFF(str)));
  for (c = 0; c < len; ++c)
    printf("%c", str[c] > 32 ? str[c] : '.');
  printf("\"\n");
#endif
  if (len)
    do
    {
      if ((c = *str) >= 'a' && c <= 'z')
        *str += 'A' - 'a';
      else if (c > 0x7f)
        *str = map[c];
      ++str;
    }
    while (--len);
#ifdef NLS_DEBUG
  printf("NLS: upMMem(): result=\"");
  for (c = 0; c < oldLen; ++c)
    printf("%c", oldStr[c] > 32 ? oldStr[c] : '.');
  printf("\"\n");
#endif
}

/********************************************************************
 ***** Lowlevel interface *******************************************
 ********************************************************************/

/* GetData function used by both the MUX-callback function and
	the direct-access interface.
	subfct == NLS_DOS_38 is a value > 0xff in order to not clash
	with subfunctions valid to be passed as DOS-65-XX. */
STATIC int nlsGetData(struct nlsPackage FAR * nls, int subfct,
                      UBYTE FAR * buf, unsigned bufsize)
{
  VOID FAR *poi;

  log(("NLS: nlsGetData(): subfct=%x, bufsize=%u, cp=%u, cntry=%u\n",
       subfct, bufsize, nls->cp, nls->cntry));

  /* Theoretically tables 1 and, if NLS_REORDER_POINTERS is enabled,
     2 and 4 could be hard-coded, because their
     data is located at predictable (calculatable) locations.
     However, 1 and subfct NLS_DOS_38 are to handle the same
     data and the "locateSubfct()" call has to be implemented anyway,
     in order to handle all subfunctions.
     Also, NLS is often NOT used in any case, so this code is more
     size than speed optimized. */
  if ((poi = locateSubfct(nls, subfct)) != NULL)
  {
    log(("NLS: nlsGetData(): subfunction found\n"));
    switch (subfct)
    {
      case 1:                  /* Extended Country Information */
        return cpyBuf(buf, bufsize, poi,
                      ((struct nlsExtCntryInfo FAR *)poi)->size + 3);
      case NLS_DOS_38:         /* Normal Country Information */
        return cpyBuf(buf, bufsize, &(((struct nlsExtCntryInfo FAR *)poi)->dateFmt), 24);       /* standard cinfo has no more 34 _used_ bytes */
        /* don't copy 34, copy only 0x18 instead, 
           see comment at DosGetCountryInformation                      TE */
      default:
        /* All other subfunctions just return the found nlsPoinerInf
           structure */
        return cpyBuf(buf, bufsize, poi, sizeof(struct nlsPointer));
    }
  }

  /* The requested subfunction could not been located within the
     NLS pkg --> error. Because the data corresponds to the subfunction
     number passed to the API, the failure is the same as that a wrong
     API function has been called. */
  log(("NLS: nlsGetData(): Subfunction not found\n"));
  return DE_INVLDFUNC;
}

VOID nlsCPchange(UWORD cp)
{
  UNREFERENCED_PARAMETER(cp);
  put_string("\7\nchange codepage not yet done ska\n");
}

/*
 *	Changes the current active codepage or cntry
 *
 *	Note: Usually any call sees a value of -1 (0xFFFF) as "the current
 *	country/CP". When a new NLS pkg is loaded, there is however a little
 *	difference, because one could mean that when switching to country XY
 *	the system may change to any codepage required.
 *	Example:
 *		MODE has prepared codepages 437 and 850.
 *		The user loaded a 2nd NLS pkg via CONFIG.SYS with:
 *			COUNTRY=49,850,C:\COUNTRY.SYS
 *		By default, the kernel maintains the hardcoded 001,437 (U.S.A./CP437)
 *		After the Country statement the system switches to codepage 850.
 *		But when the user invokes DOS-38-01/DX=FFFF (Set Country ID to 1)
 *		the system _must_ switch to codepage 437, because this is the only
 *		NLS pkg loaded.
 *	Therefore, setPackage() will substitute the current country ID, if
 *	cntry==-1, but leaves cp==-1 in order to let NLSFUNC choose the most
 *	appropriate codepage on its own.
 */

STATIC COUNT nlsSetPackage(struct nlsPackage FAR * nls)
{
  if (nls->cp != nlsInfo.actPkg->cp)    /* Codepage gets changed -->
                                           inform all character drivers thereabout.
                                           If this fails, it would be possible that the old
                                           NLS pkg had been removed from memory by NLSFUNC. */
    nlsCPchange(nls->cp);

  nlsInfo.actPkg = nls;

  return SUCCESS;
}
STATIC COUNT DosSetPackage(UWORD cp, UWORD cntry)
{
  /* Right now, we do not have codepage change support in kernel, so push
     it through the mux in any case. */
#if 0
  struct nlsPackage FAR *nls;   /* NLS package to use to return the info from */

  /* nls := NLS package of cntry/codepage */
  if ((nls = searchPackage(cp, cntry)) != NULL)
    /* OK the NLS pkg is loaded --> activate it */
    return nlsSetPackage(nls);

  /* not loaded --> invoke NLSFUNC to load it */
#endif
  return muxLoadPkg(NLSFUNC_LOAD_PKG2, cp, cntry);
}

STATIC COUNT nlsLoadPackage(struct nlsPackage FAR * nls)
{

  nlsInfo.actPkg = nls;

  return SUCCESS;
}
STATIC COUNT DosLoadPackage(UWORD cp, UWORD cntry)
{
  struct nlsPackage FAR *nls;   /* NLS package to use to return the info from */

  /* nls := NLS package of cntry/codepage */
  if ((nls = searchPackage(cp, cntry)) != NULL)
    /* OK the NLS pkg is loaded --> activate it */
    return nlsLoadPackage(nls);

  /* not loaded --> invoke NLSFUNC to load it */
  return muxLoadPkg(NLSFUNC_LOAD_PKG, cp, cntry);
}

STATIC void nlsUpMem(struct nlsPackage FAR * nls, VOID FAR * str, int len)
{
  log(("NLS: nlsUpMem()\n"));
  upMMem(getCharTbl2(nls), (UBYTE FAR *) str, len);
}
STATIC void nlsFUpMem(struct nlsPackage FAR * nls, VOID FAR * str, int len)
{
  log(("NLS: nlsFUpMem()\n"));
  upMMem(getCharTbl4(nls), (UBYTE FAR *) str, len);
}

STATIC VOID xUpMem(struct nlsPackage FAR * nls, VOID FAR * str,
                   unsigned len)
/* upcase a memory area */
{
  log(("NLS: xUpMem(): cp=%u, cntry=%u\n", nls->cp, nls->cntry));

  if (nls->flags & NLS_FLAG_DIRECT_UPCASE)
    nlsUpMem(nls, str, len);
  else
    muxBufGo(NLSFUNC_UPMEM, 0, nls->cp, nls->cntry, len, str);
}

STATIC BOOL nlsIsDBCS(UBYTE ch)
{

  if (ch < 128)
    return FALSE;              /* No leadbyte is smaller than that */

  {
    UWORD FAR *t= ((struct nlsDBCS FAR*)getTable7(nlsInfo.actPkg))->dbcsTbl;

    for (; *t != 0; ++t)
      if (ch >= (*t & 0xFF) && ch <= (*t >> 8))
        return TRUE;
  }

  return FALSE;
}

STATIC int nlsYesNo(struct nlsPackage FAR * nls, UWORD ch)
{
  /* Check if it is a dual byte character */
  if (!nlsIsDBCS(ch & 0xFF)) {
    ch &= 0xFF;
    log(("NLS: nlsYesNo(): in ch=%u (%c)\n", ch, ch > 32 ? (char)ch : ' '));
    xUpMem(nls, MK_FP(_SS, &ch), 1);          /* Upcase character */
    /* Cannot use DosUpChar(), because
       maybe: nls != current NLS pkg
       However: Upcase character within lowlevel
       function to allow a yesNo() function
       catched by external MUX-14 handler, which
       does NOT upcase character. */
    log(("NLS: nlsYesNo(): upcased ch=%u (%c)\n", ch, ch > 32 ? (char)ch : ' '));
  }
  else
    log(("NLS: nlsYesNo(): in ch=%u (DBCS)\n", ch));

  if (ch == nls->yeschar)
    return 1;
  if (ch == nls->nochar)
    return 0;
  return 2;
}

/********************************************************************
 ***** DOS API ******************************************************
 ********************************************************************/

BYTE DosYesNo(UWORD ch)
/* returns: 0: ch == "No", 1: ch == "Yes", 2: ch crap */
{
  if (nlsInfo.actPkg->flags & NLS_FLAG_DIRECT_YESNO)
    return nlsYesNo(nlsInfo.actPkg, ch);
  else
    return muxYesNo(ch);
}

#ifndef DosUpMem
VOID DosUpMem(VOID FAR * str, unsigned len)
{
  xUpMem(nlsInfo.actPkg, str, len);
}
#endif

/*
 * This function is also called by the backdoor entry specified by
 * the "upCaseFct" member of the Country Information structure. Therefore
 * the HiByte of the first argument must remain unchanged.
 *	See NLSSUPT.ASM -- 2000/03/30 ska
 */
unsigned char ASMCFUNC DosUpChar(unsigned char ch)
 /* upcase a single character */
{
  log(("NLS: DosUpChar(): in ch=%u (%c)\n", ch, ch > 32 ? ch : ' '));
  DosUpMem(MK_FP(_SS, &ch), 1);
  log(("NLS: DosUpChar(): upcased ch=%u (%c)\n", ch, ch > 32 ? ch : ' '));
  return ch;
}

VOID DosUpString(char FAR * str)
/* upcase a string */
{
  DosUpMem(str, fstrlen(str));
}

VOID DosUpFMem(VOID FAR * str, unsigned len)
/* upcase a memory area for file names */
{
#ifdef NLS_DEBUG
  unsigned c;
  log(("NLS: DosUpFMem(): len=%u, %04x:%04x=\"", len, FP_SEG(str),
       FP_OFF(str)));
  for (c = 0; c < len; ++c)
    printf("%c", ((char FAR *)str)[c] > 32 ? ((char FAR *)str)[c] : '.');
  printf("\"\n");
#endif
  if (nlsInfo.actPkg->flags & NLS_FLAG_DIRECT_FUPCASE)
    nlsFUpMem(nlsInfo.actPkg, str, len);
  else
    muxUpMem(NLSFUNC_FILE_UPMEM, str, len);
}

unsigned char DosUpFChar(unsigned char ch)
 /* upcase a single character for file names */
{
  DosUpFMem(MK_FP(_SS, & ch), 1);
  return ch;
}

VOID DosUpFString(char FAR * str)
/* upcase a string for file names */
{
  DosUpFMem(str, fstrlen(str));
}

/*
 *	Called for all subfunctions other than 0x20-0x23,& 0xA0-0xA2
 *	of DOS-65
 *
 *	If the requested NLS pkg specified via cntry and cp is _not_
 *	loaded, MUX-14 is invoked; otherwise the pkg's NLS_Fct_buf
 *	function is invoked.
 */
COUNT DosGetData(int subfct, UWORD cp, UWORD cntry, UWORD bufsize,
                 VOID FAR * buf)
{
  struct nlsPackage FAR *nls;   /* NLS package to use to return the info from */

  log(("NLS: GetData(): subfct=%x, cp=%u, cntry=%u, bufsize=%u\n",
       subfct, cp, cntry, bufsize));

  if (!buf || !bufsize)
    return DE_INVLDDATA;
  if (subfct == 0)              /* Currently not supported */
    return DE_INVLDFUNC;

  /* nls := NLS package of cntry/codepage */
  if ((nls = searchPackage(cp, cntry)) != NULL)
  {
    /* matching NLS package found */
    if (nls->flags & NLS_FLAG_DIRECT_GETDATA)
      /* Direct access to the data */
      return nlsGetData(nls, subfct, buf, bufsize);
    cp = nls->cp;
    cntry = nls->cntry;
  }

  /* If the NLS pkg is not loaded into memory or the direct-access
     flag is disabled, the request must be passed through MUX */
  return (subfct == NLS_DOS_38)
        ? mux38(cp, cntry, bufsize, buf)
        : mux65(subfct, cp, cntry, bufsize, buf);
}

/*
 *	Called for DOS-38 get info
 *
 *	Note: DOS-38 does not receive the size of the buffer; therefore
 *	it is assumed the buffer is large enough as described in RBIL,
 *	which is 34 bytes _hardcoded_.
 */
/* TE 05/04/01
 * NETX calls Int 21 AX=3800
 * and gives a buffer of (at most) 0x20 bytes
 * MSDOS 6.2 copies only 0x18 bytes
 * RBIL documents 0x18 bytes and calls 10 bytes 'reserved'
 * so we change the amount of copied bytes to 0x18
 */

#ifndef DosGetCountryInformation
COUNT DosGetCountryInformation(UWORD cntry, VOID FAR * buf)
{
  return DosGetData(NLS_DOS_38, NLS_DEFAULT, cntry, 0x18, buf);
}
#endif

/*
 *	Called for DOS-38 set country code
 */
#ifndef DosSetCountry
COUNT DosSetCountry(UWORD cntry)
{
  return DosLoadPackage(NLS_DEFAULT, cntry);
}
#endif

/*
 *	Called for DOS-66-01 get CP
 */
COUNT DosGetCodepage(UWORD * actCP, UWORD * sysCP)
{
  *sysCP = nlsInfo.sysCodePage;
  *actCP = nlsInfo.actPkg->cp;
  return SUCCESS;
}

/*
 *	Called for DOS-66-02 set CP
 *	Note: One cannot change the system CP. Why it is necessary
 *	to specify it, is lost to me. (2000/02/13 ska)
 */
COUNT DosSetCodepage(UWORD actCP, UWORD sysCP)
{
  if (sysCP == NLS_DEFAULT || sysCP == nlsInfo.sysCodePage)
    return DosSetPackage(actCP, NLS_DEFAULT);
  return DE_INVLDDATA;
}

VOID FAR *DosGetDBCS(void)
{
	return getTable7(nlsInfo.actPkg);
}

/********************************************************************
 ***** MUX-14 API ***************************************************
 ********************************************************************/

/* Registers:
	AH == 14
	AL == subfunction
	BX == codepage
	DX == country code
	DS:SI == internal global nlsInfo
	ES:DI == user block

	Return value: AL register to be returned
		if AL == 0, Carry must be cleared, otherwise set
*/
UWORD ASMCFUNC syscall_MUX14(DIRECT_IREGS)
{
  struct nlsPackage FAR *nls;   /* addressed NLS package */

  UNREFERENCED_PARAMETER(flags);
  UNREFERENCED_PARAMETER(cs);
  UNREFERENCED_PARAMETER(ip);
  UNREFERENCED_PARAMETER(ds);
  UNREFERENCED_PARAMETER(es);
  UNREFERENCED_PARAMETER(si);

  log(("NLS: MUX14(): subfct=%x, cp=%u, cntry=%u\n", AL, BX, DX));

  if ((nls = searchPackage(BX, DX)) == NULL)
    return DE_INVLDFUNC;        /* no such package */

  log(("NLS: MUX14(): NLS pkg found\n"));

  switch (AL)
  {
    case NLSFUNC_INSTALL_CHECK:
      BX = NLS_FREEDOS_NLSFUNC_ID;
      return SUCCESS;           /* kernel just simulates default functions */
    case NLSFUNC_DOS38:
      return nlsGetData(nls, NLS_DOS_38, MK_FP(ES, DI), 34);
    case NLSFUNC_GETDATA:
      return nlsGetData(nls, BP, MK_FP(ES, DI), CX);
    case NLSFUNC_DRDOS_GETDATA:
      /* Does not pass buffer length */
      return nlsGetData(nls, CL, MK_FP(ES, DI), 512);
    case NLSFUNC_LOAD_PKG:
      return nlsLoadPackage(nls);
    case NLSFUNC_LOAD_PKG2:
      return nlsSetPackage(nls);
    case NLSFUNC_YESNO:
      return nlsYesNo(nls, CX);
    case NLSFUNC_UPMEM:
      nlsUpMem(nls, MK_FP(ES, DI), CX);
      return SUCCESS;
    case NLSFUNC_FILE_UPMEM:
#ifdef NLS_DEBUG
      {
        unsigned j;
        BYTE FAR *p;
        log(("NLS: MUX14(FILE_UPMEM): len=%u, %04x:%04x=\"", CX, ES, DI));
        for (j = 0, p = MK_FP(ES, DI); j < CX; ++j)
          printf("%c", p[j] > 32 ? p[j] : '.');
        printf("\"\n");
      }
#endif
      nlsFUpMem(nls, MK_FP(ES, DI), CX);
      return SUCCESS;
  }
  log(("NLS: MUX14(): Invalid function %x\n", AL));
  return DE_INVLDFUNC;          /* no such function */
}

