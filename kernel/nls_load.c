/****************************************************************/
/*                                                              */
/*                            nls_load.c                        */
/*                           FreeDOS                            */
/*                                                              */
/*    National Languge Support functions and data structures    */
/*    Load an entry from FreeDOS COUNTRY.SYS file.				*/
/*                                                              */
/*                   Copyright (c) 2000                         */
/*                         Steffen Kaiser                       */
/*                      All Rights Reserved                     */
/*                                                              */
/* This file is part of FreeDOS.                                */
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

#include "portab.h"
#include "init-mod.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id$";
#endif

/** Setup the environment for shared source NLS_LOAD.SRC **/
/**ska obsoleted #define cfgMemory Config.cfgCSYS_memory */
/**ska obsoleted #define cfgFilename Config.cfgCSYS_fnam */
#define cfgFilename nlsInfo.fname				/* char FAR * */
/**ska obsoleted #define cfgCountry Config.cfgCSYS_cntry */
/**ska obsoleted #define cfgCodepage Config.cfgCSYS_cp */
#define cfgData Config.cfgCSYS_data /* struct nlsCSys_loadPackage FAR * */
#define getMem(bytes) KernelAlloc(bytes)
#define openSYSFile(filename) open(filename, 0)	/* read-only, binary */
#define nlsStartOfChain nlsInfo.chain
#define upCaseFct CharMapSrvc

#include "nls_load.src"
