/****************************************************************/
/*                                                              */
/*                            dsk.c                             */
/*                                                              */
/*                      Copyright (c) 1995                      */
/*                      Pasquale J. Villani                     */
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
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *dskRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.6  2000/05/26 19:25:19  jimtabor
 * Read History file for Change info
 *
 * Revision 1.5  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.4  2000/05/17 19:15:12  jimtabor
 * Cleanup, add and fix source.
 *
 * Revision 1.3  2000/05/11 04:26:26  jimtabor
 * Added code for DOS FN 69 & 6C
 *
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.6  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.5  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.4  1999/08/10 18:07:57  jprice
 * ror4 2011-04 patch
 *
 * Revision 1.3  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.2  1999/04/16 00:53:32  jprice
 * Optimized FAT handling
 *
 * Revision 1.1.1.1  1999/03/29 15:40:51  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/14 04:26:46  jprice
 * Changed check media so that it checks if a floppy disk has been changed.
 *
 * Revision 1.4  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:25  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.7   06 Dec 1998  8:45:18   patv
 * Changed due to new I/O subsystem.
 *
 *    Rev 1.6   04 Jan 1998 23:15:16   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   10 Jan 1997  5:41:48   patv
 * Modified for extended partition support
 *
 *    Rev 1.4   29 May 1996 21:03:32   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:21:36   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:54:18   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:52:00   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:32:42   patv
 * Initial revision.
 */

