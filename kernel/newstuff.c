/****************************************************************/
/*                                                              */
/*                           newstuff.c                         */
/*                            DOS-C                             */
/*                                                              */
/*                       Copyright (c) 1996                     */
/*                          Svante Frey                         */
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

#ifdef VERSION_STRINGS
static BYTE *mainRcsId =
    "$Id: newstuff.c 1479 2009-07-07 13:33:24Z bartoldeman $";
#endif

#include        "portab.h"
#include        "globals.h"

/*
    TE-TODO: if called repeatedly by same process, 
    last allocation must be freed. if handle count < 20, copy back to PSP
*/
int SetJFTSize(UWORD nHandles)
{
  UWORD block, maxBlock, i;
  psp FAR *ppsp = MK_FP(cu_psp, 0);
  UBYTE FAR *newtab;

  if (nHandles <= ppsp->ps_maxfiles)
  {
    ppsp->ps_maxfiles = nHandles;
    return SUCCESS;
  }

  if ((DosMemAlloc
       ((nHandles + 0xf) >> 4, mem_access_mode, &block, &maxBlock)) < 0)
    return DE_NOMEM;

  ++block;
  newtab = MK_FP(block, 0);

  i = ppsp->ps_maxfiles;
  /* copy existing part and fill up new part by "no open file" */
  fmemcpy(newtab, ppsp->ps_filetab, i);
  fmemset(newtab + i, 0xff, nHandles - i);

  ppsp->ps_maxfiles = nHandles;
  ppsp->ps_filetab = newtab;

  return SUCCESS;
}

long DosMkTmp(BYTE FAR * pathname, UWORD attr)
{
  /* create filename from current date and time */
  char FAR *ptmp;
  unsigned long randvar;
  long rc;
  int loop;

  ptmp = pathname + fstrlen(pathname);
  if (os_major == 5) { /* clone some bad habit of MS DOS 5.0 only */
    if (ptmp == pathname || (ptmp[-1] != '\\' && ptmp[-1] != '/'))
      *ptmp++ = '\\';
  }
  ptmp[8] = '\0';

  randvar = ((unsigned long)dos_getdate() << 16) | dos_gettime();

  loop = 0;
  do {
    unsigned long tmp = randvar++;
    int i;
    for(i = 7; i >= 0; tmp >>= 4, i--)
      ptmp[i] = ((char)tmp & 0xf) + 'A';

    /* DOS versions: > 5: characters A - P
       < 5: hex digits */
    if (os_major < 5)
      for (i = 0; i < 8; i++)
        ptmp[i] -= (ptmp[i] < 'A' + 10) ? '0' - 'A' : 10;

    /* only create new file -- 2001/09/22 ska*/
    rc = DosOpen(pathname, O_LEGACY | O_CREAT | O_RDWR, attr);
  } while (rc == DE_FILEEXISTS && loop++ < 0xfff);

  return rc;
}

#ifdef DEBUG
#define DEBUG_TRUENAME
#endif

#define drLetterToNr(dr) ((unsigned char)((dr) - 'A'))
/* Convert an uppercased drive letter into the drive index */
#define drNrToLetter(dr) ((dr) + 'A')
/* the other direction */

  /* In DOS there are no free-standing UNC paths, therefore there
     is always a logical drive letter associated with a path
     spec. This letter is also the index into the CDS */

