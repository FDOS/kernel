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

/* 
    TE thinks, that the boot info storage should be done by FORMAT, noone else
    unfortunately, that doesn't work ???
*/
#define STORE_BOOT_INFO

#define DEBUG
/* #define DDEBUG */

#define SYS_VERSION "v2.5"

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
extern WORD CDECL printf(CONST BYTE * fmt, ...);
extern WORD CDECL sprintf(BYTE * buff, CONST BYTE * fmt, ...);

#include "b_fat12.h"
#include "b_fat16.h"
#ifdef WITHFAT32
#include "b_fat32.h"
#endif

#ifndef __WATCOMC__

#include <io.h>

#else

int unlink(const char *pathname);
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
#endif
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

BYTE pgm[] = "SYS";

void put_boot(COUNT, BYTE *, BOOL);
BOOL check_space(COUNT, BYTE *);
BOOL copy(COUNT drive, BYTE * srcPath, BYTE * rootPath, BYTE * file);
COUNT DiskRead(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT DiskWrite(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);

#define SEC_SIZE        512
#define COPY_SIZE	0x7e00

#ifdef _MSC_VER
#pragma pack(1)
#endif

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
  char unused[2];
  UWORD sysRootDirSecs;         /* of sectors root dir uses */
  ULONG sysFatStart;            /* first FAT sector */
  ULONG sysRootDirStart;        /* first root directory sector */
  ULONG sysDataStart;           /* first data sector */
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
  ULONG sysFatStart;
  ULONG sysDataStart;
  UWORD sysFatSecMask;
  UWORD sysFatSecShift;
};

UBYTE newboot[SEC_SIZE], oldboot[SEC_SIZE];

#define SBOFFSET        11
#define SBSIZE          (sizeof(struct bootsectortype) - SBOFFSET)
#define SBSIZE32        (sizeof(struct bootsectortype32) - SBOFFSET)

/* essentially - verify alignment on byte boundaries at compile time  */
struct VerifyBootSectorSize {
  char failure1[sizeof(struct bootsectortype) == 78 ? 1 : -1];
  char failure2[sizeof(struct bootsectortype) == 78 ? 1 : 0];
};

int FDKrnConfigMain(int argc, char **argv);

