/****************************************************************/
/*                                                              */
/*                            nls.c                             */
/*                           FreeDOS                            */
/*                                                              */
/*    National Languge Support functions and data structures    */
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

#include "portab.h"
#include "globals.h"
#include "intr.h"
#include "nls.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.8  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 */

#ifdef NLS_REORDER_POINTERS
#define getTable2	(&nlsInfo.actPkg->nlsPointer[0].pointer)
#define getTable4	(&nlsInfo.actPkg->nlsPointer[1].pointer)
#else
#define getTable2	getTable(2)
#define getTable4	getTable(4)
#define NEED_GET_TABLE
#endif

#ifdef NLS_CACHE_POINTERS
#define normalCh nlsInfo.upTable
#define fileCh nlsInfo.fnamUpTable
#else
#define normalCh getTable2
#define fileCh getTable4
#endif
#define yesChar nlsInfo.actPkg->yeschar
#define noChar nlsInfo.actPkg->nochar


#define NLS_MUX_COUNTRY_INFO(nls)	((nls)->muxCallingFlags & NLS_FLAG_INFO)
#define NLS_MUX_POINTERS(nls)		((nls)->muxCallingFlags & NLS_FLAG_POINTERS)
#define NLS_MUX_YESNO(nls)			((nls)->muxCallingFlags & NLS_FLAG_YESNO)
#define NLS_MUX_EXTERNAL_UP(nls)	((nls)->muxCallingFlags & NLS_FLAG_UP)
#define NLS_MUX_EXTERNAL_FUP(nls)	((nls)->muxCallingFlags & NLS_FLAG_FUP)



static COUNT muxGo(int subfct, struct REGPACK *rp)
{	rp->r_si = FP_OFF(&nlsInfo);
	rp->r_ds = FP_SEG(&nlsInfo);
	rp->r_ax = 0x1400 | subfct;
	intr(0x2f, rp);
	return rp->r_ax;
}

/*
 *	Call NLSFUNC to load the NLS package
 */
COUNT muxLoadPkg(UWORD cp, UWORD cntry)
{	struct REGPACK r;

	/* Return the al register as sign extended:                     */
	/*          0x1400 == not installed, ok to install              */
	/*          0x1401 == not installed, not ok to install          */
	/*          0x14FF == installed                                 */

	r.r_bx = 0;					/* make sure the NLSFUNC ID is updated */
	if(muxGo(0, &r) != 0x14ff)
		return DE_FILENOTFND;		/* No NLSFUNC --> no load */
	if(r.r_bx != NLS_FREEDOS_NLSFUNC_ID)
		return DE_INVLDACC;

	/* OK, the correct NLSFUNC is available --> load pkg */
	r.r_dx = cntry;
	r.r_bx = cp;
	return muxGo(NLS_NLSFUNC_LOAD_PKG, &r);
}

static int muxBufGo(int subfct, int bp, UWORD cp, UWORD cntry, UWORD bufsize
	, BYTE FAR *buf)
{	struct REGPACK r;

	r.r_bx = cntry;
	r.r_dx = cp;
	r.r_es = FP_SEG(*buf);
	r.r_di = FP_OFF(*buf);
	r.r_cx = bufsize;
	r.r_bp = bp;
	return muxGo(subfct, &r);
}

#define mux38(cp,cc,bs,b)	muxBufGo(4, 0, (cp), (cc), (bs), (b))
#define mux65(s,cp,cc,bs,b)	muxBufGo(2, (s), (cp), (cc), (bs), (b))
#define muxUpMem(s,l,f)		muxBufGo((f), 0, NLS_DEFAULT, NLS_DEFAULT, l, s)

static int muxYesNo(int ch)
{	struct REGPACK r;

	r.r_cx = ch;
	return muxGo(NLS_NLSFUNC_YESNO, &r);
}


/*
 *	Search the NLS package within the chain
 *	Also resolves the default values (-1) into the current
 *	active codepage/country code.
 */
struct nlsPackage FAR *searchPackage(UWORD *cp, UWORD *cntry)
{	struct nlsPackage FAR *nls;

