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
static BYTE *dosfnsRcsId =
    "$Id$";
#endif

#include "globals.h"

/* /// Added for SHARE.  - Ron Cemer */

BYTE share_installed = 0;

        /* DOS calls this to see if it's okay to open the file.
           Returns a file_table entry number to use (>= 0) if okay
           to open.  Otherwise returns < 0 and may generate a critical
           error.  If < 0 is returned, it is the negated error return
           code, so DOS simply negates this value and returns it in
           AX. */
STATIC int share_open_check(char * filename,            /* pointer to fully qualified filename */
                            unsigned short pspseg,      /* psp segment address of owner process */
                            int openmode,       /* 0=read-only, 1=write-only, 2=read-write */
                            int sharemode);     /* SHARE_COMPAT, etc... */

        /* DOS calls this to record the fact that it has successfully
           closed a file, or the fact that the open for this file failed. */
STATIC void share_close_file(int fileno);       /* file_table entry number */

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
STATIC int share_access_check(unsigned short pspseg,    /* psp segment address of owner process */
                              int fileno,       /* file_table entry number */
                              unsigned long ofs,        /* offset into file */
                              unsigned long len,        /* length (in bytes) of region to access */
                              int allowcriter); /* allow a critical error to be generated */

        /* DOS calls this to lock or unlock a specific section of a file.
           Returns zero if successfully locked or unlocked.  Otherwise
           returns non-zero.
           If the return value is non-zero, it is the negated error
           return code for the DOS 0x5c call. */
STATIC int share_lock_unlock(unsigned short pspseg,     /* psp segment address of owner process */
                             int fileno,        /* file_table entry number */
                             unsigned long ofs, /* offset into file */
                             unsigned long len, /* length (in bytes) of region to lock or unlock */
                             int unlock);       /* non-zero to unlock; zero to lock */

/* /// End of additions for SHARE.  - Ron Cemer */

STATIC int remote_lock_unlock(sft FAR *sftp,  /* SFT for file */
                             unsigned long ofs, /* offset into file */
                             unsigned long len, /* length (in bytes) of region to lock or unlock */
                             int unlock);       /* non-zero to unlock; zero to lock */

/* get current directory structure for drive
   return NULL if the CDS is not valid or the
   drive is not within range */
struct cds FAR *get_cds(int drive)
{
  struct cds FAR *cdsp;
  unsigned flags;

  if (drive >= lastdrive)
    return NULL;
  cdsp = &CDSp[drive];
  flags = cdsp->cdsFlags;
  /* Entry is disabled or JOINed drives are accessable by the path only */
  if (!(flags & CDSVALID) || (flags & CDSJOINED) != 0)
    return NULL;
  if (!(flags & CDSNETWDRV) && cdsp->cdsDpb == NULL)
    return NULL;
  return cdsp;
}

#ifdef WITHFAT32
struct dpb FAR * GetDriveDPB(UBYTE drive, COUNT * rc)
{
  struct dpb FAR *dpb;
  struct cds FAR *cdsp;
  
  cdsp = get_cds(drive == 0 ? default_drive : drive - 1);
  
  if (cdsp == NULL)
  {
    *rc = DE_INVLDDRV;
    return 0;
  }
  dpb = cdsp->cdsDpb;
  if (dpb == 0 || cdsp->cdsFlags & CDSNETWDRV)
  {
    *rc = DE_INVLDDRV;
    return 0;
  }

  *rc = SUCCESS;
  return dpb;
}
#endif

/* Construct dir-style filename for ASCIIZ 8.3 name */
STATIC VOID DosGetFile(BYTE * lpszPath, BYTE FAR * lpszDosFileName)
{
  char fcbname[FNAME_SIZE + FEXT_SIZE];

  ParseDosName(lpszPath, fcbname, FALSE);
  fmemcpy(lpszDosFileName, fcbname, FNAME_SIZE + FEXT_SIZE);
}

sft FAR * idx_to_sft(int SftIndex)
{
  sfttbl FAR *sp;

  if (SftIndex < 0)
    return (sft FAR *) - 1;

  /* Get the SFT block that contains the SFT      */
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    if (SftIndex < sp->sftt_count)
    {
      lpCurSft = (sft FAR *) & (sp->sftt_table[SftIndex]);

      /* if not opened, the SFT is useless            */
      if (lpCurSft->sft_count == 0)
        return (sft FAR *) - 1;
      
      /* finally, point to the right entry            */
      return lpCurSft;
    }
    else
      SftIndex -= sp->sftt_count;
  }
  /* If not found, return an error                */

  return (sft FAR *) - 1;
}

int get_sft_idx(unsigned hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  int idx;

  if (hndl >= p->ps_maxfiles)
    return DE_INVLDHNDL;

  idx = p->ps_filetab[hndl];
  return idx == 0xff ? DE_INVLDHNDL : idx;
}

sft FAR *get_sft(UCOUNT hndl)
{
  /* Get the SFT block that contains the SFT      */
  return idx_to_sft(get_sft_idx(hndl));
}

