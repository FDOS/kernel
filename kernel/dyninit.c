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

 FCB's, f_nodes, buffers,...
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


extern struct DynS FAR Dyn;

void *DynAlloc(char FAR *what, unsigned num, unsigned size)
{
    void *now;
    unsigned total = num * size;
    
    UNREFERENCED_PARAMETER(what);
    
    DebugPrintf(("DYNDATA:allocating %Fs - %u * %u bytes, total %u, %u..%u\n",
        what, num, size, total, Dyn.Allocated,Dyn.Allocated+total));
    
    if (total > Dyn.AllocMax - Dyn.Allocated)
        {
        printf("DYNDATA overflow");
        for (;;);
        }
    now = (void*)&Dyn.Buffer[Dyn.Allocated];
    
    Dyn.Allocated += total;
    
    
    return now;   
}    

void DynFree(unsigned memory_needed)
{
    if (memory_needed == 0)     /* this is pass  0 */
        {

        Dyn.Allocated = 1000;    /* this reserves space for initDisk */
        Dyn.AllocMax  = sizeof(Dyn.Buffer);
        }                        
    else {
                                /* enlarge kernel data segment to 64K */
        if (memory_needed + Dyn.UsedByDiskInit > sizeof(Dyn.Buffer))
            {     
            if ((ULONG)memory_needed + Dyn.UsedByDiskInit > 0xffff)
                {
                printf("PANIC:Dyn %lu\n",memory_needed + Dyn.UsedByDiskInit);
                for (;;);    
                }                    
                
            MoveKernel(FP_SEG(&Dyn.UsedByDiskInit) + 0x1000);

            Dyn.AllocMax  = 0xffff - (unsigned)&Dyn.Buffer;
            }
        Dyn.Allocated = Dyn.UsedByDiskInit;
        }

    DebugPrintf(("DYNDATA:free to %u, max %u\n",Dyn.Allocated,Dyn.AllocMax));
}    
    
void FAR *DynLast()
{
    DebugPrintf(("dynamic data end at %p\n",(void FAR *)(Dyn.Buffer+Dyn.Allocated)));

    return Dyn.Buffer+Dyn.Allocated;
}    
