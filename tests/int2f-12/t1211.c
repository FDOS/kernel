/*
 * t1211.c  --  INT 2Fh / AX=1211h  NORMALIZE ASCIZ FILENAME
 *
 * RBIL (https://fd.lod.bz/rbil/interrup/dos_kernel/2f1211.html):
 *   AX = 1211h
 *   DS:SI -> ASCIZ filename to normalize (source)
 *   ES:DI -> buffer for normalized filename (destination)
 *   Return: destination buffer filled with uppercased filename,
 *           forward slashes '/' converted to backslashes '\',
 *           NUL terminator copied.
 *   No register return value (AX, BX, CX, DX, SI, DI, DS, ES, FLAGS
 *   are all don't-cares).
 *
 * FreeDOS inthndlr.c case 0x11:
 *   Copies DS:SI -> ES:DI, converting a-z -> A-Z and '/' -> '\',
 *   until and including the NUL terminator.
 *
 * Care:
 *   No register return value is defined; CARE_NONE for registers.
 *   The output is entirely in the destination buffer.
 *   run_case() records/compares the destination buffer via the
 *   "B" lines in the .ref file -- the reference system writes what
 *   the DOS actually produced; test mode compares against that.
 *   This avoids any local re-implementation of the normalization logic.
 *
 * Two modes (see testlib.h):
 *   ref             REFERENCE: run all cases, write 1211.ref, verbose stdout
 *   default / test  TEST:      read 1211.ref, run all cases, write 1211.tst
 *
 * Build:
 *   wcl -zq -q -ms -i=.. t1211.c ..\testlib.c
 *
 * Usage:
 *   T1211 ref      (reference mode, writes 1211.ref)
 *   T1211 test     (test mode, reads 1211.ref, writes 1211.tst)
 */

#include "../greatest.h"
#include "../testlib.h"

#include <dos.h>
#include <stdio.h>
#include <string.h>

const char *g_tbasename = "1211";

/* ----
 * No register return value is defined for 1211h, only buffer.
 * ---- */
#define CARE_1211   (CARE_BUF)

/* ----
 * Buffer sizes.
 * Source and destination are kept in separate static buffers.
 * We test strings up to 260 chars (MAX_PATH).
 * ---- */
#define MAX_STR_LEN  260
#define BUF_SIZE     (MAX_STR_LEN + 2)   /* +2: NUL + sentinel byte */

static char g_src[BUF_SIZE];   /* source string passed in DS:SI  */
static char g_dst[BUF_SIZE];   /* destination buffer at ES:DI    */

/* ----
 * Helper: set up a PROBE for AX=1211h.
 *   DS:SI -> g_src  (source)
 *   ES:DI -> g_dst  (destination)
 * DS is explicitly set because it carries a real far pointer (DS:SI).
 * ---- */
static void setup_1211(PROBE *p, unsigned char *src_ptr, unsigned char *dst_ptr, unsigned short buf_len)
{
    memset(p, 0, sizeof(*p));
    p->before.ax = 0x1211;
    p->before.ds = FP_SEG(src_ptr);   /* DS:SI -> source  */
    p->before.si = FP_OFF(src_ptr);
    p->before.es = FP_SEG(dst_ptr);   /* ES:DI -> dest    */
    p->before.di = FP_OFF(dst_ptr);
	p->buffer = (unsigned char far *)dst_ptr;
	p->bufsize = buf_len;
}

/* ----
 * Core helper: run one test case via run_case().
 *   label  -- unique test name suffix (prefixed with "1211_")
 *   src    -- input string (copied into g_src)
 *
 * In reference mode: calls the DOS, records destination buffer to .ref.
 * In test mode:      calls the DOS, compares destination buffer to .ref.
 *
 * Also checks the sentinel byte after g_dst[len] is unchanged (no overrun).
 * ---- */
