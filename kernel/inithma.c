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


extern BYTE FAR   version_flags;              /* minor version number                 */

extern BYTE
    FAR _HMATextAvailable,        /* first byte of available CODE area    */
    FAR _HMATextStart[],          /* first byte of HMAable CODE area      */
    FAR _HMATextEnd[]; /* and the last byte of it              */

#ifdef VERSION_STRINGS
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.7  2001/07/09 22:19:33  bartoldeman
 * LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
 *
 * Revision 1.6  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.5  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.4  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.3  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.2  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.1  2001/03/21 03:01:45  bartoldeman
 * New file by Tom Ehlert for HMA initialization.
 *
 * Revision 0.1 2001/03/16 12:00:00  tom ehlert
 * initial creation
 */
 
 
BYTE DosLoadedInHMA=FALSE;    /* set to TRUE if loaded HIGH          */
BYTE HMAclaimed=FALSE;        /* set to TRUE if claimed from HIMEM   */
WORD HMAFree;                 /* first byte in HMA not yet used      */
 

extern BYTE FAR * FAR XMSDriverAddress;
extern FAR _EnableA20(VOID);
extern FAR _DisableA20(VOID);


extern void FAR *DetectXMSDriver(VOID);
 
#ifdef DEBUG  
    #define int3() __int__(3);
#else
    #define int3()
#endif        


#ifdef DEBUG
    #define HMAInitPrintf(x) printf x
#else
    #define HMAInitPrintf(x)
#endif    

void MoveKernel(unsigned NewKernelSegment);


#ifdef DEBUG
VOID hdump(BYTE FAR *p)
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
 

#ifdef __TURBOC__
    void __int__(int);              /* TC 2.01 requires this. :( -- ror4 */
    unsigned char __inportb__(int portid);
    void	   __outportb__	(int portid, unsigned char value);
#endif



#define KeyboardShiftState() (*(BYTE FAR *)(MK_FP(0x40,0x17)))


/* of course, this should go to ASMSUPT */
fmemcmp(BYTE far *s1, BYTE FAR *s2, unsigned len)
{
    for ( ; len ; s1++,s2++,--len)
    {
        if (*s1 - *s2)
            return *s1-*s2;  
    }
    return 0;
}


/* Enable / Disable borrowed without understanding from FDXMS. Thanks. 
    gone done to KERNEL.ASM

OutportWithDelay(WORD port, BYTE val)
{
    int loop;
    
    __outportb__(port,val);
    
    for (loop = 100; --loop && __inportb__(0x64) & 0x02;)
        ; 
}    


void _EnableHMA()
{
    OutportWithDelay(0x64, 0xd1);
    OutportWithDelay(0x60, 0xdf);
    OutportWithDelay(0x64, 0xff);
}    
void _DisableHMA()
{
    OutportWithDelay(0x64, 0xd1);
    OutportWithDelay(0x60, 0xdd);
    OutportWithDelay(0x64, 0xff);
}    
*/

/*
    this tests, if the HMA area can be enabled.
    if so, it simply leaves it on
*/

