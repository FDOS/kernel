/*
 * t121a.c  --  INT 2Fh / AX=121Ah  GET DRIVE NUMBER FROM PATH
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f121a.html):
 *   AX = 121Ah
 *   DS:SI -> ASCIZ path string (e.g. "C:\DIR\FILE.TXT" or "C:FOO")
 *   Return: AL = drive number (0=A:, 1=B:, 2=C:, ..., 25=Z:)
 *           AL = FFh if no drive letter present or drive letter invalid
 *           CF = 0 always (errors expressed via AL=FFh, not carry)
 *
 * Logic (from FreeDOS inthndlr.c case 0x1a):
 *   Examine DS:SI[0] and DS:SI[1].
 *   If DS:SI[1] == ':':
 *     Uppercase DS:SI[0]; if in 'A'..'Z', return AL = char - 'A'.
 *     Otherwise return AL = FFh.
 *   Else (no colon at offset 1):
 *     Return AL = FFh.
 *   CF is never set.
 *
 * Care registers:
 *   AL  -- drive number or FFh (CARE_AL)
 *   All other output registers are don't-cares.
 *   ** from RBIL DS:SI may be incremented by 2
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 121a.ref, verbose stdout
 *   default / test  TEST:      read 121a.ref, run all cases, write 121a.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t121a.c ..\testlib.c
 *
 * Usage:
 *   T121A ref      (reference mode, writes 121a.ref)
 *   T121A test     (test mode, reads 121a.ref, writes 121a.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "121a";

/* ----
 * We only care about AL on return (drive number or FFh).
 * CF is always clear; all other output registers are don't-cares.
 * ---- */
#define CARE_121A   (CARE_AL)

/* ----
 * Maximum path length we will test.
 * ---- */
#define MAX_PATH_LEN  260
#define BUF_SIZE      (MAX_PATH_LEN + 2)

/* Static buffer -- avoids stack overflow in small model */
static char g_pathbuf[BUF_SIZE];

/* ----
 * Helper: set up a PROBE for AX=121Ah with DS:SI -> path string s.
 * ---- */
static void setup_121a(PROBE *p, const char *s, signed short bufsize)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x121A;
    p->before.ds = FP_SEG(s);
    p->before.si = FP_OFF(s);
    p->buffer  = (unsigned char far *)s;
    if (bufsize > 0) /* use strlen or bufsize, whichever is smaller */
    {
        p->bufsize  = (unsigned short)(strlen(s) < bufsize ? strlen(s) + 1 : bufsize); /* only track first bufsize bytes */
    } else /* use exactly bufsize even if garbage */
    {
        bufsize *= -1;
        p->bufsize  = bufsize; /* only track first bufsize bytes */
    }
}

/* ----
 * Core helper: run one test case.
 *   label  -- unique suffix (prefixed with "121a_")
 *   path   -- input string (copied into g_pathbuf)
 * ---- */
static int run_path_case(const char *label, const char *path)
{
    PROBE p;
    char  name[TC_NAME_MAX];

    strncpy(g_pathbuf, path, MAX_PATH_LEN);
    g_pathbuf[MAX_PATH_LEN] = '\0';

    setup_121a(&p, g_pathbuf, 3);

    sprintf(name, "121a_%s", label);

    if (g_ref_mode)
        printf("INPUT: \"%s\"\n", g_pathbuf);

    return run_case(name, &p, CARE_121A, NULL);
}

/* ----
 * Test suites
 * ---- */

/*
 * Valid drive letters A:..Z: as the leading two characters.
 * Each must return AL = drive_index (0..25).
 * Tests both uppercase and lowercase drive letters.
 */
TEST test_valid_drives(void)
{
    char label[32];
    char path[4];
    int  i;

    /* Uppercase A:..Z: with no path after the colon */
    for (i = 0; i < 26; i++) {
        path[0] = (char)('A' + i);
        path[1] = ':';
        path[2] = '\0';
        sprintf(label, "upper_%c_bare", 'A' + i);
        ASSERT(run_path_case(label, path));
    }

    /* Lowercase a:..z: -- must be treated identically (uppercased internally) */
    for (i = 0; i < 26; i++) {
        path[0] = (char)('a' + i);
        path[1] = ':';
        path[2] = '\0';
        sprintf(label, "lower_%c_bare", 'a' + i);
        ASSERT(run_path_case(label, path));
    }

    PASS();
}

/*
 * Valid drive letters followed by typical path suffixes.
 * The drive number returned must still be correct regardless of what
 * follows the colon.
 */
