/*
 * t121c.c  --  INT 2Fh / AX=121Ch  CHECKSUM MEMORY BLOCK
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f121c.html):
 *   AX = 121Ch
 *   DS:SI -> start of buffer
 *   CX    = byte count (0 = no bytes processed)
 *   DX    = initial checksum (seed)
 *   Return: DX = 16-bit carry-folded running checksum
 *           SI = original SI + CX  (advanced past the buffer)
 *           CX = 0
 *           All other registers (AX, BX, DI, DS, ES, FLAGS) are don't-cares.
 *
 * Algorithm (16-bit carry-folded sum):
 *   For each byte B at DS:SI:
 *     DX += (unsigned 16-bit) B
 *     if carry: DX += 1          (fold carry back into sum)
 *     SI++; CX--
 *   Repeat until CX == 0.
 *
 * This is an end-around carry (ones-complement) accumulation, identical
 * to the IP/ICMP checksum inner loop.  It differs from AX=121Dh which
 * uses a plain 8-bit truncated sum with no carry fold.
 *
 * Care registers:
 *   DX    -- final checksum  (CARE_DX)
 *   SI    -- advanced by CX  (CARE_SI)
 *   CX    -- must be 0       (CARE_CX)
 *   All other output registers are don't-cares.
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 121c.ref, verbose stdout
 *   default / test  TEST:      read 121c.ref, run all cases, write 121c.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t121c.c ..\testlib.c
 *
 * Usage:
 *   T121C ref      (reference mode, writes 121c.ref)
 *   T121C test     (test mode, reads 121c.ref, writes 121c.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "121c";

/* ----
 * We care about DX (checksum), SI (advanced pointer), and CX (must be 0).
 * All other output registers are don't-cares.
 * ---- */
#define CARE_121C   (CARE_DX | CARE_SI | CARE_CX)

/* ----
 * Maximum buffer size for tests.
 * ---- */
#define MAX_BUF  512
#define BUF_SIZE (MAX_BUF + 2)   /* +2: sentinel bytes */

/* Static buffer -- avoids stack overflow in small model */
static unsigned char g_buf[BUF_SIZE];

/* ----
 * Helper: set up a PROBE for AX=121Ch.
 *   DS:SI -> buf (start of data)
 *   CX    = count
 *   DX    = initial checksum seed
 * ---- */
static void setup_121c(PROBE *p,
                        const unsigned char *buf,
                        unsigned short count,
                        unsigned short seed)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x121C;
    p->before.ds = FP_SEG(buf);
    p->before.si = FP_OFF(buf);
    p->before.cx = count;
    p->before.dx = seed;
    /* buffer tracking: record the input bytes for verbose log only */
    p->buffer  = (unsigned char far *)buf;
    p->bufsize = (count < 32) ? count : 32;
}

/* ----
 * Core helper: run one checksum test case.
 * ---- */
static int run_cksum_case(const char *name,
                           const unsigned char *buf,
                           unsigned short count,
                           unsigned short seed)
{
    PROBE p;

    setup_121c(&p, buf, count, seed);

    if (g_ref_mode) {
        printf("INPUT: count=%u  seed=0x%04X  buf[0..%u]:",
               count, seed, (count < 8 ? count : 8) - 1);
        {
            unsigned short i;
            unsigned short show = (count < 8) ? count : 8;
            for (i = 0; i < show; i++)
                printf(" %02X", buf[i]);
            if (count > 8) printf(" ...");
        }
        printf("\n");
    }

    return run_case(name, &p, CARE_121C, NULL);
}

/* ---- */
/* Test suites                                                            */
/* ---- */

/*
 * Zero-length buffer: CX=0 on entry.
 * DX must equal the seed (no bytes processed), SI unchanged, CX=0.
 */
TEST test_zero_length(void)
{
    /* seed=0 */
    ASSERT(run_cksum_case("121c_zero_seed0",    g_buf, 0, 0x0000));
    /* seed=non-zero */
    ASSERT(run_cksum_case("121c_zero_seed1234", g_buf, 0, 0x1234));
    ASSERT(run_cksum_case("121c_zero_seedFFFF", g_buf, 0, 0xFFFF));
    PASS();
}

