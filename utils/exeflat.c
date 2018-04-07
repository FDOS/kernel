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
#include <string.h>
#include <ctype.h>

#define BUFSIZE 32768u

#define KERNEL_START 0x16 /* the kernel code really starts here at 60:16 */

typedef struct {
  UWORD off, seg;
} farptr;

static int compReloc(const void *p1, const void *p2)
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

static void usage(void)
{
  printf("usage: exeflat (src.exe) (dest.sys) (relocation-factor)\n");
  printf
      ("               -S10   - Silent relocate segment 10 (down list)\n");
  
  exit(1);
}

static int exeflat(const char *srcfile, const char *dstfile,
                   const char *start, short *silentSegments, short silentcount,
                   int UPX, exe_header *header)
{
  int i, j;
  size_t bufsize;
  farptr *reloc;
  UWORD start_seg;
  ULONG size, to_xfer;
  UBYTE **buffers;
  UBYTE **curbuf;
  FILE *src, *dest;
  short silentdone = 0;
  int compress_sys_file;
  UWORD realentry;

  if ((src = fopen(srcfile, "rb")) == NULL)
  {
    printf("Source file %s could not be opened\n", srcfile);
    exit(1);
  }
  if (fread(header, sizeof(*header), 1, src) != 1)
  {
    printf("Error reading header from %s\n", srcfile);
    fclose(src);
    exit(1);
  }
  if (header->exSignature != MAGIC)
  {
    printf("Source file %s is not a valid .EXE\n", srcfile);
    fclose(src);
    exit(1);
  }
  start_seg = (UWORD)strtol(start, NULL, 0);
  if (header->exExtraBytes == 0)
    header->exExtraBytes = 0x200;
  printf("header len = %lu = 0x%lx\n", header->exHeaderSize * 16UL,
         header->exHeaderSize * 16UL);
  size =
    ((DWORD) (header->exPages - 1) << 9) + header->exExtraBytes -
    header->exHeaderSize * 16UL;
  printf("image size (less header) = %lu = 0x%lx\n", size, size);
  printf("first relocation offset = %u = 0x%x\n", header->exRelocTable,
         header->exRelocTable);

  /* first read file into memory chunks */
  fseek(src, header->exHeaderSize * 16UL, SEEK_SET);
  buffers = malloc((size_t)((size + BUFSIZE - 1) / BUFSIZE) * sizeof(char *));
  if (buffers == NULL)
  {
    printf("Allocation error\n");
    exit(1);
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
      exit(1);
    }
    if (fread(*curbuf, sizeof(char), bufsize, src) != bufsize)
    {
      printf("Source file read error %ld %d\n", to_xfer, (int)bufsize);
      exit(1);
    }
  }
  if (header->exRelocTable && header->exRelocItems)
  {
    fseek(src, header->exRelocTable, SEEK_SET);
    reloc = malloc(header->exRelocItems * sizeof(farptr));
    if (reloc == NULL)
    {
      printf("Allocation error\n");
      exit(1);
    }
    if (fread(reloc, sizeof(farptr), header->exRelocItems, src) !=
        header->exRelocItems)
    {
      printf("Source file read error\n");
      exit(1);
    }
  }
  fclose(src);
  qsort(reloc, header->exRelocItems, sizeof(reloc[0]), compReloc);
  for (i = 0; i < header->exRelocItems; i++)
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

  printf("\nProcessed %d relocations, %d not shown\n",
         header->exRelocItems, silentdone);

  realentry = KERNEL_START;
  if (buffers[0][0] == 0xeb /* jmp short */)
  {
    realentry = buffers[0][1] + 2;
  }
  else if (buffers[0][0] == 0xe9 /* jmp near */)
  {
    realentry = ((UWORD)(buffers[0][2]) << 8) + buffers[0][1] + 3;
  }

  if ((dest = fopen(dstfile, "wb+")) == NULL)
  {
    printf("Destination file %s could not be created\n", dstfile);
    exit(1);
  }

  /* The biggest .sys file that UPX accepts seems to be 65419 bytes long */
  compress_sys_file = size < 65420;
  if (UPX) {
      printf("Compressing kernel - %s format\n", (compress_sys_file)?"sys":"exe");
  }
  if (UPX && !compress_sys_file) {
    ULONG realsize;
    /* write header without relocations to file */
    exe_header nheader = *header;
    nheader.exRelocItems = 0;
    nheader.exHeaderSize = 2;
    realsize = size + 32;
    nheader.exPages = (UWORD)(realsize >> 9);
    nheader.exExtraBytes = (UWORD)realsize & 511;
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
      exit(1);
    }
    free(*curbuf);
  }

  if (UPX && compress_sys_file) {
    /* overwrite first 8 bytes with SYS header */
    UWORD dhdr[4];
    fseek(dest, 0, SEEK_SET);
    for (i = 0; i < 3; i++)
      dhdr[i] = 0xffff;
    /* strategy will jump to us, interrupt never called */
    dhdr[3] = realentry; /* KERNEL_START; */
    fwrite(dhdr, sizeof(dhdr), 1, dest);
    printf("KERNEL_START = 0x%04x\n", realentry);
  }
  fclose(dest);
  return compress_sys_file;
}

