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

#define DEBUG
/* #define DDEBUG */

#define SYS_VERSION "v3.2"

#include <stdlib.h>
#include <dos.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef __TURBOC__
#include <mem.h>
#else
#include <memory.h>
#endif
#include <string.h>
#ifdef __TURBOC__
#include <dir.h>
#endif
#define SYS_MAXPATH   260
#include "portab.h"
#include "algnbyte.h"
#include "device.h"
#include "dcb.h"
#include "xstructs.h"
#include "date.h"
#include "../hdr/time.h"
#include "fat.h"

/* These definitions deliberately put here instead of
 * #including <stdio.h> to make executable MUCH smaller
 * using [s]printf from prf.c!
 */
extern int VA_CDECL printf(const char * fmt, ...);
extern int VA_CDECL sprintf(char * buff, const char * fmt, ...);

#include "fat12com.h"
#include "fat16com.h"
#ifdef WITHFAT32
#include "fat32chs.h"
#include "fat32lba.h"
#endif

#ifndef __WATCOMC__

#include <io.h>

#else

long filelength(int __handle);
#pragma aux filelength = \
      "mov ax, 0x4202" \
      "xor cx, cx" \
      "xor dx, dx" \
      "int 0x21" \
      "push ax" \
      "push dx" \
      "mov ax, 0x4200" \
      "xor cx, cx" \
      "xor dx, dx" \
      "int 0x21" \
      "pop dx" \
      "pop ax" \
      parm [bx] \
      modify [cx] \
      value [dx ax];

extern int unlink(const char *pathname);

/* some non-conforming functions to make the executable smaller */
int open(const char *pathname, int flags, ...)
{
  int handle;
  int result = (flags & O_CREAT ?
                _dos_creat(pathname, _A_NORMAL, &handle) :
                _dos_open(pathname, flags & (O_RDONLY | O_WRONLY | O_RDWR),
                          &handle));

  return (result == 0 ? handle : -1);
}

int read(int fd, void *buf, unsigned count)
{
  unsigned bytes;
  int result = _dos_read(fd, buf, count, &bytes);

  return (result == 0 ? bytes : -1);
}

int write(int fd, const void *buf, unsigned count)
{
  unsigned bytes;
  int result = _dos_write(fd, buf, count, &bytes);

  return (result == 0 ? bytes : -1);
}

#define close _dos_close

int stat(const char *file_name, struct stat *buf)
{
  struct find_t find_tbuf;
  UNREFERENCED_PARAMETER(buf);

  return _dos_findfirst(file_name, _A_NORMAL | _A_HIDDEN | _A_SYSTEM, &find_tbuf);
}

/* WATCOM's getenv is case-insensitive which wastes a lot of space
   for our purposes. So here's a simple case-sensitive one */
char *getenv(const char *name)
{
  char **envp, *ep;
  const char *np;
  char ec, nc;

  for (envp = environ; (ep = *envp) != NULL; envp++) {
    np = name;
    do {
      ec = *ep++;
      nc = *np++;
      if (nc == 0) {
        if (ec == '=')
          return ep;
        break;
      }
    } while (ec == nc);
  }
  return NULL;
}
#endif

BYTE pgm[] = "SYS";

void put_boot(int, char *, char *, int, int);
BOOL check_space(COUNT, ULONG);
BOOL copy(COUNT drive, BYTE * srcPath, BYTE * rootPath, BYTE * file);

#define SEC_SIZE        512
#define COPY_SIZE	0x7e00

struct bootsectortype {
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
};

struct bootsectortype32 {
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
  ULONG bsBigFatSize;
  UBYTE bsFlags;
  UBYTE bsMajorVersion;
  UWORD bsMinorVersion;
  ULONG bsRootCluster;
  UWORD bsFSInfoSector;
  UWORD bsBackupBoot;
  ULONG bsReserved2[3];
  UBYTE bsDriveNumber;
  UBYTE bsReserved3;
  UBYTE bsExtendedSignature;
  ULONG bsSerialNumber;
  char bsVolumeLabel[11];
  char bsFileSystemID[8];
};

