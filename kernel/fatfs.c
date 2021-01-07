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
BYTE *RcsId = "$Id: fatfs.c 1632 2011-06-13 16:29:14Z bartoldeman $";
#endif

/*                                                                      */
/*      function prototypes                                             */
/*                                                                      */
STATIC f_node_ptr sft_to_fnode(int fd);
STATIC void fnode_to_sft(f_node_ptr fnp);
STATIC int find_fname(const char *path, int attr, f_node_ptr fnp);
    /* /// Added - Ron Cemer */
STATIC int merge_file_changes(f_node_ptr fnp, int collect);
STATIC BOOL find_free(f_node_ptr);
STATIC int alloc_find_free(f_node_ptr fnp, char *path);
STATIC VOID wipe_out(f_node_ptr);
STATIC CLUSTER extend(f_node_ptr);
STATIC COUNT extend_dir(f_node_ptr);
CLUSTER first_fat(f_node_ptr);
COUNT map_cluster(f_node_ptr, COUNT);
STATIC int shrink_file(f_node_ptr fnp);

/* FAT time notation in the form of hhhh hmmm mmmd dddd (d = double second) */
STATIC time time_encode(struct dostime *t)
{
  return (t->hour << 11) | (t->minute << 5) | (t->second >> 1);
}

#ifdef WITHFAT32
CLUSTER getdstart(struct dpb FAR *dpbp, struct dirent *dentry)
{
  if (!ISFAT32(dpbp))
    return dentry->dir_start;
  return (((CLUSTER)dentry->dir_start_high << 16) | dentry->dir_start);
}

void setdstart(struct dpb FAR *dpbp, struct dirent *dentry, CLUSTER value)
{
  dentry->dir_start = (UWORD)value;
  if (ISFAT32(dpbp))
    dentry->dir_start_high = (UWORD)(value >> 16);
}
#endif

ULONG clus2phys(CLUSTER cl_no, struct dpb FAR * dpbp)
{
  CLUSTER data =
#ifdef WITHFAT32
      ISFAT32(dpbp) ? dpbp->dpb_xdata :
#endif
      dpbp->dpb_data;
  return ((ULONG) (cl_no - 2) << dpbp->dpb_shftcnt) + data;
}

struct dpb FAR *get_dpb(COUNT dsk)
{
  register struct cds FAR *cdsp = get_cds(dsk);
  
  if (cdsp == NULL || cdsp->cdsFlags & CDSNETWDRV)
    return NULL;
  return cdsp->cdsDpb;
}

/* initialize directory entry (creation/access stamps 0 as per MS-DOS 7.10) */
STATIC void init_direntry(struct dirent *dentry, unsigned attrib,
                          CLUSTER cluster, char *name)
{
  memset(dentry, 0, sizeof(struct dirent));
  memcpy(dentry->dir_name, name, FNAME_SIZE + FEXT_SIZE);
#ifdef WITHFAT32
  dentry->dir_start_high = (UWORD)(cluster >> 16);
#endif
  dentry->dir_start = (UWORD)cluster;
  dentry->dir_attrib = (UBYTE)attrib;
  /* set create and last modified time & date */
  dentry->dir_crtime = dentry->dir_time = dos_gettime();
  dentry->dir_crdate = dentry->dir_date = dos_getdate();
}

/************************************************************************/
/*                                                                      */
/*      Internal file handlers - open, create, read, write, close, etc. */
/*                                                                      */
/************************************************************************/

/* Open a file given the path. Flags is 0 for read, 1 for write and 2   */
/* for update.                                                          */
/* Returns an long where the high word is a status code and the low     */
/* word is an integer file descriptor or a negative error code          */
/* see DosOpenSft(), dosfns.c for an explanation of the flags bits      */
/* directory opens are allowed here; these are not allowed by DosOpenSft*/

int dos_open(char *path, unsigned flags, unsigned attrib, int fd)
{
  REG f_node_ptr fnp = sft_to_fnode(fd);
  int status = find_fname(path, D_ALL | attrib, fnp);

  /* Check that we don't have a duplicate name, so if we  */
  /* find one, truncate it (O_CREAT).                     */
  if (status == SUCCESS)
  {
    unsigned char dir_attrib = fnp->f_dir.dir_attrib;
    if (flags & O_TRUNC)
    {
      /* The only permissable attribute is archive,   */
      /* check for any other bit set. If it is, give  */
      /* an access error.                             */
      if ((dir_attrib & (D_RDONLY | D_DIR | D_VOLID))
          || (dir_attrib & ~D_ARCHIVE & ~attrib))
        return DE_ACCESS;

      /* Release the existing files FAT and set the   */
      /* length to zero, effectively truncating the   */
      /* file to zero.                                */
      wipe_out(fnp);
      status = S_REPLACED;
    }
    else if (flags & O_OPEN)
    {
      /* force r/o open for FCB if the file is read-only */
      if ((flags & O_FCB) && (dir_attrib & D_RDONLY))
        flags = (flags & ~3) | O_RDONLY;

      /* Check permissions. -- JPP
         (do not allow to open volume labels/directories,
          and do not allow writing to r/o files) */
      if ((dir_attrib & (D_DIR | D_VOLID)) ||
          ((dir_attrib & D_RDONLY) && ((flags & O_ACCMODE) != O_RDONLY)))
        return DE_ACCESS;
      status = S_OPENED;
    }
    else
    {
      return DE_FILEEXISTS;
    }
  }
  else if (status == DE_FILENOTFND && (flags & O_CREAT))
  {
    int ret = alloc_find_free(fnp, path);
    if (ret != SUCCESS)
      return ret;
    status = S_CREATED;
  }
  else
  {
    /* open: If we can't find the file, just return a not    */
    /* found error.                                          */
    return status;
  }

  /* Now change to file                                   */
  fnp->f_sft_idx = fd;
  fnp->f_offset = 0l;
  fnp->f_cluster_offset = 0;

  fnp->f_flags &= ~SFT_FDATE;
  /* use FCLEAN even on replaced/created files: the bit is reset */
  /* if the file is written to later                             */
  fnp->f_flags |= SFT_FCLEAN;
  if (status != S_OPENED)
  {
    init_direntry(&fnp->f_dir, attrib, FREE, fnp->f_dmp->dm_name_pat);
    if (!dir_write(fnp))
      return DE_ACCESS;
  }

  merge_file_changes(fnp, status == S_OPENED); /* /// Added - Ron Cemer */
  /* /// Moved from above.  - Ron Cemer */
  fnp->f_cluster = getdstart(fnp->f_dpb, &fnp->f_dir);

  fnode_to_sft(fnp);
  return status;
}

BOOL fcmp_wild(const char * s1, const char * s2, unsigned n)
{
  for ( ; n--; ++s1, ++s2)
    if (*s1 != '?' && *s1 != *s2)
      return FALSE;
  return TRUE;
}

COUNT dos_close(COUNT fd)
{
  /* Translate the fd into a useful pointer                       */
  f_node_ptr fnp = sft_to_fnode(fd);

  if (!(fnp->f_flags & SFT_FCLEAN))
  {
    if (!(fnp->f_flags & SFT_FDATE))
    {
      fnp->f_dir.dir_time = dos_gettime();
      fnp->f_dir.dir_date = dos_getdate();
    }

    merge_file_changes(fnp, FALSE);     /* /// Added - Ron Cemer */
  }
  fnp->f_sft_idx = 0xff;

  return dir_write_update(fnp, TRUE) ? SUCCESS : DE_INVLDHNDL;
}

/*                                                                      */
/* split a path into it's component directory and file name             */
/*                                                                      */
f_node_ptr split_path(const char * path, f_node_ptr fnp)
{
  /* check if the path ends in a backslash                        */
  if (path[strlen(path) - 1] == '\\')
    return (f_node_ptr) 0;

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
  if (get_cds(path[0]-'A')->cdsFlags & CDSNETWDRV)
  {
    printf("split path called for redirected file: `%s'\n", path);
    return (f_node_ptr) 0;
  }
#endif

  /* Translate the path into a useful pointer                     */
  return dir_open(path, TRUE, fnp);
}

