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
#include "dcb.h"
#include "lol.h"

#include "init-dat.h"

#include "kconfig.h"
extern struct _KernelConfig InitKernelConfig;

/*
 * Functions in `INIT_TEXT' may need to call functions in `_TEXT'. The entry
 * calls for the latter functions therefore need to be wrapped up with far
 * entry points.
 */
#define printf      init_printf
#define sprintf     init_sprintf
#define execrh      init_execrh
#define  memcpy     init_memcpy
#define fmemcpy     init_fmemcpy
#define fmemset     init_fmemset
#define fmemcmp     init_fmemcmp
#define  memcmp     init_memcmp
#define  memset     init_memset
#define  strcpy     init_strcpy
#define  strlen     init_strlen
#define fstrlen     init_fstrlen

/* execrh.asm */
WORD   ASMCFUNC  execrh(request FAR *, struct dhdr FAR *);

/* asmsupt.asm */
VOID   ASMPASCAL  memset(      void     *s,  int ch,             size_t n);
VOID   ASMPASCAL fmemset(      void FAR *s,  int ch,             size_t n);
int    ASMPASCAL  memcmp(const void     *m1, const void     *m2, size_t n);
int    ASMPASCAL fmemcmp(const void FAR *m1, const void FAR *m2, size_t n);
VOID   ASMPASCAL  memcpy(      void     *d,  const void     *s,  size_t n);
VOID   ASMPASCAL fmemcpy(      void FAR *d,  const void FAR *s,  size_t n);
VOID   ASMPASCAL  strcpy(char           *d,  const char     *s);
size_t ASMPASCAL  strlen(const char     *s);
size_t ASMPASCAL fstrlen(const char FAR *s);
 
#undef LINESIZE
#define LINESIZE KBD_MAXLENGTH

/*inithma.c*/
extern BYTE DosLoadedInHMA;
void MoveKernel(unsigned NewKernelSegment);

#define setvec(n, isr) (void)(*(intvec FAR *)MK_FP(0,4 * (n)) = (isr))
#define getvec(n) (*(intvec FAR *)MK_FP(0,4 * (n)))

#define GLOBAL extern
#define NAMEMAX         MAX_CDSPATH     /* Maximum path for CDS         */
#define NFILES          16      /* number of files in table     */
#define NFCBS           16      /* number of fcbs               */
#define NSTACKS         8       /* number of stacks             */
#define NLAST           5       /* last drive                   */
#define NUMBUFF         20      /* Number of track buffers at INIT time     */
                                        /* -- must be at least 3        */
#define MAX_HARD_DRIVE  8
#define NDEV            26      /* up to Z:                     */

#include "config.h" /* config structure */

/* config.c */
extern struct config Config;
VOID PreConfig(VOID);
VOID PreConfig2(VOID);
VOID DoConfig(int pass);
VOID PostConfig(VOID);
VOID configDone(VOID);
VOID FAR * KernelAlloc(size_t nBytes, char type, int mode);
void FAR * KernelAllocPara(size_t nPara, char type, char *name, int mode);
char *strcat(char * d, const char * s);
COUNT ASMCFUNC Umb_Test(void);
COUNT ASMCFUNC UMB_get_largest(UCOUNT * seg, UCOUNT * size);
BYTE * GetStringArg(BYTE * pLine, BYTE * pszString);
void DoInstall(void);
UWORD GetBiosKey(int timeout);

/* diskinit.c */
COUNT dsk_init(VOID);

/* int2f.asm */
COUNT ASMCFUNC Umb_Test(void);

/* inithma.c */
int MoveKernelToHMA(void);
VOID FAR * HMAalloc(COUNT bytesToAllocate);

/* initoem.c */
unsigned init_oem(void);
void movebda(size_t bytes, unsigned new_seg);
unsigned ebdasize(void);

/* intr.asm */

void ASMCFUNC init_call_intr(int nr, iregs * rp);
UCOUNT ASMCFUNC read(int fd, void *buf, UCOUNT count);
int ASMCFUNC open(const char *pathname, int flags);
int ASMCFUNC close(int fd);
int ASMCFUNC dup2(int oldfd, int newfd);
int ASMCFUNC allocmem(UWORD size, seg * segp);
VOID ASMCFUNC init_PSPSet(seg psp_seg);
COUNT ASMCFUNC init_DosExec(COUNT mode, exec_blk * ep, BYTE * lp);
int ASMCFUNC init_setdrive(int drive);
int ASMCFUNC init_switchar(int chr);
VOID ASMCFUNC keycheck(VOID);
void ASMCFUNC set_DTA(void far *dta);

/* irqstack.asm */
VOID ASMCFUNC init_stacks(VOID FAR * stack_base, COUNT nStacks,
                          WORD stackSize);

