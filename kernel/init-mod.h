/* Included by initialisation functions */
#define IN_INIT_MOD

#include "version.h"
#include "date.h"
#include "time.h"
#include "mcb.h"
#include "sft.h"
#include "fat.h"
#include "file.h"
#include "cds.h"
#include "device.h"
#include "kbd.h"
#include "error.h"
#include "fcb.h"
#include "tail.h"
#include "process.h"
#include "pcb.h"
#include "buffer.h"
#include "dcb.h"
#include "lol.h"

#include "init-dat.h"
#include "nls.h"

#include "kconfig.h"

/* MSC places uninitialized data into COMDEF records,
   that end up in DATA segment. this can't be tolerated in INIT code.
   please make sure, that ALL data in INIT is initialized !!

   These guys are marked BSS_INIT to mark that they really should be BSS
   but can't be because of MS
*/
#ifdef _MSC_VER
#define BSS_INIT(x) = x
#else
#define BSS_INIT(x)
#endif

extern struct _KernelConfig InitKernelConfig;

/*
 * Functions in `INIT_TEXT' may need to call functions in `_TEXT'. The entry
 * calls for the latter functions therefore need to be wrapped up with far
 * entry points.
 */
#define printf      init_printf
#define sprintf     init_sprintf
#ifndef __WATCOMC__
#define execrh      init_execrh
#define  memcpy     init_memcpy
#define fmemcpy     init_fmemcpy
#define fmemset     init_fmemset
#define fmemcmp     init_fmemcmp
#define  memcmp     init_memcmp
#define  memset     init_memset
#define  strchr     init_strchr
#define  strcpy     init_strcpy
#define  strlen     init_strlen
#define fstrlen     init_fstrlen
#endif
#define open        init_DosOpen

/* execrh.asm */
#ifndef __WATCOMC__
WORD   ASMPASCAL execrh(request FAR *, struct dhdr FAR *);
#endif

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
char * ASMPASCAL  strchr(const char     *s,  int ch);

#ifdef __WATCOMC__
/* bx, cx, dx and es not used or clobbered for all asmsupt.asm functions except
   (f)memchr/(f)strchr (which clobber dx) */
#pragma aux (pascal) pascal_ax modify exact [ax]
#pragma aux (pascal_ax) memset
#pragma aux (pascal_ax) fmemset
#pragma aux (pascal_ax) memcpy
#pragma aux (pascal_ax) fmemcpy
#pragma aux (pascal_ax) memcmp modify nomemory
#pragma aux (pascal_ax) fmemcmp modify nomemory
#pragma aux (pascal_ax) strcpy
#pragma aux (pascal_ax) strlen modify nomemory
#pragma aux (pascal_ax) fstrlen modify nomemory
#pragma aux (pascal) strchr modify exact [ax dx] nomemory
#endif

#undef LINESIZE
#define LINESIZE KBD_MAXLENGTH

/*inithma.c*/
extern BYTE DosLoadedInHMA;
void MoveKernel(unsigned NewKernelSegment);

void setvec(unsigned char intno, intvec vector);
#ifndef __WATCOMC__
#define getvec init_getvec
#endif
intvec getvec(unsigned char intno);

#define GLOBAL extern
#define NAMEMAX         MAX_CDSPATH     /* Maximum path for CDS         */
#define NFILES          16      /* number of files in table     */
#define NFCBS           16      /* number of fcbs               */
#define NSTACKS         8       /* number of stacks             */
#define STACKSIZE       256     /* default stacksize            */
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
BYTE * GetStringArg(BYTE * pLine, BYTE * pszString);
void DoInstall(void);
UWORD GetBiosKey(int timeout);

/* diskinit.c */
COUNT dsk_init(VOID);

/* int2f.asm */
COUNT ASMPASCAL Umb_Test(void);
COUNT ASMPASCAL UMB_get_largest(void FAR * driverAddress,
                                UCOUNT * seg, UCOUNT * size);
#ifdef __WATCOMC__
#pragma aux (pascal) UMB_get_largest modify exact [ax bx cx dx]
#endif

/* inithma.c */
int MoveKernelToHMA(void);
VOID FAR * HMAalloc(COUNT bytesToAllocate);

/* initoem.c */
unsigned init_oem(void);
void movebda(size_t bytes, unsigned new_seg);
unsigned ebdasize(void);

/* dosidle.asm */
extern void ASM DosIdle_hlt(VOID);

/* intr.asm */

/*
 * Invoke interrupt "nr" with all registers from *rp loaded
 * into the processor registers (except: SS, SP,& flags)
 * On return, all processor registers are stored into *rp (including
 * flags).
 */
unsigned ASMPASCAL init_call_intr(int nr, iregs * rp);

unsigned ASMPASCAL read(int fd, void *buf, unsigned count);
int ASMPASCAL open(const char *pathname, int flags);
int ASMPASCAL close(int fd);
int ASMPASCAL dup2(int oldfd, int newfd);
ULONG ASMPASCAL lseek(int fd, long position);
seg ASMPASCAL allocmem(UWORD size);
void ASMPASCAL init_PSPSet(seg psp_seg);
int ASMPASCAL init_DosExec(int mode, exec_blk * ep, char * lp);
int ASMPASCAL init_setdrive(int drive);
int ASMPASCAL init_switchar(int chr);
void ASMPASCAL keycheck(void);
void ASMPASCAL set_DTA(void far *dta);
#ifdef __WATCOMC__
#pragma aux (pascal) init_call_intr modify exact [ax]
#pragma aux (pascal) read modify exact [ax bx cx dx]
#pragma aux (pascal) init_DosOpen modify exact [ax bx dx]
#pragma aux (pascal) close modify exact [ax bx]
#pragma aux (pascal) dup2 modify exact [ax bx cx]
#pragma aux (pascal) allocmem modify exact [ax bx]
#pragma aux (pascal) init_PSPSet modify exact [ax bx]
#pragma aux (pascal) init_DosExec modify exact [ax bx dx es]
#pragma aux (pascal) init_setdrive modify exact [ax bx dx]
#pragma aux (pascal) init_switchar modify exact [ax bx dx]
#pragma aux (pascal) keycheck modify exact [ax]
#pragma aux (pascal) set_DTA modify exact [ax bx dx]
#endif

