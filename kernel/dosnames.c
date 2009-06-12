/****************************************************************/
/*                                                              */
/*                         dosnames.c                           */
/*                           DOS-C                              */
/*                                                              */
/*    Generic parsing functions for file name specifications    */
/*                                                              */
/*                      Copyright (c) 1994                      */
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
/*                                                              */
/****************************************************************/

#include "portab.h"

#ifdef VERSION_STRINGS
static BYTE *dosnamesRcsId =
    "$Id$";
#endif

#include "globals.h"

/*
    MSD durring an FindFirst search string looks like this;
    (*), & (.)  == Current directory *.*
    (\)         == Root directory *.*
    (..)        == Back one directory *.*

    This always has a "truename" as input, so we may do some shortcuts

    returns number of characters in the directory component (up to the
    last backslash, including d:) or negative if error
 */
int ParseDosName(const char *filename, BOOL bAllowWildcards)
{
  int nDirCnt;
  const char *lpszLclDir, *lpszLclFile;

  /* Initialize the users data fields                             */
  nDirCnt = 0;

  /* NB: this code assumes ASCII                          */

  /* Now see how long a directory component we have.              */
  lpszLclDir = lpszLclFile = filename;
  filename += 2;

  while (*filename)
  {
    if (*filename == '\\')
      lpszLclFile = filename + 1;
    ++filename;
  }
  nDirCnt = lpszLclFile - lpszLclDir;

  /* Parse out the file name portion.                             */
  filename = lpszLclFile;
  while (*filename)
    ++filename;

  if (filename == lpszLclFile)
  {
    int err = DE_PATHNOTFND;
    if (bAllowWildcards && *filename == '\0' &&
        (nDirCnt == 3 || filename[-1] != '\\'))
        /* D:\ or D:\DOS but not D:\DOS\ */
      err = DE_NFILES;
    return err;
  }

  return nDirCnt;
}