/* 
	Definition of functions for the handling of the Current
	Directory Structure.

	MUX-11-23: Qualify Remote Filename
		DOS-0x60 calls this MUX functions to let the Network Redirector
		qualify the filename. According INTRSPY MS DOS 6 does not pre-
		process the passed in filename in any way (see attached transcripts).

	The DOS-60 interface TRUENAME looks like this:

	DosTruename(src, dest) {
		if (MUX-11-23(src, dest) != Error)
			return SUCCESS
		return local_truename(src, dest);
	}

	The CDS has the following entries:

    char cdsPath[CDSPATHLEN];
    	The fully-qualified current working directory of this drive.
    	The format is DOS <dr>:\[<path>{\<path>}]
		or UNC \<id>\[<path>{\<path>}].
		The drive <dr> indicates the physical drive letter and is the
		index into the blk_device[].

    UWORD cdsFlags;
    	Indicates what kind this logical drive is:
    		NETWORK: drive is NOT local \ If both are set, drive is IFS
    		PHYSICAL: drive is local    / If none is set, drive is non-existant
    		JOIN: drive is joined in as the path cdsPath. This Flag uses the
    			index into the CDS table to indicate the physical drive.
    		SUBST: drive substitutes the path cdsPath.
    		HIDDEN: drive is not included into the redirector's list.

    struct dpb FAR *cdsDpb;
    	Pointer to the DPB driving the physical drive. In DOS-C, the physical
    	drive letter is the index into the DPB[]. But for compatibly reason
    	this field will be set correctly.

	UWORD cdsStartCluster;
		For local drives only: This holds the cluster number of
		the start of the current working directory of this
		logical drive. If 0000h, it's the root directory; if
		0ffffh, the drive was never accessed and has to be read
		again.

	void FAR *cdsIFSrecord;
	UWORD cdsIFSparameter;
		For networked drives only: Holds pointer/parameters to/for IFS
		driver. (Well, I don't know.)

    UWORD cdsPathOff;
    	Number of characters of the cdsPath[], which are hidden. The
    	logical path is combined by the logical drive letter and the
    	cdsPath[] part, which is not hidden.

    IFS FAR *cdsIFSdrv;
    	Will be zeroed for local drives.

   Revision 1.2  1995/12/03 22:17:41  ska
   bugfix: Scanning of file name in 8.3 failed on "." and on longer names.

   Revision 1.1  1995/11/09 07:43:30  ska
   #

*/

#define PATH_ERROR goto errRet
#define PATHLEN 128


/* Map a logical path into a physical one.

	1) Uppercasing path.
	2) Flipping '/' -> '\\'.
	3) Removing empty directory components & ".".
	4) Processing ".." components.
	5) Convert path components into 8.3 convention.
	6) Make it fully-qualified.
	7) Map it to SUBST/UNC.
        8) Map to JOIN.

   Return:
   	*cdsItem will be point to the appropriate CDS entry. This will allow
   	the caller to aquire the DPB or the IFS informtion of this entry.
   	error number
   	Return value:
   		DE_FILENOTFND, or DE_PATHNOTFND (as described in RBIL)
   	If the output path pnfo->physPath exceeds the length MAX_PATH, the error
   	DE_FILENOTFND will be returned.
*/

/*
 * Added support for external and internal calls.
 * Clean buffer before use. Make the true path and expand file names.
 * Example: *.* -> ????????.??? as in the currect way.
 * MSD returns \\D.\A.\????????.??? with SHSUCDX. So, this code is not
 * compatible MSD Func 60h.
 */

/*TE TODO:

    experimenting with NUL on MSDOS 7.0 (win95)
    
                        WIN95           FREEDOS
    TRUENAME NUL        C:/NUL             OK
    TRUENAME .\NUL      C:\DOS\NUL         
    TRUENAME ..\NUL     C:\NUL
    TRUENAME ..\..\NUL  path not found
    TRUENAME Z:NUL      invalid drive (not lastdrive!!)
    TRUENAME A:NUL      A:/NUL             OK
    TRUENAME A:\NUL     A:\NUL

*/

#ifdef DEBUG_TRUENAME
#define tn_printf(x) printf x
#else
#define tn_printf(x)
#endif

#define PNE_WILDCARD 1
#define PNE_DOT 2

STATIC const char _DirChars[] = "\"[]:|<>+=;,";

#define DirChar(c)  (((unsigned char)(c)) >= ' ' && \
                     !strchr(_DirChars, (c)))

#define addChar(c) \
{ \
  if (p >= dest + SFTMAX) PATH_ERROR; /* path too long */	\
  *p++ = c; \
}

