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
 *
 * /// Added SHARE support.  2000/09/04 Ron Cemer
 *
 * $Log$
 * Revision 1.24  2001/08/19 12:58:36  bartoldeman
 * Time and date fixes, Ctrl-S/P, findfirst/next, FCBs, buffers, tsr unloading
 *
 * Revision 1.23  2001/07/28 18:13:06  bartoldeman
 * Fixes for FORMAT+SYS, FATFS, get current dir, kernel init memory situation.
 *
 * Revision 1.22  2001/07/24 16:56:29  bartoldeman
 * fixes for FCBs, DJGPP ls, DBLBYTE, dyninit allocation (2024e).
 *
 * Revision 1.21  2001/07/23 12:47:42  bartoldeman
 * FCB fixes and clean-ups, exec int21/ax=4b01, initdisk.c printf
 *
 * Revision 1.20  2001/07/22 01:58:58  bartoldeman
 * Support for Brian's FORMAT, DJGPP libc compilation, cleanups, MSCDEX
 *
 * Revision 1.19  2001/07/09 22:19:33  bartoldeman
 * LBA/FCB/FAT/SYS/Ctrl-C/ioctl fixes + memory savings
 *
 * Revision 1.18  2001/06/03 14:16:17  bartoldeman
 * BUFFERS tuning and misc bug fixes/cleanups (2024c).
 *
 * Revision 1.17  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.16  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.15  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.14  2001/04/02 23:18:30  bartoldeman
 * Misc, zero terminated device names and redirector bugs fixed.
 *
 * Revision 1.13  2001/03/30 22:27:42  bartoldeman
 * Saner lastdrive handling.
 *
 * Revision 1.12  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.11  2001/03/21 02:56:25  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.10  2001/03/08 21:15:00  bartoldeman
 * Redirector and DosSelectDrv() (Martin Stromberg) fixes
 *
 * Revision 1.9  2000/10/29 23:51:56  jimtabor
 * Adding Share Support by Ron Cemer
 *
 * Revision 1.8  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.7  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.6  2000/06/01 06:37:38  jimtabor
 * Read History for Changes
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
 * Revision 1.2  2000/05/08 04:29:59  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.14  2000/04/02 05:01:08  jtabor
 *  Replaced ChgDir Code
 *
 * Revision 1.13  2000/04/02 04:53:56  jtabor
 * Fix to DosChgDir
 *
 * Revision 1.12  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
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

COUNT get_free_hndl(VOID);
sft FAR *get_free_sft(COUNT *);
BOOL cmatch(COUNT, COUNT, COUNT);

f_node_ptr xlt_fd(COUNT);


/* /// Added for SHARE.  - Ron Cemer */

int share_installed = 0;

	/* DOS calls this to see if it's okay to open the file.
	   Returns a file_table entry number to use (>= 0) if okay
	   to open.  Otherwise returns < 0 and may generate a critical
	   error.  If < 0 is returned, it is the negated error return
	   code, so DOS simply negates this value and returns it in
	   AX. */
static int share_open_check
    (char far *filename,    /* far pointer to fully qualified filename */
     unsigned short pspseg, /* psp segment address of owner process */
     int openmode,          /* 0=read-only, 1=write-only, 2=read-write */
     int sharemode);        /* SHARE_COMPAT, etc... */

	/* DOS calls this to record the fact that it has successfully
	   closed a file, or the fact that the open for this file failed. */
static void share_close_file
    (int fileno);           /* file_table entry number */

	/* DOS calls this to determine whether it can access (read or
	   write) a specific section of a file.  We call it internally
	   from lock_unlock (only when locking) to see if any portion
       of the requested region is already locked.  If pspseg is zero,
	   then it matches any pspseg in the lock table.  Otherwise, only
	   locks which DO NOT belong to pspseg will be considered.
	   Returns zero if okay to access or lock (no portion of the
	   region is already locked).  Otherwise returns non-zero and
	   generates a critical error (if allowcriter is non-zero).
	   If non-zero is returned, it is the negated return value for
	   the DOS call. */
static int share_access_check
    (unsigned short pspseg,/* psp segment address of owner process */
	 int fileno,		/* file_table entry number */
	 unsigned long ofs,	/* offset into file */
     unsigned long len, /* length (in bytes) of region to access */
     int allowcriter);  /* allow a critical error to be generated */

	/* DOS calls this to lock or unlock a specific section of a file.
	   Returns zero if successfully locked or unlocked.  Otherwise
	   returns non-zero.
	   If the return value is non-zero, it is the negated error
	   return code for the DOS 0x5c call. */
static int share_lock_unlock
    (unsigned short pspseg,/* psp segment address of owner process */
	 int fileno,		/* file_table entry number */
	 unsigned long ofs,	/* offset into file */
     unsigned long len, /* length (in bytes) of region to lock or unlock */
     int unlock);       /* non-zero to unlock; zero to lock */

/* /// End of additions for SHARE.  - Ron Cemer */


static VOID DosGetFile(BYTE * lpszPath, BYTE FAR * lpszDosFileName)
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

sft FAR *idx_to_sft(COUNT SftIndex)
{
  sfttbl FAR *sp;

  if (SftIndex < 0)
    return (sft FAR *) - 1;

  /* Get the SFT block that contains the SFT      */
  for (sp = sfthead; sp != (sfttbl FAR *) - 1;
       sp = sp->sftt_next)
  {
    if (SftIndex < sp->sftt_count)
      /* finally, point to the right entry            */
      return (sft FAR *) & (sp->sftt_table[SftIndex]);
    else
      SftIndex -= sp->sftt_count;
  }
  /* If not found, return an error                */
  return (sft FAR *) - 1;
}

