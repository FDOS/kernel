/****************************************************************/
/*                                                              */
/*                          initHMA.c                           */
/*                            DOS-C                             */
/*                                                              */
/*          move kernel to HMA area                             */
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

/*
    current status:
    
    load FreeDOS high, if DOS=HIGH detected
    
    suppress High Loading, if any SHIFT status detected (for debugging)
    
    if no XMS driver (HIMEM,FDXMS,...) loaded, should work
    
    cooperation with XMS drivers as follows:
    
    copy HMA_TEXT segment up.

    after each loaded DEVICE=SOMETHING.SYS, try to request the HMA
    (XMS function 0x01). 
    if no XMS driver detected, during ONFIG.SYS processing,
    create a dummy VDISK entry in high memory
    
    this works with
    
     FD FDXMS - no problems detected
    
    
     MS HIMEM.SYS (from DOS 6.2, 9-30-93)
     
        works if and only if
        
            /TESTMEM:OFF 
            
        is given
            
        otherwise HIMEM will TEST AND ZERO THE HIGH MEMORY+HMA.
        so, in CONFIG.C, if "HIMEM.SYS" is detected, a "/TESTMEM:OFF"
        parameter is forced.
*/

#include "portab.h"
#include "init-mod.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id: inithma.c 956 2004-05-24 17:07:04Z bartoldeman $";
#endif

BYTE DosLoadedInHMA BSS_INIT(FALSE);  /* set to TRUE if loaded HIGH          */
BYTE HMAclaimed BSS_INIT(0);          /* set to TRUE if claimed from HIMEM   */
UWORD HMAFree BSS_INIT(0);            /* first byte in HMA not yet used      */

STATIC void InstallVDISK(void);

#ifdef DEBUG
#if defined(__TURBOC__) || defined(__GNUC__)
#define int3() __int__(3);
#else
void int3()
{
  __asm int 3;
}
#endif
#else
#define int3()
#endif

#ifdef DEBUG
#define HMAInitPrintf(x) printf x
#else
#define HMAInitPrintf(x)
#endif

#ifdef DEBUG
VOID hdump(BYTE FAR * p)
{
  int loop;
  HMAInitPrintf(("%p", p));

  for (loop = 0; loop < 16; loop++)
    HMAInitPrintf(("%02x ", p[loop]));

  printf("\n");
}
#else
#define hdump(ptr)
#endif

#define KeyboardShiftState() (*(BYTE FAR *)(MK_FP(0x40,0x17)))

/*
    this tests, if the HMA area can be enabled.
    if so, it simply leaves it on
*/

STATIC int EnabledA20(void)
{
  return fmemcmp(MK_FP(0, 0), MK_FP(0xffff, 0x0010), 128);
}

int EnableHMA(VOID)
{

  _EnableA20();

  if (!EnabledA20())
  {
    printf("HMA can't be enabled\n");
    return FALSE;
  }

  _DisableA20();

#ifdef DEBUG
  if (EnabledA20())
  {
    printf("HMA can't be disabled - no problem for us\n");
  }
#endif

  _EnableA20();
  if (!EnabledA20())
  {
    printf("HMA can't be enabled second time\n");
    return FALSE;
  }

  HMAInitPrintf(("HMA success - leaving enabled\n"));

  return TRUE;

}

/*
    move the kernel up to high memory
    this is very unportable
    
    if we thin we succeeded, we return TRUE, else FALSE
*/

#define HMAOFFSET  0x20
#define HMASEGMENT 0xffff

int MoveKernelToHMA()
{
  void far *xms_addr;

  if (DosLoadedInHMA)
  {
    return TRUE;
  }

  if ((xms_addr = DetectXMSDriver()) == NULL)
    return FALSE;

  XMSDriverAddress = xms_addr;

#ifdef DEBUG
  /* A) for debugging purpose, suppress this, 
     if any shift key is pressed 
   */
  if (KeyboardShiftState() & 0x0f)
  {
    printf("Keyboard state is %0x, NOT moving to HMA\n",
           KeyboardShiftState());
    return FALSE;
  }
#endif

  /* B) check out, if we can have HMA */

  if (!EnableHMA())
  {
    printf("Can't enable HMA area (the famous A20), NOT moving to HMA\n");

    return FALSE;
  }

  /*  allocate HMA through XMS driver */

  if (HMAclaimed == 0 &&
      (HMAclaimed =
       init_call_XMScall(xms_addr, 0x0100, 0xffff)) == 0)
  {
    printf("Can't reserve HMA area ??\n");

    return FALSE;
  }

  MoveKernel(0xffff);

  {
    /* E) up to now, nothing really bad was done. 
       but now, we reuse the HMA area. bad things will happen

       to find bugs early,   
       cause INT 3 on all accesses to this area 
     */

    DosLoadedInHMA = TRUE;
  }

  /*
    on finalize, will install a VDISK
  */

  InstallVDISK();

  /* report the fact we are running high through int 21, ax=3306 */
  LoL->version_flags |= 0x10;

  return TRUE;

}