	if(*cp == NLS_DEFAULT)
		*cp = nlsInfo.actPkg->cntryInfo.codePage;
	if(*cntry == NLS_DEFAULT)
		*cntry = nlsInfo.actPkg->cntryInfo.countryCode;

	nls = &nlsInfo.chain;
	while((nls->cntryInfo.codePage != *cp
 	  || nls->cntryInfo.countryCode != *cntry)
	 && (nls = nls->nxt) != NULL);

	return nls;
}

struct nlsPointerInf FAR *locateSubfct(struct nlsPackage FAR *nls
	, UBYTE subfct)
{	int cnt;
	struct nlsPointerInf FAR *p;

	for(cnt = nls->numSubfct, p = &nls->nlsPointer[0]
	 ; cnt--; ++p)
		if(p->subfct == subfct)
			return p;

	return NULL;
}

#ifdef NEED_GET_TABLE
/*	search the table (from a subfct) from the active package */
struct nlsPointerInf FAR *getTable(UBYTE subfct)
{	struct nlsPointerInf FAR *poi;

	if((poi = locateSubfct(nlsInfo.actPkg, subfct)) != NULL)
		return poi;

	/* Failed --> return the hardcoded table */
	switch(subfct) {
	case 2:	return &nlsUpHardcodedTable;
	case 4:	return &nlsFnameUpHardcodedTable;
	case 5:	return &nlsFnameTermHardcodedTable;
	case 6: return &nlsCollHardcodedTable;
	}
}
#endif

/*
 *	Copy a buffer and test the size of the buffer
 *	Returns SUCCESS on success; DE_INVLDFUNC on failure
 */
static COUNT cpyBuf(UBYTE FAR *dst, UBYTE FAR *src
	, UWORD srclen, UWORD dstlen)
{
	if(srclen <= dstlen) {
		_fmemcpy((BYTE FAR*)dst, (BYTE FAR*)src, srclen);
		return SUCCESS;
	}
	return DE_INVLDFUNC;		/* buffer too small */
}

/*
 *	Called for all subfunctions other than 0x20-0x23,& 0xA0-0xA2
 *	of DOS-65
 */
COUNT extCtryInfo(int subfct, UWORD codepage
	, UWORD cntry, UWORD bufsize, UBYTE FAR * buf)
{	struct nlsPackage FAR*nls;	/* NLS package to use to return the info from */
	int rc;
	int muxOnCntryInfo, muxOnPointer;
	struct nlsPointerInf FAR *poi;

	if(!buf)
		return DE_INVLDDATA;
	if(subfct == 0)			/* Currently not supported */
		return DE_INVLDFUNC;

		/* nls := NLS package of cntry/codepage */
	if((nls = searchPackage(&codepage, &cntry)) == NULL)
		/* requested NLS package is not loaded -->
			pass the request to NLSFUNC */
		muxOnCntryInfo = muxOnPointer = TRUE;
	else {
		muxOnCntryInfo = NLS_MUX_COUNTRY_INFO(nls);
		muxOnPointer   = NLS_MUX_POINTERS(nls);
	}

	if(subfct == 1) { 	/* return Extended Country Information */
		if(muxOnCntryInfo)
			return mux65(1, codepage, cntry, bufsize, buf);
		return cpyBuf(buf, (BYTE FAR*)&nls->cntryInfo
		 , nls->cntryInfo.size + 3, bufsize);
	}
	if(subfct == NLS_DOS_38) { 	/* return Country Information */
		if(muxOnCntryInfo)
			return mux38(codepage, cntry, bufsize, buf);
		return cpyBuf(buf, (BYTE FAR*)&nls->cntryInfo.dateFmt
		 , nls->cntryInfo.size - 4, bufsize);
	}

	if(muxOnPointer)
		return mux65(subfct, codepage, cntry, bufsize, buf);

	/* any other subfunction returns a pointer to any sort
		of data; the pointer is located within the nlsPointers
		array */
	if((poi = locateSubfct(nls, subfct)) != NULL)
		return cpyBuf(buf, (UBYTE FAR *)poi
		 , sizeof(struct nlsPointerInf), bufsize);

	return DE_INVLDFUNC;
}

/*
 *	Changes the current active codepage or cntry 
 */