static void write_header(FILE *dest, size_t size)
{
  /* UPX HEADER jump $+2+size */
  static char JumpBehindCode[] = {
    /* kernel config header - 32 bytes */
    0xeb, 0x1b,               /*     jmp short realentry */
    'C', 'O', 'N', 'F', 'I', 'G', 6, 0,      /* WORD */
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

  fseek(dest, 0, SEEK_SET);
  /* this assumes <= 0xfe00 code in kernel */
  *(short *)&JumpBehindCode[0x1e] += (short)size;
  fwrite(JumpBehindCode, 1, 0x20, dest);
}

static void write_trailer(FILE *dest, size_t size, int compress_sys_file,
  exe_header *header)
{
  /* UPX trailer */
  /* hand assembled - so this remains ANSI C ;-) */
  /* well almost: we still need packing and assume little endian ... */
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
  *(short *)&trailer[23] = header->exInitSS;
  *(short *)&trailer[28] = header->exInitSP;
  if (compress_sys_file) {
    /* replace by jmp word ptr [6]: ff 26 06 00
       (the .SYS strategy handler which will unpack) */
    *(long *)&trailer[30] = 0x000626ffL;
    /* set up a 4K stack for the UPX decompressor to work with */
    *(short *)&trailer[23] = 0x1000;
    *(short *)&trailer[28] = 0x1000;
  }
  fwrite(trailer, 1, sizeof trailer, dest);
}

int main(int argc, char **argv)
{
  short silentSegments[20], silentcount = 0;
  static exe_header header; /* must be initialized to zero */
  int UPX = FALSE;
  int i;
  size_t sz, len, len2, n;
  int compress_sys_file;
  char *buffer, *tmpexe, *cmdbuf;
  FILE *dest;
  long size;

  /* if no arguments provided, show usage and exit */
  if (argc < 4) usage();

  /* do optional argument processing here */
  for (i = 4; i < argc && !UPX; i++)
  {
    char *argptr = argv[i];
    
    if (argptr[0] != '-' && argptr[0] != '/')
      usage();
    
    argptr++;

    switch (toupper(argptr[0]))
    {
      case 'U':
        UPX = i;
        break;
      case 'S':
        if (silentcount >= LENGTH(silentSegments))
        {
          printf("can't handle more then %d silent's\n",
                 (int)LENGTH(silentSegments));
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

  compress_sys_file = exeflat(argv[1], argv[2], argv[3],
                              silentSegments, silentcount,
                              UPX, &header);
  if (!UPX)
    exit(0);

  /* move kernel.sys tmp.exe */
  tmpexe = argv[2];
  if (!compress_sys_file)
  {
    tmpexe = "tmp.exe";
    rename(argv[2], tmpexe);
  }

  len2 = strlen(tmpexe) + 1;
  sz = len2;
  if (sz < 256) sz = 256;
  cmdbuf = malloc(sz);
  len = 0;
  for (i = UPX+1; i < argc; i++)
  {
    n = strlen(argv[i]);
    if (len + len2 + n + 2 >= sz) {
      sz *= 2;
      cmdbuf = realloc(cmdbuf, sz);
    }
    if (i > UPX+1)
      cmdbuf[len++] = ' ';
    memcpy(cmdbuf + len, argv[i], n + 1);
    len += n;
  }
  cmdbuf[len++] = ' ';
  /* if tmpexe is tmpfile set above no quotes needed, if user needs quotes should add on cmd line */
  memcpy(cmdbuf + len, tmpexe, len2);
  cmdbuf[len + len2] = '\0';
  printf("%s\n", cmdbuf);
  if (system(cmdbuf))
  {
    printf("Problems executing %s\n", cmdbuf);
    printf("Removing [%s]\n", tmpexe);
    remove(tmpexe);
    exit(1);
  }
  free(cmdbuf);

  if (!compress_sys_file)
  {
    exeflat(tmpexe, argv[2], argv[3],
            silentSegments, silentcount,
            FALSE, &header);
    remove(tmpexe);
  }

  /* argv[2] now contains the final flattened file: just
     header and trailer need to be added */
  /* the compressed file has two chunks max */

  if ((dest = fopen(argv[2], "rb+")) == NULL)
  {
    printf("Destination file %s could not be opened\n", argv[2]);
    exit(1);
  }

  buffer = malloc(0xfe01);

  fread(buffer, 0xfe01, 1, dest);
  size = ftell(dest);
  if (size >= 0xfe00u)
  {
    printf("kernel; size too large - must be <= 0xfe00\n");
    exit(1);
  }
  fseek(dest, 0, SEEK_SET);
  write_header(dest, (size_t)size);
  fwrite(buffer, (size_t)size, 1, dest);
  write_trailer(dest, (size_t)size, compress_sys_file, &header);
  return 0;
}
