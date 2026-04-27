/*
 * testlib.h  --  shared test infrastructure
 *
 * Layout:
 *   tests/
 *       greatest.h
 *       testlib.h        <- this file
 *       testlib.c
 *       int2f-12/
 *           t12xx.c      <- each subfunction test, includes "../testlib.h"
 *
 * Build with Open Watcom 16-bit (wcl -zq -q -ms ...)
 * Targets real-mode DOS (small model).
 *
 * Two runtime modes, selected by argv[1]:
 *   r / R             REFERENCE mode: run tests, write NNN.ref, print verbose
 *   (none / anything) TEST mode:      read NNN.ref, run tests, write NNN.tst
 *
 * .ref file format
 * ----------------
 * Lines beginning with '#' are comments.  Blank lines are ignored.
 *
 * Register line:
 *   NAME AX BX CX DX SI DI DS ES FLAGS
 *   Each field is a 4-digit hex value or "????" (don't-care).
 *   For AX/BX/CX/DX the high byte may be "??" and low byte a 2-digit hex
 *   value (or vice-versa) to express byte-level care:
 *     e.g.  "??1A" means care only about AL=0x1A, ignore AH.
 *           "1A??" means care only about AH=0x1A, ignore AL.
 *
 * Buffer line (optional, immediately follows its register line):
 *   B NAME COUNT HH HH ...
 *   COUNT is a decimal byte count (may be 0).
 *   HH are COUNT space-separated 2-digit hex bytes.
 *   The empty string "" is represented as:  B name 1 00
 *
 */

#ifndef TESTLIB_H
#define TESTLIB_H

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -----------------------------------------------------------------------
 * Version
 * --------------------------------------------------------------------- */

#define TESTLIB_VERSION     "2.0"

/* -----------------------------------------------------------------------
 * Sentinel: "don't care" value in a .ref field
 * --------------------------------------------------------------------- */

#define REF_DONTCARE        0xFEADU

/* Maximum line length when reading .ref files (long buffer hex lines)   */
#define REF_LINE_MAX        1024

/* Maximum name length of a test case, must be less than REF_LINE_MAX    */
#define TC_NAME_MAX 128

/* -----------------------------------------------------------------------
 * Register snapshot
 * --------------------------------------------------------------------- */

typedef struct {
    unsigned short ax;
    unsigned short bx;
    unsigned short cx;
    unsigned short dx;
    unsigned short si;
    unsigned short di;
    unsigned short ds;
    unsigned short es;
    unsigned short flags;
} REGS16;

/* -----------------------------------------------------------------------
 * PROBE -- per-call descriptor
 *
 * Caller fills ax_in, before.*, and optionally buffer/bufsize before
 * calling probe_int2f().  probe_int2f() fills after.*.
 *
 * buffer / bufsize
 *   Optional I/O buffer (far pointer as may be DOS owned != our DS)
 *   testlib does NOT free or own this memory.
 *   In run_case(), when CARE_BUF is set in care_mask, the contents of
 *   buffer[0..bufsize-1] after the call are recorded to / compared
 *   against the TC_RECORD's out_buf.
 *   Set buffer=NULL and bufsize=0 if no buffer tracking is needed.
 *
 *   Optionally may hex dump contents in reference mode only.
 *
 * value
 *   Optional word pushed on the stack immediately before INT 2Fh.
 *   Set to 0 if unused (it is always pushed; 0 is harmless for callers
 *   that ignore the stack word).
 * --------------------------------------------------------------------- */

typedef struct {
                                    /* AX --> (AH=12h, AL=subfunction)        */
    REGS16              before;     /* register state to load before INT 2Fh  */
    REGS16              after;      /* register state captured after INT 2Fh  */
    unsigned char far  *buffer;     /* I/O buffer, or NULL, hex dumped        */
                                    /* far as may be DOS provided ptr         */
    unsigned short      bufsize;    /* valid bytes in buffer (0 if none)      */
    unsigned short      value;      /* word pushed before INT 2Fh             */
} PROBE;

