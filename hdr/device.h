/****************************************************************/
/*                                                              */
/*                           device.h                           */
/*                      Device Driver Header File               */
/*                                                              */
/*                       November 20, 1991                      */
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

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *device_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.4  2000/05/25 20:56:19  jimtabor
 * Fixed project history
 *
 * Revision 1.3  2000/05/11 04:24:51  jimtabor
 * Added Boot blk structs
 *
 * Revision 1.2  2000/05/08 04:28:22  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.4  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.3  2000/03/09 06:06:38  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/04 18:50:14  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:39:26  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/08 05:58:24  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/04 03:08:47  jprice
 * no message
 *
 * Revision 1.3  1999/02/01 01:40:06  jprice
 * Clean up
 *
 * Revision 1.2  1999/01/22 04:17:40  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *         Rev 1.8   06 Dec 1998  8:41:30   patv
 *      Changed for new I/O subsystem
 *
 *         Rev 1.7   11 Jan 1998  2:05:54   patv
 *      Added functionality to ioctl.
 *
 *         Rev 1.6   04 Jan 1998 23:14:20   patv
 *      Changed Log for strip utility
 *
 *         Rev 1.5   16 Jan 1997 12:46:06   patv
 *      pre-Release 0.92 feature additions
 *
 *         Rev 1.4   29 May 1996 21:25:12   patv
 *      bug fixes for v0.91a
 *
 *         Rev 1.3   19 Feb 1996  3:15:30   patv
 *      Added NLS, int2f and config.sys processing
 *
 *         Rev 1.2   01 Sep 1995 17:35:40   patv
 *      First GPL release.
 *
 *         Rev 1.1   30 Jul 1995 20:43:48   patv
 *      Eliminated version strings in ipl
 *
 *         Rev 1.0   02 Jul 1995 10:39:30   patv
 *      Initial revision.
 */

/*
 *      Status Word Bits
 */

#define S_ERROR         0x8000  /* Error bit                    */
#define S_BUSY          0x0200  /* Device busy bit              */
#define S_DONE          0x0100  /* Device operation completed   */
#define S_MASK          0x00ff  /* Mask to extract error code   */

/*
 *      MEDIA Descriptor Byte Bits
 */

#define MD_2SIDE        1       /* MEDIA is two sided           */
#define MD_8SECTOR      2       /* MEDIA is eight sectored      */
#define MD_REMOVABLE    4       /* MEDIA is removable (floppy)  */

/*
 *      Media Return Codes
 */
#define M_CHANGED       -1      /* MEDIA was changed            */
#define M_DONT_KNOW     0       /* MEDIA state unkown           */
#define M_NOT_CHANGED   1       /* MEDIA was not changed        */

/*
 *      Error Return Codes
 */

#define E_WRPRT         0       /* Write Protect                */
#define E_UNIT          1       /* Unknown Unit                 */
#define E_NOTRDY        2       /* Device Not Ready             */
#define E_CMD           3       /* Unknown Command              */
#define E_CRC           4       /* Crc Error                    */
#define E_LENGTH        5       /* Bad Length                   */
#define E_SEEK          6       /* Seek Error                   */
#define E_MEDIA         7       /* Unknown MEDIA                */
#define E_NOTFND        8       /* Sector Not Found             */
#define E_PAPER         9       /* No Paper                     */
#define E_WRITE         10      /* Write Fault                  */
#define E_READ          11      /* Read Fault                   */
#define E_FAILURE       12      /* General Failure              */

/*
 *      Command codes
 */

#define C_INIT          0x00    /* Initialize                   */
#define C_MEDIACHK      0x01    /* MEDIA Check                  */
#define C_BLDBPB        0x02    /* Build BPB                    */
#define C_IOCTLIN       0x03    /* Ioctl In                     */
#define C_INPUT         0x04    /* Input (Read)                 */
#define C_NDREAD        0x05    /* Non-destructive Read         */
#define C_ISTAT         0x06    /* Input Status                 */
#define C_IFLUSH        0x07    /* Input Flush                  */
#define C_OUTPUT        0x08    /* Output (Write)               */
#define C_OUTVFY        0x09    /* Output with verify           */
#define C_OSTAT         0x0a    /* Output                       */
#define C_OFLUSH        0x0b    /* Output Flush                 */
#define C_IOCTLOUT      0x0c    /* Ioctl Out                    */
#define C_OPEN          0x0d    /* Device Open                  */
#define C_CLOSE         0x0e    /* Device Close                 */
#define C_REMMEDIA      0x0f    /* Removable MEDIA              */
#define C_OUB           0x10    /* Output till busy             */
#define C_GENIOCTL      0x13    /* Generic Ioctl                */
#define C_GETLDEV       0x17    /* Get Logical Device           */
#define C_SETLDEV       0x18    /* Set Logical Device           */
#define C_IOCTLQRY      0x19    /* Ioctl Query                  */

/*
 *      Convienence macros
 */