/* checks whether directory part of path exists */
BOOL dir_exists(char * path)
{
  return split_path(path, &fnode[0]) != NULL;
}

BOOL fcbmatch(const char *fcbname1, const char *fcbname2)
{
  return memcmp(fcbname1, fcbname2, FNAME_SIZE + FEXT_SIZE) == 0;
}

STATIC int find_fname(const char *path, int attr, f_node_ptr fnp)
{
  /* check for leading backslash and open the directory given that */
  /* contains the file given by path.                              */
  if ((fnp = split_path(path, fnp)) == NULL)
    return DE_PATHNOTFND;

  while (dir_read(fnp) == 1)
  {
    if (fcbmatch(fnp->f_dir.dir_name, fnp->f_dmp->dm_name_pat)
        && (fnp->f_dir.dir_attrib & ~(D_RDONLY | D_ARCHIVE | attr)) == 0)
    {
      return SUCCESS;
    }
    fnp->f_dmp->dm_entry++;
  }
  return DE_FILENOTFND;
}

/* Description.
 *  Remove entries with D_LFN attribute preceeding the directory entry
 *  pointed by fnp, fnode isn't modified (I hope).
 * Return value. 
 *  SUCCESS     - completed successfully.
 *  DE_ACCESS   - error occurred, fnode is released.
 * input: fnp with valid non-LFN directory entry, not equal to '..' or
 *  '.'
 */
COUNT remove_lfn_entries(f_node_ptr fnp)
{
  unsigned original_diroff = fnp->f_dmp->dm_entry;

  while (TRUE)
  {
    if (fnp->f_dmp->dm_entry == 0)
      break;
    fnp->f_dmp->dm_entry--;
    if (dir_read(fnp) <= 0)
      return DE_ACCESS;
    if (fnp->f_dir.dir_attrib != D_LFN)
      break;
    fnp->f_dir.dir_name[0] = DELETED;
    if (!dir_write(fnp)) return DE_ACCESS;
  }
  fnp->f_dmp->dm_entry = original_diroff;
  if (dir_read(fnp) <= 0)
    return DE_ACCESS;

  return SUCCESS;
}

    /* /// Added - Ron Cemer */
    /* If more than one SFT has a file open, and a write
       occurs, this function must be called to propagate the
       results of that write to the other f_nodes which have
       that file open.  Note that this function only has an
       effect if SHARE is installed.  This is for compatibility
       reasons, since DOS without SHARE does not share changes
       between two or more open instances of the same file
       unless these instances were generated by dup() or dup2(). */
STATIC int merge_file_changes(f_node_ptr fnp, int collect)
{
  int i, j;
  sft FAR *sftp;
  sfttbl FAR *sp;

  if (!IsShareInstalled(FALSE))
    return SUCCESS;

  i = 0;
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    for(j = sp->sftt_count, sftp = sp->sftt_table; --j >= 0; sftp++, i++)
    {
      if (i != fnp->f_sft_idx && sftp->sft_count != 0
          && fnp->f_dpb == sftp->sft_dcb
          && (fnp->f_dir.dir_attrib & D_VOLID) == 0
          && (sftp->sft_attrib & D_VOLID) == 0
          && fnp->f_diridx == sftp->sft_diridx
          && fnp->f_dirsector == sftp->sft_dirsector
        ) /* same file, but different FD */
      {
        if (collect == -1)
        {
          /* set attrib: close open files */
          int rc = DosCloseSft(i, FALSE);
          if (rc != SUCCESS)
            return rc;
        }
        else if (collect)
        {
          /* We're collecting file changes from any other
             SFT which refers to this file. */
          if ((sftp->sft_mode & O_ACCMODE) != RDONLY)
          {
            setdstart(fnp->f_dpb, &fnp->f_dir, sftp->sft_stclust);
            fnp->f_dir.dir_size = sftp->sft_size;
            fnp->f_dir.dir_date = sftp->sft_date;
            fnp->f_dir.dir_time = sftp->sft_time;
            return SUCCESS;
          }
        }
        else
        {
          /* We just made changes to this file, so we are
             distributing these changes to the other f_nodes
             which refer to this file. */
          sftp->sft_stclust = getdstart(fnp->f_dpb, &fnp->f_dir);
          sftp->sft_size = fnp->f_dir.dir_size;
          sftp->sft_date = fnp->f_dir.dir_date;
          sftp->sft_time = fnp->f_dir.dir_time;
        }
      }
    }
  }
  return SUCCESS;
}

void dos_merge_file_changes(int fd)
{
  merge_file_changes(sft_to_fnode(fd), FALSE);
}

STATIC COUNT delete_dir_entry(f_node_ptr fnp)
{
  COUNT rc;

  /* Ok, so we can delete. Start out by           */
  /* clobbering all FAT entries for this file     */
  /* (or, in English, truncate the FAT).          */
  if ((rc = remove_lfn_entries(fnp)) < 0)
    return rc;

  wipe_out(fnp);
  *(fnp->f_dir.dir_name) = DELETED;

  /* The directory has been modified, so set the  */
  /* bit before closing it, allowing it to be     */
  /* updated                                      */
  if (!dir_write(fnp))
    return DE_ACCESS;

  /* SUCCESSful completion, return it             */
  return SUCCESS;
}

COUNT dos_delete(BYTE * path, int attrib)
{
  REG f_node_ptr fnp = &fnode[0];

  /* Check that we don't have a duplicate name, so if we  */
  /* find one, it's an error.                             */
  int ret = find_fname(path, attrib, fnp);
  if (ret == SUCCESS)
  {
    /* Do not delete directories or r/o files       */
    /* lfn entries and volume labels are only found */
    /* by find_fname() if attrib is set to a        */
    /* special value                                */	  
    if (fnp->f_dir.dir_attrib & (D_RDONLY | D_DIR))
      return DE_ACCESS;

    return delete_dir_entry(fnp);
  }
  else
    /* No such file, return the error               */
    return ret;
}

COUNT dos_rmdir(BYTE * path)
{
  REG f_node_ptr fnp;

  /* prevent removal of the current directory of that drive */
  register struct cds FAR *cdsp = get_cds(path[0] - 'A');
  if (!fstrcmp(path, cdsp->cdsCurrentPath))
    return DE_RMVCUDIR;

  /* Check that we're not trying to remove the root!      */
  if (path[2] == '\\' && path[3] == '\0')
    return DE_ACCESS;

  /* Check that the directory is empty. Only the  */
  /* "." and ".." are permissable.                */
  fnp = dir_open(path, FALSE, &fnode[0]);
  if (fnp == NULL)
    return DE_PATHNOTFND;

  /* Directories may have attributes, but if other than 'archive'      */
  /* or 'read only' then deny i.e. do not allow (SYSTEM|HIDDEN)        */
  /* directory to be deleted.                                          */
  if (fnp->f_dir.dir_attrib & ~(D_DIR | D_RDONLY | D_ARCHIVE))
    return DE_ACCESS;

  dir_read(fnp);
  /* 1st entry should be ".", else directory corrupt or not empty */
  if (fnp->f_dir.dir_name[0] != '.' || fnp->f_dir.dir_name[1] != ' ')
    return DE_ACCESS;

  fnp->f_dmp->dm_entry++;
  dir_read(fnp);
  /* second entry should be ".." */
  if (fnp->f_dir.dir_name[0] != '.' || fnp->f_dir.dir_name[1] != '.')
    return DE_ACCESS;

  /* Now search through the directory and make certain */
  /* that there are no entries                         */
  fnp->f_dmp->dm_entry++;
  while (dir_read(fnp) == 1)
  {
    /* If anything was found, exit with an error.   */
    if (fnp->f_dir.dir_name[0] != DELETED && fnp->f_dir.dir_attrib != D_LFN)
      return DE_ACCESS;
    fnp->f_dmp->dm_entry++;
  }

  /* next, split the passed dir into components (i.e. -   */
  /* path to new directory and name of new directory      */
  if (find_fname(path, D_ALL, fnp) != SUCCESS)
    /* this error should not happen because dir_open() succeeded above */
    return DE_PATHNOTFND;

  return delete_dir_entry(fnp);
}

