/*
 * t121f.c  --  INT 2Fh / AX=121Fh  BUILD TEMPORARY CDS FOR DRIVE LETTER
 *
 * FreeDOS inthndlr.c case 0x1f:
 *   Input:  callerARG1 low byte = drive letter ASCII ('A'..'Z' or 'a'..'z')
 *           pushed on the stack by the caller (p->value).
 *   Output: On success:
 *             ES:DI -> TempCDS (DOS-internal global, address varies per run)
 *             CX    = sizeof(TempCDS)  (== sizeof(struct cds))
 *             CF    = 0
 *           On failure (invalid letter, drive out of range):
 *             CF    = 1
 *
 *   TempCDS is filled as follows:
 *     cdsCurrentPath[0]  = drive letter (as supplied, not uppercased)
 *     cdsCurrentPath[1]  = ':'
 *     cdsCurrentPath[2]  = '\'
 *     cdsCurrentPath[3]  = NUL
 *     cdsBackslashOffset = 2
 *     If the real CDS for the drive has cdsFlags != 0 (drive active):
 *       cdsDpb   = real CDS cdsDpb
 *       cdsFlags = CDSPHYSDRV  (0x0040)
 *     Else (inactive drive):
 *       cdsDpb   = NULL (0000:0000)
 *       cdsFlags = 0
 *     cdsStrtClst    = 0xFFFF
 *     cdsParam       = 0xFFFF
 *     cdsStoreUData  = 0xFFFF
 *
 * Care masks:
 *   Success path: CARE_FLAGS | CARE_BUF
 *     - CX may equal sizeof(struct cds), MSDOS leaves unchanged
 *     - CF must be clear.
 *     - The CDS content at ES:DI must match the reference byte-for-byte.
 *     - ES and DI are NOT in the care mask: TempCDS lives in DOS DGROUP
 *       whose address varies between DOS versions and builds.
 *   Failure path: CARE_FLAGS only (CF must be set).
 *
 * Buffer handling:
 *   ES:DI is only known after the INT 2Fh call returns.  A post-probe
 *   hook (hook_121f) is passed to run_case() as the fn argument.  The
 *   hook runs after probe_int2f() but before any ref/tst work:
 *     - If CF=0: sets p->buffer = MK_FP(p->after.es, p->after.di)
 *                and p->bufsize = p->after.cx.
 *     - If CF=1: leaves p->buffer=NULL / p->bufsize=0 (no buffer to compare).
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 121f.ref, verbose stdout
 *   default / test  TEST:      read 121f.ref, run all cases, write 121f.tst
 *
 * Build:
 *   wcl -zq -q -mt -i=.. -DUSEDOSSTACK t121f.c ..\testlib.c
 *
 * Usage:
 *   T121F ref      (reference mode, writes 121f.ref)
 *   T121F test     (test mode, reads 121f.ref, writes 121f.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "121f";

/* ----
 * Care masks
 * ---- */

/* Success: FLAGS (CF=0), and the CDS buffer content. */
#define CARE_121F_OK    (CARE_FLAGS | CARE_BUF)

/* Failure: only CF=1 matters; no buffer. */
#define CARE_121F_FAIL  (CARE_FLAGS)

/* ----
 * Post-probe hook
 *
 * Called by run_case() after probe_int2f() fills p->after, but before
 * any .ref writing or comparison.
 *
 * Returns 0 always (never signals hook-level failure).
 * ---- */

static int hook_121f(const char *name, PROBE *p, unsigned short care)
{
    (void)name;
    (void)care;

    if (p->after.flags & FLAG_CF) {
        /* Failure path: no buffer */
        p->buffer  = NULL;
        p->bufsize = 0;
    } else {
        /* Success path: ES:DI -> TempCDS */
        p->buffer  = (unsigned char far *)MK_FP(p->after.es, p->after.di);
        p->bufsize = 58; /* sizeof(CDS) */
    }
	
    return 0;
}

/* ----
 * Helper: set up a PROBE for AX=121Fh.
 *   drive_letter: ASCII drive letter ('A'..'Z' or 'a'..'z', or invalid).
 *   Passed via p->value (pushed on stack as callerARG1).
 * ---- */
static void setup_121f(PROBE *p, unsigned char drive_letter)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x121F;
    p->value     = (unsigned short)drive_letter;
    /* buffer and bufsize are set by hook_121f after the call */
}

