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
#define KERNEL_CONFIG_LENGTH (32 - 2 - 4)
	/* 32 entrypoint structure,
	   2 entrypoint short jump,
	   4 near jump / ss:sp storage  */
unsigned char kernel_config[KERNEL_CONFIG_LENGTH];

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
                   int UPX, UWORD entryparagraphs, exe_header *header)
{
  int i, j;
  size_t bufsize;
  farptr *reloc;
  UWORD start_seg;
  ULONG size, to_xfer;
  UBYTE **buffers;
  UBYTE **curbuf;
  UWORD curbufoffset;
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
  start_seg += entryparagraphs;
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

  if (UPX)
  {
    struct x {
      char y[(KERNEL_CONFIG_LENGTH + 2) <= BUFSIZE ? 1 : -1];
    };
    memcpy(kernel_config, &buffers[0][2], KERNEL_CONFIG_LENGTH);
  }

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
  compress_sys_file = (size - (0xC0 - 0x10)) < 65420;
  if (UPX) {
      printf("Compressing kernel - %s format\n", (compress_sys_file)?"sys":"exe");
   if (!compress_sys_file) {
    ULONG realsize;
    /* write header without relocations to file */
    exe_header nheader = *header;
    nheader.exRelocItems = 0;
    nheader.exHeaderSize = 2;
    realsize = size + 32 - 0xC0;
    nheader.exPages = (UWORD)(realsize >> 9);
    nheader.exExtraBytes = (UWORD)realsize & 511;
    if (nheader.exExtraBytes)
      nheader.exPages++;
    nheader.exInitCS = -0xC;
    nheader.exInitIP = 0xC0;
    if (fwrite(&nheader, sizeof(nheader), 1, dest) != 1) {
      printf("Destination file write error\n");
      exit(1);
    }
    fseek(dest, 32UL, SEEK_SET);
   } else {
    printf("DOS/SYS format for UPX not yet supported.\n");
    exit(1);
   }
  }

  /* write dest file from memory chunks */
  {
    struct x {
      char y[0xC0 < BUFSIZE ? 1 : -1];
    };
  }
  if (UPX) {
    curbufoffset = 0xC0;
    bufsize = BUFSIZE - 0xC0;
    to_xfer = size - 0xC0;
  } else {
    curbufoffset = 0;
    bufsize = BUFSIZE;
    to_xfer = size;
  }
  for (curbuf = buffers; to_xfer > 0;
       to_xfer -= bufsize, curbuf++, curbufoffset = 0, bufsize = BUFSIZE)
  {
    if (to_xfer < bufsize)
      bufsize = (size_t)to_xfer;
    if (fwrite(&(*curbuf)[curbufoffset], sizeof(char), bufsize, dest) != bufsize)
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

static void write_header(FILE *dest, char const * entryfilename,
  exe_header *header)
{
  UWORD stackpointerpatch, stacksegmentpatch, psppatch, csippatch, patchvalue;
  UWORD end;
  static unsigned char code[256 + 10 + 1];
  FILE * entryf = fopen(entryfilename, "rb");
  if (!entryf) {
    printf("Cannot open entry file\n");
    exit(1);
  }
  if (fread(code, 1, 256 + 10 + 1, entryf) != 256 + 10) {
    printf("Invalid entry file length\n");
    exit(1);
  }

  stackpointerpatch = code[0x100] + code[0x100 + 1] * 256U;
  stacksegmentpatch = code[0x102] + code[0x102 + 1] * 256U;
  psppatch = code[0x104] + code[0x104 + 1] * 256U;
  csippatch = code[0x106] + code[0x106 + 1] * 256U;
  end = code[0x108] + code[0x108 + 1] * 256U;
  if (stackpointerpatch > (0xC0 - 2) || stackpointerpatch < 32
      || stacksegmentpatch > (0xC0 - 2) || stacksegmentpatch < 32
      || psppatch > (0xC0 - 2) || psppatch < 32
      || csippatch > (0xC0 - 4) || csippatch < 32
      || end > 0xC0 || end < 32) {
    printf("Invalid entry file patch offsets\n");
    exit(1);
  }

  patchvalue = code[stackpointerpatch] + code[stackpointerpatch + 1] * 256U;
  patchvalue += header->exInitSP;
  code[stackpointerpatch] = patchvalue & 0xFF;
  code[stackpointerpatch + 1] = (patchvalue >> 8) & 0xFF;
  patchvalue = code[stacksegmentpatch] + code[stacksegmentpatch + 1] * 256U;
  patchvalue += header->exInitSS + 0x6C;
  code[stacksegmentpatch] = patchvalue & 0xFF;
  code[stacksegmentpatch + 1] = (patchvalue >> 8) & 0xFF;
  patchvalue = code[psppatch] + code[psppatch + 1] * 256U;
  patchvalue += 0x6C;
  code[psppatch] = patchvalue & 0xFF;
  code[psppatch + 1] = (patchvalue >> 8) & 0xFF;
  patchvalue = header->exInitIP;
  code[csippatch] = patchvalue & 0xFF;
  code[csippatch + 1] = (patchvalue >> 8) & 0xFF;
  patchvalue = header->exInitCS + 0x6C;
  code[csippatch + 2] = patchvalue & 0xFF;
  code[csippatch + 3] = (patchvalue >> 8) & 0xFF;

  if (0 == memcmp(kernel_config, "CONFIG", 6)) {
    unsigned long length = kernel_config[6] + kernel_config[7] * 256UL + 8;
    if (length <= KERNEL_CONFIG_LENGTH) {
      memcpy(&code[2], kernel_config, length);
      printf("Copied %lu bytes of kernel config block to header\n", length);
    } else {
      printf("Error: Found %lu bytes of kernel config block, too long!\n", length);
    }
  } else {
    printf("Error: Found no kernel config block!\n");
  }

  fseek(dest, 0, SEEK_SET);
  if (fwrite(code, 1, 0xC0, dest) != 0xC0) {
    printf("Error writing header code to output file\n");
    exit(1);
  }
}

int main(int argc, char **argv)
{
  short silentSegments[20], silentcount = 0;
  static exe_header header; /* must be initialized to zero */
  int UPX = FALSE;
  int i;
  size_t sz, len, len2, n;
  int compress_sys_file;
  char *buffer, *tmpexe, *cmdbuf, *entryfilename = "";
  FILE *dest, *source;
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
      case 'E':
        entryfilename = &argptr[1];
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
                              UPX, 0, &header);
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
            FALSE, 0xC, &header);
  }

  /* argv[2] now contains the final flattened file: just
     header and trailer need to be added */
  /* the compressed file has two chunks max */

  rename(argv[2], "tmp.bin");
  if ((dest = fopen(argv[2], "wb")) == NULL)
  {
    printf("Destination file %s could not be opened\n", argv[2]);
    exit(1);
  }
  if ((source = fopen("tmp.bin", "rb")) == NULL)
  {
    printf("Source file %s could not be opened\n", "tmp.bin");
    exit(1);
  }

  buffer = malloc(32 * 1024);
  if (!buffer)
  {
    printf("Memory allocation failure\n");
    exit(1);
  }

  write_header(dest, entryfilename, &header);
  do {
    size = fread(buffer, 1, 32 * 1024, source);
    if (fwrite(buffer, 1, size, dest) != size) {
      printf("Write failure\n");
      exit(1);
    }
  } while (size);

  remove("tmp.bin");

  return 0;
}
