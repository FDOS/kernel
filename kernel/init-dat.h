#undef DOSFAR
#undef DOSTEXTFAR
#define DOSDATA(x) (x)
#define DOSTEXT(x) (x)

/* Included by initialisation functions */

#if _MSC_VER != 0
extern __segment DosDataSeg;    /* serves for all references to the DOS DATA segment 
                                   necessary for MSC+our funny linking model
                                 */

extern __segment DosTextSeg;

#define DOSFAR __based(DosDataSeg)
#define DOSTEXTFAR __based(DosTextSeg)

#elif defined(__TURBOC__)

#define DOSFAR FAR
#define DOSTEXTFAR FAR

#elif defined(__WATCOMC__)

#define DOSFAR FAR
#define DOSTEXTFAR FAR

#elif defined(__GNUC__)

#define DosTextSeg 0x70
/* XXX not supported yet */
#define DOSFAR
#define DOSTEXTFAR
#undef DOSDATA
#undef DOSTEXT
#define DOSDATA(x) (*(typeof(x) FAR *)MK_FP(DosDataSeg, (size_t)&(x)))
#define DOSTEXT(x) (*(typeof(x) FAR *)MK_FP(DosTextSeg, (size_t)&(x)))

#elif !defined(I86)

#define DOSFAR
#define DOSTEXTFAR

#else

#error unknown compiler - please adjust
We might even deal with a pre-ANSI compiler. This will certainly not compile.
#endif