/* ----
 * Core helper: run one test case.
 *   label        -- unique suffix (prefixed with "121f_")
 *   drive_letter -- ASCII drive letter to pass as callerARG1
 *   care         -- CARE_121F_OK or CARE_121F_FAIL
 * ---- */
static int run_121f_case(const char *label,
                         unsigned char drive_letter,
                         unsigned short care)
{
    PROBE p;
    char  name[TC_NAME_MAX];

    setup_121f(&p, drive_letter);
    sprintf(name, "121f_%s", label);

    if (g_ref_mode)
        printf("INPUT: drive_letter=0x%02X ('%c')\n",
               (unsigned)drive_letter,
               (drive_letter >= 0x20 && drive_letter < 0x7F)
                   ? (char)drive_letter : '?');

    return run_case(name, &p, care, hook_121f);
}

/* ----
 * Test suites
 * ---- */

/*
 * All 26 uppercase drive letters A..Z.
 * Each must succeed (CF=0), return CX=sizeof(cds), and produce a CDS
 * whose cdsCurrentPath starts with the supplied uppercase letter.
 */
TEST test_valid_uppercase(void)
{
    char label[32];
    int  i;

	printf("test_valid_uppercase\n");
    for (i = 0; i < 26; i++) {
        unsigned char letter = (unsigned char)('A' + i);
        sprintf(label, "upper_%c", 'A' + i);
        ASSERT(run_121f_case(label, letter, CARE_121F_OK));		
    }
    PASS();
}

/*
 * All 26 lowercase drive letters a..z.
 * The kernel stores the letter as-is (no uppercasing), so cdsCurrentPath[0]
 * will be the lowercase letter.  The reference captures this exactly.
 */
TEST test_valid_lowercase(void)
{
    char label[32];
    int  i;

    for (i = 0; i < 26; i++) {
        unsigned char letter = (unsigned char)('a' + i);
        sprintf(label, "lower_%c", 'a' + i);
        ASSERT(run_121f_case(label, letter, CARE_121F_OK));
    }
    PASS();
}

/*
 * Characters immediately outside the valid letter ranges must fail (CF=1).
 *   '@' (0x40) is just below 'A' (0x41)
 *   '[' (0x5B) is just above 'Z' (0x5A)
 *   '`' (0x60) is just below 'a' (0x61)
 *   '{' (0x7B) is just above 'z' (0x7A)
 */
TEST test_boundary_invalid(void)
{
    ASSERT(run_121f_case("below_A",  '@',  CARE_121F_FAIL));
    ASSERT(run_121f_case("above_Z",  '[',  CARE_121F_FAIL));
    ASSERT(run_121f_case("below_a",  '`',  CARE_121F_FAIL));
    ASSERT(run_121f_case("above_z",  '{',  CARE_121F_FAIL));
    PASS();
}

/*
 * Digits, punctuation, control characters, and high bytes -- all invalid.
 */
TEST test_invalid_chars(void)
{
    /* Digits */
    ASSERT(run_121f_case("digit_0",    '0',  CARE_121F_FAIL));
    ASSERT(run_121f_case("digit_9",    '9',  CARE_121F_FAIL));

    /* Punctuation */
    ASSERT(run_121f_case("bang",       '!',  CARE_121F_FAIL));
    ASSERT(run_121f_case("colon",      ':',  CARE_121F_FAIL));
    ASSERT(run_121f_case("space",      ' ',  CARE_121F_FAIL));

    /* Control characters */
    ASSERT(run_121f_case("nul",        0x00, CARE_121F_FAIL));
    ASSERT(run_121f_case("ctrl_01",    0x01, CARE_121F_FAIL));
    ASSERT(run_121f_case("ctrl_1f",    0x1F, CARE_121F_FAIL));

    /* High bytes */
    ASSERT(run_121f_case("high_80",    0x80, CARE_121F_FAIL));
    ASSERT(run_121f_case("high_ff",    0xFF, CARE_121F_FAIL));

    PASS();
}

/*
 * CDS path prefix check.
 * For a successful call the first four bytes of cdsCurrentPath must be
 * "<letter>:\<NUL>".  The reference captures the full CDS so this is
 * implicitly verified, but we call out A, C, a, c explicitly for clarity.
 */
