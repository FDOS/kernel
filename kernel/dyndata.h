/*
    DynData.h
    
    declarations for dynamic NEAR data allocations
    
    the DynBuffer must initially be large enough to hold
    the PreConfig data.
    after the disksystem has been initialized, the kernel is 
    moveable and Dyn.Buffer resizable, but not before 
*/

void far *DynAlloc(char *what, unsigned num, unsigned size);
void far *DynLast(void);
void DynFree(void *ptr);

struct DynS {
  unsigned Allocated;
  char Buffer[1];
};
