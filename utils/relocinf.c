/*****************************************************************************
**    RelocInf.C
**    
**    provide some info about relocation entries in an exe file
**
**    usage:
**        RelocInfo exefile
**
**
** Copyright 2001 by tom ehlert
**
** GPL bla to be added, but intended as GPL
**       
**
** 09/06/2001 - initial revision
** not my biggest kind of software; anyone willing to add 
** comments, errormessages, usage info,...???
** 
*****************************************************************************/

* /
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
typedef unsigned short UWORD;
typedef unsigned long ULONG;
#ifndef _MSC_VER
#define const
#define __cdecl cdecl
#endif

/* from EXE.H */
typedef struct {
  UWORD exSignature;
  UWORD exExtraBytes;
  UWORD exPages;
  UWORD exRelocItems;
  UWORD exHeaderSize;
  UWORD exMinAlloc;
  UWORD exMaxAlloc;
  UWORD exInitSS;
  UWORD exInitSP;
  UWORD exCheckSum;
  UWORD exInitIP;
  UWORD exInitCS;
  UWORD exRelocTable;
  UWORD exOverlay;
} exe_header;

#define MAGIC 0x5a4d

struct relocEntry {
  UWORD off;
  UWORD seg;
  UWORD refseg;
};

int __cdecl compReloc(const void *p1, const void *p2)
{
  struct relocEntry *r1 = (struct relocEntry *)p1;
  struct relocEntry *r2 = (struct relocEntry *)p2;

  if (r1->refseg > r2->refseg)
    return 1;
  if (r1->refseg < r2->refseg)
    return -1;

  if (r1->seg > r2->seg)
    return 1;
  if (r1->seg < r2->seg)
    return -1;

  if (r1->off > r2->off)
    return 1;
  if (r1->off < r2->off)
    return -1;

  return 0;
}

main(int argc, char *argv[])
{
  FILE *fdin;
  exe_header header;
  struct relocEntry *reloc;

  int i;
  ULONG image_offset;

  if (argc < 2 || (fdin = fopen(argv[1], "rb")) == NULL)
  {
    printf("can't open %s\n", argv[1]);
    exit(1);
  }

  if (fread(&header, sizeof(header), 1, fdin) != 1 ||
      header.exSignature != MAGIC)
  {
    printf("%s is no EXE file\n");
    exit(1);
  }

  printf("%u relocation entries found\n", header.exRelocItems);

  if (header.exRelocItems > 0x8000 / sizeof(*reloc))
  {
    printf("too many relocation entries \n");
    exit(1);
  }

  if ((reloc = malloc(header.exRelocItems * sizeof(*reloc))) == NULL)
  {
    printf("can't alloc memory\n");
    exit(1);
  }

  if (fseek(fdin, header.exRelocTable, 0))
  {
    printf("can't seek\n");
    exit(1);
  }

  for (i = 0; i < header.exRelocItems; i++)
    if (fread(reloc + i, 4, 1, fdin) != 1)
    {
      printf("can't read reloc info\n");
      exit(1);
    }

  for (i = 0; i < header.exRelocItems; i++)
  {
    image_offset = (ULONG) header.exHeaderSize * 16;

    image_offset += ((ULONG) reloc[i].seg << 4) + reloc[i].off;

    if (fseek(fdin, image_offset, 0))
    {
      printf("can't seek reloc data\n");
      exit(1);
    }

    if (fread(&reloc[i].refseg, 2, 1, fdin) != 1)
    {
      printf("can't read rel data for item %d\n", i);
      exit(1);
    }
    /* printf("%04x:%04x -> %04x\n", reloc[i].seg, reloc[i].off, reloc[i].refseg); */
  }

  /* sort reloc entries */

  qsort(reloc, header.exRelocItems, sizeof(*reloc), compReloc);

  for (i = 0; i < header.exRelocItems; i++)
  {
    if (i == 0)
      printf("#    seg:off references data in -->\n");
    printf("%3d %04x:%04x -> %04x\n", i, reloc[i].seg, reloc[i].off,
           reloc[i].refseg);
  }

}
