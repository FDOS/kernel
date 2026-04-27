/*
 * t1225.c  --  INT 2Fh / AX=1225h  GET LENGTH OF ASCIZ STRING
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f1225.html):
 *   AX = 1225h
 *   DS:SI -> ASCIZ string
 *   Return: CX = length of string (not counting NUL terminator)
 *
 * All other registers (AX, BX, DX, DI, DS, ES, FLAGS) are don't-cares
 * for pass/fail purposes; they are recorded in the .ref file but marked
 * "????" so they do not affect test results on other DOS versions.
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 1225.ref, verbose stdout
 *   default / test  TEST:      read 1225.ref, run all cases, write 1225.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t1225.c ..\testlib.c
 *
 * Usage:
 *   T1225 ref      (reference mode, writes 1225.ref)
 *   T1225 test     (test mode, reads 1225.ref, writes 1225.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "1225";

/* ----
 * We only care about CX on return (the string length).
 * All other output registers are don't-cares.
 * ---- */
#define CARE_1225   (CARE_CX)

/* ----
 * Maximum string length we will test.
 * DOS path buffers are 260 bytes; we go a bit beyond to probe limits.
 * 513 bytes = 512-char string + NUL.
 * ---- */
#define MAX_STR_LEN  512
#define BUF_SIZE     (MAX_STR_LEN + 1)

/* Static buffer -- avoids stack overflow in small model */
static char g_strbuf[BUF_SIZE];

/* ----
 * Helper: set up a PROBE for AX=1225h with DS:SI -> string s
 * ---- */
static void setup_1225(PROBE *p, unsigned char far *s, unsigned short dumplen)
{
    memset(p, 0, sizeof(*p));
    p->before.ax    = 0x1225;
    p->before.ds    = FP_SEG(s);
    p->before.si    = FP_OFF(s);
    /* All other input regs zero -- they are not used by this subfunction */
    p->buffer     = s;
    p->bufsize     = dumplen;
}

/* ----
 * Helper: run one length-based test case
 * ---- */
static int run_len_case(unsigned short len)
{
    PROBE p;
    char  name[TC_NAME_MAX];

    build_test_string(g_strbuf, len);
    /* Dump at most 32 bytes of the string for the verbose log */
    setup_1225(&p, g_strbuf, (unsigned short)(_fstrlen(g_strbuf) < 32 ? _fstrlen(g_strbuf) + 1 : 32));

    sprintf(name, "1225_len_%u", (unsigned)len);

    /* In verbose/reference mode, print the input string length */
    if (g_ref_mode) {
        printf("INPUT: len=%u  str=\"", (unsigned)len);
        if (len <= 32)
            printf("%s", g_strbuf);
        else
            printf("%.32s...", g_strbuf);
        printf("\"\n");
    }

    return run_case(name, &p, CARE_1225, NULL);
}

/* ----
 * Helper: run one literal string test case
 * ---- */
static int run_lit_case(const char *label, const char *lit)
{
    PROBE p;
    char  name[TC_NAME_MAX];

    /* Copy literal into g_strbuf so it is in a known near buffer */
    strncpy(g_strbuf, lit, BUF_SIZE - 1);
    g_strbuf[BUF_SIZE - 1] = '\0';

    setup_1225(&p, g_strbuf, (unsigned short)(_fstrlen(g_strbuf) < 32 ? _fstrlen(g_strbuf) + 1 : 32));

    sprintf(name, "1225_%s", label);

    if (g_ref_mode) {
        printf("INPUT: literal=\"%s\"\n", g_strbuf);
    }

    return run_case(name, &p, CARE_1225, NULL);
}

/* ----
 * Test suites
 * ---- */

/*
 * Required lengths from the spec:
 *   0, 1, 2, 3, 64, 128, 256, 260, 512
 */
TEST test_required_lengths(void)
{
    ASSERT(run_len_case(0));
    ASSERT(run_len_case(1));
    ASSERT(run_len_case(2));
    ASSERT(run_len_case(3));
    ASSERT(run_len_case(64));
    ASSERT(run_len_case(128));
    ASSERT(run_len_case(256));
    ASSERT(run_len_case(260));
    ASSERT(run_len_case(512));
    PASS();
}

/*
 * Boundary lengths: one below and one above each power-of-two and
 * DOS-significant boundary.
 */
