/* Used by `proto.h'. */
#define IN_INIT_MOD
/*
 * The null macro `INIT' can be used to allow the reader to differentiate
 * between functions defined in `INIT_TEXT' and those defined in `_TEXT'.
 */
#define INIT
/*
 * Functions in `INIT_TEXT' may need to call functions in `_TEXT'. The entry
 * calls for the latter functions therefore need to be wrapped up with far
 * entry points.
 */
#define DosExec     reloc_call_DosExec
#define DosMemAlloc	reloc_call_DosMemAlloc
#define execrh      reloc_call_execrh
#define fatal       reloc_call_fatal
#define fmemcpy     reloc_call_fmemcpy
#define  memcpy     reloc_call_memcpy
#define fmemset     reloc_call_fmemset
#define printf      reloc_call_printf
#define strcpy      reloc_call_strcpy
#define sti         reloc_call_sti
#define strcmp      reloc_call_strcmp
#define strlen      reloc_call_strlen
#define WritePCClock	reloc_call_WritePCClock
#define DaysFromYearMonthDay reloc_call_DaysFromYearMonthDay
#define p_0         reloc_call_p_0