TEST test_valid_drives_with_paths(void)
{
    /* Absolute paths */
    ASSERT(run_path_case("c_abs",        "C:\\DIR\\FILE.TXT"));
    ASSERT(run_path_case("c_root",       "C:\\"));
    ASSERT(run_path_case("c_root_only",  "C:"));
    ASSERT(run_path_case("a_abs",        "A:\\COMMAND.COM"));
    ASSERT(run_path_case("z_abs",        "Z:\\UTIL\\TOOL.EXE"));

    /* Relative paths (drive-relative, no leading backslash) */
    ASSERT(run_path_case("c_rel",        "C:FOO.TXT"));
    ASSERT(run_path_case("c_rel_dir",    "C:DIR\\FILE"));
    ASSERT(run_path_case("d_rel",        "D:SUBDIR\\DATA.DAT"));

    /* Lowercase drive with path */
    ASSERT(run_path_case("c_lower_abs",  "c:\\windows\\system32\\cmd.exe"));
    ASSERT(run_path_case("c_lower_rel",  "c:foo\\bar.txt"));

    /* Drive followed by forward slash (non-canonical but valid input) */
    ASSERT(run_path_case("c_fwdslash",   "C:/DIR/FILE.TXT"));

    /* Drive followed by dot-dot */
    ASSERT(run_path_case("c_dotdot",     "C:..\\PARENT"));

    /* Drive followed by wildcard */
    ASSERT(run_path_case("c_wildcard",   "C:*.*"));

    /* Long path */
    ASSERT(run_path_case("c_long",
        "C:\\LONGDIR\\SUBDIR\\ANOTHER\\DEEP\\PATH\\FILE.TXT"));

    PASS();
}

/*
 * No drive letter: strings that do NOT have ':' at offset 1.
 * All must return AL = FFh.
 */
TEST test_no_drive_letter(void)
{
    /* Relative paths with no drive prefix */
    ASSERT(run_path_case("rel_simple",   "FILE.TXT"));
    ASSERT(run_path_case("rel_dir",      "DIR\\FILE.TXT"));
    ASSERT(run_path_case("rel_dotdot",   "..\\FILE.TXT"));
    ASSERT(run_path_case("rel_dot",      ".\\FILE.TXT"));

    /* Absolute paths with no drive (root-relative) */
    ASSERT(run_path_case("abs_root",     "\\DIR\\FILE.TXT"));
    ASSERT(run_path_case("abs_root_only","\\"));

    /* UNC-like paths */
    ASSERT(run_path_case("unc",          "\\\\SERVER\\SHARE\\FILE"));

    /* Single character (no room for ':' at offset 1) */
    ASSERT(run_path_case("single_A",     "A"));
    ASSERT(run_path_case("single_slash", "\\"));
    ASSERT(run_path_case("single_dot",   "."));

    /* Empty string */
    ASSERT(run_path_case("empty",        ""));

    /* Colon NOT at offset 1 */
    ASSERT(run_path_case("colon_at_0",   ":something"));   /* ':' at [0] */
    ASSERT(run_path_case("colon_at_2",   "AB:CD"));        /* ':' at [2] */
    ASSERT(run_path_case("colon_at_3",   "ABC:DEF"));      /* ':' at [3] */

    /* Digit before colon -- not a valid drive letter */
    ASSERT(run_path_case("digit_colon",  "1:FILE"));
    ASSERT(run_path_case("zero_colon",   "0:FILE"));
    ASSERT(run_path_case("nine_colon",   "9:FILE"));

    PASS();
}

/*
 * Invalid characters before the colon: non-alpha characters at [0]
 * with ':' at [1].  Must return AL = FFh.
 */
TEST test_invalid_drive_chars(void)
{
    char label[32];
    char path[4];
    unsigned int ch;

    path[1] = ':';
    path[2] = '\0';

    /* Digits 0..9 */
    for (ch = '0'; ch <= '9'; ch++) {
        path[0] = (char)ch;
        sprintf(label, "digit_0x%02X", ch);
        ASSERT(run_path_case(label, path));
    }

    /* Punctuation and special chars that are not letters */
    /* Spot-check a representative set */
    path[0] = '!';  ASSERT(run_path_case("bang_colon",       path));
    path[0] = '@';  ASSERT(run_path_case("at_colon",         path));
    path[0] = '[';  ASSERT(run_path_case("lbracket_colon",   path));  /* 0x5B, just above 'Z' */
    path[0] = '`';  ASSERT(run_path_case("backtick_colon",   path));  /* 0x60, just below 'a' */
    path[0] = '{';  ASSERT(run_path_case("lbrace_colon",     path));  /* 0x7B, just above 'z' */
    path[0] = ' ';  ASSERT(run_path_case("space_colon",      path));
    path[0] = '\t'; ASSERT(run_path_case("tab_colon",        path));
    path[0] = '\0'; ASSERT(run_path_case("nul_colon",        path));  /* empty string effectively */
    path[0] = (char)0x80; ASSERT(run_path_case("high80_colon", path));
    path[0] = (char)0xFF; ASSERT(run_path_case("highFF_colon", path));

    PASS();
}