TEST test_boundary_lengths(void)
{
    ASSERT(run_len_case(4));    /* just above 3           */
    ASSERT(run_len_case(7));    /* 8 - 1                  */
    ASSERT(run_len_case(8));
    ASSERT(run_len_case(9));    /* 8 + 1                  */
    ASSERT(run_len_case(63));   /* 64 - 1                 */
    ASSERT(run_len_case(65));   /* 64 + 1                 */
    ASSERT(run_len_case(127));  /* 128 - 1                */
    ASSERT(run_len_case(129));  /* 128 + 1                */
    ASSERT(run_len_case(255));  /* 256 - 1                */
    ASSERT(run_len_case(257));  /* 256 + 1                */
    ASSERT(run_len_case(259));  /* MAX_PATH - 1           */
    ASSERT(run_len_case(261));  /* MAX_PATH + 1           */
    ASSERT(run_len_case(511));  /* 512 - 1                */
    PASS();
}

/*
 * Literal corner-case strings that exercise specific DOS path semantics.
 */
TEST test_literal_cases(void)
{
    /* Empty string: length must be 0 */
    ASSERT(run_lit_case("empty",           ""));

    /* Single characters */
    ASSERT(run_lit_case("dot",             "."));
    ASSERT(run_lit_case("dotdot",          ".."));
    ASSERT(run_lit_case("backslash",       "\\"));
    ASSERT(run_lit_case("fwdslash",        "/"));
    ASSERT(run_lit_case("colon",           ":"));
    ASSERT(run_lit_case("space",           " "));
    ASSERT(run_lit_case("nul_like_ctrl",   "\x01"));    /* non-NUL ctrl char */

    /* Drive-letter forms */
    ASSERT(run_lit_case("drive_only",      "C:"));
    ASSERT(run_lit_case("drive_root",      "C:\\"));
    ASSERT(run_lit_case("drive_rel",       "C:FOO"));

    /* Path forms */
    ASSERT(run_lit_case("root_only",       "\\"));
    ASSERT(run_lit_case("trailing_bslash", "ABC\\"));
    ASSERT(run_lit_case("mixed_slashes",   "A/B\\C/D"));
    ASSERT(run_lit_case("deep_path",       "A\\B\\C\\D\\E\\F\\G"));
    ASSERT(run_lit_case("spaces_inside",   "A B C D"));
    ASSERT(run_lit_case("three_spaces",    "   "));

    /* Wildcard characters (not path separators, but common in DOS) */
    ASSERT(run_lit_case("wildcard_star",   "*.*"));
    ASSERT(run_lit_case("wildcard_q",      "?.?"));

    /* 8.3 filename */
    ASSERT(run_lit_case("filename_83",     "FILENAME.EXT"));

    /* All printable ASCII (length 94) */
    ASSERT(run_lit_case("printable_ascii",
        " !\"#$%&'()*+,-./0123456789:;<=>?@"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"
        "abcdefghijklmnopqrstuvwxyz{|}~"));

    /* High-byte characters (DOS codepage chars, not NUL) */
    ASSERT(run_lit_case("high_byte_80",    "\x80"));
    ASSERT(run_lit_case("high_byte_fe",    "\xFE"));
    ASSERT(run_lit_case("high_byte_ff",    "\xFF"));

    PASS();
}

/*
 * Alignment corner cases: string starting at various offsets within
 * g_strbuf to catch any word-alignment assumptions in the DOS kernel.
 */
TEST test_alignment_cases(void)
{
    PROBE        p;
    char         name[TC_NAME_MAX];
    unsigned int offset;

    /* Test offsets 0..7 with a 16-char string */
    for (offset = 0; offset <= 7; offset++) {
        char far *s;

        build_test_string(g_strbuf, 16);
        s = (char far *)(g_strbuf + offset);

        setup_1225(&p, s, 17);

        sprintf(name, "1225_align_off%u", offset);

        if (g_ref_mode)
            printf("INPUT: offset=%u  str=\"%.16s\"\n", offset, s);

        ASSERT(run_case(name, &p, CARE_1225, NULL));
    }

    PASS();
}

/*
 * Repeated-NUL / embedded-NUL: the function should stop at the FIRST NUL.
 * We manually construct strings with a NUL in the middle.
 * Expected CX = position of first NUL.
 */
TEST test_embedded_nul(void)
{
    PROBE p;

    /* "ABC\0DEF" -- length should be 3, not 7 */
    memcpy(g_strbuf, "ABC\x00" "DEF", 8);

    setup_1225(&p, g_strbuf, 8);

    if (g_ref_mode)
        printf("INPUT: \"ABC<NUL>DEF\" (embedded NUL at offset 3)\n");

    ASSERT(run_case("1225_embedded_nul", &p, CARE_1225, NULL));

    PASS();
}

/* ----
 * Suite
 * ---- */

SUITE(suite)
{
    RUN_TEST(test_required_lengths);
    RUN_TEST(test_boundary_lengths);
    RUN_TEST(test_literal_cases);
    RUN_TEST(test_alignment_cases);
    RUN_TEST(test_embedded_nul);
}