/* -----------------------------------------------------------------------
 * care_mask bit positions
 *
 * Bits 0-8: which full registers to compare.
 * Bits 9-16: byte-level overrides for AX/BX/CX/DX.
 *   CARE_xH  -- compare only the high byte of register x (ignore low byte)
 *   CARE_xL  -- compare only the low  byte of register x (ignore high byte)
 *   If both CARE_xH and CARE_xL are set (or CARE_x is set), full word compared.
 *   CARE_x alone (without CARE_xH/CARE_xL) also means full word compared.
 *
 * Bit 17: CARE_BUF -- compare probe->buffer[0..bufsize-1] against stored ref.
 * --------------------------------------------------------------------- */

/* Byte-level care for AX */
#define CARE_AL     0x0001U
#define CARE_AH     0x0002U
/* Byte-level care for BX */
#define CARE_BL     0x0004U
#define CARE_BH     0x0008U
/* Byte-level care for CX */
#define CARE_CL     0x0010U
#define CARE_CH     0x0020U
/* Byte-level care for DX */
#define CARE_DL     0x0040U
#define CARE_DH     0x0080U

#define CARE_AX     (CARE_AH | CARE_AL)
#define CARE_BX     (CARE_BH | CARE_BL)
#define CARE_CX     (CARE_CH | CARE_CL)
#define CARE_DX     (CARE_DH | CARE_DL)
#define CARE_SI     0x0100U
#define CARE_DI     0x0200U
#define CARE_DS     0x0400U
#define CARE_ES     0x0800U
#define CARE_FLAGS  0x1000U
#define CARE_BUF    0x2000U

#define CARE_NONE   0x0000U

/* -----------------------------------------------------------------------
 * TC_RECORD -- one test case, heap-allocated, singly-linked list node
 *
 * All pointer fields are heap-allocated and owned by this record.
 * tc_free() releases them all.
 * --------------------------------------------------------------------- */

typedef struct TC_RECORD {
    char                   *name;       /* heap: strlen(name)+1 bytes        */

    /* Expected register values after the call. */
    REGS16                  expected;

    /* Which registers to compare (CARE_xx bitmask, including byte bits).  */
    unsigned short          care_mask;

    /* Captured output buffer (heap: out_len bytes, or NULL if none).      */
    unsigned char          *out_buf;
    unsigned short          out_len;

    /* Linked list */
    struct TC_RECORD       *next;
} TC_RECORD;

/* -----------------------------------------------------------------------
 * TC_LIST -- linked list root/tail/count
 * --------------------------------------------------------------------- */

typedef struct {
    TC_RECORD  *root;   /* first record; NULL if empty                     */
    TC_RECORD  *tail;   /* last  record; NULL if empty                     */
    int         count;  /* total records in chain                          */
} TC_LIST;

/* -----------------------------------------------------------------------
 * Flag bit positions (8086 FLAGS register)
 * --------------------------------------------------------------------- */

#define FLAG_CF     0x0001U
#define FLAG_PF     0x0004U
#define FLAG_AF     0x0010U
#define FLAG_ZF     0x0040U
#define FLAG_SF     0x0080U
#define FLAG_TF     0x0100U
#define FLAG_IF     0x0200U
#define FLAG_DF     0x0400U
#define FLAG_OF     0x0800U

/* ----
 * fn_run_hook -- post-probe, pre-comparison callback for run_case()
 *
 * Called by run_case() after probe_int2f() has filled p->after, but
 * before any .ref writing or comparison is performed.  The hook may
 * inspect p->after and update any PROBE fields -- such as
 * p->buffer and p->bufsize -- to reflect data that is only known after
 * the INT 2Fh call returns (e.g. a pointer returned in ES:DI).
 *
 * Return value:
 *   0  -- continue normally (proceed to ref_write / tst_compare).
 *  !0  -- abort this case; run_case() returns 0 (failure) immediately
 *         without writing to .ref or comparing.  The hook should print
 *         a diagnostic before returning non-zero.
 *
 * Pass NULL as the fn argument to run_case() when no hook is needed;
 * ---- */