/*
 * globals needed by put_boot & check_space
 */
enum {FAT12 = 12, FAT16 = 16, FAT32 = 32} fs;  /* file system type */
/* static */ struct xfreespace x; /* we make this static to be 0 by default -
                                     this avoids FAT misdetections */

#define SBOFFSET        11
#define SBSIZE          (sizeof(struct bootsectortype) - SBOFFSET)
#define SBSIZE32        (sizeof(struct bootsectortype32) - SBOFFSET)

/* essentially - verify alignment on byte boundaries at compile time  */
struct VerifyBootSectorSize {
  char failure1[sizeof(struct bootsectortype) == 62 ? 1 : -1];
  char failure2[sizeof(struct bootsectortype) == 62 ? 1 : 0];
/* (Watcom has a nice warning for this, by the way) */
};

int FDKrnConfigMain(int argc, char **argv);

int main(int argc, char **argv)
{
  COUNT drive;                  /* destination drive */
  COUNT drivearg = 0;           /* drive argument position */
  COUNT srcarg = 0;             /* source argument position */
  BYTE *bsFile = NULL;          /* user specified destination boot sector */
  unsigned srcDrive;            /* source drive */
  BYTE srcPath[SYS_MAXPATH];    /* user specified source drive and/or path */
  BYTE rootPath[4];             /* alternate source path to try if not '\0' */
  WORD slen;
  int argno = 0;
  int bootonly = 0;
  int both = 0;
  char *kernel_name = "KERNEL.SYS";
  int load_segment = 0x60;

  printf("FreeDOS System Installer " SYS_VERSION ", " __DATE__ "\n\n");

  if (argc > 1 && memicmp(argv[1], "CONFIG", 6) == 0)
  {
    exit(FDKrnConfigMain(argc, argv));
  }

  for(argno = 1; argno < argc; argno++)
  {
    char *argp = argv[argno];

    if (argp[1] == ':' && argp[2] == '\0' && drivearg <= srcarg)
      drivearg = argno;

    if (srcarg == 0)
    {
      srcarg = argno;
    }
    else if (argp[0] == '/' && toupper(argp[1]) == 'K' && argno + 1 < argc)
    {
      argno++;
      kernel_name = argv[argno];
    }
    else if (argp[0] == '/' && toupper(argp[1]) == 'L' && argno + 1 < argc)
    {
      argno++;
      load_segment = (int)strtol(argv[argno], NULL, 16);
    }
    else if (memicmp(argp, "BOOTONLY", 8) == 0 && !bootonly)
    {
      bootonly = 1;
    }
    else if (memicmp(argp, "BOTH", 4) == 0 && !both)
    {
      both = 1;
    }
    else if (drivearg != argno)
    {
      if (bsFile == NULL)
      {
        bsFile = argp;
      }
      else
      {
        drivearg = 0;
        break;
      }
    }
  }

  if (drivearg == 0)
  {
    printf(
      "Usage: %s [source] drive: [bootsect [BOTH]] [BOOTONLY] [/K name] [/L segm]\n"
      "  source   = A:,B:,C:\\KERNEL\\BIN\\,etc., or current directory if not given\n"
      "  drive    = A,B,etc.\n"
      "  bootsect = name of 512-byte boot sector file image for drive:\n"
      "             to write to *instead* of real boot sector\n"
      "  BOTH     : write to *both* the real boot sector and the image file\n"
      "  BOOTONLY : do *not* copy kernel / shell, only update boot sector or image\n"
      "  /K name  : name of kernel to use instead of KERNEL.SYS\n"
      "  /L segm  : hex load segment to use instead of 60\n"
      "%s CONFIG /help\n", pgm, pgm);
    exit(1);
  }
  drive = toupper(argv[drivearg][0]) - 'A';

  if (drive < 0 || drive >= 26)
  {
    printf("%s: drive %c must be A:..Z:\n", pgm,
           *argv[(argc == 3 ? 2 : 1)]);
    exit(1);
  }

  srcPath[0] = '\0';
  if (drivearg > srcarg && srcarg)
  {
    strncpy(srcPath, argv[srcarg], SYS_MAXPATH - 12);
    /* leave room for COMMAND.COM\0 */
    srcPath[SYS_MAXPATH - 13] = '\0';
    /* make sure srcPath + "file" is a valid path */
    slen = strlen(srcPath);
    if ((srcPath[slen - 1] != ':') &&
        ((srcPath[slen - 1] != '\\') || (srcPath[slen - 1] != '/')))
    {
      srcPath[slen] = '\\';
      slen++;
      srcPath[slen] = '\0';
    }
  }

  /* Get source drive */
  if ((strlen(srcPath) > 1) && (srcPath[1] == ':'))     /* src specifies drive */
    srcDrive = toupper(*srcPath) - 'A';
  else                          /* src doesn't specify drive, so assume current drive */
  {
#ifdef __TURBOC__
    srcDrive = (unsigned) getdisk();
#else
    _dos_getdrive(&srcDrive);
    srcDrive--;
#endif
  }

  /* Don't try root if src==dst drive or source path given */
  if ((drive == srcDrive)
      || (*srcPath
          && ((srcPath[1] != ':') || ((srcPath[1] == ':') && srcPath[2]))))
    *rootPath = '\0';
  else
    sprintf(rootPath, "%c:\\", 'A' + srcDrive);

  printf("Processing boot sector...\n");
  put_boot(drive, bsFile, kernel_name, load_segment, both);

  if (!bootonly)
  {
    printf("\nCopying %s...\n", kernel_name);
    if (!copy(drive, srcPath, rootPath, kernel_name))
    {
      printf("\n%s: cannot copy \"%s\"\n", pgm, kernel_name);
      exit(1);
    } /* copy kernel */

    printf("\nCopying COMMAND.COM...\n");
    if (!copy(drive, srcPath, rootPath, "COMMAND.COM"))
    {
      char *comspec = getenv("COMSPEC");
      if (comspec != NULL)
      {
        printf("%s: Trying \"%s\"\n", pgm, comspec);
        if (!copy(drive, comspec, NULL, "COMMAND.COM"))
          comspec = NULL;
      }
      if (comspec == NULL)
      {
        printf("\n%s: cannot copy \"COMMAND.COM\"\n", pgm);
        exit(1);
      }
    } /* copy shell */
  }

  printf("\nSystem transferred.\n");
  return 0;
}

