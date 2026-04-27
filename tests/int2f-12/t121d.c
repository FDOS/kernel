/*
 * t121d.c  --  INT 2Fh / AX=121Dh  SUM MEMORY BLOCK (8-bit truncated)
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f121d.html):
 *   AX = 121Dh
 *   DS:SI -> start of buffer
 *   CX    = byte count (0 = no bytes processed)
 *   Return: AL = 8-bit truncated running sum (carry discarded each step)
 *           SI = original SI + CX  (advanced past the buffer)
 *           CX = 0
 *           All other registers (AH, BX, DX, DI, DS, ES, FLAGS) are don't-cares.
 *
 * Algorithm (8-bit truncated sum):
 *   AL = 0
 *   For each byte B at DS:SI:
 *     AL = (AL + B) & 0xFF    (8-bit addition, carry discarded)
 *     SI++; CX--
 *   Repeat until CX == 0.
 *
 * This differs from AX=121Ch which uses a 16-bit carry-folded (ones-complement)
 * accumulation.  121Dh has no seed register; the accumulator always starts at 0.
 *
 * Care registers:
 *   AL    -- 8-bit sum result  (CARE_AL)
 *   SI    -- advanced by CX   (CARE_SI)
 *   CX    -- must be 0        (CARE_CX)
 *   All other output registers are don't-cares.
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 121d.ref, verbose stdout
 *   default / test  TEST:      read 121d.ref, run all cases, write 121d.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t121d.c ..\testlib.c
 *
 * Usage:
 *   T121D ref      (reference mode, writes 121d.ref)
 *   T121D test     (test mode, reads 121d.ref, writes 121d.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "121d";

/* ----
 * We care about AL (8-bit sum), SI (advanced pointer), and CX (must be 0).
 * All other output registers are don't-cares.
 * ---- */
#define CARE_121D   (CARE_AL | CARE_SI | CARE_CX)

/* ----
 * Maximum buffer size for tests.
 * ---- */
#define MAX_BUF  512
#define BUF_SIZE (MAX_BUF + 2)   /* +2: sentinel bytes */

/* Static buffer -- avoids stack overflow in small model */
static unsigned char g_buf[BUF_SIZE];

/* ----
 * Helper: set up a PROBE for AX=121Dh.
 *   DS:SI -> buf (start of data)
 *   CX    = count
 *   (no seed register -- accumulator always starts at 0 inside DOS)
 * ---- */
static void setup_121d(PROBE *p,
                        const unsigned char *buf,
                        unsigned short count)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x121D;
    p->before.ds = FP_SEG(buf);
    p->before.si = FP_OFF(buf);
    p->before.cx = count;
    /* buffer tracking: record the input bytes for verbose log only */
    p->buffer  = (unsigned char far *)buf;
    p->bufsize = (count < 32) ? count : 32;
}

/* ----
 * Core helper: run one sum test case.
 * ---- */
static int run_sum_case(const char *name,
                         const unsigned char *buf,
                         unsigned short count)
{
    PROBE p;

    setup_121d(&p, buf, count);

    if (g_ref_mode) {
        printf("INPUT: count=%u  buf[0..%u]:",
               count, (count < 8 ? count : 8) - 1);
        {
            unsigned short i;
            unsigned short show = (count < 8) ? count : 8;
            for (i = 0; i < show; i++)
                printf(" %02X", buf[i]);
            if (count > 8) printf(" ...");
        }
        printf("\n");
    }

    return run_case(name, &p, CARE_121D, NULL);
}

/* ---- */
/* Test suites                                                            */
/* ---- */

/*
 * Zero-length buffer: CX=0 on entry.
 * AL must be 0 (no bytes processed), SI unchanged, CX=0.
 */
TEST test_zero_length(void)
{
    ASSERT(run_sum_case("121d_zero", g_buf, 0));
    PASS();
}

/*
 * Single-byte buffers: verifies the per-byte truncation.
 */
TEST test_single_byte(void)
{
    /* 0x00: AL = 0 */
    g_buf[0] = 0x00;
    ASSERT(run_sum_case("121d_1byte_00", g_buf, 1));

    /* 0x01: AL = 1 */
    g_buf[0] = 0x01;
    ASSERT(run_sum_case("121d_1byte_01", g_buf, 1));

    /* 0x7F: AL = 0x7F */
    g_buf[0] = 0x7F;
    ASSERT(run_sum_case("121d_1byte_7F", g_buf, 1));

    /* 0x80: AL = 0x80 */
    g_buf[0] = 0x80;
    ASSERT(run_sum_case("121d_1byte_80", g_buf, 1));

    /* 0xFF: AL = 0xFF */
    g_buf[0] = 0xFF;
    ASSERT(run_sum_case("121d_1byte_FF", g_buf, 1));

    PASS();
}

