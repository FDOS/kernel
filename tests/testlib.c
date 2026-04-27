/*
 * testlib.c  --  shared test infrastructure implementation
 *
 * See testlib.h for full documentation.
 * Build: wcl -zq -q -ms -c testlib.c
 */

#include "../greatest.h"
#include "../testlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* -----------------------------------------------------------------------
 * Globals
 * --------------------------------------------------------------------- */

int      g_ref_mode = 0;
FILE    *g_ref_fp   = NULL;
FILE    *g_tst_fp   = NULL;
TC_LIST  g_tclist;          /* zero-initialised by testlib_init */

/* -----------------------------------------------------------------------
 * TC_RECORD lifecycle
 * --------------------------------------------------------------------- */

TC_RECORD *tc_create(const char *name)
{
    TC_RECORD *rec;
    char      *nm;
    size_t     nlen;

    rec = (TC_RECORD *)malloc(sizeof(TC_RECORD));
    if (!rec) return NULL;
    memset(rec, 0, sizeof(TC_RECORD));

    nlen = strlen(name) + 1;
    nm   = (char *)malloc(nlen);
    if (!nm) { free(rec); return NULL; }
    memcpy(nm, name, nlen);

    rec->name      = nm;
    rec->expected.ax    = REF_DONTCARE;
    rec->expected.bx    = REF_DONTCARE;
    rec->expected.cx    = REF_DONTCARE;
    rec->expected.dx    = REF_DONTCARE;
    rec->expected.si    = REF_DONTCARE;
    rec->expected.di    = REF_DONTCARE;
    rec->expected.ds    = REF_DONTCARE;
    rec->expected.es    = REF_DONTCARE;
    rec->expected.flags = REF_DONTCARE;
    rec->care_mask = CARE_NONE;
    rec->out_buf   = NULL;
    rec->out_len   = 0;
    rec->next      = NULL;
    return rec;
}

void tc_set_buf(TC_RECORD *rec,
                const unsigned char *ptr,
                unsigned short len)
{
    if (rec->out_buf) {
        free(rec->out_buf);
        rec->out_buf = NULL;
        rec->out_len = 0;
    }
    if (!ptr || len == 0) return;

    rec->out_buf = (unsigned char *)malloc(len);
    if (!rec->out_buf) return;
    memcpy(rec->out_buf, ptr, len);
    rec->out_len = len;
}

void tc_add(TC_LIST *list, TC_RECORD *rec)
{
    rec->next = NULL;
    if (!list->root) {
        list->root = rec;
        list->tail = rec;
    } else {
        list->tail->next = rec;
        list->tail = rec;
    }
    list->count++;
}