#ifdef DDEBUG
VOID dump_sector(unsigned char far * sec)
{
  COUNT x, y;
  char c;

  for (x = 0; x < 32; x++)
  {
    printf("%03X  ", x * 16);
    for (y = 0; y < 16; y++)
    {
      printf("%02X ", sec[x * 16 + y]);
    }
    for (y = 0; y < 16; y++)
    {
      c = sec[x * 16 + y];
      if (isprint(c))
        printf("%c", c);
      else
        printf(".");
    }
    printf("\n");
  }

  printf("\n");
}

#endif

#ifdef __WATCOMC__

int absread(int DosDrive, int nsects, int foo, void *diskReadPacket);
#pragma aux absread =  \
      "push bp"           \
      "int 0x25"          \
      "sbb ax, ax"        \
      "popf"              \
      "pop bp"            \
      parm [ax] [cx] [dx] [bx] \
      modify [si di] \
      value [ax];

int abswrite(int DosDrive, int nsects, int foo, void *diskReadPacket);
#pragma aux abswrite =  \
      "push bp"           \
      "int 0x26"          \
      "sbb ax, ax"        \
      "popf"              \
      "pop bp"            \
      parm [ax] [cx] [dx] [bx] \
      modify [si di] \
      value [ax];

int fat32readwrite(int DosDrive, void *diskReadPacket, unsigned intno);
#pragma aux fat32readwrite =  \
      "mov ax, 0x7305"    \
      "mov cx, 0xffff"    \
      "int 0x21"          \
      "sbb ax, ax"        \
      parm [dx] [bx] [si] \
      modify [cx dx si]   \
      value [ax];

