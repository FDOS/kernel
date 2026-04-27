/*
 * t1214.c  --  INT 2Fh / AX=1214h  COMPARE FAR POINTERS
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f1214.html):
 *   AX = 1214h
 *   DS:SI = first  far pointer
 *   ES:DI = second far pointer
 *   Return: ZF=1 if DS==ES and SI==DI (pointers are equal)
 *           ZF=0 if DS!=ES or  SI!=DI (pointers are not equal)
 *           CF=0 always
 *           No other registers modified.
 *
 * Note: comparison is on the raw segment:offset pairs as stored, NOT
 * on normalised linear addresses.  Two pointers that address the same
 * physical byte but have different segment:offset representations are
 * considered NOT equal.
 *
 * Care registers:
 *   FLAGS  -- ZF and CF (CARE_FLAGS)
 *   All other output registers are don't-cares.
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 1214.ref, verbose stdout
 *   default / test  TEST:      read 1214.ref, run all cases, write 1214.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t1214.c ..\testlib.c
 *
 * Usage:
 *   T1214 ref      (reference mode, writes 1214.ref)
 *   T1214 test     (test mode, reads 1214.ref, writes 1214.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "1214";

/* ----
 * We only care about FLAGS on return (ZF and CF).
 * All other output registers are don't-cares.
 * ---- */
#define CARE_1214   (CARE_FLAGS)

/* ----
 * Helper: set up a PROBE for AX=1214h.
 *   DS:SI = first  pointer (seg1:off1)
 *   ES:DI = second pointer (seg2:off2)
 * ---- */
static void setup_1214(PROBE *p,
                        unsigned short seg1, unsigned short off1,
                        unsigned short seg2, unsigned short off2)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x1214;
    p->before.ds = seg1;
    p->before.si = off1;
    p->before.es = seg2;
    p->before.di = off2;
}

/* ----
 * Core helper: run one comparison test case.
 * ---- */
static int run_cmp_case(const char *name,
                         unsigned short seg1, unsigned short off1,
                         unsigned short seg2, unsigned short off2)
{
    PROBE p;

    setup_1214(&p, seg1, off1, seg2, off2);

    if (g_ref_mode) {
        printf("INPUT: ptr1=%04X:%04X  ptr2=%04X:%04X\n",
               seg1, off1, seg2, off2);
    }

    return run_case(name, &p, CARE_1214, NULL);
}

/* ---- */
/* Test suites                                                            */
/* ---- */

/*
 * Equal pointers: same segment AND same offset.
 * ZF must be set (1), CF must be clear (0).
 */
TEST test_equal_pointers(void)
{
    /* Both zero */
    ASSERT(run_cmp_case("1214_eq_zero_zero",   0x0000, 0x0000, 0x0000, 0x0000));

    /* Typical DOS data segment values */
    ASSERT(run_cmp_case("1214_eq_0070_0000",   0x0070, 0x0000, 0x0070, 0x0000));
    ASSERT(run_cmp_case("1214_eq_1234_5678",   0x1234, 0x5678, 0x1234, 0x5678));

    /* Maximum values */
    ASSERT(run_cmp_case("1214_eq_FFFF_FFFF",   0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF));
    ASSERT(run_cmp_case("1214_eq_FFFF_0000",   0xFFFF, 0x0000, 0xFFFF, 0x0000));
    ASSERT(run_cmp_case("1214_eq_0000_FFFF",   0x0000, 0xFFFF, 0x0000, 0xFFFF));

    /* Segment = 0, various offsets */
    ASSERT(run_cmp_case("1214_eq_0000_0001",   0x0000, 0x0001, 0x0000, 0x0001));
    ASSERT(run_cmp_case("1214_eq_0000_8000",   0x0000, 0x8000, 0x0000, 0x8000));

    /* Offset = 0, various segments */
    ASSERT(run_cmp_case("1214_eq_8000_0000",   0x8000, 0x0000, 0x8000, 0x0000));

    /* Pointer to our own code segment (self-referential) */
    ASSERT(run_cmp_case("1214_eq_self",
                         FP_SEG(run_cmp_case), FP_OFF(run_cmp_case),
                         FP_SEG(run_cmp_case), FP_OFF(run_cmp_case)));

    PASS();
}

/*
 * Unequal pointers: segment differs, offset same.
 * ZF must be clear (0).
 */
TEST test_unequal_seg_only(void)
{
    ASSERT(run_cmp_case("1214_ne_seg_0001_vs_0002", 0x0001, 0x0000, 0x0002, 0x0000));
    ASSERT(run_cmp_case("1214_ne_seg_0000_vs_0001", 0x0000, 0x1234, 0x0001, 0x1234));
    ASSERT(run_cmp_case("1214_ne_seg_1234_vs_5678", 0x1234, 0xABCD, 0x5678, 0xABCD));
    ASSERT(run_cmp_case("1214_ne_seg_FFFE_vs_FFFF", 0xFFFE, 0x0000, 0xFFFF, 0x0000));
    ASSERT(run_cmp_case("1214_ne_seg_0000_vs_FFFF", 0x0000, 0x0000, 0xFFFF, 0x0000));
    PASS();
}