COUNT truename(const char FAR * src, char * dest, COUNT mode)
{
  COUNT i;
  struct dhdr FAR *dhp;
  const char FAR *froot;
  COUNT result;
  unsigned state;
  struct cds FAR *cdsEntry;
  char *p = dest;	  /* dynamic pointer into dest */
  char *rootPos;
  char src0;

  tn_printf(("truename(%S)\n", src));

  /* First, adjust the source pointer */
  src = adjust_far(src);

  /* In opposite of the TRUENAME shell command, an empty string is
     rejected by MS DOS 6 */
  src0 = src[0];
  if (src0 == '\0')
    return DE_FILENOTFND;

  if (src0 == '\\' && src[1] == '\\') {
    const char FAR *unc_src = src;
    /* Flag UNC paths and short circuit processing.  Set current LDT   */
    /* to sentinel (offset 0xFFFF) for redirector processing.          */
    tn_printf(("Truename: UNC detected\n"));
    do {
      src0 = unc_src[0];
      addChar(src0);
      unc_src++;
    } while (src0);
    current_ldt = (struct cds FAR *)MK_FP(0xFFFF,0xFFFF);
    tn_printf(("Returning path: \"%s\"\n", dest));
    /* Flag as network - drive bits are empty but shouldn't get */
    /* referenced for network with empty current_ldt.           */
    return IS_NETWORK;
  }
  
  /* Do we have a drive?                                          */
  if (src[1] == ':')
    result = drLetterToNr(DosUpFChar(src0));
  else
    result = default_drive;

  dhp = IsDevice(src);

  cdsEntry = get_cds(result);
  if (cdsEntry == NULL)
  {
    /* workaround for a device prefixed with invalid drive (e.g. "@:NUL") */
    /* (MS-DOS always return drive P: for invalid drive. Why P:?) */
    if (dhp)
    {
      result = default_drive;
      cdsEntry = get_cds(result);
      if (cdsEntry == NULL)
        return DE_PATHNOTFND;
    }
    else
      return DE_PATHNOTFND;
  }

  fmemcpy(&TempCDS, cdsEntry, sizeof(TempCDS));
  tn_printf(("CDS entry: #%u @%p (%u) '%s'\n", result, cdsEntry,
            TempCDS.cdsBackslashOffset, TempCDS.cdsCurrentPath));
  /* is the current_ldt thing necessary for compatibly??
     -- 2001/09/03 ska*/
  current_ldt = cdsEntry;
  if (TempCDS.cdsFlags & CDSNETWDRV)
    result |= IS_NETWORK;
  
  if (dhp)
    result |= IS_DEVICE;

  /* Try if the Network redirector wants to do it */
  dest[0] = '\0';		/* better probable for sanity check below --
                                   included by original truename() */
  /* MUX succeeded and really something */
  if (!(mode & CDS_MODE_SKIP_PHYSICAL) &&
      QRemote_Fn(dest, src) == SUCCESS && dest[0] != '\0')
  {
    tn_printf(("QRemoteFn() returned: \"%s\"\n", dest));
#ifdef DEBUG_TRUENAME
    if (strlen(dest) >= SFTMAX)
      panic("Truename: QRemote_Fn() overflowed output buffer");
#endif
    if (dest[2] == '/' && (result & IS_DEVICE))
      result &= ~IS_NETWORK;
    return result;
  }

  /* Redirector interface failed --> proceed with local mapper */
  dest[0] = drNrToLetter(result & 0x1f);
  dest[1] = ':';

  /* Do we have a drive? */
  if (src[1] == ':')
    src += 2;

/*
    Code repoff from dosfns.c
    MSD returns X:/CON for truename con. Not X:\CON
*/
  /* check for a device  */

  dest[2] = '\\';
  if (result & IS_DEVICE)
  {
    froot = get_root(src);
    if (froot == src || froot == src + 5)
    {
      if (froot == src + 5)
      {
        fmemcpy(dest + 3, src, 5);
        DosUpMem(dest + 3, 5);
        if (dest[3] == '/') dest[3] = '\\';
        if (dest[7] == '/') dest[7] = '\\';
      }
      if (froot == src || memcmp(dest + 3, "\\DEV\\", 5) == 0)
      {
        /* /// Bugfix: NUL.LST is the same as NUL.  This is true for all
           devices.  On a device name, the extension is irrelevant
           as long as the name matches.
           - Ron Cemer */
        dest[2] = '/';
        result &= ~IS_NETWORK;
        /* /// DOS will return C:/NUL.LST if you pass NUL.LST in.
           DOS will also return C:/NUL.??? if you pass NUL.* in.
           Code added here to support this.
           - Ron Cemer */
        src = froot;
      }
    }
  }
    
  /* Make fully-qualified logical path */
  /* register these two used characters and the \0 terminator byte */
  /* we always append the current dir to stat the drive;
     the only exceptions are devices without paths */
  rootPos = p = dest + 2;
  if (*p != '/') /* i.e., it's a backslash! */
  {
    BYTE *cp;

    cp = TempCDS.cdsCurrentPath;
    /* ensure termination of strcpy */
    cp[MAX_CDSPATH - 1] = '\0';
    if ((TempCDS.cdsFlags & CDSNETWDRV) == 0)
    {
      if (media_check(TempCDS.cdsDpb) < 0)
        return DE_PATHNOTFND;

      /* dos_cd ensures that the path exists; if not, we
         need to change to the root directory */
      if (dos_cd(cp) != SUCCESS) {
        cp[TempCDS.cdsBackslashOffset + 1] =
          cdsEntry->cdsCurrentPath[TempCDS.cdsBackslashOffset + 1] = '\0';
        dos_cd(cp);
      }
    }

    if (!(mode & CDS_MODE_SKIP_PHYSICAL))
    {
      tn_printf(("SUBSTing from: %s\n", cp));
/* What to do now: the logical drive letter will be replaced by the hidden
   portion of the associated path. This is necessary for NETWORK and
   SUBST drives. For local drives it should not harm.
   This is actually the reverse mechanism of JOINED drives. */

      strcpy(dest, cp);
      if (TempCDS.cdsFlags & CDSSUBST)
      {
        /* The drive had been changed --> update the CDS pointer */
        if (dest[1] == ':')
        {  /* sanity check if this really is a local drive still */
          unsigned i = drLetterToNr(dest[0]);
          
          /* truename returns the "real", not the "virtual" drive letter! */
          if (i < lastdrive) /* sanity check #2 */
            result = (result & 0xffe0) | i;
        }
      }
      rootPos = p = dest + TempCDS.cdsBackslashOffset;
    }
    else
    {
      cp += TempCDS.cdsBackslashOffset;
      /* truename must use the CuDir of the "virtual" drive letter! */
      /* tn_printf(("DosGetCuDir drive #%u\n", prevresult & 0x1f)); */
      strcpy(p, cp);
    }
    if (p[0] == '\0')
      p[1] = p[0];
    p[0] = '\\'; /* force backslash! */

    if (*src != '\\' && *src != '/')
      p += strlen(p);
    else /* skip the absolute path marker */
      src++;
    /* remove trailing separator */
    if (p[-1] == '\\') p--;
  }

  /* append the path specified in src */

  state = 0;
  while(*src)
  {
    /* New segment.  If any wildcards in previous
       segment(s), this is an invalid path. */
    if (state & PNE_WILDCARD)
      return DE_PATHNOTFND;

    /* append backslash if not already there.
       MS DOS preserves a trailing '\\', so an access to "C:\\DOS\\"
       or "CDS.C\\" fails; in that case the last new segment consists of just
       the \ */
    if (p[-1] != *rootPos)
      addChar(*rootPos);
    /* skip multiple separators (duplicated slashes) */
    while (*src == '/' || *src == '\\')
      src++;

    if(*src == '.')
    {
      /* special directory component */
      ++src;
      if (*src == '.') /* skip the second dot */
        ++src;
      if (*src == '/' || *src == '\\' || *src == '\0')
      {
        --p; /* backup the backslash */
        if (src[-2] == '.')
        {
          /* ".." entry */
          /* remove last path component */
          while(*--p != '\\')
            if (p <= rootPos) /* already on root */
              return DE_PATHNOTFND;
        }
        continue;	/* next char */
      }

      /* ill-formed .* or ..* entries => return error */
    errRet:
      /* The error is either PATHNOTFND or FILENOTFND
         depending on if it is not the last component */
      return fstrchr(src, '/') == 0 && fstrchr(src, '\\') == 0
        ? DE_FILENOTFND
        : DE_PATHNOTFND;
    }

    /* normal component */
    /* append component in 8.3 convention */

    /* *** parse name and extension *** */
    i = FNAME_SIZE;
    state &= ~PNE_DOT;
    while(*src != '/' && *src  != '\\' && *src != '\0')
    {
      char c = *src++;
      if (c == '*')
      {
        /* register the wildcard, even if no '?' is appended */
        c = '?';
        while (i)
        {
          --i;
          addChar(c);
        }
        /** Alternative implementation:
            if (i)
            {
              if (dest + SFTMAX - *p < i)
                PATH_ERROR;
              fmemset(p, '?', i);
              p += i;
            }		**/
      }
      if (c == '.')
      {
        if (state & PNE_DOT) /* multiple dots are ill-formed */
          PATH_ERROR;
        /* strip trailing dot */
        if (*src == '/' || *src == '\\' || *src == '\0')
          break;
        /* we arrive here only when an extension-dot has been found */
        state |= PNE_DOT;
        i = FEXT_SIZE + 1;
      }
      else if (c == '?')
        state |= PNE_WILDCARD;
      if (i) {	/* name length in limits */
        --i;
        if (!DirChar(c)) PATH_ERROR;
        addChar(c);
      }
    }
    /* *** end of parse name and extension *** */
  }
  if (state & PNE_WILDCARD && !(mode & CDS_MODE_ALLOW_WILDCARDS))
    return DE_PATHNOTFND;
  if (p == dest + 2)
  {
    /* we must always add a seperator if dest = "c:" */
    addChar('\\');
  }
  
  *p = '\0';				/* add the string terminator */
  DosUpFString(rootPos);	        /* upcase the file/path name */
  
/** Note:
    Only the portions passed in by the user are upcased, because it is
    assumed that the CDS is configured correctly and if it contains
    lower case letters, it is required so **/
  
  tn_printf(("Absolute logical path: \"%s\"\n", dest));
  
  /* Now, all the steps 1) .. 7) are fullfilled. Join now */
  /* search, if this path is a joined drive */

  if (dest[2] != '/' && (!(mode & CDS_MODE_SKIP_PHYSICAL)) && njoined)
  {
    struct cds FAR *cdsp = CDSp;
    for(i = 0; i < lastdrive; ++i, ++cdsp)
    {
      /* How many bytes must match */
      size_t j = fstrlen(cdsp->cdsCurrentPath);
      /* the last component must end before the backslash offset and */
      /* the path the drive is joined to leads the logical path */
      if ((cdsp->cdsFlags & CDSJOINED) && (dest[j] == '\\' || dest[j] == '\0')
         && fmemcmp(dest, cdsp->cdsCurrentPath, j) == 0)
      { /* JOINed drive found */
        dest[0] = drNrToLetter(i);	/* index is physical here */
        dest[1] = ':';
        if (dest[j] == '\0')
        {	/* Reduce to root direc */
          dest[2] = '\\';
          dest[3] = 0;
          /* move the relative path right behind the drive letter */
        }
        else if (j != 2)
        {
          strcpy(dest + 2, dest + j);
        }
        result = (result & 0xffe0) | i; /* tweak drive letter (JOIN) */
        current_ldt = cdsp;
        result &= ~IS_NETWORK;
        if (cdsp->cdsFlags & CDSNETWDRV)
          result |= IS_NETWORK;
	tn_printf(("JOINed path: \"%s\"\n", dest));
        return result;
      }
    }
    /* nothing found => continue normally */
  }
  if ((mode & CDS_MODE_CHECK_DEV_PATH) &&
      ((result & (IS_DEVICE|IS_NETWORK)) == IS_DEVICE) &&
      dest[2] != '/' && !dir_exists(dest))
    return DE_PATHNOTFND;

  /* Note: Not reached on error or if JOIN or QRemote_Fn (2f.1123) matched */
  if (mode==CDS_MODE_ALLOW_WILDCARDS) /* DosTruename mode */
  {
    /* in other words: result & 0x60 = 0x20...: */
    if (os_major==6 && (result & (IS_DEVICE|IS_NETWORK)) == IS_DEVICE)
      result = 0x3a00; /* MS DOS 6.22, according to RBIL: AH=3a if char dev */
    else
      result = 0; /* AL is 00, 2f, 5c, or last-of-TempCDS.cdsCurrentPath? */
  }
  tn_printf(("Physical path: \"%s\"\n", dest));
  return result;
}

