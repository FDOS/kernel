/****************************************************************/
/*                                                              */
/*                           debug.h                            */
/*                                                              */
/*            Routines to assist in debugging the kernel        */
/*                                                              */
/*                       January, 2005                          */
/*                                                              */
/*                      Copyright (c) 2005                      */
/*                      FreeDOS kernel dev.                     */
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

#ifndef __DEBUG_H
#define __DEBUG_H

/* #define DEBUG (usually via 'build debug') to
   enable debug support.
   NOTE: this file included by INIT time code and normal
         resident code, so use care for all memory references
 */
 
 
/* use to limit output to debug builds */
#ifdef DEBUG
#define DebugPrintf(x) printf x
#else
#define DebugPrintf(x)
#endif

/* use to disable a chunk of debug output, but
   keep around for later use.  */
#define DDebugPrintf(x)


/* enable or disable various chunks of debug output */

/* show stored IRQ vectors */
/* #define DEBUGIRQ */

/* show output related to moving kernel into HMA */
#ifdef DEBUG
#define HMAInitPrintf(x) printf x
#else
#define HMAInitPrintf(x)
#endif

/* display output during kernel config processing phase */
/* #define DEBUGCFG */
#ifdef DEBUGCFG
#define CfgDbgPrintf(x) printf x
#else
#define CfgDbgPrintf(x)
#endif

/* display info on various DOS functions (dosfns.c) */
/* #define DEBUGDOSFNS */
#ifdef DEBUGDOSFNS
#define DFnsDbgPrintf(x) printf x
#else
#define DFnsDbgPrintf(x)
#endif

/* extra debug output related to chdir */
/* #define CHDIR_DEBUG */

/* extra debug output related to findfirst */
/* #define FIND_DEBUG */

/* display info on various DOS directory functions (fatdir.c) */
/* #define DEBUGFATDIR */
#ifdef DEBUGFATDIR
#define FDirDbgPrintf(x) printf x
#else
#define FDirDbgPrintf(x)
#endif

/* extra debug output marking sections */
/* #define DISPLAY_GETBLOCK */

/* display info on various FAT handling functions (fatfs.c) */
/* #define DEBUGFATFS */
#ifdef DEBUGFATFS
#define FatFSDbgPrintf(x) printf x
#else
#define FatFSDbgPrintf(x)
#endif


/* just to be sure printf is declared */
#if defined(DEBUG) || defined(DEBUGIRQ) || defined(DEBUGCFG) || \
    defined(DEBUGDOSFNS) || defined(CHDIR_DEBUG) || defined(FIND_DEBUG) || \
    defined(DEBUGFATDIR) || defined(DEBUGFATFS)
#ifndef DEBUG_NEED_PRINTF
#define DEBUG_NEED_PRINTF
void printf(const char *, ...);
#endif
#endif

#endif /* __DEBUG_H */
