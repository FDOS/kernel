/****************************************************************/
/*                                                              */
/*                           lfnapi.c                           */
/*                                                              */
/*       Directory access functions for LFN helper API          */
/*                                                              */
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *lfnaidRcsId =
    "$Id$";
#endif

#ifdef WITHLFNAPI

#define RESERVED_FNODES 3  /* # of reserved fnodes for the kernel internal *
                            * usage. If something wrong happens with the   *
                            * driver using helper API and it grabs all     *
                            * possible fnodes, the kernel doesn't hang.    */

/* All possible error codes, returned by directory access functions */
/* Note. SUCCESS is returned if all OK.                             */
#define LHE_INVLDHNDL  -1  /* Invalid inode handle passed.                 */
#define LHE_NOFREEHNDL -2  /* Couldn't allocate another inode handle.      */
#define LHE_IOERROR    -3  /* Couldn't do I/O because of the lack of       *
                            * memory or due to a physical damage.          */
#define LHE_INVLDDRV   -4  /* Invalid drive chosen.                        */
#define LHE_DAMAGEDFS  -5  /* Damaged file system encountered.             */
#define LHE_NOSPACE    -6  /* There are no free clusters on drive.         */
#define LHE_SEEK       -7  /* Attempt to read beyound the end of the dir.  */

#define lfn(fnp) ((struct lfn_entry FAR *)&(fnp->f_dir))
#define CHARS_IN_LFN_ENTRY 13
#define UNICODE_FILLER 0xffff

COUNT ufstrlen(REG UNICODE FAR *); /* fstrlen for UNICODE strings */
UBYTE lfn_checksum(UBYTE FAR *);
COUNT extend_dir(f_node_ptr);
BOOL lfn_to_unicode(UNICODE FAR **name, struct lfn_entry FAR *lep);
VOID unicode_to_lfn(UNICODE FAR **name, struct lfn_entry FAR *lep);

/* Description.
 *  Allocates internal fnode and returns it's handle.
 * Return value.
 *  LHE_NOFREEHNDL, LHE_INVLDDRV
 *  >= 0           - handle of the allocated fnode.
 */
COUNT lfn_allocate_inode(VOID)
{
  f_node_ptr fnp = get_f_node();
  struct dpb FAR *dpbp;
  COUNT handle;
  if (fnp == 0) return LHE_NOFREEHNDL;

  handle = xlt_fnp(fnp);
  /* Check if there is at least # RESERVED_FNODES left for the kernel */
  if (f_nodes_cnt - handle < RESERVED_FNODES)
    {
      release_f_node(fnp);
      return LHE_NOFREEHNDL;
    }

  /* Check that default drive is a block device */
  dpbp = get_dpb(default_drive);

  if (dpbp == 0)
    {
      release_f_node(fnp);
      return LHE_INVLDDRV;
    }

  fnp->f_dpb = dpbp;

  if (media_check(dpbp) < 0)
    {
      release_f_node(fnp);
      return LHE_INVLDDRV;
    }

  return handle;
}

/* Description.
 *  Free allocated internal fnode.
 * Return value.
 *  SUCCESS, LHE_INVLDHNDL
 */
COUNT lfn_free_inode(COUNT handle)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0) return LHE_INVLDHNDL;

  dir_close(fnp);

  return SUCCESS;
}

/* Description.
 *  Initialize internal fnode, so that it'll point to the directory entry
 *  at "diroff" entry offset from the start of the directory with the "dirstart"
 *  starting cluster.
 * Return value.
 *  SUCCESS, LHE_INVLDHNDL
 */
COUNT lfn_setup_inode(COUNT handle, ULONG dirstart, UWORD diroff)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0) return LHE_INVLDHNDL;

  dir_init_fnode(fnp, (CLUSTER)dirstart);
  fnp->f_diroff = diroff;

  return SUCCESS;
}

/* Description.
 *  Create LFN directory entries for the long name "lip->l_name", followed
 *  by the SFN entry with raw image "lip->l_dir". The "lip->l_diroff" points
 *  to the SFN entry on return.
 *  The internal fnode is released on fatal error, except LHE_INVLDHNDL of
 *  course.
 * Return value.
 *  SUCCESS, LHE_INVLDHNDL, LHE_IOERROR, LHE_NOSPACE
 */
