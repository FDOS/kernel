/***************************************************************

                                    sys.c
                                    DOS-C

                            sys utility for DOS-C

                             Copyright (c) 1991
                             Pasquale J. Villani
                             All Rights Reserved

 This file is part of DOS-C.

 DOS-C is free software; you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2, or (at your option) any later version.

 DOS-C is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 DOS-C; see the file COPYING.  If not, write to the Free Software Foundation,
 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************/

/* $Log$
 * Revision 1.3  2000/05/25 20:56:23  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/15 05:28:09  jimtabor
 * Cleanup CRs
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.10  2000/03/31 06:59:10  jprice
 * Added discription of program.
 *
 * Revision 1.9  1999/09/20 18:34:40  jprice
 * *** empty log message ***
 *
 * Revision 1.8  1999/09/20 18:27:19  jprice
 * Changed open/creat to fopen to make TC2 happy.
 *
 * Revision 1.7  1999/09/15 05:39:02  jprice
 * Changed boot sector writing code so easier to read.
 *
 * Revision 1.6  1999/09/14 17:30:44  jprice
 * Added debug log creation to sys.com.
 *
 * Revision 1.5  1999/08/25 03:19:51  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.4  1999/04/17 19:14:44  jprice
 * Fixed multi-sector code
 *
 * Revision 1.3  1999/04/01 07:24:05  jprice
 * SYS modified for new boot loader
 *
 * Revision 1.2  1999/03/29 16:24:48  jprice
 * Fixed error message
 *
 * Revision 1.1.1.1  1999/03/29 15:43:15  jprice
 * New version without IPL.SYS
 * Revision 1.3  1999/01/21 04:35:21  jprice Fixed comments.
 *   Added indent program
 *
 * Revision 1.2  1999/01/21 04:13:52  jprice Added messages to sys.  Also made
 *   it create a .COM file.
 *
 */

#define STORE_BOOT_INFO

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
/*#include <sys/stat.h>*/
#include <io.h>
#include <dos.h>
#include <ctype.h>
#include <mem.h>
#include "portab.h"
#include "device.h"

#include "b_fat12.h"
#include "b_fat16.h"

BYTE *pgm = "sys";

