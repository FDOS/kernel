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
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.11  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.10  2001/04/02 23:18:30  bartoldeman
 * Misc, zero terminated device names and redirector bugs fixed.
 *
 * Revision 1.9  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.8  2001/03/21 02:56:26  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.7  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.6  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.5  2000/05/26 19:25:19  jimtabor
 * Read History file for Change info
 *
 * Revision 1.4  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.3  2000/05/17 19:15:12  jimtabor
 * Cleanup, add and fix source.
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
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
   *   Read/Write from/to remote file.
   *   SFT gets updated with the amount of bytes r/w.
   *
 */
UCOUNT Remote_RW(UWORD func, UCOUNT n, BYTE FAR * bp, sft FAR * s, COUNT FAR * err)
{

  BYTE FAR *save_dta;
  UWORD rc,
    rx;

  save_dta = dta;
  lpCurSft = (sfttbl FAR *) s;
  dta = bp;
  rx = int2f_Remote_call(func, 0, n, 0, (VOID FAR *) s, 0, (VOID FAR *) & rc);
  dta = save_dta;
  *err = -rx;
  return ((UCOUNT) rc);
}

#undef FIND_DEBUG
/*

 */
COUNT Remote_find(UWORD func, BYTE FAR * name, REG dmatch FAR * dmp )
{
  COUNT i;
  char FAR *p;
  VOID FAR * test;
  struct dirent FAR *SDp = (struct dirent FAR *) &SearchDir;

  if (func == REM_FINDFIRST)
  {
    test = (VOID FAR *) current_ldt;
    i = truename(name, PriPathName, FALSE);
    if (i != SUCCESS) {
	    return i;
    }
#if defined(FIND_DEBUG)
    printf("Remote Find: n='");
    p = name;  while(*p)  printf("%c", *p++);
    printf("' p='");
    p = PriPathName;  while(*p)  printf("%c", *p++);
    printf("'\n");
#endif
  }
  else
    test = (VOID FAR *) &TempBuffer;

  fmemcpy((BYTE FAR *) &TempBuffer, dta, 21);
  p = dta;
  dta = (BYTE FAR *) &TempBuffer;
  i = int2f_Remote_call(func, 0, 0, 0, test, 0, 0);
  dta = p;
  fmemcpy(dta, (BYTE FAR *) &TempBuffer, 21);

  if (i != 0)
    return i;

  dmp->dm_attr_fnd = (BYTE) SDp->dir_attrib;
  dmp->dm_time = SDp->dir_time;
  dmp->dm_date = SDp->dir_date;
  dmp->dm_size = (LONG) SDp->dir_size;

  ConvertName83ToNameSZ((BYTE FAR *) dmp->dm_name, (BYTE FAR *) SDp->dir_name);
  return i;
}



