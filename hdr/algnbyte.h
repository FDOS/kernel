
#if defined(_MSC_VER)
#if _MSC_VER >= 700
#pragma warning(disable:4103)
#endif
#pragma pack(1)
#elif defined(_QC) || defined(__WATCOMC__) || defined(__GNUC__)
#pragma pack(1)
#elif defined(__ZTC__)
#pragma ZTC align 1
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a-
#endif