STATIC COUNT get_sft_idx(UCOUNT hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);

  if (hndl >= p->ps_maxfiles)
    return DE_INVLDHNDL;

  return p->ps_filetab[hndl] == 0xff ? DE_INVLDHNDL : p->ps_filetab[hndl];
}

sft FAR *get_sft(UCOUNT hndl)
{
  /* Get the SFT block that contains the SFT      */
  return idx_to_sft(get_sft_idx(hndl));
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
  UCOUNT ReadCount;

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
    return *err == SUCCESS ? ReadCount : 0;
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
      ReadCount = sti(&kb_buf);
      if (ReadCount < kb_buf.kb_count)
        s->sft_flags &= ~SFT_FEOF;
      fbcopy((BYTE FAR *) kb_buf.kb_buf, bp, kb_buf.kb_count);
      *err = SUCCESS;
      return ReadCount;
    }
    else
    {
      *bp = _sti(FALSE);
      *err = SUCCESS;
      return 1;
    }
  }
  else
    /* a block read                            */
  {
    COUNT rc;

    /* /// Added for SHARE - Ron Cemer */
    if (IsShareInstalled()) {
        if (s->sft_shroff >= 0) {
            int share_result = share_access_check
                (cu_psp,
                 s->sft_shroff,
                 s->sft_posit,
                 (unsigned long)n,
                 1);
            if (share_result != 0) {
                *err = share_result;
                return 0;
            }
		}
    }
    /* /// End of additions for SHARE - Ron Cemer */

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

#if 0
UCOUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err)
{
  return GenericRead(hndl, n, bp, err, FALSE);
}
#endif

UCOUNT DosWrite(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err)
{
  sft FAR *s;
  UCOUNT WriteCount;

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
    WriteCount = Remote_RW(REM_WRITE, n, bp, s, err);
    return *err == SUCCESS ? WriteCount : 0;
  }

  /* Do a device write if device                  */
  if (s->sft_flags & SFT_FDEVICE)
  {
    request rq;

    /* set to no EOF                        */
    s->sft_flags |= SFT_FEOF;

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
      REG WORD /*c,*/
      cnt = n,
      xfer = 0;

      while(cnt-- != 0 && *bp != CTL_Z){
        if (s->sft_flags & SFT_FCONOUT)
	{	
	  cso(*bp);
        }
        else
        {
          rq.r_length = sizeof(request);
          rq.r_command = C_OUTPUT;
          rq.r_count = 1;
          rq.r_trans = bp;
          rq.r_status = 0;
          execrh((request FAR *) & rq, s->sft_dev);
          if (rq.r_status & S_ERROR)
              char_error(&rq, s->sft_dev);
        }
        ++bp;
        ++xfer;
        if (control_break())
        {
          handle_break();
          break;
        }
      }
      *err = SUCCESS;
      return xfer;
    }
  }
  else
    /* a block write                           */
  {
    COUNT rc;

    /* /// Added for SHARE - Ron Cemer */
    if (IsShareInstalled()) {
        if (s->sft_shroff >= 0) {
            int share_result = share_access_check
                (cu_psp,
                 s->sft_shroff,
                 s->sft_posit,
                 (unsigned long)n,
                 1);
            if (share_result != 0) {
                *err = share_result;
                return 0;
            }
		}
    }
    /* /// End of additions for SHARE - Ron Cemer */

    WriteCount = writeblock(s->sft_status, bp, n, &rc);
/*    if (rc < SUCCESS) */
    if (rc == DE_ACCESS  ||     /* -5  Access denied                */
        rc == DE_INVLDHNDL )    /* -6  Invalid handle               */
    {
      *err = rc;
      return 0;
    }
    else
    {
      *err = SUCCESS;
      return WriteCount;
    }
  }
  *err = SUCCESS;
  return 0;
}

COUNT SftSeek(sft FAR *s, LONG new_pos, COUNT mode)
{
  ULONG data;

  /* Test for invalid mode                        */
  if (mode < 0 || mode > 2)
    return DE_INVLDFUNC;

  lpCurSft = (sfttbl FAR *) s;

  if (s->sft_flags & SFT_FSHARED)
  {
    /* seek from end of file */
    if (mode == 2)  {
/*
 *  RB list has it as Note:
 *  this function is called by the DOS 3.1+ kernel, but only when seeking
 *  from the end of a file opened with sharing modes set in such a manner
 *  that another process is able to change the size of the file while it
 *  is already open
 *  Tested this with Shsucdx ver 0.06 and 1.0. Both now work.
 *  Lredir via mfs.c from DosEMU works when writing appended files.
 *  Mfs.c looks for these mode bits set, so here is my best guess.;^)
 */
        if ((s->sft_mode & SFT_MDENYREAD) || (s->sft_mode & SFT_MDENYNONE))
        {
            int2f_Remote_call(REM_LSEEK, 0, (UWORD) FP_SEG(new_pos), (UWORD) FP_OFF(new_pos), (VOID FAR *) s, 0, (VOID FAR *)&data);
            s->sft_posit = data;
            return SUCCESS;
        }
        else
        {
            s->sft_posit = s->sft_size + new_pos;
            return SUCCESS;
        }
    }
    if (mode == 0) {
        s->sft_posit = new_pos;
        return SUCCESS;
    }
    if (mode == 1) {
        s->sft_posit += new_pos;
        return SUCCESS;
    }
    return DE_INVLDFUNC;
    }

    /* Do special return for character devices      */
    if (s->sft_flags & SFT_FDEVICE)
    {
		s->sft_posit = 0l;
        return SUCCESS;
    }
    else
    {
		LONG result = dos_lseek(s->sft_status, new_pos, mode);
		if (result < 0l)
			return (int)result;
		else {
			s->sft_posit = result;
            return SUCCESS;
        }
    }
}