#if 0
/**********************************************
	Result of INTRSPY

	Calling RBIL's INT.COM in MS DOS v6.22

=== Script: MUX.SCR

intercept 2fh
    function 11h    ; network redirector
    	subfunction 23h		; Qualify path and filename
			on_entry
				output "1123: IN: " (ds:SI->byte,asciiz,64)
			on_exit
				if (cflag == 1)
					sameline " [FAIL " ax "]"
				output "1123: OUT: " (es:dI->byte,asciiz,64)
				output "1123: orig buffer: " (ds:sI->byte,asciiz,64)
	function 12h
		subfunction 21h
			on_entry
				output "1221: IN: " (ds:SI->byte,asciiz,64)
			on_exit
				if (cflag == 1)
					sameline " [FAIL " ax "]"
				output "1221: OUT: " (es:dI->byte,asciiz,64)

=== Batch file: SPY_INT.BAT
@echo off
if exist report.out del report.out
cmdspy stop
cmdspy flush
cmdspy restart
int ax=0x6000 -buf ds:si="abcöflkgsxkf\0" -buf es:di="%256s" -int 0x21 -d es:di:128 >spy_int.out
cmdspy stop
cmdspy report report.out
more report.out
=== Intspy report file: REPORT.OUT
1123: IN:  C:\INTRSPY\SPY_INT.BAT [FAIL 0001]
1123: OUT:  
1123: orig buffer:  C:\INTRSPY\SPY_INT.BAT
1123: IN:  int.??? [FAIL 0001]
1123: OUT:  C:\INTRSPY
1123: orig buffer:  int.???
1123: IN:  C:\TOOL\int.??? [FAIL 0001]
1123: OUT:  C:\INTRSPY
1123: orig buffer:  C:\TOOL\int.???
1123: IN:  spy_int.out [FAIL 0001]
1123: OUT:  C:\TOOL\INT.???
1123: orig buffer:  spy_int.out
1123: IN:  C:\TOOL\INT.COM [FAIL 0001]
1123: OUT:  C:\INTRSPY\SPY_INT.OUT
1123: orig buffer:  C:\TOOL\INT.COM
1123: IN:  abcöflkgsxkf [FAIL 0001]
1123: OUT:  C:\TOOL\INT.COM
1123: orig buffer:  abcöflkgsxkf
1123: IN:  C:\INTRSPY\SPY_INT.BAT [FAIL 0001]
1123: OUT:  C:\INTRSPY\ABCÖFLKG
1123: orig buffer:  C:\INTRSPY\SPY_INT.BAT
1123: IN:  cmdspy.??? [FAIL 0001]
1123: OUT:  C:\INTRSPY
1123: orig buffer:  cmdspy.???
1123: IN:  C:\INTRSPY\CMDSPY.EXE [FAIL 0001]
1123: OUT:  C:\INTRSPY
1123: orig buffer:  C:\INTRSPY\CMDSPY.EXE
=== INT.COM output: SPY_INT.OUT
              000   CX=0000   DX=0000
SI=4A5E   DI=4A76   BP=FF70   SP=FF64
CS=0000   DS=279D   ES=279D   SS=0000   CPU Flags: 0n00oditsz0a0p1c

INT: 0x21

AX=0059   BX=0000   CX=0000   DX=0000
SI=4A5E   DI=4A76   BP=FF70   SP=FF64
CS=0000   DS=279D   ES=279D   SS=0000   CPU Flags: 0N11odItSz0A0P1c
DOSERR: 0000 (0)

*<es:di:128> {
43(C) 3A(:) 5C(\) 49(I) 4E(N) 54(T) 52(R) 53(S) 50(P) 59(Y) 5C(\) 41(A)
42(B) 43(C) 99(Ö) 46(F) 4C(L) 4B(K) 47(G) 00(.) 3D(=) 30(0) 30(0) 30(0)
30(0) 20( ) 20( ) 20( ) 43(C) 58(X) 3D(=) 30(0) 30(0) 30(0) 30(0) 28(()
30(0) 29()) 20( ) 32(2) 38(8) 28(() 28(() 29()) 20( ) 33(3) 30(0) 28(()
30(0) 29()) 20( ) 32(2) 39(9) 28(() 29()) 29()) 20( ) 32(2) 30(0) 28(()
20( ) 29()) 20( ) 33(3) 32(2) 28(() 32(2) 29()) 20( ) 33(3) 38(8) 28(()
38(8) 29()) 20( ) 32(2) 38(8) 28(() 28(() 29()) 20( ) 32(2) 38(8) 28(()
28(() 29()) 20( ) 32(2) 39(9) 28(() 29()) 29()) 20( ) 32(2) 30(0) 28(()
20( ) 29()) 20( ) 33(3) 33(3) 28(() 33(3) 29()) 20( ) 33(3) 30(0) 28(()
30(0) 29()) 20( ) 32(2) 38(8) 28(() 28(() 29()) 20( ) 33(3) 30(0) 28(()
30(0) 29()) 20( ) 32(2) 39(9) 28(() 29()) 29()) }
===

The actual interesting lines are the 6th "IN:" of the report file.
The DOS interface passed _exactly_ the same string to MUX-11-23 as
written on command line, the same applied to "con\0", a device driver.

***************************************/

#endif

