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
#include "dyndata.h"

#ifdef VERSION_STRINGS
static BYTE *dskRcsId =
    "$Id$";
#endif

#if defined(DEBUG)
#define DebugPrintf(x) printf x
#else
#define DebugPrintf(x)
#endif

void ASMPASCAL fl_readkey(void);
int ASMPASCAL fl_reset(UBYTE drive);
int ASMPASCAL fl_diskchanged(UBYTE drive);
int ASMPASCAL fl_setdisktype(UBYTE drive, WORD type);
int ASMPASCAL fl_setmediatype(UBYTE drive, WORD tracks, WORD sectors);
int ASMPASCAL fl_read  (UBYTE drive, WORD, WORD, WORD, WORD, void FAR *);
int ASMPASCAL fl_write (UBYTE drive, WORD, WORD, WORD, WORD, void FAR *);
int ASMPASCAL fl_verify(UBYTE drive, WORD, WORD, WORD, WORD, void FAR *);
int ASMPASCAL fl_format(UBYTE drive, WORD, WORD, WORD, WORD, void FAR *);
int ASMPASCAL fl_lba_ReadWrite(UBYTE drive, WORD mode,
                               struct _bios_LBA_address_packet FAR *);
#ifdef __WATCOMC__
#pragma aux (pascal) fl_readkey modify exact [ax]
#pragma aux (pascal) fl_reset modify exact [ax dx]
#pragma aux (pascal) fl_diskchanged modify exact [ax dx]
#pragma aux (pascal) fl_setdisktype modify exact [ax dx bx]
#pragma aux (pascal) fl_setmediatype modify exact [ax cx dx bx es]
#pragma aux (pascal) fl_read   modify exact [ax cx dx bx es]
#pragma aux (pascal) fl_write  modify exact [ax cx dx bx es]
#pragma aux (pascal) fl_format modify exact [ax cx dx bx es]
#pragma aux (pascal) fl_verify modify exact [ax cx dx bx es]
#pragma aux (pascal) fl_lba_ReadWrite modify exact [ax dx]
#endif

STATIC int LBA_Transfer(ddt * pddt, UWORD mode, VOID FAR * buffer,
                 ULONG LBA_address, unsigned total, UWORD * transferred);

#define LBA_READ         0x4200
#define LBA_WRITE        0x4300
UWORD LBA_WRITE_VERIFY = 0x4302;
#define LBA_VERIFY       0x4400
#define LBA_FORMAT       0xffff /* fake number for FORMAT track
                                   (only for NON-LBA floppies now!) */

                /* this buffer must not overlap a 64K boundary
                   due to DMA transfers
                   this is certainly true, if located somewhere
                   at 0xf+1000 and must hold already during BOOT time
                 */
UBYTE DiskTransferBuffer[1 * SEC_SIZE];

struct FS_info {
  ULONG serialno;
  BYTE volume[11];
  BYTE fstype[8];
};

extern struct DynS ASM Dyn;

/*TE - array access functions */
ddt *getddt(int dev)
{
  return (ddt*)Dyn.Buffer + dev;
}

#define getddt0() ((ddt*)Dyn.Buffer)

#define tmark(pddt) ((pddt)->ddt_fh.ddt_lasttime = ReadPCClock())

STATIC BOOL tdelay(ddt *pddt, ULONG ticks)
{
  return ReadPCClock() - pddt->ddt_fh.ddt_lasttime >= ticks;
}

#define N_PART 4 /* number of partitions per partition table */

#define PARTOFF 0x1be

typedef WORD dsk_proc(rqptr, ddt*);

STATIC dsk_proc mediachk, bldbpb, blockio, IoctlQueblk,
    Genblkdev, Getlogdev, Setlogdev, blk_Open, blk_Close,
    blk_Media, blk_noerr, blk_nondr;

STATIC WORD getbpb(ddt*);
STATIC WORD dskerr(COUNT);

/*                                                                      */
/* the function dispatch table                                          */
/*                                                                      */

