/****************************************************************/
/*                                                              */
/*                          fatdir.c                            */
/*                            DOS-C                             */
/*                                                              */
/*                 FAT File System dir Functions                */
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
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *fatdirRcsId =
    "$Id$";
#endif

/* Description.
 *  Initialize a fnode so that it will point to the directory with 
 *  dirstart starting cluster; in case of passing dirstart == 0
 *  fnode will point to the start of a root directory */
VOID dir_init_fnode(f_node_ptr fnp, CLUSTER dirstart)
{
  /* reset the directory flags    */
  fnp->f_sft_idx = 0xff;
  fnp->f_dmp = &sda_tmp_dm;
  if (fnp == &fnode[1])
    fnp->f_dmp = &sda_tmp_dm_ren;
  fnp->f_offset = 0l;
  fnp->f_cluster_offset = 0;

  /* root directory */
#ifdef WITHFAT32
  if (dirstart == 0)
    if (ISFAT32(fnp->f_dpb))
      dirstart = fnp->f_dpb->dpb_xrootclst;
#endif
  fnp->f_cluster = fnp->f_dmp->dm_dircluster = dirstart;
}

f_node_ptr dir_open(register const char *dirname, BOOL split, f_node_ptr fnp)
{
  int i;
  char *fcbname;

  /* determine what drive and dpb we are using...                 */
  fnp->f_dpb = get_dpb(dirname[0]-'A');
  /* Perform all directory common handling after all special      */
  /* handling has been performed.                                 */

  /* truename() already did a media check()                       */

  /* Walk the directory tree to find the starting cluster         */
  /*                                                              */
  /* Start from the root directory (dirstart = 0)                 */

  /* The CDS's cdsStartCls may be used to shorten the search
     beginning at the CWD, see mapPath() and CDS.H in order
     to enable this behaviour there.
           -- 2001/09/04 ska*/

  dir_init_fnode(fnp, 0);
  fnp->f_dmp->dm_entry = 0;

  dirname += 2;               /* Assume FAT style drive       */
  fcbname = fnp->f_dmp->dm_name_pat;
  while(*dirname != '\0')
  {
    /* skip the path seperator                              */
    ++dirname;

    /* don't continue if we're at the end: this check is    */
    /* for root directories, the only fully-qualified path  */
    /* names that end in a \                                */
    if (*dirname == '\0')
      break;

    /* Convert the name into an absolute name for           */
    /* comparison...                                        */

    dirname = ConvertNameSZToName83(fcbname, dirname);

    /* do not continue if we split the filename off and are */
    /* at the end                                           */
    if (split && *dirname == '\0')
      break;

    /* Now search through the directory to  */
    /* find the entry...                    */
    i = FALSE;

    while (dir_read(fnp) == 1)
    {
      if (!(fnp->f_dir.dir_attrib & D_VOLID) &&
          fcbmatch(fcbname, fnp->f_dir.dir_name))
      {
        i = TRUE;
        break;
      }
      fnp->f_dmp->dm_entry++;
    }

    if (!i || !(fnp->f_dir.dir_attrib & D_DIR))
    {
      return (f_node_ptr) 0;
    }
    else
    {
      /* make certain we've moved off */
      /* root                         */
      dir_init_fnode(fnp, getdstart(fnp->f_dpb, &fnp->f_dir));
      fnp->f_dmp->dm_entry = 0;
    }
  }
  return fnp;
}

/* swap internal and external delete flags */
STATIC void swap_deleted(char *name)
{
  if (name[0] == DELETED || name[0] == EXT_DELETED)
    name[0] ^= EXT_DELETED - DELETED; /* 0xe0 */
}

