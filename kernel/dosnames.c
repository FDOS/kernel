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
static BYTE *dosnamesRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
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

#include "globals.h"

#define PathSep(c) ((c)=='/'||(c)=='\\')
#define DriveChar(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z'))
#define DirChar(c) (!strchr("\"[]:|<>+=;,", (c)))
#define NameChar(c) (!strchr(".\"/\\[]:|<>+=;,*?", (c)))
#define WildChar(c) (!strchr(".\"/\\[]:|<>+=;,", (c)))

VOID XlateLcase(BYTE * szFname, COUNT nChars);
VOID DosTrimPath(BYTE FAR * lpszPathNamep);

/* Should be converted to a portable version after v1.0 is released.    */
VOID XlateLcase(BYTE * szFname, COUNT nChars)
{
  while (nChars--)
  {
    if (*szFname >= 'a' && *szFname <= 'z')
      *szFname -= ('a' - 'A');
    ++szFname;
  }
}

VOID SpacePad(BYTE * szString, COUNT nChars)
{
  REG COUNT i;

  for (i = strlen(szString); i < nChars; i++)
    szString[i] = ' ';
}

COUNT ParseDosName(BYTE FAR * lpszFileName,
                   COUNT * pnDrive,
                   BYTE * pszDir,
                   BYTE * pszFile,
                   BYTE * pszExt,
                   BOOL bAllowWildcards)
{
  COUNT nDirCnt,
    nFileCnt,
    nExtCnt;
  BYTE FAR *lpszLclDir,
    FAR * lpszLclFile,
    FAR * lpszLclExt;

  /* Initialize the users data fields                             */
  if (pszDir)
    *pszDir = '\0';
  if (pszFile)
    *pszFile = '\0';
  if (pszExt)
    *pszExt = '\0';
  lpszLclFile = lpszLclExt = lpszLclDir = 0;
  nDirCnt = nFileCnt = nExtCnt = 0;

  /* Start by cheking for a drive specifier ...                   */
  if (DriveChar(*lpszFileName) && ':' == lpszFileName[1])
  {
    /* found a drive, fetch it and bump pointer past drive  */
    /* NB: this code assumes ASCII                          */
    if (pnDrive)
    {
      *pnDrive = *lpszFileName - 'A';
      if (*pnDrive > 26)
        *pnDrive -= ('a' - 'A');
    }
    lpszFileName += 2;
  }
  else
  {
    if (pnDrive)
    {
      *pnDrive = -1;
    }
  }
  if (!pszDir && !pszFile && !pszExt)
    return SUCCESS;

  /* Now see how long a directory component we have.              */
  lpszLclDir = lpszLclFile = lpszFileName;
  while (DirChar(*lpszFileName))
  {
    if (PathSep(*lpszFileName))
      lpszLclFile = lpszFileName + 1;
    ++lpszFileName;
  }
  nDirCnt = lpszLclFile - lpszLclDir;

  /* Parse out the file name portion.                             */
  lpszFileName = lpszLclFile;
  while (bAllowWildcards ? WildChar(*lpszFileName) : NameChar(*lpszFileName))
  {
    ++nFileCnt;
    ++lpszFileName;
  }
  if (nFileCnt == 0)
    return DE_FILENOTFND;

  /* Now we have pointers set to the directory portion and the    */
  /* file portion.  Now determine the existance of an extension.  */
  lpszLclExt = lpszFileName;
  if ('.' == *lpszFileName)
  {
    lpszLclExt = ++lpszFileName;
    while (*lpszFileName)
    {
      if (bAllowWildcards ? WildChar(*lpszFileName) : NameChar(*lpszFileName))
      {
        ++nExtCnt;
        ++lpszFileName;
      }
      else
        return DE_FILENOTFND;
    }
  }
  else if (*lpszFileName)
    return DE_FILENOTFND;

  /* Fix lengths to maximums allowed by MS-DOS.                   */
  if (nDirCnt > PARSE_MAX)
    nDirCnt = PARSE_MAX;
  if (nFileCnt > FNAME_SIZE)
    nFileCnt = FNAME_SIZE;
  if (nExtCnt > FEXT_SIZE)
    nExtCnt = FEXT_SIZE;

  /* Finally copy whatever the user wants extracted to the user's */
  /* buffers.                                                     */
  if (pszDir)
  {
    fbcopy(lpszLclDir, (BYTE FAR *) pszDir, nDirCnt);
    pszDir[nDirCnt] = '\0';
  }
  if (pszFile)
  {
    fbcopy(lpszLclFile, (BYTE FAR *) pszFile, nFileCnt);
    pszFile[nFileCnt] = '\0';
  }
  if (pszExt)
  {
    fbcopy(lpszLclExt, (BYTE FAR *) pszExt, nExtCnt);
    pszExt[nExtCnt] = '\0';
  }

  /* Clean up before leaving                              */
  if (pszDir)
    DosTrimPath(pszDir);

  return SUCCESS;
}