static dsk_proc * const dispatch [] =
{
      /* disk init is done in initdisk.c, so this should never be called */
      NULL,                     /* 0x00 Initialize                   */
      mediachk,                 /* 0x01 Media Check                  */
      bldbpb,                   /* 0x02 Build BPB                    */
      NULL,                     /* 0x03 Ioctl In                     */
      blockio,                  /* 0x04 Input (Read)                 */
      blk_nondr,                /* 0x05 Non-destructive Read         */
      blk_noerr,                /* 0x06 Input Status                 */
      blk_noerr,                /* 0x07 Input Flush                  */
      blockio,                  /* 0x08 Output (Write)               */
      blockio,                  /* 0x09 Output with verify           */
      blk_noerr,                /* 0x0A Output Status                */
      blk_noerr,                /* 0x0B Output Flush                 */
      NULL,                     /* 0x0C Ioctl Out                    */
      blk_Open,                 /* 0x0D Device Open                  */
      blk_Close,                /* 0x0E Device Close                 */
      blk_Media,                /* 0x0F Removable Media              */
      blk_noerr,                /* 0x10 Output till busy             */
      NULL,                     /* 0x11 undefined                    */
      NULL,                     /* 0x12 undefined                    */
      Genblkdev,                /* 0x13 Generic Ioctl Call           */
      NULL,                     /* 0x14 undefined                    */
      NULL,                     /* 0x15 undefined                    */
      NULL,                     /* 0x16 undefined                    */
      Getlogdev,                /* 0x17 Get Logical Device           */
      Setlogdev,                /* 0x18 Set Logical Device           */
      IoctlQueblk               /* 0x19 Ioctl Query                  */
};

#define hd(x)   ((x) & DF_FIXED)

/* ----------------------------------------------------------------------- */
/*  F U N C T I O N S  --------------------------------------------------- */
/* ----------------------------------------------------------------------- */

int ASMCFUNC FAR blk_driver(rqptr rp)
{
  if (rp->r_command >= LENGTH (dispatch))
    return failure(E_FAILURE); /* general failure */
  {
    dsk_proc *const proc = dispatch [rp->r_command];
    if (proc == NULL)
    {
      rp->r_count = 0;
      return failure(E_FAILURE); /* general failure */
    }
    if (rp->r_unit >= blk_dev.dh_name[0])
      return failure(E_UNIT);
    return proc (rp, getddt(rp->r_unit));
  }
}

STATIC char template_string[] =
 "\nInsert diskette for drive :: and press any key when ready\n\n";
/* 012345678901234567890123456^ */
#define DRIVE_POS 27

STATIC WORD play_dj(ddt * pddt)
{
  UBYTE i;
  ddt *pddt2;

  if ((pddt->ddt_descflags & (DF_MULTLOG | DF_CURLOG)) != DF_MULTLOG)
    return M_NOT_CHANGED;

  /* play the DJ ... */
  pddt2 = getddt0() - 1;
  i = blk_dev.dh_name[0] + 1;
  do
  {
    pddt2++, i--;
    if (i == 0)
    {
      put_string("Error in the DJ mechanism!\n"); /* should not happen! */
      return M_CHANGED;
    }
  } while (pddt->ddt_driveno != pddt2->ddt_driveno ||
           (~pddt2->ddt_descflags & (DF_MULTLOG | DF_CURLOG)));

  template_string[DRIVE_POS] = 'A' + pddt->ddt_logdriveno;
  put_string(template_string);
  fl_readkey();
  pddt2->ddt_descflags &= ~DF_CURLOG;
  pddt->ddt_descflags |= DF_CURLOG;
  pokeb(0, 0x504, pddt->ddt_logdriveno);
  return M_CHANGED;
}

STATIC WORD diskchange(ddt * pddt)
{
  /* if it's a hard drive, media never changes */
  if (hd(pddt->ddt_descflags))
    return M_NOT_CHANGED;

  if (play_dj(pddt) == M_CHANGED)
    return M_CHANGED;

  if (pddt->ddt_descflags & DF_CHANGELINE)      /* if we can detect a change ... */
  {
    int ret = fl_diskchanged(pddt->ddt_driveno);
    if (ret == 1)
      /* check if it has changed... */
      return M_CHANGED;
    if (ret == 0)
      return M_NOT_CHANGED;
  }

  /* can not detect or error... */
  if (tdelay(pddt, 37))
    return M_DONT_KNOW;
  return M_NOT_CHANGED;
}

STATIC WORD mediachk(rqptr rp, ddt * pddt)
{
  /* check floppy status */
  if (pddt->ddt_descflags & DF_REFORMAT)
  {
    pddt->ddt_descflags &= ~DF_REFORMAT;
    rp->r_mcretcode = M_CHANGED;
  }
  else if (pddt->ddt_descflags & DF_DISKCHANGE)
  {
    pddt->ddt_descflags &= ~DF_DISKCHANGE;
    rp->r_mcretcode = M_DONT_KNOW;
  }
  else if ((rp->r_mcretcode = diskchange(pddt)) == M_DONT_KNOW)
  {
    /* don't know but can check serial number ... */
    ULONG serialno = pddt->ddt_serialno;
    int ret = getbpb(pddt);
    if (ret)
      return ret;
    if (serialno != pddt->ddt_serialno)
      rp->r_mcretcode = M_CHANGED;
  }
  return S_DONE;
}

