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
#define DosExec     init_call_DosExec
#define DosMemAlloc	init_call_DosMemAlloc
#define dos_close   init_call_dos_close
#define dos_getdate	init_call_dos_getdate
#define dos_gettime	init_call_dos_gettime
#define dos_open    init_call_dos_open
#define dos_read    init_call_dos_read
#define execrh      init_call_execrh
#define fatal       init_call_fatal
#define fbcopy      init_call_fbcopy
#define printf      init_call_printf
#define scopy       init_call_scopy
#define sti         init_call_sti
#define strcmp      init_call_strcmp
#define strlen      init_call_strlen
#define WritePCClock	init_call_WritePCClock