/* inthndlr.c */
VOID ASMCFUNC FAR int21_entry(iregs UserRegs);
VOID ASMCFUNC int21_service(iregs far * r);
VOID ASMCFUNC FAR int0_handler(void);
VOID ASMCFUNC FAR int6_handler(void);
VOID ASMCFUNC FAR empty_handler(void);
VOID ASMCFUNC FAR int20_handler(void);
VOID ASMCFUNC FAR int21_handler(void);
VOID ASMCFUNC FAR int22_handler(void);
VOID ASMCFUNC FAR int24_handler(void);
VOID ASMCFUNC FAR low_int25_handler(void);
VOID ASMCFUNC FAR low_int26_handler(void);
VOID ASMCFUNC FAR int27_handler(void);
VOID ASMCFUNC FAR int28_handler(void);
VOID ASMCFUNC FAR int29_handler(void);
VOID ASMCFUNC FAR int2a_handler(void);
VOID ASMCFUNC FAR int2f_handler(void);
VOID ASMCFUNC FAR cpm_entry(void);

/* kernel.asm */
VOID ASMCFUNC FAR init_call_p_0(struct config FAR *Config); /* P_0, actually */

/* main.c */
VOID ASMCFUNC FreeDOSmain(void);
BOOL init_device(struct dhdr FAR * dhp, char * cmdLine,
                      COUNT mode, char FAR **top);
VOID init_fatal(BYTE * err_msg);

/* prf.c */
WORD CDECL init_printf(CONST BYTE * fmt, ...);
WORD CDECL init_sprintf(BYTE * buff, CONST BYTE * fmt, ...);

/* procsupt.asm */
VOID ASMCFUNC FAR got_cbreak(void);

/* initclk.c */
extern void Init_clk_driver(void);

extern UWORD HMAFree;            /* first byte in HMA not yet used      */

extern unsigned CurrentKernelSegment;
extern struct _KernelConfig FAR ASM LowKernelConfig;
extern WORD days[2][13];
extern BYTE FAR *lpOldTop;
extern BYTE FAR *lpTop;
extern BYTE ASM _ib_start[], ASM _ib_end[], ASM _init_end[];
extern UWORD ram_top;               /* How much ram in Kbytes               */
extern char singleStep;
extern char SkipAllConfig;
extern char master_env[128];

extern struct lol FAR *LoL;

extern struct dhdr DOSTEXTFAR ASM blk_dev; /* Block device (Disk) driver           */

extern struct buffer FAR *DOSFAR firstAvailableBuf; /* first 'available' buffer   */
extern struct lol ASM DOSFAR DATASTART;

extern BYTE DOSFAR ASM _HMATextAvailable,    /* first byte of available CODE area    */
  FAR ASM _HMATextStart[],          /* first byte of HMAable CODE area      */
  FAR ASM _HMATextEnd[], DOSFAR ASM break_ena;  /* break enabled flag                   */
extern BYTE DOSFAR _InitTextStart,       /* first available byte of ram          */
  DOSFAR ReturnAnyDosVersionExpected;

extern BYTE FAR ASM internal_data[];
extern unsigned char FAR ASM kbdType;

extern struct {
  char  ThisIsAConstantOne;
  short TableSize;
  
  struct CountrySpecificInfo C;
  
} FAR ASM nlsCountryInfoHardcoded;

/*
    data shared between DSK.C and INITDISK.C
*/

extern UWORD DOSFAR LBA_WRITE_VERIFY;

/* floppy parameter table, at 70:xxxx */
extern unsigned char DOSTEXTFAR ASM int1e_table[0xe];

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
   DOSTEXTFAR ASM _HMARelocationTableStart[],
   DOSTEXTFAR ASM _HMARelocationTableEnd[];

extern void FAR *DOSTEXTFAR ASM XMSDriverAddress;
extern VOID ASMCFUNC FAR _EnableA20(VOID);
extern VOID ASMCFUNC FAR _DisableA20(VOID);

extern void FAR * ASMCFUNC DetectXMSDriver(VOID);
extern int ASMCFUNC init_call_XMScall(void FAR * driverAddress, UWORD ax,
                                      UWORD dx);

#if defined(WATCOM) && 0
ULONG ASMCFUNC FAR MULULUS(ULONG mul1, UWORD mul2);     /* MULtiply ULong by UShort */
ULONG ASMCFUNC FAR MULULUL(ULONG mul1, ULONG mul2);     /* MULtiply ULong by ULong */
ULONG ASMCFUNC FAR DIVULUS(ULONG mul1, UWORD mul2);     /* DIVide ULong by UShort */
ULONG ASMCFUNC FAR DIVMODULUS(ULONG mul1, UWORD mul2, UWORD * rem);     /* DIVide ULong by UShort */
#endif