typedef int (fn_run_hook)(const char *name, PROBE *p, unsigned short care);

/* -----------------------------------------------------------------------
 * Globals  (defined in testlib.c)
 * --------------------------------------------------------------------- */

extern const char *g_tbasename; /* declared by test e.g. "1204" */
extern int      g_ref_mode;     /* 1 = reference mode, 0 = test mode       */
extern FILE    *g_ref_fp;       /* .ref file (write in ref mode)            */
extern FILE    *g_tst_fp;       /* .tst file (write in test mode)           */
extern TC_LIST  g_tclist;       /* the global linked list of test records   */

/* -----------------------------------------------------------------------
 * TC_RECORD lifecycle
 * --------------------------------------------------------------------- */

/*
 * tc_create(name)
 *   Allocate and zero-initialise a new TC_RECORD.
 *   Heap-allocates a copy of name.
 *   All expected fields initialised to REF_DONTCARE.
 *   out_buf = NULL, out_len = 0, next = NULL.
 *   Returns NULL on allocation failure.
 */
TC_RECORD *tc_create(const char *name);

/*
 * tc_set_buf(rec, ptr, len)
 *   Replace rec->out_buf with a heap copy of ptr[0..len-1].
 *   Frees any previous out_buf.
 *   len=0 or ptr=NULL clears the buffer (sets out_buf=NULL, out_len=0).
 */
void tc_set_buf(TC_RECORD *rec,
                const unsigned char *ptr,
                unsigned short len);

/*
 * tc_add(list, rec)
 *   Append rec to the tail of list.  rec->next must be NULL.
 */
void tc_add(TC_LIST *list, TC_RECORD *rec);

/*
 * tc_find(list, name)
 *   Linear search by name.  Returns first match or NULL.
 */
TC_RECORD *tc_find(TC_LIST *list, const char *name);

/*
 * tc_free(rec)
 *   Free a single TC_RECORD and all its owned heap memory.
 *   Does NOT unlink from any list; caller must do that first.
 */
void tc_free(TC_RECORD *rec);

/*
 * tc_free_all(list)
 *   Free every record in the list and reset list to empty.
 */
void tc_free_all(TC_LIST *list);

/* -----------------------------------------------------------------------
 * Initialisation / teardown
 * --------------------------------------------------------------------- */

/*
 * testlib_init(basename, argc, argv)
 *   Initialises globals, opens files, loads .ref in test mode.
 *   basename: e.g. "1211" -> "1211.ref" / "1211.tst" now global g_tbasename declared in test 
 *   argv[1] == "r" or "R" -> reference mode; otherwise test mode.
 *   Returns 0 on success, -1 on error.
 */
int  testlib_init(int argc, char **argv);

/*
 * testlib_finish()
 *   Flushes/closes output files and frees all TC_RECORD heap memory.
 */
void testlib_finish(void);

/* -----------------------------------------------------------------------
 * Core INT 2Fh probe
 * --------------------------------------------------------------------- */

/*
 * probe_int2f(p)
 *   Issues INT 2Fh with registers from p->before (assumes AX indicates function).
 *   If p->before.ds == 0, substitutes the DOS data segment automatically
 *   and updates p->before.ds so the caller can see what was used.
 *   Captures all output registers and FLAGS into p->after.
 *   p->value is pushed on the stack immediately before INT 2Fh.
 */
void probe_int2f(PROBE far *p);

/* -----------------------------------------------------------------------
 * .ref file I/O
 * --------------------------------------------------------------------- */

/*
 * ref_write(name, after, care_mask)
 *   Write one register line to g_ref_fp.
 *   Fields not in care_mask are written as "????".
 *   For AX/BX/CX/DX, byte-level care bits cause "??HH" or "HH??" output.
 */
void ref_write(const char *name,
               const REGS16 far *after,
               unsigned short care_mask);

/*
 * ref_write_buf(name, ptr, len)
 *   Write a "B NAME COUNT HH HH ..." line to g_ref_fp.
 *   len=0 writes "B NAME 0" with no hex bytes.
 */
