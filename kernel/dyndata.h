/*
    DynData.h
    
    declarations for dynamic NEAR data allocations
    
    the DynBuffer must initially be large enough to hold
    the PreConfig data.
    after the disksystem has been initialized, the kernel is 
    moveable and Dyn.Buffer resizable, but not before 
*/


void *DynAlloc(char far *what, unsigned num, unsigned size);
void DynFree(unsigned memory_needed);
void far *DynLast(void);

struct DynS {
     unsigned Allocated;
     unsigned UsedByDiskInit;
     unsigned AllocMax;
     char Buffer[1000           /* for InitDisk - Miarray's */
                 + 16 * 71      /* initial f_nodes          */
                 +200           /* give some extra bytes    */
                 ];
     };
