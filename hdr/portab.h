/****************************************************************/
/*                                                              */
/*                           portab.h                           */
/*                                                              */
/*                 DOS-C portability typedefs, etc.             */
/*                                                              */
/*                         May 1, 1995                          */
/*                                                              */
/*                      Copyright (c) 1995                      */
/*                      Pasquale J. Villani                     */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of DOS-C.                                  */
/*                                                              */
/* DOS-C is free software; you can redistribute it and/or       */
/* modify it under the terms of the GNU General Public License  */
/* as published by the Free Software Foundation; either version */
/* 2, or (at your option) any later version.                    */
/*                                                              */
/* DOS-C is distributed in the hope that it will be useful, but */
/* WITHOUT ANY WARRANTY; without even the implied warranty of   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See    */
/* the GNU General Public License for more details.             */
/*                                                              */
/* You should have received a copy of the GNU General Public    */
/* License along with DOS-C; see the file COPYING.  If not,     */
/* write to the Free Software Foundation, 675 Mass Ave,         */
/* Cambridge, MA 02139, USA.                                    */
/****************************************************************/

#ifdef MAIN
#ifdef VERSION_STRINGS
static char *portab_hRcsId =
    "$Id: portab.h 1121 2005-03-15 15:25:08Z perditionc $";
#endif
#endif

/****************************************************************/
/*                                                              */
/* Machine dependant portable types. Note that this section is  */
/* used primarily for segmented architectures. Common types and */
/* types used relating to segmented operations are found here.  */
/*                                                              */
/* Be aware that segmented architectures impose on linear       */
/* architectures because they require special types to be used  */
/* throught the code that must be reduced to empty preprocessor */
/* replacements in the linear machine.                          */
/*                                                              */
/* #ifdef <segmeted machine>                                    */
/* # define FAR far                                             */
/* # define NEAR near                                           */
/* #endif                                                       */
/*                                                              */
/* #ifdef <linear machine>                                      */
/* # define FAR                                                 */
/* # define NEAR                                                */
/* #endif                                                       */
/*                                                              */
/****************************************************************/

                                                        /* commandline overflow - removing -DI86 TE */
#if defined(__TURBOC__)

#define I86
#define CDECL   cdecl
#if __TURBOC__ > 0x202
/* printf callers do the right thing for tc++ 1.01 but not tc 2.01 */
#define VA_CDECL
#else
#define VA_CDECL cdecl
#endif
#define PASCAL  pascal
void __int__(int);
#ifndef FORSYS
void __emit__(char, ...);
#define disable() __emit__(0xfa)
#define enable() __emit__(0xfb)
#endif

#elif defined	(_MSC_VER)

#define I86
#define asm __asm
#pragma warning(disable: 4761) /* "integral size mismatch in argument;
                                   conversion supplied" */
#define CDECL   _cdecl
#define VA_CDECL
#define PASCAL  pascal
#define __int__(intno) asm int intno;
#define disable() asm cli
#define enable() asm sti
#define _CS getCS()
static unsigned short __inline getCS(void)
{
  asm mov ax, cs;
}
#define _SS getSS()
static unsigned short __inline getSS(void)
{
  asm mov ax, ss;
}

#elif defined(__WATCOMC__) && defined(BUILD_UTILS)
  /* workaround for building some utils with OpenWatcom (owcc) */
#define MC68K
#elif defined(__WATCOMC__)      /* don't know a better way */

#if defined(_M_I86)
#define I86
#endif
#define __int__(intno) asm int intno;
void disable(void);
#pragma aux disable = "cli" modify exact [];
void enable(void);
#pragma aux enable = "sti" modify exact [];
#define asm __asm
#define far __far
#define CDECL   __cdecl
#define VA_CDECL
#define PASCAL  pascal
#define _CS getCS()
unsigned short getCS(void);
#pragma aux getCS = "mov dx,cs" value [dx] modify exact[dx];
#define _SS getSS()
unsigned short getSS(void);
#pragma aux getSS = "mov dx,ss" value [dx] modify exact[dx];
#if !defined(FORSYS) && !defined(EXEFLAT) && _M_IX86 >= 300
#pragma aux default parm [ax dx cx] modify [ax dx es fs] /* min.unpacked size */
#endif