void reset_drive(int DosDrive);
#pragma aux reset_drive = \
      "push ds" \
      "inc dx" \
      "mov ah, 0xd" \ 
      "int 0x21" \
      "mov ah,0x32" \
      "int 0x21" \
      "pop ds" \
      parm [dx] \
      modify [ax bx];

void truename(char far *dest, const char *src);
#pragma aux truename = \
      "mov ah,0x60"	  \
      "int 0x21"          \
      parm [es di] [si];

int generic_block_ioctl(unsigned char drive, unsigned cx, unsigned char *par);
#pragma aux generic_block_ioctl = \
      "mov ax, 0x440d" \
      "int 0x21" \
      "sbb ax, ax" \
      value [ax] \
      parm [bl] [cx] [dx];

#else

#ifndef __TURBOC__

int2526readwrite(int DosDrive, void *diskReadPacket, unsigned intno)
{
  union REGS regs;

  regs.h.al = (BYTE) DosDrive;
  regs.x.bx = (short)diskReadPacket;
  regs.x.cx = 0xffff;

  int86(intno, &regs, &regs);

  return regs.x.cflag;
}

#define absread(DosDrive, foo, cx, diskReadPacket) \
int2526readwrite(DosDrive, diskReadPacket, 0x25)

#define abswrite(DosDrive, foo, cx, diskReadPacket) \
int2526readwrite(DosDrive, diskReadPacket, 0x26)

#endif

fat32readwrite(int DosDrive, void *diskReadPacket, unsigned intno)
{
  union REGS regs;

  regs.x.ax = 0x7305;
  regs.h.dl = DosDrive;
  regs.x.bx = (short)diskReadPacket;
  regs.x.cx = 0xffff;
  regs.x.si = intno;
  intdos(&regs, &regs);
  
  return regs.x.cflag;
} /* fat32readwrite */

void reset_drive(int DosDrive)
{
  union REGS regs;

  regs.h.ah = 0xd;
  intdos(&regs, &regs);
  regs.h.ah = 0x32;
  regs.h.dl = DosDrive + 1;
  intdos(&regs, &regs);
} /* reset_drive */

int generic_block_ioctl(unsigned char drive, unsigned cx, unsigned char *par)
{
  union REGS regs;

  regs.x.ax = 0x440d;
  regs.x.cx = cx;
  regs.x.dx = (unsigned)par;
  regs.h.bl = drive + 1;
  intdos(&regs, &regs);
  return regs.x.cflag;
} /* generic_block_ioctl */

void truename(char *dest, const char *src)
{
  union REGS regs;
  struct SREGS sregs;

  regs.h.ah = 0x60;
  sregs.es = FP_SEG(dest);
  regs.x.di = FP_OFF(dest);
  sregs.ds = FP_SEG(src);
  regs.x.si = FP_OFF(src);
  intdosx(&regs, &regs, &sregs);
} /* truename */

#endif

int MyAbsReadWrite(int DosDrive, int count, ULONG sector, void *buffer,
                   int write)
{
  struct {
    unsigned long sectorNumber;
    unsigned short count;
    void far *address;
  } diskReadPacket;

  diskReadPacket.sectorNumber = sector;
  diskReadPacket.count = count;
  diskReadPacket.address = buffer;

  if ((!write && absread(DosDrive, -1, -1, &diskReadPacket) == -1)
      || (write && abswrite(DosDrive, -1, -1, &diskReadPacket) == -1))
  {
#ifdef WITHFAT32
    return fat32readwrite(DosDrive + 1, &diskReadPacket, write);
#else
    return 0xff;
#endif
  }
  return 0;
} /* MyAbsReadWrite */

