/****************************************************************/
/*                                                              */
/*                          fatfs.c                             */
/*                           DOS-C                              */
/*                                                              */
/*                 FAT File System I/O Functions                */
/*                                                              */
/*                    Copyright (c) 1995,1998                   */
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
#include "globals.h"

#ifdef VERSION_STRINGS
BYTE *RcsId = "$Id$";
#endif

/*
 * TE 12 jun 2001 bugs corrected
 *      handles disk full (in a incompatible way :-( )
 *      allows use of last cluster
 *      prevents mkdir, if disk is full (was creating crosslinked dirs)
 *   bugs detected, but NOT corrected
 *      on disk full, MSDOS will NOT write any byte, simply return SUCCESS, 0 bytes
 *      FreeDOS will write all possible bytes, then close file(BUG)
 *
 * the dos_mkdir/extenddir (with getblock() instead of getblockOver) was a real
 * performance killer on large drives. (~0.5 sec /dos_mkdir) TE 
 *
 * $Log$
 * Revision 1.21  2001/07/24 16:56:29  bartoldeman
 * fixes for FCBs, DJGPP ls, DBLBYTE, dyninit allocation (2024e).
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
 * Revision 1.15  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.14  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.13  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.12  2001/03/24 22:13:05  bartoldeman
 * See history.txt: dsk.c changes, warning removal and int21 entry handling.
 *
 * Revision 1.11  2001/03/22 04:26:14  bartoldeman
 * dos_gettime() fix by Tom Ehlert.
 *
 * Revision 1.10  2001/03/21 02:56:25  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.9  2001/03/08 21:00:00  bartoldeman
 * Disabled select_unit() since it's not used
 *
 * Revision 1.8  2000/10/29 23:51:56  jimtabor
 * Adding Share Support by Ron Cemer
 *
 * Revision 1.7  2000/09/05 00:56:50  jimtabor
 * *** empty log message ***
 *
 *
 * ///  2000/08/12 22:49:00  Ron Cemer
 * Fixed writeblock() to only use getbuf() if writing a
 * complete sector; otherwise use getbloc() and do a
 * read-modify-write to prevent writing garbage back
 * over pre-existing data in the file.
 * This was a major BUG.
 *
 * Revision 1.6  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.5  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.4  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.3  2000/05/11 04:26:26  jimtabor
 * Added code for DOS FN 69 & 6C
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
 *
 * Revision 1.23  2000/04/29 05:13:16  jtabor
 *  Added new functions and clean up code
 *
 * Revision 1.19  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.18  2000/03/17 04:13:12  kernel
 * Added Change for media_check
 *
 * Revision 1.17  2000/03/17 04:01:20  kernel
 * Added Change for media_check
 *
 * Revision 1.16  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.15  1999/09/23 04:40:46  jprice
 * *** empty log message ***
 *
 * Revision 1.12  1999/09/14 01:01:54  jprice
 * Fixed bug where you could write over directories.
 *
 * Revision 1.11  1999/08/25 03:18:08  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.10  1999/08/10 18:03:42  jprice
 * ror4 2011-03 patch
 *
 * Revision 1.9  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.8  1999/05/03 05:00:24  jprice
 * Fixed bug in map_cluster function
 *
 * Revision 1.7  1999/04/16 00:53:33  jprice
 * Optimized FAT handling
 *
 * Revision 1.6  1999/04/12 23:41:54  jprice
 * Using getbuf to write data instead of getblock
 * using getblock made it read the block before it wrote it
 *
 * Revision 1.5  1999/04/12 03:21:17  jprice
 * more ror4 patches.  Changes for multi-block IO
 *
 * Revision 1.4  1999/04/11 04:33:38  jprice
 * ror4 patches
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:42:07  jprice
 * New version without IPL.SYS
 *
 * Revision 1.8  1999/03/23 23:37:39  jprice
 * Fixed mkdir DOS function so it will create a directory with same name as the volument label
 *
 * Revision 1.7  1999/03/02 07:00:51  jprice
 * Fixed bugs with dos set attribute function.  Now returns correct
 * error code, and errors if user tries to set bits 6 & 7.
 *
 * Revision 1.6  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.5  1999/02/04 03:18:37  jprice
 * Formating.  Added comments.
 *
 * Revision 1.4  1999/02/01 01:43:28  jprice
 * Fixed findfirst function to find volume label with Windows long filenames
 *
 * Revision 1.3  1999/01/30 08:25:34  jprice
 * Clean up; Fixed bug with set attribute function.  If you tried to
 * change the attributes of a directory, it would erase it.
 *
 * Revision 1.2  1999/01/22 04:15:28  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:00  jprice
 * Imported sources
 *
 *
 *    Rev 1.14   06 Dec 1998  8:44:26   patv
 * Bug fixes.
 *
 *    Rev 1.13   09 Feb 1998  5:43:30   patv
 * Eliminated FAT12 EOF and error useage.
 *
 *    Rev 1.12   03 Feb 1998 11:28:04   patv
 * Fixed lseek bug.
 *
 *    Rev 1.11   22 Jan 1998  5:38:08   patv
 * Corrected remaining file name and extension copies that did not
 * account for far file nodes due to allocated FILES= spec.
 *
 *    Rev 1.10   22 Jan 1998  4:09:00   patv
 * Fixed pointer problems affecting SDA
 *
 *    Rev 1.9   04 Jan 1998 23:14:40   patv
 * Changed Log for strip utility
 *
 *    Rev 1.8   04 Jan 1998 17:24:14   patv
 * Corrected subdirectory bug
 *
 *    Rev 1.7   03 Jan 1998  8:36:04   patv
 * Converted data area to SDA format
 *
 *    Rev 1.6   22 Jan 1997 13:00:30   patv
 * pre-0.92 bug fixes
 *
 *    Rev 1.5   16 Jan 1997 12:46:24   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.4   29 May 1996 21:15:16   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:20:10   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:48:40   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:24   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:04:46   patv
 * Initial revision.
 */

/*                                                                      */
/*      function prototypes                                             */
/*                                                                      */
f_node_ptr xlt_fd(COUNT);
COUNT xlt_fnp(f_node_ptr);
f_node_ptr split_path(BYTE *, BYTE *, BYTE *);
BOOL find_fname(f_node_ptr, BYTE *, BYTE *);
    /* /// Added - Ron Cemer */
STATIC void merge_file_changes(f_node_ptr fnp, int collect);
    /* /// Added - Ron Cemer */
STATIC int is_same_file(f_node_ptr fnp1, f_node_ptr fnp2);
    /* /// Added - Ron Cemer */
STATIC void copy_file_changes(f_node_ptr src, f_node_ptr dst);
date dos_getdate(VOID);
time dos_gettime(VOID);
BOOL find_free(f_node_ptr);
UWORD find_fat_free(f_node_ptr);
VOID wipe_out(f_node_ptr);
BOOL last_link(f_node_ptr);
BOOL extend(f_node_ptr);
COUNT extend_dir(f_node_ptr);
BOOL first_fat(f_node_ptr);
COUNT map_cluster(f_node_ptr, COUNT);
STATIC VOID shrink_file(f_node_ptr fnp);

/************************************************************************/
/*                                                                      */
/*      Internal file handlers - open, create, read, write, close, etc. */
/*                                                                      */
/************************************************************************/

/* Open a file given the path. Flags is 0 for read, 1 for write and 2   */
/* for update.                                                          */
/* Returns an integer file desriptor or a negative error code           */