/* Description.
 *  Read next consequitive directory entry, pointed by fnp.
 *  If some error occures the other critical
 *  fields aren't changed, except those used for caching.
 *  The fnp->f_dmp->dm_entry always corresponds to the directory entry
 *  which has been read.
 * Return value.
 *  1              - all OK, directory entry having been read is not empty.
 *  0              - Directory entry is empty.
 *  DE_SEEK        - Attempt to read beyound the end of the directory.
 *  DE_BLKINVLD    - Invalid block.
 * Note. Empty directory entries always resides at the end of the directory. */
COUNT dir_read(REG f_node_ptr fnp)
{
  struct buffer FAR *bp;
  REG UWORD secsize = fnp->f_dpb->dpb_secsize;
  unsigned sector;
  unsigned entry = fnp->f_dmp->dm_entry;

  /* can't have more than 65535 directory entries */
  if (entry >= 65535U)
      return DE_SEEK;

  /* Determine if we hit the end of the directory. If we have,    */
  /* bump the offset back to the end and exit. If not, fill the   */
  /* dirent portion of the fnode, set the SFT_FCLEAN bit and leave,*/
  /* but only for root directories                                */

  if (fnp->f_dmp->dm_dircluster == 0)
  {
    if (entry >= fnp->f_dpb->dpb_dirents)
      return DE_SEEK;

    fnp->f_dirsector = entry / (secsize / DIRENT_SIZE) +
      fnp->f_dpb->dpb_dirstrt;
  }
  else
  {
    /* Do a "seek" to the directory position        */
    fnp->f_offset = entry * (ULONG)DIRENT_SIZE;

    /* Search through the FAT to find the block     */
    /* that this entry is in.                       */
    if (map_cluster(fnp, XFR_READ) != SUCCESS)
      return DE_SEEK;

    /* Compute the block within the cluster and the */
    /* offset within the block.                     */
    sector = (UBYTE)(fnp->f_offset / secsize) & fnp->f_dpb->dpb_clsmask;

    fnp->f_dirsector = clus2phys(fnp->f_cluster, fnp->f_dpb) + sector;
    /* Get the block we need from cache             */
  }

  bp = getblock(fnp->f_dirsector, fnp->f_dpb->dpb_unit);

#ifdef DISPLAY_GETBLOCK
  printf("DIR (dir_read)\n");
#endif

  /* Now that we have the block for our entry, get the    */
  /* directory entry.                                     */
  if (bp == NULL)
    return DE_BLKINVLD;

  bp->b_flag &= ~(BFR_DATA | BFR_FAT);
  bp->b_flag |= BFR_DIR | BFR_VALID;

  fnp->f_diridx = entry % (secsize / DIRENT_SIZE);
  getdirent(&bp->b_buffer[fnp->f_diridx * DIRENT_SIZE], &fnp->f_dir);

  swap_deleted(fnp->f_dir.dir_name);

  /* and for efficiency, stop when we hit the first       */
  /* unused entry.                                        */
  /* either returns 1 or 0                                */
  return (fnp->f_dir.dir_name[0] != '\0');
}

/* Description.
 *  Writes directory entry pointed by fnp to disk. In case of erroneous
 *  situation fnode is released.
 *  The caller should set
 *    1. F_DMOD flag if original directory entry was modified.
 * Return value.
 *  TRUE  - all OK.
 *  FALSE - error occured (fnode is released).
 */
