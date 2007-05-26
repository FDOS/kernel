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

/* history

	10/??/01 - Bart Oldeman 
		primary release

	11/28/01 - tom ehlert 	
		added -UPX option to make the kernel compressable with UPX

*/

#include "portab.h"
#include "exe.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFSIZE 32768u

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

void usage(void)
{
  printf("usage: exeflat (src.exe) (dest.sys) (relocation-factor)\n");
  printf
      ("               -S10   - Silent relocate segment 10 (down list)\n");
  
  exit(1);
}

int main(int argc, char **argv)
{
  exe_header header;
  int i, j;
  size_t bufsize;
  farptr *reloc;
  UWORD start_seg;
  ULONG size, to_xfer;
  UBYTE **buffers;
  UBYTE **curbuf;
  FILE *src, *dest;
  short silentSegments[20], silentcount = 0, silentdone = 0;
  int UPX = FALSE, flat_exe = FALSE;

  /* if no arguments provided, show usage and exit */
  if (argc < 4) usage();

  /* do optional argument processing here */
  for (i = 4; i < argc; i++)
  {
    char *argptr = argv[i];
    
    if (argptr[0] != '-' && argptr[0] != '/')
      usage();
    
    argptr++;

    switch (toupper(argptr[0]))
    {
      case 'U':
        UPX = TRUE;
        break;
      case 'E':
        flat_exe = TRUE;
        break;
      case 'S':
        if (silentcount >= LENGTH(silentSegments))
        {
          printf("can't handle more then %d silent's\n",
                 LENGTH(silentSegments));
          exit(1);
        }
	
        silentSegments[silentcount++] = (short)strtol(argptr + 1, NULL, 0);
        break;
	
      default:
        usage();
    }
  }

  /* arguments left :
     infile outfile relocation offset */
  
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
  start_seg = (UWORD)strtol(argv[3], NULL, 0);
  if (header.exExtraBytes == 0)
    header.exExtraBytes = 0x200;
  printf("header len = %lu = 0x%lx\n", header.exHeaderSize * 16UL,
         header.exHeaderSize * 16UL);
  size =
      ((DWORD) (header.exPages - 1) << 9) + header.exExtraBytes -
      header.exHeaderSize * 16UL;
  printf("image size (less header) = %lu = 0x%lx\n", size, size);
  printf("first relocation offset = %u = 0x%u\n", header.exOverlay,
         header.exOverlay);

  /* first read file into memory chunks */
  fseek(src, header.exHeaderSize * 16UL, SEEK_SET);
  buffers = malloc((size_t)((size + BUFSIZE - 1) / BUFSIZE) * sizeof(char *));
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
      bufsize = (size_t)to_xfer;
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
    UBYTE *spot0 = &buffers[(size_t)(spot / BUFSIZE)][(size_t)(spot % BUFSIZE)];
    UBYTE *spot1 = &buffers[(size_t)((spot + 1) / BUFSIZE)][(size_t)((spot + 1) % BUFSIZE)];
    UWORD segment = ((UWORD) * spot1 << 8) + *spot0;

    for (j = 0; j < silentcount; j++)
      if (segment == silentSegments[j])
      {
        silentdone++;
        goto dontPrint;
      }
    
    printf("relocation at 0x%04x:0x%04x ->%04x\n", reloc[i].seg,
           reloc[i].off, segment);
    
  dontPrint:
    
    segment += start_seg;
    *spot0 = segment & 0xff;
    *spot1 = segment >> 8;
  }
  
  if (UPX)
  {
    /* UPX HEADER jump $+2+size */
    static char JumpBehindCode[] = {
      /* kernel config header - 32 bytes */
      0xeb, 0x1b,               /*     jmp short realentry */
      'C', 'O', 'N', 'F', 'I', 'G', 32 - 2 - 6 - 2 - 3, 0,      /* WORD */
      0,                        /* DLASortByDriveNo            db 0  */
      1,                        /* InitDiskShowDriveAssignment db 1  */
      2,                        /* SkipConfigSeconds           db 2  */
      0,                        /* ForceLBA                    db 0  */
      1,                        /* GlobalEnableLBAsupport      db 1  */
      0,                        /* BootHarddiskSeconds               */
      
      'n', 'u', 's', 'e', 'd',     /* unused filler bytes                              */
      8, 7, 6, 5, 4, 3, 2, 1,
      /* real-entry: jump over the 'real' image do the trailer */
      0xe9, 0, 0                /* 100: jmp 103 */
    };
    
    struct x {
      char y[sizeof(JumpBehindCode) == 0x20 ? 1 : -1];
    };
    
    if (size >= 0xfe00u)
    {
      printf("kernel; size too large - must be <= 0xfe00\n");
      exit(1);
    }

    /* this assumes <= 0xfe00 code in kernel */
    *(short *)&JumpBehindCode[0x1e] += (short)size;
    fwrite(JumpBehindCode, 1, 0x20, dest);
  }

  if (flat_exe) {
    /* write header without relocations to file */
    exe_header nheader = header;
    nheader.exRelocItems = 0;
    nheader.exHeaderSize = 2;
    size += 32;
    nheader.exPages = (UWORD)(size >> 9);
    nheader.exExtraBytes = (UWORD)size & 511;
    if (nheader.exExtraBytes)
      nheader.exPages++;
    if (fwrite(&nheader, sizeof(nheader), 1, dest) != 1) {
      printf("Destination file write error\n");
      exit(1);
    }
    fseek(dest, 32UL, SEEK_SET);
  }

  /* write dest file from memory chunks */
  bufsize = BUFSIZE;
  for (to_xfer = size, curbuf = buffers; to_xfer > 0;
       to_xfer -= bufsize, curbuf++)
  {
    if (to_xfer < BUFSIZE)
      bufsize = (size_t)to_xfer;
    if (fwrite(*curbuf, sizeof(char), bufsize, dest) != bufsize)

    {
      printf("Destination file write error\n");
      return 1;
    }
  }
 
  if (UPX)
  {
    /* UPX trailer */
    /* hand assembled - so this remains ANSI C ;-)    */
    /* move kernel down to place CONFIG-block, which added above,
       at start_seg-2:0 (e.g. 0x5e:0) instead of 
       start_seg:0 (e.g. 0x60:0) and store there boot drive number
       from BL; kernel.asm will then check presence of additional
       CONFIG-block at this address. */
    static char trailer[] = {   /* shift down everything by sizeof JumpBehindCode */
      0xB9, 0x00, 0x00,         /*  0 mov cx,offset trailer     */
      0x0E,                     /*  3 push cs                   */
      0x1F,                     /*  4 pop ds (=60)              */
      0x8C, 0xC8,               /*  5 mov ax,cs                 */
      0x48,                     /*  7 dec ax                    */
      0x48,                     /*  8 dec ax                    */
      0x8E, 0xC0,               /*  9 mov es,ax                 */
      0x93,                     /* 11 xchg ax,bx (to get al=bl) */
      0x31, 0xFF,               /* 12 xor di,di                 */
      0xFC,                     /* 14 cld                       */
      0xAA,                     /* 15 stosb (store drive number)*/
      0x8B, 0xF7,               /* 16 mov si,di                 */
      0xF3, 0xA4,               /* 18 rep movsb                 */
      0x1E,                     /* 20 push ds                   */
      0x58,                     /* 21 pop  ax                   */
      0x05, 0x00, 0x00,         /* 22 add ax,...                */
      0x8E, 0xD0,               /* 25 mov ss,ax                 */
      0xBC, 0x00, 0x00,         /* 27 mov sp,...                */
      0x31, 0xC0,               /* 30 xor ax,ax                 */
      0xFF, 0xE0                /* 32 jmp ax                    */
    };

    *(short *)&trailer[1] = (short)size + 0x20;
    *(short *)&trailer[23] = header.exInitSS;
    *(short *)&trailer[28] = header.exInitSP;
    fwrite(trailer, 1, sizeof trailer, dest);
  }
  fclose(dest);
  printf("\nProcessed %d relocations, %d not shown\n",
         header.exRelocItems, silentdone);
  return 0;
}
