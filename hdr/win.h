#ifndef __WINSUPPORT_H
#define __WINSUPPORT_H
#ifdef    WIN31SUPPORT  /* defined to enable kernel hooks for win3.x compatibility */


extern UWORD winInstanced; /* internal flag marking if Windows is active */

/* contains information about data that must be kept for each active DOS
   instance, ie data that can NOT be shared between multiple VMs.
 */
struct WinStartupInfo
{
  UWORD winver;  /* this structure version, matches Windows version */
  ULONG next;    /* far pointer to next WinStartupInfo structure or NULL */
  ULONG vddName; /* far pointer to ASCIIZ pathname of virtual device driver */
  ULONG vddInfo; /* far pointer to vdd reference data or NULL if vddName=NULL */
  ULONG instanceTable; /* far pointer to array of instance data */
};
extern struct WinStartupInfo winStartupInfo;

/* contains a list of offsets relative to DOS data segment of
   various internal variables.
 */
struct WinPatchTable
{
  UWORD dosver;
  UWORD OffTempDS;
  UWORD OffTempBX;
  UWORD OffInDOS;
  UWORD OffMachineID;
  UWORD OffCritSectPatches;
  UWORD OffLastMCBSeg;  /* used by Win 3.1 if DOS version 5 or higher */
};
extern struct WinPatchTable winPatchTable;


#endif /* WIN31SUPPORT   */
#endif /* __WINSUPPORT_H */