int EnableHMA(VOID)
{
    
    _EnableA20();

    if (fmemcmp(MK_FP(0x0000,0x0000), MK_FP(0xffff,0x0010),128) == 0)
        {
            printf("HMA can't be enabled\n");
            return FALSE;
        }
    

    _DisableA20();
    
    if (fmemcmp(MK_FP(0x0000,0x0000), MK_FP(0xffff,0x0010),128) != 0)
        {
        printf("HMA can't be disabled - no problem for us\n");
        }
        
    _EnableA20();
    if (fmemcmp(MK_FP(0x0000,0x0000), MK_FP(0xffff,0x0010),128) == 0)
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
    
    if (DosLoadedInHMA) 
    {
        return TRUE;
    }            

    {   /* is very improbable - we are just booting - but
           there might already a XMS handler installed
           this is the case for DOSEMU
        */  
        
        void FAR *pXMS = DetectXMSDriver();
        
        if (pXMS != NULL) 
        {
            XMSDriverAddress = pXMS;
            
            printf("DOSEMU ? detected XMS driver at %p\n", XMSDriverAddress);
            
        }
    }        
    

    
    /* A) for debugging purpose, suppress this, 
          if any shift key is pressed 
    */
#ifdef DEBUG
    if (KeyboardShiftState() & 0x0f)
    {
        printf("Keyboard state is %0x, NOT moving to HMA\n",KeyboardShiftState()); 
        return FALSE;
    }
#endif
    
    /* B) check out, if we can have HMA */        

    if (!EnableHMA())
    {
        printf("Can't enable HMA area (the famous A20), NOT moving to HMA\n"); 
        
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
    
    /* report the fact we are running high thorugh int 21, ax=3306 */
    version_flags |= 0x10;    
    

    return TRUE;
    
errorReturn:
    printf("HMA errors, not doing HMA\n");
    return FALSE;
}


/*





/*   
    now protect against HIMEM/FDXMS/... by simulating a VDISK 
    FDXMS should detect us and not give HMA access to ohers
    unfortunately this also disables HIMEM completely

    so: we install this after all drivers have been loaded
*/
void InstallVDISK(VOID)
    {
        static struct {                /* Boot-Sektor of a RAM-Disk */
 	                 UBYTE dummy1[3];   /* HIMEM.SYS uses 3, but FDXMS uses 2 */
	                 char Name[5];
	                 BYTE dummy2[3];
	                 WORD BpS;
	                 BYTE dummy3[6];
	                 WORD Sektoren;
	                 BYTE dummy4;
	                } VDISK_BOOT_SEKTOR =
	                    {
	                        { 0xcf, ' ', ' '},
	                        { 'V', 'D', 'I', 'S', 'K'},
	                        { ' ', ' ', ' '},
	                        512,
	                        { 'F', 'D', 'O', 'S', ' ', ' '},
	                        128,    /* 128*512 = 64K */
	                        ' ' 
	                    };

    if (!DosLoadedInHMA) return;
    if (HMAclaimed)      return; 
    	                    
	                    
    fmemcpy(MK_FP(0xffff,0x0010), &VDISK_BOOT_SEKTOR, sizeof(VDISK_BOOT_SEKTOR)); 

    setvec(0x19, MK_FP(0xffff,0x0010));   /* let INT 19 point to VDISK */ 
        
    *(WORD FAR *)MK_FP(0xffff,0x002e) = 1024+64;
    }


int  init_call_XMScall( void FAR * driverAddress, UWORD ax, UWORD dx);


/*
    after each driver, we try to allocate the HMA.
    it might be HIMEM.SYS we just loaded.
*/

void ClaimHMA(VOID)
{
    void FAR *pXMS;

    if (!DosLoadedInHMA) return;
    if (HMAclaimed)      return; 
    
    
    pXMS = DetectXMSDriver();
    
    if (pXMS != NULL)
    {
        XMSDriverAddress = pXMS;
            
        if (init_call_XMScall( pXMS, 0x0100, 0xffff))
        {
            printf("HMA area successfully claimed\n");
            HMAclaimed = TRUE;
        }
    }        
}

/*
    this should be called, after each device driver 
    has been loaded with FALSE
    and on finished CONFIG processing with TRUE.
    
    will try to grab HMA;
    
    on finalize, will install a VDISK
*/    
    
    

void HMAconfig(int finalize)
{
    ClaimHMA();
    
    if (finalize)
        InstallVDISK();
}        

/*
    this allocates some bytes from the HMA area
    only available if DOS=HIGH was successful
*/    

VOID FAR *HMAalloc(COUNT bytesToAllocate)
{
    VOID FAR *HMAptr;
    
    if (!DosLoadedInHMA) return NULL;

    if (HMAFree >= 0xfff0  - bytesToAllocate) return NULL;
    
    HMAptr = MK_FP(0xffff, HMAFree);
    
    HMAFree += bytesToAllocate;

    /*printf("HMA allocated %d byte at %x\n", bytesToAllocate, HMAptr); */
    
    fmemset( HMAptr,0, bytesToAllocate);
    
    return HMAptr;
}



unsigned CurrentKernelSegment = 0;

void MoveKernel(unsigned NewKernelSegment)
{
    UBYTE FAR *HMADest;
    UBYTE FAR *HMASource;
    unsigned len;

    __int__(3);
    if (CurrentKernelSegment == 0)
       CurrentKernelSegment = FP_SEG(_HMATextEnd);
       
    if (CurrentKernelSegment == 0xffff)
        return;       
        
    
    HMASource = MK_FP(CurrentKernelSegment,(FP_OFF(_HMATextStart) & 0xfff0));
    HMADest   = MK_FP(NewKernelSegment,0x0000);
    
    len = (FP_OFF(_HMATextEnd) | 0x000f) - (FP_OFF(_HMATextStart) & 0xfff0);
    
    if (NewKernelSegment == 0xffff)
        {        
        HMASource += HMAOFFSET;
        HMADest   += HMAOFFSET;
        len       -= HMAOFFSET;
        }        
            
    HMAInitPrintf(("HMA moving %p up to %p for %04x bytes\n",
                                HMASource, HMADest, len));

    if (NewKernelSegment < CurrentKernelSegment ||
        NewKernelSegment == 0xffff)
        {
        unsigned i; UBYTE FAR *s,FAR *d;    
        
        for (i = 0, s = HMASource,d = HMADest; i < len; i++)
            d[i] = s[i];
        }
    else {
                                            /* might overlap */
        unsigned i; UBYTE FAR *s,FAR *d;    
        
        for (i = len, s = HMASource,d = HMADest; i != 0; i--)
            d[i] = s[i];
        }
       
    HMAFree = FP_OFF(HMADest)+len;  /* first free byte after HMA_TEXT */
    
    {
        /* D) but it only makes sense, if we can relocate 
              all our entries to make use of HMA
        */              

        /* this is for a 
            call near enableA20
            jmp far kernelentry
           style table
        */    
        
        struct RelocationTable {
            UBYTE jmpFar;
            UWORD jmpOffset;
            UWORD jmpSegment;
            UBYTE callNear;
            UWORD callOffset;
        };
        struct RelocatedEntry {
            UBYTE callNear;
            UWORD callOffset;
            UBYTE jmpFar;
            UWORD jmpOffset;
            UWORD jmpSegment;
        };
        extern struct RelocationTable  
                    FAR _HMARelocationTableStart[], 
                    FAR _HMARelocationTableEnd[];

        struct RelocationTable FAR *rp, rtemp ;
        
        /* verify, that all entries are valid */  
        
        for (rp = _HMARelocationTableStart; rp < _HMARelocationTableEnd; rp++)
        {
            if (rp->jmpFar     != 0xea              || /* jmp FAR */
                rp->jmpSegment != CurrentKernelSegment    || /* will only relocate HMA_TEXT */
                rp->callNear   != 0xe8              || /* call NEAR */
                0)
            {
                printf("illegal relocation entry # %d\n",rp - _HMARelocationTableStart);
                goto errorReturn;
            }            
        }
          
        /* OK, all valid, go to relocate*/  
        
        for (rp = _HMARelocationTableStart; rp < _HMARelocationTableEnd; rp++)
        {
            if (NewKernelSegment == 0xffff)
                {
                struct RelocatedEntry FAR *rel = (struct RelocatedEntry FAR *)rp;
                
                fmemcpy(&rtemp, rp, sizeof(rtemp));
                
                rel->jmpFar     = rtemp.jmpFar;
                rel->jmpSegment = NewKernelSegment;
                rel->jmpOffset  = rtemp.jmpOffset;
                rel->callNear   = rtemp.callNear;
                rel->callOffset = rtemp.callOffset+5; /* near calls are relative */
                }
            else
                rp->jmpSegment = NewKernelSegment;
            
        }
    }
    {
        struct initRelocationTable {
            UBYTE callNear;
            UWORD callOffset;
            UBYTE jmpFar;
            UWORD jmpOffset;
            UWORD jmpSegment;
        };
        extern struct initRelocationTable
                    _HMAinitRelocationTableStart[], 
                    _HMAinitRelocationTableEnd[];
        struct initRelocationTable *rp;
        
        /* verify, that all entries are valid */  
        
        for (rp = _HMAinitRelocationTableStart; rp < _HMAinitRelocationTableEnd; rp++)
        {
            if (
                rp->callNear   != 0xe8              || /* call NEAR */
                rp->jmpFar     != 0xea              || /* jmp FAR */
                rp->jmpSegment != CurrentKernelSegment    || /* will only relocate HMA_TEXT */
                0)
            {
                printf("illegal init relocation entry # %d\n",
                       rp - _HMAinitRelocationTableStart);
                goto errorReturn;
            }            
        }
          
        /* OK, all valid, go to relocate*/  
        
        for (rp = _HMAinitRelocationTableStart; rp < _HMAinitRelocationTableEnd; rp++)
        {
            rp->jmpSegment = NewKernelSegment;
        }
    }
    
    CurrentKernelSegment = NewKernelSegment;
    return;
    
errorReturn:
    for (;;);    
}
