/****************************************************************/
/*                                                              */
/*                            cds.h                             */
/*                                                              */
/*                  Current Directory structures                */
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
static BYTE *Cds_hRcsId =
    "$Id$";
#endif
#endif

#define MAX_CDSPATH 67

struct cds {
  BYTE cdsCurrentPath[MAX_CDSPATH];
  UWORD cdsFlags;           /* see below */
  struct dpb FAR *cdsDpb;   /* if != 0, associated DPB */

  union {
    BYTE FAR *_cdsRedirRec; /* IFS record */
    struct {
      UWORD _cdsStrtClst;   /* if local path (Flags & CDSPHYSDRV): 
                               start cluster of CWD; root == 0,
                               never access == 0xFFFF */
      UWORD _cdsParam;
    } _cdsRedir;
  } _cdsUnion;

  UWORD cdsStoreUData;

#define cdsJoinOffset cdsBackslashOffset
  WORD cdsBackslashOffset; /* Position of "root directory" backslash for
                               this drive within CurrentPath[]
                               prerequisites:
                                   + ofs <= strlen(currentPath)
                                   + if UNC: ofs > share component
                                     if local path: ofs > colon
                           */

  BYTE cdsNetFlag1;
  BYTE FAR *cdsIfs;
  UWORD cdsNetFlags2;

};

#define cdsStrtClst _cdsUnion._cdsRedir._cdsStrtClst
#define cdsRedirRec _cdsUnion._cdsRedirRec
#define cdsParam _cdsUnion._cdsRedir._cdsParam

/* Bits for cdsFlags (OR combination)                           */
#define CDSNETWDRV      0x8000
#define CDSPHYSDRV      0x4000
#define CDSJOINED       0x2000 /* not in combination with NETWDRV or SUBST */
#define CDSSUBST        0x1000 /* not in combination with NETWDRV or JOINED */
#define CDS_HIDDEN     (1 << 7)    /* hide drive from redirector's list */

/* NETWORK PHYSICAL        meaning
   0       0               drive not accessable
   0       1               local file system
   1       0               networked file system (UNC naming convention)
   1       1               installable file system (IFS)
*/
#define CDSMODEMASK        (CDSNETWDRV | CDSPHYSDRV)
 
/* #define CDSVALID        (CDSNETWDRV | CDSPHYSDRV) */
#define CDSVALID CDSMODEMASK

#define IS_DEVICE 0x20
#define IS_NETWORK 0x40

#define CDS_MODE_SKIP_PHYSICAL 0x01    /* don't resolve SUBST, JOIN, NETW */
#define CDS_MODE_CHECK_DEV_PATH 0x02  /* check for existence of device path */
#define CDS_MODE_ALLOW_WILDCARDS 0x04  /* allow wildcards in "truename" */