/*
 * Single-byte buffers: verifies the per-byte carry-fold logic.
 */
TEST test_single_byte(void)
{
    /* 0x00: DX = seed + 0 */
    g_buf[0] = 0x00;
    ASSERT(run_cksum_case("121c_1byte_00_seed0",    g_buf, 1, 0x0000));
    ASSERT(run_cksum_case("121c_1byte_00_seedFFFF", g_buf, 1, 0xFFFF));

    /* 0x01 */
    g_buf[0] = 0x01;
    ASSERT(run_cksum_case("121c_1byte_01_seed0",    g_buf, 1, 0x0000));
    ASSERT(run_cksum_case("121c_1byte_01_seedFFFE", g_buf, 1, 0xFFFE));

    /* 0xFF: adding 0xFF to 0xFF00 causes carry */
    g_buf[0] = 0xFF;
    ASSERT(run_cksum_case("121c_1byte_FF_seed0",    g_buf, 1, 0x0000));
    ASSERT(run_cksum_case("121c_1byte_FF_seedFF00", g_buf, 1, 0xFF00));
    ASSERT(run_cksum_case("121c_1byte_FF_seedFFFF", g_buf, 1, 0xFFFF));

    /* 0x80 */
    g_buf[0] = 0x80;
    ASSERT(run_cksum_case("121c_1byte_80_seed0",    g_buf, 1, 0x0000));
    ASSERT(run_cksum_case("121c_1byte_80_seedFF80", g_buf, 1, 0xFF80));

    PASS();
}

/*
 * All-zero buffer: checksum must equal seed (adding zeros changes nothing).
 */
TEST test_all_zeros(void)
{
    memset(g_buf, 0x00, MAX_BUF);

    ASSERT(run_cksum_case("121c_zeros_1",    g_buf,   1, 0x0000));
    ASSERT(run_cksum_case("121c_zeros_2",    g_buf,   2, 0x0000));
    ASSERT(run_cksum_case("121c_zeros_16",   g_buf,  16, 0x0000));
    ASSERT(run_cksum_case("121c_zeros_256",  g_buf, 256, 0x0000));
    ASSERT(run_cksum_case("121c_zeros_512",  g_buf, 512, 0x0000));
    /* Non-zero seed with all-zero data: result must equal seed */
    ASSERT(run_cksum_case("121c_zeros_16_seed1234",  g_buf, 16, 0x1234));
    ASSERT(run_cksum_case("121c_zeros_256_seedABCD", g_buf, 256, 0xABCD));
    PASS();
}

/*
 * All-0xFF buffer: every byte addition causes a carry.
 * Verifies the carry-fold path is exercised on every iteration.
 */
TEST test_all_ff(void)
{
    memset(g_buf, 0xFF, MAX_BUF);

    ASSERT(run_cksum_case("121c_ff_1",    g_buf,   1, 0x0000));
    ASSERT(run_cksum_case("121c_ff_2",    g_buf,   2, 0x0000));
    ASSERT(run_cksum_case("121c_ff_3",    g_buf,   3, 0x0000));
    ASSERT(run_cksum_case("121c_ff_4",    g_buf,   4, 0x0000));
    ASSERT(run_cksum_case("121c_ff_16",   g_buf,  16, 0x0000));
    ASSERT(run_cksum_case("121c_ff_255",  g_buf, 255, 0x0000));
    ASSERT(run_cksum_case("121c_ff_256",  g_buf, 256, 0x0000));
    ASSERT(run_cksum_case("121c_ff_512",  g_buf, 512, 0x0000));
    /* With non-zero seed */
    ASSERT(run_cksum_case("121c_ff_16_seed1234",  g_buf, 16, 0x1234));
    PASS();
}

/*
 * Incrementing pattern 0x00..0xFF (repeating): known-value test.
 */