COUNT ParseDosPath(BYTE FAR * lpszFileName,
                   COUNT * pnDrive,
                   BYTE * pszDir,
                   BYTE FAR * pszCurPath)
{
  COUNT nDirCnt,
    nPathCnt;
  BYTE FAR *lpszLclDir,
   *pszBase = pszDir;

  /* Initialize the users data fields                             */
  *pszDir = '\0';
  lpszLclDir = 0;
  nDirCnt = nPathCnt = 0;

  /* Start by cheking for a drive specifier ...                   */
  if (DriveChar(*lpszFileName) && ':' == lpszFileName[1])
  {
    /* found a drive, fetch it and bump pointer past drive  */
    /* NB: this code assumes ASCII                          */
    if (pnDrive)
    {
      *pnDrive = *lpszFileName - 'A';
      if (*pnDrive > 26)
        *pnDrive -= ('a' - 'A');
    }
    lpszFileName += 2;
  }
  else
  {
    if (pnDrive)
    {
      *pnDrive = -1;
    }
  }

  lpszLclDir = lpszFileName;
  if (!PathSep(*lpszLclDir))
  {
    fstrncpy(pszDir, pszCurPath, PARSE_MAX);
    nPathCnt = fstrlen(pszCurPath);
    if (!PathSep(pszDir[nPathCnt - 1]) && nPathCnt < PARSE_MAX)
      pszDir[nPathCnt++] = '\\';
    if (nPathCnt > PARSE_MAX)
      nPathCnt = PARSE_MAX;
    pszDir += nPathCnt;
  }

  /* Now see how long a directory component we have.              */
  while (NameChar(*lpszFileName)
         || PathSep(*lpszFileName)
         || '.' == *lpszFileName)
  {
    ++nDirCnt;
    ++lpszFileName;
  }

  /* Fix lengths to maximums allowed by MS-DOS.                   */
  if ((nDirCnt + nPathCnt) > PARSE_MAX)
    nDirCnt = PARSE_MAX - nPathCnt;

  /* Finally copy whatever the user wants extracted to the user's */
  /* buffers.                                                     */
  if (pszDir)
  {
    fbcopy(lpszLclDir, (BYTE FAR *) pszDir, nDirCnt);
    pszDir[nDirCnt] = '\0';
  }

  /* Clean up before leaving                              */
  DosTrimPath((BYTE FAR *) pszBase);

  /* Before returning to the user, eliminate any useless          */
  /* trailing "\\." since the path prior to this is sufficient.   */
  nPathCnt = strlen(pszBase);
  if (2 == nPathCnt)            /* Special case, root           */
  {
    if (!strcmp(pszBase, "\\."))
      pszBase[1] = '\0';
  }
  else if (2 < nPathCnt)
  {
    if (!strcmp(&pszBase[nPathCnt - 2], "\\."))
      pszBase[nPathCnt - 2] = '\0';
  }

  return SUCCESS;
}