/*
 *  Read Write Sector Zero or Hard Drive Dos Bpb
 */
STATIC int RWzero(ddt * pddt, UWORD mode)
{
  UWORD done;
  return LBA_Transfer(pddt, mode, DiskTransferBuffer, 0, 1, &done);
}

/*
   0 if not set, 1 = a, 2 = b, etc, assume set.
   page 424 MS Programmer's Ref.
 */
STATIC WORD Getlogdev(rqptr rp, ddt * pddt)
{
  UBYTE unit = 0;
  if (pddt->ddt_descflags & DF_MULTLOG)
  {
    ddt *pddt2 = getddt0() - 1;
    do
      pddt2++, unit++;
    while (unit < blk_dev.dh_name[0] &&
           (pddt->ddt_driveno != pddt2->ddt_driveno ||
            (~pddt2->ddt_descflags & (DF_MULTLOG | DF_CURLOG))));
  }
  rp->r_unit = unit;
  return S_DONE;
}

STATIC WORD Setlogdev(rqptr rp, ddt * pddt)
{
  UBYTE unit = rp->r_unit + 1;
  Getlogdev(rp, pddt);
  if (rp->r_unit)
  {
    getddt(rp->r_unit - 1)->ddt_descflags &= ~DF_CURLOG;
    pddt->ddt_descflags |= DF_CURLOG;
    /* UNDOCUMENTED: MS-DOS sets r_unit field both for			*/
    /* 0x17 (Getlogdev()) and 0x18 (Setlogdev()) functions.		*/
    rp->r_unit = unit;
  }
  return S_DONE;
}

STATIC WORD blk_Open(rqptr rp, ddt * pddt)
{
  UNREFERENCED_PARAMETER(rp);

  pddt->ddt_FileOC++;
  return S_DONE;
}

STATIC WORD blk_Close(rqptr rp, ddt * pddt)
{
  UNREFERENCED_PARAMETER(rp);

  pddt->ddt_FileOC--;
  return S_DONE;
}

STATIC WORD blk_nondr(rqptr rp, ddt * pddt)
{
  UNREFERENCED_PARAMETER(rp);
  UNREFERENCED_PARAMETER(pddt);

  return S_BUSY | S_DONE;
}

STATIC WORD blk_Media(rqptr rp, ddt * pddt)
{
  UNREFERENCED_PARAMETER(rp);

  if (hd(pddt->ddt_descflags))
    return S_BUSY | S_DONE;     /* Hard Drive */
  return S_DONE;                /* Floppy */
}

