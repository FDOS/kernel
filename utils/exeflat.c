/****************************************************************/
/*                                                              */
/*                          exeflat.c                           */
/*                                                              */
/*                 EXE flattening program                       */
/*                                                              */
/*                      Copyright (c) 2001                      */
/*                      Bart E. Oldeman                         */
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

/*

Usage: exeflat (src.exe) (dest.sys) (relocation-factor)

large portions copied from task.c

*/

#include "portab.h"
#include "exe.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFSIZE 32768u

/* JPP - changed so will accept hex number. */
COUNT our_atoi(REG BYTE * pszString)
{
  BYTE Base = 10;
  BOOL Sign = FALSE;
  COUNT pnNum = 0;
  UCOUNT digit;
  if (*pszString == '-')

  {
    pszString++;
    Sign = TRUE;
  }
  if (pszString[0] == '0' && toupper(pszString[1]) == 'X')

  {
    Base = 16;
    pszString += 2;
  }
  for (; isxdigit(*pszString); pszString++)

  {
    if (isdigit(pszString[0]))
      digit = pszString[0] - '0';

    else
      digit = toupper(pszString[0]) - 'A' + 10;
    if (digit > Base)

    {
      printf("illegal digit '%s' in offset\n", pszString);
      exit(1);
    }
    pnNum = pnNum * Base + digit;
  }
  if (Sign)
    pnNum = -pnNum;
  return pnNum;
}
typedef struct {
  UWORD off, seg;
} farptr;
int compReloc(const void *p1, const void *p2)
{
  farptr *r1 = (farptr *) p1;
  farptr *r2 = (farptr *) p2;
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
int main(int argc, char **argv)
{
  exe_header header;
  int i;
  size_t bufsize;
  farptr *reloc;
  UWORD start_seg;
  ULONG size, to_xfer;
  UBYTE **buffers;
  UBYTE **curbuf;
  FILE *src, *dest;
  if (argc != 4)

  {
    printf("usage: exeflat (src.exe) (dest.sys) (relocation-factor)\n");
    return 1;
  }
  if ((src = fopen(argv[1], "rb")) == NULL)

  {
    printf("Source file %s could not be opened\n", argv[1]);
    return 1;
  }
  if (fread(&header, sizeof(header), 1, src) != 1)

  {
    printf("Error reading header from %s\n", argv[1]);
    fclose(src);
    return 1;
  }
  if (header.exSignature != MAGIC)

  {
    printf("Source file %s is not a valid .EXE\n", argv[1]);
    fclose(src);
    return 1;
  }
  if ((dest = fopen(argv[2], "wb+")) == NULL)

  {
    printf("Destination file %s could not be created\n", argv[2]);
    return 1;
  }
  start_seg = our_atoi(argv[3]);
  printf("header len = %ld = 0x%lx\n", header.exHeaderSize * 16UL,
         header.exHeaderSize * 16UL);
  fseek(src, 0, SEEK_END);
  size = ftell(src) - header.exHeaderSize * 16UL;
  printf("image size (less header) = %ld = 0x%lx\n", size, size);
  printf("first relocation offset = 0 = 0x0\n");

  /* first read file into memory chunks */
  fseek(src, header.exHeaderSize * 16UL, SEEK_SET);
  buffers = malloc((size + BUFSIZE - 1) / BUFSIZE * sizeof(char *));
  if (buffers == NULL)

  {
    printf("Allocation error\n");
    return 1;
  }
  bufsize = BUFSIZE;
  for (to_xfer = size, curbuf = buffers; to_xfer > 0;
       to_xfer -= bufsize, curbuf++)

  {
    if (to_xfer < BUFSIZE)
      bufsize = to_xfer;
    *curbuf = malloc(bufsize);
    if (*curbuf == NULL)

    {
      printf("Allocation error\n");
      return 1;
    }
    if (fread(*curbuf, sizeof(char), bufsize, src) != bufsize)

    {
      printf("Source file read error %ld %d\n", to_xfer, bufsize);
      return 1;
    }
  }
  fseek(src, header.exRelocTable, SEEK_SET);
  reloc = malloc(header.exRelocItems * sizeof(farptr));
  if (reloc == NULL)

  {
    printf("Allocation error\n");
    return 1;
  }
  if (fread(reloc, sizeof(farptr), header.exRelocItems, src) !=
      header.exRelocItems)

  {
    printf("Source file read error\n");
    return 1;
  }
  fclose(src);
  qsort(reloc, header.exRelocItems, sizeof(reloc[0]), compReloc);
  for (i = 0; i < header.exRelocItems; i++)

  {
    ULONG spot = ((ULONG) reloc[i].seg << 4) + reloc[i].off;
    UBYTE *spot0 = &buffers[spot / BUFSIZE][spot % BUFSIZE];
    UBYTE *spot1 = &buffers[(spot + 1) / BUFSIZE][(spot + 1) % BUFSIZE];
    UWORD segment = ((UWORD) * spot1 << 8) + *spot0;
    printf("relocation at 0x%04x:0x%04x ->%04x\n", reloc[i].seg,
           reloc[i].off, segment);
    segment += start_seg;
    *spot0 = segment & 0xff;
    *spot1 = segment >> 8;
  }

  /* write dest file from memory chunks */
  bufsize = BUFSIZE;
  for (to_xfer = size, curbuf = buffers; to_xfer > 0;
       to_xfer -= bufsize, curbuf++)

  {
    if (to_xfer < BUFSIZE)
      bufsize = to_xfer;
    if (fwrite(*curbuf, sizeof(char), bufsize, dest) != bufsize)

    {
      printf("Destination file write error\n");
      return 1;
    }
  }
  fclose(dest);
  printf("\nProcessed %d relocations\n", header.exRelocItems);
  return 0;
}