#ifdef PROTO
BOOL fl_reset(WORD);
COUNT fl_readdasd(WORD);
COUNT fl_diskchanged(WORD);
COUNT fl_rd_status(WORD);
COUNT fl_read(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_write(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
COUNT fl_verify(WORD, WORD, WORD, WORD, WORD, BYTE FAR *);
BOOL fl_format(WORD, BYTE FAR *);
#else
BOOL fl_reset();
COUNT fl_readdasd();
COUNT fl_diskchanged();
COUNT fl_rd_status();
COUNT fl_read();
COUNT fl_write();
COUNT fl_verify();
BOOL fl_format();
#endif

#define NDEV            8       /* only one for demo            */
#define SEC_SIZE        512     /* size of sector in bytes      */
#define N_RETRY         5       /* number of retries permitted  */
#define NENTRY          26      /* total size of dispatch table */

extern BYTE FAR nblk_rel;

union
{
  BYTE bytes[2 * SEC_SIZE];
  boot boot_sector;
}
buffer;

static struct media_info
{
  ULONG mi_size;                /* physical sector count        */
  UWORD mi_heads;               /* number of heads (sides)      */
  UWORD mi_cyls;                /* number of cyl/drive          */
  UWORD mi_sectors;             /* number of sectors/cyl        */
  ULONG mi_offset;              /* relative partition offset    */
  BYTE mi_drive;                /* BIOS drive number            */
  COUNT mi_partidx;             /* Index to partition array     */
};

static struct FS_info
{
  ULONG fs_serialno;
  BYTE  fs_volume[11];
  BYTE  fs_fstype[8];
};

static struct Access_info
{
  BYTE  AI_spec;
  BYTE  AI_Flag;
};

static struct media_info miarray[NDEV]; /* Internal media info structs  */
static struct FS_info fsarray[NDEV];
static bpb bpbarray[NDEV];      /* BIOS parameter blocks        */
static bpb *bpbptrs[NDEV];      /* pointers to bpbs             */

#define N_PART 4                /* number of partitions per
                                   table partition              */

static WORD head,
  track,
  sector,
  ret;                          /* globals for blockio          */
static WORD count;
static COUNT nUnits;            /* number of returned units     */
static COUNT nPartitions;       /* number of DOS partitions     */

#define PARTOFF 0x1be

static struct
{
  BYTE peDrive;                 /* BIOS drive number            */
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
  LONG peAbsStart;              /* Absolute sector start        */
}
dos_partition[NDEV - 2];

#ifdef PROTO
WORD init(rqptr),
  mediachk(rqptr),
  bldbpb(rqptr),
  blockio(rqptr),
  IoctlQueblk(rqptr),
  Genblkdev(rqptr),
  blk_error(rqptr);
COUNT ltop(WORD *, WORD *, WORD *, COUNT, COUNT, LONG, byteptr);
WORD dskerr(COUNT);
COUNT processtable(COUNT ptDrive, BYTE ptHead, UWORD ptCylinder, BYTE ptSector, LONG ptAccuOff);
#else
WORD init(),
  mediachk(),
  bldbpb(),
  blockio(),
  blk_error();
WORD dskerr();
COUNT processtable();
#endif

/*                                                                      */
/* the function dispatch table                                          */
/*                                                                      */

#ifdef PROTO
static WORD(*dispatch[NENTRY]) (rqptr) =
#else
static WORD(*dispatch[NENTRY]) () =
#endif
{
  init,                         /* Initialize                   */
      mediachk,                 /* Media Check                  */
      bldbpb,                   /* Build BPB                    */
      blk_error,                /* Ioctl In                     */
      blockio,                  /* Input (Read)                 */
      blk_error,                /* Non-destructive Read         */
      blk_error,                /* Input Status                 */
      blk_error,                /* Input Flush                  */
      blockio,                  /* Output (Write)               */
      blockio,                  /* Output with verify           */
      blk_error,                /* Output Status                */
      blk_error,                /* Output Flush                 */
      blk_error,                /* Ioctl Out                    */
      blk_error,                /* Device Open                  */
      blk_error,                /* Device Close                 */
      blk_error,                /* Removable Media              */
      blk_error,                /* Output till busy             */
      blk_error,                /* undefined                    */
      blk_error,                /* undefined                    */
      Genblkdev,                /* Generic Ioctl Call           */
      blk_error,                /* undefined                    */
      blk_error,                /* undefined                    */
      blk_error,                /* undefined                    */
      blk_error,                /* Get Logical Device           */
      blk_error,                /* Set Logical Device           */
      IoctlQueblk               /* Ioctl Query                  */
};

#define SIZEOF_PARTENT  16

#define FAT12           0x01
#define FAT16SMALL      0x04
#define EXTENDED        0x05
#define FAT16LARGE      0x06

#define hd(x)   ((x) & 0x80)

COUNT processtable(COUNT ptDrive, BYTE ptHead, UWORD ptCylinder,
                   BYTE ptSector, LONG ptAccuOff)
{
  struct                        /* Temporary partition table    */
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
  }
  temp_part[N_PART];

  REG retry = N_RETRY;
  UBYTE packed_byte,
    pb1;
  COUNT Part;

  /* Read partition table                         */
  do
  {
    ret = fl_read((WORD) ptDrive, (WORD) ptHead, (WORD) ptCylinder,
                  (WORD) ptSector, (WORD) 1, (byteptr) & buffer);
  }
  while (ret != 0 && --retry > 0);
  if (ret != 0)
    return FALSE;

  /* Read each partition into temporary array     */
  for (Part = 0; Part < N_PART; Part++)
  {
    REG BYTE *p =
    (BYTE *) & buffer.bytes[PARTOFF + (Part * SIZEOF_PARTENT)];

    getbyte((VOID *) p, &temp_part[Part].peBootable);
    ++p;
    getbyte((VOID *) p, &temp_part[Part].peBeginHead);
    ++p;
    getbyte((VOID *) p, &packed_byte);
    temp_part[Part].peBeginSector = packed_byte & 0x3f;
    ++p;
    getbyte((VOID *) p, &pb1);
    ++p;
    temp_part[Part].peBeginCylinder = pb1 + ((UWORD) (0xc0 & packed_byte) << 2);
    getbyte((VOID *) p, &temp_part[Part].peFileSystem);
    ++p;
    getbyte((VOID *) p, &temp_part[Part].peEndHead);
    ++p;
    getbyte((VOID *) p, &packed_byte);
    temp_part[Part].peEndSector = packed_byte & 0x3f;
    ++p;
    getbyte((VOID *) p, &pb1);
    ++p;
    temp_part[Part].peEndCylinder = pb1 + ((UWORD) (0xc0 & packed_byte) << 2);
    getlong((VOID *) p, &temp_part[Part].peStartSector);
    p += sizeof(LONG);
    getlong((VOID *) p, &temp_part[Part].peSectors);
  };

  /* Walk through the table, add DOS partitions to global
     array and process extended partitions         */
  for (Part = 0; Part < N_PART && nUnits < NDEV; Part++)
  {
    if (temp_part[Part].peFileSystem == FAT12 ||
        temp_part[Part].peFileSystem == FAT16SMALL ||
        temp_part[Part].peFileSystem == FAT16LARGE)
    {
      miarray[nUnits].mi_offset =
          temp_part[Part].peStartSector + ptAccuOff;
      miarray[nUnits].mi_drive = ptDrive;
      miarray[nUnits].mi_partidx = nPartitions;
      nUnits++;

      dos_partition[nPartitions].peDrive = ptDrive;
      dos_partition[nPartitions].peBootable =
          temp_part[Part].peBootable;
      dos_partition[nPartitions].peBeginHead =
          temp_part[Part].peBeginHead;
      dos_partition[nPartitions].peBeginSector =
          temp_part[Part].peBeginSector;
      dos_partition[nPartitions].peBeginCylinder =
          temp_part[Part].peBeginCylinder;
      dos_partition[nPartitions].peFileSystem =
          temp_part[Part].peFileSystem;
      dos_partition[nPartitions].peEndHead =
          temp_part[Part].peEndHead;
      dos_partition[nPartitions].peEndSector =
          temp_part[Part].peEndSector;
      dos_partition[nPartitions].peEndCylinder =
          temp_part[Part].peEndCylinder;
      dos_partition[nPartitions].peStartSector =
          temp_part[Part].peStartSector;
      dos_partition[nPartitions].peSectors =
          temp_part[Part].peSectors;
      dos_partition[nPartitions].peAbsStart =
          temp_part[Part].peStartSector + ptAccuOff;
      nPartitions++;
    }
    else if (temp_part[Part].peFileSystem == EXTENDED)
    {
      /* call again to process extended part table */
      processtable(ptDrive,
                   temp_part[Part].peBeginHead,
                   temp_part[Part].peBeginCylinder,
                   temp_part[Part].peBeginSector,
                   temp_part[Part].peStartSector + ptAccuOff);
    };
  };
  return TRUE;
}

COUNT blk_driver(rqptr rp)
{
  if (rp->r_unit >= nUnits && rp->r_command != C_INIT)
    return failure(E_UNIT);
  if (rp->r_command > NENTRY)
  {
    return failure(E_FAILURE);  /* general failure */
  }
  else
    return ((*dispatch[rp->r_command]) (rp));
}

static WORD init(rqptr rp)
{
  extern COUNT fl_nrdrives(VOID);
  COUNT HardDrive,
    nHardDisk,
    Unit;

  /* Reset the drives                                             */
  fl_reset(0x80);

  /* Initial number of disk units                                 */
  nUnits = 2;
  /* Initial number of DOS partitions                             */
  nPartitions = 0;

  /* Setup media info and BPBs arrays                             */
  for (Unit = 0; Unit < NDEV; Unit++)
  {
    miarray[Unit].mi_size = 720l;
    miarray[Unit].mi_heads = 2;
    miarray[Unit].mi_cyls = 40;
    miarray[Unit].mi_sectors = 9;
    miarray[Unit].mi_offset = 0l;
    miarray[Unit].mi_drive = Unit;

    fsarray[Unit].fs_serialno = 0x12345678;


    bpbarray[Unit].bpb_nbyte = SEC_SIZE;
    bpbarray[Unit].bpb_nsector = 2;
    bpbarray[Unit].bpb_nreserved = 1;
    bpbarray[Unit].bpb_nfat = 2;
    bpbarray[Unit].bpb_ndirent = 112;
    bpbarray[Unit].bpb_nsize = 720l;
    bpbarray[Unit].bpb_mdesc = 0xfd;
    bpbarray[Unit].bpb_nfsect = 2;

    bpbptrs[Unit] = &bpbarray[Unit];
  };

  nHardDisk = fl_nrdrives();
  for (HardDrive = 0; HardDrive < nHardDisk; HardDrive++)
  {
    /* Process primary partition table                      */
    if (!processtable((HardDrive | 0x80), 0, 0l, 1, 0l))
      /* Exit if no hard drive                             */
      break;
  };

  rp->r_nunits = nUnits;
  rp->r_bpbptr = bpbptrs;
  rp->r_endaddr = device_end();
  nblk_rel = nUnits;            /* make device header reflect units */
  return S_DONE;
}

static WORD mediachk(rqptr rp)
{
  COUNT drive = miarray[rp->r_unit].mi_drive;
  COUNT result;

  /* if it's a hard drive, media never changes */
  if (hd(drive))
    rp->r_mcretcode = M_NOT_CHANGED;
  else
    /* else, check floppy status */
  {
    if ((result = fl_readdasd(drive)) == 2)	/* if we can detect a change ... */
    {
      if ((result = fl_diskchanged(drive)) == 1) /* check if it has changed... */
        rp->r_mcretcode = M_CHANGED;
      else if (result == 0)
        rp->r_mcretcode = M_NOT_CHANGED;
      else
        rp->r_mcretcode = tdelay((LONG) 37) ? M_DONT_KNOW : M_NOT_CHANGED;
    }
    else if (result == 3)       /* if it's a fixed disk, then no change */
      rp->r_mcretcode = M_NOT_CHANGED;
    else                        /* can not detect or error... */
      rp->r_mcretcode = tdelay((LONG) 37) ? M_DONT_KNOW : M_NOT_CHANGED;
  }

  return S_DONE;
}

/*
 *  Read Write Sector Zero or Hard Drive Dos Bpb
 */
static WORD RWzero(rqptr rp, WORD t)
{
  REG retry = N_RETRY;

  if (hd(miarray[rp->r_unit].mi_drive))
  {
    COUNT partidx = miarray[rp->r_unit].mi_partidx;
    head = dos_partition[partidx].peBeginHead;
    sector = dos_partition[partidx].peBeginSector;
    track = dos_partition[partidx].peBeginCylinder;
  }
  else
  {
    head = 0;
    sector = 1;
    track = 0;
  }

  do
  {
    if (!t)   /* 0 == Read */
        {
    ret = fl_read((WORD) miarray[rp->r_unit].mi_drive,
                  (WORD) head, (WORD) track, (WORD) sector, (WORD) 1, (byteptr) & buffer);
        }
    else
        {
    ret = fl_write((WORD) miarray[rp->r_unit].mi_drive,
                  (WORD) head, (WORD) track, (WORD) sector, (WORD) 1, (byteptr) & buffer);
        }
  }
  while (ret != 0 && --retry > 0);
  return ret;
}

static WORD bldbpb(rqptr rp)
{
  ULONG count, i;
  byteptr trans;
  WORD local_word;

  ret = RWzero( rp, 0);

  if (ret != 0)
    return (dskerr(ret));

  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NBYTE]), &bpbarray[rp->r_unit].bpb_nbyte);
  getbyte(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSECTOR]), &bpbarray[rp->r_unit].bpb_nsector);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NRESERVED]), &bpbarray[rp->r_unit].bpb_nreserved);
  getbyte(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NFAT]), &bpbarray[rp->r_unit].bpb_nfat);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NDIRENT]), &bpbarray[rp->r_unit].bpb_ndirent);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSIZE]), &bpbarray[rp->r_unit].bpb_nsize);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSIZE]), &bpbarray[rp->r_unit].bpb_nsize);
  getbyte(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_MDESC]), &bpbarray[rp->r_unit].bpb_mdesc);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NFSECT]), &bpbarray[rp->r_unit].bpb_nfsect);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NSECS]), &bpbarray[rp->r_unit].bpb_nsecs);
  getword(&((((BYTE *) & buffer.bytes[BT_BPB]))[BPB_NHEADS]), &bpbarray[rp->r_unit].bpb_nheads);
  getlong(&((((BYTE *) & buffer.bytes[BT_BPB])[BPB_HIDDEN])), &bpbarray[rp->r_unit].bpb_hidden);
  getlong(&((((BYTE *) & buffer.bytes[BT_BPB])[BPB_HUGE])), &bpbarray[rp->r_unit].bpb_huge);