COUNT dos_rename(BYTE * path1, BYTE * path2, int attrib)
{
  REG f_node_ptr fnp1;
  REG f_node_ptr fnp2;
  COUNT ret;
  char *fcbname;

  /* prevent renaming of the current directory of that drive */
  register struct cds FAR *cdsp = get_cds(path1[0] - 'A');
  if (!fstrcmp(path1, cdsp->cdsCurrentPath))
    return DE_RMVCUDIR;

  /* first check if the source file exists                        */
  fnp1 = &fnode[0];
  ret = find_fname(path1, attrib, fnp1);
  if (ret != SUCCESS)
    return ret;

  /* Check that we don't have a duplicate name, so if we find     */
  /* one, it's an error.                                          */
  fnp2 = &fnode[1];
  ret = find_fname(path2, attrib, fnp2);
  if (ret != DE_FILENOTFND)
    return ret == SUCCESS ? DE_ACCESS : ret;

  fcbname = fnp2->f_dmp->dm_name_pat;
  if (fnp1->f_dmp->dm_dircluster == fnp2->f_dmp->dm_dircluster)
  {
    /* rename in the same directory: change the directory entry in-place */
    fnp2 = fnp1;
    if ((ret = remove_lfn_entries(fnp1)) < 0)
      return ret;
  }
  else
  {
    /* do not allow to rename directories between different directories  */
    if (fnp1->f_dir.dir_attrib & D_DIR)
      return DE_ACCESS;

    /* create new entry in other directory */
    ret = alloc_find_free(fnp2, path2);
    if (ret != SUCCESS)
      return ret;

    if ((ret = remove_lfn_entries(fnp1)) < 0)
      return ret;

    /* init fnode for new file name to match old file name */
    memcpy(&fnp2->f_dir, &fnp1->f_dir, sizeof(struct dirent));

    /* Ok, so we can delete this one. Save the file info.           */
    *(fnp1->f_dir.dir_name) = DELETED;

    if (!dir_write(fnp1))
      return DE_ACCESS;
  }

  /* put the fnode's name into the directory.                     */
  memcpy(fnp2->f_dir.dir_name, fcbname, FNAME_SIZE + FEXT_SIZE);

  /* SUCCESSful completion, return it                             */
  return dir_write(fnp2) ? SUCCESS : DE_ACCESS;
}

/*                                                              */
/* wipe out all FAT entries starting from st for create, delete, etc. */
/*                                                              */
STATIC VOID wipe_out_clusters(struct dpb FAR * dpbp, CLUSTER st)
{
  REG CLUSTER next;

  /* Loop from start until either a FREE entry is         */
  /* encountered (due to a fractured file system) of the  */
  /* last cluster is encountered.                         */
  while (st != LONG_LAST_CLUSTER) /* remove clusters at start until empty */
  {
    /* get the next cluster pointed to              */
    next = next_cluster(dpbp, st);

    /* just exit if a damaged file system exists    */
    if (next <= 1)
      return;

    /* zap the FAT pointed to                       */
    if (link_fat(dpbp, st, FREE) != SUCCESS) /* nonfree->free */
      return; /* better abort on error */

    /* and the start of free space pointer          */
#ifdef WITHFAT32
    if (ISFAT32(dpbp))
    {
      if ((dpbp->dpb_xcluster == UNKNCLUSTER) || (dpbp->dpb_xcluster > st))
        dpbp->dpb_xcluster = st;
    }
    else
#endif
    if ((dpbp->dpb_cluster == UNKNCLUSTER) || (dpbp->dpb_cluster > (UWORD)st))
      dpbp->dpb_cluster = (UWORD)st;

    /* and just follow the linked list              */
    st = next;
  }
#ifdef WITHFAT32
  if (ISFAT32(dpbp))
    write_fsinfo(dpbp);
#endif
}

/* wipe out all FAT entries for create, delete, etc.            */
/* called by delete_dir_entry and dos_open open in O_TRUNC mode */
STATIC VOID wipe_out(f_node_ptr fnp)
{
  /* if not already free and valid file, do it */
  CLUSTER cluster = getdstart(fnp->f_dpb, &fnp->f_dir);
  if (cluster != FREE)
    wipe_out_clusters(fnp->f_dpb, cluster);
  /* no flushing here: could get lost chain or "crosslink seed" but */
  /* it would be annoying if mass-deletes could not use BUFFERS...  */
}

STATIC BOOL find_free(f_node_ptr fnp)
{
  COUNT rc;

  while ((rc = dir_read(fnp)) == 1)
  {
    if (fnp->f_dir.dir_name[0] == DELETED)
      return TRUE;
    fnp->f_dmp->dm_entry++;
  }
  return rc >= 0;
}

/* alloc_find_free: resets the directory                          */
/* Then finds a spare directory entry and if not                  */
/* available, tries to extend the directory.                      */
STATIC int alloc_find_free(f_node_ptr fnp, char *path)
{
  fnp = split_path(path, fnp);

  /* Get a free f_node pointer so that we can use */
  /* it in building the new file.                 */
  /* Note that if we're in the root and we don't  */
  /* find an empty slot, we need to abort.        */
  if (find_free(fnp) == 0)
  {
    if (fnp->f_dmp->dm_dircluster == 0)
    {
      return DE_TOOMANY;
    }
    else
    {
      /* Otherwise just expand the directory          */
      int ret;

      if ((ret = extend_dir(fnp)) != SUCCESS)
        /* fnp already closed in extend_dir */
        return ret;
    }
  }
  return SUCCESS;
}


/*                                                              */
/* dos_getdate for the file date                                */
/*                                                              */
date dos_getdate(void)
{
  struct dosdate dd;

  /* First - get the system date set by either the user   */
  /* on start-up or the CMOS clock                        */
  DosGetDate(&dd);
  return DT_ENCODE(dd.month, dd.monthday, dd.year - EPOCH_YEAR);
}

/*                                                              */
/* dos_gettime for the file time                                */
/*                                                              */
time dos_gettime(void)
{
  struct dostime dt;

  /* First - get the system time set by either the user   */
  /* on start-up or the CMOS clock                        */
  DosGetTime(&dt);
  return time_encode(&dt);
}

/*                                                              */
/* Find free cluster in disk FAT table                          */
/*                                                              */
STATIC CLUSTER find_fat_free(f_node_ptr fnp)
{
  REG CLUSTER idx, size, cluster;
  struct dpb FAR *dpbp = fnp->f_dpb;

#ifdef DISPLAY_GETBLOCK
  printf("[find_fat_free]\n");
#endif

  /* Start from optimized lookup point for start of FAT   */
  idx = 2;
  size = dpbp->dpb_size;

#ifdef WITHFAT32
  if (ISFAT32(dpbp))
  {
    if (dpbp->dpb_xcluster != UNKNCLUSTER)
      idx = dpbp->dpb_xcluster;
    size = dpbp->dpb_xsize;
  }
  else
#endif
  if (dpbp->dpb_cluster != UNKNCLUSTER)
    idx = dpbp->dpb_cluster;

  /* Search the FAT table looking for the first free      */
  /* entry.                                               */
  cluster = idx;
  for (;;)
  {
#ifdef CHECK_FAT_DURING_CLUSTER_ALLOC /* slower but nice side effect ;-) */
    if (next_cluster(dpbp, idx) == FREE)
#else
    if (is_free_cluster(dpbp, idx))
#endif
    {
      cluster = idx;
      break;
    }
    idx++;
    /* wrap the search just in case there are free clusters before */
    /* dpbp->dpb_(x)cluster (the fsinfo entry is just a hint!)     */
    if (idx > size) idx = 2;
    if (idx == cluster) {
      /* No empty clusters, disk is FULL!                     */
      cluster = UNKNCLUSTER;
      idx = LONG_LAST_CLUSTER;
      break;
    }
  }

#ifdef WITHFAT32
  if (ISFAT32(dpbp))
  {
    dpbp->dpb_xcluster = cluster;
    /* return the free entry                                */
    write_fsinfo(dpbp);
    return idx;
  }
#endif

  dpbp->dpb_cluster = (UWORD)cluster;
  /* return the free entry                                */
  return idx;
}

