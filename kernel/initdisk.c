/****************************************************************/
/*                                                              */
/*                            initDISK.c                        */
/*                                                              */
/*                      Copyright (c) 2001                      */
/*                      tom ehlert                              */
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

#include "portab.h"
#include "debug.h"
#include "init-mod.h"
#include "dyndata.h"

#define FLOPPY_SEC_SIZE 512u  /* common sector size */

UBYTE InitDiskTransferBuffer[MAX_SEC_SIZE] BSS_INIT({0});
COUNT nUnits BSS_INIT(0);

/*
 *    Rev 1.0   13 May 2001  tom ehlert
 * Initial revision.
 *
 * this module implements the disk scanning for DOS accesible partitions
 * the drive letter ordering is somewhat chaotic, but like MSDOS does it.
 *
 * this module expects to run with CS = INIT_TEXT, like other init_code,
 * but SS = DS = DATA = DOS_DS, unlike other init_code.
 *
 * history:
 * 1.0 extracted the disk init code from DSK.C
 *     added LBA support
 *     moved code to INIT_TEXT
 *     done the funny code segment stuff to switch between INIT_TEXT and TEXT
 *     added a couple of snity checks for partitions
 *
 ****************************************************************************
 *
 * Implementation note:
 * this module needs some interfacing to INT 13
 * how to implement them
 *    
 * a) using inline assembly 
 *        _ASM mov ax,0x1314
 *
 * b) using assembly routines in some external FLOPPY.ASM
 *
 * c) using the funny TURBO-C style
 *        _AX = 0x1314
 *
 * d) using intr(intno, &regs) method.
 *
 * why not?
 *
 * a) this is my personal favorite, combining the best aof all worlds.
 *    TURBO-C does support inline assembly, but only by using TASM,
 *    which is not free. 
 *    so - unfortunately- its excluded.
 *
 * b) keeping funny memory model in sync with external assembly
 *    routines is everything, but not fun
 *
 * c) you never know EXACT, what the compiler does, if its a bit
 *    more complicated. does
 *      _DL = drive & 0xff    
 *      _BL = driveParam.chs.Sector;
 *    destroy any other register? sure? _really_ sure?
 *    at least, it has it's surprises.
 *    and - I found a couple of optimizer induced bugs (TC 2.01)
 *      believe me.
 *    it was coded - and operational that way.
 *    but - there are many surprises waiting there. so I opted against.
 *
 *
 * d) this method is somewhat clumsy and certainly not the
 *    fastest way to do things.
 *    on the other hand, this is INIT code, executed once.
 *    and since it's the only portable method, I opted for it.
 *
 * e) and all this is my private opinion. tom ehlert.
 *
 *
 * Some thoughts about LBA vs. CHS. by Bart Oldeman 2001/Nov/11
 * Matthias Paul writes in www.freedos.org/freedos/news/technote/113.html:
 * (...) MS-DOS 7.10+, which will always access logical drives in a type
 * 05h extended partition via CHS, even if the individual logical drives
 * in there are of LBA type, or go beyond 8 Gb... (Although this workaround
 * is sometimes used in conjunction with OS/2, using a 05h partition going
 * beyond 8 Gb may cause MS-DOS 7.10 to hang or corrupt your data...) (...)
 *
 * Also at http://www.win.tue.nl/~aeb/partitions/partition_types-1.html:
 * (...) 5 DOS 3.3+ Extended Partition
 *   Supports at most 8.4 GB disks: with type 5 DOS/Windows will not use the
 *   extended BIOS call, even if it is available. (...)
 *
 * So MS-DOS 7.10+ is brain-dead in this respect, but we knew that ;-)
 * However there is one reason to use old-style CHS calls:
 * some programs intercept int 13 and do not support LBA addressing. So
 * it is worth using CHS if possible, unless the user asks us not to,
 * either by specifying a 0x0c/0x0e/0x0f partition type or enabling
 * the ForceLBA setting in the fd kernel (sys) config. This will make
 * multi-sector reads and BIOS computations more efficient, at the cost
 * of some compatibility.
 *
 * However we need to be safe, and with varying CHS at different levels
 * that might be difficult. Hence we _only_ trust the LBA values in the
 * partition tables and the heads and sectors values the BIOS gives us.
 * After all these are the values the BIOS uses to process our CHS values.
 * So unless the BIOS is buggy, using CHS on one partition and LBA on another
 * should be safe. The CHS values in the partition table are NOT trusted.
 * We print a warning if there is a mismatch with the calculated values.
 *
 * The CHS values in the boot sector are used at a higher level. The CHS
 * that DOS uses in various INT21/AH=44 IOCTL calls are converted to LBA
 * using the boot sector values and then converted back to CHS using BIOS
 * values if necessary. Internally we do LBA as much as possible.
 *
 * However if the partition extends beyond cylinder 1023 and is not labelled
 * as one of the LBA types, we can't use CHS and print a warning, using LBA
 * instead if possible, and otherwise refuse to use it.
 *
 * As for EXTENDED_LBA vs. EXTENDED, FreeDOS makes no difference. This is
 * boot time - there is no reason not to use LBA for reading partition tables,
 * and the MSDOS 7.10 behaviour is not desirable.
 *
 * Note: for floppies we need the boot sector values though and the boot sector
 * code does not use LBA addressing yet.
 *
 * Conclusion: with all this implemented, FreeDOS should be able to gracefully
 * handle and read foreign hard disks moved across computers, whether using
 * CHS or LBA, strengthening its role as a rescue environment.
 */

#define LBA_to_CHS   init_LBA_to_CHS

/*
    interesting macros - used internally only
*/

#define SCAN_PRIMARYBOOT 0x00
#define SCAN_PRIMARY     0x01
#define SCAN_EXTENDED    0x02
#define SCAN_PRIMARY2    0x03

#define FAT12           0x01
#define FAT16SMALL      0x04
#define EXTENDED        0x05
#define FAT16LARGE      0x06
#define FAT32           0x0b    /* FAT32 partition that ends before the 8.4  */
                              /* GB boundary                               */
#define FAT32_LBA       0x0c    /* FAT32 partition that ends after the 8.4GB */
                              /* boundary.  LBA is needed to access this.  */
#define FAT16_LBA       0x0e    /* like 0x06, but it is supposed to end past */
                              /* the 8.4GB boundary                        */
#define FAT12_LBA       0xff    /* fake FAT12 LBA entry for internal use     */
#define EXTENDED_LBA    0x0f    /* like 0x05, but it is supposed to end past */