#define failure(x)      (S_ERROR+S_DONE+x)
#ifndef TRUE
#define TRUE            1
#endif
#ifndef FALSE
#define FALSE           0
#endif
#define mk_offset(far_ptr)      ((UWORD)(far_ptr))
#define mk_segment(far_ptr) ((UWORD)((ULONG)(far_ptr) >> 16))
#define far_ptr(seg, off) ((VOID FAR *)(((ULONG)(off))+((ULONG)(seg) << 16)))

/*
 *      structures
 */

/* Device header */

struct dhdr
{
  struct dhdr
  FAR *dh_next;
  UWORD dh_attr;
    VOID(*dh_strategy) ();
    VOID(*dh_interrupt) ();
  BYTE dh_name[8];
};

#define ATTR_CHAR       0x8000
#define ATTR_IOCTL      0x4000
#define ATTR_BLDFAT     0x2000
#define ATTR_REMOTE     0x1000
#define ATTR_EXCALLS    0x0800
#define ATTR_QRYIOCTL   0x0080
#define ATTR_GENIOCTL   0x0040
#define ATTR_RAW        0x0400
#define ATTR_FASTCON    0x0010
#define ATTR_CLOCK      0x0008
#define ATTR_NULL       0x0004
#define ATTR_CONOUT     0x0002
#define ATTR_HUGE       0x0002
#define ATTR_CONIN      0x0001

/*                                                                      */
/* Bios Parameter Block structure                                       */
/*                                                                      */
/* The following offsets are computed as byte offsets and are based on  */
/* the struct below. The struct itself cannot be used because on some   */
/* compilers, structure alignement may be forced, throwing following    */
/* fields off (e.g. - BYTE, followed by a WORD may have a byte of fill  */
/* inserted in between; the WORD would then be at offset 2, not 1).     */
/*                                                                      */
#define BPB_NBYTE       0
#define BPB_NSECTOR     2
#define BPB_NRESERVED   3
#define BPB_NFAT        5
#define BPB_NDIRENT     6
#define BPB_NSIZE       8
#define BPB_MDESC       10
#define BPB_NFSECT      11
#define BPB_NSECS       13
#define BPB_NHEADS      15
#define BPB_HIDDEN      17
#define BPB_HUGE        21
#define BPB_SIZEOF      25

typedef struct
{
  UWORD bpb_nbyte;              /* Bytes per Sector             */
  UBYTE bpb_nsector;            /* Sectors per Allocation Unit  */
  UWORD bpb_nreserved;          /* # Reserved Sectors           */
  UBYTE bpb_nfat;               /* # FAT's                      */
  UWORD bpb_ndirent;            /* # Root Directory entries     */
  UWORD bpb_nsize;              /* Size in sectors              */
  UBYTE bpb_mdesc;              /* MEDIA Descriptor Byte        */
  UWORD bpb_nfsect;             /* FAT size in sectors          */
  UWORD bpb_nsecs;              /* Sectors per track            */
  UWORD bpb_nheads;             /* Number of heads              */
  ULONG bpb_hidden;             /* Hidden sectors               */
  ULONG bpb_huge;               /* Size in sectors if           */
  /* bpb_nsize== 0                                */
}
bpb;

struct gblkio
{
    UBYTE   gbio_spcfunbit;
    UBYTE   gbio_devtype;
    UWORD   gbio_devattrib;
    UWORD   gbio_ncyl;
    UBYTE   gbio_media;
    bpb     gbio_bpb;
    UWORD   gbio_nsecs;
};

struct Gioc_media
{
  WORD  ioc_level;
  ULONG ioc_serialno;
  BYTE  ioc_volume[11];
  BYTE  ioc_fstype[8];
};

/*                                                                      */
/* Boot Block (Super Block)                                             */
/*                                                                      */
/* See BPB comments for the offsets below                               */
/*                                                                      */
#define BT_JUMP         0
#define BT_OEM          3
#define BT_BPB          11
#define BT_SIZEOF       36

typedef struct
{
  BYTE  bt_jump[3];             /* Boot Jump opcodes            */
  BYTE  bt_oem[8];              /* OEM Name                     */
  bpb   bt_bpb;                 /* BPB for this media/device    */
  WORD  bt_nsecs;               /* # Sectors per Track          */
  WORD  bt_nheads;              /* # Heads                      */
  WORD  bt_hidden;              /* # Hidden sectors             */
  LONG  bt_huge;                /* use if nsecs == 0            */
  BYTE  bt_drvno;
  BYTE  bt_reserv;
  BYTE  bt_btid;
  ULONG bt_serialno;
  BYTE  bt_volume[11];
  BYTE  bt_fstype[8];
}
boot;

typedef boot super;             /* Alias for boot structure             */