/* clear out the blocks in the cluster for a directory */
STATIC int clear_dir(f_node_ptr fnp, CLUSTER cluster)
{
  int idx;
  for (idx = 0; idx <= fnp->f_dpb->dpb_clsmask; idx++)
  {
    struct buffer FAR *bp;

    /* as we are overwriting it completely, don't read first */
    bp = getblockOver(clus2phys(cluster, fnp->f_dpb) + idx,
                      fnp->f_dpb->dpb_unit);
#ifdef DISPLAY_GETBLOCK
    printf("DIR (clear_dir)\n");
#endif
    if (bp == NULL)
      return DE_ACCESS;
    fmemset(bp->b_buffer, 0, BUFFERSIZE);
    bp->b_flag |= BFR_DIRTY | BFR_VALID;

    if (idx != 0)
      bp->b_flag |= BFR_UNCACHE;        /* needs not be cached */
  }
  return SUCCESS;
}

/*                                                              */
/* create a directory - returns success or a negative error     */
/* number                                                       */
/*                                                              */
COUNT dos_mkdir(BYTE * dir)
{
  REG f_node_ptr fnp;
  CLUSTER free_fat, parent;
  COUNT ret;

  /* check that the resulting combined path does not exceed
     the 67 MAX_CDSPATH limit. this leads to problems:
     A) you can't CD to this directory later
     B) you can't create files in this subdirectory
     C) the created dir will not be found later, so you
     can create an unlimited amount of same dirs. this space
     is lost forever
   */
  if (strlen(dir) >= MAX_CDSPATH)  /* dir is already output of "truename" */
    return DE_PATHNOTFND;

  /* Check that we don't have a duplicate name, so if we  */
  /* find one, it's an error.                             */
  fnp = &fnode[0];
  ret = find_fname(dir, D_ALL, fnp);
  if (ret != DE_FILENOTFND)
    return ret == SUCCESS ? DE_ACCESS : ret;

  parent = fnp->f_dmp->dm_dircluster;

  ret = alloc_find_free(fnp, dir);
  if (ret != SUCCESS)
    return ret;

  /* get an empty cluster, so that we make it into a      */
  /* directory.                                           */
  /* TE this has to be done (and failed) BEFORE the dir entry */
  /* is changed                                           */
  free_fat = find_fat_free(fnp);

  /* No empty clusters, disk is FULL! Translate into a    */
  /* useful error message.                                */
  if (free_fat == LONG_LAST_CLUSTER)
    return DE_HNDLDSKFULL;

  init_direntry(&fnp->f_dir, D_DIR, free_fat, fnp->f_dmp->dm_name_pat);

  /* Mark the cluster in the FAT as used and create new dir there */
  if (link_fat(fnp->f_dpb, free_fat, LONG_LAST_CLUSTER) != SUCCESS) /* free->last */
    return DE_HNDLDSKFULL; /* should never happen */

  /* clean out the new directory */
  ret = clear_dir(fnp, free_fat);
  if (ret != SUCCESS)
    return ret;

  /* Write the new directory entry                         */
  if (!dir_write(fnp))
    return DE_ACCESS;

  /* Craft the new directory. Note that if we're in a new  */
  /* directory just under the root, ".." pointer is 0.     */

  dir_init_fnode(fnp, free_fat);
  fnp->f_dmp->dm_entry = 0;
  find_free(fnp);

  /* Create the "." entry                                 */
  init_direntry(&fnp->f_dir, D_DIR, free_fat, ".          ");

  /* And put it out                                       */
  if (!dir_write(fnp))
    return DE_ACCESS;

  /* create the ".." entry                                */
  if (!find_free(fnp) && ((ret = extend_dir(fnp)) != SUCCESS))
    return ret;
#ifdef WITHFAT32
  if (ISFAT32(fnp->f_dpb) && parent == fnp->f_dpb->dpb_xrootclst)
  {
    parent = 0;
  }
#endif
  /* use . to allow the compiler to merge duplicate strings */
  init_direntry(&fnp->f_dir, D_DIR, parent, ".          ");
  fnp->f_dir.dir_name[1] = '.';

  /* and put it out                                       */
  if (!dir_write(fnp))
    return DE_ACCESS;

  return SUCCESS;
}

/* extend a directory or file by exactly one cluster */
/* only map_cluster calls this in a loop (for files) */
STATIC CLUSTER extend(f_node_ptr fnp)
{
  CLUSTER free_fat;

  /* get an empty cluster, so that we use it to extend the file.  */
  free_fat = find_fat_free(fnp);

  /* No empty clusters, disk is FULL! Translate into a useful     */
  /* error message.                                               */
  if (free_fat == LONG_LAST_CLUSTER)
    return free_fat;

  /* if 1a or 1b works but 2 fails, we get a pointer into an wrong FAT entry */
  /* our new fattab.c checks should be able to trap the bad pointers for now */
  if (link_fat(fnp->f_dpb, free_fat, LONG_LAST_CLUSTER) != SUCCESS) /* 2 */ /* free->last */
      return LONG_LAST_CLUSTER; /* do not try 1a/1b if 2 did not work out */
  /* if 2 works but 1a/1b fails, we only get a harmless lost cluster here */

  /* Now that we have found a free FAT entry, mark it as the last entry of */
  /* the chain and save (note: BUFFERS cause nondeterministic write order) */
  if (fnp->f_cluster == FREE) /* if the file leaves the empty state */
    setdstart(fnp->f_dpb, &fnp->f_dir, free_fat); /* 1a */
  else
  {
    /* let previously last chain element chain to newly allocated cluster! */
    if (next_cluster(fnp->f_dpb, fnp->f_cluster) != LONG_LAST_CLUSTER)
    {
      /* we tried to "grow a file in the middle", f_node or FAT messed up? */
      put_string("FAT chain size bad!\n");
      return LONG_LAST_CLUSTER;
    }
    if (link_fat(fnp->f_dpb, fnp->f_cluster, free_fat) != SUCCESS) /* 1b */ /* last->used */
      return LONG_LAST_CLUSTER; /* should never happen */
  }

  return free_fat;
}

STATIC COUNT extend_dir(f_node_ptr fnp)
{
  int ret;
  CLUSTER cluster = extend(fnp);
  if (cluster == LONG_LAST_CLUSTER)
    return DE_HNDLDSKFULL;

  ret = clear_dir(fnp, cluster);
  if (ret != SUCCESS)
    return ret;

  if (!find_free(fnp))
    return DE_HNDLDSKFULL;

  /* flush the drive buffers so that all info is written          */
  if (!flush_buffers(fnp->f_dpb->dpb_unit))
    return DE_ACCESS;

  return SUCCESS;

}

/* Description.
 *    Finds the cluster which contains byte at the fnp->f_offset offset and
 *  stores its number to the fnp->f_cluster. The search begins from the start of
 *  a file or a directory depending on whether the SFT index is valid
 *  and continues through the FAT chain until the target cluster is found.
 *  The mode can have only XFR_READ or XFR_WRITE values.
 *    In the XFR_WRITE mode map_cluster extends the FAT chain by creating
 *  new clusters upon necessity.
 * Return value.
 *  DE_HNDLDSKFULL - [XFR_WRITE mode only] unable to find free cluster
 *                   for extending the FAT chain, the disk is full.
 *                   The fnode is released from memory.
 *  DE_SEEK        - [XFR_READ mode only] byte at f_offset lies outside of
 *                   the FAT chain. The fnode is not released.
 * Notes.
 *  If we are moving forward, then use the relative cluster number offset
 *  that we are at now (f_cluster_offset) to start, instead of starting
 *  at the beginning. */