STATIC WORD getbpb(ddt * pddt)
{
  /* pddt->ddt_descflags |= DF_NOACCESS;
   * disabled for now - problems with FORMAT ?? */

  /* set drive to not accessible and changed */
  if (diskchange(pddt) != M_NOT_CHANGED)
    pddt->ddt_descflags |= DF_DISKCHANGE;

  {
    int ret = RWzero(pddt, LBA_READ);
    if (ret)
      return ret;
  }

  if (getword(DiskTransferBuffer + 0x1fe)  != 0xaa55 ||
      getword(DiskTransferBuffer + BT_BPB) != 512) /* bpb_nbyte */
  {
    /* copy default bpb to be sure that there is no bogus data */
    memcpy(&pddt->ddt_bpb, &pddt->ddt_defbpb, sizeof pddt->ddt_bpb);
    return S_DONE;
  }

  pddt->ddt_descflags &= ~DF_NOACCESS; /* set drive to accessible */

/*TE ~ 200 bytes*/

  memcpy(&pddt->ddt_bpb, DiskTransferBuffer + BT_BPB, sizeof pddt->ddt_bpb);

  /*?? */
  /*  2b is fat16 volume label. if memcmp, then offset 0x36.
     if (memcmp(DiskTransferBuffer + 0x36, "FAT16", 5) == 0 ||
         memcmp(DiskTransferBuffer + 0x36, "FAT12", 5) == 0)
     TE: I'm not sure, what the _real_ decision point is, however MSDN
     'A_BF_BPB_SectorsPerFAT
     The number of sectors per FAT.
     Note: This member will always be zero in a FAT32 BPB.
     Use the values from A_BF_BPB_BigSectorsPerFat...
  */
  {
    struct FS_info *fs = (struct FS_info *)&DiskTransferBuffer[0x27];
#ifdef WITHFAT32
    if (pddt->ddt_bpb.bpb_nfsect == 0)
    {
      /* FAT32 boot sector */
      fs = (struct FS_info *)&DiskTransferBuffer[0x43];
    }
#endif
    pddt->ddt_serialno = getlong(&fs->serialno);
    memcpy(pddt->ddt_volume, fs->volume, sizeof fs->volume);
    memcpy(pddt->ddt_fstype, fs->fstype, sizeof fs->fstype);
  }

#ifdef DSK_DEBUG
  printf("BPB_NBYTE     = %04x\n"
	 "BPB_NSECTOR   = %02x\n"
	 "BPB_NRESERVED = %04x\n"
	 "BPB_NFAT      = %02x\n"
	 "BPB_NDIRENT   = %04x\n"
	 "BPB_NSIZE     = %04x\n"
	 "BPB_MDESC     = %02x\n"
	 "BPB_NFSECT    = %04x\n",
			pddt->ddt_bpb.bpb_nbyte,
			pddt->ddt_bpb.bpb_nsector,
			pddt->ddt_bpb.bpb_nreserved,
			pddt->ddt_bpb.bpb_nfat,
			pddt->ddt_bpb.bpb_ndirent,
			pddt->ddt_bpb.bpb_nsize,
			pddt->ddt_bpb.bpb_mdesc,
			pddt->ddt_bpb.bpb_nfsect);
#endif

  tmark(pddt);

  {
    unsigned secs_per_cyl = pddt->ddt_bpb.bpb_nheads * pddt->ddt_bpb.bpb_nsecs;
    if (secs_per_cyl == 0)
      return failure(E_FAILURE);

    /* this field is problematic for partitions > 65535 cylinders,
       in general > 512 GiB. However: we are not using it ourselves. */
    {
      unsigned nsize = pddt->ddt_bpb.bpb_nsize;
      pddt->ddt_ncyl = (UWORD)(((nsize ? nsize : pddt->ddt_bpb.bpb_huge) - 1)
						/ secs_per_cyl) + 1;
    }
  }

#ifdef DSK_DEBUG
  printf("BPB_NSECS     = %04x\n"
	 "BPB_NHEADS    = %04x\n"
	 "BPB_HIDDEN    = %08lx\n"
	 "BPB_HUGE      = %08lx\n",
			pddt->ddt_bpb.bpb_nsecs,
			pddt->ddt_bpb.bpb_nheads,
			pddt->ddt_bpb.bpb_hidden,
			pddt->ddt_bpb.bpb_huge);
#endif

  return 0;
}

STATIC WORD bldbpb(rqptr rp, ddt * pddt)
{
  int ret = getbpb(pddt);
  if (ret)
    return ret;
  rp->r_bpptr = &pddt->ddt_bpb;
  return S_DONE;
}

