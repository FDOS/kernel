/****************************************************************/
/*                                                              */
/*                           dosfns.c                           */
/*                                                              */
/*                         DOS functions                        */
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

#include "portab.h"

#ifdef VERSION_STRINGS
static BYTE *dosfnsRcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.1  2000/05/06 19:34:59  jhall1
 * Initial revision
 *
 * Revision 1.11  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.10  1999/09/23 04:40:46  jprice
 * *** empty log message ***
 *
 * Revision 1.8  1999/09/14 01:01:53  jprice
 * Fixed bug where you could write over directories.
 *
 * Revision 1.7  1999/08/25 03:18:07  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.6  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.5  1999/04/16 12:21:22  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.4  1999/04/12 03:21:17  jprice
 * more ror4 patches.  Changes for multi-block IO
 *
 * Revision 1.3  1999/04/11 04:33:38  jprice
 * ror4 patches
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:52  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
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
 *    Rev 1.10   06 Dec 1998  8:44:42   patv
 * Expanded dos functions due to new I/O subsystem.
 *
 *    Rev 1.9   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.8   03 Jan 1998  8:36:04   patv
 * Converted data area to SDA format
 *
 *    Rev 1.7   22 Jan 1997 12:59:56   patv
 * pre-0.92 bug fixes
 *
 *    Rev 1.6   16 Jan 1997 12:46:32   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.5   29 May 1996 21:15:20   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.4   19 Feb 1996  3:20:08   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:48:48   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:24   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:04:20   patv
 * Initial revision.
 */

#include "globals.h"

sft FAR *get_sft(COUNT);
WORD get_free_hndl(VOID);
sft FAR *get_free_sft(WORD FAR *);
BYTE FAR *get_root(BYTE FAR *);
BOOL cmatch(COUNT, COUNT, COUNT);
BOOL fnmatch(BYTE FAR *, BYTE FAR *, COUNT, COUNT);

struct f_node FAR *xlt_fd(COUNT);

static VOID DosGetFile(BYTE FAR * lpszPath, BYTE FAR * lpszDosFileName)
{
  BYTE szLclName[FNAME_SIZE + 1];
  BYTE szLclExt[FEXT_SIZE + 1];

  ParseDosName(lpszPath, (COUNT *) 0, (BYTE *) 0,
               szLclName, szLclExt, FALSE);
  SpacePad(szLclName, FNAME_SIZE);
  SpacePad(szLclExt, FEXT_SIZE);
  fbcopy((BYTE FAR *) szLclName, lpszDosFileName, FNAME_SIZE);
  fbcopy((BYTE FAR *) szLclExt, &lpszDosFileName[FNAME_SIZE], FEXT_SIZE);
}

sft FAR *get_sft(COUNT hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD sys_idx;
  sfttbl FAR *sp;

  if (hndl >= p->ps_maxfiles)
    return (sft FAR *) - 1;

  /* Get the SFT block that contains the SFT      */
  if (p->ps_filetab[hndl] == 0xff)
    return (sft FAR *) - 1;

  sys_idx = p->ps_filetab[hndl];
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    if (sys_idx < sp->sftt_count)
      break;
    else
      sys_idx -= sp->sftt_count;
  }

  /* If not found, return an error                */
  if (sp == (sfttbl FAR *) - 1)
    return (sft FAR *) - 1;

  /* finally, point to the right entry            */
  return (sft FAR *) & (sp->sftt_table[sys_idx]);
}

/*
 * The `force_binary' parameter is a hack to allow functions 0x01, 0x06, 0x07,
 * and function 0x40 to use the same code for performing reads, even though the
 * two classes of functions behave quite differently: 0x01 etc. always do
 * binary reads, while for 0x40 the type of read (binary/text) depends on what
 * the SFT says. -- ror4
 */