/*   
    now protect against HIMEM/FDXMS/... by simulating a VDISK 
    FDXMS should detect us and not give HMA access to ohers
    unfortunately this also disables HIMEM completely

    so: we install this after all drivers have been loaded
*/
STATIC void InstallVDISK(void)
{
  static struct {               /* Boot-Sektor of a RAM-Disk */
    UBYTE dummy1[3];            /* HIMEM.SYS uses 3, but FDXMS uses 2 */
    char Name[5];
    BYTE dummy2[3];
    WORD BpS;
    BYTE dummy3[6];
    WORD Sektoren;
    BYTE dummy4;
  } VDISK_BOOT_SEKTOR = {
    {
    0xcf, ' ', ' '},
    {
    'V', 'D', 'I', 'S', 'K'},
    {
    ' ', ' ', ' '}, 512,
    {
    'F', 'D', 'O', 'S', ' ', ' '}, 128, /* 128*512 = 64K */
  ' '};

  if (!DosLoadedInHMA)
    return;

  fmemcpy(MK_FP(0xffff, 0x0010), &VDISK_BOOT_SEKTOR,
          sizeof(VDISK_BOOT_SEKTOR));

  *(WORD FAR *) MK_FP(0xffff, 0x002e) = 1024 + 64;
}

/*
    this allocates some bytes from the HMA area
    only available if DOS=HIGH was successful
*/

VOID FAR * HMAalloc(COUNT bytesToAllocate)
{
  VOID FAR *HMAptr;

  if (!DosLoadedInHMA)
    return NULL;

  if (HMAFree > 0xfff0 - bytesToAllocate)
    return NULL;

  HMAptr = MK_FP(0xffff, HMAFree);

  /* align on 16 byte boundary */
  HMAFree = (HMAFree + bytesToAllocate + 0xf) & 0xfff0;

  /*printf("HMA allocated %d byte at %x\n", bytesToAllocate, HMAptr); */

  fmemset(HMAptr, 0, bytesToAllocate);

  return HMAptr;
}

unsigned CurrentKernelSegment = 0;

void MoveKernel(unsigned NewKernelSegment)
{
  UBYTE FAR *HMADest;
  UBYTE FAR *HMASource;
  unsigned len;
  unsigned jmpseg = CurrentKernelSegment;
 
  if (CurrentKernelSegment == 0)
    CurrentKernelSegment = FP_SEG(_HMATextEnd);

  if (CurrentKernelSegment == 0xffff)
    return;

  HMASource =
      MK_FP(CurrentKernelSegment, (FP_OFF(_HMATextStart) & 0xfff0));
  HMADest = MK_FP(NewKernelSegment, 0x0000);

  len = (FP_OFF(_HMATextEnd) | 0x000f) - (FP_OFF(_HMATextStart) & 0xfff0);

  if (NewKernelSegment == 0xffff)
  {
    HMASource += HMAOFFSET;
    HMADest += HMAOFFSET;
    len -= HMAOFFSET;
  }

  HMAInitPrintf(("HMA moving %p up to %p for %04x bytes\n",
                 HMASource, HMADest, len));

  if (NewKernelSegment < CurrentKernelSegment ||
      NewKernelSegment == 0xffff)
    fmemcpy(HMADest, HMASource, len);
  /* else it's the very first relocation: handled by kernel.asm */

  HMAFree = (FP_OFF(HMADest) + len + 0xf) & 0xfff0;
  /* first free byte after HMA_TEXT on 16 byte boundary */

  {
    /* D) but it only makes sense, if we can relocate 
       all our entries to make use of HMA
     */

    /* this is for a 
       call near enableA20
       jmp far kernelentry
       style table
     */

    struct RelocationTable FAR *rp;
    struct RelocationTable rtemp;

    /* verify, that all entries are valid */

    for (rp = _HMARelocationTableStart; rp < _HMARelocationTableEnd; rp++)
    {
      if (rp->jmpFar != 0xea || /* jmp FAR */
          rp->jmpSegment != jmpseg ||     /* will only relocate HMA_TEXT */
          rp->callNear != 0xe8 ||       /* call NEAR */
          0)
      {
        printf("illegal relocation entry # %d\n",
               (FP_OFF(rp) -
                FP_OFF(_HMARelocationTableStart)) /
               sizeof(struct RelocationTable));
        int3();
        goto errorReturn;
      }
    }

    /* OK, all valid, go to relocate */

    for (rp = _HMARelocationTableStart; rp < _HMARelocationTableEnd; rp++)
    {
      if (NewKernelSegment == 0xffff)
      {
        struct RelocatedEntry FAR *rel = (struct RelocatedEntry FAR *)rp;

        fmemcpy(&rtemp, rp, sizeof(rtemp));

        rel->jmpFar = rtemp.jmpFar;
        rel->jmpSegment = NewKernelSegment;
        rel->jmpOffset = rtemp.jmpOffset;
        rel->callNear = rtemp.callNear;
        rel->callOffset = rtemp.callOffset + 5; /* near calls are relative */
      }
      else
        rp->jmpSegment = NewKernelSegment;

    }

    if (NewKernelSegment == 0xffff)
    {
      /* jmp far cpm_entry (copy from 0:c0) */
      pokeb(0xffff, 0x30 * 4 + 0x10, 0xea);
      pokel(0xffff, 0x30 * 4 + 0x11, (ULONG)cpm_entry);
    }
  }

  CurrentKernelSegment = NewKernelSegment;
  return;

errorReturn:
  for (;;) ;
}