static COUNT setPackage(UWORD cp, UWORD cntry)
{	struct nlsPackage FAR*nls;	/* NLS package to use to return the info from */
	int rc;

		/* nls := NLS package of cntry/codepage */
	if((nls = searchPackage(&cp, &cntry)) == NULL) {
		/* not loaded --> invoke NLSFUNC to load it */
		if((rc = muxLoadPkg(cp, cntry)) != SUCCESS)
			return rc;
		if((nls = searchPackage(&cp, &cntry)) == NULL)
			/* something went wrong */
			return DE_INVLDFUNC;
	}

	nlsInfo.actPkg = nls;
#ifdef NLS_CACHE_POINTERS
	/* Fill the quick-access pointers */
	nlsInfo.fnamUpTable = getTable4->pointer - 0x80;
	nlsInfo.upTable = getTable2->pointer - 0x80;
#endif
	return SUCCESS;
}

/*
 *	Called for DOS-38 get info
 *
 *	Note: DOS-38 does not receive the size of the buffer; therefore
 *	it is assumed the buffer is large enough as described in RBIL,
 *	which is 34 bytes _hardcoded_.
 */
COUNT getCountryInformation(UWORD cntry, BYTE FAR *buf)
{	return extCtryInfo(NLS_DOS_38, NLS_DEFAULT, cntry, 34, buf);
}

/*
 *	Called for DOS-38 set country code
 */
COUNT setCountryCode(UWORD cntry)
{	return setPackage(NLS_DEFAULT, cntry);
}

/*
 *	Called for DOS-66-01 get CP
 */
COUNT getCodePage(UWORD FAR* actCP, UWORD FAR*sysCP)
{	*sysCP = nlsInfo.sysCodePage;
	*actCP = nlsInfo.actPkg->cntryInfo.codePage;
	return SUCCESS;
}
/*
 *	Called for DOS-66-02 set CP
 *	Note: One cannot change the system CP. Why it is necessary
 *	to specify it, is lost to me. (2000/02/13 ska)
 */
COUNT setCodePage(UWORD actCP, UWORD sysCP)
{	if(sysCP == NLS_DEFAULT || sysCP == nlsInfo.sysCodePage)
		return setPackage(actCP, NLS_DEFAULT);
	return DE_INVLDDATA;
}



static VOID upMMem(unsigned char FAR *map, unsigned char FAR * str
	, unsigned len)
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


BYTE yesNo(unsigned char ch)           
/* returns: 0: ch == "No", 1: ch == "Yes", 2: ch crap */
{
	if(NLS_MUX_YESNO(nlsInfo.actPkg))
		return muxYesNo(ch);

  ch = upChar(ch);
  if (ch == noChar)
    return 0;
  if (ch == yesChar)
    return 1;
  return 2;
}

VOID upMem(unsigned char FAR * str, unsigned len)
/* upcase a memory area */
{
#ifndef NLS_CACHE_POINTERS
	if(NLS_MUX_EXTERNAL_UP(nlsInfo.actPkg)) {
		muxUpMem(str, len, NLS_NLSFUNC_UP);
		return;
	}
#endif
  upMMem(normalCh, str, len);
}

unsigned char upChar(unsigned char ch)     
 /* upcase a single character */
{	unsigned char buf[1];
	*buf = ch;
  upMem((BYTE FAR*)buf, 1);
  return *buf;
}

VOID upString(unsigned char FAR * str)  
/* upcase a string */
{
  upMem(str, fstrlen(str));
}

VOID upFMem(unsigned char FAR * str, unsigned len)
/* upcase a memory area for file names */
{
#ifndef NLS_CACHE_POINTERS
	if(NLS_MUX_EXTERNAL_FUP(nlsInfo.actPkg)) {
		muxUpMem(str, len, NLS_NLSFUNC_FUP);
		return;
	}
#endif
  upMMem(fileCh, str, len);
}

unsigned char upFChar(unsigned char ch)         
 /* upcase a single character for file names */
{	unsigned char buf[1];

	*buf = ch;
  upFMem((BYTE FAR*)buf, 1);
  return *buf;
}

VOID upFString(unsigned char FAR * str) 
/* upcase a string for file names */
{
  upFMem(str, fstrlen(str));
}