COUNT DosSeek(COUNT hndl, LONG new_pos, COUNT mode, ULONG * set_pos)
{
	sft FAR *s;
	COUNT result;

	/* Get the SFT block that contains the SFT      */
	if ((s = get_sft(hndl)) == (sft FAR *) - 1)
		return DE_INVLDHNDL;

	result = SftSeek(s, new_pos, mode);
	if (result == SUCCESS) {
		*set_pos = s->sft_posit;
	}
	return result;
}

STATIC COUNT get_free_hndl(void)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  WORD hndl;

  for (hndl = 0; hndl < p->ps_maxfiles; hndl++)
  {
    if (p->ps_filetab[hndl] == 0xff)
      return hndl;
  }
  return DE_TOOMANY;
}

sft FAR *get_free_sft(COUNT *sft_idx)
{
  COUNT sys_idx = 0;
  sfttbl FAR *sp;

  /* Get the SFT block that contains the SFT      */
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    REG COUNT i = sp->sftt_count;
    sft FAR *sfti = sp->sftt_table;

    for(sys_idx += i; i >= 1 ; sfti++, i--)
    {
      if (sfti->sft_count == 0)
      {
        *sft_idx = sys_idx - i;
        return sfti;
      }
    }
  }
  /* If not found, return an error                */
  return (sft FAR *) - 1;
}

BYTE FAR *get_root(BYTE FAR * fname)
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

BOOL fnmatch(BYTE FAR * s, BYTE FAR * d, COUNT n, COUNT mode)
{
  while (n--)
  {
    if (!cmatch(*s++, *d++, mode))
      return FALSE;
  }
  return TRUE;
}

