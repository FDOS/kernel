/****************************************************************/
/*                                                              */
/*                            exe.h                             */
/*                                                              */
/*                 DOS EXE Header Data Structure                */
/*                                                              */
/*                       December 1, 1991                       */
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
static BYTE *exe_hRcsId =
    "$Id$";
#endif
#endif

typedef struct {
  UWORD exSignature;
  UWORD exExtraBytes;
  UWORD exPages;
  UWORD exRelocItems;
  UWORD exHeaderSize;
  UWORD exMinAlloc;
  UWORD exMaxAlloc;
  UWORD exInitSS;
  UWORD exInitSP;
  UWORD exCheckSum;
  UWORD exInitIP;
  UWORD exInitCS;
  UWORD exRelocTable;
  UWORD exOverlay;
} exe_header;

#define MAGIC 0x5a4d
#define OLD_MAGIC 0x4d5a

