/*
 * t1204.c  --  INT 2Fh / AX=1204h  NORMALIZE PATH SEPARATOR
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f1204.html):
 *   AX = 1204h
 *   STACK: WORD character to normalize
 *   Return: AL = normalized character ('/' -> '\', others unchanged)
 *           ZF set if character is a path separator ('\', '/', or SWITCHAR)
 *           STACK unchanged
 *
 * NOTE ON CALLING CONVENTION:
 *   This function is unique among INT 2Fh/12xx subfunctions: the input
 *   character is passed as a WORD pushed onto the stack BEFORE the INT 2Fh,
 *   not in any register. 
 *
 * Care registers:
 *   AL  -- normalized character (CARE_AL)
 *   ZF  -- set iff path separator (CARE_ZF)
 *   All other output registers are don't-cares.
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 1204.ref, verbose stdout
 *   default / test  TEST:      read 1204.ref, run all cases, write 1204.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t1204.c ..\testlib.c
 *
 * Usage:
 *   T1204 ref      (reference mode, writes 1204.ref)
 *   T1204 test     (test mode, reads 1204.ref, writes 1204.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>


const char *g_tbasename = "1204"; /* declared by test e.g. "1204" */


/* ----
 * We care about AL (normalized char) and ZF (separator flag).
 * All other output registers are don't-cares.
 * ---- */
#define CARE_1204   (CARE_AL | CARE_FLAGS)


/* ------------------------------------------------------------------ */
/* Helper: fill a PROBE for recording purposes.                        */
/* after.ax low byte = AL result; after.flags = FLAGS result.         */
/* ------------------------------------------------------------------ */
static void setup_1204(PROBE *p, unsigned char ch)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x1204;
    p->before.cx = (unsigned short)ch;  /* input char stored in CX for log */
	p->value = (unsigned short)ch;
}

/* ------------------------------------------------------------------ */
/* Core test runner for one character.                                 */
/* ------------------------------------------------------------------ */
static int run_char_case(const char *name, unsigned char ch)
{
    PROBE      p;

    setup_1204(&p, ch);

    if (g_ref_mode) {
        printf("INPUT: ch=0x%02X ('%c')\n",
               (unsigned)ch,
               (ch >= 0x20 && ch < 0x7F) ? (char)ch : '.'
		);
    }

    return run_case(name, &p, CARE_1204, NULL);
}

/* ------------------------------------------------------------------ */
/* Test suites                                                         */
/* ------------------------------------------------------------------ */

/*
 * Path separator characters: '/', '\', and the default SWITCHAR '/'.
 * All must return AL='\' (5Ch) and ZF=1.
 */
TEST test_separators(void)
{
    char name[TC_NAME_MAX];

    /* Forward slash -- must be converted to backslash, ZF=1 */
    sprintf(name, "1204_fwdslash");
    ASSERT(run_char_case(name, '/'));

    /* Backslash -- already canonical, ZF=1 */
    sprintf(name, "1204_backslash");
    ASSERT(run_char_case(name, '\\'));

    PASS();
}

/*
 * Non-separator printable ASCII characters: AL unchanged, ZF=0.
 */