/*
 * Two-byte buffers: verifies carry is discarded (8-bit truncation).
 * 0xFF + 0x01 = 0x100 -> AL = 0x00 (carry discarded).
 * 0xFF + 0x02 = 0x101 -> AL = 0x01.
 */
TEST test_two_byte_overflow(void)
{
    /* 0xFF + 0x01 -> 0x00 (overflow, carry discarded) */
    g_buf[0] = 0xFF; g_buf[1] = 0x01;
    ASSERT(run_sum_case("121d_2byte_FF_01", g_buf, 2));

    /* 0xFF + 0x02 -> 0x01 */
    g_buf[0] = 0xFF; g_buf[1] = 0x02;
    ASSERT(run_sum_case("121d_2byte_FF_02", g_buf, 2));

    /* 0xFF + 0xFF -> 0xFE (0x1FE & 0xFF) */
    g_buf[0] = 0xFF; g_buf[1] = 0xFF;
    ASSERT(run_sum_case("121d_2byte_FF_FF", g_buf, 2));

    /* 0x80 + 0x80 -> 0x00 */
    g_buf[0] = 0x80; g_buf[1] = 0x80;
    ASSERT(run_sum_case("121d_2byte_80_80", g_buf, 2));

    /* 0x00 + 0x00 -> 0x00 */
    g_buf[0] = 0x00; g_buf[1] = 0x00;
    ASSERT(run_sum_case("121d_2byte_00_00", g_buf, 2));

    PASS();
}

/*
 * All-zero buffer: sum must be 0 for any length.
 */
TEST test_all_zeros(void)
{
    memset(g_buf, 0x00, MAX_BUF);

    ASSERT(run_sum_case("121d_zeros_1",    g_buf,   1));
    ASSERT(run_sum_case("121d_zeros_2",    g_buf,   2));
    ASSERT(run_sum_case("121d_zeros_16",   g_buf,  16));
    ASSERT(run_sum_case("121d_zeros_256",  g_buf, 256));
    ASSERT(run_sum_case("121d_zeros_512",  g_buf, 512));
    PASS();
}

/*
 * All-0xFF buffer: every byte addition overflows.
 * For N bytes of 0xFF: AL = (N * 0xFF) & 0xFF = (0xFF * N) mod 256.
 * N=1: 0xFF; N=2: 0xFE; N=256: 0x00; N=257: 0xFF; etc.
 */
TEST test_all_ff(void)
{
    memset(g_buf, 0xFF, MAX_BUF);

    ASSERT(run_sum_case("121d_ff_1",    g_buf,   1));
    ASSERT(run_sum_case("121d_ff_2",    g_buf,   2));
    ASSERT(run_sum_case("121d_ff_3",    g_buf,   3));
    ASSERT(run_sum_case("121d_ff_4",    g_buf,   4));
    ASSERT(run_sum_case("121d_ff_16",   g_buf,  16));
    ASSERT(run_sum_case("121d_ff_255",  g_buf, 255));
    ASSERT(run_sum_case("121d_ff_256",  g_buf, 256));
    ASSERT(run_sum_case("121d_ff_257",  g_buf, 257));
    ASSERT(run_sum_case("121d_ff_512",  g_buf, 512));
    PASS();
}

/*
 * Incrementing pattern 0x00..0xFF (repeating).
 * Sum of 0..255 = 0x7F80; truncated to 8 bits = 0x80.
 * Sum of 0..255 twice = 0xFF00; truncated = 0x00.
 */
TEST test_incrementing(void)
{
    unsigned short i;
    for (i = 0; i < MAX_BUF; i++)
        g_buf[i] = (unsigned char)(i & 0xFF);

    ASSERT(run_sum_case("121d_incr_1",    g_buf,   1));
    ASSERT(run_sum_case("121d_incr_2",    g_buf,   2));
    ASSERT(run_sum_case("121d_incr_4",    g_buf,   4));
    ASSERT(run_sum_case("121d_incr_16",   g_buf,  16));
    ASSERT(run_sum_case("121d_incr_64",   g_buf,  64));
    ASSERT(run_sum_case("121d_incr_128",  g_buf, 128));
    ASSERT(run_sum_case("121d_incr_256",  g_buf, 256));
    ASSERT(run_sum_case("121d_incr_512",  g_buf, 512));
    PASS();
}

/*
 * Alternating 0x55/0xAA pattern.
 * Pair sum: 0x55 + 0xAA = 0xFF.
 * 2 bytes: 0xFF; 4 bytes: 0xFE; 256 bytes: 0x80 (128 * 0xFF mod 256).
 */