#ifdef __WATCOMC__

unsigned getextdrivespace(void far *drivename, void *buf, unsigned buf_size);
#pragma aux getextdrivespace =  \
      "mov ax, 0x7303"    \
      "stc"		  \
      "int 0x21"          \
      "sbb ax, ax"        \
      parm [es dx] [di] [cx] \
      value [ax];

#else /* !defined __WATCOMC__ */

unsigned getextdrivespace(void *drivename, void *buf, unsigned buf_size)
{
  union REGS regs;
  struct SREGS sregs;

  regs.x.ax = 0x7303;         /* get extended drive free space */

  sregs.es = FP_SEG(buf);
  regs.x.di = FP_OFF(buf);
  sregs.ds = FP_SEG(drivename);
  regs.x.dx = FP_OFF(drivename);

  regs.x.cx = buf_size;

  intdosx(&regs, &regs, &sregs);
  return regs.x.ax == 0x7300 || regs.x.cflag;
} /* getextdrivespace */

#endif /* defined __WATCOMC__ */

#ifdef __WATCOMC__
/*
 * If BIOS has got LBA extensions, after the Int 13h call BX will be 0xAA55.
 * If extended disk access functions are supported, bit 0 of CX will be set.
 */
BOOL haveLBA(void);     /* return TRUE if we have LBA BIOS, FALSE otherwise */
#pragma aux haveLBA =  \
      "mov ax, 0x4100"  /* IBM/MS Int 13h Extensions - installation check */ \
      "mov bx, 0x55AA" \
      "mov dl, 0x80"   \
      "int 0x13"       \
      "xor ax, ax"     \
      "cmp bx, 0xAA55" \
      "jne quit"       \
      "and cx, 1"      \
      "xchg cx, ax"    \
"quit:"                \
      modify [bx cx]   \
      value [ax];
#else

BOOL haveLBA(void)
{
  union REGS r;
  r.x.ax = 0x4100;
  r.x.bx = 0x55AA;
  r.h.dl = 0x80;
  int86(0x13, &r, &r);
  return r.x.bx == 0xAA55 && r.x.cx & 1;
}
#endif

void correct_bpb(struct bootsectortype *default_bpb,
                 struct bootsectortype *oldboot)
{
  /* don't touch partitions (floppies most likely) that don't have hidden
     sectors */
  if (default_bpb->bsHiddenSecs == 0)
    return;
#ifdef DEBUG
  printf("Old boot sector values: sectors/track: %u, heads: %u, hidden: %lu\n",
         oldboot->bsSecPerTrack, oldboot->bsHeads, oldboot->bsHiddenSecs);
  printf("Default and new boot sector values: sectors/track: %u, heads: %u, "
         "hidden: %lu\n", default_bpb->bsSecPerTrack, default_bpb->bsHeads,
         default_bpb->bsHiddenSecs);
#endif

  oldboot->bsSecPerTrack = default_bpb->bsSecPerTrack;
  oldboot->bsHeads = default_bpb->bsHeads;
  oldboot->bsHiddenSecs = default_bpb->bsHiddenSecs;
}