COUNT dos_open(BYTE * path, COUNT flag)
{
  REG f_node_ptr fnp;

  /* First test the flag to see if the user has passed a valid    */
  /* file mode...                                                 */
  if (flag < 0 || flag > 2)
    return DE_INVLDACC;

  /* first split the passed dir into comopnents (i.e. - path to   */
  /* new directory and name of new directory.                     */
  if ((fnp = split_path(path, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  /* Look for the file. If we can't find it, just return a not    */
  /* found error.                                                 */
  if (!find_fname(fnp, szFileName, szFileExt))
  {
    dir_close(fnp);
    return DE_FILENOTFND;
  }

  /* Set the fnode to the desired mode                            */
  fnp->f_mode = flag;

  /* Initialize the rest of the fnode.                            */
  fnp->f_offset = 0l;
  fnp->f_highwater = fnp->f_dir.dir_size;

  fnp->f_back = LONG_LAST_CLUSTER;
/* /// Moved to below.  - Ron Cemer
  fnp->f_cluster = fnp->f_dir.dir_start;
  fnp->f_cluster_offset = 0l; */   /*JPP */

  fnp->f_flags.f_dmod = FALSE;
  fnp->f_flags.f_dnew = FALSE;
  fnp->f_flags.f_ddir = FALSE;

  merge_file_changes(fnp, TRUE);    /* /// Added - Ron Cemer */

    /* /// Moved from above.  - Ron Cemer */
  fnp->f_cluster = fnp->f_dir.dir_start;
  fnp->f_cluster_offset = 0l;   /*JPP */

  return xlt_fnp(fnp);
}

BOOL fcmp(BYTE * s1, BYTE * s2, COUNT n)
{
  while (n--)
    if (*s1++ != *s2++)
      return FALSE;
  return TRUE;
}

BOOL fcmp_wild(BYTE FAR * s1, BYTE FAR * s2, COUNT n)
{
  while (n--)
  {
    if (*s1 == '?')
    {
      ++s1, ++s2;
      continue;
    }
    if (*s1++ != *s2++)
      return FALSE;
  }
  return TRUE;
}

COUNT dos_close(COUNT fd)
{
  f_node_ptr fnp;

  /* Translate the fd into a useful pointer                       */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
    return DE_INVLDHNDL;

  if (fnp->f_mode != RDONLY)
  {
    /*TE experimental */
    if (fnp->f_flags.f_dmod)
        {
        fnp->f_dir.dir_attrib |= D_ARCHIVE;
        fnp->f_dir.dir_time = dos_gettime();
        fnp->f_dir.dir_date = dos_getdate();

        shrink_file(fnp);               /* reduce allocated filesize in FAT */
        }
    
    fnp->f_dir.dir_size = fnp->f_highwater;
    fnp->f_flags.f_dmod = TRUE;
    merge_file_changes(fnp, FALSE);    /* /// Added - Ron Cemer */
  }
  fnp->f_flags.f_ddir = TRUE;

  dir_close(fnp);
  return SUCCESS;
}

/*                                                                      */
/* split a path into it's component directory and file name             */
/*                                                                      */
f_node_ptr
  split_path(BYTE * path, BYTE * fname, BYTE * fext)
{
  REG f_node_ptr fnp;
  COUNT nDrive;
  struct cds FAR *cdsp;

  /* Start off by parsing out the components.                     */
  if (ParseDosName(path, &nDrive, &szDirName[2], fname, fext, FALSE)
      != SUCCESS)
    return (f_node_ptr)0;
  if (nDrive < 0)
    nDrive = default_drive;

  szDirName[0] = 'A' + nDrive;
  szDirName[1] = ':';

  /* Add trailing spaces to the file name and extension           */
  SpacePad(fname, FNAME_SIZE);
  SpacePad(fext, FEXT_SIZE);

  if (nDrive >= lastdrive) {
    return (f_node_ptr)0;
  }
  cdsp = &CDSp->cds_table[nDrive];

  /* If the path is null, we to default to the current            */
  /* directory...                                                 */
  if (!szDirName[2])
  {
    fsncopy(cdsp->cdsCurrentPath, (BYTE FAR *) szDirName, PARSE_MAX);
  }

/*  11/29/99 jt
   * Networking and Cdroms. You can put in here a return.
   * Maybe a return of 0xDEADBEEF or something for Split or Dir_open.
   * Just to let upper level Fdos know its a sft, CDS function.
   * Right now for Networking there is no support for Rename, MkDir
   * RmDir & Delete.

   <insert code here or in dir_open. I would but it in Dir_open.
   Do the redirection in Network.c>

 */
#ifdef DEBUG
  if (cdsp->cdsFlags & CDSNETWDRV) {
    printf("split path called for redirected file: `%s.%s'\n",
	    fname, fext);
    return (f_node_ptr)0;
  }
#endif

  /* Translate the path into a useful pointer                     */
  fnp = dir_open(szDirName);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit...     */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
  {
    dir_close(fnp);
    return (f_node_ptr)0;
  }

  /* Convert the name into an absolute name for comparison...     */
  DosUpFString((BYTE FAR *) szDirName);
  DosUpFMem((BYTE FAR *) fname, FNAME_SIZE);
  DosUpFMem((BYTE FAR *) fext, FEXT_SIZE);

  return fnp;
}

STATIC BOOL find_fname(f_node_ptr fnp, BYTE * fname, BYTE * fext)
{
  BOOL found = FALSE;

  while (dir_read(fnp) == DIRENT_SIZE)
  {
    if (fnp->f_dir.dir_name[0] != '\0')
    {
      if (fnp->f_dir.dir_name[0] == DELETED)
        continue;

      if (fcmp(fname, (BYTE *)fnp->f_dir.dir_name, FNAME_SIZE)
          && fcmp(fext, (BYTE *)fnp->f_dir.dir_ext, FEXT_SIZE)
          && ((fnp->f_dir.dir_attrib & D_VOLID) == 0))
      {
        found = TRUE;
        break;
      }
    }
  }
  return found;
}

    /* /// Added - Ron Cemer */
    /* If more than one f_node has a file open, and a write
       occurs, this function must be called to propagate the
       results of that write to the other f_nodes which have
       that file open.  Note that this function only has an
       effect if SHARE is installed.  This is for compatibility
       reasons, since DOS without SHARE does not share changes
       between two or more open instances of the same file
       unless these instances were generated by dup() or dup2(). */
STATIC void merge_file_changes(f_node_ptr fnp, int collect) {
    f_node_ptr fnp2;
    int i;

    if (!IsShareInstalled()) return;
    for (i = 0; i < f_nodes_cnt; i++) {
        fnp2 = (f_node_ptr)&f_nodes[i];
        if (   (fnp != (f_node_ptr)0)
            && (fnp != fnp2)
            && (fnp->f_count > 0)
            && (is_same_file(fnp, fnp2))   ) {
            if (collect) {
                    /* We're collecting file changes from any other
                       f_node which refers to this file. */
                if (fnp2->f_mode != RDONLY) {
                    fnp2->f_dir.dir_size = fnp2->f_highwater;
                    copy_file_changes(fnp2, fnp);
                    break;
                }
            } else {
                    /* We just made changes to this file, so we are
                       distributing these changes to the other f_nodes
                       which refer to this file. */
                if (fnp->f_mode != RDONLY)
                    fnp->f_dir.dir_size = fnp->f_highwater;
                copy_file_changes(fnp, fnp2);
            }
        }
    }
}

    /* /// Added - Ron Cemer */
STATIC int is_same_file(f_node_ptr fnp1, f_node_ptr fnp2) {
    return
           (fnp1->f_dpb->dpb_unit == fnp2->f_dpb->dpb_unit)
        && (fnp1->f_dpb->dpb_subunit == fnp2->f_dpb->dpb_subunit)
        && (fcmp
                ((BYTE *)fnp1->f_dir.dir_name,
                 (BYTE *)fnp2->f_dir.dir_name, FNAME_SIZE))
        && (fcmp
                ((BYTE *)fnp1->f_dir.dir_ext,
                 (BYTE *)fnp2->f_dir.dir_ext, FEXT_SIZE))
        && ((fnp1->f_dir.dir_attrib & D_VOLID) == 0)
        && ((fnp2->f_dir.dir_attrib & D_VOLID) == 0)
        && (fnp1->f_flags.f_dremote == fnp2->f_flags.f_dremote)
        && (fnp1->f_diroff == fnp2->f_diroff)
        && (fnp1->f_dirstart == fnp2->f_dirstart)
        && (fnp1->f_dpb == fnp2->f_dpb);
}

    /* /// Added - Ron Cemer */
STATIC void copy_file_changes(f_node_ptr src, f_node_ptr dst) {
    dst->f_highwater = src->f_highwater;
    dst->f_dir.dir_start = src->f_dir.dir_start;
    dst->f_dir.dir_size = src->f_dir.dir_size;
    dst->f_dir.dir_date = src->f_dir.dir_date;
    dst->f_dir.dir_time = src->f_dir.dir_time;
}

COUNT dos_creat(BYTE * path, COUNT attrib)
{
  REG f_node_ptr fnp;

                                   /* NEVER EVER allow directories to be created */
  if (attrib & ~(D_RDONLY|D_HIDDEN|D_SYSTEM|D_ARCHIVE))
    {
        return DE_ACCESS;
    }

  /* first split the passed dir into comopnents (i.e. -   */
  /* path to new directory and name of new directory      */
  if ((fnp = split_path(path, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  /* Check that we don't have a duplicate name, so if we  */
  /* find one, truncate it.                               */
  if (find_fname(fnp, szFileName, szFileExt))
  {
    /* The only permissable attribute is archive,   */
    /* check for any other bit set. If it is, give  */
    /* an access error.                             */
    if ((fnp->f_dir.dir_attrib & (D_RDONLY | D_DIR | D_VOLID))
        || (fnp->f_dir.dir_attrib & ~D_ARCHIVE & ~attrib))
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    /* Release the existing files FAT and set the   */
    /* length to zero, effectively truncating the   */
    /* file to zero.                                */
    wipe_out(fnp);
  }
  else
  {
    BOOL is_free;

    /* Reset the directory by a close followed by   */
    /* an open                                      */
    fnp->f_flags.f_dmod = FALSE;
    dir_close(fnp);
    fnp = split_path(path, szFileName, szFileExt);

    /* Get a free f_node pointer so that we can use */
    /* it in building the new file.                 */
    /* Note that if we're in the root and we don't  */
    /* find an empty slot, we need to abort.        */
    if (((is_free = find_free(fnp)) == 0) && (fnp->f_flags.f_droot))
    {
      fnp->f_flags.f_dmod = FALSE;
      dir_close(fnp);
      return DE_TOOMANY;
    }

    /* Otherwise just expand the directory          */
    else if (!is_free && !(fnp->f_flags.f_droot))
    {
      COUNT ret;

      if ((ret = extend_dir(fnp)) != SUCCESS)
        return ret;
    }

    /* put the fnode's name into the directory.             */
    bcopy(szFileName, fnp->f_dir.dir_name, FNAME_SIZE);
    bcopy(szFileExt, fnp->f_dir.dir_ext, FEXT_SIZE);
  }
  /* Set the fnode to the desired mode                    */
  /* Updating the directory entry first.                  */
  fnp->f_mode = RDWR;

  fnp->f_dir.dir_size = 0l;
  fnp->f_dir.dir_start = FREE;
  fnp->f_dir.dir_attrib = attrib | D_ARCHIVE;
  fnp->f_dir.dir_time = dos_gettime();
  fnp->f_dir.dir_date = dos_getdate();

  fnp->f_flags.f_dmod = TRUE;
  fnp->f_flags.f_dnew = FALSE;
  fnp->f_flags.f_ddir = TRUE;
  if (dir_write(fnp) != DIRENT_SIZE)
  {
    release_f_node(fnp);
    return DE_ACCESS;
  }

  /* Now change to file                                   */
  fnp->f_offset = 0l;
  fnp->f_highwater = 0l;

  fnp->f_back = LONG_LAST_CLUSTER;
  fnp->f_cluster = fnp->f_dir.dir_start = FREE;
  fnp->f_cluster_offset = 0l;   /*JPP */
  fnp->f_flags.f_dmod = TRUE;
  fnp->f_flags.f_dnew = FALSE;
  fnp->f_flags.f_ddir = FALSE;

  merge_file_changes(fnp, FALSE);   /* /// Added - Ron Cemer */

  return xlt_fnp(fnp);
}

COUNT dos_delete(BYTE * path)
{
  REG f_node_ptr fnp;

  /* first split the passed dir into components (i.e. -   */
  /* path to new directory and name of new directory      */
  if ((fnp = split_path(path, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  /* Check that we don't have a duplicate name, so if we  */
  /* find one, it's an error.                             */
  if (find_fname(fnp, szFileName, szFileExt))
  {
    /* The only permissable attribute is archive,   */
    /* TE +hidden + system                          */
    /* check for any other bit set. If it is, give  */
    /* an access error.                             */
    if (fnp->f_dir.dir_attrib & ~(D_ARCHIVE | D_HIDDEN | D_SYSTEM))
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    /* Ok, so we can delete. Start out by           */
    /* clobbering all FAT entries for this file     */
    /* (or, in English, truncate the FAT).          */
    wipe_out(fnp);
    fnp->f_dir.dir_size = 0l;
    *(fnp->f_dir.dir_name) = DELETED;

    /* The directory has been modified, so set the  */
    /* bit before closing it, allowing it to be     */
    /* updated                                      */
    fnp->f_flags.f_dmod = TRUE;
    dir_close(fnp);

    /* SUCCESSful completion, return it             */
    return SUCCESS;
  }
  else
  {
    /* No such file, return the error               */
    dir_close(fnp);
    return DE_FILENOTFND;
  }
}

COUNT dos_rmdir(BYTE * path)
{
  REG f_node_ptr fnp;
  REG f_node_ptr fnp1;
  BOOL found;

  /* first split the passed dir into comopnents (i.e. -   */
  /* path to new directory and name of new directory      */
  if ((fnp = split_path(path, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }

  /* Check that we're not trying to remove the root!      */
  if ((path[0] == '\\') && (path[1] == NULL))
  {
    dir_close(fnp);
    return DE_ACCESS;
  }

  /* Check that we don't have a duplicate name, so if we  */
  /* find one, it's an error.                             */
  if (find_fname(fnp, szFileName, szFileExt))
  {
    /* The only permissable attribute is directory, */
    /* check for any other bit set. If it is, give  */
    /* an access error.                             */
    /* if (fnp->f_dir.dir_attrib & ~D_DIR)          */
    
    /* directories may have attributes, too. at least my WinNT disk
       has many 'archive' directories
       we still don't allow SYSTEM or RDONLY directories to be deleted TE*/
    
    if (fnp->f_dir.dir_attrib & ~(D_DIR |D_HIDDEN|D_ARCHIVE))
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    /* Check that the directory is empty. Only the  */
    /* "." and ".." are permissable.                */
    fnp->f_flags.f_dmod = FALSE;
    fnp1 = dir_open(path);
    dir_read(fnp1);
    if (fnp1->f_dir.dir_name[0] != '.')
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    dir_read(fnp1);
    if (fnp1->f_dir.dir_name[0] != '.')
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    /* Now search through the directory and make certain    */
    /* that there are no entries.                           */
    found = FALSE;
    while (dir_read(fnp1) == DIRENT_SIZE)
    {
      if (fnp1->f_dir.dir_name[0] == '\0')
        break;
      if (fnp1->f_dir.dir_name[0] == DELETED)
        continue;
      else
      {
        found = TRUE;
        break;
      }
    }

    dir_close(fnp1);
    /* If anything was found, exit with an error.   */
    if (found)
    {
      dir_close(fnp);
      return DE_ACCESS;
    }

    /* Ok, so we can delete. Start out by           */
    /* clobbering all FAT entries for this file     */
    /* (or, in English, truncate the FAT).          */
    wipe_out(fnp);
    fnp->f_dir.dir_size = 0l;
    *(fnp->f_dir.dir_name) = DELETED;

    /* The directory has been modified, so set the  */
    /* bit before closing it, allowing it to be     */
    /* updated                                      */
    fnp->f_flags.f_dmod = TRUE;
    dir_close(fnp);

    /* SUCCESSful completion, return it             */
    return SUCCESS;
  }
  else
  {
    /* No such file, return the error               */
    dir_close(fnp);
    return DE_FILENOTFND;
  }
}

COUNT dos_rename(BYTE * path1, BYTE * path2)
{
  REG f_node_ptr fnp1;
  REG f_node_ptr fnp2;
  BOOL is_free;

  /* first split the passed target into compnents (i.e. - path to */
  /* new file name and name of new file name                      */
  if ((fnp2 = split_path(path2, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp2);
    return DE_PATHNOTFND;
  }

  /* Check that we don't have a duplicate name, so if we find     */
  /* one, it's an error.                                          */
  if (find_fname(fnp2, szFileName, szFileExt))
  {
    dir_close(fnp2);
    return DE_ACCESS;
  }

  /* next split the passed source into compnents (i.e. - path to  */
  /* old file name and name of old file name                      */
  if ((fnp1 = split_path(path1, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp1);
    dir_close(fnp2);
    return DE_PATHNOTFND;
  }

  if (!find_fname(fnp1, szFileName, szFileExt))
  {
    /* No such file, return the error                       */
    dir_close(fnp1);
    dir_close(fnp2);
    return DE_FILENOTFND;
  }

  /* Reset the directory by a close followed by an open           */
  fnp2->f_flags.f_dmod = FALSE;
  dir_close(fnp2);
  fnp2 = split_path(path2, szFileName, szFileExt);

  /* Now find a free slot to put the file into.                   */
  /* If it's the root and we don't have room, return an error.    */
  if (((is_free = find_free(fnp2)) == 0) && (fnp2->f_flags.f_droot))
  {
    fnp2->f_flags.f_dmod = FALSE;
    dir_close(fnp1);
    dir_close(fnp2);
    return DE_TOOMANY;
  }

  /* Otherwise just expand the directory                          */
  else if (!is_free && !(fnp2->f_flags.f_droot))
  {
    COUNT ret;

    if ((ret = extend_dir(fnp2)) != SUCCESS)
    {
      dir_close(fnp1);
      return ret;
    }
  }

  /* put the fnode's name into the directory.                     */
  bcopy(szFileName, (BYTE *)fnp2->f_dir.dir_name, FNAME_SIZE);
  bcopy(szFileExt, (BYTE *)fnp2->f_dir.dir_ext, FEXT_SIZE);

  /* Set the fnode to the desired mode                            */
  fnp2->f_dir.dir_size = fnp1->f_dir.dir_size;
  fnp2->f_dir.dir_start = fnp1->f_dir.dir_start;
  fnp2->f_dir.dir_attrib = fnp1->f_dir.dir_attrib;
  fnp2->f_dir.dir_time = fnp1->f_dir.dir_time;
  fnp2->f_dir.dir_date = fnp1->f_dir.dir_date;

  /* The directory has been modified, so set the bit before       */
  /* closing it, allowing it to be updated.                       */
  fnp1->f_flags.f_dmod = fnp2->f_flags.f_dmod = TRUE;
  fnp1->f_flags.f_dnew = fnp2->f_flags.f_dnew = FALSE;
  fnp1->f_flags.f_ddir = fnp2->f_flags.f_ddir = TRUE;

  fnp2->f_highwater = fnp2->f_offset = fnp1->f_dir.dir_size;

  /* Ok, so we can delete this one. Save the file info.           */
  fnp1->f_dir.dir_size = 0l;
  *(fnp1->f_dir.dir_name) = DELETED;

  dir_close(fnp1);
  dir_close(fnp2);

  /* SUCCESSful completion, return it                             */
  return SUCCESS;
}

/*                                                              */
/* wipe out all FAT entries for create, delete, etc.            */
/*                                                              */
STATIC VOID wipe_out(f_node_ptr fnp)
{
  REG UWORD st,
    next;
  struct dpb FAR *dpbp = fnp->f_dpb;

  /* if already free or not valid file, just exit         */
  if ((fnp == NULL) || (fnp->f_dir.dir_start == FREE))
    return;

  /* if there are no FAT entries, just exit               */
  if (fnp->f_dir.dir_start == FREE)
    return;

  /* Loop from start until either a FREE entry is         */
  /* encountered (due to a fractured file system) of the  */
  /* last cluster is encountered.                         */
  for (st = fnp->f_dir.dir_start;
       st != LONG_LAST_CLUSTER;)
  {
    /* get the next cluster pointed to              */
    next = next_cluster(dpbp, st);

    /* just exit if a damaged file system exists    */
    if (next == FREE)
      return;

    /* zap the FAT pointed to                       */
    link_fat(dpbp, st, FREE);

    /* and the start of free space pointer          */
    if ((dpbp->dpb_cluster == UNKNCLUSTER)
        || (dpbp->dpb_cluster > st))
      dpbp->dpb_cluster = st;

    /* and just follow the linked list              */
    st = next;
  }
}

STATIC BOOL find_free(f_node_ptr fnp)
{
  while (dir_read(fnp) == DIRENT_SIZE)
  {
    if (fnp->f_dir.dir_name[0] == '\0'
        || fnp->f_dir.dir_name[0] == DELETED)
    {
      return TRUE;
    }
  }
  return !fnp->f_flags.f_dfull;
}

/*                                                              */
/* dos_getdate for the file date                                */
/*                                                              */
date dos_getdate()
{
#ifndef NOTIME
  BYTE WeekDay,
    Month,
    MonthDay;
  COUNT Year;
  date Date;

  /* First - get the system date set by either the user   */
  /* on start-up or the CMOS clock                        */
  DosGetDate((BYTE FAR *) & WeekDay,
             (BYTE FAR *) & Month,
             (BYTE FAR *) & MonthDay,
             (COUNT FAR *) & Year);
  Date = DT_ENCODE(Month, MonthDay, Year - EPOCH_YEAR);
  return Date;

#else

  return 0;

#endif
}

/*                                                              */
/* dos_gettime for the file time                                */
/*                                                              */
time dos_gettime()
{
#ifndef NOTIME
  BYTE Hour,
    Minute,
    Second,
    Hundredth;

  /* First - get the system time set by either the user   */
  /* on start-up or the CMOS clock                        */
  DosGetTime((BYTE FAR *) & Hour,
             (BYTE FAR *) & Minute,
             (BYTE FAR *) & Second,
             (BYTE FAR *) & Hundredth);
  return TM_ENCODE(Hour, Minute, Second/2);
#else
  return 0;
#endif
}

/*                                                              */
/* dos_getftime for the file time                               */
/*                                                              */
COUNT dos_getftime(COUNT fd, date FAR * dp, time FAR * tp)
{
  f_node_ptr fnp;

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
    return DE_INVLDHNDL;

  /* Get the date and time from the fnode and return              */
  *dp = fnp->f_dir.dir_date;
  *tp = fnp->f_dir.dir_time;

  return SUCCESS;
}

/*                                                              */
/* dos_setftime for the file time                               */
/*                                                              */
COUNT dos_setftime(COUNT fd, date FAR * dp, time FAR * tp)
{
  f_node_ptr fnp;

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
    return DE_INVLDHNDL;

  /* Set the date and time from the fnode and return              */
  fnp->f_dir.dir_date = *dp;
  fnp->f_dir.dir_time = *tp;

  return SUCCESS;
}

/*                                                              */
/* dos_getfsize for the file time                               */
/*                                                              */
LONG dos_getcufsize(COUNT fd)
{
  f_node_ptr fnp;

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
    return -1l;

  /* Return the file size                                         */
  return fnp->f_highwater;
}

/*                                                              */
/* dos_getfsize for the file time                               */
/*                                                              */
LONG dos_getfsize(COUNT fd)
{
  f_node_ptr fnp;

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
    return -1l;

  /* Return the file size                                         */
  return fnp->f_dir.dir_size;
}

/*                                                              */
/* dos_setfsize for the file time                               */
/*                                                              */
BOOL dos_setfsize(COUNT fd, LONG size)
{
  f_node_ptr fnp;

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
    return FALSE;

  /* Change the file size                                         */
  fnp->f_dir.dir_size = size;
  fnp->f_highwater = size;

  merge_file_changes(fnp, FALSE);   /* /// Added - Ron Cemer */

  return TRUE;
}

/*                                                              */
/* Find free cluster in disk FAT table                          */
/*                                                              */
STATIC UWORD find_fat_free(f_node_ptr fnp)
{
  REG UWORD idx;

#ifdef DISPLAY_GETBLOCK
  printf("[find_fat_free]\n");
#endif
  /* Start from optimized lookup point for start of FAT   */
  if (fnp->f_dpb->dpb_cluster != UNKNCLUSTER)
    idx = fnp->f_dpb->dpb_cluster;
  else
    idx = 2;

  /* Search the FAT table looking for the first free      */
  /* entry.                                               */
  for (; idx <= fnp->f_dpb->dpb_size; idx++)
  {
    if (next_cluster(fnp->f_dpb, idx) == FREE)
      break;
  }

  /* No empty clusters, disk is FULL!                     */
  if (idx > fnp->f_dpb->dpb_size)
  {
    fnp->f_dpb->dpb_cluster = UNKNCLUSTER;
    dir_close(fnp);
    return LONG_LAST_CLUSTER;
  }

  if (fnp->f_dpb->dpb_nfreeclst != UNKNCLSTFREE)
      fnp->f_dpb->dpb_nfreeclst--;  /* TE: moved from link_fat() */

  /* return the free entry                                */
  fnp->f_dpb->dpb_cluster = idx;
  return idx;
}

/*                                                              */
/* create a directory - returns success or a negative error     */
/* number                                                       */
/*                                                              */
COUNT dos_mkdir(BYTE * dir)
{
  REG f_node_ptr fnp;
  REG COUNT idx;
  struct buffer FAR *bp;
  UWORD free_fat;
  UWORD parent;
  COUNT ret;

  /* first split the passed dir into comopnents (i.e. -   */
  /* path to new directory and name of new directory      */
  if ((fnp = split_path(dir, szFileName, szFileExt)) == NULL)
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }
  
  /* check that the resulting combined path does not exceed 
     the 64 PARSE_MAX limit. this leeds to problems:
     A) you can't CD to this directory later
     B) you can't create files in this subdirectory
     C) the created dir will not be found later, so you
        can create an unlimited amount of same dirs. this space
        is lost forever
  */
  if (strlen(dir) > PARSE_MAX+2) /* dir is already output of "truename" */
  {
    dir_close(fnp);
    return DE_PATHNOTFND;
  }
     
     
  
  

  /* Check that we don't have a duplicate name, so if we  */
  /* find one, it's an error.                             */
  if (find_fname(fnp, szFileName, szFileExt))
  {
    dir_close(fnp);
    return DE_ACCESS;
  }


    /* Reset the directory by a close followed by   */
    /* an open                                      */
    fnp->f_flags.f_dmod = FALSE;
    parent = fnp->f_dirstart;
    dir_close(fnp);
    fnp = split_path(dir, szFileName, szFileExt);

    /* Get a free f_node pointer so that we can use */
    /* it in building the new file.                 */
    /* Note that if we're in the root and we don't  */
    /* find an empty slot, we need to abort.        */
    if (find_free(fnp) == 0)
    {
        if  (fnp->f_flags.f_droot)
        {
            fnp->f_flags.f_dmod = FALSE;
            dir_close(fnp);
            return DE_TOOMANY;
        }

    /* Otherwise just expand the directory          */

      if ((ret = extend_dir(fnp)) != SUCCESS)
        return ret;
    }

    
    /* get an empty cluster, so that we make it into a      */
    /* directory.                                           */
    /* TE this has to be done (and failed) BEFORE the dir entry */
    /* is changed                                           */
    free_fat = find_fat_free(fnp);
    
    /* No empty clusters, disk is FULL! Translate into a    */
    /* useful error message.                                */
    if (free_fat == LONG_LAST_CLUSTER)
    {
        dir_close(fnp);
        return DE_HNDLDSKFULL;
    }
    

    /* put the fnode's name into the directory.             */
    bcopy(szFileName, (BYTE *) fnp->f_dir.dir_name, FNAME_SIZE);
    bcopy(szFileExt, (BYTE *) fnp->f_dir.dir_ext, FEXT_SIZE);

    /* Set the fnode to the desired mode                            */
    fnp->f_mode = WRONLY;
    fnp->f_back = LONG_LAST_CLUSTER;

    fnp->f_dir.dir_size = 0l;
    fnp->f_dir.dir_start = FREE;
    fnp->f_dir.dir_attrib = D_DIR;
    fnp->f_dir.dir_time = dos_gettime();
    fnp->f_dir.dir_date = dos_getdate();

    fnp->f_flags.f_dmod = TRUE;
    fnp->f_flags.f_dnew = FALSE;
    fnp->f_flags.f_ddir = TRUE;

    fnp->f_highwater = 0l;
    fnp->f_offset = 0l;


  /* Mark the cluster in the FAT as used                  */
  fnp->f_dir.dir_start = fnp->f_cluster = free_fat;
  link_fat(fnp->f_dpb, (UCOUNT) free_fat, LONG_LAST_CLUSTER);

  /* Craft the new directory. Note that if we're in a new */
  /* directory just under the root, ".." pointer is 0.    */
  /* as we are overwriting it completely, don't read first */
  bp = getblockOver((ULONG) clus2phys(free_fat,
                                  (fnp->f_dpb->dpb_clsmask + 1),
                                  fnp->f_dpb->dpb_data),
                fnp->f_dpb->dpb_unit);
#ifdef DISPLAY_GETBLOCK
  printf("FAT (dos_mkdir)\n");
#endif
  if (bp == NULL)
  {
    dir_close(fnp);
    return DE_BLKINVLD;
  }

  /* Create the "." entry                                 */
  bcopy(".       ", (BYTE *) DirEntBuffer.dir_name, FNAME_SIZE);
  bcopy("   ", (BYTE *) DirEntBuffer.dir_ext, FEXT_SIZE);
  DirEntBuffer.dir_attrib = D_DIR;
  DirEntBuffer.dir_time = dos_gettime();
  DirEntBuffer.dir_date = dos_getdate();
  DirEntBuffer.dir_start = free_fat;
  DirEntBuffer.dir_size = 0l;

  /* And put it out                                       */
  putdirent((struct dirent FAR *)&DirEntBuffer, (BYTE FAR *) bp->b_buffer);

  /* create the ".." entry                                */
  bcopy("..      ", (BYTE *) DirEntBuffer.dir_name, FNAME_SIZE);
  DirEntBuffer.dir_start = parent;

  /* and put it out                                       */
  putdirent((struct dirent FAR *)&DirEntBuffer, (BYTE FAR *) & bp->b_buffer[DIRENT_SIZE]);

  /* fill the rest of the block with zeros                */
  fmemset( & bp->b_buffer[2 * DIRENT_SIZE],0, BUFFERSIZE - 2 * DIRENT_SIZE);

  /* Mark the block to be written out                     */
  bp->b_flag |= BFR_DIRTY | BFR_VALID;

  /* clear out the rest of the blocks in the cluster      */
  for (idx = 1; idx < (fnp->f_dpb->dpb_clsmask + 1); idx++)
  {

  /* as we are overwriting it completely, don't read first */
    bp = getblockOver((ULONG) clus2phys(fnp->f_dir.dir_start,
                                    (fnp->f_dpb->dpb_clsmask + 1),
                                    fnp->f_dpb->dpb_data) + idx,
                  fnp->f_dpb->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("DIR (dos_mkdir)\n");
#endif
    if (bp == NULL)
    {
      dir_close(fnp);
      return DE_BLKINVLD;
    }
    fmemset(bp->b_buffer, 0, BUFFERSIZE);
    bp->b_flag |= BFR_DIRTY | BFR_VALID;
    bp->b_flag |= BFR_UNCACHE;          /* need not be cached */
  }

  /* flush the drive buffers so that all info is written  */
  flush_buffers((COUNT) (fnp->f_dpb->dpb_unit));

  /* Close the directory so that the entry is updated     */
  fnp->f_flags.f_dmod = TRUE;
  dir_close(fnp);

  return SUCCESS;
}

BOOL last_link(f_node_ptr fnp)
{
  return (((UWORD) fnp->f_cluster == (UWORD) LONG_LAST_CLUSTER));
}

STATIC BOOL extend(f_node_ptr fnp)
{
  UWORD free_fat;

#ifdef DISPLAY_GETBLOCK
  printf("extend\n");
#endif
  /* get an empty cluster, so that we use it to extend the file.  */
  free_fat = find_fat_free(fnp);

  /* No empty clusters, disk is FULL! Translate into a useful     */
  /* error message.                                               */
  if (free_fat == LONG_LAST_CLUSTER)
    return FALSE;

  /* Now that we've found a free FAT entry, mark it as the last   */
  /* entry and save.                                              */
  link_fat(fnp->f_dpb, (UCOUNT) fnp->f_back, free_fat);
  fnp->f_cluster = free_fat;
  link_fat(fnp->f_dpb, (UCOUNT) free_fat, LONG_LAST_CLUSTER);

  /* Mark the directory so that the entry is updated              */
  fnp->f_flags.f_dmod = TRUE;
  return TRUE;
}

STATIC COUNT extend_dir(f_node_ptr fnp)
{
  REG COUNT idx;

  if (!extend(fnp))
  {
    dir_close(fnp);
    return DE_HNDLDSKFULL;
  }

  /* clear out the rest of the blocks in the cluster              */
  for (idx = 0; idx < (fnp->f_dpb->dpb_clsmask + 1); idx++)
  {
    REG struct buffer FAR *bp;

    bp = getblockOver((ULONG) clus2phys(fnp->f_cluster,
                                    (fnp->f_dpb->dpb_clsmask + 1),
                                    fnp->f_dpb->dpb_data) + idx,
                  fnp->f_dpb->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("DIR (extend_dir)\n");
#endif
    if (bp == NULL)
    {
      dir_close(fnp);
      return DE_BLKINVLD;
    }
    fmemset(bp->b_buffer,0, BUFFERSIZE);
    bp->b_flag |= BFR_DIRTY | BFR_VALID;
    
    if (idx != 0)
        bp->b_flag |= BFR_UNCACHE;      /* needs not be cached */
  }

  if (!find_free(fnp))
  {
    dir_close(fnp);
    return DE_HNDLDSKFULL;
  }

  /* flush the drive buffers so that all info is written          */
  flush_buffers((COUNT) (fnp->f_dpb->dpb_unit));

  return SUCCESS;

}

/* JPP: finds the next free cluster in the FAT */
STATIC BOOL first_fat(f_node_ptr fnp)
{
  UWORD free_fat;

  /* get an empty cluster, so that we make it into a file.        */
  free_fat = find_fat_free(fnp);

  /* No empty clusters, disk is FULL! Translate into a useful     */
  /* error message.                                               */
  if (free_fat == LONG_LAST_CLUSTER)
    return FALSE;

  /* Now that we've found a free FAT entry, mark it as the last   */
  /* entry and save it.                                           */
  /* BUG!! this caused wrong allocation, if file was created, 
     then seeked, then written */
  fnp->f_cluster = 
      fnp->f_dir.dir_start = free_fat;
  link_fat(fnp->f_dpb, (UCOUNT) free_fat, LONG_LAST_CLUSTER);

  /* Mark the directory so that the entry is updated              */
  fnp->f_flags.f_dmod = TRUE;
  return TRUE;
}

/* JPP:  I think this starts at the beginning of a file, and follows
   the fat chain to find the cluster that contains the data for the
   file at f_offset. */

/* JPP: new map_cluster.  If we are moving forward, then use the offset
   that we are at now (f_cluster_offset) to start, instead of starting
   at the beginning. */
COUNT map_cluster(REG f_node_ptr fnp, COUNT mode)
{
  ULONG idx;
  ULONG clssize;                /* might be 64K (by WinNT) TE */

#ifdef DISPLAY_GETBLOCK
  printf("map_cluster: current %lu, offset %lu, diff=%lu ",
         fnp->f_cluster_offset, fnp->f_offset,
         fnp->f_offset - fnp->f_cluster_offset);
#endif
  /* The variable clssize will be used later.             */
  clssize = (ULONG)fnp->f_dpb->dpb_secsize * (fnp->f_dpb->dpb_clsmask + 1);

  /* If someone did a seek, but no writes have occured, we will   */
  /* need to initialize the fnode.                                */
  if ((mode == XFR_WRITE) && (fnp->f_dir.dir_start == FREE))
  {
    /* If there are no more free fat entries, then we are full! */
    if (!first_fat(fnp))
      return DE_HNDLDSKFULL;
  }

  if (fnp->f_offset >= fnp->f_cluster_offset)	/*JPP */
  {
    /* Set internal index and cluster size.                 */
    idx = fnp->f_offset - fnp->f_cluster_offset;
  }
  else
  {
    /* Set internal index and cluster size.                 */
    idx = fnp->f_offset;

    fnp->f_cluster = fnp->f_flags.f_ddir ? fnp->f_dirstart :
        fnp->f_dir.dir_start;
    fnp->f_cluster_offset = 0;
  }

  /* Now begin the linear search. The relative cluster is         */
  /* maintained as part of the set of physical indices. It is     */
  /* also the highest order index and is mapped directly into     */
  /* physical cluster. Our search is performed by pacing an index */
  /* up to the relative cluster position where the index falls    */
  /* within the cluster.                                          */
  /*                                                              */
  /* NOTE: make sure your compiler does not optimize for loop     */
  /* tests to the loop exit. We need to fall out immediately for  */
  /* files whose length < cluster size.                           */
  for (; idx >= clssize; idx -= clssize)
  {
    /* If this is a read and the next is a LAST_CLUSTER,    */
    /* then we are going to read past EOF, return zero read */
    if ((mode == XFR_READ) && last_link(fnp))
      return DE_SEEK;
/* expand the list if we're going to write and have run into    */
/* the last cluster marker.                                     */
    else if ((mode == XFR_WRITE) && last_link(fnp))
    {

      if (!extend(fnp))
      {
        dir_close(fnp);
        return DE_HNDLDSKFULL;
      }
    }
    fnp->f_back = fnp->f_cluster;

    /* get next cluster in the chain */
    fnp->f_cluster = next_cluster(fnp->f_dpb, fnp->f_cluster);
    fnp->f_cluster_offset += clssize;
  }
#ifdef DISPLAY_GETBLOCK
  printf("done.\n");
#endif

  return SUCCESS;
}

/* Read block from disk */
UCOUNT readblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err)
{
  REG f_node_ptr fnp;
  REG struct buffer FAR *bp;
  UCOUNT xfr_cnt = 0;
  UCOUNT ret_cnt = 0;
  UWORD secsize;
  UCOUNT to_xfer = count;

#if defined( DEBUG ) && 0
  if (bDumpRdWrParms)
  {
    printf("readblock:fd %02x  buffer %04x:%04x count %x\n",
           fd, FP_SEG(buffer), FP_OFF(buffer), count);
  }
#endif
  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

  /* Test that we are really about to do a data transfer. If the
     count is zero, just exit. (Any read with a count of zero is a nop). */

  /* NOTE: doing this up front saves a lot of headaches later.    */
  if (count == 0)
  {
    *err = SUCCESS;
    return 0;
  }

  /* Another test is to check for a seek past EOF  */
/*  if (!fnp->f_flags.f_ddir && (fnp->f_offset >= fnp->f_dir.dir_size)) BUG :-< */
  if (!fnp->f_flags.f_ddir && (fnp->f_offset >= fnp->f_highwater))
  {
    *err = SUCCESS;
    return 0;
  }

  /* test that we have a valid mode for this fnode                */
  if (fnp->f_mode != RDONLY && fnp->f_mode != RDWR)
  {
    *err = DE_INVLDACC;
    return 0;
  }

  /* The variable secsize will be used later.                     */
  secsize = fnp->f_dpb->dpb_secsize;

  /* Adjust the far pointer from user space to supervisor space   */
  buffer = adjust_far((VOID FAR *) buffer);

  /* Do the data transfer. Use block transfer methods so that we  */
  /* can utilize memory management in future DOS-C versions.      */
  while (ret_cnt < count)
  {
    /* Position the file to the fnode's pointer position. This is   */
    /* done by updating the fnode's cluster, block (sector) and     */
    /* byte offset so that read becomes a simple data move          */
    /* out of the block data buffer.                                */
    if (fnp->f_offset == 0l)
    {
      /* complete the common operations of            */
      /* initializing to the starting cluster and     */
      /* setting all offsets to zero.                 */
      fnp->f_cluster = fnp->f_flags.f_ddir ? fnp->f_dirstart :
          fnp->f_dir.dir_start;

      fnp->f_cluster_offset = 0l;
      fnp->f_back = LONG_LAST_CLUSTER;
      fnp->f_sector = 0;
      fnp->f_boff = 0;
    }
    /* The more difficult scenario is the (more common)     */
    /* file offset case. Here, we need to take the fnode's  */
    /* offset pointer (f_offset) and translate it into a    */
    /* relative cluster position, cluster block (sector)    */
    /* offset (f_sector) and byte offset (f_boff). Once we  */
    /* have this information, we need to translate the      */
    /* relative cluster position into an absolute cluster   */
    /* position (f_cluster). This is unfortunate because it */
    /* requires a linear search through the file's FAT      */
    /* entries. It made sense when DOS was originally       */
    /* designed as a simple floppy disk operating system    */
    /* where the FAT was contained in core, but now         */
    /* requires a search through the FAT blocks.            */
    /*                                                      */
    /* The algorithm in this function takes advantage of    */
    /* the blockio block buffering scheme to simplify the   */
    /* task.                                                */
    else
    {
#ifdef DISPLAY_GETBLOCK
      printf("readblock: ");
#endif
      switch (map_cluster(fnp, XFR_READ))
      {
        case DE_SEEK:
          *err = DE_SEEK;
          dir_close(fnp);
          return ret_cnt;

        default:
          dir_close(fnp);
          *err = DE_HNDLDSKFULL;
          return ret_cnt;

        case SUCCESS:
          break;
      }
    }

    /* Compute the block within the cluster and the offset  */
    /* within the block.                                    */
    fnp->f_sector = (fnp->f_offset / secsize) & fnp->f_dpb->dpb_clsmask;
    fnp->f_boff = fnp->f_offset & (secsize - 1);

#ifdef DSK_DEBUG
    printf("read %d links; dir offset %ld, cluster %d\n",
           fnp->f_count,
           fnp->f_diroff,
           fnp->f_cluster);
#endif
    /* Do an EOF test and return whatever was transferred   */
    /* but only for regular files.                          */
    if (!(fnp->f_flags.f_ddir)
        && (fnp->f_offset >= fnp->f_highwater))
    {
      *err = SUCCESS;
      return ret_cnt;
    }

    /* Get the block we need from cache                     */
    bp = getblock((ULONG) clus2phys(fnp->f_cluster,
                                    (fnp->f_dpb->dpb_clsmask + 1),
                                    fnp->f_dpb->dpb_data) + fnp->f_sector,
                  fnp->f_dpb->dpb_unit);

#ifdef DISPLAY_GETBLOCK
    printf("DATA (readblock)\n");
#endif
    if (bp == NULL)         /* (struct buffer *)0 --> DS:0 !! */
    {
      *err = DE_BLKINVLD;
      return ret_cnt;
    }

    /* transfer a block                                     */
    /* Transfer size as either a full block size, or the    */
    /* requested transfer size, whichever is smaller.       */
    /* Then compare to what is left, since we can transfer  */
    /* a maximum of what is left.                           */
    if (fnp->f_flags.f_ddir)
      xfr_cnt = min(to_xfer, secsize - fnp->f_boff);
    else
      xfr_cnt = (UWORD)min(min(to_xfer, secsize - fnp->f_boff),
                    fnp->f_highwater - fnp->f_offset);

    fbcopy((BYTE FAR *) & bp->b_buffer[fnp->f_boff], buffer, xfr_cnt);

                                        /* complete buffer read ? 
                                           probably not reused later
                                         */
    if (xfr_cnt == sizeof(bp->b_buffer) ||
            fnp->f_offset + xfr_cnt == fnp->f_highwater )
        {    
        bp->b_flag |= BFR_UNCACHE;   
        }

    /* update pointers and counters                         */
    ret_cnt += xfr_cnt;
    to_xfer -= xfr_cnt;
    fnp->f_offset += xfr_cnt;
    buffer = add_far((VOID FAR *) buffer, (ULONG) xfr_cnt);
  }
  *err = SUCCESS;
  return ret_cnt;
}

/* Write block to disk */
UCOUNT writeblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err)
{
  REG f_node_ptr fnp;
  struct buffer FAR *bp;
  UCOUNT xfr_cnt = 0;
  UCOUNT ret_cnt = 0;
  UWORD secsize;
  UCOUNT to_xfer = count;

#ifdef DEBUG
  if (bDumpRdWrParms)
  {
    printf("writeblock: fd %02d buffer %04x:%04x count %d\n",
           fd, (COUNT) FP_SEG(buffer), (COUNT) FP_OFF(buffer), count);
  }
#endif
  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
  {
    *err = DE_INVLDHNDL;
    return 0;
  }

/* /// BUG!!! Moved from below next block because we should NOT be able
       to truncate the file if we can't write to it.  - Ron Cemer */
  /* test that we have a valid mode for this fnode                */
  if (fnp->f_mode != WRONLY && fnp->f_mode != RDWR)
  {
    *err = DE_INVLDACC;
    return 0;
  }

  fnp->f_flags.f_dmod = TRUE;         /* mark file as modified */


  /* Test that we are really about to do a data transfer. If the  */
  /* count is zero and the mode is XFR_READ, just exit. (Any      */
  /* read with a count of zero is a nop).                         */
  /*                                                              */
  /* A write (mode is XFR_WRITE) is a special case.  It sets the  */
  /* file length to the current length (truncates it).            */
  /*                                                              */
  /* NOTE: doing this up front saves a lot of headaches later.    */
  
  if (count == 0)
  {
    /* NOTE: doing this up front made a lot of headaches later :-( TE */ 
    /* FAT allocation has to be extended if necessary              TE */

    *err = SUCCESS;
    if (fnp->f_highwater < fnp->f_offset)
        {
          switch (map_cluster(fnp, XFR_WRITE))
          {
            case DE_SEEK:
              *err = DE_SEEK;
              dir_close(fnp);
              break;    

            default:
              dir_close(fnp);
              *err = DE_HNDLDSKFULL;
              break;
    
            case SUCCESS:
              break;
          }
        }
    
    fnp->f_highwater = fnp->f_offset;
    merge_file_changes(fnp, FALSE);     /* /// Added - Ron Cemer */
    return 0;
  }

/* /// BUG!!! Moved to above previous block because we should NOT be able
       to truncate the file if we can't write to it.  - Ron Cemer */
  /* test that we have a valid mode for this fnode                */
/*
  if (fnp->f_mode != WRONLY && fnp->f_mode != RDWR)
  {
    *err = DE_INVLDACC;
    return 0;
  }
*/

  /* The variable secsize will be used later.                     */
  secsize = fnp->f_dpb->dpb_secsize;

  /* Adjust the far pointer from user space to supervisor space   */
  buffer = adjust_far((VOID FAR *) buffer);

  /* Do the data transfer. Use block transfer methods so that we  */
  /* can utilize memory management in future DOS-C versions.      */
  while (ret_cnt < count)
  {
    /* Position the file to the fnode's pointer position. This is   */
    /* done by updating the fnode's cluster, block (sector) and     */
    /* byte offset so that read or write becomes a simple data move */
    /* into or out of the block data buffer.                        */
    if (fnp->f_offset == 0l)
    {
      /* For the write case, a newly created file     */
      /* will have a start cluster of FREE. If we're  */
      /* doing a write and this is the first time     */
      /* through, allocate a new cluster to the file. */
      if (fnp->f_dir.dir_start == FREE)
        if (!first_fat(fnp))    /* get a free cluster */
        {                       /* error means disk full */
          dir_close(fnp);
          *err = DE_HNDLDSKFULL;
          return ret_cnt;
        }
      /* complete the common operations of            */
      /* initializing to the starting cluster and     */
      /* setting all offsets to zero.                 */
      fnp->f_cluster = fnp->f_flags.f_ddir ? fnp->f_dirstart :
          fnp->f_dir.dir_start;

      fnp->f_cluster_offset = 0l;
      fnp->f_back = LONG_LAST_CLUSTER;
      fnp->f_sector = 0;
      fnp->f_boff = 0;
      merge_file_changes(fnp, FALSE);   /* /// Added - Ron Cemer */
    }

    /* The more difficult scenario is the (more common)     */
    /* file offset case. Here, we need to take the fnode's  */
    /* offset pointer (f_offset) and translate it into a    */
    /* relative cluster position, cluster block (sector)    */
    /* offset (f_sector) and byte offset (f_boff). Once we  */
    /* have this information, we need to translate the      */
    /* relative cluster position into an absolute cluster   */
    /* position (f_cluster). This is unfortunate because it */
    /* requires a linear search through the file's FAT      */
    /* entries. It made sense when DOS was originally       */
    /* designed as a simple floppy disk operating system    */
    /* where the FAT was contained in core, but now         */
    /* requires a search through the FAT blocks.            */
    /*                                                      */
    /* The algorithm in this function takes advantage of    */
    /* the blockio block buffering scheme to simplify the   */
    /* task.                                                */
    else
    {
#ifdef DISPLAY_GETBLOCK
      printf("writeblock: ");
#endif
      switch (map_cluster(fnp, XFR_WRITE))
      {
        case DE_SEEK:
          *err = DE_SEEK;
          dir_close(fnp);
          return ret_cnt;

        default:
          dir_close(fnp);
          *err = DE_HNDLDSKFULL;
          return ret_cnt;

        case SUCCESS:
          merge_file_changes(fnp, FALSE);   /* /// Added - Ron Cemer */
          break;
      }
    }

    /* XFR_WRITE case only - if we're at the end, the next  */
    /* FAT is an EOF marker, so just extend the file length */
    if (last_link(fnp)) {
      if (!extend(fnp))
      {
        dir_close(fnp);
        *err = DE_HNDLDSKFULL;
        return ret_cnt;
      }
      merge_file_changes(fnp, FALSE);   /* /// Added - Ron Cemer */
    }

    /* Compute the block within the cluster and the offset  */
    /* within the block.                                    */
    fnp->f_sector =
        (fnp->f_offset / secsize) & fnp->f_dpb->dpb_clsmask;
    fnp->f_boff = fnp->f_offset & (secsize - 1);

#ifdef DSK_DEBUG
    printf("write %d links; dir offset %ld, cluster %d\n",
           fnp->f_count,
           fnp->f_diroff,
           fnp->f_cluster);
#endif

/* /// Moved xfr_cnt calculation from below so we can
       use it to help decide how to get the block:
           read-modify-write using getblock() if we are only
               going to write part of the block, or
           write using getbuf() if we are going to write
               the entire block.
       - Ron Cemer */
    xfr_cnt = min(to_xfer, secsize - fnp->f_boff);

    /* get a buffer to store the block in */
    /* /// BUG!!! Added conditional to only use getbuf() if we're going
           to write the entire block, which is faster because it does
           not first read the block from disk.  However, if we are
           going to only write part of the block, we MUST use the
           getblock() call, which first reads the block from disk.
           Without this modification, the kernel was writing garbage
           to the file when sequential writes were attempted at less
           than the block size.  This was causing problems with
           piping and redirection in FreeCOM, as well as many other
           potential problems.
           - Ron Cemer */
    if ( (fnp->f_boff == 0) && (xfr_cnt == secsize) ) {
        bp = getblockOver((ULONG) clus2phys(fnp->f_cluster,
                                           (fnp->f_dpb->dpb_clsmask + 1),
                                       fnp->f_dpb->dpb_data) + fnp->f_sector,
                    fnp->f_dpb->dpb_unit);
        
    } else {
        bp = getblock((ULONG) clus2phys(fnp->f_cluster,
                                  (fnp->f_dpb->dpb_clsmask + 1),
                                  fnp->f_dpb->dpb_data) + fnp->f_sector,
                fnp->f_dpb->dpb_unit);
    }
    if (bp == NULL) {
      *err = DE_BLKINVLD;
      return ret_cnt;
      }
    

    /* transfer a block                                     */
    /* Transfer size as either a full block size, or the    */
    /* requested transfer size, whichever is smaller.       */
    /* Then compare to what is left, since we can transfer  */
    /* a maximum of what is left.                           */
/* /// Moved xfr_cnt calculation to above getbuf/getblock calls so we can
       use it to help decide which one to call.
       - Ron Cemer
    xfr_cnt = min(to_xfer, secsize - fnp->f_boff);
*/
    fbcopy(buffer, (BYTE FAR *) & bp->b_buffer[fnp->f_boff], xfr_cnt);
    bp->b_flag |= BFR_DIRTY | BFR_VALID;

    if (xfr_cnt == sizeof(bp->b_buffer)) /* probably not used later */
        {    
        bp->b_flag |= BFR_UNCACHE;   
        }


    fnp->f_flags.f_dmod = TRUE;  /* mark file as modified */


    /* update pointers and counters                         */
    ret_cnt += xfr_cnt;
    to_xfer -= xfr_cnt;
    fnp->f_offset += xfr_cnt;
    buffer = add_far((VOID FAR *) buffer, (ULONG) xfr_cnt);
    if (fnp->f_offset > fnp->f_highwater)
    {
      fnp->f_highwater = fnp->f_offset;
      fnp->f_dir.dir_size = fnp->f_highwater;
    }
    merge_file_changes(fnp, FALSE);   /* /// Added - Ron Cemer */
  }
  *err = SUCCESS;
  return ret_cnt;
}

COUNT dos_read(COUNT fd, VOID FAR * buffer, UCOUNT count)
{
  COUNT err;
  UCOUNT xfr;

  xfr = readblock(fd, buffer, count, &err);
  return err != SUCCESS ? err : xfr;
}

#ifndef IPL
COUNT dos_write(COUNT fd, VOID FAR * buffer, UCOUNT count)
{
  REG f_node_ptr fnp;
  COUNT err,
    xfr;

  /* First test if we need to fill to new EOF.                    */

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  /* note: an invalid fd is indicated by a 0 return               */
  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
  {
    return DE_INVLDHNDL;
  }

  /* Future note: for security purposes, this should be set to    */
  /* blocks of 0.  This satisfies spec and guarantees no secure   */
  /* info is written to disk.                                     */
  /* Also, with real memory management, this may cause a page     */
  /* fault.                                                       */
  if (fnp->f_offset > fnp->f_highwater)
  {
    ULONG lCount = fnp->f_offset - fnp->f_highwater;

    while (lCount > 0)
    {
      writeblock(fd, buffer,
                 lCount > 512l ? 512 : (UCOUNT) lCount,
                 &err);
      lCount -= 512;
    }
  }

  xfr = writeblock(fd, buffer, count, &err);
  return err != SUCCESS ? err : xfr;
}
#endif

/* Position the file pointer to the desired offset                      */
/* Returns a long current offset or a negative error code               */
LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin)
{
  REG f_node_ptr fnp;

  /* Translate the fd into a useful pointer                       */

  fnp = xlt_fd(fd);

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit                */
  /* note: an invalid fd is indicated by a 0 return               */

  if (fnp == (f_node_ptr)0 || fnp->f_count <= 0)
    return (LONG) DE_INVLDHNDL;

  /* now do the actual lseek adjustment to the file poitner       */

  switch (origin)
  {
      /* offset from beginning of file                                */
    case 0:
      return fnp->f_offset = (ULONG) foffset;

      /* offset from current location                                 */
    case 1:
      return fnp->f_offset += foffset;

      /* offset from eof                                              */
    case 2:
      return fnp->f_offset = fnp->f_highwater + foffset;

      /* default to an invalid function                               */
    default:
      return (LONG) DE_INVLDFUNC;
  }
}

/* returns the number of unused clusters */
UWORD dos_free(struct dpb FAR *dpbp)
{
  /* There's an unwritten rule here. All fs       */
  /* cluster start at 2 and run to max_cluster+2  */
  REG UWORD i;
  REG UWORD cnt = 0;
/*  UWORD max_cluster = ( ((ULONG) dpbp->dpb_size
 *                 * (ULONG) (dpbp->dpb_clsmask + 1) - (dpbp->dpb_data + 1) )
 * / (dpbp->dpb_clsmask + 1) ) + 2;
 */

/*??  UWORD max_cluster = ( ((ULONG) dpbp->dpb_size * (ULONG) (dpbp->dpb_clsmask + 1))
                        / (dpbp->dpb_clsmask + 1) ) + 1;
*/
  UWORD max_cluster = dpbp->dpb_size  + 1;

  if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
    return dpbp->dpb_nfreeclst;
  else
  {
    for (i = 2; i < max_cluster; i++)
    {
      if (next_cluster(dpbp, i) == 0)
        ++cnt;
    }
    dpbp->dpb_nfreeclst = cnt;
    return cnt;
  }
}



#ifndef IPL
COUNT dos_cd(struct cds FAR * cdsp, BYTE *PathName)
{
  f_node_ptr fnp;

	/* first check for valid drive          */
	if (cdsp->cdsDpb == 0)
		return DE_INVLDDRV;

	if ((media_check(cdsp->cdsDpb) < 0))
		return DE_INVLDDRV;

  /* now test for its existance. If it doesn't, return an error.  */
  /* If it does, copy the path to the current directory           */
  /* structure.                                                   */
  if ((fnp = dir_open(PathName)) == NULL)
    return DE_PATHNOTFND;

  cdsp->cdsStrtClst = fnp->f_dirstart;
  dir_close(fnp);
  fstrncpy(cdsp->cdsCurrentPath,&PathName[0],sizeof(cdsp->cdsCurrentPath)-1);
  return SUCCESS;
}
#endif

/* Try to allocate an f_node from the available files array */

f_node_ptr get_f_node(void)
{
  REG i;

  for (i = 0; i < f_nodes_cnt; i++)
  {
    if (f_nodes[i].f_count == 0)
    {
      ++f_nodes[i].f_count;
      return &f_nodes[i];
    }
  }
  return (f_node_ptr)0;
}

VOID release_f_node(f_node_ptr fnp)
{
  if (fnp->f_count > 0)
    --fnp->f_count;
  else
    fnp->f_count = 0;
}

#ifndef IPL
VOID dos_setdta(BYTE FAR * newdta)
{
  dta = newdta;
}

COUNT dos_getfattr(BYTE * name)
{
  f_node_ptr fnp;
  COUNT fd, result;

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  if ((fd = dos_open(name, O_RDONLY)) < SUCCESS)
    return DE_FILENOTFND;

  /* note: an invalid fd is indicated by a 0 return               */
  if ((fnp = xlt_fd(fd)) == (f_node_ptr)0)
    return DE_TOOMANY;

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  if (fnp->f_count <= 0)
  {
    dos_close(fd);
    return DE_FILENOTFND;
  }

  /* Get the attribute from the fnode and return          */
  result = fnp->f_dir.dir_attrib;
  dos_close(fd);
  return result;
}

COUNT dos_setfattr(BYTE * name, UWORD attrp)
{
  f_node_ptr fnp;
  COUNT fd;

  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  if ((fd = dos_open(name, O_RDONLY)) < SUCCESS)
    return DE_FILENOTFND;

  /* note: an invalid fd is indicated by a 0 return               */
  if ((fnp = xlt_fd(fd)) == (f_node_ptr)0)
    return DE_TOOMANY;

  /* If the fd was invalid because it was out of range or the     */
  /* requested file was not open, tell the caller and exit        */
  if (fnp->f_count <= 0)
  {
    dos_close(fd);
    return DE_FILENOTFND;
  }
  /* JPP-If user tries to set VOLID or DIR bits, return error */
  if ((attrp & (D_VOLID | D_DIR | 0xC0)) != 0)
  {
    dos_close(fd);
    return DE_ACCESS;
  }

  /* Set the attribute from the fnode and return          */
  /* clear all attributes but DIR and VOLID */
  fnp->f_dir.dir_attrib &= (D_VOLID | D_DIR);	/* JPP */

  /* set attributes that user requested */
  fnp->f_dir.dir_attrib |= attrp;	/* JPP */
  fnp->f_flags.f_dmod = TRUE;
  dos_close(fd);
  return SUCCESS;
}
#endif

VOID bpb_to_dpb(bpb FAR *bpbp, REG struct dpb FAR * dpbp)
{
  ULONG size;
  REG COUNT i;

      dpbp->dpb_mdb = bpbp->bpb_mdesc;
      dpbp->dpb_secsize = bpbp->bpb_nbyte;
      dpbp->dpb_clsmask = bpbp->bpb_nsector - 1;
      dpbp->dpb_fatstrt = bpbp->bpb_nreserved;
      dpbp->dpb_fats = bpbp->bpb_nfat;
      dpbp->dpb_dirents = bpbp->bpb_ndirent;
      size = bpbp->bpb_nsize == 0 ?
          bpbp->bpb_huge :
          (ULONG) bpbp->bpb_nsize;
/* patch point
      dpbp->dpb_size = size / ((ULONG) bpbp->bpb_nsector);
*/
      dpbp->dpb_fatsize = bpbp->bpb_nfsect;
      dpbp->dpb_dirstrt = dpbp->dpb_fatstrt
          + dpbp->dpb_fats * dpbp->dpb_fatsize;
      dpbp->dpb_data = dpbp->dpb_dirstrt
          + ((DIRENT_SIZE * dpbp->dpb_dirents
              + (dpbp->dpb_secsize - 1))
             / dpbp->dpb_secsize);
/*
	Michal Meller <maceman@priv4,onet.pl> patch to jimtabor
*/
      dpbp->dpb_size = ((size - dpbp->dpb_data) / ((ULONG) bpbp->bpb_nsector) + 1);

      dpbp->dpb_flags = 0;
/*      dpbp->dpb_next = (struct dpb FAR *)-1;*/
      dpbp->dpb_cluster = UNKNCLUSTER;
      dpbp->dpb_nfreeclst = UNKNCLSTFREE;	/* number of free clusters */
      for (i = 1, dpbp->dpb_shftcnt = 0;
           i < (sizeof(dpbp->dpb_shftcnt) * 8);	/* 8 bit bytes in C */
           dpbp->dpb_shftcnt++, i <<= 1)
      {
        if (i >= bpbp->bpb_nsector)
          break;
      }
}

COUNT media_check(REG struct dpb FAR * dpbp)
{
  /* First test if anyone has changed the removable media         */
  FOREVER
  {
    MediaReqHdr.r_length = sizeof(request);
    MediaReqHdr.r_unit = dpbp->dpb_subunit;
    MediaReqHdr.r_command = C_MEDIACHK;
    MediaReqHdr.r_mcmdesc = dpbp->dpb_mdb;
    MediaReqHdr.r_status = 0;
    execrh((request FAR *) & MediaReqHdr, dpbp->dpb_device);
    if (!(MediaReqHdr.r_status & S_ERROR) && (MediaReqHdr.r_status & S_DONE))
      break;
    else
    {
    loop1:
      switch (block_error(&MediaReqHdr, dpbp->dpb_unit, dpbp->dpb_device))
      {
        case ABORT:
        case FAIL:
          return DE_INVLDDRV;

        case RETRY:
          continue;

        case CONTINUE:
          break;

        default:
          goto loop1;
      }
    }
  }

  switch (MediaReqHdr.r_mcretcode | dpbp->dpb_flags)
  {
    case M_NOT_CHANGED:
      /* It was definitely not changed, so ignore it          */
      return SUCCESS;

      /* If it is forced or the media may have changed,       */
      /* rebuild the bpb                                      */
    case M_DONT_KNOW:
      flush_buffers(dpbp->dpb_unit);

      /* If it definitely changed, don't know (falls through) */
      /* or has been changed, rebuild the bpb.                */
    case M_CHANGED:
    default:
      setinvld(dpbp->dpb_unit);
      FOREVER
      {
        MediaReqHdr.r_length = sizeof(request);
        MediaReqHdr.r_unit = dpbp->dpb_subunit;
        MediaReqHdr.r_command = C_BLDBPB;
        MediaReqHdr.r_mcmdesc = dpbp->dpb_mdb;
        MediaReqHdr.r_status = 0;
        execrh((request FAR *) & MediaReqHdr, dpbp->dpb_device);
        if (!(MediaReqHdr.r_status & S_ERROR) && (MediaReqHdr.r_status & S_DONE))
          break;
        else
        {
        loop2:
          switch (block_error(&MediaReqHdr, dpbp->dpb_unit, dpbp->dpb_device))
          {
            case ABORT:
            case FAIL:
              return DE_INVLDDRV;

            case RETRY:
              continue;

            case CONTINUE:
              break;

            default:
              goto loop2;
          }
        }
      }
      bpb_to_dpb(MediaReqHdr.r_bpptr, dpbp);
      return SUCCESS;
  }
}

/* translate the fd into an f_node pointer */
f_node_ptr xlt_fd(COUNT fd)
{
  return fd >= f_nodes_cnt ? (f_node_ptr)0 : &f_nodes[fd];
}

/* translate the f_node pointer into an fd */
COUNT xlt_fnp(f_node_ptr fnp)
{
  return (COUNT)(fnp - f_nodes);
}

#if 0
struct dhdr FAR *select_unit(COUNT drive)
{
  /* Just get the header from the dhdr array                      */
/*  return blk_devices[drive].dpb_device; */

    return (struct dhdr FAR *)CDSp->cds_table[drive].cdsDpb;

}
#endif


/* TE
    if the current filesize in FAT is larger then the dir_size
    it's truncated here.
    the BUG was:
    copy COMMAND.COM xxx
    echo >xxx
    
    then, the dirsize of xxx was set to ~20, but the allocated 
    FAT entries not returned.
    this code corrects this
    
    Unfortunately, this code _nearly_ works, but fails one of the 
    Apps tested (VB ISAM); so it's disabled for the moment
*/

STATIC VOID shrink_file(f_node_ptr fnp)
{
#if 0
    UNREFERENCED_PARAMETER(fnp);
#else

    ULONG lastoffset = fnp->f_offset;    /* has to be saved */
    UCOUNT next,st;
    struct dpb FAR *dpbp = fnp->f_dpb;


    fnp->f_offset = fnp->f_highwater;     /* end of file */
    
    if (map_cluster(fnp, XFR_READ) != SUCCESS)  /* error, don't truncate */
        goto done;
        
        
    st = fnp->f_cluster;    
    
    if (st == FREE || st == LONG_LAST_CLUSTER) /* first cluster is free or EOC, done */
        goto done;
    
    next = next_cluster(dpbp, st);

    if ( next == LONG_LAST_CLUSTER)         /* last cluster found */
        goto done;

          /* Loop from start until either a FREE entry is         */
          /* encountered (due to a fractured file system) of the  */
          /* last cluster is encountered.                         */
          /* zap the FAT pointed to                       */

          
    if (fnp->f_highwater == 0)
        {
        fnp->f_dir.dir_start = FREE;
        link_fat(dpbp, st, FREE);
        }    
      else        
        {
        link_fat(dpbp, st,LONG_LAST_CLUSTER);
        }

    for ( st = next; st != LONG_LAST_CLUSTER; st = next)
      {
        /* get the next cluster pointed to              */
        next = next_cluster(dpbp, st);
    
        /* just exit if a damaged file system exists    */
        if (next == FREE)
          return;
    
        /* zap the FAT pointed to                       */
        link_fat(dpbp, st, FREE);
    
        /* and the start of free space pointer          */
        if ((dpbp->dpb_cluster == UNKNCLUSTER)
            || (dpbp->dpb_cluster > st))
          dpbp->dpb_cluster = st;
    
      }
                
done:
    fnp->f_offset = lastoffset;    /* has to be restored */
    
    
#endif    
}    