long DosRWSft(int sft_idx, size_t n, void FAR * bp, int mode)
{
  /* Get the SFT block that contains the SFT      */
  sft FAR *s = idx_to_sft(sft_idx);

  if (FP_OFF(s) == (size_t) - 1)
  {
    return DE_INVLDHNDL;
  }
  /* If for read and write-only or for write and read-only then exit */
  if((mode == XFR_READ && (s->sft_mode & SFT_MWRITE)) ||
     (mode == XFR_WRITE && !(s->sft_mode & (SFT_MWRITE | SFT_MRDWR))))
  {
    return DE_ACCESS;
  }
  if (mode == XFR_FORCE_WRITE)
    mode = XFR_WRITE;
    
/*
 *   Do remote first or return error.
 *   must have been opened from remote.
 */
  if (s->sft_flags & SFT_FSHARED)
  {
    UCOUNT XferCount;
    VOID FAR *save_dta;
    int err;

    save_dta = dta;
    lpCurSft = s;
    current_filepos = s->sft_posit;     /* needed for MSCDEX */
    dta = bp;
    XferCount = (mode == XFR_READ ? remote_read : remote_write)(s, n, &err);
    dta = save_dta;
    return err == SUCCESS ? XferCount : err;
  }

  /* Do a device transfer if device                   */
  if (s->sft_flags & SFT_FDEVICE)
  {
    /* Now handle raw and cooked modes      */
    if (s->sft_flags & SFT_FBINARY)
    {
      long rc = BinaryCharIO(s->sft_dev, n, bp,
                             mode == XFR_READ ? C_INPUT : C_OUTPUT);
      if (mode == XFR_WRITE && rc > 0 && (s->sft_flags & SFT_FCONOUT))
      {
        size_t cnt = (size_t)rc;
        const char FAR *p = bp;
        while (cnt--)
        {
          switch (*p++)
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
      return rc;
    }

    /* cooked mode */
    if (mode==XFR_READ)
    {
      long rc;
      
      /* Test for eof and exit                */
      /* immediately if it is                 */
      if (!(s->sft_flags & SFT_FEOF) || (s->sft_flags & SFT_FNUL))
        return 0;

      if (s->sft_flags & SFT_FCONIN)
        rc = read_line_handle(sft_idx, n, bp);
      else
        rc = cooked_read(sft_idx, n, bp);
      if (*(char *)bp == CTL_Z)
        s->sft_flags &= ~SFT_FEOF;
      return rc;
    }
    else
    {
      /* reset EOF state (set to no EOF)      */  
      s->sft_flags |= SFT_FEOF;

      /* if null just report full transfer    */
      if (s->sft_flags & SFT_FNUL)
        return n;
      else
        return cooked_write(sft_idx, n, bp);
    }
  }

  /* a block transfer                           */
  /* /// Added for SHARE - Ron Cemer */
  if (IsShareInstalled() && (s->sft_shroff >= 0))
  {
    int rc = share_access_check(cu_psp, s->sft_shroff, s->sft_posit,
                                 (unsigned long)n, 1);
    if (rc != SUCCESS)
      return rc;
  }
  /* /// End of additions for SHARE - Ron Cemer */
  {
    UCOUNT XferCount;
    XferCount = rwblock(s->sft_status, bp, n, mode);
    if (mode == XFR_WRITE)
      s->sft_size = dos_getfsize(s->sft_status);
    s->sft_posit += XferCount;
    return XferCount;
  }
}

COUNT SftSeek(int sft_idx, LONG new_pos, COUNT mode)
{
  sft FAR *s = idx_to_sft(sft_idx);
  if (FP_OFF(s) == (size_t) -1)
    return DE_INVLDHNDL;
        
  /* Test for invalid mode                        */
  if (mode < 0 || mode > 2)
    return DE_INVLDFUNC;

  lpCurSft = s;

  if (s->sft_flags & SFT_FSHARED)
  {
    /* seek from end of file */
    if (mode == 2)
    {
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
        s->sft_posit = remote_lseek(s, new_pos);
        return SUCCESS;
      }
      else
      {
        s->sft_posit = s->sft_size + new_pos;
        return SUCCESS;
      }
    }
    if (mode == 0)
    {
      s->sft_posit = new_pos;
      return SUCCESS;
    }
    if (mode == 1)
    {
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
    else
    {
      s->sft_posit = result;
      return SUCCESS;
    }
  }
}

ULONG DosSeek(unsigned hndl, LONG new_pos, COUNT mode)
{
  int sft_idx = get_sft_idx(hndl);
  COUNT result;

  /* Get the SFT block that contains the SFT      */
  result = SftSeek(sft_idx, new_pos, mode);
  if (result == SUCCESS)
  {
    return idx_to_sft(sft_idx)->sft_posit;
  }
  return (ULONG)-1;
}

STATIC long get_free_hndl(void)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  unsigned hndl;

  for (hndl = 0; hndl < p->ps_maxfiles; hndl++)
  {
    if (p->ps_filetab[hndl] == 0xff)
      return hndl;
  }
  return DE_TOOMANY;
}

STATIC sft FAR *get_free_sft(COUNT * sft_idx)
{
  COUNT sys_idx = 0;
  sfttbl FAR *sp;

  /* Get the SFT block that contains the SFT      */
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    REG COUNT i = sp->sftt_count;
    sft FAR *sfti = sp->sftt_table;

    for (; --i >= 0; sys_idx++, sfti++)
    {
      if (sfti->sft_count == 0)
      {
        *sft_idx = sys_idx;

        /* MS NET uses this on open/creat TE */
        {
          extern WORD ASM current_sft_idx;
          current_sft_idx = sys_idx;
        }

        return sfti;
      }
    }
  }
  /* If not found, return an error                */
  return (sft FAR *) - 1;
}