void put_boot(int drive, char *bsFile, char *kernel_name, int load_seg, int both)
{
#ifdef WITHFAT32
  struct bootsectortype32 *bs32;
#endif
  struct bootsectortype *bs;
  static unsigned char oldboot[SEC_SIZE], newboot[SEC_SIZE];
  static unsigned char default_bpb[0x5c];

#ifdef DEBUG
  printf("Reading old bootsector from drive %c:\n", drive + 'A');
#endif

  /* lock drive */
  generic_block_ioctl((unsigned char)drive + 1, 0x84a, NULL);

  reset_drive(drive);
  /* suggestion: allow reading from a boot sector or image file here */
  if (MyAbsReadWrite(drive, 1, 0, oldboot, 0) != 0)
  {
    printf("can't read old boot sector for drive %c:\n", drive + 'A');
    exit(1);
  }

#ifdef DDEBUG
  printf("Old Boot Sector:\n");
  dump_sector(oldboot);
#endif

  bs = (struct bootsectortype *)&oldboot;

  if (bs->bsBytesPerSec != SEC_SIZE)
  {
    printf("Sector size is not 512 but %d bytes - not currently supported!\n",
      bs->bsBytesPerSec);
    exit(1); /* Japan?! */
  }

  {
   /* see "FAT: General Overview of On-Disk Format" v1.02, 5.V.1999
    * (http://www.nondot.org/sabre/os/files/FileSystems/FatFormat.pdf)
    */
    ULONG fatSize, totalSectors, dataSectors, clusters;
    UCOUNT rootDirSectors;

    bs32 = (struct bootsectortype32 *)&oldboot;
    rootDirSectors = (bs->bsRootDirEnts * DIRENT_SIZE  /* 32 */
                 + bs32->bsBytesPerSec - 1) / bs32->bsBytesPerSec;
    fatSize      = bs32->bsFATsecs ? bs32->bsFATsecs : bs32->bsBigFatSize;
    totalSectors = bs32->bsSectors ? bs32->bsSectors : bs32->bsHugeSectors;
    dataSectors = totalSectors
      - bs32->bsResSectors - (bs32->bsFATs * fatSize) - rootDirSectors;
    clusters = dataSectors / bs32->bsSecPerClust;
 
    if (clusters < FAT_MAGIC)        /* < 4085 */
      fs = FAT12;
    else if (clusters < FAT_MAGIC16) /* < 65525 */
      fs = FAT16;
    else
      fs = FAT32;
  }

  /* bit 0 set if function to use current BPB, clear if Device
           BIOS Parameter Block field contains new default BPB
     bit 1 set if function to use track layout fields only
           must be clear if CL=60h
     bit 2 set if all sectors in track same size (should be set) (RBIL) */
  default_bpb[0] = 4;

  if (fs == FAT32)
  {
    printf("FAT type: FAT32\n");
    /* get default bpb (but not for floppies) */
    if (drive >= 2 &&
        generic_block_ioctl((unsigned char)drive + 1, 0x4860, default_bpb) == 0)
      correct_bpb((struct bootsectortype *)(default_bpb + 7 - 11), bs);

#ifdef WITHFAT32                /* copy one of the FAT32 boot sectors */
    memcpy(newboot, haveLBA() ? fat32lba : fat32chs, SEC_SIZE);
#else
    printf("SYS hasn't been compiled with FAT32 support.\n"
           "Consider using -DWITHFAT32 option.\n");
    exit(1);
#endif
  }
  else
  { /* copy the FAT12/16 CHS+LBA boot sector */
    printf("FAT type: FAT1%c\n", fs + '0' - 10);
    if (drive >= 2 &&
        generic_block_ioctl((unsigned char)drive + 1, 0x860, default_bpb) == 0)
      correct_bpb((struct bootsectortype *)(default_bpb + 7 - 11), bs);
    memcpy(newboot, fs == FAT16 ? fat16com : fat12com, SEC_SIZE);
  }

  /* Copy disk parameter from old sector to new sector */
#ifdef WITHFAT32
  if (fs == FAT32)
    memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE32);
  else
#endif
    memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE);

  bs = (struct bootsectortype *)&newboot;

  memcpy(bs->OemName, "FreeDOS ", 8);