COUNT map_cluster(REG f_node_ptr fnp, COUNT mode)
{
  CLUSTER relcluster, cluster;

#ifdef DISPLAY_GETBLOCK
  printf("map_cluster: current %lu, offset %lu, diff=%lu ",
         (ULONG)fnp->f_cluster_offset, fnp->f_offset,
         fnp->f_offset - fnp->f_cluster_offset);
#endif

  if (fnp->f_cluster == FREE)
  {
    /* If this is a read but the file still has zero bytes return   */
    /* immediately....                                              */
    if (mode == XFR_READ)
      return DE_SEEK;

    /* If someone did a seek, but no writes have occured, we will   */
    /* need to initialize the fnode.                                */
    /*  (mode == XFR_WRITE) */
    /* If there are no more free fat entries, then we are full! */
    cluster = extend(fnp);
    if (cluster == LONG_LAST_CLUSTER)
    {
      return DE_HNDLDSKFULL;
    }
    fnp->f_cluster = cluster;
  }

  relcluster = (CLUSTER)((fnp->f_offset / fnp->f_dpb->dpb_secsize) >>
                         fnp->f_dpb->dpb_shftcnt);
  if (relcluster < fnp->f_cluster_offset)
  {
    /* If seek is to earlier in file than current position, */
    /* we have to follow chain from the beginning again...  */
    /* Set internal index and cluster size.                 */
    fnp->f_cluster = fnp->f_sft_idx == 0xff ? fnp->f_dmp->dm_dircluster :
        getdstart(fnp->f_dpb, &fnp->f_dir);
    fnp->f_cluster_offset = 0;
  }

  /* Now begin the linear search. The relative cluster is         */
  /* maintained as part of the set of physical indices. It is     */
  /* also the highest order index and is mapped directly into     */
  /* physical cluster. Our search is performed by pacing an index */
  /* up to the relative cluster position where the index falls    */
  /* within the cluster.                                          */

  while (fnp->f_cluster_offset != relcluster)
  {
    /* get next cluster in the chain */
    cluster = next_cluster(fnp->f_dpb, fnp->f_cluster);
    if (cluster <= 1) /* 1/error or 0/FREE chain into the void */
      return DE_SEEK;

    /* If this is a read and the next is a LAST_CLUSTER,               */
    /* then we are going to read past EOF, return zero read            */
    /* or expand the list if we're going to write and have run into    */
    /* the last cluster marker.                                        */
    if (cluster == LONG_LAST_CLUSTER)
    {
      if (mode == XFR_READ)
        return DE_SEEK;

      /* mode == XFR_WRITE */
      cluster = extend(fnp);
      if (cluster == LONG_LAST_CLUSTER)
        return DE_HNDLDSKFULL;
    }

    fnp->f_cluster = cluster;
    fnp->f_cluster_offset++;
  }

#ifdef DISPLAY_GETBLOCK
  printf("done.\n");
#endif

  return SUCCESS;
}

/* extends a file from f_dir.dir_size to f_offset              */
/* Proper OSes write zeros in between, but DOS just adds       */
/* garbage sectors, and lets the caller do the zero filling    */
/* if you prefer you can have this enabled using               */
/* #define WRITEZEROS 1                                        */
/* but because we want to be compatible, we don't do this by   */
/* default                                                     */
STATIC COUNT dos_extend(f_node_ptr fnp)
{
#ifdef WRITEZEROS
  struct buffer FAR *bp;
  UCOUNT xfr_cnt = 0;
  /* The variable secsize will be used later.                     */
  UWORD secsize = fnp->f_dpb->dpb_secsize;
  ULONG count;
  unsigned sector, boff;
#endif

  if (fnp->f_offset <= fnp->f_dir.dir_size)
    return SUCCESS;

#ifdef WRITEZEROS
  count = fnp->f_offset - fnp->f_dir.dir_size;
  fnp->f_offset = fnp->f_dir.dir_size;
  while (count > 0)
#endif
  {
    if (map_cluster(fnp, XFR_WRITE) != SUCCESS)
      return DE_HNDLDSKFULL;

#ifdef WRITEZEROS
    /* Compute the block within the cluster and the offset  */
    /* within the block.                                    */
    sector = (UBYTE)(fnp->f_offset / secsize) & fnp->f_dpb->dpb_clsmask;
    boff = (UWORD)(fnp->f_offset % secsize);

#ifdef DSK_DEBUG
    printf("write %d links; dir offset %ld, cluster %d\n",
           fnp->f_count, fnp->f_dmp->dm_entry, fnp->f_cluster);
#endif

    xfr_cnt = count < (ULONG) secsize - boff ?
        (UWORD) count : secsize - boff;

    /* get a buffer to store the block in */
    if ((boff == 0) && (xfr_cnt == secsize))
    {
      bp = getblockOver(clus2phys(fnp->f_cluster, fnp->f_dpb) +
                        sector, fnp->f_dpb->dpb_unit);

    }
    else
    {
      bp = getblock(clus2phys(fnp->f_cluster, fnp->f_dpb) + sector,
                    fnp->f_dpb->dpb_unit);
    }
    if (bp == NULL)
    {
      return DE_ACCESS;
    }

    /* set a block to zero                                  */
    fmemset((BYTE FAR *) & bp->b_buffer[boff], 0, xfr_cnt);
    bp->b_flag |= BFR_DIRTY | BFR_VALID;

    if (xfr_cnt == sizeof(bp->b_buffer))        /* probably not used later */
    {
      bp->b_flag |= BFR_UNCACHE;
    }

    /* update pointers and counters                         */
    count -= xfr_cnt;
    fnp->f_offset += xfr_cnt;
#endif
    fnp->f_dir.dir_size = fnp->f_offset;
    merge_file_changes(fnp, FALSE);     /* /// Added - Ron Cemer */
  }
  return SUCCESS;
}

/*
  comments read optimization for large reads: read total clusters in one piece

  running a program like 
  
  while (1) {
    read(fd, header, sizeof(header));   // small read 
    read(fd, buffer, header.size);      // where size is large, up to 63K 
                                        // with average ~32K
    }                                        

    FreeDOS 2025 is really slow. 
    on a P200 with modern 30GB harddisk, doing above for a 14.5 MB file
    
    MSDOS 6.22 clustersize 8K  ~2.5 sec (accumulates over clusters, reads for 63 sectors seen),
    IBM PCDOS 7.0          8K  ~4.3 
    IBM PCDOS 7.0          16K ~2.8 
    FreeDOS ke2025             ~17.5

    with the read optimization (ke2025a),    
    
        clustersize 8K  ~6.5 sec
        clustersize 16K ~4.2 sec
        
    it was verified with IBM feature tool,
    that the drive read ahead cache (says it) is on. still this huge difference ;-)
        

    it's coded pretty conservative to avoid all special cases, 
    so it shouldn't break anything :-)

    possible further optimization:
    
        collect read across clusters (if file is not fragmented).
        MSDOS does this (as readcounts up to 63 sectors where seen)
        specially important for diskettes, where clustersize is 1 sector 
        
        the same should be done for writes as well

    the time to compile the complete kernel (on some P200) is 
    reduced from 67 to 56 seconds - in an otherwise identical configuration.

    it's not clear if this improvement shows up elsewhere, but it shouldn't harm either
    

    TE 10/18/01 14:00
    
    collect read across clusters (if file is not fragmented) done.
        
    seems still to work :-))
    
    no large performance gains visible, but should now work _much_
    better for the people, that complain about slow floppy access

    the 
        fnp->f_offset +to_xfer < fnp->f_dir.dir_size &&  avoid EOF problems 

    condition can probably _carefully_ be dropped    
    
    
    TE 10/18/01 19:00
    
*/

/* Read/write block from disk */
/* checking for valid access was already done by the functions in
   dosfns.c */