/* Let's play it safe and do not allow partitions with clusters above  *
 * or equal to 0xff0/0xfff0/0xffffff0 to be created                    *
 * the problem with fff0-fff6 is that they might be interpreted as BAD *
 * even though the standard BAD value is ...ff7                        */

#define FAT12MAX        (FAT_MAGIC-6)
#define FAT16MAX        (FAT_MAGIC16-6)
#define FAT32MAX        (FAT_MAGIC32-6)

#define IsExtPartition(parttyp) ((parttyp) == EXTENDED || \
                                 (parttyp) == EXTENDED_LBA )

#define IsLBAPartition(parttyp) ((parttyp) == FAT12_LBA  || \
                                 (parttyp) == FAT16_LBA  || \
                                 (parttyp) == FAT32_LBA)

#ifdef WITHFAT32
#define IsFATPartition(parttyp) ((parttyp) == FAT12      || \
                                 (parttyp) == FAT16SMALL || \
                                 (parttyp) == FAT16LARGE || \
                                 (parttyp) == FAT16_LBA  || \
                                 (parttyp) == FAT32      || \
                                 (parttyp) == FAT32_LBA)
#else
#define IsFATPartition(parttyp) ((parttyp) == FAT12      || \
                                 (parttyp) == FAT16SMALL || \
                                 (parttyp) == FAT16LARGE || \
                                 (parttyp) == FAT16_LBA)
#endif

#define MSDOS_EXT_SIGN 0x29     /* extended boot sector signature */
#define MSDOS_FAT12_SIGN "FAT12   "     /* FAT12 filesystem signature */
#define MSDOS_FAT16_SIGN "FAT16   "     /* FAT16 filesystem signature */
#define MSDOS_FAT32_SIGN "FAT32   "     /* FAT32 filesystem signature */

/* local - returned and used for BIOS interface INT 13, AH=48*/
struct _bios_LBA_disk_parameterS {
  UWORD size;
  UWORD information;
  ULONG cylinders;
  ULONG heads;
  ULONG sectors;

  ULONG totalSect;
  ULONG totalSectHigh;
  UWORD BytesPerSector;

  ULONG eddparameters;
};

/* physical characteristics of a drive */

struct DriveParamS {
  UBYTE driveno;                /* = 0x8x                           */
  UWORD descflags;
  ULONG total_sectors;

  struct CHS chs;               /* for normal   INT 13 */
};

struct PartTableEntry           /* INTERNAL representation of partition table entry */
{
  UBYTE Bootable;
  UBYTE FileSystem;
  struct CHS Begin;
  struct CHS End;
  ULONG RelSect;
  ULONG NumSect;
};

/*
    internal global data
*/

BOOL ExtLBAForce = FALSE;

COUNT init_readdasd(UBYTE drive)
{
  static iregs regs;

  regs.a.b.h = 0x15;
  regs.d.b.l = drive;
  init_call_intr(0x13, &regs);
  if ((regs.flags & 1) == 0)
    switch (regs.a.b.h)
    {
      case 2:
        return DF_CHANGELINE;
      case 3:
        return DF_FIXED;
    }
  return 0;
}

typedef struct {
  UWORD bpb_nbyte;              /* Bytes per Sector             */
  UBYTE bpb_nsector;            /* Sectors per Allocation Unit  */
  UWORD bpb_nreserved;          /* # Reserved Sectors           */
  UBYTE bpb_nfat;               /* # FATs                       */
  UWORD bpb_ndirent;            /* # Root Directory entries     */
  UWORD bpb_nsize;              /* Size in sectors              */
  UBYTE bpb_mdesc;              /* MEDIA Descriptor Byte        */
  UWORD bpb_nfsect;             /* FAT size in sectors          */
  UWORD bpb_nsecs;              /* Sectors per track            */
  UWORD bpb_nheads;             /* Number of heads              */
} floppy_bpb;

floppy_bpb floppy_bpbs[5] = {
/* copied from Brian Reifsnyder's FORMAT, bpb.h */
  {FLOPPY_SEC_SIZE, 2, 1, 2, 112, 720, 0xfd, 2, 9, 2}, /* FD360  5.25 DS   */
  {FLOPPY_SEC_SIZE, 1, 1, 2, 224, 2400, 0xf9, 7, 15, 2},       /* FD1200 5.25 HD   */
  {FLOPPY_SEC_SIZE, 2, 1, 2, 112, 1440, 0xf9, 3, 9, 2},        /* FD720  3.5  LD   */
  {FLOPPY_SEC_SIZE, 1, 1, 2, 224, 2880, 0xf0, 9, 18, 2},       /* FD1440 3.5  HD   */
  {FLOPPY_SEC_SIZE, 2, 1, 2, 240, 5760, 0xf0, 9, 36, 2}        /* FD2880 3.5  ED   */
};

COUNT init_getdriveparm(UBYTE drive, bpb * pbpbarray)
{
  static iregs regs;
  REG UBYTE type;

  if (drive & 0x80)
    return 5;
  regs.a.b.h = 0x08;
  regs.d.b.l = drive;
  init_call_intr(0x13, &regs);
  type = regs.b.b.l - 1;
  if (regs.flags & 1)
    type = 0;                   /* return 320-360 for XTs */
  else if (type > 6)
    type = 8;                   /* any odd ball drives get 8&7=0: the 320-360 table */
  else if (type == 5)
    type = 4;                   /* 5 and 4 are both 2.88 MB */

  memcpy(pbpbarray, &floppy_bpbs[type & 7], sizeof(floppy_bpb));
  ((bpb *)pbpbarray)->bpb_hidden = 0;  /* very important to init to 0, see bug#1789 */
  ((bpb *)pbpbarray)->bpb_huge = 0;

  if (type == 3)
    return 7;                   /* 1.44 MB */

  if (type == 4)
    return 9;                   /* 2.88 almost forgot this one */

  /* 0=320-360kB, 1=1.2MB, 2=720kB, 8=any odd ball drives */
  return type;
}

