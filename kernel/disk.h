/*
    DISK.H
    
    common constants + structures, shared between
    
    DSK.C and INITDISK.C
    
    note:
    
*/


#define MAX_HARD_DRIVE  8
#define N_RETRY         5       /* number of retries permitted  */
#define NDEV            20      /* only one for demo            */
#define SEC_SIZE        512     /* size of sector in bytes      */


#define LBA_READ         0x4200
#define LBA_WRITE        0x4300


/* physical characteristics of a drive */

struct CHS {
    ULONG Cylinder;
    UWORD Head;
    UWORD Sector;
    };

struct DriveParamS
{
    UBYTE driveno;              /* = 0x8x                           */
    unsigned LBA_supported:1;        /* set, if INT13 extensions enabled */
    unsigned WriteVerifySupported:1; /* */
    ULONG total_sectors;

    struct CHS chs;             /* for normal   INT 13 */
};

struct media_info
{
  struct DriveParamS drive;     /* physical charactereistics of drive */
  ULONG mi_size;                /* physical sector count        */
  ULONG mi_offset;              /* relative partition offset    */
  ULONG mi_FileOC;              /* Count of Open files on Drv   */
  

  struct FS_info
    {
    ULONG serialno;
    BYTE  volume[11];
    BYTE  fstype[8];
    }fs;

};
    
struct _bios_LBA_address_packet        /* Used to access a hard disk via LBA */
                                       /*       Added by Brian E. Reifsnyder */
{
  unsigned char    packet_size;        /* size of this packet...set to 16  */
  unsigned char    reserved_1;         /* set to 0...unused                */
  unsigned char    number_of_blocks;   /* 0 < number_of_blocks < 128       */
  unsigned char    reserved_2;         /* set to 0...unused                */
  UBYTE    far *   buffer_address;     /* addr of transfer buffer          */
  unsigned long    block_address;      /* LBA address                      */
  unsigned long    block_address_high; /* high bytes of LBA addr...unused  */
};