/* Needs fat32 offset code */

  getlong(&((((BYTE *) & buffer.bytes[0x27])[0])), &fsarray[rp->r_unit].fs_serialno);
  for(i = 0; i < 11 ;i++ )
    fsarray[rp->r_unit].fs_volume[i] = buffer.bytes[0x2B + i];
  for(i = 0; i < 8; i++ )
    fsarray[rp->r_unit].fs_fstype[i] = buffer.bytes[0x36 + i];



#ifdef DSK_DEBUG
  printf("BPB_NBYTE     = %04x\n", bpbarray[rp->r_unit].bpb_nbyte);
  printf("BPB_NSECTOR   = %02x\n", bpbarray[rp->r_unit].bpb_nsector);
  printf("BPB_NRESERVED = %04x\n", bpbarray[rp->r_unit].bpb_nreserved);
  printf("BPB_NFAT      = %02x\n", bpbarray[rp->r_unit].bpb_nfat);
  printf("BPB_NDIRENT   = %04x\n", bpbarray[rp->r_unit].bpb_ndirent);
  printf("BPB_NSIZE     = %04x\n", bpbarray[rp->r_unit].bpb_nsize);
  printf("BPB_MDESC     = %02x\n", bpbarray[rp->r_unit].bpb_mdesc);
  printf("BPB_NFSECT    = %04x\n", bpbarray[rp->r_unit].bpb_nfsect);
