/*
    KConfig.h  
    
    DLASortByDriveNo
        0 : Drive Letter Assignement ike MSDOS
        1 : DLA - first drive completely first, then to next drive    

    InitDiskShowDriveAssignment
        0   : don't show what drive/partition assigned to what drive letter
        1   : show info 
        
    SkipConfigSeconds:
        < 0 : not possible to skip config.sys
        = 0 : only possible if already pressed before, no message
        > 0 : wait so long for F5/F8  
        
    BootHarddiskSeconds:  boots by default - and without user interaction - from HD
        <= 0: normal
        >  0: 
        	  display message 
        	  ' hit any key to continue to boot from 'diskette or CD'
        	  wait ## seconds
        	  if no key hit, boot from HD

	Version_: only in kernel 2042 or higher, offline version identification
		OemID: 0xFD for FreeDOS kernel, 0xDC for DosC kernel
		Major: actual kernel version (not MS-DOS compatibility version), e.g. 2
		Revision: revision sequence, e.g. 42 for kernel 2042
		Release: 0 if released version, >0 for svn builds (e.g. svn revision #)
            
	CheckDebugger: during initialization enable/disable check to stop in debugger if one is active
	   0 = do not check (assume absent),
	   1 = do check by running breakpoint,
	   2 = assume present
	   
    Verbose: amount of messages to display during initialization
       -1 = quiet
        0 = normal
        1 = verbose

	PartitionMode: how to handle GPT and MBR partitions
		bits 0-1: 01=GPT if found, 00=MBR if found, 11=Hybrid, GPT first then MBR, 10=Hybrid, MBR first then GPT
		          in hybrid mode, EE partitions ignored, drives assigned by GPT or MBR first based on hybrid type
		bits 2-4: 001=mount ESP (usually FAT32) partition, 010=mount MS Basic partitions, 100=mount unknown partitions
		          111=attempt to mount all paritions, 110=attempt to mount all but ESP partitions
		bits 5-7: reserved, 0 else undefined behavior
*/
typedef struct _KernelConfig {
  char CONFIG[6];               /* "CONFIG" */
  unsigned short ConfigSize;

  unsigned char DLASortByDriveNo;
  unsigned char InitDiskShowDriveAssignment;
  signed char SkipConfigSeconds;
  unsigned char ForceLBA;
  unsigned char GlobalEnableLBAsupport; /* = 0 --> disable LBA support */
  signed char BootHarddiskSeconds;

  /* for version 2042 and higher only */
  unsigned char Version_OemID;
  unsigned char Version_Major;
  unsigned short Version_Revision;
  unsigned short Version_Release;

  /* for version 2044 and higher only */
  unsigned char CheckDebugger;
  signed char Verbose;
  unsigned char PartitionMode;
} KernelConfig;