UCOUNT GenericRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err,
                   BOOL force_binary)
{
  sft FAR *s;
  WORD sys_idx;
  sfttbl FAR *sp;
  UCOUNT ReadCount;

  /* Test that the handle is valid                */
  if (hndl < 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  /* If not open or write permission - exit       */
  if (s->sft_count == 0 || (s->sft_mode & SFT_MWRITE))
  {
    *err = DE_INVLDACC;
    return 0;
  }

/*
   *   Do remote first or return error.
   *   must have been opened from remote.
 */
  if (s->sft_flags & SFT_FSHARED)
  {
    ReadCount = Remote_RW(REM_READ, n, bp, s, err);
    if (err)
    {
      *err = SUCCESS;
      return ReadCount;
    }
    else
      return 0;
  }
  /* Do a device read if device                   */
  if (s->sft_flags & SFT_FDEVICE)
  {
    request rq;

    /* First test for eof and exit          */
    /* immediately if it is                 */
    if (!(s->sft_flags & SFT_FEOF) || (s->sft_flags & SFT_FNUL))
    {
      s->sft_flags &= ~SFT_FEOF;
      *err = SUCCESS;
      return 0;
    }

    /* Now handle raw and cooked modes      */
    if (force_binary || (s->sft_flags & SFT_FBINARY))
    {
      rq.r_length = sizeof(request);
      rq.r_command = C_INPUT;
      rq.r_count = n;
      rq.r_trans = (BYTE FAR *) bp;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
      {
        char_error(&rq, s->sft_dev);
      }
      else
      {
        *err = SUCCESS;
        return rq.r_count;
      }
    }
    else if (s->sft_flags & SFT_FCONIN)
    {
      kb_buf.kb_size = LINESIZE - 1;
      kb_buf.kb_count = 0;
      sti((keyboard FAR *) & kb_buf);
      fbcopy((BYTE FAR *) kb_buf.kb_buf, bp, kb_buf.kb_count);
      *err = SUCCESS;
      return kb_buf.kb_count;
    }
    else
    {
      *bp = _sti();
      *err = SUCCESS;
      return 1;
    }
  }
  else
    /* a block read                            */
  {
    COUNT rc;

    ReadCount = readblock(s->sft_status, bp, n, &rc);
    if (rc != SUCCESS)
    {
      *err = rc;
      return 0;
    }
    else
    {
      *err = SUCCESS;
      return ReadCount;
    }
  }
  *err = SUCCESS;
  return 0;
}

UCOUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err)
{
  return GenericRead(hndl, n, bp, err, FALSE);
}

