/* Included by initialisation functions */
#define IN_INIT_MOD

#include "version.h"
#include "date.h"
#include "time.h"
#include "mcb.h"
#include "sft.h"
#include "fat.h"
#include "fnode.h"
#include "file.h"
#include "dcb.h"
#include "cds.h"
#include "device.h"
#include "kbd.h"
#include "error.h"
#include "fcb.h"
#include "tail.h"
#include "process.h"
#include "pcb.h"
#include "nls.h"
#include "buffer.h"

/*
 * The null macro `INIT' can be used to allow the reader to differentiate
 * between functions defined in `INIT_TEXT' and those defined in `_TEXT'.
 */
#define INIT
/*
 * Functions in `INIT_TEXT' may need to call functions in `_TEXT'. The entry
 * calls for the latter functions therefore need to be wrapped up with far
 * entry points.
 */
#define printf      init_printf
#define execrh      reloc_call_execrh
#define fmemcpy     reloc_call_fmemcpy
#define fmemset     reloc_call_fmemset
#define memset      reloc_call_memset
#define fstrncpy    reloc_call_fstrncpy
#define strcpy      reloc_call_strcpy
#define strlen      reloc_call_strlen
WORD ASMCFUNC execrh(request FAR *, struct dhdr FAR *);
VOID ASMCFUNC fmemcpy(REG VOID FAR * d, REG VOID FAR * s, REG COUNT n);
void ASMCFUNC fmemset(REG VOID FAR * s, REG int ch, REG COUNT n);
void ASMCFUNC memset(REG VOID * s, REG int ch, REG COUNT n);
VOID ASMCFUNC strcpy(REG BYTE * d, REG BYTE * s);
VOID ASMCFUNC fstrncpy(REG BYTE FAR * d, REG BYTE FAR * s, REG COUNT n);
COUNT ASMCFUNC fstrlen(REG BYTE FAR * s);
COUNT ASMCFUNC strlen(REG BYTE * s);

#undef LINESIZE
#define LINESIZE KBD_MAXLENGTH
#define fbcopy(s, d, n)    fmemcpy(d,s,n)

/*inithma.c*/
extern BYTE DosLoadedInHMA;
extern fmemcmp(BYTE far *s1, BYTE FAR *s2, unsigned len);

#define setvec(n, isr)  (void)(*(VOID (FAR * FAR *)())(MK_FP(0,4 * (n))) = (isr))

#define fbcopy(s, d, n)    fmemcpy(d,s,n)
#define GLOBAL extern
#define NAMEMAX         MAX_CDSPATH     /* Maximum path for CDS         */
#define PARSE_MAX       MAX_CDSPATH     /* maximum # of bytes in path   */
#define NFILES          16      /* number of files in table     */
#define NFCBS           16      /* number of fcbs               */
#define NSTACKS         8       /* number of stacks             */
#define NLAST           5       /* last drive                   */
#define NUMBUFF         6       /* Number of track buffers      */
                                        /* -- must be at least 3        */
#define MAX_HARD_DRIVE  8
#define NDEV            26      /* up to Z:                     */



/* Start of configuration variables                                     */
struct config
{
    UBYTE cfgBuffers;
    /* number of buffers in the system      */
    UBYTE cfgFiles;
    /* number of available files            */
    UBYTE cfgFcbs;
    /* number of available FCBs             */
    UBYTE cfgProtFcbs;
    /* number of protected FCBs             */
    BYTE cfgInit[NAMEMAX];
    /* init of command.com          */
    BYTE cfgInitTail[NAMEMAX];
    /* command.com's tail           */
    UBYTE cfgLastdrive;
    /* last drive                           */
    BYTE cfgStacks;
    /* number of stacks                     */
    UWORD cfgStackSize;
    /* stacks size for each stack           */
    /* COUNTRY=
              In Pass #1 these information is collected and in PostConfig()
                     the NLS package is loaded into memory.
                     -- 2000/06/11 ska*/
    WORD cfgCSYS_cntry;
    /* country ID to be loaded */
    WORD cfgCSYS_cp;
    /* requested codepage; NLS_DEFAULT if default */
    BYTE cfgCSYS_fnam[NAMEMAX];
    /* filename of COUNTRY= */
    WORD cfgCSYS_memory;
    /* number of bytes required for the NLS pkg;
       0 if none */
    VOID FAR *cfgCSYS_data;
    /* where the loaded data is for PostConfig() */
    UBYTE cfgP_0_startmode;
    /* load command.com high or not */
};