BOOL dir_write_update(REG f_node_ptr fnp, BOOL update)
{
  struct buffer FAR *bp;
  UBYTE FAR *vp;

  /* Update the entry if it was modified by a write or create...  */
  if (!update || (fnp->f_flags & (SFT_FCLEAN|SFT_FDATE)) != SFT_FCLEAN)
  {
    bp = getblock(fnp->f_dirsector, fnp->f_dpb->dpb_unit);

    /* Now that we have a block, transfer the directory      */
    /* entry into the block.                                */
    if (bp == NULL)
      return FALSE;

#ifdef DISPLAY_GETBLOCK
    printf("DIR (dir_write)\n");
#endif

    swap_deleted(fnp->f_dir.dir_name);

    vp = &bp->b_buffer[fnp->f_diridx * DIRENT_SIZE];

    if (update)
    {
      /* only update fields that are also in the SFT, for dos_close/commit */
      fmemcpy(&vp[DIR_NAME], fnp->f_dir.dir_name, FNAME_SIZE + FEXT_SIZE);
      fputbyte(&vp[DIR_ATTRIB], fnp->f_dir.dir_attrib);
      fputword(&vp[DIR_TIME], fnp->f_dir.dir_time);
      fputword(&vp[DIR_DATE], fnp->f_dir.dir_date);
      fputword(&vp[DIR_START], fnp->f_dir.dir_start);
#ifdef WITHFAT32
      if (ISFAT32(fnp->f_dpb))
        fputword(&vp[DIR_START_HIGH], fnp->f_dir.dir_start_high);
#endif
      fputlong(&vp[DIR_SIZE], fnp->f_dir.dir_size);
    }
    else
    {
      putdirent(&fnp->f_dir, vp);
    }

    swap_deleted(fnp->f_dir.dir_name);

    bp->b_flag &= ~(BFR_DATA | BFR_FAT);
    bp->b_flag |= BFR_DIR | BFR_DIRTY | BFR_VALID;
  }
  /* Clear buffers after directory write or DOS close                     */
  /* hazard: no error checking!                                           */
  flush_buffers(fnp->f_dpb->dpb_unit);
  return TRUE;
}

#ifndef IPL
COUNT dos_findfirst(UCOUNT attr, BYTE * name)
{
  REG f_node_ptr fnp;
  REG dmatch *dmp = &sda_tmp_dm;

/*  printf("ff %Fs\n", name);*/

  /* The findfirst/findnext calls are probably the worst of the   */
  /* DOS calls. They must work somewhat on the fly (i.e. - open   */
  /* but never close). The near fnodes now work this way. Every   */
  /* time a directory is searched, we will initialize the DOS     */
  /* dirmatch structure and then for every find, we will open the */
  /* current directory, do a seek and read.                       */

  /* first: findfirst("D:\\") returns DE_NFILES */
  if (name[3] == '\0')
    return DE_NFILES;

  /* Now open this directory so that we can read the      */
  /* fnode entry and do a match on it.                    */
  if ((fnp = split_path(name, &fnode[0])) == NULL)
    return DE_PATHNOTFND;

  /* Now search through the directory to find the entry...        */

  /* Special handling - the volume id is only in the root         */
  /* directory and only searched for once.  So we need to open    */
  /* the root and return only the first entry that contains the   */
  /* volume id bit set (while ignoring LFN entries).              */
  /* RBIL: ignore ReaDONLY and ARCHIVE bits but DEVICE ignored too*/
  /* For compatibility with bad search requests, only treat as    */
  /*   volume search if only volume bit set, else ignore it.      */
  if ((attr & ~(D_RDONLY | D_ARCHIVE | D_DEVICE)) == D_VOLID)
    /* if ONLY label wanted redirect search to root dir */
    dir_init_fnode(fnp, 0);

  /* Now further initialize the dirmatch structure.       */
  dmp->dm_drive = name[0] - 'A';
  dmp->dm_attr_srch = attr;

  return dos_findnext();
}

/*
    BUGFIX TE 06/28/01 
    
    when using FcbFindXxx, the only information available is
    the cluster number + entrycount. everything else MUST\
    be recalculated. 
    a good test for this is MSDOS CHKDSK, which now (seems too) work
*/

