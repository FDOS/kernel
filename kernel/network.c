/****************************************************************/
/*                                                              */
/*                          network.c                           */
/*                            DOS-C                             */
/*                                                              */
/*                 Networking Support functions                 */
/*                                                              */
/*                   Copyright (c) 1995, 1999                   */
/*                         James Tabor                          */
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

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *RcsId =
    "$Id$";
#endif

/* see RBIL D-2152 and D-215D06 before attempting
   to change these two functions!
 */
UWORD get_machine_name(BYTE FAR * netname)
{
  fmemcpy(netname, &net_name, 16);
  return (NetBios);
}

VOID set_machine_name(BYTE FAR * netname, UWORD name_num)
{
  NetBios = name_num;
  fmemcpy(&net_name, netname, 15);
  net_set_count++;
}

/*
 * Log: network.c,v - for newer entries see "cvs log network.c"
 *
 * Revision 1.5  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.4  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.3  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/09/23 04:40:48  jprice
 * *** empty log message ***
 *
 */