/*
    translate LBA sectors into CHS addressing
    initially copied and pasted from dsk.c!

    LBA to/from CHS conversion - see http://www.ata-atapi.com/ How It Works section on CHSxlat - CHS Translation
    LBA (logical block address) simple 0 to N-1 used internally and with extended int 13h (BIOS)
    L-CHS (logical CHS) is the CHS view when using int 13h (BIOS)
    P-CHS (physical CHS) is the CHS view when directly accessing disk, should not, but could be used in BS or MBR

    LBA = ( (cylinder * heads_per_cylinder + heads ) * sectors_per_track ) + sector - 1

    cylinder = LBA / (heads_per_cylinder * sectors_per_track)
        temp = LBA % (heads_per_cylinder * sectors_per_track)
        head = temp / sectors_per_track
      sector = temp % sectors_per_track + 1

    where heads_per_cylinder and sectors_per_track are the current translation mode values.
    cyclinder and heads are 0 to N-1 based, sector is 1 to N based
*/

void init_LBA_to_CHS(struct CHS *chs, ULONG LBA_address,
                     struct DriveParamS *driveparam)
{
  unsigned hs = driveparam->chs.Sector * driveparam->chs.Head;
  unsigned hsrem = (unsigned)(LBA_address % hs);
  
  LBA_address /= hs;

  chs->Cylinder = LBA_address >= 0x10000ul ? 0xffffu : (unsigned)LBA_address;
  chs->Head = hsrem / driveparam->chs.Sector;
  chs->Sector = hsrem % driveparam->chs.Sector + 1;
}

void printCHS(char *title, struct CHS *chs)
{
  /* has no fixed size for head/sect: is often 1/1 in our context */
  printf("%s%4u-%u-%u", title, chs->Cylinder, chs->Head, chs->Sector);
}

/*
    reason for this modules existence:
    
    we have found a partition, and add them to the global 
    partition structure.

*/

/* Compute ceil(a/b) */
#define cdiv(a, b) (((a) + (b) - 1) / (b))

/* calculates FAT data:
   code adapted by Bart Oldeman from mkdosfs from the Linux dosfstools:
      Author:       Dave Hudson
      Updated by:   Roman Hodek
      Portions copyright 1992, 1993 Remy Card
      and 1991 Linus Torvalds
*/
/* defaults: */
#define MAXCLUSTSIZE 128
#define NSECTORFAT12 8
#define NFAT 2

VOID CalculateFATData(ddt * pddt, ULONG NumSectors, UBYTE FileSystem)
{
  ULONG fatdata;

  bpb *defbpb = &pddt->ddt_defbpb;

  /* FAT related items */
  defbpb->bpb_nfat = NFAT;
  /* normal value of number of entries in root dir */
  defbpb->bpb_ndirent = 512;
  defbpb->bpb_nreserved = 1;
  /* SEC_SIZE * DIRENT_SIZE / defbpb->bpb_ndirent + defbpb->bpb_nreserved */
  fatdata = NumSectors - (DIRENT_SIZE + 1);
  if (FileSystem == FAT12 || FileSystem == FAT12_LBA)
  {
    unsigned fatdat;
    /* in DOS, FAT12 defaults to 4096kb (8 sector) - clusters. */
    defbpb->bpb_nsector = NSECTORFAT12;
    /* Force maximal fatdata=32696 sectors since with our only possible sector
       size (512 bytes) this is the maximum for 4k clusters.
       #clus*secperclus+#fats*fatlength= 4077 * 8 + 2 * 12 = 32640.
       max FAT12 size for FreeDOS = 16,728,064 bytes */
    fatdat = (unsigned)fatdata;
    if (fatdata > 32640)
      fatdat = 32640;
    /* The "+2*NSECTORFAT12" is for the reserved first two FAT entries */
    defbpb->bpb_nfsect = (UWORD)cdiv((fatdat + 2 * NSECTORFAT12) * 3UL,
                                     FLOPPY_SEC_SIZE * 2 * NSECTORFAT12 + NFAT*3);
#ifdef DEBUG
    /* Need to calculate number of clusters, since the unused parts of the
     * FATS and data area together could make up space for an additional,
     * not really present cluster.
     * (This is really done in fatfs.c, bpbtodpb) */
    {
      unsigned clust = (fatdat - 2 * defbpb->bpb_nfsect) / NSECTORFAT12;
      unsigned maxclust = (defbpb->bpb_nfsect * 2 * SEC_SIZE) / 3;
      if (maxclust > FAT12MAX)
        maxclust = FAT12MAX;
      printf("FAT12: #clu=%u, fatlength=%u, maxclu=%u, limit=%u\n",
             clust, defbpb->bpb_nfsect, maxclust, FAT12MAX);
      if (clust > maxclust - 2)
      {
        clust = maxclust - 2;
        printf("FAT12: too many clusters: setting to maxclu-2\n");
      }
    }
#endif
    memcpy(pddt->ddt_fstype, MSDOS_FAT12_SIGN, 8);
  }
  else
  { /* FAT16/FAT32 */
    CLUSTER fatlength, maxcl;
    unsigned long clust, maxclust;
    unsigned fatentpersec;
    unsigned divisor;

#ifdef WITHFAT32
    if (FileSystem == FAT32 || FileSystem == FAT32_LBA)
    {
      /* For FAT32, use the cluster size table described in the FAT spec:
       * http://www.microsoft.com/hwdev/download/hardware/fatgen103.pdf
       */
      unsigned sz_gb = (unsigned)(NumSectors / 2097152UL);
      unsigned char nsector = 64; /* disks greater than 32 GB, 32K cluster */
      if (sz_gb <= 32)            /* disks up to 32 GB, 16K cluster */
        nsector = 32;
      if (sz_gb <= 16)            /* disks up to 16 GB, 8K cluster */
        nsector = 16;
      if (sz_gb <= 8)             /* disks up to 8 GB, 4K cluster */
        nsector = 8;
      if (NumSectors <= 532480UL)   /* disks up to 260 MB, 0.5K cluster */
        nsector = 1;
      defbpb->bpb_nsector = nsector;
      defbpb->bpb_ndirent = 0;
      defbpb->bpb_nreserved = 0x20;
      fatdata = NumSectors - 0x20;
      fatentpersec = FLOPPY_SEC_SIZE/4;
      maxcl = FAT32MAX;
    }
    else
#endif
    {
      /* FAT16: start at 4 sectors per cluster */
      defbpb->bpb_nsector = 4;
      /* Force maximal fatdata=8387584 sectors (NumSectors=8387617)
         since with our only possible sectorsize (512 bytes) this is the
         maximum we can address with 64k clusters
         #clus*secperclus+#fats*fatlength=65517 * 128 + 2 * 256=8386688.
         max FAT16 size for FreeDOS = 4,293,984,256 bytes = 4GiB-983,040 */
      if (fatdata > 8386688ul)
        fatdata = 8386688ul;
      fatentpersec = FLOPPY_SEC_SIZE/2;
      maxcl = FAT16MAX;
    }

    DebugPrintf(("%ld sectors for FAT+data, starting with %d sectors/cluster\n", fatdata, defbpb->bpb_nsector));
    do
    {
      DebugPrintf(("Trying with %d sectors/cluster:\n", defbpb->bpb_nsector));
      divisor = fatentpersec * defbpb->bpb_nsector + NFAT;
      fatlength = (CLUSTER)((fatdata + (2 * defbpb->bpb_nsector + divisor - 1))/
                            divisor);
      /* Need to calculate number of clusters, since the unused parts of the
       * FATS and data area together could make up space for an additional,
       * not really present cluster. */
      clust = (fatdata - NFAT * fatlength) / defbpb->bpb_nsector;
      maxclust = fatlength * fatentpersec;
      if (maxclust > maxcl)
        maxclust = maxcl;
      DebugPrintf(("FAT: #clu=%lu, fatlen=%lu, maxclu=%lu, limit=%lu\n",
                   clust, fatlength, maxclust, maxcl));
      if (clust > maxclust - 2)
      {
        clust = 0;
        DebugPrintf(("FAT: too many clusters\n"));
      }
      else if (clust <= FAT_MAGIC)
      {
        /* The <= 4086 avoids that the filesystem will be misdetected as having a
         * 12 bit FAT. */
        DebugPrintf(("FAT: would be misdetected as FAT12\n"));
        clust = 0;
      }
      if (clust)
        break;
      defbpb->bpb_nsector <<= 1;
    }
    while (defbpb->bpb_nsector && defbpb->bpb_nsector <= MAXCLUSTSIZE);
#ifdef WITHFAT32
    if (FileSystem == FAT32 || FileSystem == FAT32_LBA)
    {
      defbpb->bpb_nfsect = 0;
      defbpb->bpb_xnfsect = fatlength;
      /* set up additional FAT32 fields */
      defbpb->bpb_xflags = 0;
      defbpb->bpb_xfsversion = 0;
      defbpb->bpb_xrootclst = 2;
      defbpb->bpb_xfsinfosec = 1;
      defbpb->bpb_xbackupsec = 6;
      memcpy(pddt->ddt_fstype, MSDOS_FAT32_SIGN, 8);
    }
    else
#endif
    {
      defbpb->bpb_nfsect = (UWORD)fatlength;
      memcpy(pddt->ddt_fstype, MSDOS_FAT16_SIGN, 8);
    }
  }
  pddt->ddt_fstype[8] = '\0';
}

