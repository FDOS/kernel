#if defined (_MSC_VER) || defined(_QC) || defined(__WATCOMC__) || defined(__GNUC__)
#pragma pack()
#elif defined (__ZTC__)
#pragma ZTC align
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#pragma option -a.
#endif
