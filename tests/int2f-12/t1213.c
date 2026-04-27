/*
 * t1213.c  --  INT 2Fh / AX=1213h  UPPERCASE CHARACTER
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f1213.html):
 *   AX = 1213h
 *   STACK: WORD containing character to uppercase (low byte used)
 *   Return: AL = uppercased character
 *           a-z (0x61..0x7A) -> A-Z (0x41..0x5A)
 *           all other byte values returned unchanged
 *
 * NOTE ON CALLING CONVENTION:
 *   Like AX=1204h, the input character is passed as a WORD pushed onto
 *   the stack BEFORE INT 2Fh, not in any register.  The PROBE.value
 *   field carries this word; probe_int2f() pushes it before the INT.
 *
 * Care registers:
 *   AL  -- uppercased character (CARE_AL)
 *   All other output registers are don't-cares.
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 1213.ref, verbose stdout
 *   default / test  TEST:      read 1213.ref, run all cases, write 1213.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t1213.c ..\testlib.c
 *
 * Usage:
 *   T1213 ref      (reference mode, writes 1213.ref)
 *   T1213 test     (test mode, reads 1213.ref, writes 1213.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "1213";

/* ----
 * We only care about AL on return (the uppercased character).
 * All other output registers are don't-cares.
 * ---- */
#define CARE_1213   (CARE_AL)

/* ----
 * Helper: set up a PROBE for AX=1213h.
 * The input character is passed via p->value (pushed on stack before INT 2Fh).
 * ---- */
static void setup_1213(PROBE *p, unsigned char ch)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x1213;
    p->before.cx = (unsigned short)ch;  /* store in CX for verbose log only */
    p->value     = (unsigned short)ch;  /* actual input: pushed on stack     */
}

/* ----
 * Core helper: run one character test case.
 * ---- */
static int run_char_case(const char *name, unsigned char ch)
{
    PROBE p;

    setup_1213(&p, ch);

    if (g_ref_mode) {
        printf("INPUT: ch=0x%02X ('%c')\n",
               (unsigned)ch,
               (ch >= 0x20 && ch < 0x7F) ? (char)ch : '.');
    }

    return run_case(name, &p, CARE_1213, NULL);
}

/* ---- */
/* Test suites                                                            */
/* ---- */

/*
 * Lowercase letters a..z: each must be uppercased to A..Z.
 */
TEST test_lowercase_letters(void)
{
    char name[TC_NAME_MAX];
    int  i;

    for (i = 0; i < 26; i++) {
        unsigned char ch = (unsigned char)('a' + i);
        sprintf(name, "1213_lower_%c", ch);
        ASSERT(run_char_case(name, ch));
    }

    PASS();
}

/*
 * Uppercase letters A..Z: must be returned unchanged.
 */
TEST test_uppercase_letters(void)
{
    char name[TC_NAME_MAX];
    int  i;

    for (i = 0; i < 26; i++) {
        unsigned char ch = (unsigned char)('A' + i);
        sprintf(name, "1213_upper_%c", ch);
        ASSERT(run_char_case(name, ch));
    }

    PASS();
}

/*
 * Digits 0..9: must be returned unchanged.
 */
TEST test_digits(void)
{
    char name[TC_NAME_MAX];
    int  i;

    for (i = 0; i <= 9; i++) {
        unsigned char ch = (unsigned char)('0' + i);
        sprintf(name, "1213_digit_%c", ch);
        ASSERT(run_char_case(name, ch));
    }

    PASS();
}

/*
 * Boundary characters immediately adjacent to the a-z range.
 * 0x60 = '`' (just below 'a' 0x61) -- must NOT be uppercased.
 * 0x7B = '{' (just above 'z' 0x7A) -- must NOT be uppercased.
 * 0x40 = '@' (just below 'A' 0x41) -- must pass through unchanged.
 * 0x5B = '[' (just above 'Z' 0x5A) -- must pass through unchanged.
 */