static int run_norm_case(const char *label, const char *src)
{
    PROBE         p;
    char          name[TC_NAME_MAX];
    unsigned short srclen;

    /* Prepare source */
    strncpy(g_src, src, MAX_STR_LEN);
    g_src[MAX_STR_LEN] = '\0';
    srclen = (unsigned short)strlen(g_src);

    /* Poison destination so we can detect partial writes */
    memset(g_dst, 0xCC, sizeof(g_dst));
    g_dst[BUF_SIZE - 1] = '\xAA';   /* sentinel */

    setup_1211(&p, g_src, g_dst, srclen+1);

    sprintf(name, "1211_%s", label);

    if (g_ref_mode)
        printf("INPUT:  \"%s\"\n", g_src);

    /* run_case() handles probe + ref_write + ref_write_buf (ref mode)
     * or probe + tst_compare + tst_compare_buf (test mode).
     * buf_len = srclen + 1 to include the NUL terminator.              */
    if (!run_case(name, &p, CARE_1211, NULL))
        return 0;

    if (g_ref_mode)
        printf("OUTPUT: \"%s\"\n", g_dst);

    /* Verify sentinel not overwritten (both modes) */
    if ((unsigned char)g_dst[BUF_SIZE - 1] != 0xAA) {
        printf("FAIL  %s  (sentinel overwritten)\n", name);
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* Test suites                                                         */
/* ------------------------------------------------------------------ */

/*
 * Empty string: NUL is the only byte; destination must contain NUL only.
 */
TEST test_empty(void)
{
    ASSERT(run_norm_case("empty", ""));
    PASS();
}

/*
 * Already-uppercase filenames: output must equal input exactly.
 */
TEST test_already_upper(void)
{
    ASSERT(run_norm_case("upper_simple",    "FILENAME.EXT"));
    ASSERT(run_norm_case("upper_path",      "C:\\DIR\\FILE.TXT"));
    ASSERT(run_norm_case("upper_root",      "\\"));
    ASSERT(run_norm_case("upper_dot",       "."));
    ASSERT(run_norm_case("upper_dotdot",    ".."));
    ASSERT(run_norm_case("upper_drive",     "C:"));
    ASSERT(run_norm_case("upper_drive_rel", "C:FOO"));
    ASSERT(run_norm_case("upper_wildcard",  "*.*"));
    ASSERT(run_norm_case("upper_wildcard2", "?.?"));
    PASS();
}

/*
 * Lowercase letters: must be uppercased.
 */
TEST test_lowercase(void)
{
    ASSERT(run_norm_case("lower_simple",    "filename.ext"));
    ASSERT(run_norm_case("lower_mixed",     "FileName.Ext"));
    ASSERT(run_norm_case("lower_all_az",    "abcdefghijklmnopqrstuvwxyz"));
    ASSERT(run_norm_case("lower_path",      "c:\\dir\\file.txt"));
    ASSERT(run_norm_case("lower_drive_rel", "c:foo"));
    ASSERT(run_norm_case("lower_deep",      "a\\b\\c\\d\\e\\f\\g.txt"));
    ASSERT(run_norm_case("lower_single_a",  "a"));
    ASSERT(run_norm_case("lower_single_z",  "z"));
    PASS();
}

/*
 * Forward-slash conversion: '/' must become '\'.
 */
TEST test_slash_conversion(void)
{
    ASSERT(run_norm_case("slash_only",      "/"));
    ASSERT(run_norm_case("slash_double",    "//"));
    ASSERT(run_norm_case("slash_path",      "dir/file.txt"));
    ASSERT(run_norm_case("slash_mixed",     "a/b\\c/d"));
    ASSERT(run_norm_case("slash_leading",   "/dir/file"));
    ASSERT(run_norm_case("slash_trailing",  "dir/"));
    ASSERT(run_norm_case("slash_drive",     "c:/dir/file.txt"));
    ASSERT(run_norm_case("slash_all",       "a/b/c/d/e/f/g"));
    PASS();
}

/*
 * Combined: lowercase + forward slashes together.
 */
TEST test_combined(void)
{
    ASSERT(run_norm_case("combo_simple",    "c:/dir/file.txt"));
    ASSERT(run_norm_case("combo_deep",      "c:/users/foo/bar/baz.txt"));
    ASSERT(run_norm_case("combo_mixed",     "C:/Dir/File.Txt"));
    ASSERT(run_norm_case("combo_all_lower", "abcdefghijklmnopqrstuvwxyz/xyz"));
    ASSERT(run_norm_case("combo_83",        "c:/progra~1/myapp.exe"));
    PASS();
}

/*
 * Characters that must pass through unchanged:
 * digits, punctuation, high bytes, control chars (except '/' and a-z).
 */
TEST test_passthrough(void)
{
    /* Digits */
    ASSERT(run_norm_case("pass_digits",     "0123456789"));

    /* Punctuation that is NOT '/' */
    ASSERT(run_norm_case("pass_punct",      "!@#$%^&*()-_=+[]{}|;:',.<>?`~"));

    /* Backslash already canonical */
    ASSERT(run_norm_case("pass_backslash",  "\\"));

    /* Space */
    ASSERT(run_norm_case("pass_space",      "A B C"));

    /* High bytes (DOS codepage chars) -- must be copied unchanged */
    ASSERT(run_norm_case("pass_high_80",    "\x80"));
    ASSERT(run_norm_case("pass_high_fe",    "\xFE"));
    ASSERT(run_norm_case("pass_high_ff",    "\xFF"));
    ASSERT(run_norm_case("pass_high_mix",
                         "\x80\x90\xA0\xB0\xC0\xD0\xE0\xF0"));

    /* Control characters (non-NUL) */
    ASSERT(run_norm_case("pass_ctrl_01",    "\x01"));
    ASSERT(run_norm_case("pass_ctrl_1a",    "\x1A"));   /* Ctrl-Z */
    ASSERT(run_norm_case("pass_ctrl_1b",    "\x1B"));   /* ESC    */
    ASSERT(run_norm_case("pass_ctrl_1f",    "\x1F"));

    PASS();
}

/*
 * Boundary: characters immediately adjacent to 'a' (0x61), 'z' (0x7A),
 * and '/' (0x2F) to confirm the range checks are tight.
 */
TEST test_boundary_chars(void)
{
    /* 0x60 = '`' -- just below 'a', must NOT be uppercased */
    ASSERT(run_norm_case("bound_backtick",  "`"));

    /* 0x7B = '{' -- just above 'z', must NOT be uppercased */
    ASSERT(run_norm_case("bound_lbrace",    "{"));

    /* 0x2E = '.' -- just below '/', must NOT become '\' */
    ASSERT(run_norm_case("bound_dot",       "."));

    /* 0x30 = '0' -- just above '/', must NOT become '\' */
    ASSERT(run_norm_case("bound_zero",      "0"));

    /* 0x5B = '[' -- just below '\' (0x5C), must pass through */
    ASSERT(run_norm_case("bound_lbracket",  "["));

    /* 0x5D = ']' -- just above '\' (0x5C), must pass through */
    ASSERT(run_norm_case("bound_rbracket",  "]"));

    PASS();
}

/*
 * Typical DOS path forms.
 */
TEST test_dos_paths(void)
{
    ASSERT(run_norm_case("path_root_only",   "\\"));
    ASSERT(run_norm_case("path_drive_root",  "C:\\"));
    ASSERT(run_norm_case("path_abs",
                         "C:\\WINDOWS\\SYSTEM32\\CMD.EXE"));
    ASSERT(run_norm_case("path_rel",         "DIR\\FILE.TXT"));
    ASSERT(run_norm_case("path_dotdot",      "..\\..\\FILE.TXT"));
    ASSERT(run_norm_case("path_trailing_bs", "C:\\DIR\\"));
    ASSERT(run_norm_case("path_unc_like",    "\\\\SERVER\\SHARE\\FILE"));
    ASSERT(run_norm_case("path_spaces",      "C:\\MY DOCS\\FILE.TXT"));
    ASSERT(run_norm_case("path_83_upper",    "C:\\PROGRA~1\\MYAPP.EXE"));
    ASSERT(run_norm_case("path_83_lower",    "c:\\progra~1\\myapp.exe"));
    ASSERT(run_norm_case("path_mixed_slash", "c:/windows/system32/cmd.exe"));
    PASS();
}

/*
 * Alignment: source and destination at various byte offsets within
 * their respective buffers, to catch word-alignment assumptions.
 */
TEST test_alignment(void)
{
    static char src_aligned[BUF_SIZE + 8];
    static char dst_aligned[BUF_SIZE + 8];
    PROBE        p;
    char         name[TC_NAME_MAX];
    unsigned int off;
    const char  *base = "abcdef/GHIJKL";
    unsigned short baselen;

    baselen = (unsigned short)strlen(base);

    for (off = 0; off <= 7; off++) {
        char *src_ptr = src_aligned + off;
        char *dst_ptr = dst_aligned + off;

        strcpy(src_ptr, base);
        memset(dst_ptr, 0xCC, (unsigned short)(baselen + 2));

        setup_1211(&p, src_ptr, dst_ptr, baselen+1);

        sprintf(name, "1211_align_off%u", off);

        if (g_ref_mode)
            printf("INPUT (off=%u): \"%s\"\n", off, base);

        ASSERT(run_case(name, &p, CARE_1211, NULL));
    }

    PASS();
}

/*
 * Source == destination (in-place normalization).
 * RBIL does not forbid src==dst; FreeDOS reads one byte then writes it,
 * so in-place is safe.
 */
TEST test_inplace(void)
{
    PROBE p;
    const char *src = "c:/dir/file.txt";
    unsigned short srclen;

    strncpy(g_src, src, MAX_STR_LEN);
    g_src[MAX_STR_LEN] = '\0';
    srclen = (unsigned short)strlen(g_src);

    setup_1211(&p, g_src, g_src, srclen+1);

    if (g_ref_mode)
        printf("INPUT (in-place): \"%s\"\n", src);

    ASSERT(run_case("1211_inplace", &p, CARE_1211, NULL));

    PASS();
}

/*
 * Single-character strings for every printable ASCII value.
 * Verifies the per-character transformation table is correct.
 */
TEST test_single_chars(void)
{
    char         name[TC_NAME_MAX];
    unsigned int ch;
    char         src[2];

    src[1] = '\0';
    for (ch = 0x01; ch <= 0x7E; ch++) {
        src[0] = (char)ch;
        sprintf(name, "char_0x%02X", ch);
        ASSERT(run_norm_case(name, src));
    }

    /* A few high bytes */
    src[0] = (char)0x80; ASSERT(run_norm_case("char_0x80", src));
    src[0] = (char)0xA0; ASSERT(run_norm_case("char_0xA0", src));
    src[0] = (char)0xFF; ASSERT(run_norm_case("char_0xFF", src));

    PASS();
}

/* ------------------------------------------------------------------ */
/* Suite                                                               */
/* ------------------------------------------------------------------ */

SUITE(suite)
{
    RUN_TEST(test_empty);
    RUN_TEST(test_already_upper);
    RUN_TEST(test_lowercase);
    RUN_TEST(test_slash_conversion);
    RUN_TEST(test_combined);
    RUN_TEST(test_passthrough);
    RUN_TEST(test_boundary_chars);
    RUN_TEST(test_dos_paths);
    RUN_TEST(test_alignment);
    RUN_TEST(test_inplace);
    RUN_TEST(test_single_chars);
}