long rwblock(COUNT fd, VOID FAR * buffer, UCOUNT count, int mode)
{
  /* Translate the fd into an fnode pointer, since all internal   */
  /* operations are achieved through fnodes.                      */
  REG f_node_ptr fnp = sft_to_fnode(fd);
  REG struct buffer FAR *bp;
  UCOUNT xfr_cnt = 0;
  UCOUNT ret_cnt = 0;
  unsigned secsize;
  unsigned to_xfer = count;
  ULONG currentblock;

#if 0 /*DSK_DEBUG*/
  if (bDumpRdWrParms)
  {
    printf("rwblock:fd %02x  buffer %04x:%04x count %x\n",
           fd, FP_SEG(buffer), FP_OFF(buffer), count);
  }
#endif

  if (mode==XFR_WRITE)
  {
    fnp->f_dir.dir_attrib |= D_ARCHIVE;
    /* mark file as modified and set date not valid any more */
    fnp->f_flags &= ~(SFT_FCLEAN|SFT_FDATE); 
    
    if (dos_extend(fnp) != SUCCESS)
    {
      fnode_to_sft(fnp);
      return 0;
    }
  }
  
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
    /* Now done in dos_extend                                      BO */
    /* remove all the following allocated clusters in shrink_file     */
    if (mode == XFR_WRITE)
    {
      fnp->f_dir.dir_size = fnp->f_offset;
      if (shrink_file(fnp) < 0) /* this is the only call to shrink_file... */
        return DE_ACCESS;
      /* why does empty write -always- truncate to current offset? */
    }
    fnode_to_sft(fnp);
    return 0;
  }

  /* The variable secsize will be used later.                     */
  secsize = fnp->f_dpb->dpb_secsize;

  /* Adjust the far pointer from user space to supervisor space   */
  buffer = adjust_far(buffer);

  /* Do the data transfer. Use block transfer methods so that we  */
  /* can utilize memory management in future DOS-C versions.      */
  while (ret_cnt < count)
  {
    unsigned sector, boff;

    /* Do an EOF test and return whatever was transferred   */
    if (mode == XFR_READ && fnp->f_offset >= fnp->f_dir.dir_size)
    {
      fnode_to_sft(fnp);
      return ret_cnt;
    }

    /* Position the file to the fnode's pointer position. This is   */
    /* done by updating the fnode's cluster, block (sector) and     */
    /* byte offset so that read or write becomes a simple data move */
    /* into or out of the block data buffer.                        */

    /* The more difficult scenario is the (more common)     */
    /* file offset case. Here, we need to take the fnode's  */
    /* offset pointer (f_offset) and translate it into a    */
    /* relative cluster position, cluster block (sector)    */
    /* offset (sector) and byte offset (boff). Once we      */
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
#ifdef DISPLAY_GETBLOCK
    printf("rwblock: ");
#endif
    if (map_cluster(fnp, mode) != SUCCESS)
    {
      fnode_to_sft(fnp);
      return ret_cnt;
    }
    if (mode == XFR_WRITE)
    {
      merge_file_changes(fnp, FALSE);   /* /// Added - Ron Cemer */
    }

    /* Compute the block within the cluster and the offset  */
    /* within the block.                                    */
    sector = (UBYTE)(fnp->f_offset / secsize) & fnp->f_dpb->dpb_clsmask;
    boff = (UWORD)(fnp->f_offset % secsize);

    currentblock = clus2phys(fnp->f_cluster, fnp->f_dpb) + sector;

    /* see comments above */

    if (boff == 0)              /* complete sectors only */
    {
      static ULONG startoffset;
      UCOUNT sectors_to_xfer, sectors_wanted;

      startoffset = fnp->f_offset;
      sectors_wanted = to_xfer;

      /* avoid EOF problems */
      if (mode == XFR_READ && to_xfer > fnp->f_dir.dir_size - fnp->f_offset)
        sectors_wanted = (UCOUNT)(fnp->f_dir.dir_size - fnp->f_offset);
      
      sectors_wanted /= secsize;

      if (sectors_wanted == 0)
        goto normal_xfer;

      sectors_to_xfer = fnp->f_dpb->dpb_clsmask + 1 - sector;

      sectors_to_xfer = min(sectors_to_xfer, sectors_wanted);

      fnp->f_offset += sectors_to_xfer * secsize;

      while (sectors_to_xfer < sectors_wanted)
      {
        if (map_cluster(fnp, mode) != SUCCESS)
          break;

        if (clus2phys(fnp->f_cluster, fnp->f_dpb) !=
            currentblock + sectors_to_xfer)
          break;

        sectors_to_xfer += fnp->f_dpb->dpb_clsmask + 1;

        sectors_to_xfer = min(sectors_to_xfer, sectors_wanted);

        fnp->f_offset = startoffset + sectors_to_xfer * secsize;

      }

      xfr_cnt = sectors_to_xfer * secsize;

      /* avoid caching trouble */

      DeleteBlockInBufferCache(currentblock,
                               currentblock + sectors_to_xfer - 1,
                               fnp->f_dpb->dpb_unit, mode);

      if (dskxfer(fnp->f_dpb->dpb_unit,
                  currentblock,
                  (VOID FAR *) buffer, sectors_to_xfer,
                  mode == XFR_READ ? DSKREAD : DSKWRITE))
      {
        fnp->f_offset = startoffset;
        fnode_to_sft(fnp);
        return DE_ACCESS;
      }

      goto update_pointers;
    }

    /* normal read: just the old, buffer = sector based read */
  normal_xfer:

#ifdef DSK_DEBUG
    printf("r/w %d links; dir offset %d, cluster %d, mode %x\n",
           fnp->f_count, fnp->f_dmp->dm_entry, fnp->f_cluster, mode);
#endif

    /* Get the block we need from cache                     */
    bp = getblock(currentblock
                    /*clus2phys(fnp->f_cluster, fnp->f_dpb) + fnp->f_sector */
                    , fnp->f_dpb->dpb_unit);
    
#ifdef DISPLAY_GETBLOCK
    printf("DATA (rwblock)\n");
#endif
    if (bp == NULL)             /* (struct buffer *)0 --> DS:0 !! */
    {
      fnode_to_sft(fnp);
      return ret_cnt;
    }

    /* transfer a block                                     */
    /* Transfer size as either a full block size, or the    */
    /* requested transfer size, whichever is smaller.       */
    /* Then compare to what is left, since we can transfer  */
    /* a maximum of what is left.                           */
    xfr_cnt = min(to_xfer, secsize - boff);
    if (mode == XFR_READ)
      xfr_cnt = (UWORD) min(xfr_cnt, fnp->f_dir.dir_size - fnp->f_offset);

    /* transfer a block                                     */
    /* Transfer size as either a full block size, or the    */
    /* requested transfer size, whichever is smaller.       */
    /* Then compare to what is left, since we can transfer  */
    /* a maximum of what is left.                           */
    if (mode == XFR_WRITE)
    {
      fmemcpy(&bp->b_buffer[boff], buffer, xfr_cnt);
      bp->b_flag |= BFR_DIRTY | BFR_VALID;
    }
    else
    {
      fmemcpy(buffer, &bp->b_buffer[boff], xfr_cnt);
    }

    /* complete buffer transferred ? 
       probably not reused later
     */
    if (xfr_cnt == sizeof(bp->b_buffer) ||
        (mode == XFR_READ && fnp->f_offset + xfr_cnt == fnp->f_dir.dir_size))
    {
      bp->b_flag |= BFR_UNCACHE;
    }

    /* update pointers and counters                         */
    fnp->f_offset += xfr_cnt;

  update_pointers:
    ret_cnt += xfr_cnt;
    to_xfer -= xfr_cnt;
    buffer = adjust_far((char FAR *)buffer + xfr_cnt);
    if (mode == XFR_WRITE)
    {
      if (fnp->f_offset > fnp->f_dir.dir_size)
      {
        fnp->f_dir.dir_size = fnp->f_offset;
      }
      merge_file_changes(fnp, FALSE);     /* /// Added - Ron Cemer */
    }
  }
  fnode_to_sft(fnp);
  return ret_cnt;
}