extern struct config Config;

/* config.c */
INIT VOID PreConfig(VOID);
INIT VOID DoConfig(VOID);
INIT VOID PostConfig(VOID);
INIT BYTE FAR *KernelAlloc(WORD nBytes);
INIT BYTE *skipwh(BYTE * s);
INIT BYTE *scan(BYTE * s, BYTE * d);
INIT BOOL isnum(BYTE * pszString);
INIT BYTE *GetNumber(REG BYTE * pszString, REG COUNT * pnNum);
INIT COUNT tolower(COUNT c);
INIT COUNT toupper(COUNT c);
INIT VOID mcb_init(UCOUNT seg, UWORD size);
INIT VOID strcat(REG BYTE * d, REG BYTE * s);
INIT BYTE FAR *KernelAlloc(WORD nBytes);
INIT COUNT ASMCFUNC Umb_Test(void);
INIT COUNT ASMCFUNC UMB_get_largest(UCOUNT *seg, UCOUNT *size);
INIT BYTE *GetStringArg(BYTE * pLine, BYTE * pszString);

/* diskinit.c */
COUNT dsk_init(VOID);

/* int2f.asm */
COUNT ASMCFUNC Umb_Test(void);

/* inithma.c */
int MoveKernelToHMA(void);
VOID FAR *HMAalloc(COUNT bytesToAllocate);

/* initoem.c */
UWORD init_oem(void);

/* intr.asm */

void ASMCFUNC init_call_intr(int nr, iregs *rp);
UCOUNT ASMCFUNC read(int fd, void *buf, UCOUNT count); 
int ASMCFUNC open(const char *pathname, int flags);
int ASMCFUNC close(int fd);
int ASMCFUNC dup2(int oldfd, int newfd); 
int ASMCFUNC allocmem(UWORD size, seg *segp);
INIT VOID ASMCFUNC init_PSPInit(seg psp_seg);
INIT VOID ASMCFUNC init_PSPSet(seg psp_seg);
INIT COUNT ASMCFUNC init_DosExec(COUNT mode, exec_blk * ep, BYTE * lp);
INIT VOID ASMCFUNC keycheck(VOID);

/* irqstack.asm */
VOID ASMCFUNC init_stacks(VOID FAR * stack_base, COUNT nStacks, WORD stackSize);

/* inthndlr.c */
VOID far 	   ASMCFUNC int21_entry(iregs UserRegs);
VOID 		   ASMCFUNC int21_service(iregs far * r);
VOID FAR ASMCFUNC int0_handler(void);
VOID FAR ASMCFUNC int6_handler(void);
VOID FAR ASMCFUNC empty_handler(void);
VOID far ASMCFUNC got_cbreak(void);  /* procsupt.asm */
VOID far ASMCFUNC int20_handler(iregs UserRegs);
VOID far ASMCFUNC int21_handler(iregs UserRegs);
VOID FAR ASMCFUNC int22_handler(void);
VOID FAR ASMCFUNC int24_handler(void);
VOID FAR ASMCFUNC low_int25_handler(void);
VOID FAR ASMCFUNC low_int26_handler(void);
VOID FAR ASMCFUNC int27_handler(void);
VOID FAR ASMCFUNC int28_handler(void);
VOID FAR ASMCFUNC int29_handler(void);
VOID FAR ASMCFUNC int2a_handler(void);
VOID FAR ASMCFUNC int2f_handler(void);

/* main.c */
INIT VOID ASMCFUNC FreeDOSmain(void);
INIT BOOL init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine, COUNT mode, COUNT top);
INIT VOID init_fatal(BYTE * err_msg);

/* prf.c */
WORD init_printf(CONST BYTE * fmt,...);

void MoveKernel(unsigned NewKernelSegment);
extern WORD HMAFree;          /* first byte in HMA not yet used      */

extern unsigned CurrentKernelSegment;

#if defined(WATCOM) && 0
ULONG FAR ASMCFUNC MULULUS(ULONG mul1, UWORD mul2); /* MULtiply ULong by UShort */
ULONG FAR ASMCFUNC MULULUL(ULONG mul1, ULONG mul2); /* MULtiply ULong by ULong */
ULONG FAR ASMCFUNC DIVULUS(ULONG mul1, UWORD mul2); /* DIVide ULong by UShort */
ULONG FAR ASMCFUNC DIVMODULUS(ULONG mul1, UWORD mul2,UWORD *rem); /* DIVide ULong by UShort */
#endif