TEST test_incrementing(void)
{
    unsigned short i;
    for (i = 0; i < MAX_BUF; i++)
        g_buf[i] = (unsigned char)(i & 0xFF);

    ASSERT(run_cksum_case("121c_incr_1",    g_buf,   1, 0x0000));
    ASSERT(run_cksum_case("121c_incr_2",    g_buf,   2, 0x0000));
    ASSERT(run_cksum_case("121c_incr_4",    g_buf,   4, 0x0000));
    ASSERT(run_cksum_case("121c_incr_16",   g_buf,  16, 0x0000));
    ASSERT(run_cksum_case("121c_incr_64",   g_buf,  64, 0x0000));
    ASSERT(run_cksum_case("121c_incr_128",  g_buf, 128, 0x0000));
    ASSERT(run_cksum_case("121c_incr_256",  g_buf, 256, 0x0000));
    ASSERT(run_cksum_case("121c_incr_512",  g_buf, 512, 0x0000));
    /* With non-zero seed */
    ASSERT(run_cksum_case("121c_incr_256_seedABCD", g_buf, 256, 0xABCD));
    PASS();
}

/*
 * Alternating 0x55/0xAA pattern.
 */
TEST test_alternating(void)
{
    unsigned short i;
    for (i = 0; i < MAX_BUF; i++)
        g_buf[i] = (i & 1) ? 0xAA : 0x55;

    ASSERT(run_cksum_case("121c_alt_2",    g_buf,   2, 0x0000));
    ASSERT(run_cksum_case("121c_alt_4",    g_buf,   4, 0x0000));
    ASSERT(run_cksum_case("121c_alt_16",   g_buf,  16, 0x0000));
    ASSERT(run_cksum_case("121c_alt_256",  g_buf, 256, 0x0000));
    ASSERT(run_cksum_case("121c_alt_512",  g_buf, 512, 0x0000));
    PASS();
}

/*
 * Seed wrap-around: seeds that cause the initial DX to be near 0xFFFF,
 * so the very first byte addition triggers a carry.
 */
TEST test_seed_wraparound(void)
{
    g_buf[0] = 0x01;
    g_buf[1] = 0x02;
    g_buf[2] = 0xFF;
    g_buf[3] = 0x00;

    /* seed=0xFFFF, byte=0x01 -> DX=0x0000 + carry -> DX=0x0001 */
    ASSERT(run_cksum_case("121c_wrap_FFFF_01",   g_buf, 1, 0xFFFF));
    /* seed=0xFFFE, byte=0x02 -> DX=0x0000 + carry -> DX=0x0001 */
    ASSERT(run_cksum_case("121c_wrap_FFFE_02",   g_buf+1, 1, 0xFFFE));
    /* seed=0xFF00, byte=0xFF -> DX=0xFEFF + carry? no; 0xFF00+0xFF=0xFFFF no carry */
    ASSERT(run_cksum_case("121c_wrap_FF00_FF",   g_buf+2, 1, 0xFF00));
    /* seed=0xFFFF, byte=0xFF -> DX=0xFFFE + carry -> DX=0xFFFF */
    ASSERT(run_cksum_case("121c_wrap_FFFF_FF",   g_buf+2, 1, 0xFFFF));
    /* Multi-byte with wrapping seed */
    ASSERT(run_cksum_case("121c_wrap_multi",     g_buf, 4, 0xFF00));
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

    ASSERT(run_cksum_case("121c_bnd_3",    g_buf,   3, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_7",    g_buf,   7, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_8",    g_buf,   8, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_9",    g_buf,   9, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_15",   g_buf,  15, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_17",   g_buf,  17, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_31",   g_buf,  31, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_33",   g_buf,  33, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_63",   g_buf,  63, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_65",   g_buf,  65, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_127",  g_buf, 127, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_129",  g_buf, 129, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_255",  g_buf, 255, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_257",  g_buf, 257, 0x0000));
    ASSERT(run_cksum_case("121c_bnd_511",  g_buf, 511, 0x0000));
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
        sprintf(name, "121c_align_off%u", off);
        ASSERT(run_cksum_case(name, g_buf + off, 16, 0x0000));
    }

    PASS();
}

/* ---- */
/* Suite                                                                  */
/* ---- */

SUITE(suite)
{
    RUN_TEST(test_zero_length);
    RUN_TEST(test_single_byte);
    RUN_TEST(test_all_zeros);
    RUN_TEST(test_all_ff);
    RUN_TEST(test_incrementing);
    RUN_TEST(test_alternating);
    RUN_TEST(test_seed_wraparound);
    RUN_TEST(test_boundary_lengths);
    RUN_TEST(test_alignment);
}