/* irqstack.asm */
VOID ASMCFUNC init_stacks(VOID FAR * stack_base, COUNT nStacks,
                          WORD stackSize);

/* inthndlr.c */
VOID ASMCFUNC FAR int0_handler(void);
VOID ASMCFUNC FAR int6_handler(void);
VOID ASMCFUNC FAR int19_handler(void);
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
#ifdef __GNUC__
VOID ASMCFUNC init_call_p_0(struct config FAR *Config) FAR __attribute__((noreturn));
#else
VOID ASMCFUNC FAR init_call_p_0(struct config FAR *Config); /* P_0, actually */
#endif

/* main.c */
VOID ASMCFUNC FreeDOSmain(void);
BOOL init_device(struct dhdr FAR * dhp, char * cmdLine,
                      COUNT mode, char FAR **top);
VOID init_fatal(BYTE * err_msg);

/* prf.c */
int VA_CDECL init_printf(CONST char * fmt, ...);
int VA_CDECL init_sprintf(char * buff, CONST char * fmt, ...);

/* procsupt.asm */
VOID ASMCFUNC FAR got_cbreak(void);

/* initclk.c */
extern void Init_clk_driver(void);

extern UWORD HMAFree;            /* first byte in HMA not yet used      */

extern unsigned CurrentKernelSegment;
extern struct _KernelConfig FAR ASM LowKernelConfig;
extern WORD days[2][13];
extern BYTE FAR *lpTop;
extern BYTE ASM _ib_start[], ASM _ib_end[], ASM _init_end[];
extern UWORD ram_top;               /* How much ram in Kbytes               */
extern char singleStep;
extern char SkipAllConfig;
extern char master_env[128];

extern struct lol FAR *LoL;

extern struct dhdr DOSTEXTFAR ASM blk_dev; /* Block device (Disk) driver           */

extern struct buffer FAR *DOSFAR firstAvailableBuf; /* first 'available' buffer   */
extern struct lol ASM FAR DATASTART;

extern BYTE DOSFAR ASM _HMATextAvailable;    /* first byte of available CODE area    */
extern BYTE FAR ASM _HMATextStart[];          /* first byte of HMAable CODE area      */
extern BYTE FAR ASM _HMATextEnd[];
extern BYTE DOSFAR ASM break_ena;  /* break enabled flag                   */
extern BYTE DOSFAR ASM _InitTextStart[];     /* first available byte of ram          */
extern BYTE DOSFAR ASM _InitTextEnd[];
extern BYTE DOSFAR ASM ReturnAnyDosVersionExpected;
extern BYTE DOSFAR ASM HaltCpuWhileIdle;

extern BYTE DOSFAR ASM internal_data[];
extern unsigned char DOSTEXTFAR ASM kbdType;

extern struct {
  char  ThisIsAConstantOne;
  short TableSize;
  
  struct CountrySpecificInfo C;
  
} DOSFAR ASM nlsCountryInfoHardcoded;

/*
    data shared between DSK.C and INITDISK.C
*/

extern UWORD DOSFAR LBA_WRITE_VERIFY;

/* original interrupt vectors, at 70:xxxx */
extern struct lowvec {
  unsigned char intno;
  intvec isv;
} DOSTEXTFAR ASM intvec_table[5];

/* floppy parameter table, at 70:xxxx */
extern unsigned char DOSTEXTFAR ASM int1e_table[0xe];

extern char DOSFAR DiskTransferBuffer[/*MAX_SEC_SIZE*/]; /* in dsk.c */

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

extern struct RelocationTable DOSFAR ASM _HMARelocationTableStart[];
extern struct RelocationTable DOSFAR ASM _HMARelocationTableEnd[];

extern void FAR *DOSFAR ASM XMSDriverAddress;
#ifdef __GNUC__
extern VOID ASMPASCAL _EnableA20(VOID) FAR;
extern VOID ASMPASCAL _DisableA20(VOID) FAR;
#else
extern VOID ASMPASCAL FAR _EnableA20(VOID);
extern VOID ASMPASCAL FAR _DisableA20(VOID);
#endif

extern void FAR * ASMPASCAL DetectXMSDriver(VOID);
extern int ASMPASCAL init_call_XMScall(void FAR * driverAddress, UWORD ax,
                                      UWORD dx);
#ifdef __WATCOMC__
#pragma aux (pascal) DetectXMSDriver modify exact [ax dx]
#pragma aux (pascal) _EnableA20 modify exact [ax]
#pragma aux (pascal) _DisableA20 modify exact [ax]
#endif

#if defined(WATCOM) && 0
ULONG ASMCFUNC FAR MULULUS(ULONG mul1, UWORD mul2);     /* MULtiply ULong by UShort */
ULONG ASMCFUNC FAR MULULUL(ULONG mul1, ULONG mul2);     /* MULtiply ULong by ULong */
ULONG ASMCFUNC FAR DIVULUS(ULONG mul1, UWORD mul2);     /* DIVide ULong by UShort */
ULONG ASMCFUNC FAR DIVMODULUS(ULONG mul1, UWORD mul2, UWORD * rem);     /* DIVide ULong by UShort */
#endif