BOOL IsDevice(BYTE FAR * pszFileName)
{
  REG struct dhdr FAR *dhp = (struct dhdr FAR *)&nul_dev;
  BYTE szName[FNAME_SIZE];

  /* break up the name first                              */
  if (ParseDosName(pszFileName,
                   (COUNT *) 0, TempBuffer, szName, (BYTE *) 0, FALSE)
      != SUCCESS)
    return FALSE;
  SpacePad(szName, FNAME_SIZE);

  /* Test 1 - does it start with a \dev or /dev           */
  if ((strcmp(szName, "/dev") == 0)
      || (strcmp(szName, "\\dev") == 0))
    return TRUE;

  /* Test 2 - is it on the device chain?                  */
  for (; -1l != (LONG) dhp; dhp = dhp->dh_next)
  {
    COUNT nIdx;

    /* Skip if not char device                      */
    if (!(dhp->dh_attr & ATTR_CHAR))
      continue;

    /* now compare                                  */
    for (nIdx = 0; nIdx < FNAME_SIZE; ++nIdx)
    {
      if (dhp->dh_name[nIdx] != szName[nIdx])
        break;
    }
    if (nIdx >= FNAME_SIZE)
      return TRUE;
  }

  return FALSE;
}

VOID DosTrimPath(BYTE FAR * lpszPathNamep)
{
  BYTE FAR *lpszLast,
    FAR * lpszNext,
    FAR * lpszRoot = (BYTE FAR *) 0;
  COUNT nChars,
    flDotDot;

  /* First, convert all '/' to '\'.  Look for root as we scan     */
  if (*lpszPathNamep == '\\')
    lpszRoot = lpszPathNamep;
  for (lpszNext = lpszPathNamep; *lpszNext; ++lpszNext)
  {
    if (*lpszNext == '/')
      *lpszNext = '\\';
    if (!lpszRoot &&
        *lpszNext == ':' && *(lpszNext + 1) == '\\')
      lpszRoot = lpszNext + 1;
  }

  for (lpszLast = lpszNext = lpszPathNamep, nChars = 0;
       *lpszNext != '\0' && nChars < NAMEMAX;)
  {
    /* Initialize flag for loop.                            */
    flDotDot = FALSE;

    /* If we are at a path seperator, check for extra path  */
    /* seperator, '.' and '..' to reduce.                   */
    if (*lpszNext == '\\')
    {
      /* If it's '\', just move everything down one.  */
      if (*(lpszNext + 1) == '\\')
        fstrncpy(lpszNext, lpszNext + 1, NAMEMAX);
      /* also check for '.' and '..' and move down    */
      /* as appropriate.                              */
      else if (*(lpszNext + 1) == '.')
      {
        if (*(lpszNext + 2) == '.'
            && !(*(lpszNext + 3)))
        {
          /* At the end, just truncate    */
          /* and exit.                    */
          if (lpszLast == lpszRoot)
            *(lpszLast + 1) = '\0';
          else
            *lpszLast = '\0';
          return;
        }

        if (*(lpszNext + 2) == '.'
            && *(lpszNext + 3) == '\\')
        {
          fstrncpy(lpszLast, lpszNext + 3, NAMEMAX);
          /* bump back to the last        */
          /* seperator.                   */
          lpszNext = lpszLast;
          /* set lpszLast to the last one */
          if (lpszLast <= lpszPathNamep)
            continue;
          do
          {
            --lpszLast;
          }
          while (lpszLast != lpszPathNamep
                 && *lpszLast != '\\');
          flDotDot = TRUE;
        }
        /* Note: we skip strange stuff that     */
        /* starts with '.'                      */
        else if (*(lpszNext + 2) == '\\')
        {
          fstrncpy(lpszNext, lpszNext + 2, NAMEMAX);
        }
        /* If we're at the end of a string,     */
        /* just exit.                           */
        else if (*(lpszNext + 2) == NULL)
        {
          return;
        }
        /*
           Added this "else" because otherwise we might not pass
           any of the foregoing tests, as in the case where the
           incoming string refers to a suffix only, like ".bat"

           -SRM
         */
        else
        {
          lpszLast = lpszNext++;
        }
      }
      else
      {
        /* No '.' or '\' so mark it and bump    */
        /* past                                 */
        lpszLast = lpszNext++;
        continue;
      }

      /* Done.  Now set last to next to mark this     */
      /* instance of path seperator.                  */
      if (!flDotDot)
        lpszLast = lpszNext;
    }
    else
      /* For all other cases, bump lpszNext for the   */
      /* next check                                   */
      ++lpszNext;
  }
}