COUNT lfn_create_entries(COUNT handle, lfn_inode_ptr lip)
{
  f_node_ptr fnp = xlt_fd(handle);
  COUNT entries_needed, free_entries, i, rc;
  UNICODE FAR *lfn_name = lip->l_name;
  UBYTE id = 1, sfn_checksum = lfn_checksum(lip->l_dir.dir_name);
  unsigned sfn_offset;
  if (fnp == 0 || fnp->f_count <= 0) return LHE_INVLDHNDL;

  entries_needed = (ufstrlen(lfn_name) + CHARS_IN_LFN_ENTRY - 1)
    / CHARS_IN_LFN_ENTRY + 1; /* We want to create SFN entry too */
  
  /* Scan the directory from the very begining for the free directory entries */
  lfn_setup_inode(handle, fnp->f_dirstart, 0);

  free_entries = 0;
  while (TRUE)
    {
      rc = dir_read(fnp);
      if (rc == 0 || fnp->f_dir.dir_name[0] == DELETED)
        {
          free_entries++;
          if (free_entries == entries_needed)
            break;
        }
      else if (rc == DE_BLKINVLD)
        {
          dir_close(fnp);
          return LHE_IOERROR;
        }
      else if (rc == DE_SEEK)
        {
          if (extend_dir(fnp) != SUCCESS) return LHE_NOSPACE;
          /* fnp points to the first free dir entry on return from extend_dir,
           * so we go to previous entry to read this free entry on next cycle */
          fnp->f_diroff--;
        }
      else free_entries = 0;  /* rc == 1 here => we've read some sfn entry */
    }
  sfn_offset = fnp->f_diroff;

  fnp->f_flags.f_dmod = TRUE;
  /* Write SFN entry */
  fmemcpy(&fnp->f_dir, &lip->l_dir, sizeof(struct dirent));
  dir_write(fnp);
  
  fnp->f_diroff--;
  /* Go in the reverse direction and create LFN entries */
  for (i = 0; i < entries_needed - 1; i++, id++)
    {
      /* If this is the last LFN entry mark it's as those (6th bit is on) */
      if (i == (entries_needed - 2)) id |= 0x40;
      lfn_name = &lip->l_name[i * CHARS_IN_LFN_ENTRY];
      unicode_to_lfn(&lfn_name, lfn(fnp));
      lfn(fnp)->lfn_checksum = sfn_checksum;
      lfn(fnp)->lfn_id = id;
      fnp->f_dir.dir_attrib = D_LFN;
      if (!dir_write(fnp)) return LHE_IOERROR;
      fnp->f_diroff--;
    }
  fnp->f_flags.f_dmod = FALSE;
  
  fnp->f_diroff = sfn_offset;

  return SUCCESS;
}

/* Description.
 *  Read next consequitve long file name. The LFN is stored into the
 *  "lip->l_name" in unicode and the corresponding SFN entry raw image into the
 *  "lip->l_dir". If directory entry being read is a 8.3 file name, then
 *  it's image is stored into the "lip->l_dir" and "lip->l_name" has zero
 *  length, i.e. "lip->l_name[0]" == 0.
 * Return value.
 *  SUCCESS, LHE_INVLDHNDL, LHE_IOERROR, LHE_SEEK, LHE_DAMAGEDFS
 */
