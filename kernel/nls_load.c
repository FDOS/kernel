/****************************************************************/
/*                                                              */
/*                            nls_load.c                        */
/*                           FreeDOS                            */
/*                                                              */
/*    National Languge Support functions and data structures    */
/*    Load an entry from FreeDOS COUNTRY.SYS file.				*/
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

#include "portab.h"
#include "init-mod.h"
//#include "pcb.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id$";
#endif

#define filename Config.cfgCSYS_fnam
#define cntry Config.cfgCSYS_cntry
#define cp Config.cfgCSYS_cp

STATIC int err(void)
{
  printf("Syntax error in or invalid COUNTRY.SYS: \"%s\"\n", filename);
  return 0;
}

#define readStruct(s)	readStructure(&(s), sizeof(s), fd)
STATIC int readStructure(void *buf, unsigned size, COUNT fd)
{
  if (read(fd, buf, size) == size)
    return 1;

  return err();
}

        /* Evaluate each argument only once */
#define readFct(p,f)	readFct_((p), (f), fd)
int readFct_(void *buf, struct csys_function *fct, COUNT fd)
{
  if (lseek(fd, fct->csys_rpos, 0) >= 0)
    return readStructure(buf, fct->csys_length, fd);
  return err();
}

#define seek(n)	rseek((LONG)(n), fd)
static int rseek(LONG rpos, COUNT fd)
{
  if (lseek(fd, rpos, 1) >= 0)
    return 1;

  return err();
}

COUNT csysOpen(void)
{
  COUNT fd;
  struct nlsCSys_fileHeader header;

  if ((fd = open(filename, 0)) < 0)
  {
    printf("Cannot open: \"%s\"\n", filename);
    return 1;
  }

  if ((read(fd, &header, sizeof(header)) != sizeof(header))
      ||strcmp(header.csys_idstring, CSYS_FD_IDSTRING) != 0
      || lseek(fd, (LONG) sizeof(struct csys_completeFileHeader), 0)
      != (LONG) sizeof(struct csys_completeFileHeader))
  {
    printf("No valid COUNTRY.SYS: \"%s\"\n\nTry NLSFUNC /i %s\n", filename,
           filename);
    close(fd);
    return -1;
  }

  return fd;
}

/* Searches for function definition of table #fctID and
	moves it at index idx */
STATIC int chkTable(int idx, int fctID, struct csys_function *fcts,
                    int numFct)
{
  struct csys_function *fct, hfct;
  int i;

  for (i = 0, fct = fcts; i < numFct; ++i, ++fct)
    if (fct->csys_fctID == fctID)
    {
      /* function found */
      if (i == idx)             /* already best place */
        return 1;
      /* Swap both places */
      fmemcpy(&hfct, fct, sizeof(hfct));
      fmemcpy(fct, &fcts[idx], sizeof(hfct));
      fmemcpy(&fcts[idx], &hfct, sizeof(hfct));
      return 1;
    }

  printf("Mandatory table %u not found.\n", fctID);
  return 0;
}

