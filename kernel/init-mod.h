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

#ifdef __TURBOC__
void __int__(int);              /* TC 2.01 requires this. :( -- ror4 */
#endif

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
WORD execrh(request FAR *, struct dhdr FAR *);
VOID fmemcpy(REG VOID FAR * d, REG VOID FAR * s, REG COUNT n);
void fmemset(REG VOID FAR * s, REG int ch, REG COUNT n);
void memset(REG VOID * s, REG int ch, REG COUNT n);
VOID strcpy(REG BYTE * d, REG BYTE * s);
VOID fstrncpy(REG BYTE FAR * d, REG BYTE FAR * s, REG COUNT n);
COUNT fstrlen(REG BYTE FAR * s);
COUNT strlen(REG BYTE * s);

#undef LINESIZE
#define LINESIZE KBD_MAXLENGTH
#define fbcopy(s, d, n)    fmemcpy(d,s,n)

/*inithma.c*/
extern BYTE DosLoadedInHMA;
extern fmemcmp(BYTE far *s1, BYTE FAR *s2, unsigned len);

#define setvec(n, isr)  (void)(*(VOID (INRPT FAR * FAR *)())(4 * (n)) = (isr))

#define fbcopy(s, d, n)    fmemcpy(d,s,n)
#define GLOBAL extern
#define NAMEMAX         MAX_CDSPATH     /* Maximum path for CDS         */
#define PARSE_MAX       MAX_CDSPATH     /* maximum # of bytes in path   */
#define NFILES          16      /* number of files in table     */
#define NFCBS           16      /* number of fcbs               */
#define NSTACKS         8       /* number of stacks             */
#define NLAST           6       /* last drive                   */
#define NUMBUFF         6       /* Number of track buffers      */
                                        /* -- must be at least 3        */


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
INIT VOID mcb_init(mcb FAR * mcbp, UWORD size);
INIT VOID strcat(REG BYTE * d, REG BYTE * s);
INIT BYTE FAR *KernelAlloc(WORD nBytes);
INIT COUNT Umb_Test(void);
INIT BYTE *GetStringArg(BYTE * pLine, BYTE * pszString);

/* int2f.asm */
COUNT Umb_Test(void);

/* inithma.c */
int MoveKernelToHMA(void);
VOID FAR *HMAalloc(COUNT bytesToAllocate);

/* initoem.c */
UWORD init_oem(void);

/* intr.asm */
/* void init_call_intr(int nr, iregs *rp); */
UCOUNT read(int fd, void *buf, UCOUNT count); 
int open(const char *pathname, int flags);
int close(int fd);
int dup2(int oldfd, int newfd); 
int allocmem(UWORD size, seg *segp);
INIT VOID init_PSPInit(seg psp_seg);
INIT COUNT init_DosExec(COUNT mode, exec_blk * ep, BYTE * lp);
INIT VOID keycheck(VOID);

/* irqstack.asm */
VOID init_stacks(VOID FAR * stack_base, COUNT nStacks, WORD stackSize);

/* inthndlr.c */
VOID far int21_entry(iregs UserRegs);
VOID int21_service(iregs far * r);
VOID INRPT FAR int0_handler(void);
VOID INRPT FAR int6_handler(void);
VOID INRPT FAR empty_handler(void);
VOID INRPT far got_cbreak(void);  /* procsupt.asm */
VOID INRPT far int20_handler(iregs UserRegs);
VOID INRPT far int21_handler(iregs UserRegs);
VOID INRPT FAR int22_handler(void);
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs
                             , int flags);
VOID INRPT FAR int24_handler(void);
VOID INRPT FAR low_int25_handler(void);
VOID INRPT FAR low_int26_handler(void);
VOID INRPT FAR int27_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs
                             , int flags);
VOID INRPT FAR int28_handler(void);
VOID INRPT FAR int29_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs
                             , int flags);
VOID INRPT FAR int2a_handler(void);
VOID INRPT FAR int2f_handler(void);

/* main.c */
INIT VOID main(void);
INIT BOOL init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine, COUNT mode, COUNT top);
INIT VOID init_fatal(BYTE * err_msg);

/* prf.c */
WORD init_printf(CONST BYTE * fmt,...);
