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
static BYTE *mainRcsId =
    "$Id$";
#endif

#include        "portab.h"
#include        "globals.h"

/*
    TE-TODO: if called repeatedly by same process, 
    last allocation must be freed. if handle count < 20, copy back to PSP
*/
int SetJFTSize(UWORD nHandles)
{
  UWORD block, maxBlock;
  psp FAR *ppsp = MK_FP(cu_psp, 0);
  UBYTE FAR *newtab;
  COUNT i;

  if (nHandles <= ppsp->ps_maxfiles)
  {
    ppsp->ps_maxfiles = nHandles;
    return SUCCESS;
  }

  if ((DosMemAlloc
       ((nHandles + 0xf) >> 4, mem_access_mode, &block, &maxBlock)) < 0)
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

/* this is the same, is shorter (~170)and slightly easier to understand TE*/
int DosMkTmp(BYTE FAR * pathname, UWORD attr)
{
  /* create filename from current date and time */
  char FAR *ptmp = pathname;
  BYTE wd, month, day;
  BYTE h, m, s, hund;
  WORD sh;
  WORD year;
  int rc;
  char name83[13];
  int loop;

  while (*ptmp)
    ptmp++;

  if (ptmp == pathname || (ptmp[-1] != '\\' && ptmp[-1] != '/'))
    *ptmp++ = '\\';

  DosGetDate(&wd, &month, &day, (COUNT FAR *) & year);
  DosGetTime(&h, &m, &s, &hund);

  sh = s * 100 + hund;

  for (loop = 0; loop < 0xfff; loop++)
  {
    sprintf(name83, "%x%x%x%x%x%03x.%03x",
            year & 0xf, month & 0xf, day & 0xf, h & 0xf, m & 0xf,
            sh & 0xfff, loop & 0xfff);

    fmemcpy(ptmp, name83, 13);

    if ((rc = DosOpen(pathname, 0)) < 0 && rc != DE_ACCESS      /* subdirectory ?? */
        /* todo: sharing collision on
           network drive
         */
        )
      break;

    if (rc >= 0)
      DosClose(rc);
  }

  if (rc == DE_FILENOTFND)
  {
    rc = DosCreat(pathname, attr);
  }
  return rc;
}

COUNT get_verify_drive(char FAR * src)
{
  UBYTE drive;

  /* Do we have a drive?                                          */
  if (src[1] == ':')
    drive = ((src[0] - 1) | 0x20) - ('a' - 1);
  else
    return default_drive;
  if (drive < lastdrive && CDSp->cds_table[drive].cdsFlags & CDSVALID)
    return drive;
  else
    return DE_INVLDDRV;
}

/*
 * Added support for external and internal calls.
 * Clean buffer before use. Make the true path and expand file names.
 * Example: *.* -> ????????.??? as in the currect way.
 * MSD returns \\D.\A.\????????.??? with SHSUCDX. So, this code is not
 * compatible MSD Func 60h.
 */

/*TE TODO:

    experimenting with NUL on MSDOS 7.0 (win95)
    
                        WIN95           FREEDOS
    TRUENAME NUL        C:/NUL             OK
    TRUENAME .\NUL      C:\DOS\NUL         
    TRUENAME ..\NUL     C:\NUL
    TRUENAME ..\..\NUL  path not found
    TRUENAME Z:NUL      invalid drive (not lastdrive!!)
    TRUENAME A:NUL      A:/NUL             OK
    TRUENAME A:\NUL     A:\NUL

*/

COUNT truename(char FAR * src, char FAR * dest, COUNT t)
{
  static char buf[128] = "A:\\\0\0\0\0\0\0\0\0\0";
  char *bufp = buf + 3;
  COUNT i, rootEndPos = 2;      /* renamed x to rootEndPos - Ron Cemer */
  struct dhdr FAR *dhp;
  BYTE FAR *froot;
  WORD d;

  dest[0] = '\0';

  i = get_verify_drive(src);
  if (i < 0)
    return DE_INVLDDRV;

  buf[0] = i + 'A';
  buf[1] = ':';                 /* Just to be sure */

  /* First, adjust the source pointer */
  src = adjust_far(src);

  /* Do we have a drive? */
  if (src[1] == ':')
    src += 2;

/*
    Code repoff from dosfns.c
    MSD returns X:/CON for truename con. Not X:\CON
*/
  /* check for a device  */

  if ((*src != '.') && (*src != '\\') && (*src != '/')
      && ((dhp = IsDevice(src)) != NULL))
  {

    froot = get_root(src);

    /* /// Bugfix: NUL.LST is the same as NUL.  This is true for all
       devices.  On a device name, the extension is irrelevant
       as long as the name matches.
       - Ron Cemer */

    buf[2] = '/';
    /* /// Bug: should be only copying up to first space.
       - Ron Cemer */

    for (d = 0;
         d < FNAME_SIZE && dhp->dh_name[d] != 0 && dhp->dh_name[d] != ' ';
         d++)
      *bufp++ = dhp->dh_name[d];
    /* /// DOS will return C:/NUL.LST if you pass NUL.LST in.
       DOS will also return C:/NUL.??? if you pass NUL.* in.
       Code added here to support this.
       - Ron Cemer */
    while ((*froot != '.') && (*froot != '\0'))
      froot++;
    if (*froot)
      froot++;
    if (*froot)
    {
      *bufp++ = '.';
      for (i = 0; i < FEXT_SIZE; i++)
      {
        if ((*froot == '\0') || (*froot == '.'))
          break;
        if (*froot == '*')
        {
          for (; i < FEXT_SIZE; i++)
            *bufp++ = '?';
          break;
        }
        *bufp++ = *froot++;
      }
    }
    /* /// End of code additions.  - Ron Cemer */
    goto exit_tn;
  }

  /* /// Added to adjust for filenames which begin with ".\"
   *     The problem was manifesting itself in the inability
   *     to run an program whose filename (without the extension)
   *     was longer than six characters and the PATH variable
   *     contained ".", unless you explicitly specified the full
   *     path to the executable file.
   *     Jun 11, 2000 - rbc */
  /* /// Changed to "while" from "if".  - Ron Cemer */
  while ((src[0] == '.') && (src[1] == '\\'))
    src += 2;

  current_ldt = &CDSp->cds_table[i];

  /* Always give the redirector a chance to rewrite the filename */
  fmemcpy(bufp - 1, src, sizeof(buf) - (bufp - buf));
  if ((t == FALSE) && (QRemote_Fn(buf, dest) == SUCCESS)
      && (dest[0] != '\0'))
  {
    return SUCCESS;
  }
  else
  {
    bufp[-1] = '\\';
  }
  if (t == FALSE)
  {
    fmemcpy(buf, current_ldt->cdsCurrentPath, current_ldt->cdsJoinOffset);
    bufp = buf + current_ldt->cdsJoinOffset;
    rootEndPos = current_ldt->cdsJoinOffset;    /* renamed x to rootEndPos - Ron Cemer */
    *bufp++ = '\\';
  }

  if (*src != '\\' && *src != '/')      /* append current dir */
  {
    DosGetCuDir((UBYTE) (i + 1), bufp);
    if (*bufp)
    {
      while (*bufp)
        bufp++;
      *bufp++ = '\\';
    }
  }
  else
    src++;

/*move_name:*/

  /* /// The block inside the "#if (0) ... #endif" is
     seriously broken.  New code added below to replace it.
     This eliminates many serious bugs, specifically
     with FreeCOM where truename is required to work
     according to the DOS specification in order for
     the COPY and other file-related commands to work
     properly.
     This should be a major improvement to all apps which
     use truename.
     - Ron Cemer */

#if (0)
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
    if (!n)
      return DE_PATHNOTFND;     /* do this for now */
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

        if (bufp[-1] != '\\')
        {
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
              if (bufp < buf + rootEndPos)      /* '..' illegal in root dir */
                return DE_PATHNOTFND;
            }
            src++;
            if (bufp[-1] == ':')
              bufp++;
          }
          else if (*src == '/' || *src == '\\' || *src == 0)
            break;
          /*  --bufp; */
          else
            return DE_PATHNOTFND;
        }
        else if (*src == '/' || *src == '\\' || *src == 0)
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
#endif