/*
 *	Description of the algorithm the COUNTRY= information is loaded.

1) LoadCountry() is activated in pass 1, because it offers the functionality
how to upcase characters to the kernel, it should be activated as
soon as possible within processing CONFIG.SYS.

2) The method to locate the actual data within COUNTRY.SYS is pretty
basic and straight forward; so no detailed description here.

3) To reduce permanent memory useage the option NLS_MODIFYABLE_DATA
controls whether or not the loaded NLS pkg may be modified. By default,
modifying this data is _not_ supported.
The reason is to allow to re-use already loaded data, e.g. to use the
same physical table for #2 (normal upcase) and #4 (filename upcase).
NLSFUNC even can re-use the data loaded via COUNTRY= and the hardcoded data.

4) The problem is that even without point 3) it is not easily possible to
pre-judge how many bytes the NLS pkg will allocate in memory, because
this NLS implementation wants to support a wider range of NLS pkgs; because
neither the number of subfunctions nor the size of the data per subfunction
is fixed globally.
Therefore, the package is built-up incrementally:
4.1) First all function definition structures (S3) are read into memory.
While doing so the position field is transformed into the absolute
position within COUNTRY.SYS.
4.2) Then they are checked if a subfunction is defined more than once.
4.3) Then the entries are sorted in that way that tables 0x23, 1, 2, 4 and 5
lead the table in that order.
4.4) Then the basic nlsPackage with as many entries within nlsPointers[]
is allocated as there are function definitions left (minus one as the
pseudo-table 0x23 does not require an entry).
4.5) Pseudo-table 0x23 is installed.
4.6) Table 1 is installed at the very end of the nlsPointers[] array.
4.7) In the order all remaining function definitions are installed in
the same order as in the other array.

5) "Installing" means (except table 0x23):
5.1) Enlarge the nlsPackage structure to hold the necessary bytes of the
function definition (member csys_length).
5.2) Only if NLS_MODIFYABLE_DATA is _not_ defined and table is not #1:
The loaded data is compared to already loaded data and if such pattern is
already found in memory, a pointer to that memory area is used and the
loaded data is discarded.
First the local data is searched through, then the area of the hardcoded
NLS pkg.
Efficiency: function definitions with the same file position can automatically
use the same memory area.

6) When all function definitions are loaded, the nlsPackage structure is
tightly filled without any pad bytes; two areas are wasted:
a) The area containing the S3 structures, and
b) probably the last loaded data could be found within the memory already,
so the nlsPackage structure is larger than necessary.

8) But the memory allocation in pass 1 is temporary anyway, because in
the PostConfig() phase, all memory allocations are revoked and created
anew. At this point -- immediately after revoking all memory and
_before_ allocating any new memory -- the NLS pkg is located completely
within memory and one knows exactly which bytes to spare, and which data
can share the same physical memory; but if the normal PostConfig()
process would go on, this information would be lost, because it could be
overwritten.
==> Therefore the almost first operation within PostConfig() is to
move the NLS pkg upto the top (base?) of memory, thus, making sure
it is not overwritten and one need not re-load all the structures from
memory and, by doing so, loose the information which memory can be shared.

9) Once this operation has been completed, the NLS pkg is joined into the
nlsInfo chain of loaded packages and is made active.

===

To ease implementation the value of FP_SEG(nlsPointers[].pointer) != 0,
if the pointer refers to an absolute place, whereas FP_SEG() == 0,
indicates that the FP_OFF(...) is the offset base-relative to the data
offset; which is base-relative to the "nls" pointer.
 */