STATIC void push_ddt(ddt *pddt)
{
  ddt FAR *fddt = DynAlloc("ddt", 1, sizeof(ddt));
  fmemcpy(fddt, pddt, sizeof(ddt));
  if (pddt->ddt_logdriveno != 0) {
    (fddt - 1)->ddt_next = fddt;
    if (pddt->ddt_driveno == 0 && pddt->ddt_logdriveno == 1)
      (fddt - 1)->ddt_descflags |= DF_CURLOG | DF_MULTLOG;
  }
}

void DosDefinePartition(struct DriveParamS *driveParam,
                        ULONG StartSector, struct PartTableEntry *pEntry,
                        int extendedPartNo, int PrimaryNum)
{
  ddt nddt;
  ddt *pddt = &nddt;
  struct CHS chs;

  if (nUnits >= NDEV)
  {
    printf("more Partitions detected then possible, max = %d\n", NDEV);
    return;                     /* we are done */
  }

  pddt->ddt_next = MK_FP(0, 0xffff);
  pddt->ddt_driveno = driveParam->driveno;
  pddt->ddt_logdriveno = nUnits;
  pddt->ddt_descflags = driveParam->descflags;
  /* Turn of LBA if not forced and the partition is within 1023 cyls and of the right type */
  /* the FileSystem type was internally converted to LBA_xxxx if a non-LBA partition
     above cylinder 1023 was found */
  if (!InitKernelConfig.ForceLBA && !ExtLBAForce && !IsLBAPartition(pEntry->FileSystem))
    pddt->ddt_descflags &= ~DF_LBA;
  pddt->ddt_ncyl = driveParam->chs.Cylinder;

  DebugPrintf(("LBA %senabled for drive %c:\n", (pddt->ddt_descflags & DF_LBA)?"":"not ", 'A' + nUnits));

  pddt->ddt_offset = StartSector;

  pddt->ddt_defbpb.bpb_nbyte = FLOPPY_SEC_SIZE;
  pddt->ddt_defbpb.bpb_mdesc = 0xf8;
  pddt->ddt_defbpb.bpb_nheads = driveParam->chs.Head;
  pddt->ddt_defbpb.bpb_nsecs = driveParam->chs.Sector;
  pddt->ddt_defbpb.bpb_hidden = pEntry->RelSect;

  pddt->ddt_defbpb.bpb_nsize = 0;
  pddt->ddt_defbpb.bpb_huge = pEntry->NumSect;
  if (pEntry->NumSect <= 0xffff)
  {
    pddt->ddt_defbpb.bpb_nsize = (UWORD) (pEntry->NumSect);
    pddt->ddt_defbpb.bpb_huge = 0;  /* may still be set on Win95 */
  }

  /* sectors per cluster, sectors per FAT etc. */
  CalculateFATData(pddt, pEntry->NumSect, pEntry->FileSystem);

  pddt->ddt_serialno = 0x12345678l;
  /* drive inaccessible until bldbpb successful */
  pddt->ddt_descflags |= init_readdasd(pddt->ddt_driveno) | DF_NOACCESS;
  pddt->ddt_type = 5;
  memcpy(&pddt->ddt_bpb, &pddt->ddt_defbpb, sizeof(bpb));

  push_ddt(pddt);

  /* Alain whishes to keep this in later versions, too 
     Tom likes this too, so he made it configurable by SYS CONFIG ...
   */

  if (InitKernelConfig.InitDiskShowDriveAssignment)
  {
    char *ExtPri;
    int num;

    LBA_to_CHS(&chs, StartSector, driveParam);

    ExtPri = "Pri";
    num = PrimaryNum + 1;
    if (extendedPartNo)
    {
      ExtPri = "Ext";
      num = extendedPartNo;
    }
    printf("\r%c: HD%d, %s[%2d]", 'A' + nUnits,
           (driveParam->driveno & 0x7f) + 1, ExtPri, num);

    printCHS(", CHS= ", &chs);

    printf(", start=%6lu MB, size=%6lu MB\n",
           StartSector / 2048, pEntry->NumSect / 2048);
  }

  nUnits++;
}