#endif
  rp->r_bpptr = &bpbarray[rp->r_unit];
  count = miarray[rp->r_unit].mi_size =
      bpbarray[rp->r_unit].bpb_nsize == 0 ?
      bpbarray[rp->r_unit].bpb_huge :
      bpbarray[rp->r_unit].bpb_nsize;
  getword((&(((BYTE *) & buffer.bytes[BT_BPB])[BPB_NHEADS])), &miarray[rp->r_unit].mi_heads);
  head = miarray[rp->r_unit].mi_heads;
  getword((&(((BYTE *) & buffer.bytes[BT_BPB])[BPB_NSECS])), &miarray[rp->r_unit].mi_sectors);
  if (miarray[rp->r_unit].mi_size == 0)
    getlong(&((((BYTE *) & buffer.bytes[BT_BPB])[BPB_HUGE])), &miarray[rp->r_unit].mi_size);
  sector = miarray[rp->r_unit].mi_sectors;

  if (head == 0 || sector == 0)
  {
    tmark();
    return failure(E_FAILURE);
  }
  miarray[rp->r_unit].mi_cyls = count / (head * sector);
  tmark();

#ifdef DSK_DEBUG
  printf("BPB_NSECS     = %04x\n", sector);
  printf("BPB_NHEADS    = %04x\n", head);
  printf("BPB_HIDDEN    = %08lx\n", bpbarray[rp->r_unit].bpb_hidden);
  printf("BPB_HUGE      = %08lx\n", bpbarray[rp->r_unit].bpb_huge);