UCOUNT DosWrite(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err)
{
  sft FAR *s;
  WORD sys_idx;
  sfttbl FAR *sp;
  UCOUNT ReadCount;

  /* Test that the handle is valid                */
  if (hndl < 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  /* If this is not opened and it's not a write   */
  /* another error                                */
  if (s->sft_count == 0 ||
      (!(s->sft_mode & SFT_MWRITE) && !(s->sft_mode & SFT_MRDWR)))
  {
    *err = DE_ACCESS;
    return 0;
  }
  if (s->sft_flags & SFT_FSHARED)
  {
    ReadCount = Remote_RW(REM_WRITE, n, bp, s, err);
    if (err)
    {
      return ReadCount;
    }
    else
      return 0;
  }

  /* Do a device write if device                  */
  if (s->sft_flags & SFT_FDEVICE)
  {
    request rq;

    /* set to no EOF                        */
    s->sft_flags &= ~SFT_FEOF;

    /* if null just report full transfer    */
    if (s->sft_flags & SFT_FNUL)
    {
      *err = SUCCESS;
      return n;
    }

    /* Now handle raw and cooked modes      */
    if (s->sft_flags & SFT_FBINARY)
    {
      rq.r_length = sizeof(request);
      rq.r_command = C_OUTPUT;
      rq.r_count = n;
      rq.r_trans = (BYTE FAR *) bp;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
      {
        char_error(&rq, s->sft_dev);
      }
      else
      {
        if (s->sft_flags & SFT_FCONOUT)
        {
          WORD cnt = rq.r_count;
          while (cnt--)
          {
            switch (*bp++)
            {
              case CR:
                scr_pos = 0;
                break;
              case LF:
              case BELL:
                break;
              case BS:
                --scr_pos;
                break;
              default:
                ++scr_pos;
            }
          }
        }
        *err = SUCCESS;
        return rq.r_count;
      }
    }
    else
    {
      REG WORD c,
        cnt = n,
        spaces_left = 0,
        next_pos,
        xfer = 0;
      static BYTE space = ' ';

    start:
      if (cnt-- == 0)
        goto end;
      if (*bp == CTL_Z)
        goto end;
      if (s->sft_flags & SFT_FCONOUT)
      {
        switch (*bp)
        {
          case CR:
            next_pos = 0;
            break;
          case LF:
          case BELL:
            next_pos = scr_pos;
            break;
          case BS:
            next_pos = scr_pos ? scr_pos - 1 : 0;
            break;
          case HT:
            spaces_left = 8 - (scr_pos & 7);
            next_pos = scr_pos + spaces_left;
            goto output_space;
          default:
            next_pos = scr_pos + 1;
        }
      }
      rq.r_length = sizeof(request);
      rq.r_command = C_OUTPUT;
      rq.r_count = 1;
      rq.r_trans = bp;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
        char_error(&rq, s->sft_dev);
      goto post;
    output_space:
      rq.r_length = sizeof(request);
      rq.r_command = C_OUTPUT;
      rq.r_count = 1;
      rq.r_trans = &space;
      rq.r_status = 0;
      execrh((request FAR *) & rq, s->sft_dev);
      if (rq.r_status & S_ERROR)
        char_error(&rq, s->sft_dev);
      --spaces_left;
    post:
      if (spaces_left)
        goto output_space;
      ++bp;
      ++xfer;
      if (s->sft_flags & SFT_FCONOUT)
        scr_pos = next_pos;
      if (break_ena && control_break())
      {
        handle_break();
        goto end;
      }
      goto start;
    end:
      *err = SUCCESS;
      return xfer;
    }
  }
  else
    /* a block write                           */
  {
    COUNT rc;

    ReadCount = writeblock(s->sft_status, bp, n, &rc);
    if (rc < SUCCESS)
    {
      *err = rc;
      return 0;
    }
    else
    {
      *err = SUCCESS;
      return ReadCount;
    }
  }
  *err = SUCCESS;
  return 0;
}

COUNT DosSeek(COUNT hndl, LONG new_pos, COUNT mode, ULONG * set_pos)
{
  sft FAR *s;
  ULONG lrx;

  /* Test for invalid mode                        */
  if (mode < 0 || mode > 2)
    return DE_INVLDFUNC;

  /* Test that the handle is valid                */
  if (hndl < 0)
    return DE_INVLDHNDL;

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  lpCurSft = (sfttbl FAR *) s;

  if (s->sft_flags & SFT_FSHARED)
  {
    if (mode == 2)
    {                           /* seek from end of file */
      int2f_Remote_call(REM_LSEEK, 0, (UWORD) FP_SEG(new_pos), (UWORD) FP_OFF(new_pos), (VOID FAR *) s, 0, 0);
      *set_pos = s->sft_posit;
      return SUCCESS;
    }
    if (mode == 0)
    {
      s->sft_posit = new_pos;
      *set_pos = new_pos;
      return SUCCESS;
    }
    if (mode == 1)
    {
      s->sft_posit += new_pos;
      *set_pos = s->sft_posit;
      return SUCCESS;
    }
    return DE_INVLDFUNC;
  }

  /* Do special return for character devices      */
  if (s->sft_flags & SFT_FDEVICE)
  {
    *set_pos = 0l;
    return SUCCESS;
  }
  else
  {
    *set_pos = dos_lseek(s->sft_status, new_pos, mode);
    if ((LONG) * set_pos < 0)
      return (int)*set_pos;
    else
      return SUCCESS;
  }
}

static WORD get_free_hndl(void)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD hndl;

  for (hndl = 0; hndl < p->ps_maxfiles; hndl++)
  {
    if (p->ps_filetab[hndl] == 0xff)
      return hndl;
  }
  return 0xff;
}