/* Get the parameters of the hard disk */
STATIC int LBA_Get_Drive_Parameters(int drive, struct DriveParamS *driveParam)
{
  iregs regs;
  struct _bios_LBA_disk_parameterS lba_bios_parameters;

  ExtLBAForce = FALSE;

  memset(driveParam, 0, sizeof *driveParam);
  drive |= 0x80;

  /* for tests - disable LBA support,
     even if exists                    */
  if (!InitKernelConfig.GlobalEnableLBAsupport)
  {
    goto StandardBios;
  }
  /* check for LBA support */
  regs.b.x = 0x55aa;
  regs.a.b.h = 0x41;
  regs.d.b.l = drive;

  init_call_intr(0x13, &regs);

  if (regs.b.x != 0xaa55 || (regs.flags & 0x01))
  {
    goto StandardBios;
  }

  /* by ralph :
     if DAP cannot be used, don't use
     LBA
   */
  if ((regs.c.x & 1) == 0)
  {
    goto StandardBios;
  }

  /* drive supports LBA addressing */

  /* version 1.0, 2.0 have different verify */
  if (regs.a.x < 0x2100)
    LBA_WRITE_VERIFY = 0x4301;

  memset(&lba_bios_parameters, 0, sizeof(lba_bios_parameters));
  lba_bios_parameters.size = sizeof(lba_bios_parameters);

  regs.si = FP_OFF(&lba_bios_parameters);
  regs.ds = FP_SEG(&lba_bios_parameters);
  regs.a.b.h = 0x48;
  regs.d.b.l = drive;
  init_call_intr(0x13, &regs);

  /* error or DMA boundary errors not handled transparently */
  if (regs.flags & 0x01)
  {
    goto StandardBios;
  }

  /* verify maximum settings, we can't handle more */

  if (lba_bios_parameters.heads > 0xffff ||
      lba_bios_parameters.sectors > 0xffff ||
      lba_bios_parameters.totalSectHigh != 0)
  {
    printf("Drive is too large to handle, using only 1st 8 GB\n"
           " drive %02x heads %lu sectors %lu , total=0x%lx-%08lx\n",
           drive,
           (ULONG) lba_bios_parameters.heads,
           (ULONG) lba_bios_parameters.sectors,
           (ULONG) lba_bios_parameters.totalSect,
           (ULONG) lba_bios_parameters.totalSectHigh);

    goto StandardBios;
  }

  driveParam->total_sectors = lba_bios_parameters.totalSect;

  /* if we arrive here, success */
  driveParam->descflags = DF_LBA;
  if (lba_bios_parameters.information & 8)
    driveParam->descflags |= DF_WRTVERIFY;

  if (lba_bios_parameters.information & 1)
  {
    driveParam->descflags |= DF_DMA_TRANSPARENT;        /* DMA boundary errors are handled transparently */
  }
  
StandardBios:                  /* old way to get parameters */

  regs.a.b.h = 0x08;
  regs.d.b.l = drive;

  init_call_intr(0x13, &regs);

  if (regs.flags & 0x01)
    goto ErrorReturn;

  /* int13h call returns max value, store as count (#) i.e. +1 for 0 based heads & cylinders */
  driveParam->chs.Head = (regs.d.x >> 8) + 1; /* DH = max head value = # of heads - 1 (0-255) */
  driveParam->chs.Sector = (regs.c.x & 0x3f); /* CL bits 0-5 = max sector value = # (sectors/track) - 1 (1-63) */
  /* max cylinder value = # cylinders - 1 (0-1023) = [high two bits]CL7:6=cyls9:8, [low byte]CH=cyls7:0 */
  driveParam->chs.Cylinder = (regs.c.x >> 8) | ((regs.c.x & 0xc0) << 2) + 1; 
  
  if (driveParam->chs.Sector == 0) {
    /* happens e.g. with Bochs 1.x if no harddisk defined */
    driveParam->chs.Sector = 63; /* avoid division by zero...! */
    printf("BIOS reported 0 sectors/track, assuming 63!\n");
  }

  if (!(driveParam->descflags & DF_LBA))
  {
    driveParam->total_sectors =
        (ULONG)driveParam->chs.Cylinder
        * driveParam->chs.Head * driveParam->chs.Sector;
  }

  driveParam->driveno = drive;

  DebugPrintf(("drive %02Xh total: C = %u, H = %u, S = %u,",
               drive,
               driveParam->chs.Cylinder,
               driveParam->chs.Head, driveParam->chs.Sector));
  DebugPrintf((" total size %luMB\n\n", driveParam->total_sectors / 2048));

ErrorReturn:

  return driveParam->driveno;
}

/*
    converts physical into logical representation of partition entry
*/

STATIC void ConvCHSToIntern(struct CHS *chs, UBYTE * pDisk)
{
  chs->Head = pDisk[0];
  chs->Sector = pDisk[1] & 0x3f;
  chs->Cylinder = pDisk[2] + ((pDisk[1] & 0xc0) << 2);
}

BOOL ConvPartTableEntryToIntern(struct PartTableEntry * pEntry,
                                UBYTE * pDisk)
{
  int i;

  if (pDisk[0x1fe] != 0x55 || pDisk[0x1ff] != 0xaa)
  {
    memset(pEntry, 0, 4 * sizeof(struct PartTableEntry));

    return FALSE;
  }

  pDisk += 0x1be;

  for (i = 0; i < 4; i++, pDisk += 16, pEntry++)
  {

    pEntry->Bootable = pDisk[0];
    pEntry->FileSystem = pDisk[4];

    ConvCHSToIntern(&pEntry->Begin, pDisk+1);
    ConvCHSToIntern(&pEntry->End, pDisk+5);

    pEntry->RelSect = *(ULONG *) (pDisk + 8);
    pEntry->NumSect = *(ULONG *) (pDisk + 12);
  }
  return TRUE;
}