#ifdef WITHFAT32
  if (fs == FAT32)
  {
    bs32 = (struct bootsectortype32 *)&newboot;
    /* put 0 for A: or B: (force booting from A:), otherwise use DL */
    bs32->bsDriveNumber = drive < 2 ? 0 : 0xff;
    /* the location of the "0060" segment portion of the far pointer
       in the boot sector is just before cont: in boot*.asm.
       This happens to be offset 0x78 (=0x3c * 2) for FAT32 and
       offset 0x5c (=0x2e * 2) for FAT16 */
    /* i.e. BE CAREFUL WHEN YOU CHANGE THE BOOT SECTORS !!! */
    ((int *)newboot)[0x3C] = load_seg;
#ifdef DEBUG
    printf(" FAT starts at sector %lx + %x\n",
           bs32->bsHiddenSecs, bs32->bsResSectors);
#endif
  }
  else
#endif
  {
    /* put 0 for A: or B: (force booting from A:), otherwise use DL */
    bs->bsDriveNumber = drive < 2 ? 0 : 0xff;
    ((int *)newboot)[0x2E] = load_seg;
  }

#ifdef DEBUG /* add an option to display this on user request? */
  printf("Root dir entries = %u\n", bs->bsRootDirEnts);

  printf("FAT starts at sector (%lu + %u)\n",
         bs->bsHiddenSecs, bs->bsResSectors);
  printf("Root directory starts at sector (PREVIOUS + %u * %u)\n",
         bs->bsFATsecs, bs->bsFATs);
#endif
  {
    int i = 0;
    memset(&newboot[0x1f1], ' ', 11);
    while (kernel_name[i] && kernel_name[i] != '.')
    {
      if (i < 8)
        newboot[0x1f1+i] = toupper(kernel_name[i]);
      i++;
    }
    if (kernel_name[i] == '.')
    {
      /* copy extension */
      int j = 0;
      i++;
      while (kernel_name[i+j] && j < 3)
      {
        newboot[0x1f9+j] = toupper(kernel_name[i+j]);
        j++;
      }
    }
  }

#ifdef DEBUG
  /* there's a zero past the kernel name in all boot sectors */
  printf("Boot sector kernel name set to %s\n", &newboot[0x1f1]);
  printf("Boot sector load segment set to %Xh\n", load_seg);
#endif

#ifdef DDEBUG
  printf("\nNew Boot Sector:\n");
  dump_sector(newboot);
#endif

  if ((bsFile == NULL) || both)
  {

#ifdef DEBUG
    printf("writing new bootsector to drive %c:\n", drive + 'A');
#endif

    /* write newboot to a drive */
    if (MyAbsReadWrite(drive, 1, 0, newboot, 1) != 0)
    {
      printf("Can't write new boot sector to drive %c:\n", drive + 'A');
      exit(1);
    }
  } /* if write boot sector */

  if (bsFile != NULL)
  {
    int fd;

#ifdef DEBUG
    printf("writing new bootsector to file %s\n", bsFile);
#endif

    /* write newboot to bsFile */
    if ((fd = /* suggestion: do not trunc - allows to write to images */
         open(bsFile, O_RDWR | O_TRUNC | O_CREAT | O_BINARY,
              S_IREAD | S_IWRITE)) < 0)
    {
      printf(" %s: can't create\"%s\"\nDOS errnum %d", pgm, bsFile, errno);
      exit(1);
    }
    if (write(fd, newboot, SEC_SIZE) != SEC_SIZE)
    {
      printf("Can't write %u bytes to %s\n", SEC_SIZE, bsFile);
      close(fd);
      unlink(bsFile);
      exit(1);
    }
    close(fd);
  } /* if write boot sector file */
  reset_drive(drive);

  /* unlock_drive */
  generic_block_ioctl((unsigned char)drive + 1, 0x86a, NULL);
} /* put_boot */


/*
 * Returns TRUE if `drive` has at least `bytes` free space, FALSE otherwise.
 * put_sector() must have been already called to determine file system type.
 */