/* /// Beginning of new code.  - Ron Cemer */
  bufp--;
  {
    char c, *bufend = buf + (sizeof(buf) - 1);
    int gotAnyWildcards = 0;
    int seglen, copylen, state;
    int error = DE_PATHNOTFND;
    while ((*src) && (bufp < bufend))
    {
      /* Skip duplicated slashes. */
      while ((*src == '/') || (*src == '\\'))
        src++;
      if (!(*src))
        break;
      /* Find the end of this segment in the source string. */
      for (seglen = 0;; seglen++)
      {
        c = src[seglen];
        if (c == '\0')
        {
          error = DE_FILENOTFND;
          break;
        }
        else if ((c == '/') || (c == '\\'))
          break;
      }
      if (seglen > 0)
      {
        /* Ignore all ".\" or "\." path segments. */
        if ((seglen != 1) || (*src != '.'))
        {
          /* Apply ".." to the path by removing
             last path segment from buf. */
          if ((seglen == 2) && (src[0] == '.') && (src[1] == '.'))
          {
            if (bufp > (buf + rootEndPos))
            {
              bufp--;
              while ((bufp > (buf + rootEndPos))
                     && (*bufp != '/') && (*bufp != '\\'))
                bufp--;
            }
            else
            {
              /* .. in root dir illegal */
              return error;
            }
          }
          else
          {
            /* New segment.  If any wildcards in previous
               segment(s), this is an invalid path. */
            if (gotAnyWildcards || src[0] == '.')
              return error;
            /* Append current path segment to result. */
            *(bufp++) = '\\';
            if (bufp >= bufend)
              break;
            copylen = state = 0;
            for (i = 0; ((i < seglen) && (bufp < bufend)); i++)
            {
              c = src[i];
              gotAnyWildcards |= ((c == '?') || (c == '*'));
              switch (state)
              {
                case 0:        /* Copying filename (excl. extension) */
                  if (c == '*')
                  {
                    while (copylen < FNAME_SIZE)
                    {
                      *(bufp++) = '?';
                      if (bufp >= bufend)
                        break;
                      copylen++;
                    }
                    break;
                  }
                  if (c == '.')
                  {
                    if (src[i + 1] != '.' && i + 1 < seglen)
                      *(bufp++) = '.';
                    copylen = 0;
                    state = 1;  /* Copy extension next */
                    break;
                  }
                  if (copylen < FNAME_SIZE)
                  {
                    *(bufp++) = c;
                    copylen++;
                    break;
                  }
                  break;
                case 1:        /* Copying extension */
                  if (c == '*')
                  {
                    while (copylen < FEXT_SIZE)
                    {
                      *(bufp++) = '?';
                      if (bufp >= bufend)
                        break;
                      copylen++;
                    }
                  }
                  if (c == '.')
                    return error;
                  if (copylen < FEXT_SIZE)
                  {
                    *(bufp++) = c;
                    copylen++;
                  }
                  break;
              }
            }
          }
        }
      }                         /* if (seglen > 0) */
      src += seglen;
      if (*src)
        src++;
    }                           /* while ( (*src) && (bufp < bufend) ) */
  }
/* /// End of new code.  - Ron Cemer */

  if (bufp == buf + 2)
    ++bufp;

exit_tn:

  *bufp++ = 0;

  /* finally, uppercase everything */
  DosUpString(buf);

  /* copy to user's buffer */
  fmemcpy(dest, buf, bufp - buf);

  return SUCCESS;
}

/*
 * Log: newstuff.c,v - for newer entries see "cvs log newstuff.c"
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
