/****************************************************************/
/*                                                              */
/*                           newstuff.c                         */
/*                            DOS-C                             */
/*                                                              */
/*                       Copyright (c) 1996                     */
/*                          Svante Frey                         */
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

#ifdef VERSION_STRINGS
static BYTE *mainRcsId = "$Id$";
#endif

/*
 * $Log$
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
 * Revision 1.8  2000/04/02 06:11:35  jtabor
 * Fix ChgDir Code
 *
 * Revision 1.7  2000/04/02 05:30:48  jtabor
 * Fix ChgDir Code
 *
 * Revision 1.6  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.5  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.4  1999/08/25 03:18:09  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.3  1999/04/11 04:33:39  jprice
 * ror4 patches
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:22  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:26  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *    Rev 1.4   06 Dec 1998  8:49:02   patv
 * Bug fixes.
 *
 *    Rev 1.3   04 Jan 1998 23:15:22   patv
 * Changed Log for strip utility
 *
 *    Rev 1.2   04 Jan 1998 17:26:14   patv
 * Corrected subdirectory bug
 *
 *    Rev 1.1   22 Jan 1997 13:21:22   patv
 * pre-0.92 Svante Frey bug fixes.
 */

#include        "portab.h"
#include        "globals.h"
#include        "proto.h"

int SetJFTSize(UWORD nHandles)
{
  UWORD block,
    maxBlock;
  psp FAR *ppsp = MK_FP(cu_psp, 0);
  UBYTE FAR *newtab;
  COUNT i;

  if (nHandles <= ppsp->ps_maxfiles)
  {
    ppsp->ps_maxfiles = nHandles;
    return SUCCESS;
  }

  if ((DosMemAlloc((nHandles + 0xf) >> 4, mem_access_mode, &block, &maxBlock)) < 0)
    return DE_NOMEM;

  ++block;
  newtab = MK_FP(block, 0);

  for (i = 0; i < ppsp->ps_maxfiles; i++)
    newtab[i] = ppsp->ps_filetab[i];

  for (; i < nHandles; i++)
    newtab[i] = 0xff;

  ppsp->ps_maxfiles = nHandles;
  ppsp->ps_filetab = newtab;

  return SUCCESS;
}

int DosMkTmp(BYTE FAR * pathname, UWORD attr)
{
  /* create filename from current date and time */
  static const char tokens[] = "0123456789ABCDEF";
  char FAR *ptmp = pathname;
  BYTE wd,
    month,
    day;
  BYTE h,
    m,
    s,
    hund;
  WORD sh;
  WORD year;
  int rc;

  while (*ptmp)
    ptmp++;

  if (ptmp == pathname || (ptmp[-1] != '\\' && ptmp[-1] != '/'))
    *ptmp++ = '\\';

  DosGetDate(&wd, &month, &day, (COUNT FAR *) & year);
  DosGetTime(&h, &m, &s, &hund);

  sh = s * 100 + hund;

  ptmp[0] = tokens[year & 0xf];
  ptmp[1] = tokens[month];
  ptmp[2] = tokens[day & 0xf];
  ptmp[3] = tokens[h & 0xf];
  ptmp[4] = tokens[m & 0xf];
  ptmp[5] = tokens[(sh >> 8) & 0xf];
  ptmp[6] = tokens[(sh >> 4) & 0xf];
  ptmp[7] = tokens[sh & 0xf];
  ptmp[8] = '.';
  ptmp[9] = 'A';
  ptmp[10] = 'A';
  ptmp[11] = 'A';
  ptmp[12] = 0;

  while ((rc = DosOpen(pathname, 0)) >= 0)
  {
    DosClose(rc);

    if (++ptmp[11] > 'Z')
    {
      if (++ptmp[10] > 'Z')
      {
        if (++ptmp[9] > 'Z')
          return DE_TOOMANY;

        ptmp[10] = 'A';
      }
      ptmp[11] = 'A';
    }
  }

  if (rc == DE_FILENOTFND)
  {
    rc = DosCreat(pathname, attr);
  }
  return rc;
}

COUNT get_verify_drive(char FAR *src)
{
  COUNT drive;
  /* First, adjust the source pointer                             */
  src = adjust_far(src);

  /* Do we have a drive?                                          */
  if (src[1] == ':')
  {
    drive = (src[0] | 0x20) - 'a';
  }
  else
    drive = default_drive;
  if ((drive < 0) || (drive > lastdrive)) {
    drive = DE_INVLDDRV;
  }
  return drive;
}