/* returns the number of unused clusters */
CLUSTER dos_free(struct dpb FAR * dpbp)
{
  /* There's an unwritten rule here. All fs       */
  /* cluster start at 2 and run to max_cluster+2  */
  REG CLUSTER i;
  REG CLUSTER cnt;
  CLUSTER max_cluster = dpbp->dpb_size;

#ifdef WITHFAT32
  if (ISFAT32(dpbp))
  {
    if (dpbp->dpb_xnfreeclst != XUNKNCLSTFREE)
      return dpbp->dpb_xnfreeclst;
    max_cluster = dpbp->dpb_xsize;
  }
  else
#endif
  if (dpbp->dpb_nfreeclst != UNKNCLSTFREE)
    return dpbp->dpb_nfreeclst;

  cnt = 0;
  for (i = 2; i <= max_cluster; i++)
  {
#ifdef CHECK_FAT_DURING_SPACE_CHECK /* slower but nice side effect ;-) */
    if (next_cluster(dpbp, i) == FREE)
#else
    if (is_free_cluster(dpbp, i))
#endif
    {
      if (cnt == 0)
      {
        /* update first free cluster number */
#ifdef WITHFAT32
        if (ISFAT32(dpbp))
          dpbp->dpb_xcluster = i;
        else
#endif
          dpbp->dpb_cluster = (UWORD)i;
      }
      ++cnt;
    }
  }
#ifdef WITHFAT32
  if (ISFAT32(dpbp))
  {
    dpbp->dpb_xnfreeclst = cnt;
    write_fsinfo(dpbp);
    return cnt;
  }
#endif
  dpbp->dpb_nfreeclst = (UWORD)cnt;
  return cnt;
}

#ifndef IPL
int dos_cd(char * PathName)
{
  f_node_ptr fnp;
  struct cds FAR *cdsp;

  /* now test for its existance. If it doesn't, return an error.  */
  if ((fnp = dir_open(PathName, FALSE, &fnode[0])) == NULL)
    return DE_PATHNOTFND;

  /* problem: RBIL table 01643 does not give a FAT32 field for the
     CDS start cluster. But we are not using this field ourselves */
  cdsp = get_cds(PathName[0] - 'A');
  cdsp->cdsStrtClst = (UWORD)fnp->f_dmp->dm_dircluster;
  return SUCCESS;
}
#endif

#ifndef IPL
COUNT dos_getfattr(BYTE * name)
{
  f_node_ptr fnp = &fnode[0];
  int ret = find_fname(name, D_ALL, fnp);
  return ret == SUCCESS ? fnp->f_dir.dir_attrib : ret;
}

COUNT dos_setfattr(BYTE * name, UWORD attrp)
{
  f_node_ptr fnp;
  int rc;

  /* JPP-If user tries to set VOLID or RESERVED bits, return error.
     We used to also check for D_DIR here, but causes issues with deltree
     which is trying to work around another issue.  So now we check
     these here, and only report DE_ACCESS if user tries to set directory
     bit on a non-directory entry.
   */
  if ((attrp & (D_VOLID | 0xC0)) != 0)
    return DE_ACCESS;

  fnp = &fnode[0];
  rc = find_fname(name, D_ALL, fnp);
  if (rc != SUCCESS)
    return rc;

  /* if caller tries to set DIR on non-directory, return error */
  if ((attrp & D_DIR) && !(fnp->f_dir.dir_attrib & D_DIR))
    return DE_ACCESS;

  /* Set the attribute from the fnode and return          */
  /* clear all attributes but DIR and VOLID */
  fnp->f_dir.dir_attrib &= (D_VOLID | D_DIR);   /* JPP */

  /* set attributes that user requested */
  fnp->f_dir.dir_attrib |= attrp;       /* JPP */

  /* close open files in compat mode, otherwise there was a critical error */
  rc = merge_file_changes(fnp, -1);
  if (rc == SUCCESS && !dir_write(fnp))
    rc = DE_ACCESS;
  return rc;
}
#endif

#ifdef WITHFAT32
VOID dpb16to32(struct dpb FAR *dpbp)
{
  dpbp->dpb_xflags = 0;
  dpbp->dpb_xfsinfosec = 0xffff;
  dpbp->dpb_xbackupsec = 0xffff;
  dpbp->dpb_xrootclst = 0;
  dpbp->dpb_xdata = dpbp->dpb_data;
  dpbp->dpb_xsize = dpbp->dpb_size;
}

VOID bpb_to_dpb(bpb FAR * bpbp, REG struct dpb FAR * dpbp, BOOL extended)
#else
VOID bpb_to_dpb(bpb FAR * bpbp, REG struct dpb FAR * dpbp)
#endif
{
  ULONG size;
  REG UWORD shftcnt;
  bpb sbpb;

  fmemcpy(&sbpb, bpbp, sizeof(sbpb));
  for (shftcnt = 0; (sbpb.bpb_nsector >> shftcnt) > 1; shftcnt++)
    ;
  dpbp->dpb_shftcnt = shftcnt;

  dpbp->dpb_mdb = sbpb.bpb_mdesc;
  dpbp->dpb_secsize = sbpb.bpb_nbyte;
  dpbp->dpb_clsmask = sbpb.bpb_nsector - 1;
  dpbp->dpb_fatstrt = sbpb.bpb_nreserved;
  dpbp->dpb_fats = sbpb.bpb_nfat;
  dpbp->dpb_dirents = sbpb.bpb_ndirent;
  size = sbpb.bpb_nsize == 0 ? sbpb.bpb_huge : (ULONG) sbpb.bpb_nsize;
  dpbp->dpb_fatsize = sbpb.bpb_nfsect;
  dpbp->dpb_dirstrt = dpbp->dpb_fatstrt + dpbp->dpb_fats * dpbp->dpb_fatsize;
  dpbp->dpb_data = dpbp->dpb_dirstrt
      + (dpbp->dpb_dirents + dpbp->dpb_secsize/DIRENT_SIZE - 1) /
          (dpbp->dpb_secsize/DIRENT_SIZE);
  dpbp->dpb_size = (UWORD)((size - dpbp->dpb_data) >> shftcnt) + 1;
  { /* Make sure the number of FAT sectors is actually enough to hold that */
    /* many clusters. Otherwise back the number of clusters down (LG & AB) */
    unsigned fatsiz;
    ULONG tmp = dpbp->dpb_fatsize * (ULONG)(dpbp->dpb_secsize / 2);/* entries/2 */
    if (tmp >= 0x10000UL)
      goto ckok;
    fatsiz = (unsigned) tmp;
    if (dpbp->dpb_size > FAT_MAGIC) {/* FAT16 */
      if (fatsiz <= FAT_MAGIC)       /* FAT12 - let it pass through rather */
        goto ckok;                   /* than lose data correcting FAT type */
    } else {                         /* FAT12 */
      if (fatsiz >= 0x4000)
        goto ckok;
      fatsiz = fatsiz * 4 / 3;
    }
    if (dpbp->dpb_size >= fatsiz)    /* FAT too short */
      dpbp->dpb_size = fatsiz - 1;   /* - 2 reserved entries + 1 */
ckok:;
  }
  dpbp->dpb_flags = 0;
  dpbp->dpb_cluster = UNKNCLUSTER;
  /* number of free clusters */
  dpbp->dpb_nfreeclst = UNKNCLSTFREE;

#ifdef WITHFAT32
  if (extended)
  {
    dpbp->dpb_xfatsize = sbpb.bpb_nfsect == 0 ? sbpb.bpb_xnfsect
        : sbpb.bpb_nfsect;
    dpbp->dpb_xcluster = UNKNCLUSTER;
    dpbp->dpb_xnfreeclst = XUNKNCLSTFREE;       /* number of free clusters */

    dpb16to32(dpbp);

    if (ISFAT32(dpbp))
    {
      dpbp->dpb_xflags = sbpb.bpb_xflags;
      dpbp->dpb_xfsinfosec = sbpb.bpb_xfsinfosec;
      dpbp->dpb_xbackupsec = sbpb.bpb_xbackupsec;
      dpbp->dpb_dirents = 0;
      dpbp->dpb_dirstrt = 0xffff;
      dpbp->dpb_size = 0;
      dpbp->dpb_xdata =
          dpbp->dpb_fatstrt + dpbp->dpb_fats * dpbp->dpb_xfatsize;
      dpbp->dpb_xsize = ((size - dpbp->dpb_xdata) >> shftcnt) + 1;
      dpbp->dpb_xrootclst = sbpb.bpb_xrootclst;
      read_fsinfo(dpbp);
    }
  }
#endif
}