static sft FAR *get_free_sft(WORD FAR * sft_idx)
{
  WORD sys_idx = 0;
  sfttbl FAR *sp;

  /* Get the SFT block that contains the SFT      */
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    REG WORD i;

    for (i = 0; i < sp->sftt_count; i++)
    {
      if (sp->sftt_table[i].sft_count == 0)
      {
        *sft_idx = sys_idx + i;
        return (sft FAR *) & sp->sftt_table[sys_idx + i];
      }
    }
    sys_idx += i;
  }
  /* If not found, return an error                */
  return (sft FAR *) - 1;
}

static BYTE FAR *get_root(BYTE FAR * fname)
{
  BYTE FAR *froot;
  REG WORD length;

  /* find the end                                 */
  for (length = 0, froot = fname; *froot != '\0'; ++froot)
    ++length;
  /* now back up to first path seperator or start */
  for (--froot; length > 0 && !(*froot == '/' || *froot == '\\'); --froot)
    --length;
  return ++froot;
}

/* Ascii only file name match routines                  */
static BOOL cmatch(COUNT s, COUNT d, COUNT mode)
{
  if (s >= 'a' && s <= 'z')
    s -= 'a' - 'A';
  if (d >= 'a' && d <= 'z')
    d -= 'a' - 'A';
  if (mode && s == '?' && (d >= 'A' && s <= 'Z'))
    return TRUE;
  return s == d;
}

static BOOL fnmatch(BYTE FAR * s, BYTE FAR * d, COUNT n, COUNT mode)
{
  while (n--)
  {
    if (!cmatch(*s++, *d++, mode))
      return FALSE;
  }
  return TRUE;
}