TEST test_non_separators(void)
{
    char         name[TC_NAME_MAX];
    unsigned int ch;

    /* Letters */
    sprintf(name, "1204_letter_A");
    ASSERT(run_char_case(name, 'A'));
    sprintf(name, "1204_letter_a");
    ASSERT(run_char_case(name, 'a'));
    sprintf(name, "1204_letter_Z");
    ASSERT(run_char_case(name, 'Z'));
    sprintf(name, "1204_letter_z");
    ASSERT(run_char_case(name, 'z'));

    /* Digits */
    sprintf(name, "1204_digit_0");
    ASSERT(run_char_case(name, '0'));
    sprintf(name, "1204_digit_9");
    ASSERT(run_char_case(name, '9'));

    /* Common filename chars */
    sprintf(name, "1204_dot");
    ASSERT(run_char_case(name, '.'));
    sprintf(name, "1204_colon");
    ASSERT(run_char_case(name, ':'));
    sprintf(name, "1204_space");
    ASSERT(run_char_case(name, ' '));
    sprintf(name, "1204_underscore");
    ASSERT(run_char_case(name, '_'));
    sprintf(name, "1204_dash");
    ASSERT(run_char_case(name, '-'));
    sprintf(name, "1204_tilde");
    ASSERT(run_char_case(name, '~'));
    sprintf(name, "1204_bang");
    ASSERT(run_char_case(name, '!'));
    sprintf(name, "1204_hash");
    ASSERT(run_char_case(name, '#'));
    sprintf(name, "1204_dollar");
    ASSERT(run_char_case(name, '$'));
    sprintf(name, "1204_percent");
    ASSERT(run_char_case(name, '%'));
    sprintf(name, "1204_ampersand");
    ASSERT(run_char_case(name, '&'));
    sprintf(name, "1204_at");
    ASSERT(run_char_case(name, '@'));

    /* -All- printable ASCII except '/' and '\' */
	/* *** we only print some to avoid excessive tests */
    for (ch = 0x20; ch <= 0x7E; ch+=3) {
        if (ch == '/' || ch == '\\')
            continue;
        sprintf(name, "1204_ascii_0x%02X", ch);
        ASSERT(run_char_case(name, (unsigned char)ch));
    }

    PASS();
}

/*
 * Control characters and NUL: AL unchanged, ZF=0.
 */
TEST test_control_chars(void)
{
    char name[TC_NAME_MAX];

    sprintf(name, "1204_nul");
    ASSERT(run_char_case(name, 0x00));

    sprintf(name, "1204_ctrl_01");
    ASSERT(run_char_case(name, 0x01));

    sprintf(name, "1204_ctrl_1A_EOF");   /* Ctrl-Z / EOF */
    ASSERT(run_char_case(name, 0x1A));

    sprintf(name, "1204_ctrl_1B_ESC");   /* ESC */
    ASSERT(run_char_case(name, 0x1B));

    sprintf(name, "1204_ctrl_1F");
    ASSERT(run_char_case(name, 0x1F));

    sprintf(name, "1204_del_7F");
    ASSERT(run_char_case(name, 0x7F));

    PASS();
}

/*
 * High-byte characters (0x80..0xFF): AL unchanged, ZF=0.
 * These are DOS codepage characters; none are path separators.
 */
TEST test_high_bytes(void)
{
    char         name[TC_NAME_MAX];
    unsigned int ch;

    /* Spot-check a few */
    sprintf(name, "1204_high_80");
    ASSERT(run_char_case(name, 0x80));

    sprintf(name, "1204_high_FE");
    ASSERT(run_char_case(name, 0xFE));

    sprintf(name, "1204_high_FF");
    ASSERT(run_char_case(name, 0xFF));

    /* Sweep 0x80..0xFF */
	/* *** we only print some to avoid excess tests */
    for (ch = 0x80; ch <= 0xFF; ch+=3) {
        sprintf(name, "1204_high_0x%02X", ch);
        ASSERT(run_char_case(name, (unsigned char)ch));
    }

    PASS();
}

/*
 * Boundary: characters immediately adjacent to '/' (0x2F) and '\' (0x5C).
 */
TEST test_adjacent_to_separators(void)
{
    char name[TC_NAME_MAX];

    sprintf(name, "1204_before_fwdslash");   /* 0x2E = '.' */
    ASSERT(run_char_case(name, 0x2E));

    sprintf(name, "1204_after_fwdslash");    /* 0x30 = '0' */
    ASSERT(run_char_case(name, 0x30));

    sprintf(name, "1204_before_backslash");  /* 0x5B = '[' */
    ASSERT(run_char_case(name, 0x5B));

    sprintf(name, "1204_after_backslash");   /* 0x5D = ']' */
    ASSERT(run_char_case(name, 0x5D));

    PASS();
}

/* ------------------------------------------------------------------ */
/* Suite -- called in main to run tests                               */
/* ------------------------------------------------------------------ */

SUITE(suite)
{
    RUN_TEST(test_separators);
    RUN_TEST(test_non_separators);
    RUN_TEST(test_control_chars);
    RUN_TEST(test_high_bytes);
    RUN_TEST(test_adjacent_to_separators);
}