#endif
  return S_DONE;
}

static COUNT write_and_verify(WORD drive, WORD head, WORD track, WORD sector,
                              WORD count, BYTE FAR * buffer)
{
  REG COUNT ret;

  ret = fl_write(drive, head, track, sector, count, buffer);
  if (ret != 0)
    return ret;
  return fl_verify(drive, head, track, sector, count, buffer);
}

static WORD IoctlQueblk(rqptr rp)
{
    switch(rp->r_count){
        case 0x0846:
        case 0x0847:
        case 0x0860:
        case 0x0866:
        case 0x0867:
            break;
        default:
            return S_ERROR;
    }
  return S_DONE;

}

static WORD Genblkdev(rqptr rp)
{
    switch(rp->r_count){
        case 0x0860:            /* get device parameters */
        {
        struct gblkio FAR * gblp = (struct gblkio FAR *) rp->r_trans;
        REG COUNT x = 5,y = 1,z = 0;

        if (!hd(miarray[rp->r_unit].mi_drive)){
            y = 2;
            switch(miarray[rp->r_unit].mi_size)
            {
                case 640l:
                case 720l:      /* 320-360 */
                    x = 0;
                    z = 1;
                break;
                case 1440l:     /* 720 */
                    x = 2;
                break;
                case 2400l:     /* 1.2 */
                    x = 1;
                break;
                case 2880l:     /* 1.44 */
                    x = 7;
                break;
                case 5760l:     /* 2.88 almost forgot this one*/
                    x = 9;
                break;
                default:
                    x = 8;      /* any odd ball drives return this */
            }
        }
        gblp->gbio_devtype = (UBYTE) x;
        gblp->gbio_devattrib = (UWORD) y;
        gblp->gbio_media = (UBYTE) z;
        gblp->gbio_ncyl = miarray[rp->r_unit].mi_cyls;
        gblp->gbio_bpb = bpbarray[rp->r_unit];
        gblp->gbio_nsecs = bpbarray[rp->r_unit].bpb_nsector;
        break;
        }
        case 0x0866:        /* get volume serial number */
        {
        REG COUNT i;
        struct Gioc_media FAR * gioc = (struct Gioc_media FAR *) rp->r_trans;
        struct FS_info FAR * fs = &fsarray[rp->r_unit];

            gioc->ioc_serialno = fs->fs_serialno;
            for(i = 0; i < 11 ;i++ )
                gioc->ioc_volume[i] = fs->fs_volume[i];
            for(i = 0; i < 8; i++ )
                gioc->ioc_fstype[i] = fs->fs_fstype[i];
        }
        break;
        case 0x0846:        /* set volume serial number */
        {
        struct Gioc_media FAR * gioc = (struct Gioc_media FAR *) rp->r_trans;
        struct FS_info FAR * fs = (struct FS_info FAR *) &buffer.bytes[0x27];

            ret = RWzero( rp, 0);
            if (ret != 0)
                return (dskerr(ret));

            fs->fs_serialno =  gioc->ioc_serialno;
            fsarray[rp->r_unit].fs_serialno = fs->fs_serialno;

            ret = RWzero( rp, 1);
            if (ret != 0)
                return (dskerr(ret));
        }
        break;
        case 0x0867:        /* get access flag, always on*/
        {
        struct Access_info FAR * ai = (struct Access_info FAR *) rp->r_trans;
        ai->AI_Flag = 1;
        }
        break;
        case 0x0847:        /* set access flag, no real use*/
        break;
        default:
            return S_ERROR;
    }
  return S_DONE;
}