COUNT DosCreatSft(BYTE * fname, COUNT attrib)
{
  COUNT sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  WORD result;
  COUNT drive;

                                   /* NEVER EVER allow directories to be created */
  if (attrib & ~(D_RDONLY|D_HIDDEN|D_SYSTEM|D_ARCHIVE))
    {
        return DE_ACCESS;
    }

  /* now get a free system file table entry       */
  if ((sftp = get_free_sft(&sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  fmemset(sftp, 0, sizeof(sft));

  sftp->sft_shroff = -1;  /* /// Added for SHARE - Ron Cemer */
  sftp->sft_psp = cu_psp;
  sftp->sft_mode = SFT_MRDWR;
  sftp->sft_attrib = attrib;
  sftp->sft_psp = cu_psp;

    /* check for a device   */
    dhp = IsDevice(fname);
    if ( dhp )
      {
        sftp->sft_count += 1;
        sftp->sft_flags =
            ((dhp->dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF;
        fbcopy((BYTE FAR *) SecPathName, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
        sftp->sft_dev = dhp;
        return sft_idx;
      }

    drive = get_verify_drive(fname);
    if(drive < 0) {
        return drive;
    }

    if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV) {
		lpCurSft = (sfttbl FAR *)sftp;
		sftp->sft_mode = attrib;
		result = -int2f_Remote_call(REM_CREATE, 0, 0, 0, (VOID FAR *) sftp, 0, MK_FP(0, attrib));
                if (result == SUCCESS) {
                    sftp->sft_count += 1;
                    return sft_idx;
                }
		return result;
  }

/* /// Added for SHARE.  - Ron Cemer */
  if (IsShareInstalled()) {
     if ((sftp->sft_shroff = share_open_check
        ((char far *)fname,
         cu_psp,
         0x02,      /* read-write */
         0)) < 0)        /* compatibility mode */
        return sftp->sft_shroff;
  }
/* /// End of additions for SHARE.  - Ron Cemer */

  sftp->sft_status = dos_creat(fname, attrib);
  if (sftp->sft_status >= 0)
  {
    sftp->sft_count += 1;
    sftp->sft_flags = drive;
    DosGetFile(fname, sftp->sft_name);
    return sft_idx;
  } else {
/* /// Added for SHARE *** CURLY BRACES ADDED ALSO!!! ***.  - Ron Cemer */
    if (IsShareInstalled()) {
        share_close_file(sftp->sft_shroff);
        sftp->sft_shroff = -1;
    }
/* /// End of additions for SHARE.  - Ron Cemer */
    return sftp->sft_status;
  }
}

COUNT DosCreat(BYTE FAR * fname, COUNT attrib)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  COUNT sft_idx, hndl, result;
        
  /* get a free handle  */
  if ((hndl = get_free_hndl()) < 0)
    return hndl;

  result = truename(fname, PriPathName, FALSE);
  if (result != SUCCESS) {
    return result;
  }

  sft_idx = DosCreatSft(PriPathName, attrib);

  if (sft_idx < SUCCESS)
      return sft_idx;
  
  p->ps_filetab[hndl] = sft_idx;
  return hndl;
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
  if ((NewHandle = get_free_hndl()) < 0)
    return NewHandle;

  /* If everything looks ok, bump it up.                          */
  if ((Sftp->sft_flags & (SFT_FDEVICE | SFT_FSHARED)) || (Sftp->sft_status >= 0))
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
  if ((Sftp->sft_flags & (SFT_FDEVICE | SFT_FSHARED)) || (Sftp->sft_status >= 0))
  {
    p->ps_filetab[NewHandle] = p->ps_filetab[OldHandle];

    Sftp->sft_count += 1;
    return NewHandle;
  }
  else
    return DE_INVLDHNDL;
}

COUNT DosOpenSft(BYTE * fname, COUNT mode)
{
  COUNT sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  COUNT drive, result;

  OpenMode = (BYTE) mode;

  /* test if mode is in range                     */
  if ((mode & ~SFT_OMASK) != 0)
    return DE_INVLDACC;

  mode &= 3;

  OpenMode = (BYTE) mode;

  /* now get a free system file table entry       */
  if ((sftp = get_free_sft(&sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  fmemset(sftp, 0, sizeof(sft));
  sftp->sft_psp = cu_psp;
  sftp->sft_mode = mode;

  /* check for a device                           */
  dhp = IsDevice(fname);
  if ( dhp )
  {
    sftp->sft_shroff = -1;  /* /// Added for SHARE - Ron Cemer */


    sftp->sft_count += 1;
    sftp->sft_flags =
        ((dhp->dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF;
    fbcopy((BYTE FAR *) SecPathName, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
    sftp->sft_dev = dhp;
    sftp->sft_date = dos_getdate();
    sftp->sft_time = dos_gettime();
    return sft_idx;
  }

  drive = get_verify_drive(fname);
  if (drive < 0) {
      return drive;
  }

  if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV) {
      lpCurSft = (sfttbl FAR *)sftp;
      result = -int2f_Remote_call(REM_OPEN, 0, 0, 0, (VOID FAR *) sftp, 0, MK_FP(0, mode));
      if (result == SUCCESS) {
          sftp->sft_count += 1;
          return sft_idx;
      }
      return result;
  }
  sftp->sft_shroff = -1;  /* /// Added for SHARE - Ron Cemer */

  /* /// Added for SHARE.  - Ron Cemer */
  if (IsShareInstalled()) {
    if ((sftp->sft_shroff = share_open_check
        ((char far *)fname,
         (unsigned short)cu_psp,
         mode & 0x03,
         (mode >> 2) & 0x07)) < 0)
        return sftp->sft_shroff;
  }
/* /// End of additions for SHARE.  - Ron Cemer */

  sftp->sft_status = dos_open(fname, mode);

  if (sftp->sft_status >= 0)
  {
    f_node_ptr fnp = xlt_fd(sftp->sft_status);

    sftp->sft_attrib = fnp->f_dir.dir_attrib;

    /* Check permissions. -- JPP */
    if ((sftp->sft_attrib & (D_DIR | D_VOLID)) ||
        ((sftp->sft_attrib & D_RDONLY) && (mode != O_RDONLY)))
    {
      return DE_ACCESS;
    }

    sftp->sft_size = dos_getfsize(sftp->sft_status);
    dos_getftime(sftp->sft_status,
	    (date FAR *) & sftp->sft_date,
	    (time FAR *) & sftp->sft_time);
    sftp->sft_count += 1;
    sftp->sft_mode = mode;
    sftp->sft_flags = drive;
    DosGetFile(fname, sftp->sft_name);
    return sft_idx;
  } else {
/* /// Added for SHARE *** CURLY BRACES ADDED ALSO!!! ***.  - Ron Cemer */
    if (IsShareInstalled()) {
        share_close_file(sftp->sft_shroff);
        sftp->sft_shroff = -1;
    }
/* /// End of additions for SHARE.  - Ron Cemer */
    return sftp->sft_status;
  }
}

COUNT DosOpen(BYTE FAR * fname, COUNT mode)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  COUNT sft_idx, result, hndl;

  /* get a free handle                            */
  if ((hndl = get_free_hndl()) < 0)
    return hndl;

  result = truename(fname, PriPathName, FALSE);
  if (result != SUCCESS) {
      return result;
  }

  sft_idx = DosOpenSft(PriPathName, mode);

  if (sft_idx < SUCCESS)
      return sft_idx;
  
  p->ps_filetab[hndl] = sft_idx;
  return hndl;
}

COUNT DosCloseSft(WORD sft_idx)
{
  sft FAR *s = idx_to_sft(sft_idx);
    
  if (s == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If this is not opened another error          */
  /* The second condition is a sanity check and necessary for FcbCloseAll */
  if (s->sft_count == 0 || (s->sft_count == 1 && s->sft_psp != cu_psp))
    return DE_ACCESS;

  lpCurSft = (sfttbl FAR *) s;
/*
   remote sub sft_count.
 */
  if (s->sft_flags & SFT_FSHARED)
    return -int2f_Remote_call(REM_CLOSE, 0, 0, 0, (VOID FAR *) s, 0, 0);

  /* now just drop the count if a device, else    */
  /* call file system handler                     */
  s->sft_count -= 1;
  if (s->sft_flags & SFT_FDEVICE)
    return SUCCESS;
  else
  {
    if (s->sft_count > 0)
      return SUCCESS;
    else {
/* /// Added for SHARE *** CURLY BRACES ADDED ALSO!!! ***.  - Ron Cemer */
      if (IsShareInstalled()) {
          if (s->sft_shroff >= 0) share_close_file(s->sft_shroff);
          s->sft_shroff = -1;
      }
/* /// End of additions for SHARE.  - Ron Cemer */
      return dos_close(s->sft_status);
    }
  }
}

COUNT DosClose(COUNT hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  COUNT ret;

  /* Get the SFT block that contains the SFT      */
  ret = DosCloseSft(get_sft_idx(hndl));
  if (ret != DE_INVLDHNDL && ret != DE_ACCESS)
    p->ps_filetab[hndl] = 0xff;
  return ret;
}

VOID DosGetFree(UBYTE drive, COUNT FAR * spc, COUNT FAR * navc, COUNT FAR * bps, COUNT FAR * nc)
{
  struct dpb FAR *dpbp;
  struct cds FAR *cdsp;
	     COUNT rg[4];

  /* next - "log" in the drive            */
  drive = (drive == 0 ? default_drive : drive - 1);

	/* first check for valid drive          */
  *spc = -1;
  if (drive >= lastdrive)
      return;
  
  cdsp = &CDSp->cds_table[drive];

  if (!(cdsp->cdsFlags & CDSVALID))
      return;
                
  if (cdsp->cdsFlags & CDSNETWDRV)
  {
      int2f_Remote_call(REM_GETSPACE, 0, 0, 0, cdsp, 0, &rg);
		
      *spc  = (COUNT) rg[0];
      *nc   = (COUNT) rg[1];
      *bps  = (COUNT) rg[2];
      *navc = (COUNT) rg[3]; 
      return;
  }

  dpbp = CDSp->cds_table[drive].cdsDpb;
  if (dpbp == NULL || media_check(dpbp) < 0)
      return;

  /* get the data vailable from dpb       */
  *nc = dpbp->dpb_size;
  *spc = dpbp->dpb_clsmask + 1;
  *bps = dpbp->dpb_secsize;

  /* now tell fs to give us free cluster  */
  /* count                                */
  *navc = dos_free(dpbp);
}

COUNT DosGetCuDir(UBYTE drive, BYTE FAR * s)
{
  BYTE FAR *cp;
    
  /* next - "log" in the drive            */
  drive = (drive == 0 ? default_drive : drive - 1);

  /* first check for valid drive          */
  if (drive >= lastdrive || !(CDSp->cds_table[drive].cdsFlags & CDSVALID)) {
    return DE_INVLDDRV;
  }
  
  current_ldt = &CDSp->cds_table[drive];

  cp = &current_ldt->cdsCurrentPath[current_ldt->cdsJoinOffset];
  if (*cp == '\0')
    s[0]='\0';
  else
    fstrncpy(s, cp+1, 64);

  return SUCCESS;
}

#undef CHDIR_DEBUG
COUNT DosChangeDir(BYTE FAR * s)
{
  REG struct cds FAR *cdsp;
  REG COUNT drive;
	COUNT result;
	BYTE FAR *p;
	
	                            /* don't do wildcard CHDIR --TE*/
    for (p = s; *p; p++)
        if (*p == '*' || *p == '?')
            return DE_PATHNOTFND;
	
	

    drive = get_verify_drive(s);
    if (drive < 0 ) {
        return drive;
    }

    result = truename(s, PriPathName, FALSE);
	if (result != SUCCESS) {
		return result;
    }

  cdsp = &CDSp->cds_table[drive];
  current_ldt = cdsp;

	if (cdsp->cdsFlags & CDSNETWDRV)
  {
#if defined(CHDIR_DEBUG)
        printf("Remote Chdir: n='");
        p = s; while(*p)    printf("%c", *p++);
        printf("' p='");
        p = PriPathName; while(*p) printf("%c", *p++);
        printf("'\n");
#endif
	result = -int2f_Remote_call(REM_CHDIR, 0, 0, 0, PriPathName, 0, 0);
#if defined(CHDIR_DEBUG)
        printf("status = %04x, new_path='", result);
        p = cdsd->cdsCurrentPath; while(p) printf("%c", *p++)
        printf("'\n");
#endif
		if (result != SUCCESS) {
                return DE_PATHNOTFND;
		}
/*
	Some redirectors do not write back to the CDS.
	SHSUCdX needs this. jt
*/
    fstrncpy(cdsp->cdsCurrentPath,&PriPathName[0],sizeof(cdsp->cdsCurrentPath)-1);
	if (PriPathName[7] == 0)
		cdsp->cdsCurrentPath[8] = 0;	/* Need two Zeros at the end */

	} else {
  /* now get fs to change to new          */
  /* directory                            */
		result = dos_cd(cdsp, PriPathName);
	}
	if (result == SUCCESS) {
        fstrncpy(cdsp->cdsCurrentPath,&PriPathName[0],sizeof(cdsp->cdsCurrentPath)-1);
	}
	return result;
}

STATIC VOID pop_dmp(dmatch FAR * dmp)
{
  dmp->dm_attr_fnd = (BYTE) SearchDir.dir_attrib;
  dmp->dm_time = SearchDir.dir_time;
  dmp->dm_date = SearchDir.dir_date;
  dmp->dm_size = (LONG) SearchDir.dir_size;
  ConvertName83ToNameSZ(dmp->dm_name, (BYTE FAR *)SearchDir.dir_name);
}

COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name)
{
  COUNT rc;
  REG dmatch FAR *dmp = (dmatch FAR *) dta;
  BYTE FAR *p;

      /* /// Added code here to do matching against device names.
           DOS findfirst will match exact device names if the
           filename portion (excluding the extension) contains
           a valid device name.
           Credits: some of this code was ripped off from truename()
           in newstuff.c.
           - Ron Cemer */

  fmemset(dta, 0, sizeof(dmatch));
  
  /* initially mark the dta as invalid for further findnexts */
  ((dmatch FAR *)dta)->dm_attr_fnd = D_DEVICE;
  
  memset(&SearchDir, 0, sizeof(struct dirent));
  
  rc = truename(name, PriPathName, FALSE);
  if (rc != SUCCESS)
      return rc;
  
  if (IsDevice(PriPathName))
  {
    COUNT i;
      
    /* Found a matching device. Hence there cannot be wildcards. */
    SearchDir.dir_attrib = D_DEVICE;
    SearchDir.dir_time = dos_gettime();
    SearchDir.dir_date = dos_getdate();
    p = get_root(PriPathName);
    memset(SearchDir.dir_name, ' ', FNAME_SIZE+FEXT_SIZE);
    for (i = 0; i < FNAME_SIZE && *p && *p != '.'; i++)
        SearchDir.dir_name[i] = *p++;
    if (*p == '.') p++;
    for (i = 0; i < FEXT_SIZE && *p && *p != '.'; i++)
        SearchDir.dir_ext[i] = *p++;
    pop_dmp(dmp);
    return SUCCESS;
  }
  /* /// End of additions.  - Ron Cemer ; heavily edited - Bart Oldeman */

  SAttr = (BYTE) attr;

#if defined(FIND_DEBUG)
  printf("Remote Find: n='%Fs\n", PriPathName);
#endif

  fmemcpy(TempBuffer, dta, 21);
  p = dta;
  dta = (BYTE FAR *)TempBuffer;
  
  rc = current_ldt->cdsFlags & CDSNETWDRV ?
        -int2f_Remote_call(REM_FINDFIRST, 0, 0, 0, (VOID FAR *)current_ldt, 0, 0) :
        dos_findfirst(attr, PriPathName);

  dta = p;

  fmemcpy(dta, TempBuffer, 21);
  pop_dmp((dmatch FAR *)dta);
  if (rc != SUCCESS) ((dmatch FAR *)dta)->dm_attr_fnd = D_DEVICE; /* mark invalid */ 
  return rc;
}

COUNT DosFindNext(void)
{
  COUNT rc;
  BYTE FAR *p;

  /* /// findnext will always fail on a device name.  - Ron Cemer */
  if (((dmatch FAR *)dta)->dm_attr_fnd == D_DEVICE)
      return DE_NFILES;

/*
 *  The new version of SHSUCDX 1.0 looks at the dm_drive byte to
 *  test 40h. I used RamView to see location MSD 116:04be and
 *  FD f??:04be, the byte set with 0xc4 = Remote/Network drive 4.
 *  Ralf Brown docs for dos 4eh say bit 7 set == remote so what is
 *  bit 6 for? 
 *  SHSUCDX Mod info say "test redir not network bit".
 *  Just to confuse the rest, MSCDEX sets bit 5 too.
 *
 *  So, assume bit 6 is redirector and bit 7 is network.
 *  jt
 *  Bart: dm_drive can be the drive _letter_.
 *  but better just stay independent of it: we only use
 *  bit 7 to detect a network drive; the rest untouched.
 *  RBIL says that findnext can only return one error type anyway
 *  (12h, DE_NFILES)
 */
#if 0
  printf("findnext: %d\n", 
	  ((dmatch FAR *)dta)->dm_drive);
#endif
  fmemcpy(TempBuffer, dta, 21);
  fmemset(dta, 0, sizeof(dmatch));
  p = dta;
  dta = (BYTE FAR *)TempBuffer;
  rc = (((dmatch *)TempBuffer)->dm_drive & 0x80) ?
      -int2f_Remote_call(REM_FINDNEXT, 0, 0, 0, (VOID FAR *)current_ldt, 0, 0) :
      dos_findnext();
  
  dta = p;
  fmemcpy(dta, TempBuffer, 21);
  pop_dmp((dmatch FAR *)dta);
  return rc;
}

COUNT DosGetFtime(COUNT hndl, date FAR * dp, time FAR * tp)
{
  sft FAR *s;
/*sfttbl FAR *sp;*/

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

COUNT DosSetFtimeSft(WORD sft_idx, date dp, time tp)
{
  /* Get the SFT block that contains the SFT      */
  sft FAR *s = idx_to_sft(sft_idx);
    
  if (s == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return DE_ACCESS;

  /* If SFT entry refers to a device, do nothing */
  if (s->sft_flags & SFT_FDEVICE)
    return SUCCESS;

  s->sft_flags |= SFT_FDATE;
  s->sft_date = dp;
  s->sft_time = tp;

  if (s->sft_flags & SFT_FSHARED)
    return SUCCESS;

  /* call file system handler                     */
  return dos_setftime(s->sft_status, dp, tp);
}

COUNT DosGetFattr(BYTE FAR * name)
{
	UWORD srfa[5];
	COUNT result, drive;
	struct cds FAR *last_cds;

    if (IsDevice(name)) {
		return DE_FILENOTFND;
	}

    drive = get_verify_drive(name);
	if (drive < 0) {
		return drive;
	}

	result = truename(name, PriPathName, FALSE);
	if (result != SUCCESS) {
		return result;
	}

/* /// Added check for "d:\", which returns 0x10 (subdirectory) under DOS.
       - Ron Cemer */
    if (   (PriPathName[0] != '\0')
        && (PriPathName[1] == ':')
        && ( (PriPathName[2] == '/') || (PriPathName[2] == '\\') )
        && (PriPathName[3] == '\0')   ) {
        return 0x10;
    }

    if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV)
	{
		last_cds = current_ldt;
		current_ldt = &CDSp->cds_table[drive];
		result = -int2f_Remote_call(REM_GETATTRZ, 0, 0, 0, 0, 0, (VOID FAR *) srfa);
		current_ldt = last_cds;
		return (result < SUCCESS ? result : srfa[0]);
	}
	else {
/* /// Use truename()'s result, which we already have in PriPathName.
       I copy it to tmp_name because PriPathName is global and seems
       to get trashed somewhere in transit.
       The reason for using truename()'s result is that dos_?etfattr()
       are very low-level functions and don't handle full path expansion
       or cleanup, such as converting "c:\a\b\.\c\.." to "C:\A\B".
       - Ron Cemer
*/
/*
          memcpy(SecPathName,PriPathName,sizeof(SecPathName));
          return dos_getfattr(SecPathName, attrp);
*/
    /* no longer true. dos_getfattr() is 
       A) intelligent (uses dos_open) anyway
       B) there are some problems with MAX_PARSE, i.e. if PATH ~= 64
          and TRUENAME adds a C:, which leeds to trouble. 
       
       the problem was discovered, when VC did something like
       
            fd = DosOpen(filename,...)
            jc can't_copy_dialog;

            attr = DosGetAttrib(filename);
            jc can't_copy_dialog;
                                    and suddenly, the filehandle stays open
                                    shit.
          tom
    */ 
          return dos_getfattr(PriPathName);

	}
}

COUNT DosSetFattr(BYTE FAR * name, UWORD attrp)
{
	COUNT result, drive;
	struct cds FAR *last_cds;

    if (IsDevice(name) ) {
		return DE_FILENOTFND;
	}

    drive = get_verify_drive(name);
	if (drive < 0) {
		return drive;
	}

	result = truename(name, PriPathName, FALSE);
	if (result != SUCCESS) {
		return result;
	}

	if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV)
	{
		last_cds = current_ldt;
		current_ldt = &CDSp->cds_table[drive];
		result = -int2f_Remote_call(REM_SETATTR, 0, 0, 0, 0, 0, MK_FP(0, attrp));
		current_ldt = last_cds;
		return result;
	}
	else {
/* /// Use truename()'s result, which we already have in PriPathName.
       I copy it to tmp_name because PriPathName is global and seems
       to get trashed somewhere in transit.
       - Ron Cemer
*/
/*
          memcpy(SecPathName,PriPathName,sizeof(SecPathName));
          return dos_setfattr(SecPathName, attrp);
          
          see DosGetAttr()
*/          
          return dos_setfattr(PriPathName, attrp);

        }
}

UBYTE DosSelectDrv(UBYTE drv)
{
  struct cds FAR *cdsp = &CDSp->cds_table[drv];
    
  if ((drv < lastdrive) && (cdsp->cdsFlags & CDSVALID))
/*
      &&
      ((cdsp->cdsFlags & CDSNETWDRV) ||
       (cdsp->cdsDpb!=NULL && media_check(cdsp->cdsDpb)==SUCCESS)))
*/       
  {
    current_ldt = cdsp;
    default_drive = drv;
  }
  return lastdrive;
}

COUNT DosDelete(BYTE FAR *path)
{
	COUNT result, drive;

    if (IsDevice(path)) {
		return DE_FILENOTFND;
	}

    drive = get_verify_drive(path);
	if (drive < 0) {
		return drive;
	}
    result = truename(path, PriPathName, FALSE);
    if (result != SUCCESS) {
		return result;
	}
    current_ldt = &CDSp->cds_table[drive];
	if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV) {
		return -int2f_Remote_call(REM_DELETE, 0, 0, 0, 0, 0, 0);
	}  else {
		return dos_delete(PriPathName);
	}
}

COUNT DosRenameTrue(BYTE * path1, BYTE * path2)
{
    COUNT drive1, drive2;

    if (IsDevice(path1) || IsDevice(path2)) {
        return DE_FILENOTFND;
    }

    drive1 = get_verify_drive(path1);
    drive2 = get_verify_drive(path2);
    if ((drive1 != drive2) || (drive1 < 0)) {
        return DE_INVLDDRV;
    }
    current_ldt = &CDSp->cds_table[drive1];
    if (CDSp->cds_table[drive1].cdsFlags & CDSNETWDRV) {
        return -int2f_Remote_call(REM_RENAME, 0, 0, 0, 0, 0, 0);
    } else {
        return dos_rename(PriPathName, SecPathName);
    }
}

COUNT DosRename(BYTE FAR * path1, BYTE FAR * path2)
{
    COUNT result;

    result = truename(path1, PriPathName, FALSE);
    if (result != SUCCESS) {
		return result;
	}

    result = truename(path2, SecPathName, FALSE);
    if (result != SUCCESS) {
		return result;
	}

    return DosRenameTrue(PriPathName, SecPathName);
}

COUNT DosMkdir(BYTE FAR * dir)
{
	COUNT result, drive;

    if (IsDevice(dir)) {
		return DE_PATHNOTFND;
	}

    drive = get_verify_drive(dir);
	if (drive < 0) {
		return drive;
	}
	result = truename(dir, PriPathName, FALSE);
	if (result != SUCCESS) {
		return result;
	}
    current_ldt = &CDSp->cds_table[drive];
	if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV) {
		return -int2f_Remote_call(REM_MKDIR, 0, 0, 0, 0, 0, 0);
	}  else {
		return dos_mkdir(PriPathName);
	}
}

COUNT DosRmdir(BYTE FAR * dir)
{
	COUNT result, drive;

    if (IsDevice(dir)) {
		return DE_PATHNOTFND;
	}

    drive = get_verify_drive(dir);
	if (drive < 0) {
		return drive;
	}
	result = truename(dir, PriPathName, FALSE);
	if (result != SUCCESS) {
		return result;
	}
    current_ldt = &CDSp->cds_table[drive];
	if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV) {
		return -int2f_Remote_call(REM_RMDIR, 0, 0, 0, 0, 0, 0);
	}  else {
		return dos_rmdir(PriPathName);
	}
}

/* /// Added for SHARE.  - Ron Cemer */

COUNT DosLockUnlock(COUNT hndl, LONG pos, LONG len, COUNT unlock)
{
  sft FAR *s;

  /* Invalid function unless SHARE is installed. */
  if (!IsShareInstalled()) return DE_INVLDFUNC;

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) -1) return DE_INVLDHNDL;

  /* Lock violation if this SFT entry does not support locking. */
  if (s->sft_shroff < 0) return DE_LOCK;

  /* Let SHARE do the work. */
  return share_lock_unlock(cu_psp, s->sft_shroff, pos, len, unlock);
}

/* /// End of additions for SHARE.  - Ron Cemer */

/*
 * This seems to work well.
 */

struct dhdr FAR * IsDevice(BYTE FAR * fname)
{
  struct dhdr FAR *dhp;
  BYTE FAR *froot;
  WORD i;
  BYTE tmpPathName[FNAME_SIZE+1];

  /* check for a device  */
  froot = get_root(fname);
  for (i = 0; i < FNAME_SIZE; i++)
  {
    if (*froot != '\0' && *froot != '.')
      tmpPathName[i] = *froot++;
    else
      break;
  }

  for (; i < FNAME_SIZE; i++)
    tmpPathName[i] = ' ';

  tmpPathName[i] = 0;

/* /// BUG!!! This is absolutely wrong.  A filename of "NUL.LST" must be
       treated EXACTLY the same as a filename of "NUL".  The existence or
       content of the extension is irrelevent in determining whether a
       filename refers to a device.
       - Ron Cemer
  // if we have an extension, can't be a device <--- WRONG.
  if (*froot != '.')
  {
*/

    for (dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1; dhp = dhp->dh_next)
    {

                    /*  BUGFIX: MSCD000<00> should be handled like MSCD000<20> TE */

      char dev_name_buff[FNAME_SIZE];

      int namelen = fstrlen(dhp->dh_name);
      
      memset(dev_name_buff, ' ', FNAME_SIZE);
      
      fmemcpy(dev_name_buff,dhp->dh_name, min(namelen,FNAME_SIZE)); 
      
      if (fnmatch((BYTE FAR *) tmpPathName, (BYTE FAR *) dev_name_buff, FNAME_SIZE, FALSE))
      {
	memcpy(SecPathName, tmpPathName, i+1);
        return dhp;
      }
    }

  return (struct dhdr FAR *)0;
}


/* /// Added for SHARE.  - Ron Cemer */

BOOL IsShareInstalled(void) {
    if (!share_installed) {
        iregs regs;

        regs.a.x = 0x1000;
        intr(0x2f, &regs);
        share_installed = ((regs.a.x & 0xff) == 0xff);
    }
    return share_installed;
}

	/* DOS calls this to see if it's okay to open the file.
	   Returns a file_table entry number to use (>= 0) if okay
	   to open.  Otherwise returns < 0 and may generate a critical
	   error.  If < 0 is returned, it is the negated error return
	   code, so DOS simply negates this value and returns it in
	   AX. */
static int share_open_check
    (char far *filename,    /* far pointer to fully qualified filename */
     unsigned short pspseg, /* psp segment address of owner process */
     int openmode,          /* 0=read-only, 1=write-only, 2=read-write */
     int sharemode) {       /* SHARE_COMPAT, etc... */
    iregs regs;

    regs.a.x = 0x10a0;
    regs.ds = FP_SEG(filename);
    regs.si = FP_OFF(filename);
    regs.b.x = pspseg;
    regs.c.x = openmode;
    regs.d.x = sharemode;
    intr(0x2f, &regs);
    return (int)regs.a.x;
}

	/* DOS calls this to record the fact that it has successfully
	   closed a file, or the fact that the open for this file failed. */
static void share_close_file
    (int fileno) {          /* file_table entry number */
    iregs regs;

    regs.a.x = 0x10a1;
    regs.b.x = fileno;
    intr(0x2f, &regs);
}

	/* DOS calls this to determine whether it can access (read or
	   write) a specific section of a file.  We call it internally
	   from lock_unlock (only when locking) to see if any portion
       of the requested region is already locked.  If pspseg is zero,
	   then it matches any pspseg in the lock table.  Otherwise, only
	   locks which DO NOT belong to pspseg will be considered.
	   Returns zero if okay to access or lock (no portion of the
	   region is already locked).  Otherwise returns non-zero and
	   generates a critical error (if allowcriter is non-zero).
	   If non-zero is returned, it is the negated return value for
	   the DOS call. */
static int share_access_check
    (unsigned short pspseg,/* psp segment address of owner process */
	 int fileno,		/* file_table entry number */
	 unsigned long ofs,	/* offset into file */
     unsigned long len, /* length (in bytes) of region to access */
     int allowcriter) { /* allow a critical error to be generated */
    iregs regs;

    regs.a.x = 0x10a2 | (allowcriter ? 0x01 : 0x00);
    regs.b.x = pspseg;
    regs.c.x = fileno;
    regs.si = (unsigned short)((ofs >> 16) & 0xffffL);
    regs.di = (unsigned short)(ofs & 0xffffL);
    regs.es = (unsigned short)((len >> 16) & 0xffffL);
    regs.d.x = (unsigned short)(len & 0xffffL);
    intr(0x2f, &regs);
    return (int)regs.a.x;
}

	/* DOS calls this to lock or unlock a specific section of a file.
	   Returns zero if successfully locked or unlocked.  Otherwise
	   returns non-zero.
	   If the return value is non-zero, it is the negated error
	   return code for the DOS 0x5c call. */
static int share_lock_unlock
    (unsigned short pspseg,/* psp segment address of owner process */
	 int fileno,		/* file_table entry number */
	 unsigned long ofs,	/* offset into file */
     unsigned long len, /* length (in bytes) of region to lock or unlock */
     int unlock) {      /* non-zero to unlock; zero to lock */
    iregs regs;

    regs.a.x = 0x10a4 | (unlock ? 0x01 : 0x00);
    regs.b.x = pspseg;
    regs.c.x = fileno;
    regs.si = (unsigned short)((ofs >> 16) & 0xffffL);
    regs.di = (unsigned short)(ofs & 0xffffL);
    regs.es = (unsigned short)((len >> 16) & 0xffffL);
    regs.d.x = (unsigned short)(len & 0xffffL);
    intr(0x2f, &regs);
    return (int)regs.a.x;
}

/* /// End of additions for SHARE.  - Ron Cemer */