BOOL check_space(COUNT drive, ULONG bytes)
{
#ifdef WITHFAT32
  if (fs == FAT32)
  {
    char *drivename = "A:\\";
    drivename[0] = 'A' + drive;
    getextdrivespace(drivename, &x, sizeof(x));
    return x.xfs_freeclusters > (bytes / (x.xfs_clussize * x.xfs_secsize));
  }
  else
#endif
  {
#ifdef __TURBOC__
    struct dfree df;
    getdfree(drive + 1, &df);
    return (ULONG)df.df_avail * df.df_sclus * df.df_bsec >= bytes;
#else
    struct _diskfree_t df;
    _dos_getdiskfree(drive + 1, &df);
    return (ULONG)df.avail_clusters * df.sectors_per_cluster
      * df.bytes_per_sector >= bytes;
#endif
  }
} /* check_space */


BYTE copybuffer[COPY_SIZE];

BOOL copy(COUNT drive, BYTE * srcPath, BYTE * rootPath, BYTE * file)
{
  static BYTE dest[SYS_MAXPATH], source[SYS_MAXPATH];
  unsigned ret;
  int fdin, fdout;
  ULONG copied = 0;
  struct stat fstatbuf;

  strcpy(source, srcPath);
  if (rootPath != NULL) /* trick for comspec */
    strcat(source, file);

  if (stat(source, &fstatbuf))
  {
    printf("%s: \"%s\" not found\n", pgm, source);

    if ((rootPath != NULL) && (*rootPath) /* && (errno == ENOENT) */ )
    {
      sprintf(source, "%s%s", rootPath, file);
      printf("%s: Trying \"%s\"\n", pgm, source);
      if (stat(source, &fstatbuf))
      {
        printf("%s: \"%s\" not found\n", pgm, source);
        return FALSE;
      }
    }
    else
      return FALSE;
  }

  truename(dest, source);
  strcpy(source, dest);
  sprintf(dest, "%c:\\%s", 'A' + drive, file);
  if (stricmp(source, dest) == 0)
  {
    printf("%s: source and destination are identical: skipping \"%s\"\n",
           pgm, source);
    return TRUE;
  }

  if ((fdin = open(source, O_RDONLY | O_BINARY)) < 0)
  {
    printf("%s: failed to open \"%s\"\n", pgm, source);
    return FALSE;
  }

  if (!check_space(drive, filelength(fdin)))
  {
    printf("%s: Not enough space to transfer %s\n", pgm, file);
    close(fdin);
    exit(1);
  }

  if ((fdout =
       open(dest, O_RDWR | O_TRUNC | O_CREAT | O_BINARY,
            S_IREAD | S_IWRITE)) < 0)
  {
    printf(" %s: can't create\"%s\"\nDOS errnum %d", pgm, dest, errno);
    close(fdin);
    return FALSE;
  }

  while ((ret = read(fdin, copybuffer, COPY_SIZE)) > 0)
  {
    if (write(fdout, copybuffer, ret) != ret)
    {
      printf("Can't write %u bytes to %s\n", ret, dest);
      close(fdout);
      unlink(dest);
      break;
    }
    copied += ret;
  }

  {
#if defined __WATCOMC__ || defined _MSC_VER /* || defined __BORLANDC__ */
    unsigned short date, time;	  
    _dos_getftime(fdin, &date, &time);
    _dos_setftime(fdout, date, time);
#elif defined __TURBOC__
    struct ftime ftime;
    getftime(fdin, &ftime);
    setftime(fdout, &ftime);
#endif
  }

  close(fdin);
  close(fdout);

#ifdef __SOME_OTHER_COMPILER__
  {
#include <utime.h>
    struct utimbuf utimb;

    utimb.actime =              /* access time */
        utimb.modtime = fstatbuf.st_mtime;      /* modification time */
    utime(dest, &utimb);
  };
#endif

  printf("%lu Bytes transferred", copied);

  return TRUE;
} /* copy */