/*
 * Added support for external and internal calls.
 * Clean buffer before use. Make the true path and expand file names.
 * Example: *.* -> ????????.??? as in the currect way.
 * MSD returns \\D.\A.\????????.??? with SHSUCDX. So, this code is not
 * compatible MSD Func 60h.
 */
COUNT truename(char FAR * src, char FAR * dest, COUNT t)
{
  static char buf[128] = "A:\\\0\0\0\0\0\0\0\0\0";
  char *bufp = buf + 3;
  COUNT i, n, x = 2;
  struct cds FAR *cdsp;

  dest[0] = '\0';

  /* First, adjust the source pointer */
  src = adjust_far(src);

  /* Do we have a drive? */
  if (src[1] == ':')
  {
    buf[0] = (src[0] | 0x20) + 'A' - 'a';

    if (buf[0] > lastdrive + 'A')
      return DE_PATHNOTFND;

    src += 2;
  }
  else
    buf[0] = default_drive + 'A';

  i = buf[0] - 'A';

  cdsp = &CDSp->cds_table[i];
  current_ldt = cdsp;

  /* Always give the redirector a chance to rewrite the filename */
  fsncopy((BYTE FAR *) src, bufp -1, sizeof(buf) - (bufp - buf));
  if ((QRemote_Fn(buf, dest) == SUCCESS) && (dest[0] != '\0')) {
    return SUCCESS;
  } else {
    bufp[-1] = '\\';
  }
    if (t == FALSE)
    {
      fsncopy((BYTE FAR *) & cdsp->cdsCurrentPath[0], (BYTE FAR *) & buf[0], cdsp->cdsJoinOffset);
      bufp = buf + cdsp->cdsJoinOffset;
      x = cdsp->cdsJoinOffset;
      *bufp++ = '\\';
    }

  if (*src != '\\' && *src != '/')	/* append current dir */
  {
    DosGetCuDir(i+1, bufp);
    if (*bufp)
    {
      while (*bufp)
        bufp++;
      *bufp++ = '\\';
    }
  }
  else
    src++;
/*
 *  The code here is brain dead. It works long as the calling
 *  function are operating with in normal parms.
 *  jt
 */
    n = 9;
  /* convert all forward slashes to backslashes, and uppercase all characters */
  while (*src)
  {
    char c;
    c = *src++;
    if(!n)
        return DE_PATHNOTFND; /* do this for now */
    n--;
    switch (c)
    {
      case '*':
        if (*src == '.')
        {
          while (n--)
            *bufp++ = '?';
          break;
        }
        else
        {
          if (src[-2] == '.')
          {
            while (n--)
              *bufp++ = '?';
            break;
          }
          else
          {
            while (n--)
              *bufp++ = '?';
            break;
          }
        }
      case '/':                /* convert to backslash */
      case '\\':

        if (bufp[-1] != '\\'){
            *bufp++ = '\\';
            n = 9;
        }
        break;

        /* look for '.' and '..' dir entries */
      case '.':
        if (bufp[-1] == '\\')
        {
          if (*src == '.' && (src[1] == '/' || src[1] == '\\' || !src[1]))
          {
            /* '..' dir entry: rewind bufp to last backslash */

            for (bufp -= 2; *bufp != '\\'; bufp--)
            {
              if (bufp < buf + x)	/* '..' illegal in root dir */
                return DE_PATHNOTFND;
            }
            src++;
            if (bufp[-1] == ':')
              bufp++;
          }
          else if (*src == '/' || *src == '\\' || *src == 0)
                break;
              /*  --bufp;*/
            else
                return DE_PATHNOTFND;
        }
        else if ( *src == '/' || *src == '\\' || *src == 0)
            {
                break;
            }
            else
            {
                n = 4;
                *bufp++ = c;
            }
        break;

      default:
        *bufp++ = c;
        break;
    }
  }
  /* remove trailing backslashes */
  while (bufp[-1] == '\\')
    --bufp;

  if (bufp == buf + 2)
    ++bufp;

  *bufp++ = 0;

  /* finally, uppercase everything */
  upString(buf);

  /* copy to user's buffer */
  fbcopy(buf, dest, bufp - buf);

  return SUCCESS;
}