/* enable Possible loss of precision warning for compatibility with Borland */
#pragma enable_message(130)

#if _M_IX86 >= 300 || defined(M_I386)
#ifndef I386
#define I386
#endif
#endif

#elif defined (_MYMC68K_COMILER_)

#define MC68K

#elif defined(__GNUC__)

#ifdef __FAR
#define I86
#define __int__(intno) asm volatile("int " ## #intno)
static inline void disable(void)
{
  asm volatile("cli");
}
static inline void enable(void)
{
  asm volatile("sti");
}
#define far __far
#define CDECL __attribute__((cdecl))
#define VA_CDECL
#define PASCAL

#define _CS getCS()
static inline unsigned short getCS(void)
{
  unsigned short ret;
  asm volatile("mov %%cs, %0" : "=r"(ret));
  return ret;
}

#define _SS getSS()
static inline unsigned short getSS(void)
{
  unsigned short ret;
  asm volatile("mov %%ss, %0" : "=r"(ret));
  return ret;
}
extern char DosDataSeg[];
#else
/* for warnings only ! */
#define MC68K
#endif

#else
#error Unknown compiler
We might even deal with a pre-ANSI compiler. This will certainly not compile.
#endif

#ifdef I86
#if _M_IX86 >= 300 || defined(M_I386)
#ifndef I386
#define I386
#endif
#elif _M_IX86 >= 100 || defined(M_I286)
#ifndef I186
#define I186
#endif
#endif
#endif

#ifdef MC68K
#define far                     /* No far type          */
#define interrupt               /* No interrupt type    */
#define VOID           void
#define FAR                     /* linear architecture  */
#define NEAR                    /*    "        "        */
#define INRPT          interrupt
#define REG            register
#define API            int      /* linear architecture  */
#define NONNATIVE
#define PARASIZE       4096     /* "paragraph" size     */
#define CDECL
#define PASCAL
#ifdef __GNUC__
#define CONST          const
#define PROTO
typedef __SIZE_TYPE__  size_t;
#else
#define CONST
#if !(defined(_SIZE_T) || defined(_SIZE_T_DEFINED) || defined(__SIZE_T_DEFINED))
typedef unsigned       size_t;
#endif
#endif
#endif
#if defined(I86) && !defined(MC68K)
#define VOID           void
#define FAR            far      /* segment architecture */
#define NEAR           near     /*    "          "      */
#define INRPT          interrupt
#define CONST          const
#define REG            register
#define API            int far pascal   /* segment architecture */
#define NATIVE
#define PARASIZE       16       /* "paragraph" size     */
typedef unsigned       size_t;
#endif
           /* functions, that are shared between C and ASM _must_ 
              have a certain calling standard. These are declared
              as 'ASMCFUNC', and is (and will be ?-) cdecl */
#define ASMCFUNC CDECL
#define ASMPASCAL PASCAL
#if defined(__GNUC__)
#define ASM
#else
#define ASM ASMCFUNC
#endif

/* variables that can be near or far: redefined in init-dat.h */
#define DOSFAR
#define DOSTEXTFAR

/*                                                              */
/* Boolean type & definitions of TRUE and FALSE boolean values  */
/*                                                              */
typedef int BOOL;
#define FALSE           (1==0)
#define TRUE            (1==1)

/*                                                              */
/* Common pointer types                                         */
/*                                                              */
#ifndef NULL
#define NULL            0
#endif

/*                                                              */
/* Convienence defines                                          */
/*                                                              */
#define FOREVER         while(TRUE)
#ifndef max
#define max(a,b)       (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)       (((a) < (b)) ? (a) : (b))
#endif

/*                                                              */
/* Common byte, 16 bit and 32 bit types                         */
/*                                                              */
typedef char BYTE;
typedef short WORD;
typedef long DWORD;