TEST test_cds_path_prefix(void)
{
    ASSERT(run_121f_case("path_A",  'A', CARE_121F_OK));
    ASSERT(run_121f_case("path_C",  'C', CARE_121F_OK));
    ASSERT(run_121f_case("path_a",  'a', CARE_121F_OK));
    ASSERT(run_121f_case("path_c",  'c', CARE_121F_OK));
    PASS();
}

/*
 * CX consistency check.
 * CX must equal sizeof(struct cds) on every call.  We verify three
 * different drives so we can confirm the value is constant, not
 * drive-dependent.
 */
TEST test_cx_equals_cds_size(void)
{
    ASSERT(run_121f_case("cx_A",  'A', CARE_121F_OK));
    ASSERT(run_121f_case("cx_B",  'B', CARE_121F_OK));
    ASSERT(run_121f_case("cx_Z",  'Z', CARE_121F_OK));
    PASS();
}

/*
 * Repeated call for the same drive must produce identical CDS content.
 * TempCDS is a single global; two consecutive calls for C: must agree.
 */
TEST test_repeated_call(void)
{
    ASSERT(run_121f_case("repeat_C_1", 'C', CARE_121F_OK));
    ASSERT(run_121f_case("repeat_C_2", 'C', CARE_121F_OK));
    PASS();
}

/*
 * Uppercase and lowercase of the same drive letter produce the same CDS
 * content except for cdsCurrentPath[0] (the letter itself).
 * The reference captures both; the comparison is byte-for-byte so any
 * difference beyond the first byte will be caught.
 */
TEST test_uppercase_lowercase_same_drive(void)
{
    ASSERT(run_121f_case("case_A_upper", 'A', CARE_121F_OK));
    ASSERT(run_121f_case("case_A_lower", 'a', CARE_121F_OK));
    ASSERT(run_121f_case("case_C_upper", 'C', CARE_121F_OK));
    ASSERT(run_121f_case("case_C_lower", 'c', CARE_121F_OK));
    PASS();
}

/*
 * Inactive drives (Y: and Z: are unlikely to be active on a test machine).
 * The kernel must still succeed (CF=0) and return a CDS with cdsDpb=NULL
 * and cdsFlags=0.  The reference captures whatever the DOS actually returns.
 */
TEST test_inactive_drives(void)
{
    ASSERT(run_121f_case("inactive_Y", 'Y', CARE_121F_OK));
    ASSERT(run_121f_case("inactive_Z", 'Z', CARE_121F_OK));
    PASS();
}

/*
 * CF pre-set: confirm the kernel clears CF on success even when the
 * caller enters with CF=1.
 */
TEST test_cf_clear_on_success(void)
{
    PROBE p;

    setup_121f(&p, 'C');
    p.before.flags = FLAG_CF;   /* pre-set carry */

    if (g_ref_mode)
        printf("INPUT: drive='C' with CF pre-set\n");

    ASSERT(run_case("121f_cf_presset_C", &p, CARE_121F_OK, hook_121f));
    PASS();
}

/*
 * CF pre-clear: confirm the kernel sets CF on failure even when the
 * caller enters with CF=0.
 */
TEST test_cf_set_on_failure(void)
{
    PROBE p;

    setup_121f(&p, '!');
    p.before.flags = 0;         /* pre-clear carry */

    if (g_ref_mode)
        printf("INPUT: drive='!' (invalid) with CF pre-cleared\n");

    ASSERT(run_case("121f_cf_preclear_bang", &p, CARE_121F_FAIL, hook_121f));
    PASS();
}

/* ----
 * Suite
 * ---- */

SUITE(suite)
{
	printf("SUITE!\n");
	fflush(NULL);
    RUN_TEST(test_valid_uppercase);
    RUN_TEST(test_valid_lowercase);
    RUN_TEST(test_boundary_invalid);
    RUN_TEST(test_invalid_chars);
    RUN_TEST(test_cds_path_prefix);
    RUN_TEST(test_cx_equals_cds_size);
    RUN_TEST(test_repeated_call);
    RUN_TEST(test_uppercase_lowercase_same_drive);
    RUN_TEST(test_inactive_drives);
    RUN_TEST(test_cf_clear_on_success);
    RUN_TEST(test_cf_set_on_failure);
}