BOOL is_suspect(struct CHS *chs, struct CHS *pEntry_chs)
{
  /* Valid entry:
     entry == chs ||           // partition entry equal to computed values
     (chs->Cylinder > 1023 &&  // or LBA partition
      (entry->Cylinder == 1023 ||
       entry->Cylinder == (0x3FF & chs->Cylinder)))
  */
  return !((pEntry_chs->Cylinder == chs->Cylinder &&
            pEntry_chs->Head     == chs->Head     &&
            pEntry_chs->Sector   == chs->Sector)        ||
           chs->Cylinder > 1023u &&
           (pEntry_chs->Cylinder == 1023 ||
            pEntry_chs->Cylinder == (0x3ff & chs->Cylinder)));
}

void print_warning_suspect(char *partitionName, UBYTE fs, struct CHS *chs,
                           struct CHS *pEntry_chs)
{
  if (!InitKernelConfig.ForceLBA)
  {
    printf("WARNING: using suspect partition %s FS %02x:", partitionName, fs);
    printCHS(" with calculated values ", chs);
    printCHS(" instead of ", pEntry_chs);
    printf("\n");
  }
  memcpy(pEntry_chs, chs, sizeof(struct CHS));
}

BOOL ScanForPrimaryPartitions(struct DriveParamS * driveParam, int scan_type,
                         struct PartTableEntry * pEntry, ULONG startSector,
                         int partitionsToIgnore, int extendedPartNo)
{
  int i;
  struct CHS chs, end;
  ULONG partitionStart;
  char partitionName[12];

  for (i = 0; i < 4; i++, pEntry++)
  {
    if (pEntry->FileSystem == 0)
      continue;

    if (partitionsToIgnore & (1 << i))
      continue;

    if (IsExtPartition(pEntry->FileSystem))
      continue;

    if (scan_type == SCAN_PRIMARYBOOT && !pEntry->Bootable)
      continue;

    partitionStart = startSector + pEntry->RelSect;

    if (!IsFATPartition(pEntry->FileSystem))
    {
      continue;
    }

    if (extendedPartNo)
      sprintf(partitionName, "Ext:%d", extendedPartNo);
    else
      sprintf(partitionName, "Pri:%d", i + 1);

    /*
       some sanity checks, that partition
       structure is OK
     */
    LBA_to_CHS(&chs, partitionStart, driveParam);
    LBA_to_CHS(&end, partitionStart + pEntry->NumSect - 1, driveParam);

    /* some FDISKs enter for partitions 
       > 8 GB cyl = 1023, other (cyl&1023)
     */

    if (is_suspect(&chs, &pEntry->Begin))
    {
      print_warning_suspect(partitionName, pEntry->FileSystem, &chs,
                            &pEntry->Begin);
    }

    if (is_suspect(&end, &pEntry->End))
    {
      if (pEntry->NumSect == 0)
      {
        printf("Not using partition %s with 0 sectors\n", partitionName);
        continue;
      }
      print_warning_suspect(partitionName, pEntry->FileSystem, &end,
                            &pEntry->End);
    }

    if (chs.Cylinder > 1023 || end.Cylinder > 1023)
    {

      if (!(driveParam->descflags & DF_LBA))
      {
        printf
            ("can't use LBA partition without LBA support - part %s FS %02x",
             partitionName, pEntry->FileSystem);

        printCHS(" start ", &chs);
        printCHS(", end ", &end);
        printf("\n");

        continue;
      }

      if (!InitKernelConfig.ForceLBA && !ExtLBAForce 
          && !IsLBAPartition(pEntry->FileSystem))
      {
        printf
            ("WARNING: Partition ID does not suggest LBA - part %s FS %02x.\n"
             "Please run FDISK to correct this - using LBA to access partition.\n",
             partitionName, pEntry->FileSystem);

        printCHS(" start ", &chs);
        printCHS(", end ", &end);
        printf("\n");
        pEntry->FileSystem = (pEntry->FileSystem == FAT12 ? FAT12_LBA :
                              pEntry->FileSystem == FAT32 ? FAT32_LBA :
                              /*  pEntry->FileSystem == FAT16 ? */
                              FAT16_LBA);
      }

      /* else its a diagnostic message only */
#ifdef DEBUG
      printf("found and using LBA partition %s FS %02x",
             partitionName, pEntry->FileSystem);
      printCHS(" start ", &chs);
      printCHS(", end ", &end);
      printf("\n");
#endif
    }

    /*
       here we have a partition table in our hand !!
     */

    partitionsToIgnore |= 1 << i;

    DosDefinePartition(driveParam, partitionStart, pEntry,
                       extendedPartNo, i);

    if (scan_type == SCAN_PRIMARYBOOT || scan_type == SCAN_PRIMARY)
    {
      return partitionsToIgnore;
    }
  }

  return partitionsToIgnore;
}

void BIOS_drive_reset(unsigned drive);

int Read1LBASector(struct DriveParamS *driveParam, unsigned drive,
                   ULONG LBA_address, void * buffer)
{
  static struct _bios_LBA_address_packet dap = {
    16, 0, 0, 0, 0, 0, 0
  };

  struct CHS chs;
  iregs regs;
  int num_retries;

/* disabled because this should not happen and if it happens the BIOS
   should complain; also there are weird disks around with
   CMOS geometry < real geometry */
#if 0
  if (LBA_address >= driveParam->total_sectors)
  {
    printf("LBA-Transfer error : address overflow = %lu, > %lu total sectors\n",
           LBA_address, driveParam->total_sectors);
    return 1;
  }
#endif

  for (num_retries = 0; num_retries < N_RETRY; num_retries++)
  {
    regs.d.b.l = drive | 0x80;
    LBA_to_CHS(&chs, LBA_address, driveParam);
    /* Some old "security" software (PROT) traps int13 and assumes non
       LBA accesses. This statement causes partition tables to be read
       using CHS methods even if LBA is available unless CHS can't reach
       them. This can be overridden using kernel config parameters and
       the extended LBA partition type indicator.
    */
    if ((driveParam->descflags & DF_LBA) &&
        (InitKernelConfig.ForceLBA || ExtLBAForce || chs.Cylinder > 1023))
    {
      dap.number_of_blocks = 1;
      dap.buffer_address = buffer;
      dap.block_address_high = 0;       /* clear high part */
      dap.block_address = LBA_address;  /* clear high part */

      /* Load the registers and call the interrupt. */
      regs.a.x = LBA_READ;
      regs.si = FP_OFF(&dap);
      regs.ds = FP_SEG(&dap);
    }
    else
    {                           /* transfer data, using old bios functions */
      /* avoid overflow at end of track */

      if (chs.Cylinder > 1023)
      {
        printf("LBA-Transfer error : address = %lu, cylinder %u > 1023\n", LBA_address, chs.Cylinder);
        return 1;
      }

      regs.a.x = 0x0201;
      regs.b.x = FP_OFF(buffer);
      regs.c.x =
          ((chs.Cylinder & 0xff) << 8) + ((chs.Cylinder & 0x300) >> 2) +
          chs.Sector;
      regs.d.b.h = chs.Head;
      regs.es = FP_SEG(buffer);
    }                           /* end of retries */
    init_call_intr(0x13, &regs);
    if ((regs.flags & FLG_CARRY) == 0)
      break;
    BIOS_drive_reset(driveParam->driveno);
  }

  return regs.flags & FLG_CARRY ? 1 : 0;
}