typedef unsigned char UBYTE;
typedef unsigned short UWORD;
typedef unsigned long UDWORD;

typedef short SHORT;

typedef unsigned int BITS;      /* for use in bit fields(!)     */

typedef int COUNT;
typedef unsigned int UCOUNT;
typedef unsigned long ULONG;

#ifdef WITHFAT32
typedef unsigned long CLUSTER;
#else
typedef unsigned short CLUSTER;
#endif
typedef unsigned short UNICODE;

#if defined(STATICS) || defined(__WATCOMC__) || defined(__GNUC__)
#define STATIC static		 /* local calls inside module */
#else
#define STATIC
#endif

#ifdef UNIX
typedef char FAR *ADDRESS;
#else
typedef void FAR *ADDRESS;
#endif

#ifdef STRICT
typedef signed long LONG;
#else
#define LONG long
#endif

#define MK_UWORD(hib,lob) (((UWORD)(hib) <<  8u) | (UBYTE)(lob))
#define MK_ULONG(hiw,low) (((ULONG)(hiw) << 16u) | (UWORD)(low))

/* General far pointer macros                                           */
#ifdef I86
#ifndef MK_FP

#if defined(__WATCOMC__)
#define MK_FP(seg,ofs) 	      (((UWORD)(seg)):>((VOID *)(ofs)))
#elif defined(__TURBOC__) && (__TURBOC__ > 0x202)
#define MK_FP(seg,ofs)        ((void _seg *)(seg) + (void near *)(ofs))
#else
#define MK_FP(seg,ofs)        ((void FAR *)(((ULONG)(seg)<<16)|(UWORD)(ofs)))
#endif

#define pokeb(seg, ofs, b) (*((unsigned char far *)MK_FP(seg,ofs)) = b)
#define poke(seg, ofs, w) (*((unsigned far *)MK_FP(seg,ofs)) = w)
#define pokew poke
#define pokel(seg, ofs, l) (*((unsigned long far *)MK_FP(seg,ofs)) = l)
#define peekb(seg, ofs) (*((unsigned char far *)MK_FP(seg,ofs)))
#define peek(seg, ofs) (*((unsigned far *)MK_FP(seg,ofs)))
#define peekw peek
#define peekl(seg, ofs) (*((unsigned long far *)MK_FP(seg,ofs)))

#if defined(__TURBOC__) && (__TURBOC__ > 0x202)
#define FP_SEG(fp)            ((unsigned)(void _seg *)(void far *)(fp))
#else
#define FP_SEG(fp)            ((unsigned)((ULONG)(VOID FAR *)(fp)>>16))
#endif

#if defined(__GNUC__) && defined(__BUILTIN_IA16_FP_OFF)
#define FP_OFF(fp)            __builtin_ia16_FP_OFF(fp)
#else
#define FP_OFF(fp)            ((unsigned)(fp))
#endif

#endif
#endif

#ifdef MC68K
#define MK_FP(seg,ofs)         ((VOID *)(&(((BYTE *)(size_t)(seg))[(ofs)])))
#define FP_SEG(fp)             (0)
#define FP_OFF(fp)             ((size_t)(fp))
#endif

#if defined(__GNUC__) && defined(__FAR)
typedef VOID FAR *intvec;
#else
typedef VOID (FAR ASMCFUNC * intvec) (void);
#endif

#define MK_PTR(type,seg,ofs) ((type FAR*) MK_FP (seg, ofs))
#if __TURBOC__ > 0x202
# define MK_SEG_PTR(type,seg) ((type _seg*) (seg))
#else
# define _seg FAR
# define MK_SEG_PTR(type,seg) MK_PTR (type, seg, 0)
#endif

/*
	this suppresses the warning
	unreferenced parameter 'x'
	and (hopefully) generates no code
*/
#define UNREFERENCED_PARAMETER(x) (void)x;

#ifdef I86                      /* commandline overflow - removing /DPROTO TE */
#define PROTO
#endif

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))