BOOL fl_reset(WORD);
COUNT fl_rd_status(WORD);
COUNT fl_read(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_write(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_verify(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_format(WORD, BYTE FAR *);
VOID get_boot(COUNT);
VOID put_boot(COUNT);
BOOL check_space(COUNT, BYTE *);
COUNT ltop(WORD *, WORD *, WORD *, COUNT, COUNT, LONG, byteptr);
BOOL copy(COUNT, BYTE *);
BOOL DiskReset(COUNT);
COUNT DiskRead(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT DiskWrite(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);

/* special word packing prototypes        */

#ifdef NATIVE
#define getlong(vp, lp) (*(LONG *)(lp)=*(LONG *)(vp))
#define getword(vp, wp) (*(WORD *)(wp)=*(WORD *)(vp))
#define getbyte(vp, bp) (*(BYTE *)(bp)=*(BYTE *)(vp))
#define fgetlong(vp, lp) (*(LONG FAR *)(lp)=*(LONG FAR *)(vp))
#define fgetword(vp, wp) (*(WORD FAR *)(wp)=*(WORD FAR *)(vp))
#define fgetbyte(vp, bp) (*(BYTE FAR *)(bp)=*(BYTE FAR *)(vp))
#define fputlong(lp, vp) (*(LONG FAR *)(vp)=*(LONG FAR *)(lp))
#define fputword(wp, vp) (*(WORD FAR *)(vp)=*(WORD FAR *)(wp))
#define fputbyte(bp, vp) (*(BYTE FAR *)(vp)=*(BYTE FAR *)(bp))
#else
VOID getword(VOID *, WORD *);
VOID getbyte(VOID *, BYTE *);
VOID fgetlong(VOID FAR *, LONG FAR *);
VOID fgetword(VOID FAR *, WORD FAR *);
VOID fgetbyte(VOID FAR *, BYTE FAR *);
VOID fputlong(LONG FAR *, VOID FAR *);
VOID fputword(WORD FAR *, VOID FAR *);
VOID fputbyte(BYTE FAR *, VOID FAR *);
#endif

#define SEC_SIZE        512
#define NDEV            4
#define COPY_SIZE       32768
#define NRETRY          5

static struct media_info
{
  ULONG mi_size;                /* physical sector size         */
  UWORD mi_heads;               /* number of heads (sides)      */
  UWORD mi_cyls;                /* number of cyl/drive          */
  UWORD mi_sectors;             /* number of sectors/cyl        */
  ULONG mi_offset;              /* relative partition offset    */
};

union
{
  BYTE bytes[2 * SEC_SIZE];
  boot boot_sector;
}

  buffer;

static struct media_info miarray[NDEV] =
{
  {720l, 2, 40, 9, 0l},
  {720l, 2, 40, 9, 0l},
  {720l, 2, 40, 9, 0l},
  {720l, 2, 40, 9, 0l}
};

#define PARTOFF 0x1be
#define N_PART 4

static struct
{
  BYTE peBootable;
  BYTE peBeginHead;
  BYTE peBeginSector;
  UWORD peBeginCylinder;
  BYTE peFileSystem;
  BYTE peEndHead;
  BYTE peEndSector;
  UWORD peEndCylinder;
  LONG peStartSector;
  LONG peSectors;
} partition[N_PART];

struct bootsectortype
{
  UBYTE bsJump[3];
  char OemName[8];
  UWORD bsBytesPerSec;
  UBYTE bsSecPerClust;
  UWORD bsResSectors;
  UBYTE bsFATs;
  UWORD bsRootDirEnts;
  UWORD bsSectors;
  UBYTE bsMedia;
  UWORD bsFATsecs;
  UWORD bsSecPerTrack;
  UWORD bsHeads;
  ULONG bsHiddenSecs;
  ULONG bsHugeSectors;
  UBYTE bsDriveNumber;
  UBYTE bsReserved1;
  UBYTE bsBootSignature;
  ULONG bsVolumeID;
  char bsVolumeLabel[11];
  char bsFileSysType[8];
  char unused[2];
  UWORD sysRootDirSecs;         /* of sectors root dir uses */
  ULONG sysFatStart;            /* first FAT sector */
  ULONG sysRootDirStart;        /* first root directory sector */
  ULONG sysDataStart;           /* first data sector */
};


static int DrvMap[4] =
{0, 1, 0x80, 0x81};

COUNT drive, active;
UBYTE newboot[SEC_SIZE], oldboot[SEC_SIZE];

#ifdef DEBUG
FILE *log;
#endif

#define SBSIZE          51
#define SBOFFSET        11

#define SIZEOF_PARTENT  16

#define FAT12           0x01
#define FAT16SMALL      0x04
#define EXTENDED        0x05
#define FAT16LARGE      0x06

#define N_RETRY         5

COUNT get_part(COUNT drive, COUNT idx)
{
  REG retry = N_RETRY;
  REG BYTE *p = (BYTE *) & buffer.bytes[PARTOFF + (idx * SIZEOF_PARTENT)];
  REG ret;
  BYTE packed_byte, pb1;

  do
  {
    ret = fl_read((WORD) DrvMap[drive], (WORD) 0, (WORD) 0, (WORD) 1, (WORD) 1, (byteptr) & buffer);
  }
  while (ret != 0 && --retry > 0);
  if (ret != 0)
    return FALSE;
  getbyte((VOID *) p, &partition[idx].peBootable);
  ++p;
  getbyte((VOID *) p, &partition[idx].peBeginHead);
  ++p;
  getbyte((VOID *) p, &packed_byte);
  partition[idx].peBeginSector = packed_byte & 0x3f;
  ++p;
  getbyte((VOID *) p, &pb1);
  ++p;
  partition[idx].peBeginCylinder = pb1 + ((0xc0 & packed_byte) << 2);
  getbyte((VOID *) p, &partition[idx].peFileSystem);
  ++p;
  getbyte((VOID *) p, &partition[idx].peEndHead);
  ++p;
  getbyte((VOID *) p, &packed_byte);
  partition[idx].peEndSector = packed_byte & 0x3f;
  ++p;
  getbyte((VOID *) p, &pb1);
  ++p;
  partition[idx].peEndCylinder = pb1 + ((0xc0 & packed_byte) << 2);
  getlong((VOID *) p, &partition[idx].peStartSector);
  p += sizeof(LONG);
  getlong((VOID *) p, &partition[idx].peSectors);
  return TRUE;
}

VOID main(COUNT argc, char **argv)
{
	printf("FreeDOS System Installer v1.0\n\n");

	if (argc != 2)
  {
    fprintf(stderr, "Usage: %s drive\n drive = A,B,etc.\n", pgm);
    exit(1);
  }

  drive = *argv[1] - (islower(*argv[1]) ? 'a' : 'A');
  if (drive < 0 || drive >= NDEV)
  {
    fprintf(stderr, "%s: drive out of range\n", pgm);
    exit(1);
  }

  if (!DiskReset(drive))
  {
    fprintf(stderr, "%s: cannot reset drive %c:",
            drive, 'A' + drive);
    exit(1);
  }

  get_boot(drive);

  if (!check_space(drive, oldboot))
  {
    fprintf(stderr, "%s: Not enough space to transfer system files\n", pgm);
    exit(1);
  }

#ifdef DEBUG
  if ((log = fopen("sys.log", "w")) == NULL)
  {
    printf("Can't write log file.\n");
    log = NULL;
  }
#endif

  printf("Writing boot sector...\n");
  put_boot(drive);

  printf("\nCopying KERNEL.SYS...");
  if (!copy(drive, "kernel.sys"))
  {
    fprintf(stderr, "\n%s: cannot copy \"KERNEL.SYS\"\n", pgm);
#ifdef DEBUG
    fclose(log);
#endif
    exit(1);
  }

  printf("\nCopying COMMAND.COM...");
  if (!copy(drive, "command.com"))
  {
    fprintf(stderr, "\n%s: cannot copy \"COMMAND.COM\"\n", pgm);
#ifdef DEBUG
    fclose(log);
#endif
    exit(1);
  }
  printf("\nSystem transfered.\n");
#ifdef DEBUG
  fclose(log);
#endif
  exit(0);
}

#ifdef DEBUG
VOID dump_sector(unsigned char far * sec)
{
  if (log)
  {
    COUNT x, y;
    char c;

    for (x = 0; x < 32; x++)
    {
      fprintf(log, "%03X  ", x * 16);
      for (y = 0; y < 16; y++)
      {
        fprintf(log, "%02X ", sec[x * 16 + y]);
      }
      for (y = 0; y < 16; y++)
      {
        c = oldboot[x * 16 + y];
        if (isprint(c))
          fprintf(log, "%c", c);
        else
          fprintf(log, ".");
      }
      fprintf(log, "\n");
    }
    fprintf(log, "\n");
  }
}

#endif


VOID put_boot(COUNT drive)
{
  COUNT i, z;
  WORD head, track, sector, ret;
  WORD count;
  ULONG temp;
  struct bootsectortype *bs;

  if (drive >= 2)
  {
    head = partition[active].peBeginHead;
    sector = partition[active].peBeginSector;
    track = partition[active].peBeginCylinder;
  }
  else
  {
    head = 0;
    sector = 1;
    track = 0;
  }

  /* Read current boot sector */
  if ((i = DiskRead(DrvMap[drive], head, track, sector, 1, (BYTE far *) oldboot)) != 0)
  {
    fprintf(stderr, "%s: disk read error (code = 0x%02x)\n", pgm, i & 0xff);
    exit(1);
  }

#ifdef DEBUG
  fprintf(log, "Old Boot Sector:\n");
  dump_sector(oldboot);
#endif

  bs = (struct bootsectortype *) & oldboot;
  if ((bs->bsFileSysType[4] == '6') && (bs->bsBootSignature == 0x29))
  {
    memcpy(newboot, b_fat16, SEC_SIZE); /* copy FAT16 boot sector */
    printf("FAT type: FAT16\n");
#ifdef DEBUG
    fprintf(log, "FAT type: FAT16\n");
#endif
  }
  else
  {
    memcpy(newboot, b_fat12, SEC_SIZE); /* copy FAT12 boot sector */
    printf("FAT type: FAT12\n");
#ifdef DEBUG
    fprintf(log, "FAT type: FAT12\n");
#endif
  }

  /* Copy disk parameter from old sector to new sector */
  memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE);

  bs = (struct bootsectortype *) & newboot;
  /* root directory sectors */
#ifdef STORE_BOOT_INFO
  bs->sysRootDirSecs = bs->bsRootDirEnts / 16;
#endif
#ifdef DEBUG
  fprintf(log, "root dir entries = %u\n", bs->bsRootDirEnts);
  fprintf(log, "root dir sectors = %u\n", bs->sysRootDirSecs);
#endif

  /* sector FAT starts on */
  temp = bs->bsHiddenSecs + bs->bsResSectors;
#ifdef STORE_BOOT_INFO
  bs->sysFatStart = temp;
#endif
#ifdef DEBUG
  fprintf(log, "FAT starts at sector %lu = (%lu + %u)\n", temp,
          bs->bsHiddenSecs, bs->bsResSectors);
#endif

  /* sector root directory starts on */
  temp = temp + bs->bsFATsecs * bs->bsFATs;
#ifdef STORE_BOOT_INFO
  bs->sysRootDirStart = temp;
#endif
#ifdef DEBUG
  fprintf(log, "Root directory starts at sector %lu = (PREVIOUS + %u * %u)\n",
          temp, bs->bsFATsecs, bs->bsFATs);
#endif

  /* sector data starts on */
  temp = temp + bs->sysRootDirSecs;
#ifdef STORE_BOOT_INFO
  bs->sysDataStart = temp;
#endif
#ifdef DEBUG
  fprintf(log, "DATA starts at sector %lu = (PREVIOUS + %u)\n", temp,
          bs->sysRootDirSecs);
#endif


#ifdef DEBUG
  fprintf(log, "\nNew Boot Sector:\n");
  dump_sector(newboot);
#endif

  if ((i = DiskWrite(DrvMap[drive], head, track, sector, 1, (BYTE far *) newboot)) != 0)
  {
    fprintf(stderr, "%s: disk write error (code = 0x%02x)\n", pgm, i & 0xff);
    exit(1);
  }
}

VOID get_boot(COUNT drive)
{
  COUNT i;
  COUNT ifd;
  WORD head, track, sector, ret;
  WORD count;

  if (drive >= 2)
  {
    head = partition[active].peBeginHead;
    sector = partition[active].peBeginSector;
    track = partition[active].peBeginCylinder;
  }
  else
  {
    head = 0;
    sector = 1;
    track = 0;
  }

  if ((i = DiskRead(DrvMap[drive], head, track, sector, 1, (BYTE far *) oldboot)) != 0)
  {
    fprintf(stderr, "%s: disk read error (code = 0x%02x)\n", pgm, i & 0xff);
    exit(1);
  }
}

BOOL check_space(COUNT drive, BYTE * BlkBuffer)
{
  BYTE *bpbp;
  BYTE nfat;
  UWORD nfsect;
  ULONG hidden;
  ULONG count;
  ULONG block;
  UBYTE nreserved;
  UCOUNT i;
  WORD track, head, sector;
  UBYTE buffer[SEC_SIZE];
  ULONG bpb_huge;
  UWORD bpb_nsize;

  /* get local information                                */
  getbyte((VOID *) & BlkBuffer[BT_BPB + BPB_NFAT], &nfat);
  getword((VOID *) & BlkBuffer[BT_BPB + BPB_NFSECT], &nfsect);
  getlong((VOID *) & BlkBuffer[BT_BPB + BPB_HIDDEN], &hidden);
  getbyte((VOID *) & BlkBuffer[BT_BPB + BPB_NRESERVED], &nreserved);

  getlong((VOID *) & BlkBuffer[BT_BPB + BPB_HUGE], &bpb_huge);
  getword((VOID *) & BlkBuffer[BT_BPB + BPB_NSIZE], &bpb_nsize);

  count = miarray[drive].mi_size = bpb_nsize == 0 ?
      bpb_huge : bpb_nsize;

  /* Fix media information for disk                       */
  getword((&(((BYTE *) & BlkBuffer[BT_BPB])[BPB_NHEADS])), &miarray[drive].mi_heads);
  head = miarray[drive].mi_heads;
  getword((&(((BYTE *) & BlkBuffer[BT_BPB])[BPB_NSECS])), &miarray[drive].mi_sectors);
  if (miarray[drive].mi_size == 0)
    getlong(&((((BYTE *) & BlkBuffer[BT_BPB])[BPB_HUGE])), &miarray[drive].mi_size);
  sector = miarray[drive].mi_sectors;
  if (head == 0 || sector == 0)
  {
    fprintf(stderr, "Drive initialization failure.\n");
    exit(1);
  }
  miarray[drive].mi_cyls = count / (head * sector);

  return 1;
}

/* */
/* Do logical block number to physical head/track/sector mapping        */
/* */
static COUNT ltop(trackp, sectorp, headp, unit, count, strt_sect, strt_addr)
WORD *trackp, *sectorp, *headp;
REG COUNT unit;
LONG strt_sect;
COUNT count;
byteptr strt_addr;
{
#ifdef I86
  ULONG ltemp;
#endif
  REG ls, ps;

#ifdef I86
  /* Adjust for segmented architecture                            */
  ltemp = (((ULONG) mk_segment(strt_addr) << 4) + mk_offset(strt_addr)) & 0xffff;
  /* Test for 64K boundary crossing and return count large        */
  /* enough not to exceed the threshold.                          */
  count = (((ltemp + SEC_SIZE * count) & 0xffff0000l) != 0l)
      ? (0xffffl - ltemp) / SEC_SIZE
      : count;
#endif

  *trackp = strt_sect / (miarray[unit].mi_heads * miarray[unit].mi_sectors);
  *sectorp = strt_sect % miarray[unit].mi_sectors + 1;
  *headp = (strt_sect % (miarray[unit].mi_sectors * miarray[unit].mi_heads))
      / miarray[unit].mi_sectors;
  if (((ls = *headp * miarray[unit].mi_sectors + *sectorp - 1) + count) >
      (ps = miarray[unit].mi_heads * miarray[unit].mi_sectors))
    count = ps - ls;
  return count;
}

BOOL copy(COUNT drive, BYTE * file)
{
  BYTE dest[64];
  COUNT ifd, ofd, ret;
  FILE *s, *d;
  BYTE buffer[COPY_SIZE];
  struct ftime ftime;

  sprintf(dest, "%c:\\%s", 'A' + drive, file);
  if ((s = fopen(file, "rb")) == NULL)
  {
    fprintf(stderr, "%s: \"%s\" not found\n", pgm, file);
    return FALSE;
  }
  _fmode = O_BINARY;
  if ((d = fopen(dest, "wb")) == NULL)
  {
    fclose(s);
    fprintf(stderr, "%s: can't create\"%s\"\n", pgm, dest);
    return FALSE;
  }

  while ((ret = fread(buffer, 1, COPY_SIZE, s)) != 0)
    fwrite(buffer, 1, ret, d);

  getftime(fileno(s), &ftime);
  setftime(fileno(d), &ftime);

  fclose(s);
  fclose(d);

  return TRUE;
}

BOOL DiskReset(COUNT Drive)
{
  REG COUNT idx;

  /* Reset the drives                                             */
  fl_reset(DrvMap[drive]);

  if (Drive >= 2 && Drive < NDEV)
  {
    COUNT RetCode;

    /* Retrieve all the partition information               */
    for (RetCode = TRUE, idx = 0; RetCode && (idx < N_PART); idx++)
      RetCode = get_part(Drive, idx);
    if (!RetCode)
      return FALSE;

    /* Search for the first DOS partition and start         */
    /* building the map for the hard drive                  */
    for (idx = 0; idx < N_PART; idx++)
    {
      if (partition[idx].peFileSystem == FAT12
          || partition[idx].peFileSystem == FAT16SMALL
          || partition[idx].peFileSystem == FAT16LARGE)
      {
        miarray[Drive].mi_offset
            = partition[idx].peStartSector;
        active = idx;
        break;
      }
    }
  }

  return TRUE;
}

COUNT DiskRead(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR * buffer)
{
  int nRetriesLeft;

  for (nRetriesLeft = NRETRY; nRetriesLeft > 0; --nRetriesLeft)
  {
    if (fl_read(drive, head, track, sector, count, buffer) == count)
      return count;
  }
  return 0;
}

COUNT DiskWrite(WORD drive, WORD head, WORD track, WORD sector, WORD count, BYTE FAR * buffer)
{
  int nRetriesLeft;

  for (nRetriesLeft = NRETRY; nRetriesLeft > 0; --nRetriesLeft)
  {
    if (fl_write(drive, head, track, sector, count, buffer) == count)
      return count;
  }
  return 0;
}