void ref_write_buf(const char *name,
                   const unsigned char far *ptr,
                   unsigned short len);

/*
 * ref_load(filename, list)
 *   Parse a .ref file and populate list with TC_RECORDs.
 *   Register lines create new records; "B" lines attach buffers to the
 *   most-recently created record.
 *   Returns number of register records loaded, or -1 on file-open error.
 */
int ref_load(const char *filename, TC_LIST *list);

/* -----------------------------------------------------------------------
 * Test mode comparison
 * --------------------------------------------------------------------- */

/*
 * tst_compare(name, after, rec)
 *   Compare after against rec->exp_* fields honoring rec->care_mask.
 *   Prints PASS/FAIL to stdout and g_tst_fp.
 *   Returns 1 if passed, 0 if failed or rec==NULL.
 */
int tst_compare(const char *name,
                const REGS16 far *after,
                const TC_RECORD *rec);

/*
 * tst_compare_buf(name, ptr, len, rec)
 *   Compare ptr[0..len-1] against rec->out_buf[0..rec->out_len-1].
 *   Both NULL/0 -> PASS (neither side recorded a buffer).
 *   One side NULL/0 and the other not -> FAIL.
 *   Prints PASS_BUF/FAIL_BUF to stdout and g_tst_fp.
 *   Returns 1 if passed, 0 if failed.
 */
int tst_compare_buf(const char *name,
                    const unsigned char far *ptr,
                    unsigned short len,
                    const TC_RECORD *rec);

/* -----------------------------------------------------------------------
 * All-in-one per-case runner
 * --------------------------------------------------------------------- */

/*
 * run_case(name, p, care_mask, fn)
 *   1. Calls probe_int2f(p) to issue INT 2Fh and capture p->after.
 *   2. If fn != NULL, calls fn(name, p, care_mask).
 *        - fn may update p->buffer / p->bufsize (or any other PROBE field)
 *          using information from p->after (e.g. ES:DI returned by DOS).
 *        - If fn returns non-zero the case is treated as failed and
 *          run_case() returns 0 immediately without writing to .ref or
 *          comparing against .ref.
 *   3. Reference mode: print verbose output, write register line to .ref,
 *      and (if p->buffer != NULL && p->bufsize > 0) write buffer line.
 *   4. Test mode: compare registers and (if CARE_BUF set) buffer against
 *      the loaded TC_RECORD for name.
 *
 *   Pass fn=NULL when no post-probe hook is needed.
 *
 *   Returns 1 if passed (or reference mode with no hook failure), 0 if failed.
 */
int run_case(const char *name,
             PROBE *p,
             unsigned short care_mask,
             fn_run_hook *fn);

/* -----------------------------------------------------------------------
 * Verbose stdout output helpers  (used in reference mode)
 * --------------------------------------------------------------------- */

void print_section(const char *label);
void print_call(unsigned short ax, unsigned short value, const char *name);
void print_regs(const REGS16 far *before, const REGS16 far *after);
void print_flags(unsigned short before, unsigned short after);
void print_mem_dump(unsigned char far *ptr, unsigned short len, signed short max_dump_len);
void print_end(void);
void print_probe_verbose(const PROBE *p, const char *name, signed short max_dump_len);

/* -----------------------------------------------------------------------
 * Utilities
 * --------------------------------------------------------------------- */

/*
 * build_test_string(buf, len)
 *   Fill buf[0..len-1] with a repeating printable pattern; NUL-terminate.
 *   buf must be at least len+1 bytes.
 */
void build_test_string(char far *buf, unsigned short len);

/*
 * get_dos_data_segment()
 *   Return the DOS internal data segment via INT 2F/AX=1203h.
 */
unsigned short get_dos_data_segment(void);

/*
 * get_dos_stack_offset(void)
 *   Return a likely offset within DOS data segment for use as valid stack pointer.
 */
unsigned short get_dos_stack_offset(void);

#endif /* TESTLIB_H */