STATIC WORD IoctlQueblk(rqptr rp, ddt * pddt)
{
  UNREFERENCED_PARAMETER(pddt);

#ifdef WITHFAT32
  if ((UBYTE)(~0x40 & rp->r_cat) != 8 ||	/* 0x08,0x48 */
#else
  if (rp->r_cat != 8 ||
#endif
      (rp->r_fun != 0x60 &&
       (UBYTE)(~0x21 & rp->r_fun) != 0x46))	/* 0x46,0x47,0x66,0x67 */
    return failure(E_CMD);
  return S_DONE;
}

STATIC COUNT Genblockio(ddt * pddt, UWORD mode, WORD head, WORD track,
                 WORD sector, WORD count, VOID FAR * buffer)
{
  UWORD done;

  /* apparently here sector is ZERO, not ONE based !!! */
  return LBA_Transfer(pddt, mode, buffer,
                      ((ULONG) track * pddt->ddt_bpb.bpb_nheads
                             + head) * pddt->ddt_bpb.bpb_nsecs + sector,
                      count, &done);
}

STATIC WORD Genblkdev(rqptr rp, ddt * pddt)
{
  unsigned descflags = pddt->ddt_descflags;
#ifdef WITHFAT32
  unsigned copy_size = sizeof (bpb);

  if (rp->r_cat != 0x48)
  {
    if (rp->r_cat != 8)
      return failure(E_CMD);
    copy_size = BPB_SIZEOF;
  }
#else
  if (rp->r_cat != 8)
    return failure(E_CMD);
#endif

  switch (rp->r_fun)
  {
    case 0x40:                 /* set device parameters */
      {
        struct gblkio FAR *gblp = rp->r_io;
        bpb *pbpb;

        pddt->ddt_type = gblp->gbio_devtype;
        pddt->ddt_descflags = (descflags & ~3) |
                              (gblp->gbio_devattrib & 3) |
                              (DF_DPCHANGED | DF_REFORMAT);
        pddt->ddt_ncyl = gblp->gbio_ncyl;
        /* use default dpb or current bpb? */
        pbpb = (gblp->gbio_spcfunbit & 1) ? &pddt->ddt_bpb : &pddt->ddt_defbpb;
#ifdef WITHFAT32
        fmemcpy(pbpb, &gblp->gbio_bpb, copy_size);
#else
        fmemcpy(pbpb, &gblp->gbio_bpb, sizeof(gblp->gbio_bpb));
#endif
        /*pbpb->bpb_nsector = gblp->gbio_nsecs;*/
        break;
      }
    case 0x41:                 /* write track */
      {
        struct gblkrw FAR *rw = rp->r_rw;
        int ret = Genblockio(pddt, LBA_WRITE, rw->gbrw_head, rw->gbrw_cyl,
                             rw->gbrw_sector, rw->gbrw_nsecs, rw->gbrw_buffer);
        if (ret)
          return ret;
        break;
      }
    case 0x42:                 /* format/verify track */
      {
        struct gblkfv FAR *fv = rp->r_fv;
        int ret;

        pddt->ddt_descflags &= ~DF_DPCHANGED;
        if (hd(descflags))
        {
          /* XXX no low-level formatting for hard disks implemented */
          fv->gbfv_spcfunbit = 1; /* "not supported by bios" */
          break;
        }

        /* first try newer setmediatype function */
        if ((descflags & DF_DPCHANGED) &&
            (ret = fl_setmediatype(pddt->ddt_driveno, pddt->ddt_ncyl,
                                   pddt->ddt_bpb.bpb_nsecs)) != 0)
        {
          if (ret == 0xc)
          {
            /* specified tracks, sectors/track not allowed for drive */
            fv->gbfv_spcfunbit = 2;
            return failure(E_NOTFND); /*dskerr(0xc)*/
          }
          if (ret == 0x80 ||
              (fv->gbfv_spcfunbit & 1) &&
               (ret = fl_read(pddt->ddt_driveno, 0, 0, 1, 1,
                              DiskTransferBuffer)) != 0)
          {
            fv->gbfv_spcfunbit = 3; /* no disk in drive */
            return dskerr(ret);
          }
          /* otherwise, setdisktype */
          {
            /* type 1: 320/360K disk in 360K drive */
            /* type 2: 320/360K disk in 1.2M drive */
            unsigned tracks = pddt->ddt_ncyl;
            unsigned secs = pddt->ddt_bpb.bpb_nsecs;
            UBYTE type = pddt->ddt_type + 1;
            if (!(tracks == 40 && (secs == 9 || secs == 8) && type < 3))
            {
              /* type 3: 1.2M disk in 1.2M drive */
              /* type 4: 720kb disk in 1.44M or 720kb drive */
              type++;
              if (type == 9) /* 1.44M drive */
                type = 4;
              if (!(tracks == 80 && ((secs == 15 && type == 3) ||
                                     (secs == 9 && type == 4))))
              {
                /* specified tracks, sectors/track not allowed for drive */
                fv->gbfv_spcfunbit = 2;
                return failure(E_NOTFND); /*dskerr(0xc)*/
              }
            }
            fl_setdisktype(pddt->ddt_driveno, type);
          }
        }
        if (fv->gbfv_spcfunbit & 1)
        {
          fv->gbfv_spcfunbit = 0; /* success */
          break;
        }

        {
          unsigned cyl = fv->gbfv_cyl;
          unsigned head = fv->gbfv_head;
          unsigned tracks = fv->gbfv_spcfunbit & 2 ? fv->gbfv_ntracks : 1;

          for (; tracks; tracks--)
          {
            if (cyl >= pddt->ddt_ncyl)		/* ??? remove --avb */
              return failure(E_FAILURE);
            {
              struct thst {
                UBYTE cyl, head, sector, type;
              } *addrfield = (struct thst *)DiskTransferBuffer;
              unsigned sector = 0;
              do
              {
                sector++;
                addrfield->type = 2; /* 512 byte sectors */
                addrfield->cyl = cyl;
                addrfield->head = head;
                addrfield->sector = sector;
                addrfield++;
              } while (sector < pddt->ddt_bpb.bpb_nsecs);
            }
            {
              int ret = Genblockio(pddt, LBA_FORMAT, head, cyl, 0,
                                   pddt->ddt_bpb.bpb_nsecs, DiskTransferBuffer);
              if (ret)
                return ret;
            }
            if (++head >= pddt->ddt_bpb.bpb_nheads)
            {
              head = 0;
              cyl++;
            }
          }
        }

        fv->gbfv_spcfunbit >>= 1; /* move bit 1 to bit 0 */
      }
      /* fall through to verify */

    case 0x62:                 /* verify track */
      {
        struct gblkfv FAR *fv = rp->r_fv;
        int ret = Genblockio(pddt, LBA_VERIFY, fv->gbfv_head, fv->gbfv_cyl, 0,
                             (fv->gbfv_spcfunbit & 1)
                              ? fv->gbfv_ntracks * pddt->ddt_defbpb.bpb_nsecs
                              :                    pddt->ddt_defbpb.bpb_nsecs,
                             DiskTransferBuffer);
        /* !!! ret should be analyzed to fill fv->gbfv_spcfunbit by
           1=function not supported by BIOS
           2=specified tracks, sector/track not allowed for drive
           3=no disk in drive
           --avb
        */
        if (ret)
          return ret;
        fv->gbfv_spcfunbit = 0; /* success */
        break;
      }
    case 0x46:                 /* set volume serial number */
      {
        struct FS_info *fs;
        int ret = getbpb(pddt);
        if (ret)
          return ret;
        fs = (struct FS_info *)(pddt->ddt_bpb.bpb_nfsect
                                ? DiskTransferBuffer + 0x27
                                : DiskTransferBuffer + 0x43);
        pddt->ddt_serialno = fs->serialno = rp->r_gioc->ioc_serialno;
        ret = RWzero(pddt, LBA_WRITE);
        if (ret)
          return ret;
        break;
      }
    case 0x47:                 /* set access flag */
      pddt->ddt_descflags |= DF_NOACCESS;
      if (rp->r_ai->AI_Flag)
        pddt->ddt_descflags &= ~DF_NOACCESS;
      break;
    case 0x60:                 /* get device parameters */
      {
        struct gblkio FAR *gblp = rp->r_io;
        bpb *pbpb;

        gblp->gbio_devtype = pddt->ddt_type;
        gblp->gbio_devattrib = descflags & 3;
        /* 360 kb disk in 1.2 MB drive */
        gblp->gbio_media = (pddt->ddt_type == 1) && (pddt->ddt_ncyl == 40);
        gblp->gbio_ncyl = pddt->ddt_ncyl;
        /* use default dpb or current bpb? */
        pbpb = (gblp->gbio_spcfunbit & 1) ? &pddt->ddt_bpb : &pddt->ddt_defbpb;
#ifdef WITHFAT32
        fmemcpy(&gblp->gbio_bpb, pbpb, copy_size);
#else
        fmemcpy(&gblp->gbio_bpb, pbpb, sizeof(gblp->gbio_bpb));
#endif
        /*gblp->gbio_nsecs = pbpb->bpb_nsector;*/
        break;
      }
    case 0x61:                 /* read track */
      {
        struct gblkrw FAR *rw = rp->r_rw;
        int ret = Genblockio(pddt, LBA_READ, rw->gbrw_head, rw->gbrw_cyl,
                             rw->gbrw_sector, rw->gbrw_nsecs, rw->gbrw_buffer);
        if (ret)
          return ret;
        break;
      }
    case 0x66:                 /* get volume serial number */
      {
        struct Gioc_media FAR *gioc;
        int ret = getbpb(pddt);
        if (ret)
          return ret;

        gioc = rp->r_gioc;
        gioc->ioc_serialno = pddt->ddt_serialno;
        fmemcpy(gioc->ioc_volume, pddt->ddt_volume, sizeof gioc->ioc_volume);
        fmemcpy(gioc->ioc_fstype, pddt->ddt_fstype, sizeof gioc->ioc_fstype);
        break;
      }
    case 0x67:                 /* get access flag */
      rp->r_ai->AI_Flag = (UBYTE)~(descflags / DF_NOACCESS) & 1; /* bit 9 */
      break;
    default:
      return failure(E_CMD);
  }
  return S_DONE;
}

STATIC WORD blockio(rqptr rp, ddt * pddt)
{
  ULONG start;
  int action;

  switch (rp->r_command)
  {
    case C_INPUT:
      action = LBA_READ;
      break;
    case C_OUTPUT:
      action = LBA_WRITE;
      break;
    case C_OUTVFY:
      action = LBA_WRITE_VERIFY;
      break;
    default:
      return failure(E_FAILURE);
  }

  if (pddt->ddt_descflags & DF_NOACCESS)      /* drive inaccessible */
    return failure(E_FAILURE);

  tmark(pddt);
  start = (rp->r_start != HUGECOUNT ? rp->r_start : rp->r_huge);
  {
    const bpb *pbpb = hd(pddt->ddt_descflags) ? &pddt->ddt_defbpb : &pddt->ddt_bpb;
    ULONG size = (pbpb->bpb_nsize ? pbpb->bpb_nsize : pbpb->bpb_huge);
    if (start >= size || rp->r_count > size - start)
      return 0x0408;
  }

  {
    UWORD done;
    int ret = LBA_Transfer(pddt, action, rp->r_trans,
                           start, rp->r_count, &done);
    rp->r_count = done;
    if (ret)
      return ret;
  }
  return S_DONE;
}

STATIC WORD blk_noerr(rqptr rp, ddt * pddt)
{
  UNREFERENCED_PARAMETER(rp);
  UNREFERENCED_PARAMETER(pddt);

  return S_DONE;
}

STATIC WORD dskerr(COUNT code)
{
/*      printf("diskette error:\nhead = %d\ntrack = %d\nsector = %d\ncount = %d\n",
   head, track, sector, count); */
  switch (code & 0x03)
  {
    case 1:                    /* invalid command - general failure */
      if (code & 0x08)
        return S_ERROR | E_NOTRDY; /* failure(E_NOTRDY); at least on the
                                      INT25 route, 0x8002 is returned */
      return failure(E_CMD);

    case 3:                    /* write protect */
      return failure(E_WRPRT);

    default:
      if (code & 0x80)          /* time-out */
        return failure(E_NOTRDY);
      if (code & 0x40)          /* seek error */
        return failure(E_SEEK);
      if (code & 0x10)          /* CRC error */
        return failure(E_CRC);
      if (code & 0x04)
        return failure(E_NOTFND);

    case 2:                    /* address mark not found - general failure */
      return failure(E_FAILURE);
  }
}

/*
    translate LBA sectors into CHS addressing
*/

STATIC void LBA_to_CHS(ULONG LBA_address, struct CHS *chs, const ddt * pddt)
{
  /* we need the defbpb values since those are taken from the
     BIOS, not from some random boot sector, except when
     we're dealing with a floppy */

  const bpb *p = hd(pddt->ddt_descflags) ? &pddt->ddt_defbpb : &pddt->ddt_bpb;
  unsigned hs = p->bpb_nsecs * p->bpb_nheads;
  chs->Cylinder = 0xffffu;
  if (hs > hiword(LBA_address))	/* LBA_address < hs * 0x10000ul	*/
  {
    chs->Cylinder = (unsigned)(LBA_address / hs);
	       hs = (unsigned)(LBA_address % hs);
    chs->Head     = hs / p->bpb_nsecs;
    chs->Sector   = hs % p->bpb_nsecs + 1;
  }
}

  /* Test for 64K boundary crossing and return count small        */
  /* enough not to exceed the threshold.                          */

STATIC unsigned DMA_max_transfer(void FAR * buffer, unsigned count)
{
  unsigned dma_off = (UWORD)((FP_SEG(buffer) << 4) + FP_OFF(buffer));
  unsigned sectors_to_dma_boundary = (dma_off == 0 ?
    0xffff / SEC_SIZE :
    (UWORD)(-dma_off) / SEC_SIZE);

  return min(count, sectors_to_dma_boundary);
}

/*
    int LBA_Transfer(
        ddt *pddt,                          physical characteristics of drive
        UWORD mode,                         LBA_READ/WRITE/WRITE_VERIFY/VERIFY
        VOID FAR *buffer,                   user buffer
        ULONG LBA_address,                  absolute sector address
        unsigned totaltodo,                 number of sectors to transfer
        UWORD *transferred                  sectors actually transferred

    Read/Write/Write+verify some sectors, using LBA addressing.

    This function handles all the minor details, including:
        retry in case of errors
        crossing the 64K DMA boundary
        translation to CHS addressing if necessary
        crossing track boundaries (necessary for some BIOS's)
        High memory doesn't work very well, use internal buffer
        write with verify details for LBA
*/

STATIC int LBA_Transfer(ddt * pddt, UWORD mode, VOID FAR * buffer,
                 ULONG LBA_address, unsigned totaltodo,
                 UWORD * transferred)
{
  *transferred = 0;

  /* only low-level format floppies for now ! */
  if (mode == LBA_FORMAT && hd(pddt->ddt_descflags))
    return 0;

  /* optionally change from A: to B: or back */
  play_dj(pddt);

  if (!hd(pddt->ddt_descflags))
  {
    UBYTE FAR *int1e_ptr = (UBYTE FAR *)getvec(0x1e);
    UBYTE nsecs = (UBYTE)pddt->ddt_bpb.bpb_nsecs;
    if (int1e_ptr[4] != nsecs)
    {
      int1e_ptr[4] = nsecs;
      fl_reset(pddt->ddt_driveno);
    }
  }

  LBA_address += pddt->ddt_offset;
/*
    if (LBA_address+totaltodo > pddt->total_sectors)
        {
        printf("LBA-Transfer error : address overflow = %lu > %lu max\n",LBA_address+totaltodo,driveParam->total_sectors);
        return failure(E_CMD); // dskerr(1)
        }
*/

  buffer = adjust_far(buffer);
  while (totaltodo)
  {
    int num_retries;

    /* avoid overflowing 64K DMA boundary */
    void FAR *transfer_address = buffer;
    unsigned count = DMA_max_transfer(buffer, totaltodo);
    if (FP_SEG(buffer) >= 0xa000 || count == 0)
    {
      transfer_address = DiskTransferBuffer;
      count = 1;
      if ((mode & 0xff00) == (LBA_WRITE & 0xff00))
        fmemcpy(DiskTransferBuffer, buffer, 512);
    }

    for (num_retries = N_RETRY;;)
    {
      unsigned error_code;
      if (mode != LBA_FORMAT && (pddt->ddt_descflags & DF_LBA))
      {
        UWORD m;
        static struct _bios_LBA_address_packet dap = {
          16, 0, 0, 0, 0, 0, 0
        };
        dap.number_of_blocks = count;
        dap.buffer_address = transfer_address;
        dap.block_address = LBA_address;
        dap.block_address_high = 0;     /* clear high part */

        m = mode;
        if (mode == LBA_WRITE_VERIFY && !(pddt->ddt_descflags & DF_WRTVERIFY))
        {
          /* verify requested, but not supported */
          error_code = fl_lba_ReadWrite(pddt->ddt_driveno, LBA_WRITE, &dap);
          if (error_code == 0)
          {
            m = LBA_VERIFY;
            error_code = fl_lba_ReadWrite(pddt->ddt_driveno, m, &dap);
          }
        }
        else
          error_code = fl_lba_ReadWrite(pddt->ddt_driveno, m, &dap);
      }
      else
      {                         /* transfer data, using old bios functions */
        struct CHS chs;
        LBA_to_CHS(LBA_address, &chs, pddt);
        if (chs.Cylinder > 1023u)
        {
#ifdef DEBUG
          printf("IO error: cylinder (%u) > 1023\n", chs.Cylinder);
#else
          put_string("IO error: cylinder > 1023\n");
#endif
          return failure(E_CMD); /*dskerr(1)*/
        }

        /* avoid overflow at end of track */
        if (count > pddt->ddt_bpb.bpb_nsecs + 1 - chs.Sector)
            count = pddt->ddt_bpb.bpb_nsecs + 1 - chs.Sector;

        error_code = (mode == LBA_READ ? fl_read :
                      mode == LBA_VERIFY ? fl_verify :
                      mode == LBA_FORMAT ? fl_format : fl_write)
                        (pddt->ddt_driveno, chs.Head, chs.Cylinder,
                         chs.Sector, count, transfer_address);
        if (error_code == 0 && mode == LBA_WRITE_VERIFY)
          error_code = fl_verify(pddt->ddt_driveno, chs.Head, chs.Cylinder,
                                 chs.Sector, count, transfer_address);
      }
      if (error_code == 0)
        break;

      fl_reset(pddt->ddt_driveno);
      if (--num_retries == 0)
        return dskerr(error_code);
    } /* end of retries */

    /* copy to user buffer if nesessary */
    if (transfer_address == DiskTransferBuffer &&
        (mode & 0xff00) == (LBA_READ & 0xff00))
      fmemcpy(buffer, DiskTransferBuffer, 512);

    *transferred += count;
    LBA_address += count;
    totaltodo -= count;

    buffer = adjust_far((char FAR *)buffer + count * 512u);
  }

  return 0;
}