COUNT dos_findnext(void)
{
  REG f_node_ptr fnp;
  REG dmatch *dmp;

  /* Select the default to help non-drive specified path          */
  /* searches...                                                  */
  fnp = &fnode[0];
  dmp = &sda_tmp_dm;
  fnp->f_dpb = get_dpb(dmp->dm_drive);
  if (media_check(fnp->f_dpb) < 0)
    return DE_NFILES;

  dir_init_fnode(fnp, dmp->dm_dircluster);

  /* Search through the directory to find the entry, but do a     */
  /* seek first.                                                  */
  /* Loop through the directory                                   */
  while (dir_read(fnp) == 1)
  {
    ++dmp->dm_entry;
    if (fnp->f_dir.dir_name[0] != DELETED
        && (fnp->f_dir.dir_attrib & D_LFN) != D_LFN)
    {
      if (fcmp_wild(dmp->dm_name_pat, fnp->f_dir.dir_name, FNAME_SIZE + FEXT_SIZE))
      {
        /*
           MSD Command.com uses FCB FN 11 & 12 with attrib set to 0x16.
           Bits 0x21 seem to get set some where in MSD so Rd and Arc
           files are returned. 
           RdOnly + Archive bits are ignored
         */

        /* Test the attribute as the final step */
        /* It's either a special volume label search or an                 */
        /* attribute inclusive search. The attribute inclusive search      */
        /* can also find volume labels if you set e.g. D_DIR|D_VOLUME      */
        UBYTE attr_srch;
        attr_srch = dmp->dm_attr_srch & ~(D_RDONLY | D_ARCHIVE | D_DEVICE);
        if (attr_srch == D_VOLID)
        {
          if (!(fnp->f_dir.dir_attrib & D_VOLID))
            continue;
        }
        else if (~attr_srch & (D_DIR | D_SYSTEM | D_HIDDEN | D_VOLID) &
                 fnp->f_dir.dir_attrib)
          continue;
        /* If found, transfer it to the dmatch structure                */
        memcpy(&SearchDir, &fnp->f_dir, sizeof(struct dirent));
        /* return the result                                            */
        return SUCCESS;
      }
    }
  }


#ifdef DEBUG
  printf("dos_findnext: %11s\n", fnp->f_dir.dir_name);
#endif
  /* return the result                                            */
  return DE_NFILES;
}
#endif
/*
    this receives a name in 11 char field NAME+EXT and builds 
    a zeroterminated string

    unfortunately, blanks are allowed in filenames. like 
        "test e", " test .y z",...
        
    so we have to work from the last blank backward 
*/
void ConvertName83ToNameSZ(BYTE FAR * destSZ, BYTE FAR * srcFCBName)
{
  int loop;
  int noExtension = FALSE;

  if (*srcFCBName == '.')
  {
    noExtension = TRUE;
  }

  fmemcpy(destSZ, srcFCBName, FNAME_SIZE);

  srcFCBName += FNAME_SIZE;

  for (loop = FNAME_SIZE; --loop >= 0;)
  {
    if (destSZ[loop] != ' ')
      break;
  }
  destSZ += loop + 1;

  if (!noExtension)             /* not for ".", ".." */
  {

    for (loop = FEXT_SIZE; --loop >= 0;)
    {
      if (srcFCBName[loop] != ' ')
        break;
    }
    if (loop >= 0)
    {
      *destSZ++ = '.';
      fmemcpy(destSZ, srcFCBName, loop + 1);
      destSZ += loop + 1;
    }
  }
  *destSZ = '\0';
}

#if 0
/*
    returns the asciiSZ length of a 8.3 filename
*/

int FileName83Length(BYTE * filename83)
{
  BYTE buff[13];

  ConvertName83ToNameSZ(buff, filename83);

  return strlen(buff);
}
#endif

/* this routine converts a name portion of a fully qualified path       */
/* name, so . and .. are not allowed, only straightforward 8+3 names    */
const char *ConvertNameSZToName83(char *fcbname, const char *dirname)
{
  int i;
  memset(fcbname, ' ', FNAME_SIZE + FEXT_SIZE);

  for (i = 0; i < FNAME_SIZE + FEXT_SIZE; i++, dirname++)
  {
    char c = *dirname;
    if (c == '.')
      i = FNAME_SIZE - 1;
    else if (c != '\0' && c != '\\')
      fcbname[i] = c;
    else
      break;
  }
  return dirname;
}

