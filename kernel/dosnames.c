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

const char _DirWildNameChars[] = "*?./\\\"[]:|<>+=;,";

STATIC BOOL charOK(const char *s, const char c)
{
  return (UBYTE)c >= ' ' && !strchr(s, c);
}

/*
    MSD durring an FindFirst search string looks like this;
    (*), & (.)  == Current directory *.*
    (\)         == Root directory *.*
    (..)        == Back one directory *.*

    This always has a "truename" as input, so we may do some shortcuts

    returns number of characters in the directory component (up to the
    last backslash, including d:) or negative if error
 */
int ParseDosName(const char *filename, char *fcbname, BOOL bAllowWildcards)
{
  int nDirCnt, nFileCnt, nExtCnt;
  const char *lpszLclDir, *lpszLclFile, *lpszLclExt;

  /* Initialize the users data fields                             */
  nDirCnt = nFileCnt = nExtCnt = 0;

  /* NB: this code assumes ASCII                          */

  /* Now see how long a directory component we have.              */
  lpszLclDir = lpszLclFile = filename;
  filename += 2;
  
  while (charOK(_DirWildNameChars + 5, *filename))
    if (*filename++ == '\\')
      lpszLclFile = filename;
  nDirCnt = lpszLclFile - lpszLclDir;

  /* Parse out the file name portion.                             */
  filename = lpszLclFile;
  while (charOK(bAllowWildcards ? _DirWildNameChars + 2
                                : _DirWildNameChars, *filename))
  {
    ++nFileCnt;
    ++filename;
  }

  if (nFileCnt == 0)
  {
    if (bAllowWildcards && *filename == '\0' &&
        (nDirCnt == 3 || filename[-1] != '\\'))
        /* D:\ or D:\DOS but not D:\DOS\ */
      return DE_NFILES;
    return DE_PATHNOTFND;
  }

  /* Now we have pointers set to the directory portion and the    */
  /* file portion.  Now determine the existance of an extension.  */
  lpszLclExt = filename;
  if ('.' == *filename)
  {
    lpszLclExt = ++filename;
    while (*filename)
    {
      if (charOK(bAllowWildcards ? _DirWildNameChars + 2
                                 : _DirWildNameChars, *filename))
      {
        ++nExtCnt;
        ++filename;
      }
      else
      {
        return DE_FILENOTFND;
      }
    }
  }
  else if (*filename)
    return DE_FILENOTFND;

  /* Finally copy whatever the user wants extracted to the user's */
  /* buffers.                                                     */
  memset(fcbname, ' ', FNAME_SIZE + FEXT_SIZE);
  memcpy(fcbname, lpszLclFile, nFileCnt);
  memcpy(&fcbname[FNAME_SIZE], lpszLclExt, nExtCnt);

  /* Clean up before leaving                              */

  return nDirCnt;
}