/* Load the Partition Tables and get information on all drives */
int ProcessDisk(int scanType, unsigned drive, int PartitionsToIgnore)
{

  struct PartTableEntry PTable[4];
  ULONG RelSectorOffset;
  ULONG ExtendedPartitionOffset;
  int iPart;
  int strangeHardwareLoop;

  int num_extended_found = 0;

  struct DriveParamS driveParam;

  /* Get the hard drive parameters and ensure that the drive exists. */
  /* If there was an error accessing the drive, skip that drive. */

  if (!LBA_Get_Drive_Parameters(drive, &driveParam))
  {
    printf("can't get drive parameters for drive %02x\n", drive);
    return PartitionsToIgnore;
  }

  RelSectorOffset = 0;          /* boot sector */
  ExtendedPartitionOffset = 0;  /* not found yet */

  /* Read the Primary Partition Table. */

ReadNextPartitionTable:

  strangeHardwareLoop = 0;
strange_restart:

  if (Read1LBASector
      (&driveParam, drive, RelSectorOffset, InitDiskTransferBuffer))
  {
    printf("Error reading partition table drive %02Xh sector %lu", drive,
           RelSectorOffset);
    return PartitionsToIgnore;
  }

  if (!ConvPartTableEntryToIntern(PTable, InitDiskTransferBuffer))
  {
    /* there is some strange hardware out in the world,
       which returns OK on first read, but the data are
       rubbish. simply retrying works fine.
       there is no logic behind this, but it works TE */

    if (++strangeHardwareLoop < 3)
      goto strange_restart;

    printf("illegal partition table - drive %02x sector %lu\n", drive,
           RelSectorOffset);
    return PartitionsToIgnore;
  }

  if (scanType == SCAN_PRIMARYBOOT ||
      scanType == SCAN_PRIMARY ||
      scanType == SCAN_PRIMARY2 || num_extended_found != 0)
  {

    PartitionsToIgnore = ScanForPrimaryPartitions(&driveParam, scanType,
                                                  PTable, RelSectorOffset,
                                                  PartitionsToIgnore,
                                                  num_extended_found);
  }

  if (scanType != SCAN_EXTENDED)
  {
    return PartitionsToIgnore;
  }

  /* scan for extended partitions now */
  PartitionsToIgnore = 0;

  for (iPart = 0; iPart < 4; iPart++)
  {
    if (IsExtPartition(PTable[iPart].FileSystem))
    {
      RelSectorOffset = ExtendedPartitionOffset + PTable[iPart].RelSect;

      if (ExtendedPartitionOffset == 0)
      {
        ExtendedPartitionOffset = PTable[iPart].RelSect;
        /* grand parent LBA -> all children and grandchildren LBA */
        ExtLBAForce = (PTable[iPart].FileSystem == EXTENDED_LBA);
      }

      num_extended_found++;

      if (num_extended_found > 30)
      {
        printf("found more then 30 extended partitions, terminated\n");
        return 0;
      }

      goto ReadNextPartitionTable;
    }
  }

  return PartitionsToIgnore;
}

int BIOS_nrdrives(void)
{
  iregs regs;

  regs.a.b.h = 0x08;
  regs.d.b.l = 0x80;
  init_call_intr(0x13, &regs);

  if (regs.flags & 1)
  {
    printf("no hard disks detected\n");
    return 0;
  }

  return regs.d.b.l;
}

void BIOS_drive_reset(unsigned drive)
{
  iregs regs;

  regs.d.b.l = drive | 0x80;
  regs.a.b.h = 0;

  init_call_intr(0x13, &regs);
}

/* 
    thats what MSDN says:

    How Windows 2000 Assigns, Reserves, and Stores Drive Letters
    ID: q234048 
 
  BASIC Disk - Drive Letter Assignment Rules
The following are the basic disk drive letter assignment rules for Windows 2000: 
Scan all fixed hard disks as they are enumerated, assign drive letters 
starting with any active primary partitions (if there is one), otherwise,
scan the first primary partition on each drive. Assign next available 
letter starting with C:

Repeat scan for all fixed hard disks and removable (JAZ, MO) disks 
and assign drive letters to all logical drives in an extended partition, 
or the removable disk(s) as enumerated. Assign next available letter 
starting with C: 

Finally, repeat scan for all fixed hard disk drives, and assign drive 
letters to all remaining primary partitions. Assign next available letter 
starting with C:

Floppy drives. Assign letter starting with A:

CD-ROM drives. Assign next available letter starting with D:

*************************************************************************
Order in Which MS-DOS and Windows Assign Drive Letters
ID: q51978 
 
MORE INFORMATION
The following occurs at startup: 

MS-DOS checks all installed disk devices, assigning the drive letter A 
to the first physical floppy disk drive that is found.

If a second physical floppy disk drive is present, it is assigned drive letter B. If it is not present, a logical drive B is created that uses the first physical floppy disk drive.

Regardless of whether a second floppy disk drive is present, 
MS-DOS then assigns the drive letter C to the primary MS-DOS 
partition on the first physical hard disk, and then goes on 
to check for a second hard disk. 

If a second physical hard disk is found, and a primary partition exists 
on the second physical drive, the primary MS-DOS partition on the second
physical hard drive is assigned the letter D. MS-DOS version 5.0, which 
supports up to eight physical drives, will continue to search for more 
physical hard disk drives at this point. For example, if a third physical 
hard disk is found, and a primary partition exists on the third physical 
drive, the primary MS-DOS partition on the third physical hard drive is 
assigned the letter E.

MS-DOS returns to the first physical hard disk drive and assigns drive 
letters to any additional logical drives (in extended MS-DOS partitions) 
on that drive in sequence.

MS-DOS repeats this process for the second physical hard disk drive, 
if present. MS-DOS 5.0 will repeat this process for up to eight physical 
hard drives, if present. After all logical drives (in extended MS-DOS 
partitions) have been assigned drive letters, MS-DOS 5.0 returns to 
the first physical drive and assigns drive letters to any other primary 
MS-DOS partitions that exist, then searches other physical drives for 
additional primary MS-DOS partitions. This support for multiple primary 
MS-DOS partitions was added to version 5.0 for backward compatibility 
with the previous OEM MS-DOS versions that support multiple primary partitions.

After all logical drives on the hard disk(s) have been assigned drive 
letters, drive letters are assigned to drives installed using DRIVER.SYS 
or created using RAMDRIVE.SYS in the order in which the drivers are loaded 
in the CONFIG.SYS file. Which drive letters are assigned to which devices 
can be influenced by changing the order of the device drivers or, if necessary, 
by creating "dummy" drive letters with DRIVER.SYS.

********************************************************

or

  as rather well documented, DOS searches 1st) 1 primary patitions on
     all drives, 2nd) all extended partitions. that
     makes many people (including me) unhappy, as all DRIVES D:,E:...
     on 1st disk will move up/down, if other disk with
     primary partitions are added/removed, but
     thats the way it is (hope I got it right)
     TE (with a little help from my friends)
     see also above for WIN2000,DOS,MSDN

I don't know, if I did it right, but I tried to do it that way. TE

***********************************************************************/