static WORD blockio(rqptr rp)
{
  REG retry = N_RETRY,
    remaining;
  UWORD cmd,
    total;
  ULONG start;
  byteptr trans;
  COUNT(*action) (WORD, WORD, WORD, WORD, WORD, BYTE FAR *);

  cmd = rp->r_command;
  total = 0;
  trans = rp->r_trans;
  tmark();
  for (
        remaining = rp->r_count,
        start = (rp->r_start != HUGECOUNT ? rp->r_start : rp->r_huge)
        + miarray[rp->r_unit].mi_offset;
        remaining > 0;
        remaining -= count, trans += count * SEC_SIZE, start += count
      )
  {
    count = ltop(&track, &sector, &head, rp->r_unit, remaining, start, trans);
    do
    {
      switch (cmd)
      {
        case C_INPUT:
          action = fl_read;
          break;
        case C_OUTPUT:
          action = fl_write;
          break;
        case C_OUTVFY:
          action = write_and_verify;
          break;
        default:
          return failure(E_FAILURE);
      }
      if (count)
        ret = action((WORD) miarray[rp->r_unit].mi_drive, head, track, sector,
                     count, trans);
      else
      {
        count = 1;
        /* buffer crosses DMA boundary, use scratchpad */
        if (cmd != C_INPUT)
          fbcopy(trans, dma_scratch, SEC_SIZE);
        ret = action((WORD) miarray[rp->r_unit].mi_drive, head, track, sector,
                     1, dma_scratch);
        if (cmd == C_INPUT)
          fbcopy(dma_scratch, trans, SEC_SIZE);
      }
      if (ret != 0)
        fl_reset((WORD) miarray[rp->r_unit].mi_drive);
    }
    while (ret != 0 && --retry > 0);
    if (ret != 0)
    {
      rp->r_count = total;
      return dskerr(ret);
    }
    total += count;
  }
  rp->r_count = total;
  return S_DONE;
}

static WORD blk_error(rqptr rp)
{
  rp->r_count = 0;
  return failure(E_FAILURE);    /* general failure */
}

static WORD dskerr(COUNT code)
{
/*      printf("diskette error:\nhead = %d\ntrack = %d\nsector = %d\ncount = %d\n",
   head, track, sector, count); */
  switch (code & 0x03)
  {
    case 1:                    /* invalid command - general failure */
      if (code & 0x08)
        return (E_FAILURE);
      else
        return failure(E_CMD);

    case 2:                    /* address mark not found - general  failure */
      return failure(E_FAILURE);

    case 3:                    /* write protect */
      return failure(E_WRPRT);

    default:
      if (code & 0x80)          /* time-out */
        return failure(E_NOTRDY);
      else if (code & 0x40)     /* seek error */
        return failure(E_SEEK);
      else if (code & 0x10)     /* CRC error */
        return failure(E_CRC);
      else if (code & 0x04)
        return failure(E_NOTFND);
      else
        return failure(E_FAILURE);
  }
}

/*                                                                      */
/* Do logical block number to physical head/track/sector mapping        */
/*                                                                      */
static COUNT ltop(WORD * trackp, WORD * sectorp, WORD * headp, REG COUNT unit, COUNT count, LONG strt_sect, byteptr strt_addr)
{
#ifdef I86
  ULONG ltemp;
#endif
  REG ls,
    ps;

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
  if (*sectorp + count > miarray[unit].mi_sectors + 1)
    count = miarray[unit].mi_sectors + 1 - *sectorp;
  return count;
}
