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

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))
#define DirChar(c)  (((unsigned char)(c)) >= ' ' && \
                     !strchr(_DirWildNameChars+5, (c)))
#define WildChar(c) (((unsigned char)(c)) >= ' ' && \
                     !strchr(_DirWildNameChars+2, (c)))
#define NameChar(c) (((unsigned char)(c)) >= ' ' && \
                     !strchr(_DirWildNameChars, (c)))

VOID XlateLcase(BYTE * szFname, COUNT nChars);
VOID DosTrimPath(BYTE * lpszPathNamep);

/* Should be converted to a portable version after v1.0 is released.    */
#if 0
VOID XlateLcase(BYTE * szFname, COUNT nChars)
{
  while (nChars--)
  {
    if (*szFname >= 'a' && *szFname <= 'z')
      *szFname -= ('a' - 'A');
    ++szFname;
  }
}
#endif

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
  
  while (DirChar(*filename))
  {
    if (*filename == '\\')
      lpszLclFile = filename + 1;
    ++filename;
  }
  nDirCnt = lpszLclFile - lpszLclDir;
  /* Fix lengths to maximums allowed by MS-DOS.                   */
  if (nDirCnt > PARSE_MAX - 1)
    nDirCnt = PARSE_MAX - 1;

  /* Parse out the file name portion.                             */
  filename = lpszLclFile;
  while (bAllowWildcards ? WildChar(*filename) :
         NameChar(*filename))
  {
    ++nFileCnt;
    ++filename;
  }

  if (nFileCnt == 0)
  {
/* Lixing Yuan Patch */
    if (bAllowWildcards)        /* for find first */
    {
      if (*filename != '\0')
        return DE_FILENOTFND;
      if (nDirCnt == 1)         /* for d:\ */
        return DE_NFILES;
      memset(fcbname, '?', FNAME_SIZE + FEXT_SIZE);
      return nDirCnt;
    }
    else
      return DE_FILENOTFND;
  }
  
  /* Now we have pointers set to the directory portion and the    */
  /* file portion.  Now determine the existance of an extension.  */
  lpszLclExt = filename;
  if ('.' == *filename)
  {
    lpszLclExt = ++filename;
    while (*filename)
    {
      if (bAllowWildcards ? WildChar(*filename) :
          NameChar(*filename))
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

/*
 * Log: dosnames.c,v - for newer log entries do "cvs log dosnames.c"
 *
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.4  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.3  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:54  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/02 04:40:49  jprice
 * Steve Miller fixed a bug with doing "cd ." would lock the machine.
 *
 * Revision 1.3  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.8   22 Jan 1998  4:09:00   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.7   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.6   03 Jan 1998  8:36:04   patv
 * Converted data area to SDA format
 *
 *    Rev 1.5   16 Jan 1997 12:46:36   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.4   29 May 1996 21:15:12   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:20:08   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:48:44   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:05:56   patv
 * Initial revision.
 *
 */