int main(int argc, char **argv)
{
  COUNT drive;                  /* destination drive */
  COUNT drivearg = 0;           /* drive argument position */
  BYTE *bsFile = NULL;          /* user specified destination boot sector */
  unsigned srcDrive;            /* source drive */
  BYTE srcPath[SYS_MAXPATH];    /* user specified source drive and/or path */
  BYTE rootPath[4];             /* alternate source path to try if not '\0' */
  WORD slen;

  printf("FreeDOS System Installer " SYS_VERSION "\n\n");

  if (argc > 1 && memicmp(argv[1], "CONFIG", 6) == 0)
  {
    exit(FDKrnConfigMain(argc, argv));
  }

  srcPath[0] = '\0';
  if (argc > 1 && argv[1][1] == ':' && argv[1][2] == '\0')
    drivearg = 1;

  if (argc > 2 && argv[2][1] == ':' && argv[2][2] == '\0')
  {
    drivearg = 2;
    strncpy(srcPath, argv[1], SYS_MAXPATH - 12);
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

  if (drivearg == 0)
  {
    printf("Usage: %s [source] drive: [bootsect [BOTH|BOOTONLY]]\n", pgm);
    printf
        ("  source   = A:,B:,C:\\KERNEL\\BIN\\,etc., or current directory if not given\n");
    printf("  drive    = A,B,etc.\n");
    printf
        ("  bootsect = name of 512-byte boot sector file image for drive:\n");
    printf("             to write to *instead* of real boot sector\n");
    printf
        ("  BOTH     : write to *both* the real boot sector and the image file\n");
    printf
        ("  BOOTONLY : do *not* copy kernel / shell, only update boot sector or image\n");
    printf("%s CONFIG /help\n", pgm);
    exit(1);
  }
  drive = toupper(argv[drivearg][0]) - 'A';

  if (drive < 0 || drive >= 26)
  {
    printf("%s: drive %c must be A:..Z:\n", pgm,
           *argv[(argc == 3 ? 2 : 1)]);
    exit(1);
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

  if ((argc <= drivearg + 2)
      || (memicmp(argv[drivearg + 2], "BOOTONLY", 8) != 0))
  {
    if (!check_space(drive, oldboot))
    {
      printf("%s: Not enough space to transfer system files\n", pgm);
      exit(1);
    }

    printf("\nCopying KERNEL.SYS...\n");
    if (!copy(drive, srcPath, rootPath, "kernel.sys"))
    {
      printf("\n%s: cannot copy \"KERNEL.SYS\"\n", pgm);
      exit(1);
    }
  } /* copy kernel */

  if (argc > drivearg + 1)
    bsFile = argv[drivearg + 1];
    
  printf("\nWriting boot sector...\n");
  put_boot(drive, bsFile,
           (argc > drivearg + 2)
           && memicmp(argv[drivearg + 2], "BOTH", 4) == 0);

  if ((argc <= drivearg + 2)
      || (memicmp(argv[drivearg + 2], "BOOTONLY", 8) != 0))
  {
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
    }
  } /* copy shell */

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
      c = oldboot[x * 16 + y];
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
      "int 0x25"          \
      "sbb ax, ax"        \
      parm [ax] [cx] [dx] [bx] \
      modify [si di bp] \
      value [ax];

int abswrite(int DosDrive, int nsects, int foo, void *diskReadPacket);
#pragma aux abswrite =  \
      "int 0x26"          \
      "sbb ax, ax"        \
      parm [ax] [cx] [dx] [bx] \
      modify [si di bp] \
      value [ax];

fat32readwrite(int DosDrive, void *diskReadPacket, unsigned intno);
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

#define absread(int DosDrive, int foo, int cx, void *diskReadPacket) \
int2526readwrite(DosDrive, diskReadPacket, 0x25)

#define abswrite(int DosDrive, int foo, int cx, void *diskReadPacket) \
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

unsigned getdrivespace(COUNT drive, unsigned *total_clusters);
#pragma aux getdrivespace =  \
      "mov ah, 0x36"      \
      "inc dx"            \
      "int 0x21"          \
      "mov [si], dx"      \
      parm [dx] [si]      \
      modify [bx cx dx]   \
      value [ax];

unsigned getextdrivespace(void far *drivename, void *buf, unsigned buf_size);
#pragma aux getextdrivespace =  \
      "mov ax, 0x7303"    \
      "stc"		  \
      "int 0x21"          \
      "sbb ax, ax"        \
      parm [es dx] [di] [cx] \
      value [ax];

#else

unsigned getdrivespace(COUNT drive, unsigned *total_clusters)
{
  union REGS regs;

  regs.h.ah = 0x36;             /* get drive free space */
  regs.h.dl = drive + 1;        /* 1 = 'A',... */
  intdos(&regs, &regs);
  *total_clusters = regs.x.dx;
  return regs.x.ax;
} /* getdrivespace */

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

#endif

VOID put_boot(COUNT drive, BYTE * bsFile, BOOL both)
{
  ULONG temp;
  struct bootsectortype *bs;
#ifdef WITHFAT32
  struct bootsectortype32 *bs32;
#endif
  int fs;
  char *drivename = "A:\\";
  static unsigned char x[0x40]; /* we make this static to be 0 by default -
				   this avoids FAT misdetections */
  unsigned total_clusters;

#ifdef DEBUG
  printf("Reading old bootsector from drive %c:\n", drive + 'A');
#endif

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
  if ((bs->bsFileSysType[4] == '6') && (bs->bsBootSignature == 0x29))
  {
    fs = 16;
  }
  else
  {
    fs = 12;
  }

/*
    the above code is not save enough for me (TE), so we change the
    FS detection method to GetFreeDiskSpace().
    this should work, as the disk was writeable, so GetFreeDiskSpace should work.
*/

  /* would work different when reading from an image */
  if (getdrivespace(drive, &total_clusters) == 0xffff)
  {
    printf("can't get free disk space for %c:\n", drive + 'A');
    exit(1);
  }

  if (total_clusters <= 0xff6)
  {
    if (fs != 12)
      printf("warning : new detection overrides old detection\a\n");
    fs = 12;
  }
  else
  {

    if (fs != 16)
      printf("warning : new detection overrides old detection\a\n");
    fs = 16;

    /* fs = 16/32.
       we don't want to crash a FAT32 drive
     */

    drivename[0] = 'A' + drive;
    /* would also work different when reading from an image */
    if (getextdrivespace(drivename, x, sizeof(x)))
    /* error --> no Win98 --> no FAT32 */
    {
      printf("get extended drive space not supported --> no FAT32\n");
    }
    else
    {
      if (*(unsigned long *)(x + 0x10)  /* total number of clusters */
          > (unsigned)65526l)
      {
        fs = 32;
      }
    }
  }

  if (fs == 16)
  {
    memcpy(newboot, b_fat16, SEC_SIZE); /* copy FAT16 boot sector */
    printf("FAT type: FAT16\n");
  }
  else if (fs == 12)
  {
    memcpy(newboot, b_fat12, SEC_SIZE); /* copy FAT12 boot sector */
    printf("FAT type: FAT12\n");
  }
  else
  {
    printf("FAT type: FAT32\n");
#ifdef WITHFAT32
    memcpy(newboot, b_fat32, SEC_SIZE); /* copy FAT32 boot sector */
#else
    printf("SYS hasn't been compiled with FAT32 support.");
    printf("Consider using -DWITHFAT32 option.\n");
    exit(1);
#endif
  }

  /* Copy disk parameter from old sector to new sector */
#ifdef WITHFAT32
  if (fs == 32)
    memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE32);
  else
#endif
    memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE);

  bs = (struct bootsectortype *)&newboot;

  memcpy(bs->OemName, "FreeDOS ", 8);

#ifdef WITHFAT32
  if (fs == 32)
  {
    bs32 = (struct bootsectortype32 *)&newboot;

    temp = bs32->bsHiddenSecs + bs32->bsResSectors;
    bs32->sysFatStart = temp;

    bs32->sysDataStart = temp + bs32->bsBigFatSize * bs32->bsFATs;
    bs32->sysFatSecMask = bs32->bsBytesPerSec / 4 - 1;

    temp = bs32->sysFatSecMask + 1;
    for (bs32->sysFatSecShift = 0; temp != 1;
         bs32->sysFatSecShift++, temp >>= 1) ;
    /* put 0 for A: or B: (force booting from A:), otherwise use DL */
    bs32->bsDriveNumber = drive < 2 ? 0 : 0xff;
  }
#ifdef DEBUG
  if (fs == 32)
  {
    printf("FAT starts at sector %lx = (%lx + %x)\n", bs32->sysFatStart,
           bs32->bsHiddenSecs, bs32->bsResSectors);
    printf("DATA starts at sector %lx\n", bs32->sysDataStart);
  }
#endif
  else
#endif
  {
#ifdef STORE_BOOT_INFO
    /* TE thinks : never, see above */
    /* temporary HACK for the load segment (0x0060): it is in unused */
    /* only needed for older kernels */
    *((UWORD *) (bs->unused)) =
        *((UWORD *) (((struct bootsectortype *)&b_fat16)->unused));
    /* end of HACK */
    /* root directory sectors */

    bs->sysRootDirSecs = bs->bsRootDirEnts / 16;

    /* sector FAT starts on */
    temp = bs->bsHiddenSecs + bs->bsResSectors;
    bs->sysFatStart = temp;

    /* sector root directory starts on */
    temp = temp + bs->bsFATsecs * bs->bsFATs;
    bs->sysRootDirStart = temp;

    /* sector data starts on */
    temp = temp + bs->sysRootDirSecs;
    bs->sysDataStart = temp;
    /* put 0 for A: or B: (force booting from A:), otherwise use DL */
    bs->bsDriveNumber = drive < 2 ? 0 : 0xff;
  }

#ifdef DEBUG /* add an option to display this on user request? */
  printf("Root dir entries = %u\n", bs->bsRootDirEnts);
  printf("Root dir sectors = %u\n", bs->sysRootDirSecs);

  printf("FAT starts at sector %lu = (%lu + %u)\n", bs->sysFatStart,
         bs->bsHiddenSecs, bs->bsResSectors);
  printf("Root directory starts at sector %lu = (PREVIOUS + %u * %u)\n",
         bs->sysRootDirStart, bs->bsFATsecs, bs->bsFATs);
  printf("DATA starts at sector %lu = (PREVIOUS + %u)\n", bs->sysDataStart,
         bs->sysRootDirSecs);
#endif
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
} /* put_boot */


