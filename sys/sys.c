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
#define WITHOEMCOMPATBS /* include support for OEM MS/PC DOS 3.??-6.x */

#define SYS_VERSION "v3.5"

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
#ifdef WITHOEMCOMPATBS
#include "oemfat12.h"
#include "oemfat16.h"
#endif

#ifndef __WATCOMC__

#include <io.h>

/* returns current DOS drive, A=0, B=1,C=2, ... */
#ifdef __TURBOC__
#define getcurdrive (unsigned)getdisk
#else
unsigned getcurdrive(void)
{
  union REGS regs;
  regs.h.ah = 0x19;
  int86(0x21, &regs, &regs);
  return regs.h.al;
}
#endif

#else

/* returns current DOS drive, A=0, B=1,C=2, ... */
unsigned getcurdrive(void);
#pragma aux getcurdrive = \
      "mov ah, 0x19"      \
      "int 0x21"          \
      "xor ah, ah"        \
      value [ax];


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

int stat(const char *file_name, struct stat *statbuf)
{
  struct find_t find_tbuf;

  int ret = _dos_findfirst(file_name, _A_NORMAL | _A_HIDDEN | _A_SYSTEM, &find_tbuf);
  statbuf->st_size = (off_t)find_tbuf.size;
  /* statbuf->st_attr = (ULONG)find_tbuf.attrib; */
  return ret;
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

/* FreeDOS sys, we default to our kernel and load segment, but
   if not found (or explicitly given) support OEM DOS variants
   (such as DR-DOS or a FreeDOS kernel mimicing other DOSes).
   Note: other (especially older) DOS versions expect the boot
   loader to perform particular steps, which we may not do;
   older PC/MS DOS variants may work with the OEM compatible
   boot sector (optionally included).
*/
typedef struct DOSBootFiles {
  const char * kernel;   /* filename boot sector loads and chains to */
  const char * dos;      /* optional secondary file for OS */
  WORD         loadseg;  /* segment kernel file expects to start at */
  BOOL         stdbs;    /* use FD boot sector (T) or oem compat one (F) */
  LONG         minsize;  /* smallest dos file can be and be valid, 0=existance optional */
} DOSBootFiles;
DOSBootFiles bootFiles[] = {
  /* Note: This order is the order OEM:AUTO uses to determine DOS flavor. */
  /* FreeDOS */ { "KERNEL.SYS", NULL, 0x60, 1, 0 },
  /* DR-DOS  */ { "IBMBIO.COM", "IBMDOS.COM", 0x70, 1, 1 },
#ifdef WITHOEMCOMPATBS
  /* PC-DOS  */ { "IBMBIO.COM", "IBMDOS.COM", 0x70, 0, 6138 },  /* pre v7 DR ??? */
  /* MS-DOS  */ { "IO.SYS", "MSDOS.SYS", 0x70, 0, 10240 },
#endif
  /* W9x-DOS */ { "IO.SYS", "MSDOS.SYS", 0x70, 1, 0},
};
#define DOSFLAVORS (sizeof(bootFiles) / sizeof(*bootFiles))

#define OEM_AUTO (-1) /* attempt to guess DOS on source drive */
#define OEM_FD     0  /* standard FreeDOS mode */
#define OEM_DR     1  /* use FreeDOS boot sector, but OEM names */
#define OEM_PC     2  /* use PC-DOS compatible boot sector and names */ 
#define OEM_MS     3  /* use PC-DOS compatible BS with MS names */
#define OEM_W9x    4  /* use FreeDOS boot sector but with MS names */

CONST char * msgDOS[DOSFLAVORS] = {
  "\n",  /* In standard FreeDOS mode, don't print anything special */
  "DR DOS (OpenDOS Enhancement Project) mode\n",
#ifdef WITHOEMCOMPATBS
  "PC-DOS compatibility mode\n",
  "MS-DOS compatibility mode\n",
#endif
  "Win9x DOS compatibility mode\n",
};

typedef struct SYSOptions {
  BYTE srcDrive[SYS_MAXPATH];   /* source drive:[path], root assumed if no path */
  BYTE dstDrive;                /* destination drive [STD SYS option] */
  int flavor;                   /* DOS variant we want to boot, default is AUTO/FD */
  DOSBootFiles kernel;          /* file name(s) and relevant data for kernel */
  BYTE defBootDrive;            /* value stored in boot sector for drive, eg 0x0=A, 0x80=C */
  BOOL ignoreBIOS;              /* true to NOP out boot sector code to get drive# from BIOS */
  BOOL copyFiles;               /* true to copy kernel files and command interpreter */
  BOOL writeBS;                 /* true to write boot sector to drive/partition LBA 0 */
  BYTE *bsFile;                 /* file name & path to save bs to when saving to file */
  BYTE *bsFileOrig;             /* file name & path to save original bs when backing up */
  BYTE *fnKernel;               /* optional override to source kernel filename (src only) */
  BYTE *fnCmd;                  /* optional override to cmd interpreter filename (src & dest) */
} SYSOptions;


void restoreBS(const char *, int);
void put_boot(SYSOptions *opts);
BOOL check_space(COUNT, ULONG);
BOOL copy(const BYTE *source, COUNT drive, const BYTE * filename);

void showHelpAndExit(void)
{
  printf(
      "Usage: \n"
      "%s [source] drive: [bootsect] [{option}]\n"
      "  source   = A:,B:,C:\\KERNEL\\BIN\\,etc., or current directory if not given\n"
      "  drive    = A,B,etc.\n"
      "  bootsect = name of 512-byte boot sector file image for drive:\n"
      "             to write to *instead* of real boot sector\n"
      "  {option} is one or more of the following:\n"
      "  /BOTH    : write to *both* the real boot sector and the image file\n"
      "  /BOOTONLY: do *not* copy kernel / shell, only update boot sector or image\n"
      "  /OEM     : indicates boot sector, filenames, and load segment to use\n"
      "             /OEM:FD use FreeDOS compatible settings\n"
      "             /OEM:DR use DR DOS 7+ compatible settings (same as /OEM)\n"
#ifdef WITHOEMCOMPATBS
      "             /OEM:PC use PC-DOS compatible settings\n"
      "             /OEM:MS use MS-DOS compatible settings\n"
#endif
      "             /OEM:W9x use MS Win9x DOS compatible settings\n"
      "             default is /OEM:AUTO, select DOS based on existing files\n"
      "  /K name  : name of kernel to use in boot sector instead of KERNEL.SYS\n"
      "  /L segm  : hex load segment to use in boot sector instead of 60\n"
      "  /B btdrv : hex BIOS # of boot drive set in bs, 0=A:, 80=1st hd,...\n"
      "  /FORCEDRV: force use of drive # set in bs instead of BIOS boot value\n"
      "%s CONFIG /help\n", pgm, pgm
  );
  exit(1);
}


/* get and validate arguments */
void initOptions(int argc, char *argv[], SYSOptions *opts)
{
  int argno;
  int drivearg = 0;           /* drive argument, position of 1st or 2nd non option */
  int srcarg = 0;             /* nonzero if optional source argument */
  BYTE srcFile[SYS_MAXPATH];  /* full path+name of [kernel] file [to copy] */
  struct stat fstatbuf;

  /* initialize to defaults */
  memset(opts, 0, sizeof(SYSOptions));
  /* set srcDrive and dstDrive after processing args */
  opts->flavor = OEM_AUTO;      /* attempt to detect DOS user wants to boot */
  opts->copyFiles = 1;          /* actually copy the kernel and cmd interpreter to dstDrive */

  /* cycle through processing cmd line arguments */
  for(argno = 1; argno < argc; argno++)
  {
    char *argp = argv[argno];

    if (argp[0] == '/')  /* optional switch */
    {
      argp++;  /* skip past the '/' character */

      /* explicit request for base help/usage */
      if ((*argp == '?') || (memicmp(argp, "HELP", 4) == 0))
      {
        showHelpAndExit();
      }
      /* write to *both* the real boot sector and the image file */
      else if (memicmp(argp, "BOTH", 4) == 0)
      {
        opts->writeBS = 1;  /* note: if bs file omitted, then same as omitting /BOTH */
      }
      /* do *not* copy kernel / shell, only update boot sector or image */
      else if (memicmp(argp, "BOOTONLY", 8) == 0)
      {
        opts->copyFiles = 0;
      }
      /* indicates compatibility mode, fs, filenames, and load segment to use */
      else if (memicmp(argp, "OEM", 3) == 0)
      {
        argp += 3;
        if (!*argp)
            opts->flavor = OEM_DR;
        else if (*argp == ':')
        {
          argp++;  /* point to DR/PC/MS that follows */
          if (memicmp(argp, "AUTO", 4) == 0)
            opts->flavor = OEM_AUTO;
          else if (memicmp(argp, "DR", 2) == 0)
            opts->flavor = OEM_DR;
#ifdef WITHOEMCOMPATBS
          else if (memicmp(argp, "PC", 2) == 0)
            opts->flavor = OEM_PC;
          else if (memicmp(argp, "MS", 2) == 0)
            opts->flavor = OEM_MS;
#endif
          else if (memicmp(argp, "W9", 2) == 0)
            opts->flavor = OEM_W9x;
          else /* if (memicmp(argp, "FD", 2) == 0) */
            opts->flavor = OEM_FD;
        }
        else
        {
          printf("%s: unknown OEM qualifier %s\n", pgm, argp);
          showHelpAndExit();
        }
      }
      /* force use of drive # set in bs instead of BIOS boot value */
      else if (memicmp(argp, "FORCEDRV", 8) == 0)
      {
        opts->ignoreBIOS = 1;
      }
      else if (argno + 1 < argc)   /* two part options, /SWITCH VALUE */
      {
        argno++;
        if (toupper(*argp) == 'K')      /* set Kernel name */
        {
          opts->kernel.kernel = argv[argno];
        }
        else if (toupper(*argp) == 'L') /* set Load segment */
        {
          opts->kernel.loadseg = (WORD)strtol(argv[argno], NULL, 16);
        }
        else if (memicmp(argp, "B", 2) == 0) /* set boot drive # */
        {
          opts->defBootDrive = (BYTE)strtol(argv[argno], NULL, 16);
        }
        /* options not documented by showHelpAndExit() */
        else if (memicmp(argp, "SKFN", 4) == 0) /* set KERNEL.SYS input file and /OEM:FD */
        {
          opts->flavor = OEM_FD;
          opts->fnKernel = argv[argno];
        }
        else if (memicmp(argp, "SCFN", 4) == 0) /* sets COMMAND.COM input file */
        {
          opts->fnCmd = argv[argno];
        }
        else if (memicmp(argp, "BACKUPBS", 8) == 0) /* save current bs before overwriting */
        {
          opts->bsFileOrig = argv[argno];
        }
        else if (memicmp(argp, "RESTORBS", 8) == 0) /* overwrite bs and exit */
        {
          if (drivearg)
            restoreBS(argv[argno], (BYTE)(toupper(*(argv[drivearg])) - 'A'));
          else
            printf("%s: unspecified drive, unable to restore boot sector\n", pgm);
          exit(1);
        }
        else
        {
          printf("%s: unknown option, %s\n", pgm, argv[argno]);
          showHelpAndExit();
        }
      }
      else
      {
        printf("%s: unknown option or missing parameter, %s\n", pgm, argv[argno]);
        showHelpAndExit();
      }
    }
    else if (!drivearg)
    {
      drivearg = argno;         /* either source or destination drive */
    }
    else if (!srcarg /* && drivearg */)
    {
      srcarg = drivearg; /* set source path */
      drivearg = argno;  /* set destination drive */
    }
    else if (!opts->bsFile /* && srcarg && drivearg */)
    {
      opts->bsFile = argv[argno];
    }
    else /* if (opts->bsFile && srcarg && drivearg) */
    {
      printf("%s: invalid argument %s\n", pgm, argv[argno]);
      showHelpAndExit();
    }
  }

  /* if neither BOTH nor a boot sector file specified, then write to boot record */
  if (!opts->bsFile)
    opts->writeBS = 1;

  /* set dest path */
  if (!drivearg)
    showHelpAndExit();
  opts->dstDrive = (BYTE)(toupper(*(argv[drivearg])) - 'A');
  if (/* (opts->dstDrive < 0) || */ (opts->dstDrive >= 26))
  {
    printf("%s: drive %c must be A:..Z:\n", pgm, *(argv[drivearg]));
    exit(1);
  }

  /* set source path, default to current drive */
  sprintf(opts->srcDrive, "%c:", 'A' + getcurdrive());
  if (srcarg)
  {
	int slen;
    /* set source path, reserving room to append filename */
    if ( (argv[srcarg][1] == ':') /* || ((argv[srcarg][0]=='\\') && (argv[srcarg][1] == '\\'))*/ ) 
      strncpy(opts->srcDrive, argv[srcarg], SYS_MAXPATH-13);
    else /* only path provided, append to default drive */
      strncat(opts->srcDrive, argv[srcarg], SYS_MAXPATH-15);
    slen = strlen(opts->srcDrive);
    /* if path follows drive, ensure ends in a slash, ie X:-->X: or X:.\mypath-->X:.\mypath\ */
    if ((slen>2) && (opts->srcDrive[slen-1] != '\\') && (opts->srcDrive[slen-1] != '/'))
      strcat(opts->srcDrive, "\\");
  }
  /* source path is now in form of just a drive, "X:" 
     or form of drive + path + directory separator, "X:\path\" or "\\path\"
     If just drive we try current path then root, else just indicated path.
  */


  /* if source and dest are same drive, then source should not be root,
     so if is same drive and not explicit path, force only current 
     Note: actual copy routine prevents overwriting self when src=dst
  */
  if ( (opts->dstDrive == (toupper(*(opts->srcDrive))-'A')) && (!opts->srcDrive[2]) )
    strcat(opts->srcDrive, ".\\");


  /* attempt to detect compatibility settings user needs */
  if (opts->flavor == OEM_AUTO)
  {
    /* 1st loop checking current just source path provided */
    for (argno = 0; argno < DOSFLAVORS; argno++)
    {
      /* look for existing file matching kernel filename */
      sprintf(srcFile, "%s%s", opts->srcDrive, bootFiles[argno].kernel);
      if (stat(srcFile, &fstatbuf)) continue; /* if !exists() try again */
      if (!fstatbuf.st_size) continue;  /* file must not be empty */

      /* now check if secondary file exists and of minimal size */
      if (bootFiles[argno].minsize)
      {
        sprintf(srcFile, "%s%s", opts->srcDrive, bootFiles[argno].dos);
        if (stat(srcFile, &fstatbuf)) continue;
        if (fstatbuf.st_size < bootFiles[argno].minsize) continue;
      }

      /* above criteria succeeded, so default to corresponding DOS */
      opts->flavor = argno;
      break;
    }

    /* if no match, and source just drive, try root */
    if ( (opts->flavor == OEM_AUTO) && (!opts->srcDrive[2]) )
    {
      for (argno = 0; argno < DOSFLAVORS; argno++)
      {
        /* look for existing file matching kernel filename */
        sprintf(srcFile, "%s\\%s", opts->srcDrive, bootFiles[argno].kernel);
        if (stat(srcFile, &fstatbuf)) continue; /* if !exists() try again */
        if (!fstatbuf.st_size) continue;  /* file must not be empty */

        /* now check if secondary file exists and of minimal size */
        if (bootFiles[argno].minsize)
        {
          sprintf(srcFile, "%s\\%s", opts->srcDrive, bootFiles[argno].dos);
          if (stat(srcFile, &fstatbuf)) continue;
          if (fstatbuf.st_size < bootFiles[argno].minsize) continue;
        }

        /* above criteria succeeded, so default to corresponding DOS */
        opts->flavor = argno;
        strcat(opts->srcDrive, "\\"); /* indicate to use root from now on */
        break;
      }
    }
  }

  /* if unable to determine DOS, assume FreeDOS */
  if (opts->flavor == OEM_AUTO) opts->flavor = OEM_FD;

  printf(msgDOS[opts->flavor]);

  /* set compatibility settings not explicitly set */
  if (!opts->kernel.kernel) opts->kernel.kernel = bootFiles[opts->flavor].kernel;
  if (!opts->kernel.dos) opts->kernel.dos = bootFiles[opts->flavor].dos;
  if (!opts->kernel.loadseg) opts->kernel.loadseg = bootFiles[opts->flavor].loadseg;
  opts->kernel.stdbs = bootFiles[opts->flavor].stdbs;
  opts->kernel.minsize = bootFiles[opts->flavor].minsize;


  /* if destination is floppy (A: or B:) then use drive # stored in boot sector */
  if (opts->dstDrive < 2)
    opts->ignoreBIOS = 1;

  /* if bios drive to store in boot sector not set and not floppy set to 1st hd */
  if (!opts->defBootDrive && (opts->dstDrive >= 2))
    opts->defBootDrive = 0x80;


  /* unless we are only setting boot sector, verify kernel file exists */
  if (opts->copyFiles)
  {
    /* check kernel (primary file) 1st */
    sprintf(srcFile, "%s%s", opts->srcDrive, (opts->fnKernel)?opts->fnKernel:opts->kernel.kernel);
    if (stat(srcFile, &fstatbuf))  /* if !exists() */
    {
      /* check root path as well if src is drive only */
      sprintf(srcFile, "%s\\%s", opts->srcDrive, (opts->fnKernel)?opts->fnKernel:opts->kernel.kernel);
      if (opts->srcDrive[2] || stat(srcFile, &fstatbuf))
      {
        printf("%s: failed to find kernel file %s\n", pgm, (opts->fnKernel)?opts->fnKernel:opts->kernel.kernel);
        exit(1);
      }
      /* else found, but in root, so force to always use root */
      strcat(opts->srcDrive, "\\");
    }

    /* now check for secondary file */
    if (opts->kernel.dos && opts->kernel.minsize)
    {
      sprintf(srcFile, "%s%s", opts->srcDrive, opts->kernel.dos);
      if (stat(srcFile, &fstatbuf))
      {
        printf("%s: failed to find source file %s\n", pgm, opts->kernel.dos);
        exit(1);
      }
      if (fstatbuf.st_size < opts->kernel.minsize)
      {
        printf("%s: source file %s appears corrupt, invalid size\n", pgm, opts->kernel.dos);
        exit(1);
      }
    }

    /* lastly check for command interpreter */
    sprintf(srcFile, "%s%s", opts->srcDrive, (opts->fnCmd)?opts->fnCmd:"COMMAND.COM");
    if (stat(srcFile, &fstatbuf))  /* if !exists() */
    {
      char *comspec = getenv("COMSPEC");
      if (opts->fnCmd || (comspec == NULL) || stat(comspec, &fstatbuf))
      {
        printf("%s: failed to find command interpreter (shell) file %s\n", pgm, (opts->fnCmd)?opts->fnCmd:"COMMAND.COM");
        exit(1);
      }
    }
  }
}

int main(int argc, char **argv)
{
  SYSOptions opts;            /* boot options and other flags */
  BYTE srcFile[SYS_MAXPATH];  /* full path+name of [kernel] file [to copy] */

  printf("FreeDOS System Installer " SYS_VERSION ", " __DATE__ "\n");

  if (argc > 1 && memicmp(argv[1], "CONFIG", 6) == 0)
  {
    exit(FDKrnConfigMain(argc, argv));
  }

  initOptions(argc, argv, &opts);

  printf("Processing boot sector...\n");
  put_boot(&opts);

  if (opts.copyFiles)
  {
    printf("Now copying system files...\n");

    sprintf(srcFile, "%s%s", opts.srcDrive, (opts.fnKernel)?opts.fnKernel:opts.kernel.kernel);
    if (!copy(srcFile, opts.dstDrive, opts.kernel.kernel))
    {
      printf("%s: cannot copy \"%s\"\n", pgm, srcFile);
      exit(1);
    } /* copy kernel */

    if (opts.kernel.dos)
    {
      sprintf(srcFile, "%s%s", opts.srcDrive, opts.kernel.dos);
      if (!copy(srcFile, opts.dstDrive, opts.kernel.dos) && opts.kernel.minsize)
      {
        printf("%s: cannot copy \"%s\"\n", pgm, srcFile);
        exit(1);
      } /* copy secondary file (DOS) */
    }

    /* copy command.com, 1st try source path, then try %COMSPEC% */
    sprintf(srcFile, "%s%s", opts.srcDrive, (opts.fnCmd)?opts.fnCmd:"COMMAND.COM");
    if (!copy(srcFile, opts.dstDrive, "COMMAND.COM"))
    {
      char *comspec = getenv("COMSPEC");
      if (!opts.fnCmd && (comspec != NULL))
        printf("%s: Trying shell from %COMSPEC%=\"%s\"\n", pgm, comspec);
      if (opts.fnCmd || (comspec == NULL) || !copy(comspec, opts.dstDrive, "COMMAND.COM"))
      {
        printf("\n%s: failed to find command interpreter (shell) file %s\n", pgm, (opts.fnCmd)?opts.fnCmd:"COMMAND.COM");
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

int generic_block_ioctl(unsigned drive, unsigned cx, unsigned char *par);
#pragma aux generic_block_ioctl = \
      "mov ax, 0x440d" \
      "int 0x21" \
      "sbb ax, ax" \
      value [ax] \
      parm [bx] [cx] [dx]; /* BH must be 0 for lock! */

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

int fat32readwrite(int DosDrive, void *diskReadPacket, unsigned intno)
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

int generic_block_ioctl(unsigned drive, unsigned cx, unsigned char *par)
{
  union REGS regs;

  regs.x.ax = 0x440d;
  regs.x.cx = cx;
  regs.x.dx = (unsigned)par;
  regs.x.bx = drive; /* BH must be 0 for lock! */
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

/* reads in boot sector (1st SEC_SIZE bytes) from file */
void readBS(const char *bsFile, UBYTE *bootsector)
{
  if (bsFile != NULL)
  {
    int fd;

#ifdef DEBUG
    printf("reading bootsector from file %s\n", bsFile);
#endif

    /* open boot sector file, it must exists, then overwrite
       drive with 1st SEC_SIZE bytes from the [image] file
    */
    if ((fd = open(bsFile, O_RDONLY | O_BINARY)) < 0)
    {
      printf("%s: can't open\"%s\"\nDOS errnum %d", pgm, bsFile, errno);
      exit(1);
    }
    if (read(fd, bootsector, SEC_SIZE) != SEC_SIZE)
    {
      printf("%s: failed to read %u bytes from %s\n", pgm, SEC_SIZE, bsFile);
      close(fd);
      exit(1);
    }
    close(fd);
  }
}

/* write bs in bsFile to drive's boot record unmodified */
void restoreBS(const char *bsFile, int drive)
{
  UBYTE bootsector[SEC_SIZE];

  if (bsFile == NULL)
  {
    printf("%s: missing filename of boot sector to restore\n", pgm);
    exit(1);
  }

  readBS(bsFile, bootsector);

  /* lock drive */
  generic_block_ioctl(drive + 1, 0x84a, NULL);

  reset_drive(drive);

  /* write bootsector to drive */
  if (MyAbsReadWrite(drive, 1, 0, bootsector, 1) != 0)
  {
    printf("%s: failed to write boot sector to drive %c:\n", pgm, drive + 'A');
    exit(1);
  }

  reset_drive(drive);

  /* unlock_drive */
  generic_block_ioctl(drive + 1, 0x86a, NULL);
}

/* write bootsector to file bsFile */
void saveBS(const char *bsFile, UBYTE *bootsector)
{
  if (bsFile != NULL)
  {
    int fd;

#ifdef DEBUG
    printf("writing bootsector to file %s\n", bsFile);
#endif

    /* open boot sector file, create it if not exists,
       but don't truncate if exists so we can replace
       1st SEC_SIZE bytes of an image file
    */
    if ((fd = open(bsFile, O_WRONLY | O_CREAT | O_BINARY,
              S_IREAD | S_IWRITE)) < 0)
    {
      printf("%s: can't create\"%s\"\nDOS errnum %d", pgm, bsFile, errno);
      exit(1);
    }
    if (write(fd, bootsector, SEC_SIZE) != SEC_SIZE)
    {
      printf("%s: failed to write %u bytes to %s\n", pgm, SEC_SIZE, bsFile);
      close(fd);
      /* unlink(bsFile); don't delete in case was image */
      exit(1);
    }
    close(fd);
  } /* if write boot sector file */
}

void put_boot(SYSOptions *opts)
{
#ifdef WITHFAT32
  struct bootsectortype32 *bs32;
#endif
  struct bootsectortype *bs;
  static unsigned char oldboot[SEC_SIZE], newboot[SEC_SIZE];
  static unsigned char default_bpb[0x5c];
  int bsBiosMovOff;  /* offset in bs to mov [drive],dl that we NOP out */

#ifdef DEBUG
  printf("Reading old bootsector from drive %c:\n", opts->dstDrive + 'A');
#endif

  /* lock drive */
  generic_block_ioctl(opts->dstDrive + 1, 0x84a, NULL);

  reset_drive(opts->dstDrive);

  /* suggestion: allow reading from a boot sector or image file here */
  if (MyAbsReadWrite(opts->dstDrive, 1, 0, oldboot, 0) != 0)
  {
    printf("%s: can't read old boot sector for drive %c:\n", pgm, opts->dstDrive + 'A');
    exit(1);
  }

#ifdef DDEBUG
  printf("Old Boot Sector:\n");
  dump_sector(oldboot);
#endif

  /* backup original boot sector when requested */
  if (opts->bsFileOrig)
  {
    printf("Backing up original boot sector to %s\n", opts->bsFileOrig);
    saveBS(opts->bsFileOrig, oldboot);
  }

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

    bs32 = (struct bootsectortype32 *)bs;
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
    if (opts->dstDrive >= 2 &&
        generic_block_ioctl(opts->dstDrive + 1, 0x4860, default_bpb) == 0)
      correct_bpb((struct bootsectortype *)(default_bpb + 7 - 11), bs);

#ifdef WITHFAT32                /* copy one of the FAT32 boot sectors */
    if (!opts->kernel.stdbs)    /* MS/PC DOS compatible BS requested */
    {
      printf("%s: FAT32 versions of PC/MS DOS compatible boot sectors\n"
             "are not supported.\n", pgm);
      exit(1);
    }
 
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
    if (opts->dstDrive >= 2 &&
        generic_block_ioctl(opts->dstDrive + 1, 0x860, default_bpb) == 0)
      correct_bpb((struct bootsectortype *)(default_bpb + 7 - 11), bs);

    if (opts->kernel.stdbs)
	{
      memcpy(newboot, (fs == FAT16) ? fat16com : fat12com, SEC_SIZE);
	}
    else
    {
      printf("Using OEM (PC/MS-DOS) compatible boot sector.\n");
      memcpy(newboot, (fs == FAT16) ? oemfat16 : oemfat12, SEC_SIZE);
    }
  }

  /* Copy disk parameter from old sector to new sector */
#ifdef WITHFAT32
  if (fs == FAT32)
    memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE32);
  else
#endif
    memcpy(&newboot[SBOFFSET], &oldboot[SBOFFSET], SBSIZE);

  bs = (struct bootsectortype *)&newboot;

  /* originally OemName was "FreeDOS", changed for better compatibility */
  memcpy(bs->OemName, "FRDOS4.1", 8);

#ifdef WITHFAT32
  if (fs == FAT32)
  {
    bs32 = (struct bootsectortype32 *)&newboot;
    bs32->bsDriveNumber = opts->defBootDrive;

    /* the location of the "0060" segment portion of the far pointer
       in the boot sector is just before cont: in boot*.asm.
       This happens to be offset 0x78 for FAT32 and offset 0x5c for FAT16 

       force use of value stored in bs by NOPping out mov [drive], dl
       0x82: 88h,56h,40h for fat32 chs & lba boot sectors

       i.e. BE CAREFUL WHEN YOU CHANGE THE BOOT SECTORS !!! 
    */
    if (opts->kernel.stdbs)
    {
      ((int *)newboot)[0x78/sizeof(int)] = opts->kernel.loadseg;
      bsBiosMovOff = 0x82;
    }
    else /* compatible bs */
    {
      printf("%s: INTERNAL ERROR: how did you get here?\n", pgm);
      exit(1);
    }

#ifdef DEBUG
    printf(" FAT starts at sector %lx + %x\n",
           bs32->bsHiddenSecs, bs32->bsResSectors);
#endif
  }
  else
#endif
  {

    /* establish default BIOS drive # set in boot sector */
    bs->bsDriveNumber = opts->defBootDrive;

    /* the location of the "0060" segment portion of the far pointer
       in the boot sector is just before cont: in boot*.asm.
       This happens to be offset 0x78 for FAT32 and offset 0x5c for FAT16 

       force use of value stored in bs by NOPping out mov [drive], dl
       0x66: 88h,56h,24h for fat16 and fat12 boot sectors
       0x4F: 88h,56h,24h for oem compatible fat16 and fat12 boot sectors

       i.e. BE CAREFUL WHEN YOU CHANGE THE BOOT SECTORS !!! 
    */
    if (opts->kernel.stdbs)
    {
      ((int *)newboot)[0x5c/sizeof(int)] = opts->kernel.loadseg;
      bsBiosMovOff = 0x66;
    }
    else
    {
      /* load segment hard coded to 0x70 in oem compatible boot sector */
      if (opts->kernel.loadseg != 0x70)
        printf("%s: Warning: ignoring load segment, compat bs always uses 0x70!\n");
      bsBiosMovOff = 0x4F;
    }
  }

  if (opts->ignoreBIOS)
  {
    if ( (newboot[bsBiosMovOff]==0x88) && (newboot[bsBiosMovOff+1]==0x56) )
    {
      newboot[bsBiosMovOff] = 0x90;  /* NOP */  ++bsBiosMovOff;
      newboot[bsBiosMovOff] = 0x90;  /* NOP */  ++bsBiosMovOff;
      newboot[bsBiosMovOff] = 0x90;  /* NOP */  ++bsBiosMovOff;
    }
    else
    {
      printf("%s : fat boot sector does not match expected layout\n", pgm);
      exit(1);
    }
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
    while (opts->kernel.kernel[i] && opts->kernel.kernel[i] != '.')
    {
      if (i < 8)
        newboot[0x1f1+i] = toupper(opts->kernel.kernel[i]);
      i++;
    }
    if (opts->kernel.kernel[i] == '.')
    {
      /* copy extension */
      int j = 0;
      i++;
      while (opts->kernel.kernel[i+j] && j < 3)
      {
        newboot[0x1f9+j] = toupper(opts->kernel.kernel[i+j]);
        j++;
      }
    }
  }

#ifdef DEBUG
  /* there's a zero past the kernel name in all boot sectors */
  printf("Boot sector kernel name set to %s\n", &newboot[0x1f1]);
  printf("Boot sector load segment set to %Xh\n", opts->kernel.loadseg);
#endif

#ifdef DDEBUG
  printf("\nNew Boot Sector:\n");
  dump_sector(newboot);
#endif

  if (opts->writeBS)
  {
#ifdef DEBUG
    printf("writing new bootsector to drive %c:\n", opts->dstDrive + 'A');
#endif

    /* write newboot to a drive */
    if (MyAbsReadWrite(opts->dstDrive, 1, 0, newboot, 1) != 0)
    {
      printf("Can't write new boot sector to drive %c:\n", opts->dstDrive + 'A');
      exit(1);
    }
  } /* if write boot sector to boot record*/

  if (opts->bsFile != NULL)
  {
#ifdef DEBUG
    printf("writing new bootsector to file %s\n", opts->bsFile);
#endif

    saveBS(opts->bsFile, newboot);
  } /* if write boot sector to file*/

  reset_drive(opts->dstDrive);

  /* unlock_drive */
  generic_block_ioctl(opts->dstDrive + 1, 0x86a, NULL);
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

/* copies file (path+filename specified by srcFile) to drive:\filename */
BOOL copy(const BYTE *source, COUNT drive, const BYTE * filename)
{
  static BYTE src[SYS_MAXPATH];
  static BYTE dest[SYS_MAXPATH];
  unsigned ret;
  int fdin, fdout;
  ULONG copied = 0;

  printf("Copying %s...\n", source);

  truename(src, source);
  sprintf(dest, "%c:\\%s", 'A' + drive, filename);
  if (stricmp(src, dest) == 0)
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
    printf("%s: Not enough space to transfer %s\n", pgm, filename);
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