typedef struct
{
  BYTE r_length;                /*  Request Header length               */
  BYTE r_unit;                  /*  Unit Code                           */
  BYTE r_command;               /*  Command Code                        */
  WORD r_status;                /*  Status                              */
  BYTE r_reserved[8];           /*  DOS Reserved Area                   */
  union
  {
    struct
    {
      BYTE _r_nunits;           /*  number of units     */
      BYTE FAR *_r_endaddr;     /*  Ending Address      */
      bpb *FAR * _r_bpbptr;     /*  ptr to BPB array    */
      BYTE _r_firstunit;
    }
    _r_init;
    struct
    {
      BYTE _r_meddesc;          /*  MEDIA Descriptor    */
      BYTE _r_retcode;          /*  Return Code         */
      BYTE FAR
      * _r_vid;                 /* volume id */
    }
    _r_media;
    struct
    {
      BYTE _r_meddesc;          /*  MEDIA Descriptor    */
      boot FAR
      * _r_fat;                 /*  boot sector pointer */
      bpb FAR
      * _r_bpbpt;               /*  ptr to BPB table    */
    }
    _r_bpb;
    struct
    {
      BYTE _r_meddesc;          /*  MEDIA Descriptor    */
      BYTE FAR
      * _r_trans;               /*  Transfer Address    */
      UWORD _r_count;           /*  Byte/Sector Count   */
      UWORD _r_start;           /*  Starting Sector No. */
      BYTE FAR
      * _r_vid;                 /* Pointer to volume id */
      LONG _r_huge;             /* for > 32Mb drives    */
    }
    _r_rw;
    struct
    {
      BYTE _r_ndbyte;           /*  Byte Read From Device       */
    }
    _r_nd;
  }
  _r_x;
}
request;

#define HUGECOUNT       0xffff
#define MAXSHORT        0xffffl

/*
 * Macros to assist request structure legibility
 */

/* Init packet macros                                                   */
#define r_nunits        _r_x._r_init._r_nunits
#define r_endaddr       _r_x._r_init._r_endaddr
#define r_bpbptr        _r_x._r_init._r_bpbptr
#define r_firstunit     _r_x._r_init._r_firstunit

/* MEDIA Check packet macros                                            */
#define r_mcmdesc       _r_x._r_media._r_meddesc
#define r_mcretcode     _r_x._r_media._r_retcode
#define r_mcvid         _r_x._r_media._r_vid

/* Build BPB packet macros                                              */
#define r_bpmdesc       _r_x._r_bpb._r_meddesc
#define r_bpfat         _r_x._r_bpb._r_fat
#define r_bpptr         _r_x._r_bpb._r_bpbpt

/* rw packet macros                                                     */
#define r_meddesc       _r_x._r_rw._r_meddesc
#define r_trans         _r_x._r_rw._r_trans
#define r_count         _r_x._r_rw._r_count
#define r_start         _r_x._r_rw._r_start
#define r_rwvid         _r_x._r_rw._r_vid
#define r_huge          _r_x._r_rw._r_huge

/* ndread packet macros                                                 */
#define r_ndbyte        _r_x._r_nd._r_ndbyte

/*
 *interrupt support (spl & splx) support - IBM style
 */

#define I_NONE          0       /* Initial value                */

/* predefined interrupt levels - 8259 support                           */
#define IRQ0            0x01    /* Level 0 - highest            */
#define IRQ1            0x02
#define IRQ2            0x04
#define IRQ3            0x08
#define IRQ4            0x10
#define IRQ5            0x20
#define IRQ6            0x40
#define IRQ7            0x80    /* Level 7 - lowest             */

/* standard hardware configuration                                      */
#define I_RTC           IRQ0    /* Timer                        */
#define I_KBD           IRQ1    /* Keyboard                     */
#define I_COM2          IRQ3    /* COM1:                        */
#define I_COM1          IRQ4    /* COM2:                        */
#define I_HDC           IRQ5    /* Fixed disk                   */
#define I_FDC           IRQ6    /* Diskette                     */
#define I_PRT           IRQ7    /* Printer                      */

/* standard hardware vectors - 8259 defined                             */
#define V_RTC           0x08    /* Timer                        */
#define V_KBD           0x09    /* Keyboard                     */
#define V_LEV2          0x0a    /* Level 2 - uncomitted         */
#define V_COM2          0x0b    /* COM1:                        */
#define V_COM1          0x0c    /* COM2:                        */
#define V_HDC           0x0d    /* Fixed disk                   */
#define V_FDC           0x0e    /* Diskette                     */
#define V_PRT           0x0f    /* Printer                      */

#define V_LEV0          0x08    /* Level 0 - highest            */
#define V_LEV1          0x09
#define V_LEV2          0x0a    /* Level 2 - uncomitted         */
#define V_LEV3          0x0b
#define V_LEV4          0x0c
#define V_LEV5          0x0d
#define V_LEV6          0x0e
#define V_LEV7          0x0f    /* Level 7 - lowest             */

/*
 */
typedef request FAR *rqptr;
typedef bpb FAR *bpbptr;
typedef BYTE FAR *byteptr;
typedef struct dhdr FAR *dhdrptr;

/*
 *      externals
 */

extern BYTE FAR *device_end();

/*
 *      end of device.h
 */

