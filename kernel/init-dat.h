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

#else
#pragma error("unknown compiler - please adjust")
this should simply not compile ! !
#endif
extern BYTE DOSFAR version_flags;       /* minor version number                 */