TC_RECORD *tc_find(TC_LIST *list, const char *name)
{
    TC_RECORD *cur = list->root;
    while (cur) {
        if (strcmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

void tc_free(TC_RECORD *rec)
{
    if (!rec) return;
    if (rec->name)    free(rec->name);
    if (rec->out_buf) free(rec->out_buf);
    free(rec);
}

void tc_free_all(TC_LIST *list)
{
    TC_RECORD *cur = list->root;
    while (cur) {
        TC_RECORD *nxt = cur->next;
        tc_free(cur);
        cur = nxt;
    }
    list->root  = NULL;
    list->tail  = NULL;
    list->count = 0;
}

/* -----------------------------------------------------------------------
 * testlib_init / testlib_finish
 * --------------------------------------------------------------------- */

int testlib_init(int argc, char **argv)
{
    char fname[20];
    int  i;

    memset(&g_tclist, 0, sizeof(g_tclist));
    g_ref_mode = 0;     /* default: test mode */

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == 'r' || argv[i][0] == 'R') {  /* ref */
            g_ref_mode = 1;
        }
    }

    if (g_ref_mode) {
        sprintf(fname, "%s.ref", g_tbasename);
        g_ref_fp = fopen(fname, "w");
        if (!g_ref_fp) {
            fprintf(stderr, "testlib: cannot open %s for writing\n", fname);
            return -1;
        }
        fprintf(g_ref_fp,
            "# INT 2F/AH=12h subfunction %s reference file\n", g_tbasename);
        fprintf(g_ref_fp,
            "# Register line: NAME AX BX CX DX SI DI DS ES FLAGS\n");
        fprintf(g_ref_fp,
            "#   Each field: 4-digit hex, or ???? (don't-care)\n");
        fprintf(g_ref_fp,
            "#   Byte-level: ??HH = care low byte only; HH?? = care high byte only\n");
        fprintf(g_ref_fp,
            "# Buffer line:  B NAME COUNT HH HH ...\n");
        fprintf(g_ref_fp, "#\n");
        printf("=== INT 2F/12h test %s  [REFERENCE MODE] ===\n\n", g_tbasename);
    } else {
        sprintf(fname, "%s.ref", g_tbasename);
        if (ref_load(fname, &g_tclist) < 0) {
            fprintf(stderr, "testlib: cannot load %s\n", fname);
            return -1;
        }
        sprintf(fname, "%s.tst", g_tbasename);
        g_tst_fp = fopen(fname, "w");
        if (!g_tst_fp) {
            fprintf(stderr, "testlib: cannot open %s for writing\n", fname);
            return -1;
        }
        fprintf(g_tst_fp,
            "# INT 2F/AH=12h subfunction %s test results\n", g_tbasename);
        fprintf(g_tst_fp,
            "# Format: PASS/FAIL NAME [FIELD EXP GOT]\n");
        fprintf(g_tst_fp, "#\n");
        printf("=== INT 2F/12h test %s  [TEST MODE] ===\n\n", g_tbasename);
    }

    return 0;
}

void testlib_finish(void)
{
    if (g_ref_fp) { fflush(g_ref_fp); fclose(g_ref_fp); g_ref_fp = NULL; }
    if (g_tst_fp) { fflush(g_tst_fp); fclose(g_tst_fp); g_tst_fp = NULL; }
    tc_free_all(&g_tclist);
}

/* -----------------------------------------------------------------------
 * probe_int2f
 * --------------------------------------------------------------------- */

void probe_int2f(PROBE far *p)
{
    unsigned short ax_in    = p->before.ax;
    unsigned short bx_in    = p->before.bx;
    unsigned short cx_in    = p->before.cx;
    unsigned short dx_in    = p->before.dx;
    unsigned short si_in    = p->before.si;
    unsigned short di_in    = p->before.di;
    unsigned short ds_in    = p->before.ds;
    unsigned short es_in    = p->before.es;
    unsigned short value_in = p->value;

    unsigned short ax_out, bx_out, cx_out, dx_out;
    unsigned short si_out, di_out, ds_out, es_out, flags_out;

    unsigned short dos_ds = get_dos_data_segment();
    unsigned short my_sp, my_new_sp;

    if (!ds_in) {
        ds_in = dos_ds;
        p->before.ds = ds_in;
    }

    _asm {

        /* Save harness segments (push DS then ES) */
        push ds
        push es

        /* save our SP */
        mov  my_sp, sp
    }
    printf("Probing: SP=%04X\n", my_sp);
    _asm {

        /* Load input registers */
        mov  bx, bx_in
        mov  cx, cx_in
        mov  dx, dx_in
        mov  si, si_in
        mov  di, di_in

        /* Load DS and ES */
        mov  ax, es_in
        mov  es, ax
        mov  ax, ds_in
        mov  ds, ax

#if USEDOSSTACK /* assumes tiny mode so CS=DS=SS */
        cli
        /* update to DOS stack */
        mov  ax, cs:dos_ds
        mov  ss, ax
        mov  sp, 0x580 /* roughly top of stack */
        /* now SS=DS=dos segment, SP = safe spot in DOS kernel */
        sti

        /* Push optional stack word (parameter for INT2F) */
        push cs:value_in

        /* Load AX */
        mov  ax, cs:ax_in
#else
        /* Push optional stack word (parameter for INT2F) */
        push ss:value_in

        /* Load AX */
        mov  ax, ss:ax_in  
#endif

        /* invoke DOS interrupt */
        int  0x2F

        /* restore stack and program segments but with
        care to avoid overwriting results */
#if USEDOSSTACK /* assumes tiny mode so CS=DS=SS */
        pushf
        pop  cs:flags_out

        mov  cs:ax_out, ax  /* save current AX so can use it   */
        mov  ax, cs         /* !!! only true in tiny model     */
        cli
        mov ss, ax      /* we don't care about DOS stack   */
        mov sp, ss:my_sp
        sti
    
        /* value_in was on temp stack, so nothing to pop off   */
#else
        pushf
        pop  ss:flags_out

        mov ss:ax_out, ax

        /* Discard the parameter we pushed */
        add  sp, 2        ; removes value_in (pop ??)
#endif

        mov ss:my_new_sp, sp

        /* capture outputs */

        mov  ss:bx_out, bx
        mov  ss:cx_out, cx
        mov  ss:dx_out, dx
        mov  ss:si_out, si
        mov  ss:di_out, di
        mov  ss:ds_out, ds
        mov  ss:es_out, es    

        /* Restore harness segments in reverse order */
        pop  es
        pop  ds
    }

    p->after.ax    = ax_out;
    p->after.bx    = bx_out;
    p->after.cx    = cx_out;
    p->after.dx    = dx_out;
    p->after.si    = si_out;
    p->after.di    = di_out;
    p->after.ds    = ds_out;
    p->after.es    = es_out;
    p->after.flags = flags_out;
    
    if (my_new_sp != my_sp) printf("STACK CORRUPT!\n");
    else printf("STACK OK\n");
}

/* -----------------------------------------------------------------------
 * Verbose stdout output helpers
 * --------------------------------------------------------------------- */

void print_section(const char *label)
{
    printf("--- %s ---\n", label);
}

void print_call(unsigned short ax, unsigned short value, const char *name)
{
    printf("CALL  AX=%04X [%04X=%d='%c'] (%s)\n", ax, value, value, value&0xFF, name);
}

static void print_one_reg(const char *rname,
                          unsigned short before,
                          unsigned short after)
{
    if (before != after)
        printf("  %-5s  %04X -> %04X  *CHANGED*\n", rname, before, after);
    else
        printf("  %-5s  %04X\n", rname, after);
}

void print_regs(const REGS16 far *b, const REGS16 far *a)
{
    printf("REGS:\n");
    print_one_reg("AX",    b->ax,    a->ax);
    print_one_reg("BX",    b->bx,    a->bx);
    print_one_reg("CX",    b->cx,    a->cx);
    print_one_reg("DX",    b->dx,    a->dx);
    print_one_reg("SI",    b->si,    a->si);
    print_one_reg("DI",    b->di,    a->di);
    print_one_reg("DS",    b->ds,    a->ds);
    print_one_reg("ES",    b->es,    a->es);
}

static void print_one_flag(const char *fname,
                           unsigned short mask,
                           unsigned short before,
                           unsigned short after)
{
    int bv = (before & mask) ? 1 : 0;
    int av = (after  & mask) ? 1 : 0;
    if (bv != av)
        printf("  %-4s  %d -> %d  *CHANGED*\n", fname, bv, av);
    else
        printf("  %-4s  %d\n", fname, av);
}

void print_flags(unsigned short before, unsigned short after)
{
    printf("FLAGS:\n");
    print_one_flag("CF",  FLAG_CF,  before, after);
    print_one_flag("PF",  FLAG_PF,  before, after);
    print_one_flag("AF",  FLAG_AF,  before, after);
    print_one_flag("ZF",  FLAG_ZF,  before, after);
    print_one_flag("SF",  FLAG_SF,  before, after);
    print_one_flag("OF",  FLAG_OF,  before, after);
}

void print_mem_dump(unsigned char far *ptr, unsigned short len, signed short max_dump_len)
{
    unsigned short i;

    if (!ptr || !len) {
        printf("MEM:  (none)\n");
        return;
    }
    printf("MEM:  %04X:%04X  len=%u\n",
           FP_SEG(ptr), FP_OFF(ptr), len);
           
    /* don't print out more than buffer size, up to max requested,
       where a negative value indicates print whole buffer */
    if (max_dump_len < len && max_dump_len >= 0) len = max_dump_len;
    for (i = 0; i < len; i++) {
        if (i && (i % 16) == 0) printf("\n");
        if ((i % 16) == 0)      printf("  %04X: ", i);
        printf("%02X ", ptr[i]);
    }
    printf("\n");
}

void print_end(void)
{
    printf("END\n\n");
}

void print_probe_verbose(const PROBE *p, const char *name, signed short max_dump_len)
{
    print_call(p->before.ax, p->value, name);
    print_regs(&p->before, &p->after);
    print_flags(p->before.flags, p->after.flags);
    print_mem_dump(p->buffer, p->bufsize, max_dump_len);
    print_end();
}

/* -----------------------------------------------------------------------
 * .ref file I/O
 * --------------------------------------------------------------------- */

/*
 * Write one 4-character register field to fp.
 * If the full-word care bit is set: write 4 hex digits.
 * If only the low-byte care bit is set: write "??HH".
 * If only the high-byte care bit is set: write "HH??".
 * If neither: write "????".
 */
static void write_reg_field(FILE *fp,
                            unsigned short val,
                            unsigned short care_hi_lo_mask)
{
    /* printf("val is %04X, mask is %04X\n", val, care_hi_lo_mask); */
    
    if (care_hi_lo_mask & 0x02) 
        fprintf(fp, "%02X", (val >> 8) & 0xFF);
    else
        fprintf(fp, "??");
    if (care_hi_lo_mask & 0x01)
        fprintf(fp, "%02X", val & 0xFF);
    else
        fprintf(fp, "??");
}

void ref_write(const char *name,
               const REGS16 far *after,
               unsigned short care_mask)
{
    if (!g_ref_fp) return;

    fprintf(g_ref_fp, "%s ", name);

    write_reg_field(g_ref_fp, after->ax, care_mask & CARE_AX);
    fputc(' ', g_ref_fp);
    write_reg_field(g_ref_fp, after->bx, (care_mask & CARE_BX)>>2);
    fputc(' ', g_ref_fp);
    write_reg_field(g_ref_fp, after->cx, (care_mask & CARE_CX)>>4);
    fputc(' ', g_ref_fp);
    write_reg_field(g_ref_fp, after->dx, (care_mask & CARE_DX)>>6);
    fputc(' ', g_ref_fp);

    /* SI, DI, DS, ES: full word or don't-care only */
    if (care_mask & CARE_SI)    fprintf(g_ref_fp, "%04X", after->si);
    else                        fprintf(g_ref_fp, "????");
    fputc(' ', g_ref_fp);

    if (care_mask & CARE_DI)    fprintf(g_ref_fp, "%04X", after->di);
    else                        fprintf(g_ref_fp, "????");
    fputc(' ', g_ref_fp);

    if (care_mask & CARE_DS)    fprintf(g_ref_fp, "%04X", after->ds);
    else                        fprintf(g_ref_fp, "????");
    fputc(' ', g_ref_fp);

    if (care_mask & CARE_ES)    fprintf(g_ref_fp, "%04X", after->es);
    else                        fprintf(g_ref_fp, "????");
    fputc(' ', g_ref_fp);

    if (care_mask & CARE_FLAGS) fprintf(g_ref_fp, "%04X", after->flags);
    else                        fprintf(g_ref_fp, "????");

    fputc('\n', g_ref_fp);
}

void ref_write_buf(const char *name,
                   const unsigned char far *ptr,
                   unsigned short len)
{
    unsigned short i;
    if (!g_ref_fp) return;
    fprintf(g_ref_fp, "B %s %u", name, (unsigned)len);
    for (i = 0; i < len; i++)
        fprintf(g_ref_fp, " %02X", ptr[i]);
    fputc('\n', g_ref_fp);
}

/*
 * Parse a single register field from a .ref line.
 * Handles "????", "HHHH", "??HH" (lo only), "HH??" (hi only).
 * Returns pointer past the field (always consumes exactly 4 chars + leading ws).
 * *out  : parsed value; 0 if lo-only (high byte unknown), val<<8 if hi-only.
 *         REF_DONTCARE if "????".
 * *care_mask is 1 for low byte OR'd 2 for high byte a number (not ?)
 */
static const char *parse_reg_field(const char *p,
                                   unsigned short *out,
                                   unsigned short *care_mask)
{
    unsigned char hi_val = (REF_DONTCARE>>8)&0xFFU, lo_val = REF_DONTCARE&0xFFU;
    int hi_ok, lo_ok;

    *care_mask = 0;
    *out    = REF_DONTCARE;

    while (*p == ' ' || *p == '\t') p++;

    /* High byte: chars 0-1 */
    hi_ok = (isxdigit((unsigned char)p[0]) && isxdigit((unsigned char)p[1]));
    if (hi_ok) {
        char tmp[3]; 
        tmp[0]=p[0]; tmp[1]=p[1]; tmp[2]='\0';
        hi_val = (unsigned char)strtoul(tmp, NULL, 16);
        *care_mask |= 0x02;
    }

    /* Low byte: chars 2-3 */
    lo_ok = (isxdigit((unsigned char)p[2]) && isxdigit((unsigned char)p[3]));
    if (lo_ok) {
        char tmp[3]; 
        tmp[0]=p[2]; tmp[1]=p[3]; tmp[2]='\0';
        lo_val = (unsigned char)strtoul(tmp, NULL, 16);
        *care_mask |= 0x01;
    }

    *out = ((unsigned short)hi_val << 8) | lo_val;

    return p + 4;
}

int ref_load(const char *filename, TC_LIST *list)
{
    FILE      *fp;
    char      *line;
    int        count = 0;
    TC_RECORD *last  = NULL;    /* most recently added register record */

    line = (char *)malloc(REF_LINE_MAX);
    if (!line) return -1;

    fp = fopen(filename, "r");
    if (!fp) { free(line); return -1; }

    while (fgets(line, REF_LINE_MAX, fp)) {
        const char    *p = line;
        TC_RECORD     *rec;
        unsigned short vals[9];         /* parsed register values            */
        unsigned short mask;
        unsigned short care = CARE_NONE;
        int            i;
        char           name_buf[128];

        /* Skip leading whitespace */
        while (*p == ' ' || *p == '\t') p++;
        /* Skip comments and blank lines */
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        /* ---- Buffer line: "B NAME COUNT HH HH ..." ---- */
        if (p[0] == 'B' && (p[1] == ' ' || p[1] == '\t')) {
            unsigned short bcount;
            unsigned char *bbuf;

            p += 2;
            while (*p == ' ' || *p == '\t') p++;

            /* Read name */
            i = 0;
            while (*p && *p != ' ' && *p != '\t' && i < (int)sizeof(name_buf)-1)
                name_buf[i++] = *p++;
            name_buf[i] = '\0';

            /* Read count */
            while (*p == ' ' || *p == '\t') p++;
            bcount = (unsigned short)strtoul(p, (char **)&p, 10);

            if (bcount == 0 || !last) continue;

            bbuf = (unsigned char *)malloc(bcount);
            if (!bbuf) continue;

            for (i = 0; i < (int)bcount; i++) {
                while (*p == ' ' || *p == '\t') p++;
                bbuf[i] = (unsigned char)strtoul(p, (char **)&p, 16);
            }

            tc_set_buf(last, bbuf, bcount);
            free(bbuf);
            continue;
        }

        /* ---- Register line: NAME AX BX CX DX SI DI DS ES FLAGS ---- */
        i = 0;
        while (*p && *p != ' ' && *p != '\t' && i < (int)sizeof(name_buf)-1)
            name_buf[i++] = *p++;
        name_buf[i] = '\0';

        /* Parse AX/BX/CX/DX with byte-level awareness */
        for (i = 0; i < 4; i++) {
            p = parse_reg_field(p, &vals[i], &mask);
            care |= (mask << (i*2));  /* AX-DX 2bits each for AL/AH-DL/DH */
        }

        /* Parse SI/DI/DS/ES/FLAGS -- full word or don't-care only */
        for (i = 4; i < 9; i++) {
            p = parse_reg_field(p, &vals[i], &mask);
            care |= (mask << (8+i-4)); /* remaining registers 1 bit each in 2nd byte */
        }

        rec = tc_create(name_buf);
        if (!rec) continue;

        rec->expected.ax    = vals[0];
        rec->expected.bx    = vals[1];
        rec->expected.cx    = vals[2];
        rec->expected.dx    = vals[3];
        rec->expected.si    = vals[4];
        rec->expected.di    = vals[5];
        rec->expected.ds    = vals[6];
        rec->expected.es    = vals[7];
        rec->expected.flags = vals[8];

        rec->care_mask = care;

        tc_add(list, rec);
        last = rec;
        count++;
    }

    fclose(fp);
    free(line);
    return count;
}

/* -----------------------------------------------------------------------
 * Test mode comparison
 * --------------------------------------------------------------------- */

/*
 * Emit one mismatch line to stdout and g_tst_fp.
 */
static void tst_mismatch(const char *rname, const char *suffix,
                         unsigned short exp, unsigned short got,
                         int byte_wide, int *failed)
{
    if (byte_wide) {
        printf("  MISMATCH %s%s  exp=%02X  got=%02X\n",
               rname, suffix, exp & 0xFF, got & 0xFF);
        if (g_tst_fp)
            fprintf(g_tst_fp, "  MISMATCH %s%s  exp=%02X  got=%02X\n",
                    rname, suffix, exp & 0xFF, got & 0xFF);
    } else {
        printf("  MISMATCH %s  exp=%04X  got=%04X\n", rname, exp, got);
        if (g_tst_fp)
            fprintf(g_tst_fp, "  MISMATCH %s  exp=%04X  got=%04X\n",
                    rname, exp, got);
    }
    *failed = 1;
}

/*
 * Compare one AX/BX/CX/DX register using its two byte-level care bits.
 * hi_bit / lo_bit: the CARE_xH / CARE_xL constants for this register.
 * exp / got: full 16-bit values (even if only one byte is compared).
 */
static void tst_field_byte(const char *rname,
                           unsigned short exp,
                           unsigned short got,
                           unsigned short care_mask,
                           unsigned short hi_bit,
                           unsigned short lo_bit,
                           int *failed)
{
    int has_hi = (care_mask & hi_bit);
    int has_lo = (care_mask & lo_bit);
    
    /*
    printf("TESTING BYTE for (%s) - expected:%04X, got:%04X, care_mask:%04X, hi_bit:%04X, lo_bit:%04X, AH:%04X, AL:%04X\n",
            rname, exp, got, care_mask, hi_bit, lo_bit, (hi_bit & CARE_AH), (lo_bit & CARE_AL));
    */

    if (!has_hi && !has_lo) return;     /* register not in care set */

    if (has_hi && has_lo) {
        /* Full word */
        if (exp != got)
            tst_mismatch(rname, "", exp, got, 0, failed);
    } else if (has_hi) {
        /* High byte only */
        if (((exp >> 8) & 0xFF) != ((got >> 8) & 0xFF))
            tst_mismatch(rname, "(H)", exp >> 8, got >> 8, 1, failed);
    } else {
        /* Low byte only */
        if ((exp & 0xFF) != (got & 0xFF))
            tst_mismatch(rname, "(L)", exp, got, 1, failed);
    }
}

/* Full-word field: SI/DI/DS/ES/FLAGS */
static void tst_field_full(const char *rname,
                           unsigned short exp,
                           unsigned short got,
                           unsigned short care_mask,
                           unsigned short full_bit,
                           int *failed)
{
    if (!(care_mask & full_bit)) return;
    if (exp == REF_DONTCARE)     return;
    if (exp != got)
        tst_mismatch(rname, "", exp, got, 0, failed);
}

int tst_compare(const char *name,
                const REGS16 far *after,
                const TC_RECORD *rec)
{
    int            failed = 0;
    unsigned short cm;

    if (!rec) {
        printf("  NO_REF  %s\n", name);
        if (g_tst_fp) fprintf(g_tst_fp, "NO_REF %s\n", name);
        return 0;
    }

    cm = rec->care_mask;

    tst_field_byte("AX", rec->expected.ax,    after->ax,    cm, CARE_AH, CARE_AL, &failed);
    tst_field_byte("BX", rec->expected.bx,    after->bx,    cm, CARE_BH, CARE_BL, &failed);
    tst_field_byte("CX", rec->expected.cx,    after->cx,    cm, CARE_CH, CARE_CL, &failed);
    tst_field_byte("DX", rec->expected.dx,    after->dx,    cm, CARE_DH, CARE_DL, &failed);

    tst_field_full("SI",    rec->expected.si,    after->si,    cm, CARE_SI,    &failed);
    tst_field_full("DI",    rec->expected.di,    after->di,    cm, CARE_DI,    &failed);
    tst_field_full("DS",    rec->expected.ds,    after->ds,    cm, CARE_DS,    &failed);
    tst_field_full("ES",    rec->expected.es,    after->es,    cm, CARE_ES,    &failed);
    tst_field_full("FLAGS", rec->expected.flags, after->flags, cm, CARE_FLAGS, &failed);

    if (!failed) {
        printf("PASS  %s\n", name);
        if (g_tst_fp) fprintf(g_tst_fp, "PASS %s\n", name);
        return 1;
    } else {
        printf("FAIL  %s\n", name);
        if (g_tst_fp) fprintf(g_tst_fp, "FAIL %s\n", name);
        return 0;
    }
}

int tst_compare_buf(const char *name,
                    const unsigned char far *ptr,
                    unsigned short len,
                    const TC_RECORD *rec)
{
    unsigned short i, mismatches, cmp_len;
    int            failed = 0;
    int            got_empty = (!ptr || len == 0);
    int            ref_empty = (!rec || !rec->out_buf || rec->out_len == 0);

    /* Both sides empty -> pass silently (no buffer involvement) */
    if (got_empty && ref_empty) return 1;

    if (!rec) {
        printf("  NO_REF_BUF  %s\n", name);
        if (g_tst_fp) fprintf(g_tst_fp, "NO_REF_BUF %s\n", name);
        return 0;
    }

    if (got_empty && !ref_empty) {
        printf("  FAIL_BUF  %s  got=empty  exp=%u bytes\n", name, rec->out_len);
        if (g_tst_fp)
            fprintf(g_tst_fp, "FAIL_BUF %s got=empty exp=%u bytes\n",
                    name, rec->out_len);
        return 0;
    }
    if (!got_empty && ref_empty) {
        printf("  FAIL_BUF  %s  got=%u bytes  exp=empty\n", name, len);
        if (g_tst_fp)
            fprintf(g_tst_fp, "FAIL_BUF %s got=%u bytes exp=empty\n",
                    name, len);
        return 0;
    }

    /* Length mismatch (continue to report byte mismatches too) */
    if (len != rec->out_len) {
        printf("  FAIL_BUF  %s  len: exp=%u  got=%u\n",
               name, rec->out_len, len);
        if (g_tst_fp)
            fprintf(g_tst_fp, "FAIL_BUF %s len: exp=%u got=%u\n",
                    name, rec->out_len, len);
        failed = 1;
    }

    cmp_len    = (len < rec->out_len) ? len : rec->out_len;
    mismatches = 0;
    for (i = 0; i < cmp_len && mismatches < 8; i++) {
        if (ptr[i] != rec->out_buf[i]) {
            printf("  FAIL_BUF  %s  [%u] exp=%02X  got=%02X\n",
                   name, i, rec->out_buf[i], ptr[i]);
            if (g_tst_fp)
                fprintf(g_tst_fp, "FAIL_BUF %s [%u] exp=%02X got=%02X\n",
                        name, i, rec->out_buf[i], ptr[i]);
            failed = 1;
            mismatches++;
        }
    }

    if (!failed) {
        printf("PASS_BUF  %s\n", name);
        if (g_tst_fp) fprintf(g_tst_fp, "PASS_BUF %s\n", name);
        return 1;
    }
    printf("FAIL_BUF  %s\n", name);
    if (g_tst_fp) fprintf(g_tst_fp, "FAIL_BUF %s\n", name);
    return 0;
}

/* -----------------------------------------------------------------------
 * All-in-one per-case runner
 * --------------------------------------------------------------------- */

int run_case(const char *name, PROBE *p, unsigned short care_mask, fn_run_hook *fn)
{
    int reg_ok, buf_ok;

    probe_int2f(p);
    
    if (fn != NULL) {
        if (fn(name, p, care_mask))
            return 0;
    }
	/* printf("buffer:%04X:%04X == %04X:%04X\n", FP_SEG(p->buffer), FP_OFF(p->buffer), p->after.es, p->after.di); */

    if (g_ref_mode) {
        print_probe_verbose(p, name, -1);
        ref_write(name, &p->after, care_mask);
        if (p->buffer && p->bufsize > 0)
            ref_write_buf(name, p->buffer, p->bufsize);
        return 1;
    } else {
        TC_RECORD *rec = tc_find(&g_tclist, name);
        reg_ok = tst_compare(name, &p->after, rec);
        buf_ok = tst_compare_buf(name, p->buffer, p->bufsize, rec);
        return reg_ok && buf_ok;
    }
}

/* -----------------------------------------------------------------------
 * Utilities
 * --------------------------------------------------------------------- */

void build_test_string(char far *buf, unsigned short len)
{
    static const char pat[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";
    unsigned short pat_len = (unsigned short)(sizeof(pat) - 1);
    unsigned short i;
    for (i = 0; i < len; i++)
        buf[i] = pat[i % pat_len];
    buf[len] = '\0';
}

unsigned short get_dos_data_segment(void)
{
    unsigned short dos_ds;
    _asm {
        push ds
        mov  ax, 1203h
        int  2Fh
        mov  ax, ds
        mov  dos_ds, ax
        pop  ds
    }
    return dos_ds;
}


#ifndef NO_MAIN
/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

GREATEST_MAIN_DEFS();

void suite(void); /* SUITE(suite) required in test */

int main(int argc, char **argv)
{
    int rc;

    rc = testlib_init(argc, argv);
    if (rc < 0) return EXIT_FAILURE;

    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite);

    testlib_finish();

    GREATEST_MAIN_END();  /* returns EXIT_SUCCESS if all tests pass */
}
#endif