int csysLoadPackage(COUNT fd)
{
  struct csys_numEntries entries;
  struct csys_ccDefinition entry;
  struct csys_function *fcts;
  struct nlsPackage *nls;
  struct nlsPointer *poi;
  int highmark, numFct, i, j;
  int totalSize;
#ifndef NLS_MODIFYABLE_DATA
  BYTE FAR *p;
#endif
#define numE entries.csys_entries
#define bufp(offset)	(((BYTE*)nls) + (offset))
#define fct fcts[numFct]

  /* When this function is called, the position of the file is
     at offset 128 (number of country/codepage pairs) */
  if (!readStruct(entries))
    return 0;
  while (numE--)
  {
    if (!readStruct(entry))
      return 0;
    if (entry.csys_cntry == cntry
        && (cp == NLS_DEFAULT || entry.csys_cp == cp))
    {
      /* Requested entry found! */
      if (!seek(entry.csys_rpos) || !readStruct(entries))
        return 0;
      /* Now reading the function definitions at this position */
      if (numE < 5)
      {
        printf("Syntax error in COUNTRY.SYS: Too few subfunctions\n");
        return 0;
      }
      /* If the file structure is good, each but one entry (0x23) is
         one item within nlsPointers[] array */
      fcts = KernelAlloc(sizeof(struct csys_function) * numE);
      numFct = 0;               /* number of already loaded fct definition */
      totalSize = 0;
      {
        if (!readStruct(fct))
          return 0;
        switch (fct.csys_fctID)
        {
          case 0:
          case 0x20:
          case 0x21:
          case 0x22:
          case 0xA0:
          case 0xA1:
          case 0xA2:
            printf("Invalid subfunction %u ignored", fct.csys_fctID);
            continue;
          case 0x23:
            if (fct.csys_length != 2)
            {
              printf("Pseudo-table 35 length mismatch\n");
              continue;
            }
        }
        /* Search if the subfunction is already there */
        for (j = 0; j < numFct && fcts[j].csys_fctID != fct.csys_fctID;
             ++j) ;
        if (j != numFct)
        {
          printf("Subfunction %u defined multiple times, ignored\n",
                 fct.csys_fctID);
          continue;
        }

        /* OK --> update the rpos member */
        fct.csys_rpos += DosLtell(fd);
        totalSize += fct.csys_length;
        ++numFct;
      }
      while (--numE) ;

      /* i is the number of available function definition */
      /* check if all mandatory tables are loaded, at the same
         time re-order the function definitions like that:
         0x23, 1, 2, 4, 5
       */

      /* That's automatically a check that more than 3 definitions
         are available */
      if (!chkTable(0, 0x23, fcts, numFct)      /* pseudo-table 0x23 yes/no */
          || !chkTable(1, 1, fcts, numFct)      /* ext cntry info */
          || !chkTable(2, 2, fcts, numFct)      /* normal upcase */
          || !chkTable(3, 4, fcts, numFct)      /* filename upcase */
          || !chkTable(4, 5, fcts, numFct))     /* filename terminator chars */
        return 0;

      /* Begin the loading process by to allocate memory as if
         we had to load every byte */
      /* One nlsPointers structure is already part of nlsPackage;
         two function definitions need no nlsPointers entry (0x32, 1);
         one additional byte is required by table 1, but which is
         already within totalSize as the length of pseudo-table
         0x23 has been counted. */
      nls = KernelAlloc((data = sizeof(struct nlsPackage)
                         + (numFct - 3) * sizeof(struct nlsPointer)) +
                        totalSize);
      /* data := first byte not used by the control area of
         the nlsPackage structure; at this point it is the
         offset where table #1 is to be loaded to */

      /* Install pseudo-table 0x23 */
      if (!readFct((BYTE *) & nls->yeschar, fcts))
        return 0;
      nls->numSubfct = numFct - 1;      /* pseudo-table 0x23 */

      /* Install table #1 has it must overlay the last nlsPointers[]
         item */
      *bufp(data) = 1;          /* table #1 starts with the subfctID
                                   then the data from the file follows */
      if (!readFct(bufp(++data), ++fcts))
        return 0;
      data += fcts->csys_length;        /* first byte of local data area */
      highmark = data;          /* first unused byte */

      for (j = 0, poi = nls->nlsPointers; j < numFct - 1; ++j, ++poi)
      {
        /* consecutively load all functions */
        if (!readFct(bufp(data), ++fcts))
          return 0;
        poi->subfct = fcts->csys_fctID;
        /* Now the function data is located at the current top of
           used memory and, if allowed, the other memory is
           tested, if such image is already loaded */
#ifndef NLS_MODIFYABLE_DATA
        /* Try to locate the contents of the buffer */
                                /** brute force **/
        /* For the standard tables one need to match tables
           2 and 4 only. */
        for (i = data; i + fcts->csys_length < highmark; ++i)
        {
          if (memcmp(bufp(i), bufp(highmark), fcts->csys_length) == 0)
          {
            /* found! */
            /* ==> leave highmark untouch, but modify pointer */
            poi->pointer = MK_FP(0, i);
            /* the segment portion == 0 identifies this pointer
               as local within the current data area */
            goto nxtEntry;
          }
        }
        /* Now try the hardcoded area */
        for (p = hcTablesStart; p < hcTablesEnd - fcts->csys_length; ++p)
        {
          if (fmemcmp(p, bufp(highmark), fcts->csys_length) == 0)
          {
            /* found! */
            /* ==> leave highmark untouch, but modify pointer */
            poi->pointer = p;
            /* the segment portion != 0 identifies this is an
               absolute pointer */
            goto nxtEntry;
          }
        }
#endif
        /* Either not found or modifyable data allowed */
        poi->pointer = MK_FP(0, highmark);      /* local address */
        highmark += fcts->csys_length;  /* need to keep the data */
      nxtEntry:
      }
      /* how many memory is really required */
      Country.cfgCSYS_memory = highmark;
      Country.cfgCSYS_data = nls;
      return 1;
    }
  }
#undef numE
  if (cp == NLS_DEFAULT)
    printf("No definition of country ID %u in file \"%s\"\n",
           cntry, filename);
  else
    printf
        ("No definition of country ID %u for codepage %u in file \"%s\"\n",
         cntry, cp, filename);

  return 0;
}

BOOL LoadCountryInfo(char *fnam)
{
  COUNT fd;
  int rc;

  if (strlen(fnam) < sizeof(filename))
  {
    strcpy(filename, fnam);
    if ((fd = csysOpen()) >= 0)
    {
      rc = csysLoadPackage(fd);
      close(fd);
      return rc;
    }
  }
  else
    printf("Filename too long\n");
  return 0;
}

/*
 * Log: nls_load.c,v  for newer entries do "cvs log nls_load.c"
 *
 * Revision 1.1  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 */