/*
 * Boundary: characters immediately adjacent to 'A'/'Z' and 'a'/'z'.
 * Confirms the alpha range check is tight.
 */
TEST test_boundary_drive_chars(void)
{
    char path[4];

    path[1] = ':';
    path[2] = '\0';

    /* 0x40 = '@' -- just below 'A' (0x41), must be invalid */
    path[0] = '@';
    ASSERT(run_path_case("below_A",  path));

    /* 'A' = 0x41 -- valid */
    path[0] = 'A';
    ASSERT(run_path_case("at_A",     path));

    /* 'Z' = 0x5A -- valid */
    path[0] = 'Z';
    ASSERT(run_path_case("at_Z",     path));

    /* 0x5B = '[' -- just above 'Z', must be invalid */
    path[0] = '[';
    ASSERT(run_path_case("above_Z",  path));

    /* 0x60 = '`' -- just below 'a' (0x61), must be invalid */
    path[0] = '`';
    ASSERT(run_path_case("below_a",  path));

    /* 'a' = 0x61 -- valid (lowercase, uppercased to 'A') */
    path[0] = 'a';
    ASSERT(run_path_case("at_a",     path));

    /* 'z' = 0x7A -- valid (lowercase, uppercased to 'Z') */
    path[0] = 'z';
    ASSERT(run_path_case("at_z",     path));

    /* 0x7B = '{' -- just above 'z', must be invalid */
    path[0] = '{';
    ASSERT(run_path_case("above_z",  path));

    PASS();
}

/*
 * Alignment: path string starting at various byte offsets within
 * g_pathbuf to catch any word-alignment assumptions in the DOS kernel.
 */
TEST test_alignment(void)
{
    static char aligned_buf[BUF_SIZE + 8];
    PROBE        p;
    char         name[TC_NAME_MAX];
    unsigned int off;

    for (off = 0; off <= 7; off++) {
        char *s = aligned_buf + off;

        /* Use "C:\FILE" as a representative path with a valid drive */
        strcpy(s, "C:\\FILE.TXT");
        setup_121a(&p, s, BUF_SIZE);

        sprintf(name, "121a_align_valid_off%u", off);
        if (g_ref_mode)
            printf("INPUT (off=%u, valid): \"%s\"\n", off, s);
        ASSERT(run_case(name, &p, CARE_121A, NULL));

        /* Also test a no-drive path at each offset */
        strcpy(s, "NODRV.TXT");
        setup_121a(&p, s, BUF_SIZE);

        sprintf(name, "121a_align_nodrv_off%u", off);
        if (g_ref_mode)
            printf("INPUT (off=%u, no-drive): \"%s\"\n", off, s);
        ASSERT(run_case(name, &p, CARE_121A, NULL));
    }

    PASS();
}

/*
 * Colon at offset 1 but with NUL at offset 0 (degenerate: empty string
 * with a colon embedded).  Behaviour is implementation-defined but must
 * not crash; we record whatever the DOS returns.
 */
TEST test_degenerate(void)
{
    PROBE p;

    /* "\0:" -- NUL then colon */
    g_pathbuf[0] = '\0';
    g_pathbuf[1] = ':';
    g_pathbuf[2] = '\0';

    setup_121a(&p, g_pathbuf, 3);

    if (g_ref_mode)
        printf("INPUT: NUL-colon degenerate\n");

    ASSERT(run_case("121a_nul_colon", &p, CARE_121A, NULL));

    /* "  :" -- two spaces then colon (space is not alpha) */
    g_pathbuf[0] = ' ';
    g_pathbuf[1] = ':';
    g_pathbuf[2] = '\0';

    setup_121a(&p, g_pathbuf, 3);

    if (g_ref_mode)
        printf("INPUT: space-colon degenerate\n");

    ASSERT(run_case("121a_space_colon", &p, CARE_121A, NULL));

    PASS();
}

/* ----
 * Suite
 * ---- */

SUITE(suite)
{
    RUN_TEST(test_valid_drives);
    RUN_TEST(test_valid_drives_with_paths);
    RUN_TEST(test_no_drive_letter);
    RUN_TEST(test_invalid_drive_chars);
    RUN_TEST(test_boundary_drive_chars);
    RUN_TEST(test_alignment);
    RUN_TEST(test_degenerate);
}