COUNT DosCreat(BYTE FAR * fname, COUNT attrib)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD hndl,
    sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  BYTE FAR *froot;
  WORD i;

  /* get a free handle                            */
  if ((hndl = get_free_hndl()) == 0xff)
    return DE_TOOMANY;

  /* now get a free system file table entry       */
  if ((sftp = get_free_sft((WORD FAR *) & sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  /* check for a device                           */
  froot = get_root(fname);
  for (i = 0; i < FNAME_SIZE; i++)
  {
    if (*froot != '\0' && *froot != '.')
      PriPathName[i] = *froot++;
    else
      break;
  }

  for (; i < FNAME_SIZE; i++)
    PriPathName[i] = ' ';

  /* if we have an extension, can't be a device   */
  if (*froot != '.')
  {
    for (dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp->dh_next)
    {
      if (fnmatch((BYTE FAR *) PriPathName, (BYTE FAR *) dhp->dh_name, FNAME_SIZE, FALSE))
      {
        sftp->sft_count += 1;
        sftp->sft_mode = SFT_MRDWR;
        sftp->sft_attrib = attrib;
        sftp->sft_flags =
            ((dhp->dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF;
        sftp->sft_psp = cu_psp;
        fbcopy((BYTE FAR *) PriPathName, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
        sftp->sft_dev = dhp;
        p->ps_filetab[hndl] = sft_idx;
        return hndl;
      }
    }
  }

  if (Remote_OCT(REM_CREATE, fname, attrib, sftp) == 0)
  {
    if (sftp->sft_flags & SFT_FSHARED)
    {
      sftp->sft_count += 1;
      p->ps_filetab[hndl] = sft_idx;
      return hndl;
    }
  }

  sftp->sft_status = dos_creat(fname, attrib);
  if (sftp->sft_status >= 0)
  {
    p->ps_filetab[hndl] = sft_idx;
    sftp->sft_count += 1;
    sftp->sft_mode = SFT_MRDWR;
    sftp->sft_attrib = attrib;
    sftp->sft_flags = 0;
    sftp->sft_psp = cu_psp;
    DosGetFile(fname, sftp->sft_name);
    return hndl;
  }
  else
    return sftp->sft_status;
}

COUNT CloneHandle(COUNT hndl)
{
  sft FAR *sftp;

  /* now get the system file table entry                          */
  if ((sftp = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* now that we have the system file table entry, get the fnode  */
  /* index, and increment the count, so that we've effectively    */
  /* cloned the file.                                             */
  sftp->sft_count += 1;
  return SUCCESS;
}

COUNT DosDup(COUNT Handle)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  COUNT NewHandle;
  sft FAR *Sftp;

  /* Get the SFT block that contains the SFT                      */
  if ((Sftp = get_sft(Handle)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If not open - exit                                           */
  if (Sftp->sft_count <= 0)
    return DE_INVLDHNDL;

  /* now get a free handle                                        */
  if ((NewHandle = get_free_hndl()) == 0xff)
    return DE_TOOMANY;

  /* If everything looks ok, bump it up.                          */
  if ((Sftp->sft_flags & SFT_FDEVICE) || (Sftp->sft_status >= 0))
  {
    p->ps_filetab[NewHandle] = p->ps_filetab[Handle];
    Sftp->sft_count += 1;
    return NewHandle;
  }
  else
    return DE_INVLDHNDL;
}

COUNT DosForceDup(COUNT OldHandle, COUNT NewHandle)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  sft FAR *Sftp;

  /* Get the SFT block that contains the SFT                      */
  if ((Sftp = get_sft(OldHandle)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If not open - exit                                           */
  if (Sftp->sft_count <= 0)
    return DE_INVLDHNDL;

  /* now close the new handle if it's open                        */
  if ((UBYTE) p->ps_filetab[NewHandle] != 0xff)
  {
    COUNT ret;

    if ((ret = DosClose(NewHandle)) != SUCCESS)
      return ret;
  }

  /* If everything looks ok, bump it up.                          */
  if ((Sftp->sft_flags & SFT_FDEVICE) || (Sftp->sft_status >= 0))
  {
    p->ps_filetab[NewHandle] = p->ps_filetab[OldHandle];

    Sftp->sft_count += 1;
    return NewHandle;
  }
  else
    return DE_INVLDHNDL;
}

COUNT DosOpen(BYTE FAR * fname, COUNT mode)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD hndl;
  WORD sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  BYTE FAR *froot;
  WORD i;

  /* test if mode is in range                     */
  if ((mode & ~SFT_OMASK) != 0)
    return DE_INVLDACC;

  mode &= 3;
  /* get a free handle                            */
  if ((hndl = get_free_hndl()) == 0xff)
    return DE_TOOMANY;

  OpenMode = (BYTE) mode;

  /* now get a free system file table entry       */
  if ((sftp = get_free_sft((WORD FAR *) & sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  /* check for a device                           */
  froot = get_root(fname);
  for (i = 0; i < FNAME_SIZE; i++)
  {
    if (*froot != '\0' && *froot != '.')
      PriPathName[i] = *froot++;
    else
      break;
  }

  for (; i < FNAME_SIZE; i++)
    PriPathName[i] = ' ';

  /* if we have an extension, can't be a device   */
  if (*froot != '.')
  {
    for (dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp->dh_next)
    {
      if (fnmatch((BYTE FAR *) PriPathName, (BYTE FAR *) dhp->dh_name, FNAME_SIZE, FALSE))
      {
        sftp->sft_count += 1;
        sftp->sft_mode = mode;
        sftp->sft_attrib = 0;
        sftp->sft_flags =
            ((dhp->dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF;
        sftp->sft_psp = cu_psp;
        fbcopy((BYTE FAR *) PriPathName, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
        sftp->sft_dev = dhp;
        sftp->sft_date = dos_getdate();
        sftp->sft_time = dos_gettime();

        p->ps_filetab[hndl] = sft_idx;
        return hndl;
      }
    }
  }

  if (Remote_OCT(REM_OPEN, fname, mode, sftp) == 0)
  {
    if (sftp->sft_flags & SFT_FSHARED)
    {
      sftp->sft_count += 1;
      p->ps_filetab[hndl] = sft_idx;
      return hndl;
    }
  }

  sftp->sft_status = dos_open(fname, mode);

  if (sftp->sft_status >= 0)
  {
    struct f_node FAR *fnp = xlt_fd(sftp->sft_status);

    sftp->sft_attrib = fnp->f_dir.dir_attrib;

    /* Check permissions. -- JPP */
    if ((sftp->sft_attrib & (D_DIR | D_VOLID)) ||
        ((sftp->sft_attrib & D_RDONLY) && (mode != O_RDONLY)))
    {
      return DE_ACCESS;
    }
    p->ps_filetab[hndl] = sft_idx;

    sftp->sft_count += 1;
    sftp->sft_mode = mode;
    sftp->sft_attrib = 0;
    sftp->sft_flags = 0;
    sftp->sft_psp = cu_psp;
    DosGetFile(fname, sftp->sft_name);
    return hndl;
  }
  else
    return sftp->sft_status;
}

COUNT DosClose(COUNT hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  sft FAR *s;

  /* Test that the handle is valid                */
  if (hndl < 0)
    return DE_INVLDHNDL;

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return DE_ACCESS;

  lpCurSft = (sfttbl FAR *) s;
/*
   remote sub sft_count.
 */
  if (s->sft_flags & SFT_FSHARED)
  {
    int2f_Remote_call(REM_CLOSE, 0, 0, 0, (VOID FAR *) s, 0, 0);
    p->ps_filetab[hndl] = 0xff;
    s->sft_flags = 0;
    return SUCCESS;
  }

  /* now just drop the count if a device, else    */
  /* call file system handler                     */
  if (s->sft_flags & SFT_FDEVICE)
  {
    p->ps_filetab[hndl] = 0xff;
    s->sft_count -= 1;
    return SUCCESS;
  }
  else
  {
    p->ps_filetab[hndl] = 0xff;
    s->sft_count -= 1;
    if (s->sft_count > 0)
      return SUCCESS;
    else
      return dos_close(s->sft_status);
  }
}

VOID DosGetFree(COUNT drive, COUNT FAR * spc, COUNT FAR * navc, COUNT FAR * bps, COUNT FAR * nc)
{
  struct dpb *dpbp;
  struct cds FAR *cdsp;
  static char rg[8];

  /* next - "log" in the drive            */
  drive = (drive == 0 ? default_drive : drive - 1);

  cdsp = &CDSp->cds_table[drive];
  if (cdsp->cdsFlags & 0x8000)
  {
    int2f_Remote_call(REM_GETSPACE, 0, 0, 0, cdsp, 0, &rg);

    *spc = (COUNT) rg[0];
    *navc = (COUNT) rg[2];
    *bps = (COUNT) rg[4];
    *nc = (COUNT) rg[6];
    return;
  }

  /* first check for valid drive          */
  if (drive < 0 || drive > lastdrive)
  {
    *spc = -1;
    return;
  }

  dpbp = (struct dpb *)CDSp->cds_table[drive].cdsDpb;
  if (dpbp == 0)
  {
    *spc = -1;
    return;
  }

  if ((media_check(dpbp) < 0))
  {
    *spc = -1;
    return;
  }

  /* get the data vailable from dpb       */
  *nc = dpbp->dpb_size;
  *spc = dpbp->dpb_clsmask + 1;
  *bps = dpbp->dpb_secsize;

  /* now tell fs to give us free cluster  */
  /* count                                */
  *navc = dos_free(dpbp);
}

COUNT DosGetCuDir(COUNT drive, BYTE FAR * s)
{
  REG struct cds FAR *cdsp;

  /* next - "log" in the drive            */
  drive = (drive == 0 ? default_drive : drive - 1);

  cdsp = &CDSp->cds_table[drive];

  current_ldt = cdsp;

  if (cdsp->cdsFlags & 0x8000)
  {
    dos_pwd(cdsp, s);
    return SUCCESS;
  }

  /* first check for valid drive          */

  if (cdsp->cdsDpb == 0)
    return DE_INVLDDRV;

/*  if (drive < 0 || drive > nblkdev)
   return DE_INVLDDRV;
 */
  dos_pwd(cdsp, s);
  return SUCCESS;
}

COUNT DosChangeDir(BYTE FAR * s)
{
  REG struct dpb *dpbp;
  REG struct cds FAR *cdsp;
  REG COUNT drive;
  struct f_node FAR *fp;
  COUNT ret;
  /* Parse and extract drive              */
  if (*(s + 1) == ':')
  {
    drive = *s - '@';
    if (drive > 26)
      drive -= 'a' - 'A';
  }
  else
    drive = 0;

  /* next - "log" in the drive            */
  drive = (drive == 0 ? default_drive : drive - 1);

  cdsp = &CDSp->cds_table[drive];

  current_ldt = cdsp;

  if (cdsp->cdsFlags & 0x8000)
  {
    ret = dos_cd(cdsp, s);
    return ret;
  }

  /* first check for valid drive          */
  if (cdsp->cdsDpb == 0)
    return DE_INVLDDRV;

  /* test for path existance from fs      */
  if ((fp = dir_open((BYTE FAR *) s)) == (struct f_node FAR *)0)
    return DE_PATHNOTFND;
  else
    dir_close(fp);

/*  if (drive < 0 || drive > nblkdev)
   return DE_INVLDDRV; */

  dpbp = (struct dpb *)cdsp->cdsDpb;
  if ((media_check(dpbp) < 0))
    return DE_INVLDDRV;
  /* now get fs to change to new          */
  /* directory                            */
  ret = dos_cd(cdsp, s);
  return ret;
}

COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name)
{
  SAttr = (BYTE) attr;
  return dos_findfirst(attr, name);
}

COUNT DosFindNext(void)
{
  return dos_findnext();
}

COUNT DosGetFtime(COUNT hndl, date FAR * dp, time FAR * tp)
{
  sft FAR *s;
  sfttbl FAR *sp;

  /* Test that the handle is valid                */
  if (hndl < 0)
    return DE_INVLDHNDL;

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return DE_ACCESS;

  /* If SFT entry refers to a device, return the date and time of opening */
  if (s->sft_flags & (SFT_FDEVICE | SFT_FSHARED))
  {
    *dp = s->sft_date;
    *tp = s->sft_time;
    return SUCCESS;
  }

  /* call file system handler                     */
  return dos_getftime(s->sft_status, dp, tp);
}

COUNT DosSetFtime(COUNT hndl, date FAR * dp, time FAR * tp)
{
  sft FAR *s;
  sfttbl FAR *sp;

  /* Test that the handle is valid                */
  if (hndl < 0)
    return DE_INVLDHNDL;

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return DE_ACCESS;

  /* If SFT entry refers to a device, do nothing */
  if (s->sft_flags & SFT_FDEVICE)
    return SUCCESS;

  if (s->sft_flags & SFT_FSHARED)
  {
    s->sft_date = *dp;
    s->sft_time = *tp;
    return SUCCESS;
  }

  /* call file system handler                     */
  return dos_setftime(s->sft_status, dp, tp);
}

COUNT DosGetFattr(BYTE FAR * name, UWORD FAR * attrp)
{

  if (Remote_GSattr(REM_GETATTRZ, name, attrp) == 0)
    return SUCCESS;

  return dos_getfattr(name, attrp);

}

COUNT DosSetFattr(BYTE FAR * name, UWORD FAR * attrp)
{
  if (Remote_GSattr(REM_SETATTR, name, attrp) == 0)
    return SUCCESS;

  return dos_setfattr(name, attrp);
}

BYTE DosSelectDrv(BYTE drv)
{
  if (CDSp->cds_table[drv].cdsFlags & 0xf000)
  {
    current_ldt = &CDSp->cds_table[drv];
    default_drive = drv;
  }
  return lastdrive;
}

