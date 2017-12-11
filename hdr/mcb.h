/****************************************************************/
/*                                                              */
/*                           mcb.h                              */
/*                                                              */
/*     Memory Control Block data structures and declarations    */
/*                                                              */
/*                       November 23, 1991                      */
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
static BYTE *mcb_hRcsId =
    "$Id: mcb.h 822 2004-03-25 00:20:20Z bartoldeman $";
#endif
#endif

#define LARGEST         -1
#define FIRST_FIT       0
#define BEST_FIT        1
#define LAST_FIT        2
#define FIRST_FIT_UO    0x40
#define BEST_FIT_UO     0x41
#define LAST_FIT_UO     0x42
#define FIRST_FIT_U     0x80
#define BEST_FIT_U      0x81
#define LAST_FIT_U      0x82
#define FIT_U_MASK      0xc0
#define FIT_MASK        0x3f

#define MCB_NORMAL      0x4d
#define MCB_LAST        0x5a

#define DOS_PSP         0x0060  /* 0x0008 What? seg 8 =0:0080 */
#define FREE_PSP        0

#define MCB_SIZE(x)     ((((LONG)(x))<<4)+sizeof(mcb))

typedef UWORD seg;
typedef UWORD offset;

typedef struct {
  BYTE m_type;                  /* mcb type - chain or end              */
  UWORD m_psp;                  /* owner id via psp segment             */
  UWORD m_size;                 /* size of segment in paragraphs        */
  BYTE m_fill[3];
  BYTE m_name[8];               /* owner name limited to 8 bytes        */
} mcb;