TEST test_alternating(void)
{
    unsigned short i;
    for (i = 0; i < MAX_BUF; i++)
        g_buf[i] = (i & 1) ? 0xAA : 0x55;

    ASSERT(run_sum_case("121d_alt_1",    g_buf,   1));
    ASSERT(run_sum_case("121d_alt_2",    g_buf,   2));
    ASSERT(run_sum_case("121d_alt_4",    g_buf,   4));
    ASSERT(run_sum_case("121d_alt_16",   g_buf,  16));
    ASSERT(run_sum_case("121d_alt_256",  g_buf, 256));
    ASSERT(run_sum_case("121d_alt_512",  g_buf, 512));
    PASS();
}

/*
 * Boundary lengths: one below and one above each power-of-two.
 */
TEST test_boundary_lengths(void)
{
    unsigned short i;
    for (i = 0; i < MAX_BUF; i++)
        g_buf[i] = (unsigned char)(i * 3 + 7);  /* pseudo-random pattern */

    ASSERT(run_sum_case("121d_bnd_3",    g_buf,   3));
    ASSERT(run_sum_case("121d_bnd_7",    g_buf,   7));
    ASSERT(run_sum_case("121d_bnd_8",    g_buf,   8));
    ASSERT(run_sum_case("121d_bnd_9",    g_buf,   9));
    ASSERT(run_sum_case("121d_bnd_15",   g_buf,  15));
    ASSERT(run_sum_case("121d_bnd_17",   g_buf,  17));
    ASSERT(run_sum_case("121d_bnd_31",   g_buf,  31));
    ASSERT(run_sum_case("121d_bnd_33",   g_buf,  33));
    ASSERT(run_sum_case("121d_bnd_63",   g_buf,  63));
    ASSERT(run_sum_case("121d_bnd_65",   g_buf,  65));
    ASSERT(run_sum_case("121d_bnd_127",  g_buf, 127));
    ASSERT(run_sum_case("121d_bnd_129",  g_buf, 129));
    ASSERT(run_sum_case("121d_bnd_255",  g_buf, 255));
    ASSERT(run_sum_case("121d_bnd_257",  g_buf, 257));
    ASSERT(run_sum_case("121d_bnd_511",  g_buf, 511));
    PASS();
}

/*
 * Alignment: buffer starting at various byte offsets within g_buf.
 * Catches any word-alignment assumptions in the DOS kernel.
 */
TEST test_alignment(void)
{
    unsigned short i, off;
    char name[TC_NAME_MAX];

    for (i = 0; i < MAX_BUF; i++)
        g_buf[i] = (unsigned char)(i + 0x41);

    for (off = 0; off <= 7; off++) {
        sprintf(name, "121d_align_off%u", off);
        ASSERT(run_sum_case(name, g_buf + off, 16));
    }

    PASS();
}

/*
 * Difference from 121Ch: confirm 121Dh does NOT carry-fold.
 * 0xFF + 0x01 = 0x100 -> 121Dh gives AL=0x00 (carry discarded).
 * (121Ch would give DX=0x0001 because it folds the carry back in.)
 * We record whatever DOS returns; the .ref file captures the truth.
 */
TEST test_no_carry_fold(void)
{
    /* Three bytes that would produce different results with carry-fold */
    g_buf[0] = 0xFF;
    g_buf[1] = 0x01;
    g_buf[2] = 0x00;
    ASSERT(run_sum_case("121d_nocarry_FF_01_00", g_buf, 3));

    g_buf[0] = 0xFF;
    g_buf[1] = 0xFF;
    g_buf[2] = 0x02;
    ASSERT(run_sum_case("121d_nocarry_FF_FF_02", g_buf, 3));

    /* 256 bytes of 0x01: 8-bit sum = 0x00 (256 mod 256); carry-fold would give 0x01 */
    memset(g_buf, 0x01, 256);
    ASSERT(run_sum_case("121d_nocarry_256x01", g_buf, 256));

    PASS();
}

/* ---- */
/* Suite                                                                  */
/* ---- */

SUITE(suite)
{
    RUN_TEST(test_zero_length);
    RUN_TEST(test_single_byte);
    RUN_TEST(test_two_byte_overflow);
    RUN_TEST(test_all_zeros);
    RUN_TEST(test_all_ff);
    RUN_TEST(test_incrementing);
    RUN_TEST(test_alternating);
    RUN_TEST(test_boundary_lengths);
    RUN_TEST(test_alignment);
    RUN_TEST(test_no_carry_fold);
}