TEST test_boundary_chars(void)
{
    ASSERT(run_char_case("1213_backtick",   0x60));  /* just below 'a' */
    ASSERT(run_char_case("1213_lbrace",     0x7B));  /* just above 'z' */
    ASSERT(run_char_case("1213_at",         0x40));  /* just below 'A' */
    ASSERT(run_char_case("1213_lbracket",   0x5B));  /* just above 'Z' */
    ASSERT(run_char_case("1213_rbracket",   0x5D));  /* 'Z'+3          */
    ASSERT(run_char_case("1213_underscore", 0x5F));  /* 'Z'+5          */
    PASS();
}

/*
 * Punctuation and special printable ASCII (not letters or digits):
 * all must be returned unchanged.
 */
TEST test_punctuation(void)
{
    ASSERT(run_char_case("1213_space",     ' '));
    ASSERT(run_char_case("1213_bang",      '!'));
    ASSERT(run_char_case("1213_hash",      '#'));
    ASSERT(run_char_case("1213_dollar",    '$'));
    ASSERT(run_char_case("1213_percent",   '%'));
    ASSERT(run_char_case("1213_ampersand", '&'));
    ASSERT(run_char_case("1213_star",      '*'));
    ASSERT(run_char_case("1213_plus",      '+'));
    ASSERT(run_char_case("1213_comma",     ','));
    ASSERT(run_char_case("1213_minus",     '-'));
    ASSERT(run_char_case("1213_dot",       '.'));
    ASSERT(run_char_case("1213_fwdslash",  '/'));
    ASSERT(run_char_case("1213_colon",     ':'));
    ASSERT(run_char_case("1213_semicolon", ';'));
    ASSERT(run_char_case("1213_lt",        '<'));
    ASSERT(run_char_case("1213_eq",        '='));
    ASSERT(run_char_case("1213_gt",        '>'));
    ASSERT(run_char_case("1213_question",  '?'));
    ASSERT(run_char_case("1213_backslash", '\\'));
    ASSERT(run_char_case("1213_caret",     '^'));
    ASSERT(run_char_case("1213_tilde",     '~'));
    ASSERT(run_char_case("1213_del",       0x7F));
    PASS();
}

/*
 * Control characters (0x00..0x1F): must be returned unchanged.
 */
TEST test_control_chars(void)
{
    char name[TC_NAME_MAX];
    unsigned int ch;

    for (ch = 0x00; ch <= 0x1F; ch++) {
        sprintf(name, "1213_ctrl_0x%02X", ch);
        ASSERT(run_char_case(name, (unsigned char)ch));
    }

    PASS();
}

/*
 * High-byte characters (0x80..0xFF): must be returned unchanged.
 * These are DOS codepage characters; none are in the a-z ASCII range.
 * Sweep every third value to keep the test count manageable.
 */
TEST test_high_bytes(void)
{
    char         name[TC_NAME_MAX];
    unsigned int ch;

    /* Spot-check boundaries */
    ASSERT(run_char_case("1213_high_80", 0x80));
    ASSERT(run_char_case("1213_high_81", 0x81));
    ASSERT(run_char_case("1213_high_FE", 0xFE));
    ASSERT(run_char_case("1213_high_FF", 0xFF));

    /* Sweep 0x82..0xFD every 3rd byte */
    for (ch = 0x82; ch <= 0xFD; ch += 3) {
        sprintf(name, "1213_high_0x%02X", ch);
        ASSERT(run_char_case(name, (unsigned char)ch));
    }

    PASS();
}

/*
 * Full sweep of all 256 byte values.
 * Verifies the complete transformation table in one pass.
 * Every third value is tested to keep the .ref file compact.
 */
TEST test_full_sweep(void)
{
    char         name[TC_NAME_MAX];
    unsigned int ch;

    for (ch = 0x00; ch <= 0xFF; ch += 3) {
        sprintf(name, "1213_sweep_0x%02X", ch);
        ASSERT(run_char_case(name, (unsigned char)ch));
    }

    PASS();
}

/* ---- */
/* Suite                                                                  */
/* ---- */

SUITE(suite)
{
    RUN_TEST(test_lowercase_letters);
    RUN_TEST(test_uppercase_letters);
    RUN_TEST(test_digits);
    RUN_TEST(test_boundary_chars);
    RUN_TEST(test_punctuation);
    RUN_TEST(test_control_chars);
    RUN_TEST(test_high_bytes);
    RUN_TEST(test_full_sweep);
}
