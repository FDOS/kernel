#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

void die(char *msg)
{
  printf("%s\n", msg);
  exit(1);
}

/* Both traditional DOS and FAT32 DOS return this structure, but
   FAT32 return a lot more data, so make sure we have plenty of space */
struct deviceparams {
    uint8_t specfunc;
    uint8_t devtype;
    uint16_t devattr;
    uint16_t cylinders;
    uint8_t mediatype;
    uint16_t bytespersec;
    uint8_t secperclust;
    uint16_t ressectors;
    uint8_t fats;
    uint16_t rootdirents;
    uint16_t sectors;
    uint8_t media;
    uint16_t fatsecs;
    uint16_t secpertrack;
    uint16_t heads;
    uint32_t hiddensecs;
    uint32_t hugesectors;
    uint8_t lotsofpadding[224];
};

void get_partition_offset(int drive)
{
    static struct deviceparams dp;
    int err=0;
    
    printf("Getting drive info: int21h function 440Dh subfunction 0860h\n");
    
    dp.specfunc = 1;		/* Get current information */

    __asm {
      mov ax, 0x440d
      mov bx, drive
      mov cx, 0x0860
      mov dx, offset dp
      int 0x21
      mov ax, 0
      adc ax, 0
      mov err, ax
    }

    if (!err)
    {
      printf("Partition Offset = %02lXh  [c=%02u,h=%02u,s=%2u]\n",  dp.hiddensecs, dp.cylinders, dp.heads, dp.secpertrack);
      printf("FAT32 version 4860h to compare\n"); 
    }
    else
      printf("Original version failed, checking FAT32 version using 4860h\n");
     
    __asm {
      mov ax, 0x440d
      mov bx, drive
      mov cx, 0x4860
      mov dx, offset dp
      int 0x21
      mov ax, 0
      adc ax, 0
      mov err, ax
    }

    if (!err)
    {
      printf("Partition Offset = %02lXh\n",  dp.hiddensecs);
      return;
    }
    else
      printf("FAT32 version failed.\n");
}

struct rwblock {
    uint8_t special;
    uint16_t head;
    uint16_t cylinder;
    uint16_t firstsector;
    uint16_t sectors;
    uint16_t bufferoffset;
    uint16_t bufferseg;
};

static struct rwblock mbr = {
    .special = 0,
    .head = 0,
    .cylinder = 0, /* 5265, 3fh */
    .firstsector = 0,		/* MS-DOS, unlike the BIOS, zero-base sectors */
    .sectors = 1,
    .bufferoffset = 0,
    .bufferseg = 0
};


void read_mbr(int drive, const void *buf)
{
    uint8_t err=0;
    uint16_t ds_seg=0;

    //printf("read_mbr(%c:,%p)", 'a'+drive-1, buf);

    mbr.bufferoffset = (unsigned short)buf;
    __asm {
      push ds
      pop ax
      mov ds_seg, ax
    }
    mbr.bufferseg = ds_seg;
    //printf("DS = %04Xh\n", ds_seg);

    __asm {
      mov ax, 0x440d
      mov bx, drive
      mov cx, 0x0861
      mov dx, offset mbr
      int 0x21
      mov ax, 0
      adc ax, 0
      mov err, al
    }

    if (!err) {
      //printf("Success reading MBR\n\n");
      return;
    }
    
    __asm {
      mov ax, 0x440d
      mov bx, drive
      mov cx, 0x4861
      mov dx, offset mbr
      int 0x21
      mov ax, 0
      adc ax, 0
      mov err, al
    }

    if (!err) {
      //printf("Success reading FAT32 MBR\n\n");
      return;
    }
    
    printf("mbr read error");
}

int main(int argc, char *argv[]) 
{
    unsigned int c,h,s;
    static uint8_t buf[1024];
    unsigned drive = (unsigned)(argv[1][0]);  /* a=1,b=2,... */
    drive = drive-'a'+1;
    printf("Query drive %c:\n", 'a'+drive-1);
    
    get_partition_offset(drive);
    
    read_mbr(drive, buf);
    printf("MBR: %02Xh %02Xh %02Xh %02Xh %02Xh %02Xh %02Xh %02Xh\n",
	    ((const uint8_t *)buf)[0],
	    ((const uint8_t *)buf)[1],
	    ((const uint8_t *)buf)[2],
	    ((const uint8_t *)buf)[3],
	    ((const uint8_t *)buf)[4],
	    ((const uint8_t *)buf)[5],
	    ((const uint8_t *)buf)[6],
	    ((const uint8_t *)buf)[7]);

    printf("Finding drive\n");
    for (c = 1024; c <= 5265; c++) {
    for (h = 0; h < 255; h++)
    for (s = 0; s <= 63; s++)
    {
      mbr.cylinder = c;
      mbr.head =h;
      mbr.sectors=s;
      read_mbr(drive, buf);
      if (buf[0] != 0) {
          printf("Found at %u,%u,%u\n", c,h,s); 

          printf("MBR: %02Xh %02Xh %02Xh %02Xh %02Xh %02Xh %02Xh %02Xh\n",
	    ((const uint8_t *)buf)[0],
	    ((const uint8_t *)buf)[1],
	    ((const uint8_t *)buf)[2],
	    ((const uint8_t *)buf)[3],
	    ((const uint8_t *)buf)[4],
	    ((const uint8_t *)buf)[5],
	    ((const uint8_t *)buf)[6],
	    ((const uint8_t *)buf)[7]);
          
          exit(0);
      }
    }
    printf("c=%u,", c); fflush(NULL);
    }
      
    return 0;
}