BOOL check_space(COUNT drive, BYTE * BlkBuffer)
{
  /* this should check, if on destination is enough space
     to hold command.com+ kernel.sys */

  UNREFERENCED_PARAMETER(drive);
  UNREFERENCED_PARAMETER(BlkBuffer);

  return TRUE;
} /* check_space */


BYTE copybuffer[COPY_SIZE];

BOOL copy(COUNT drive, BYTE * srcPath, BYTE * rootPath, BYTE * file)
{
  BYTE dest[SYS_MAXPATH], source[SYS_MAXPATH];
  unsigned ret;
  int fdin, fdout;
  ULONG copied = 0;
  struct stat fstatbuf;

  sprintf(dest, "%c:\\%s", 'A' + drive, file);
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

  if ((fdin = open(source, O_RDONLY | O_BINARY)) < 0)
  {
    printf("%s: failed to open \"%s\"\n", pgm, source);
    return FALSE;
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

#ifdef __TURBOC__
  {
    struct ftime ftime;
    getftime(fdin, &ftime);
    setftime(fdout, &ftime);
  }
#endif
#ifdef __WATCOMC__
  {
    unsigned short date, time;	  
    _dos_getftime(fdin, &date, &time);
    _dos_setftime(fdout, date, time);
  }
#endif  

  close(fdin);
  close(fdout);

#ifdef _MSV_VER
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


/* version 2.2 jeremyd 2001/9/20
   Changed so if no source given or only source drive (no path)
   given, then checks for kernel.sys & command.com in current
   path (of current drive or given drive) and if not there
   uses root (but only if source & destination drive are different).
   Fix printf to include count(ret) if copy can't write all requested bytes
*/

/* version 2.1a jeremyd 2001/8/19
   modified so takes optional 2nd parameter (similar to PC DOS)
   where if only 1 argument is given, assume to be destination drive,
   but if two arguments given, 1st is source (drive and/or path)
   and second is destination drive
*/

/* Revision 2.1 tomehlert 2001/4/26

    changed the file system detection code.
*/

/* Revision 2.0 tomehlert 2001/4/26
   
   no direct access to the disk any more, this is FORMAT's job
   no floppy.asm anymore, no segmentation problems.
   no access to partition tables
   
   instead copy boot sector using int25/int26 = absdiskread()/write
   
   if xxDOS is able to handle the disk, SYS should work
   
   additionally some space savers:
   
   replaced fopen() by open() 
   
   included (slighly modified) PRF.c from kernel
   
   size is no ~7500 byte vs. ~13690 before
*/