STATIC int rqblockio(unsigned char command, struct dpb FAR * dpbp)
{
 retry:
  MediaReqHdr.r_length = sizeof(request);
  MediaReqHdr.r_unit = dpbp->dpb_subunit;
  MediaReqHdr.r_command = command;
  MediaReqHdr.r_mcmdesc = dpbp->dpb_mdb;
  MediaReqHdr.r_status = 0;

  if (command == C_BLDBPB) /* help USBASPI.SYS & DI1000DD.SYS (TE) */
    MediaReqHdr.r_bpfat = (boot FAR *)DiskTransferBuffer;
  execrh((request FAR *) & MediaReqHdr, dpbp->dpb_device);
  if ((MediaReqHdr.r_status & S_ERROR) || !(MediaReqHdr.r_status & S_DONE))
  {
    FOREVER
    {
      switch (block_error(&MediaReqHdr, dpbp->dpb_unit, dpbp->dpb_device, 0))
      {
      case ABORT:
      case FAIL:
        return DE_INVLDDRV;

      case RETRY:
        goto retry;

      case CONTINUE:
        return SUCCESS;
      }
    }
  }
  return SUCCESS;
}

COUNT media_check(REG struct dpb FAR * dpbp)
{
  int ret;
  if (dpbp == NULL)
    return DE_INVLDDRV;

  /* First test if anyone has changed the removable media         */
  ret = rqblockio(C_MEDIACHK, dpbp);
  if (ret < SUCCESS)
    return ret;

  switch (MediaReqHdr.r_mcretcode | dpbp->dpb_flags)
  {
    case M_NOT_CHANGED:
      /* It was definitely not changed, so ignore it          */
      return SUCCESS;

      /* If it is forced or the media may have changed,       */
      /* rebuild the bpb                                      */
    case M_DONT_KNOW:
      /* IBM PCDOS technical reference says to call BLDBPB if */
      /* there are no used buffers                            */
      if (dirty_buffers(dpbp->dpb_unit))
        return SUCCESS;

      /* If it definitely changed, don't know (falls through) */
      /* or has been changed, rebuild the bpb.                */
    /* case M_CHANGED: */
    default:
      setinvld(dpbp->dpb_unit);
      ret = rqblockio(C_BLDBPB, dpbp);
      if (ret < SUCCESS)
        return ret;
#ifdef WITHFAT32
      /* extend dpb only for internal or FAT32 devices */
      bpb_to_dpb(MediaReqHdr.r_bpptr, dpbp,
                 MediaReqHdr.r_bpptr->bpb_nfsect == 0 ||
                 FP_SEG(dpbp) == FP_SEG(&os_major));
#else
      bpb_to_dpb(MediaReqHdr.r_bpptr, dpbp);
#endif
      return SUCCESS;
  }
}

/* copy the SFT fd into the first near fnode */
STATIC f_node_ptr sft_to_fnode(int fd)
{
  sft FAR *sftp = idx_to_sft(fd);
  f_node_ptr fnp = &fnode[0];

  fnp->f_sft_idx = fd;

  fnp->f_flags = sftp->sft_flags;

  fnp->f_dir.dir_attrib = sftp->sft_attrib;
  fmemcpy(fnp->f_dir.dir_name, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
  fnp->f_dir.dir_time = sftp->sft_time;
  fnp->f_dir.dir_date = sftp->sft_date;
  fnp->f_dir.dir_size = sftp->sft_size;
  fnp->f_dpb = sftp->sft_dcb;
  setdstart(fnp->f_dpb, &fnp->f_dir, sftp->sft_stclust);

  fnp->f_diridx = sftp->sft_diridx;
  fnp->f_dirsector = sftp->sft_dirsector;
  fnp->f_offset = sftp->sft_posit;
  fnp->f_cluster = sftp->sft_cuclust;
#ifdef WITHFAT32
  fnp->f_cluster_offset = sftp->sft_relclust |
    ((ULONG)sftp->sft_relclust_high << 16);
#else
  fnp->f_cluster_offset = sftp->sft_relclust;
#endif
  return fnp;
}

STATIC void fnode_to_sft(f_node_ptr fnp)
{
  sft FAR *sftp = idx_to_sft(fnp->f_sft_idx);

  sftp->sft_flags = fnp->f_flags;

  sftp->sft_attrib = fnp->f_dir.dir_attrib;
  fmemcpy(sftp->sft_name, fnp->f_dir.dir_name, FNAME_SIZE + FEXT_SIZE);
  sftp->sft_time = fnp->f_dir.dir_time;
  sftp->sft_date = fnp->f_dir.dir_date;
  sftp->sft_size = fnp->f_dir.dir_size;
  sftp->sft_stclust = getdstart(fnp->f_dpb, &fnp->f_dir);

  sftp->sft_diridx = fnp->f_diridx;
  sftp->sft_dirsector = fnp->f_dirsector;
  sftp->sft_dcb = fnp->f_dpb;
  sftp->sft_posit = fnp->f_offset;
  sftp->sft_cuclust = fnp->f_cluster;
  sftp->sft_relclust = (UWORD)fnp->f_cluster_offset;
#ifdef WITHFAT32
  sftp->sft_relclust_high = (UWORD)(fnp->f_cluster_offset >> 16);
#endif
}

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
    Apps tested (VB ISAM); BO: confirmation???
*/

STATIC int shrink_file(f_node_ptr fnp)
{

  ULONG lastoffset = fnp->f_offset;     /* has to be saved */
  CLUSTER last, next, st;
  struct dpb FAR *dpbp = fnp->f_dpb;
  int ret = DE_ACCESS;

  if (fnp->f_offset)
    fnp->f_offset--;            /* last existing cluster */
  else if (fnp->f_cluster == FREE)
    /* zero offset, 0-byte file: nothing to do ! */
    goto done_success;

  if (map_cluster(fnp, XFR_READ) != SUCCESS)    /* error, don't truncate */
    goto done;

  st = fnp->f_cluster;

  next = next_cluster(dpbp, st); /* return nr. of 1st cluster after new end */

  if (next <= 1) /* 1/error or 0/FREE chain points into the void */
    goto done;

  /* Loop from start until either a FREE entry is         */
  /* encountered (due to a damaged file system) or the    */
  /* last cluster is encountered.                         */
  /* zap the FAT pointed to                       */

  if (fnp->f_dir.dir_size == 0) /* file shrinks to size 0 */
  {
    fnp->f_cluster = FREE;
    setdstart(dpbp, &fnp->f_dir, FREE); /* file no longer has start cluster */
    last = FREE;
  }
  else
  {
    if (next == LONG_LAST_CLUSTER) /* nothing to do, file already ends here */
      goto done_success;
    last = LONG_LAST_CLUSTER; /* make file end */
  }
  if (link_fat(dpbp, st, last) != SUCCESS)
    goto done; /* do not wipe remainder of chain if FAT is broken */

  wipe_out_clusters(dpbp, next); /* free clusters after the end */
  /* flush buffers, make sure disk is updated */
  if (!flush_buffers(fnp->f_dpb->dpb_unit))
    goto done;

done_success:
  ret = SUCCESS;

done:
  fnp->f_offset = lastoffset;   /* has to be restored */
  return ret;
}

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
 */
