/*
 * t1203.c  --  INT 2Fh / AX=1203h  GET DOS DATA SEGMENT
 *
 * Expected: ES = DOS internal data segment
 *           No register changes other than ES (and possibly AX)
 */
#include "../greatest.h"
#include "../testlib.h"

const char * g_tbasename = "1203";

TEST test_1203(void)
{
    PROBE p;
    memset(&p, 0, sizeof(p));
    p.before.ax = 0x1203;
    /* No special input registers needed for this subfunction */

    probe_int2f(&p);

    /* Record result -- actual assertion is the diff, not a hard value,
       but we can sanity-check that ES is nonzero (DOS is loaded) */
    print_probe_verbose(&p, "GET_DOS_DATA_SEG", -1);

    ASSERT_FALSE(p.after.es == 0);   /* ES must point somewhere */
    PASS();
}

SUITE(suite)
{
    test_1203();
}
