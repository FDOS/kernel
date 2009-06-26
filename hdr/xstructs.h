/****************************************************************/
/*                                                              */
/*                        xstructs.h                            */
/*                                                              */
/*                Extended DOS 7.0+ structures                  */
/*                                                              */
/****************************************************************/

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *XStructs_hRcsId =
    "$Id$";
#endif
#endif

struct xdpbdata {
  UWORD xdd_dpbsize;
  struct dpb xdd_dpb;
};

struct xfreespace {
  UWORD xfs_datasize;           /* size of this structure                */
  union {
    UWORD requested;            /* requested structure version           */
    UWORD actual;               /* actual structure version              */
  } xfs_version;
  ULONG xfs_clussize;           /* number of sectors per cluster         */
  ULONG xfs_secsize;            /* number of bytes per sector            */
  ULONG xfs_freeclusters;       /* number of available clusters          */
  ULONG xfs_totalclusters;      /* total number of clusters on the drive */
  ULONG xfs_freesectors;        /* number of physical sectors available  */
  ULONG xfs_totalsectors;       /* total number of physical sectors      */
  ULONG xfs_freeunits;          /* number of available allocation units  */
  ULONG xfs_totalunits;         /* total allocation units                */
  UBYTE xfs_reserved[8];
};

struct xdpbforformat {
  UWORD xdff_datasize;          /* size of this structure                */
  union {
    UWORD requested;            /* requested structure version           */
    UWORD actual;               /* actual structure version              */
  } xdff_version;
  UDWORD xdff_function;         /* function number:
                                   00h invalidate DPB counts
                                   01h rebuild DPB from BPB
                                   02h force media change
                                   03h get/set active FAT number and mirroring
                                   04h get/set root directory cluster number
                                 */
  union {
    struct {
      DWORD nfreeclst;          /* # free clusters
                                   (-1 - unknown, 0 - don't change) */
      DWORD cluster;            /* cluster # of first free          
                                   (-1 - unknown, 0 - don't change) */
      UDWORD reserved[2];
    } setdpbcounts;

    struct {
      UDWORD unknown;
      bpb FAR *bpbp;
      UDWORD reserved[2];
    } rebuilddpb;

    struct {
      DWORD new;                /* new active FAT/mirroring state, or -1 to get
                                   bits 3-0: the 0-based FAT number of the active FAT
                                   bits 6-4: reserved (0)
                                   bit 7: do not mirror active FAT to inactive FATs
                                   or:
                                   set new root directory cluster, -1 - get current
                                 */
      DWORD old;                 /* previous active FAT/mirroring state (as above)
                                    or
                                    get previous root directory cluster
                                 */
      UDWORD reserved[2];
    } setget;
  } xdff_f;
};

COUNT DosGetExtFree(BYTE FAR * DriveString, struct xfreespace FAR * xfsp);