/*
 * Unequal pointers: offset differs, segment same.
 * ZF must be clear (0).
 */
TEST test_unequal_off_only(void)
{
    ASSERT(run_cmp_case("1214_ne_off_0000_vs_0001", 0x1000, 0x0000, 0x1000, 0x0001));
    ASSERT(run_cmp_case("1214_ne_off_0001_vs_0000", 0x1000, 0x0001, 0x1000, 0x0000));
    ASSERT(run_cmp_case("1214_ne_off_1234_vs_5678", 0xABCD, 0x1234, 0xABCD, 0x5678));
    ASSERT(run_cmp_case("1214_ne_off_FFFE_vs_FFFF", 0x0000, 0xFFFE, 0x0000, 0xFFFF));
    ASSERT(run_cmp_case("1214_ne_off_0000_vs_FFFF", 0x0000, 0x0000, 0x0000, 0xFFFF));
    PASS();
}

/*
 * Unequal pointers: both segment and offset differ.
 * ZF must be clear (0).
 */
TEST test_unequal_both(void)
{
    ASSERT(run_cmp_case("1214_ne_both_0001_0001_vs_0002_0002",
                         0x0001, 0x0001, 0x0002, 0x0002));
    ASSERT(run_cmp_case("1214_ne_both_1234_5678_vs_ABCD_EF01",
                         0x1234, 0x5678, 0xABCD, 0xEF01));
    ASSERT(run_cmp_case("1214_ne_both_0000_0000_vs_FFFF_FFFF",
                         0x0000, 0x0000, 0xFFFF, 0xFFFF));
    ASSERT(run_cmp_case("1214_ne_both_FFFF_FFFF_vs_0000_0000",
                         0xFFFF, 0xFFFF, 0x0000, 0x0000));
    PASS();
}

/*
 * Aliased pointers: different segment:offset pairs that address the
 * same physical byte (e.g. 0x1000:0x0010 == 0x1001:0x0000 == linear 0x10010).
 * The function compares raw pairs, so ZF must be CLEAR (not equal).
 * This confirms the function does NOT normalise before comparing.
 */
TEST test_aliased_not_equal(void)
{
    /* 0x1000:0x0010 vs 0x1001:0x0000 -- same linear address, different pairs */
    ASSERT(run_cmp_case("1214_alias_1000_0010_vs_1001_0000",
                         0x1000, 0x0010, 0x1001, 0x0000));

    /* 0x0000:0x0100 vs 0x0010:0x0000 -- same linear address 0x100 */
    ASSERT(run_cmp_case("1214_alias_0000_0100_vs_0010_0000",
                         0x0000, 0x0100, 0x0010, 0x0000));

    /* 0xF000:0x1000 vs 0xF001:0x0FF0 -- same linear address 0xF1000 */
    ASSERT(run_cmp_case("1214_alias_F000_1000_vs_F001_0FF0",
                         0xF000, 0x1000, 0xF001, 0x0FF0));

    PASS();
}

/*
 * Symmetry: swap ptr1 and ptr2 for a selection of unequal pairs.
 * The result (ZF=0) must be the same regardless of argument order.
 */
TEST test_symmetry(void)
{
    /* Forward */
    ASSERT(run_cmp_case("1214_sym_fwd_1234_5678",
                         0x1234, 0x5678, 0xABCD, 0xEF01));
    /* Reversed */
    ASSERT(run_cmp_case("1214_sym_rev_1234_5678",
                         0xABCD, 0xEF01, 0x1234, 0x5678));

    /* Forward */
    ASSERT(run_cmp_case("1214_sym_fwd_0000_FFFF",
                         0x0000, 0x0000, 0x0000, 0xFFFF));
    /* Reversed */
    ASSERT(run_cmp_case("1214_sym_rev_0000_FFFF",
                         0x0000, 0xFFFF, 0x0000, 0x0000));

    PASS();
}

/*
 * Null pointer (0000:0000) compared against itself and against non-null.
 */
TEST test_null_pointer(void)
{
    /* NULL == NULL */
    ASSERT(run_cmp_case("1214_null_eq_null",
                         0x0000, 0x0000, 0x0000, 0x0000));

    /* NULL != non-null (segment differs) */
    ASSERT(run_cmp_case("1214_null_ne_seg",
                         0x0000, 0x0000, 0x0001, 0x0000));

    /* NULL != non-null (offset differs) */
    ASSERT(run_cmp_case("1214_null_ne_off",
                         0x0000, 0x0000, 0x0000, 0x0001));

    PASS();
}

/* ---- */
/* Suite                                                                  */
/* ---- */

SUITE(suite)
{
    RUN_TEST(test_equal_pointers);
    RUN_TEST(test_unequal_seg_only);
    RUN_TEST(test_unequal_off_only);
    RUN_TEST(test_unequal_both);
    RUN_TEST(test_aliased_not_equal);
    RUN_TEST(test_symmetry);
    RUN_TEST(test_null_pointer);
}