COUNT lfn_dir_read(COUNT handle, lfn_inode_ptr lip)
{
  COUNT rc;
  UBYTE id = 1, real_id;
  UNICODE FAR *lfn_name = lip->l_name;
  UWORD sfn_diroff;
  BOOL name_tail;
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0) return LHE_INVLDHNDL;

  /* Scan a directory for the next valid SFN entry */
  while (TRUE)
    {
      rc = dir_read(fnp);
      if (rc == DE_SEEK) return LHE_SEEK;
      else if (rc == DE_BLKINVLD) return LHE_IOERROR;
      if (fnp->f_dir.dir_name[0] != DELETED && fnp->f_dir.dir_attrib != D_LFN)
        {
          fmemcpy(&lip->l_dir, &fnp->f_dir, sizeof(struct dirent));
          sfn_diroff = fnp->f_diroff;
          break;
        }
    } 
    
  /* Go in the reverse direction and find the LFN entries corresponding to
   * the found SFN entry */
  while (TRUE)
    {
      if (fnp->f_diroff == 0) break;
      fnp->f_diroff -= 2;
      rc = dir_read(fnp);
      if (rc == DE_BLKINVLD) return LHE_IOERROR;
      if (fnp->f_dir.dir_name[0] == DELETED
          || fnp->f_dir.dir_attrib != D_LFN) break;
      real_id = lfn(fnp)->lfn_id;
      if ((real_id & 0x3f) > 20) return LHE_DAMAGEDFS;
      name_tail = lfn_to_unicode(&lfn_name, lfn(fnp));
      if (real_id & 0x40)
        {
          if ((id | 0x40) != real_id) return LHE_DAMAGEDFS;
          break;
        }
      else
        {
          if (name_tail || real_id != id
             || lfn(fnp)->lfn_checksum != lfn_checksum(lip->l_dir.dir_name))
            return LHE_DAMAGEDFS;
        }
        id++;
    }

  *lfn_name = 0;    /* Terminate LFN string */

  fnp->f_diroff = lip->l_diroff = sfn_diroff;

  return SUCCESS;
}

/* Description.
 *  Writes directory entry pointed by internal fnode to disk.
 *  The fnode is released on error.
 * Return value.
 *  SUCCESS, LHE_INVLDHNDL, LHE_IOERROR.
 */
COUNT lfn_dir_write(COUNT handle)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0) return LHE_INVLDHNDL;

  fnp->f_flags.f_dmod = TRUE;
  if (!dir_write(fnp)) return LHE_IOERROR;
  fnp->f_flags.f_dmod = FALSE;

  return SUCCESS;
}

/*********************************************************************/
/* Miscellaneous functions. These functions are not part of the API. */
/*********************************************************************/

/* Description.
 *  Copy the block of "count" unicode characters from source pointed by
 *  "sptr" to destination pointed by "dptr". The trailing characters of
 *  destination are filled with "UNICODE_FILLER".
 * Return value.
 *  TRUE  - The source string is too short, trailing characters are filled
 *          with "UNICODE_FILLER".
 *  FALSE - The unicode strings have the same size, no filling occured.
 */
BOOL transfer_unicode(UNICODE FAR **dptr, UNICODE FAR **sptr, COUNT count)
{
  COUNT j;
  BOOL found_zerro = FALSE;

  for (j = 0; j < count; j++, (*dptr)++, (*sptr)++)
    {
      if (found_zerro) **dptr = UNICODE_FILLER;
      else **dptr = **sptr;
      if (**sptr == 0) found_zerro = TRUE;
    }

  return found_zerro;
}

/* Description.
 *  Retrieve the LFN string chunk from the directory entry "lep" and store it
 *  to the "name" unicode string.
 * Return value.
 *  TRUE  - The LFN string chunk contains zero character.
 *  FALSE - Doesn't terminate with zero.
 */
BOOL lfn_to_unicode(UNICODE FAR **name, struct lfn_entry FAR *lep)
{
  UNICODE FAR *ptr;
    
  ptr = lep->lfn_name0_4;
  if (transfer_unicode(name, &ptr, 5)) return TRUE;
  ptr = lep->lfn_name5_10;
  if (transfer_unicode(name, &ptr, 6)) return TRUE;
  ptr = lep->lfn_name11_12;
  if (transfer_unicode(name, &ptr, 2)) return TRUE;

  return FALSE;
}

/* Description.
 */
VOID unicode_to_lfn(UNICODE FAR **name, struct lfn_entry FAR *lep)
{
  UNICODE FAR *ptr;
  
  ptr = lep->lfn_name0_4;
  transfer_unicode(&ptr, name, 5);
  ptr = lep->lfn_name5_10;
  transfer_unicode(&ptr, name, 6);
  ptr = lep->lfn_name11_12;
  transfer_unicode(&ptr, name, 2);
}

/* Calculate checksum for the 8.3 name */
UBYTE lfn_checksum(UBYTE FAR *sfn_name)
{
  UBYTE sum;
  COUNT i;

  for (sum = 0, i = 11; --i >= 0; sum += *sfn_name++)
    sum = (sum << 7) | (sum >> 1);

  return sum;
}

COUNT ufstrlen(REG UNICODE FAR *s)
{
  REG COUNT cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

#endif