const char FAR *get_root(const char FAR * fname)
{
  /* find the end                                 */
  register unsigned length = fstrlen(fname) - 1;
  char c;

  /* now back up to first path seperator or start */
  while (length != (unsigned)-1)
  {
    c = fname[length];
    if (c == '/' || c == '\\' || c == ':')
      break;
    --length;
  }
  return fname + length + 1;
}

/* initialize SFT fields (for open/creat) for character devices */
STATIC void DeviceOpenSft(struct dhdr FAR *dhp, sft FAR *sftp)
{
  int i;

  sftp->sft_shroff = -1;      /* /// Added for SHARE - Ron Cemer */
  sftp->sft_count += 1;
  sftp->sft_flags =
    ((dhp->
      dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF;
  fmemcpy(sftp->sft_name, dhp->dh_name, FNAME_SIZE);

  /* pad with spaces */
  for (i = FNAME_SIZE + FEXT_SIZE - 1; sftp->sft_name[i] == '\0'; i--)
    sftp->sft_name[i] = ' ';
  /* and uppercase */
  DosUpFMem(sftp->sft_name, FNAME_SIZE + FEXT_SIZE);

  sftp->sft_dev = dhp;
  sftp->sft_date = dos_getdate();
  sftp->sft_time = dos_gettime();
  sftp->sft_attrib = D_DEVICE;
}

/*
extended open codes
0000 0000 always fail
0000 0001 open O_OPEN
0000 0010 replace O_TRUNC

0001 0000 create new file O_CREAT
0001 0001 create if not exists, open if exists O_CREAT | O_OPEN
0001 0010 create O_CREAT | O_TRUNC

bits for flags (bits 11-8 are internal FreeDOS bits only)
15 O_FCB  called from FCB open
14 O_SYNC commit for each write (not implemented yet)
13 O_NOCRIT do not invoke int23 (not implemented yet)
12 O_LARGEFILE allow files >= 2gb but < 4gb (not implemented yet)
11 O_LEGACY not called from int21/ah=6c: find right fn for redirector
10 O_CREAT if file does not exist, create it
9 O_TRUNC if file exists, truncate and open it \ not both 
8 O_OPEN  if file exists, open it              /
7 O_NOINHERIT do not inherit handle on exec
6 \ 
5  - sharing modes
4 / 
3 reserved 
2 bits 2,1,0 = 100: RDONLY and do not modify file's last access time
                    (not implemented yet)
1 \ 0=O_RDONLY, 1=O_WRONLY,
0 / 2=O_RDWR, 3=O_EXECCASE (preserve case for redirector EXEC,
                            (not implemented yet))
*/

long DosOpenSft(char FAR * fname, unsigned flags, unsigned attrib)
{
  COUNT sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  long result;

  result = truename(fname, PriPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
    return result;

  /* now get a free system file table entry       */
  if ((sftp = get_free_sft(&sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  fmemset(sftp, 0, sizeof(sft));

  sftp->sft_psp = cu_psp;
  sftp->sft_mode = flags & 0xf0ff;
  OpenMode = (BYTE) flags;

  sftp->sft_shroff = -1;        /* /// Added for SHARE - Ron Cemer */
  sftp->sft_attrib = attrib = attrib | D_ARCHIVE;

  if (result & IS_NETWORK)
  {
    int status;
    if ((flags & (O_TRUNC | O_CREAT)) == O_CREAT)
      attrib |= 0x100;
  
    lpCurSft = sftp;
    if (!(flags & O_LEGACY))
    {
      extern UWORD ASM ext_open_mode, ASM ext_open_attrib, ASM ext_open_action;
      ext_open_mode = flags & 0x70ff;
      ext_open_attrib = attrib & 0xff;
      ext_open_action = ((flags & 0x0300) >> 8) | ((flags & O_CREAT) >> 6);
      status = remote_extopen(sftp, attrib);
    }
    else if (flags & O_CREAT)
      status = remote_creat(sftp, attrib);
    else
      status = remote_open(sftp, (BYTE)flags);
    if (status >= SUCCESS)
    {
      if (sftp->sft_count == 0)
        sftp->sft_count++;
      return sft_idx | ((long)status << 16);
    }
    return status;
  }

  /* check for a device   */
  if ((result & IS_DEVICE) && (dhp = IsDevice(fname)) != NULL)
  {
    DeviceOpenSft(dhp, sftp);
    return sft_idx;
  }

/* /// Added for SHARE.  - Ron Cemer */
  if (IsShareInstalled())
  {
    if ((sftp->sft_shroff =
         share_open_check(PriPathName, cu_psp,
                          flags & 0x03, (flags >> 4) & 0x07)) < 0)
      return sftp->sft_shroff;
  }
  
/* /// End of additions for SHARE.  - Ron Cemer */

  /* NEVER EVER allow directories to be created */
  /* ... though FCB's are weird :) */
  if (!(flags & O_FCB) &&
      (attrib & ~(D_RDONLY | D_HIDDEN | D_SYSTEM | D_ARCHIVE | D_VOLID)))
    return DE_ACCESS;
  
  result = dos_open(PriPathName, flags, attrib);
  if (result >= 0)
  {
    int status = (int)(result >> 16);
    if (status == S_OPENED)
    {
      sftp->sft_attrib = dos_getfattr_fd((COUNT)result);
      /* Check permissions. -- JPP
         (do not allow to open volume labels/directories) */
      if (sftp->sft_attrib & (D_DIR | D_VOLID))
      {
        dos_close((COUNT)result);
        return DE_ACCESS;
      }
      sftp->sft_size = dos_getfsize((COUNT)result);
    }
    sftp->sft_status = (COUNT)result;
    sftp->sft_flags = PriPathName[0] - 'A';
    sftp->sft_count += 1;
    DosGetFile(PriPathName, sftp->sft_name);
    dos_getftime(sftp->sft_status,
                 (date FAR *) & sftp->sft_date,
                 (time FAR *) & sftp->sft_time);
    return sft_idx | ((long)status << 16);
  }
  else
  {
/* /// Added for SHARE *** CURLY BRACES ADDED ALSO!!! ***.  - Ron Cemer */
    if (IsShareInstalled())
    {
      share_close_file(sftp->sft_shroff);
      sftp->sft_shroff = -1;
    }
/* /// End of additions for SHARE.  - Ron Cemer */
    return result;
  }    
}

long DosOpen(char FAR * fname, unsigned mode, unsigned attrib)
{
  long result;
  unsigned hndl;
  
  /* test if mode is in range                     */
  if ((mode & ~SFT_OMASK) != 0)
    return DE_INVLDACC;

  /* get a free handle  */
  if ((result = get_free_hndl()) < 0)
    return result;
  hndl = (unsigned)result;

  result = DosOpenSft(fname, mode, attrib);
  if (result < SUCCESS)
    return result;

  ((psp FAR *)MK_FP(cu_psp, 0))->ps_filetab[hndl] = (UBYTE)result;
  return hndl | (result & 0xffff0000l);
}

COUNT CloneHandle(unsigned hndl)
{
  /* now get the system file table entry                          */
  sft FAR *sftp = get_sft(hndl);

  if (sftp == (sft FAR *) -1 || (sftp->sft_mode & O_NOINHERIT))
    return DE_INVLDHNDL;
  
  /* now that we have the system file table entry, get the fnode  */
  /* index, and increment the count, so that we've effectively    */
  /* cloned the file.                                             */
  sftp->sft_count += 1;
  return SUCCESS;
}

long DosDup(unsigned Handle)
{
  long NewHandle;

  if ((NewHandle = get_free_hndl()) < 0)
    return NewHandle;

  if (DosForceDup(Handle, (unsigned)NewHandle) < 0)
    return DE_INVLDHNDL;
  else
    return NewHandle;
}

COUNT DosForceDup(unsigned OldHandle, unsigned NewHandle)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  sft FAR *Sftp;

  /* Get the SFT block that contains the SFT                      */
  if ((Sftp = get_sft(OldHandle)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* now close the new handle if it's open                        */
  if ((UBYTE) p->ps_filetab[NewHandle] != 0xff)
  {
    COUNT ret;

    if ((ret = DosClose(NewHandle)) != SUCCESS)
      return ret;
  }

  /* If everything looks ok, bump it up.                          */
  if ((Sftp->sft_flags & (SFT_FDEVICE | SFT_FSHARED))
      || (Sftp->sft_status >= 0))
  {
    p->ps_filetab[NewHandle] = p->ps_filetab[OldHandle];
    /* possible hazard: integer overflow ska*/
    Sftp->sft_count += 1;
    return SUCCESS;
  }
  else
    return DE_INVLDHNDL;
}

COUNT DosCloseSft(int sft_idx, BOOL commitonly)
{
  sft FAR *sftp = idx_to_sft(sft_idx);

  if (FP_OFF(sftp) == (size_t) - 1)
    return DE_INVLDHNDL;

  lpCurSft = sftp;
/*
   remote sub sft_count.
 */
  if (sftp->sft_flags & SFT_FSHARED)
  {
    /* printf("closing SFT %d = %p\n",sft_idx,sftp); */
    return (commitonly ? remote_commit(sftp) : remote_close(sftp));
  }

  /* now just drop the count if a device, else    */
  /* call file system handler                     */
  if (!commitonly)
    sftp->sft_count -= 1;

  if (sftp->sft_flags & SFT_FDEVICE)
    return SUCCESS;

  if (commitonly)
    return dos_commit(sftp->sft_status);
  
  if (sftp->sft_count > 0)
    return SUCCESS;

/* /// Added for SHARE *** CURLY BRACES ADDED ALSO!!! ***.  - Ron Cemer */
  if (IsShareInstalled())
  {
    if (sftp->sft_shroff >= 0)
      share_close_file(sftp->sft_shroff);
    sftp->sft_shroff = -1;
  }
/* /// End of additions for SHARE.  - Ron Cemer */
  return dos_close(sftp->sft_status);
}

COUNT DosClose(COUNT hndl)
{
  psp FAR *p = MK_FP(cu_psp, 0);
  COUNT ret;

  /* Get the SFT block that contains the SFT      */
  ret = DosCloseSft(get_sft_idx(hndl), FALSE);
  if (ret != DE_INVLDHNDL && ret != DE_ACCESS)
    p->ps_filetab[hndl] = 0xff;
  return ret;
}

BOOL DosGetFree(UBYTE drive, UWORD * spc, UWORD * navc,
                UWORD * bps, UWORD * nc)
{
  /* navc==NULL means: called from FatGetDrvData, fcbfns.c */
  struct dpb FAR *dpbp;
  struct cds FAR *cdsp;
  COUNT rg[4];

  /* next - "log" in the drive            */
  drive = (drive == 0 ? default_drive : drive - 1);

  /* first check for valid drive          */
  *spc = -1;
  cdsp = get_cds(drive);

  if (cdsp == NULL)
    return FALSE;

  if (cdsp->cdsFlags & CDSNETWDRV)
  {
    if (remote_getfree(cdsp, rg) != SUCCESS)
      return FALSE;

    /* for int21/ah=1c:
       Undoc DOS says, its not supported for
       network drives. so it's probably OK */
    /* some programs such as RHIDE want it though and
       the redirector can provide all info
       - Bart, 2002 Apr 1 */

    if (navc != NULL)
    {
      *navc = (COUNT) rg[3];
      *spc &= 0xff; /* zero out media ID byte */
    }

    *spc = (COUNT) rg[0];
    *nc = (COUNT) rg[1];
    *bps = (COUNT) rg[2];
    return TRUE;
  }

  dpbp = cdsp->cdsDpb;
  if (dpbp == NULL)
    return FALSE;

  if (navc == NULL)
  {
      /* hazard: no error checking! */
    flush_buffers(dpbp->dpb_unit);
    dpbp->dpb_flags = M_CHANGED;
  }

  if (media_check(dpbp) < 0)
    return FALSE;
  /* get the data available from dpb      */
  *spc = (dpbp->dpb_clsmask + 1);
  *bps = dpbp->dpb_secsize;

  /* now tell fs to give us free cluster  */
  /* count                                */
#ifdef WITHFAT32
  if (ISFAT32(dpbp))
  {
    ULONG cluster_size, ntotal, nfree;

    /* we shift ntotal until it is equal to or below 0xfff6 */
    cluster_size = (ULONG) dpbp->dpb_secsize << dpbp->dpb_shftcnt;
    ntotal = dpbp->dpb_xsize - 1;
    if (navc != NULL)
      nfree = dos_free(dpbp);
    while (ntotal > FAT_MAGIC16 && cluster_size < 0x8000)
    {
      cluster_size <<= 1;
      *spc <<= 1;
      ntotal >>= 1;
      nfree >>= 1;
    }
    /* get the data available from dpb      */
    *nc = ntotal > FAT_MAGIC16 ? FAT_MAGIC16 : (UCOUNT) ntotal;

    /* now tell fs to give us free cluster  */
    /* count                                */
    if (navc != NULL)
      *navc = nfree > FAT_MAGIC16 ? FAT_MAGIC16 : (UCOUNT) nfree;
    return TRUE;
  }
#endif
  /* a passed nc of 0xffff means: skip free; see FatGetDrvData
     fcbfns.c */
  if (*nc != 0xffff)
    *navc = (COUNT) dos_free(dpbp);
  *nc = dpbp->dpb_size - 1;
  if (*spc > 64)
  {
    /* fake for 64k clusters do confuse some DOS programs, but let
       others work without overflowing */
    *spc >>= 1;
    *navc = ((unsigned)*navc < FAT_MAGIC16 / 2) ? ((unsigned)*navc << 1) : FAT_MAGIC16;
    *nc = ((unsigned)*nc < FAT_MAGIC16 / 2) ? ((unsigned)*nc << 1) : FAT_MAGIC16;
  }
  return TRUE;
}

#ifdef WITHFAT32
/* network names like \\SERVER\C aren't supported yet */
#define IS_SLASH(ch) (ch == '\\' || ch == '/')
COUNT DosGetExtFree(BYTE FAR * DriveString, struct xfreespace FAR * xfsp)
{
  struct dpb FAR *dpbp;
  struct cds FAR *cdsp;
  UCOUNT rg[4];

  if (IS_SLASH(DriveString[0]) || !IS_SLASH(DriveString[2])
      || DriveString[1] != ':')
    return DE_INVLDDRV;

  cdsp = get_cds(DosUpFChar(*DriveString) - 'A');

  if (cdsp == NULL)
    return DE_INVLDDRV;

  if (cdsp->cdsFlags & CDSNETWDRV)
  {
    remote_getfree(cdsp, rg);

    xfsp->xfs_clussize = rg[0];
    xfsp->xfs_totalclusters = rg[1];
    xfsp->xfs_secsize = rg[2];
    xfsp->xfs_freeclusters = rg[3];
  }
  else
  {
    dpbp = cdsp->cdsDpb;
    if (dpbp == NULL || media_check(dpbp) < 0)
      return DE_INVLDDRV;
    xfsp->xfs_secsize = dpbp->dpb_secsize;
    xfsp->xfs_totalclusters =
        (ISFAT32(dpbp) ? dpbp->dpb_xsize : dpbp->dpb_size);
    xfsp->xfs_freeclusters = dos_free(dpbp);
    xfsp->xfs_clussize = dpbp->dpb_clsmask + 1;
  }
  xfsp->xfs_totalunits = xfsp->xfs_totalclusters;
  xfsp->xfs_freeunits = xfsp->xfs_freeclusters;
  xfsp->xfs_totalsectors = xfsp->xfs_totalclusters * xfsp->xfs_clussize;
  xfsp->xfs_freesectors = xfsp->xfs_freeclusters * xfsp->xfs_clussize;
  xfsp->xfs_datasize = sizeof(struct xfreespace);

  fmemset(xfsp->xfs_reserved, 0, 8);

  return SUCCESS;
}
#endif

COUNT DosGetCuDir(UBYTE drive, BYTE FAR * s)
{
  BYTE *cp;
  struct cds FAR *cdsp;

  /* next - "log" in the drive            */
  /* first check for valid drive          */
  cdsp = get_cds(drive == 0 ? default_drive : drive - 1);
  if (cdsp == NULL)
    return DE_INVLDDRV;

  fmemcpy(&TempCDS, cdsp, sizeof(TempCDS));
  cp = TempCDS.cdsCurrentPath;
  /* ensure termination of fstrcpy */
  cp[MAX_CDSPATH - 1] = '\0';

  if ((TempCDS.cdsFlags & CDSNETWDRV) == 0)
  {
    /* dos_cd ensures that the path exists; if not, we
       need to change to the root directory */
    int result = dos_cd(cp);
    if (result == DE_PATHNOTFND)
      cp[TempCDS.cdsBackslashOffset + 1] =
        cdsp->cdsCurrentPath[TempCDS.cdsBackslashOffset + 1] = '\0';
    else if (result < SUCCESS)
      return result;
  }

  cp += TempCDS.cdsBackslashOffset;
  if (*cp == '\0')
    s[0] = '\0';
  else
    fstrcpy(s, cp + 1);

  return SUCCESS;
}

#undef CHDIR_DEBUG
COUNT DosChangeDir(BYTE FAR * s)
{
  COUNT result;
  BYTE FAR *p;

  /* don't do wildcard CHDIR --TE */
  for (p = s; *p; p++)
    if (*p == '*' || *p == '?')
      return DE_PATHNOTFND;

  result = truename(s, PriPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
  {
    return result;
  }

  if (strlen(PriPathName) > sizeof(current_ldt->cdsCurrentPath) - 1)
    return DE_PATHNOTFND;

#if defined(CHDIR_DEBUG)
  printf("Remote Chdir: n='%Fs' p='%Fs\n", s, PriPathName);
#endif
  /* now get fs to change to new          */
  /* directory                            */
  result = (result & IS_NETWORK ? remote_chdir() : dos_cd(PriPathName));
#if defined(CHDIR_DEBUG)
  printf("status = %04x, new_path='%Fs'\n", result, cdsd->cdsCurrentPath);
#endif
  if (result != SUCCESS)
    return result;
/*
   Copy the path to the current directory
   structure.

        Some redirectors do not write back to the CDS.
        SHSUCdX needs this. jt
*/
  fstrcpy(current_ldt->cdsCurrentPath, PriPathName);
  if (PriPathName[7] == 0)
    current_ldt->cdsCurrentPath[8] = 0; /* Need two Zeros at the end */
  return SUCCESS;
}

STATIC VOID pop_dmp(dmatch FAR * dmp)
{
  dmp->dm_attr_fnd = (BYTE) SearchDir.dir_attrib;
  dmp->dm_time = SearchDir.dir_time;
  dmp->dm_date = SearchDir.dir_date;
  dmp->dm_size = (LONG) SearchDir.dir_size;
  ConvertName83ToNameSZ(dmp->dm_name, (BYTE FAR *) SearchDir.dir_name);
}

COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name)
{
  int rc;
  register dmatch FAR *dmp = dta;

  rc = truename(name, PriPathName,
                CDS_MODE_CHECK_DEV_PATH | CDS_MODE_ALLOW_WILDCARDS);
  if (rc < SUCCESS)
    return rc;

  /* /// Added code here to do matching against device names.
     DOS findfirst will match exact device names if the
     filename portion (excluding the extension) contains
     a valid device name.
     Credits: some of this code was ripped off from truename()
     in newstuff.c.
     - Ron Cemer */

  fmemset(dmp, 0, sizeof(dmatch));

  /* initially mark the dta as invalid for further findnexts */
  dmp->dm_attr_fnd = D_DEVICE;

  memset(&SearchDir, 0, sizeof(struct dirent));

  SAttr = (BYTE) attr;

#if defined(FIND_DEBUG)
  printf("Remote Find: n='%Fs\n", PriPathName);
#endif

  fmemcpy(&sda_tmp_dm, dta, 21);
  dta = &sda_tmp_dm;

  if (rc & IS_NETWORK)
    rc = remote_findfirst(current_ldt);
  else if (rc & IS_DEVICE)
  {
    const char *p;
    COUNT i;

    /* Found a matching device. Hence there cannot be wildcards. */
    SearchDir.dir_attrib = D_DEVICE;
    SearchDir.dir_time = dos_gettime();
    SearchDir.dir_date = dos_getdate();
    p = (char *)FP_OFF(get_root(PriPathName));
    memset(SearchDir.dir_name, ' ', FNAME_SIZE + FEXT_SIZE);
    for (i = 0; i < FNAME_SIZE && *p && *p != '.'; i++)
      SearchDir.dir_name[i] = *p++;
    rc = SUCCESS;
    /* /// End of additions.  - Ron Cemer ; heavily edited - Bart Oldeman */
  }
  else
    rc = dos_findfirst(attr, PriPathName);

  dta = dmp;
  fmemcpy(dta, &sda_tmp_dm, 21);
  pop_dmp(dmp);
  if (rc != SUCCESS)
    dmp->dm_attr_fnd = D_DEVICE;       /* mark invalid */
  return rc;
}

COUNT DosFindNext(void)
{
  COUNT rc;
  register dmatch FAR *dmp = dta;

  /* /// findnext will always fail on a device name device name or volume id */
  if (dmp->dm_attr_fnd == D_DEVICE || dmp->dm_attr_fnd & D_VOLID)
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
  printf("findnext: %d\n", dmp->dm_drive);
#endif
  fmemcpy(&sda_tmp_dm, dmp, 21);
  fmemset(dmp, 0, sizeof(*dmp));
  dta = &sda_tmp_dm;
  rc = (sda_tmp_dm.dm_drive & 0x80) ?
    remote_findnext(&sda_tmp_dm) : dos_findnext();

  dta = dmp;
  fmemcpy(dmp, &sda_tmp_dm, 21);
  pop_dmp(dmp);
  return rc;
}

COUNT DosGetFtime(COUNT hndl, date * dp, time * tp)
{
  sft FAR *s;
/*sfttbl FAR *sp;*/

  /* Get the SFT block that contains the SFT      */
  if (FP_OFF(s = get_sft(hndl)) == (size_t) - 1)
    return DE_INVLDHNDL;

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

COUNT DosSetFtimeSft(int sft_idx, date dp, time tp)
{
  /* Get the SFT block that contains the SFT      */
  sft FAR *s = idx_to_sft(sft_idx);

  if (FP_OFF(s) == (size_t) - 1)
    return DE_INVLDHNDL;

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
  COUNT result;

  result = truename(name, PriPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
    return result;
  
/* /// Added check for "d:\", which returns 0x10 (subdirectory) under DOS.
       - Ron Cemer */
           /* Theoretically: If the redirectory's qualify function
               doesn't return nonsense this check can be reduced to
               PriPathname[3] == 0, because local path names always
               have the three-byte string ?:\ and UNC path shouldn't
               validy consist of just two slashes.
               -- 2001/09/03 ska*/

  if (PriPathName[3] == '\0')
    return 0x10;

  if (result & IS_NETWORK)
    return remote_getfattr();

  if (result & IS_DEVICE)
    return DE_FILENOTFND;

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

/* This function is almost identical to DosGetFattr().
   Maybe it is nice to join both functions.
       -- 2001/09/03 ska*/
COUNT DosSetFattr(BYTE FAR * name, UWORD attrp)
{
  COUNT result;

  result = truename(name, PriPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
    return result;

  if (result & IS_NETWORK)
    return remote_setfattr(attrp);

  if (result & IS_DEVICE)
    return DE_FILENOTFND;

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

UBYTE DosSelectDrv(UBYTE drv)
{
  current_ldt = get_cds(drv);

  if (current_ldt != NULL)
    default_drive = drv;

  return lastdrive;
}

COUNT DosDelete(BYTE FAR * path, int attrib)
{
  COUNT result;

  result = truename(path, PriPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
    return result;

  if (result & IS_NETWORK)
    return remote_delete();

  if (result & IS_DEVICE)
    return DE_FILENOTFND;

  return dos_delete(PriPathName, attrib);
}

COUNT DosRenameTrue(BYTE * path1, BYTE * path2, int attrib)
{
  if (path1[0] != path2[0])
  {
    return DE_DEVICE; /* not same device */
  }
  if (current_ldt->cdsFlags & CDSNETWDRV)
    return remote_rename();

  return dos_rename(path1, path2, attrib);
}

COUNT DosRename(BYTE FAR * path1, BYTE FAR * path2)
{
  COUNT result;

  result = truename(path2, SecPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
    return result;

  if (!(result & IS_NETWORK) && (result & IS_DEVICE))
    return DE_FILENOTFND;

  result = truename(path1, PriPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
    return result;

  if (!(result & IS_NETWORK) && (result & IS_DEVICE))
    return DE_FILENOTFND;

  return DosRenameTrue(PriPathName, SecPathName, D_ALL);
}

COUNT DosMkdir(const char FAR * dir)
{
  COUNT result;

  result = truename(dir, PriPathName, CDS_MODE_CHECK_DEV_PATH);
  if (result < SUCCESS)
    return result;

  if (result & IS_NETWORK)
    return remote_mkdir();

  if (result & IS_DEVICE)
    return DE_ACCESS;

  return dos_mkdir(PriPathName);
}

/* This function is almost identical to DosMkdir().
   Maybe it would be nice to merge both functions.
       -- 2001/09/03 ska*/
COUNT DosRmdir(const char FAR * dir)
{
  COUNT result;

  result = truename(dir, PriPathName, CDS_MODE_CHECK_DEV_PATH);

  if (result < SUCCESS)
    return result;

  if (result & IS_NETWORK)
    return remote_rmdir();

  if (result & IS_DEVICE)
    return DE_ACCESS;

  return dos_rmdir(PriPathName);
}

/* /// Added for SHARE.  - Ron Cemer */

COUNT DosLockUnlock(COUNT hndl, LONG pos, LONG len, COUNT unlock)
{
  sft FAR *s;

  /* Get the SFT block that contains the SFT      */
  if (FP_OFF(s = get_sft(hndl)) == (size_t) - 1)
    return DE_INVLDHNDL;

  if (s->sft_flags & SFT_FSHARED)
    return remote_lock_unlock(s, pos, len, unlock);

  /* Invalid function unless SHARE is installed or remote. */
  if (!IsShareInstalled())
    return DE_INVLDFUNC;

  /* Lock violation if this SFT entry does not support locking. */
  if (s->sft_shroff < 0)
    return DE_LOCK;

  /* Let SHARE do the work. */
  return share_lock_unlock(cu_psp, s->sft_shroff, pos, len, unlock);
}

/* /// End of additions for SHARE.  - Ron Cemer */

/*
 * This seems to work well.
 */

/* check for a device  */
struct dhdr FAR *IsDevice(const char FAR * fname)
{
  struct dhdr FAR *dhp;
  const char FAR *froot = get_root(fname);
  int i;

/* /// BUG!!! This is absolutely wrong.  A filename of "NUL.LST" must be
       treated EXACTLY the same as a filename of "NUL".  The existence or
       content of the extension is irrelevent in determining whether a
       filename refers to a device.
       - Ron Cemer
  // if we have an extension, can't be a device <--- WRONG.
  if (*froot != '.')
  {
*/

  for (dhp = (struct dhdr FAR *)&nul_dev; dhp != (struct dhdr FAR *)-1;
       dhp = dhp->dh_next)
  {

    /*  BUGFIX: MSCD000<00> should be handled like MSCD000<20> TE */

    for (i = 0; i < FNAME_SIZE; i++)
    {
      char c1 = froot[i];
      if (c1 == '.' || c1 == '\0')
      {
        /* check if remainder of device name consists of spaces or nulls */
        for (; i < FNAME_SIZE; i++)
        {
          char c2 = dhp->dh_name[i];
          if (c2 != ' ' && c2 != '\0')
            break;
        }
        break;
      }
      if (DosUpFChar(c1) != DosUpFChar(dhp->dh_name[i]))
        break;
    }
    if (i == FNAME_SIZE)
      return dhp;
  }

  return NULL;
}

/* /// Added for SHARE.  - Ron Cemer */

BOOL IsShareInstalled(void)
{
  if (!share_installed)
  {
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
STATIC int share_open_check(char * filename,            /* pointer to fully qualified filename */
                            unsigned short pspseg,      /* psp segment address of owner process */
                            int openmode,       /* 0=read-only, 1=write-only, 2=read-write */
                            int sharemode)
{                               /* SHARE_COMPAT, etc... */
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
STATIC void share_close_file(int fileno)
{                               /* file_table entry number */
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
STATIC int share_access_check(unsigned short pspseg,    /* psp segment address of owner process */
                              int fileno,       /* file_table entry number */
                              unsigned long ofs,        /* offset into file */
                              unsigned long len,        /* length (in bytes) of region to access */
                              int allowcriter)
{                               /* allow a critical error to be generated */
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
STATIC int share_lock_unlock(unsigned short pspseg,     /* psp segment address of owner process */
                             int fileno,        /* file_table entry number */
                             unsigned long ofs, /* offset into file */
                             unsigned long len, /* length (in bytes) of region to lock or unlock */
                             int unlock)
{                               /* non-zero to unlock; zero to lock */
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
STATIC int remote_lock_unlock(sft FAR *sftp,     /* SFT for file */
                             unsigned long ofs, /* offset into file */
                             unsigned long len, /* length (in bytes) of region to lock or unlock */
                             int unlock)
{                               /* non-zero to unlock; zero to lock */
  iregs regs;
  unsigned long param_block[2];
  param_block[0] = ofs;
  param_block[1] = len;

  regs.a.x = 0x110a;
  regs.b.b.l = (unlock ? 0x01 : 0x00);
  regs.c.x = 1;
  regs.ds = FP_SEG(param_block);
  regs.d.x = FP_OFF(param_block);
  regs.es = FP_SEG(sftp);
  regs.di = FP_OFF(sftp);
  intr(0x2f, &regs);
  return ((regs.flags & 1) ? -(int)regs.a.b.l : 0);
}

COUNT DosTruename(const char FAR *src, char FAR *dest)
{
  /* RBIL: The buffer has be unchanged, if the call fails.
     Therefore, the name is created in an internal buffer
     and copied into the user buffer only on success.
  */  
  COUNT rc = truename(src, PriPathName, CDS_MODE_ALLOW_WILDCARDS);
  if (rc >= SUCCESS)
    fstrcpy(dest, PriPathName);
  return rc;
}