STATIC void make_ddt (ddt *pddt, int Unit, int driveno, int flags)
{
  pddt->ddt_next = MK_FP(0, 0xffff);
  pddt->ddt_logdriveno = Unit;
  pddt->ddt_driveno = driveno;
  pddt->ddt_type = init_getdriveparm(driveno, &pddt->ddt_defbpb);
  pddt->ddt_ncyl = (pddt->ddt_type & 7) ? 80 : 40;
  pddt->ddt_descflags = init_readdasd(driveno) | flags;

  pddt->ddt_offset = 0;
  pddt->ddt_serialno = 0x12345678l;
  memcpy(&pddt->ddt_bpb, &pddt->ddt_defbpb, sizeof(bpb));
  push_ddt(pddt);
}

void ReadAllPartitionTables(void)
{
  UBYTE foundPartitions[MAX_HARD_DRIVE];

  int HardDrive;
  int nHardDisk;
  ddt nddt;
  static iregs regs;

  /* quick adjustment of diskette parameter table */
  fmemcpy(int1e_table, *(char FAR * FAR *)MK_FP(0, 0x1e*4), sizeof(int1e_table));
  /* currently only 512 bytes per sector floppies are supported, log2(512/128)=2 */
  int1e_table[3] = 2;
  /* enforce min. 9 sectors per track */
  if (int1e_table[4] < 9)
    int1e_table[4] = 9;
  /* and adjust int1e */
  setvec(0x1e, (intvec)int1e_table);

  /* Setup media info and BPBs arrays for floppies */
  make_ddt(&nddt, 0, 0, 0);

  /*
     this is a quick patch - see if B: exists
     test for A: also, need not exist
   */
  init_call_intr(0x11, &regs);  /* get equipment list */
/*if ((regs.AL & 1)==0)*//* no floppy drives installed  */
  if ((regs.AL & 1) && (regs.AL & 0xc0))
  {
    /* floppy drives installed and a B: drive */
    make_ddt(&nddt, 1, 1, 0);
  }
  else
  {
    /* set up the DJ method : multiple logical drives */
    make_ddt(&nddt, 1, 0, DF_MULTLOG);
  }

  /* Initial number of disk units                                 */
  nUnits = 2;

  nHardDisk = BIOS_nrdrives();
  if (nHardDisk > LENGTH(foundPartitions))
    nHardDisk = LENGTH(foundPartitions);

  DebugPrintf(("DSK init: found %d disk drives\n", nHardDisk));

  /* Reset the drives                                             */
  for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
  {
    BIOS_drive_reset(HardDrive);
    foundPartitions[HardDrive] = 0;
  }

  if (InitKernelConfig.DLASortByDriveNo == 0)
  {
    /* printf("Drive Letter Assignment - DOS order\n"); */

    /* Process primary partition table   1 partition only      */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
      foundPartitions[HardDrive] =
          ProcessDisk(SCAN_PRIMARYBOOT, HardDrive, 0);

      if (foundPartitions[HardDrive] == 0)
        foundPartitions[HardDrive] =
            ProcessDisk(SCAN_PRIMARY, HardDrive, 0);
    }

    /* Process extended partition table                      */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
      ProcessDisk(SCAN_EXTENDED, HardDrive, 0);
    }

    /* Process primary a 2nd time */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
      ProcessDisk(SCAN_PRIMARY2, HardDrive, foundPartitions[HardDrive]);
    }
  }
  else
  {
    UBYTE bootdrv = peekb(0,0x5e0);

    /* printf("Drive Letter Assignment - sorted by drive\n"); */

    /* Process primary partition table   1 partition only      */
    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
      struct DriveParamS driveParam;
      if (LBA_Get_Drive_Parameters(HardDrive, &driveParam) &&
          driveParam.driveno == bootdrv)
      {
        foundPartitions[HardDrive] =
          ProcessDisk(SCAN_PRIMARYBOOT, HardDrive, 0);
        break;
      }
    }

    for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
    {
      if (foundPartitions[HardDrive] == 0)
      {
        foundPartitions[HardDrive] =
          ProcessDisk(SCAN_PRIMARYBOOT, HardDrive, 0);

        if (foundPartitions[HardDrive] == 0)
          foundPartitions[HardDrive] =
            ProcessDisk(SCAN_PRIMARY, HardDrive, 0);
      }

      /* Process extended partition table                      */
      ProcessDisk(SCAN_EXTENDED, HardDrive, 0);

      /* Process primary a 2nd time */
      ProcessDisk(SCAN_PRIMARY2, HardDrive, foundPartitions[HardDrive]);
    }
  }
}

/* disk initialization: returns number of units */
COUNT dsk_init()
{
  printf(" - InitDisk");

#ifdef DEBUG
  {
    iregs regs;
    regs.a.x = 0x1112;          /* select 43 line mode - more space for partinfo */
    regs.b.x = 0;
    init_call_intr(0x10, &regs);
  }
#endif

  /* Reset the drives                                             */
  BIOS_drive_reset(0);

  ReadAllPartitionTables();

  return nUnits;
}
