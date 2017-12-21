/*
    DYNINIT.C
    
    this serves requests from the INIT modules to
    allocate dynamic data.

kernel layout:
 00000H 000FFH 00100H PSP                PSP
 00100H 004E1H 003E2H _TEXT              CODE
 004E2H 007A7H 002C6H _IO_TEXT           CODE
 007A8H 008E5H 0013EH _IO_FIXED_DATA     CODE
 008F0H 0139FH 00AB0H _FIXED_DATA        DATA
 013A0H 019F3H 00654H _DATA              DATA
 019F4H 0240DH 00A1AH _BSS               BSS

additionally: 
                      DYN_DATA           DYN 

 
 02610H 0F40EH 0CDFFH HMA_TEXT           HMA

 FCBs, f_nodes, buffers,...
 drivers 
 
 
 0F410H 122DFH 02ED0H INIT_TEXT          INIT
 122E0H 12AA5H 007C6H ID                 ID
 12AA6H 12CBFH 0021AH IB                 IB

 purpose is to move the HMA_TEXT = resident kernel
 around, so that below it - after BSS, there is data 
 addressable near by the kernel, to hold some arrays
 like f_nodes    
 
 making f_nodes near saves ~2.150 code in HMA
    
*/
#include "portab.h"
#include "init-mod.h"
#include "dyndata.h"

#if defined(DEBUG)
#define DebugPrintf(x) printf x
#else
#define DebugPrintf(x)
#endif

/*extern struct DynS FAR Dyn;*/

#ifndef __TURBOC__
extern struct DynS DOSFAR ASM Dyn;
#else
extern struct DynS FAR ASM Dyn;
#endif

void far *DynAlloc(char *what, unsigned num, unsigned size)
{
  void far *now;
  unsigned total = num * size;
  struct DynS far *Dynp = MK_FP(FP_SEG(LoL), FP_OFF(&Dyn));

#ifndef DEBUG
  UNREFERENCED_PARAMETER(what);
#endif

  if ((ULONG) total + Dynp->Allocated > 0xffff)
  {
    printf("PANIC:Dyn %lu\n", (ULONG) total + Dynp->Allocated);
    for (;;) ;
  }

  DebugPrintf(("DYNDATA:allocating %s - %u * %u bytes, total %u, %u..%u\n",
               what, num, size, total, Dynp->Allocated,
               Dynp->Allocated + total));

  now = (void far *)&Dynp->Buffer[Dynp->Allocated];
  fmemset(now, 0, total);

  Dynp->Allocated += total;

  return now;
}

void DynFree(void *ptr)
{
  struct DynS far *Dynp = MK_FP(FP_SEG(LoL), FP_OFF(&Dyn));
  Dynp->Allocated = (char *)ptr - (char *)Dynp->Buffer;
}

void FAR * DynLast()
{
  struct DynS far *Dynp = MK_FP(FP_SEG(LoL), FP_OFF(&Dyn));
  DebugPrintf(("dynamic data end at %p\n",
               (void FAR *)(Dynp->Buffer + Dynp->Allocated)));

  return Dynp->Buffer + Dynp->Allocated;
}
