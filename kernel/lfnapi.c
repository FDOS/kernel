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

#define LHE_INVLDHNDL  -1
#define LHE_NOFREEHNDL -2
#define LHE_IOERROR    -3
#define LHE_INVLDDRV   -4
#define LHE_DAMAGEDFS  -5
#define LHE_NOSPACE    -6
#define LHE_SEEK       -7

#define lfn(fnp) ((struct lfn_entry FAR *)&(fnp->f_dir))
#define CHARS_IN_LFN_ENTRY 13
#define UNICODE_FILLER 0xffff

COUNT ufstrlen(REG UNICODE FAR *);      /* fstrlen for UNICODE strings */
UBYTE lfn_checksum(UBYTE *);
COUNT extend_dir(f_node_ptr);
COUNT remove_lfn_entries(f_node_ptr fnp);

COUNT lfn_allocate_inode(VOID)
{
  f_node_ptr fnp = get_f_node();
  struct cds FAR *cdsp;
  if (fnp == 0)
    return LHE_NOFREEHNDL;

  cdsp = &CDSp->cds_table[default_drive];

  if (cdsp->cdsDpb == 0)
  {
    release_f_node(fnp);
    return LHE_INVLDDRV;
  }

  fnp->f_dpb = cdsp->cdsDpb;

  if (media_check(fnp->f_dpb) < 0)
  {
    release_f_node(fnp);
    return LHE_INVLDDRV;
  }

  return xlt_fnp(fnp);
}

COUNT lfn_free_inode(COUNT handle)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0)
    return LHE_INVLDHNDL;

  release_f_node(fnp);

  return SUCCESS;
}

COUNT lfn_setup_inode(COUNT handle, CLUSTER dirstart, ULONG diroff)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0)
    return LHE_INVLDHNDL;

  dir_init_fnode(fnp, dirstart);
  fnp->f_diroff = diroff;

  return SUCCESS;
}

BOOL transfer_unicode(UNICODE FAR ** dptr, UNICODE FAR ** sptr,
                      COUNT count)
{
  COUNT j;
  BOOL found_zerro = FALSE;

  for (j = 0; j < count; j++, (*dptr)++, (*sptr)++)
  {
    if (found_zerro)
      **dptr = UNICODE_FILLER;
    else
      **dptr = **sptr;
    if (**sptr == 0)
      found_zerro = TRUE;
  }

  return found_zerro;
}

BOOL lfn_to_unicode(UNICODE FAR ** name, struct lfn_entry FAR * lep)
{
  UNICODE FAR *ptr;

  ptr = lep->lfn_name0_4;
  if (!transfer_unicode(name, &ptr, 5))
    return FALSE;
  ptr = lep->lfn_name5_10;
  if (!transfer_unicode(name, &ptr, 6))
    return FALSE;
  ptr = lep->lfn_name11_12;
  if (!transfer_unicode(name, &ptr, 2))
    return FALSE;

  return TRUE;
}

VOID unicode_to_lfn(UNICODE FAR ** name, struct lfn_entry FAR * lep)
{
  UNICODE FAR *ptr;

  ptr = lep->lfn_name0_4;
  transfer_unicode(&ptr, name, 5);
  ptr = lep->lfn_name5_10;
  transfer_unicode(&ptr, name, 6);
  ptr = lep->lfn_name11_12;
  transfer_unicode(&ptr, name, 2);
}

COUNT lfn_dir_read(COUNT handle, lfn_inode_ptr lip)
{
  COUNT rc;
  UBYTE id = 1, real_id;
  UNICODE FAR *lfn_name = lip->name;
  ULONG sfn_diroff;
  BOOL name_tail = FALSE;
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0)
    return LHE_INVLDHNDL;

  while (TRUE)
  {
    rc = dir_read(fnp);
    if (rc == 0)
      return SUCCESS;
    else if (rc == DE_SEEK)
      return LHE_SEEK;
    else if (rc == DE_BLKINVLD)
      return LHE_IOERROR;
    if (fnp->f_dir.dir_name[0] != DELETED
        && fnp->f_dir.dir_attrib != D_LFN)
    {
      fmemcpy(&lip->l_dir, &fnp->f_dir, sizeof(struct dirent));
      sfn_diroff = fnp->f_diroff;
      break;
    }
  }
  fnp->f_diroff = lip->l_diroff;

  fmemset(lip->name, 0, 256 * sizeof(UNICODE));
  while (TRUE)
  {
    if (fnp->f_diroff == 0)
      break;
    fnp->f_diroff -= 2 * DIRENT_SIZE;
    rc = dir_read(fnp);
    if (rc == DE_BLKINVLD)
      return LHE_IOERROR;
    if (fnp->f_dir.dir_name[0] == DELETED
        || fnp->f_dir.dir_attrib != D_LFN)
      break;
    name_tail = lfn_to_unicode(&lfn_name, lfn(fnp));
    real_id = lfn(fnp)->lfn_id;
    if (real_id & 0x40)
    {
      if ((real_id | 0x40) != id)
        return LHE_DAMAGEDFS;
    }
    else
    {
      if (name_tail || real_id != id
          || lfn(fnp)->lfn_checksum != lfn_checksum(fnp->f_dir.dir_name))
        return LHE_DAMAGEDFS;
    }
  }

  fnp->f_diroff = lip->l_diroff = sfn_diroff;
  fnp->f_flags.f_dnew = TRUE;

  return SUCCESS;
}

COUNT lfn_dir_write(COUNT handle)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0)
    return LHE_INVLDHNDL;

  if (!dir_write(fnp))
  {
    lfn_allocate_inode();       /* protection against dir_write fault
                                 * this must restore things to the state before
                                 * the call */
    /* Yes, it's a hack! */
    return LHE_IOERROR;
  }

  return SUCCESS;
}

COUNT lfn_create_entries(COUNT handle, lfn_inode_ptr lip)
{
  f_node_ptr fnp = xlt_fd(handle);
  COUNT entries_needed, free_entries, i, rc;
  UNICODE FAR *lfn_name = lip->name;
  ULONG sfn_offset;
  if (fnp == 0 || fnp->f_count <= 0)
    return LHE_INVLDHNDL;

  entries_needed = (ufstrlen(lfn_name) + CHARS_IN_LFN_ENTRY - 1) / CHARS_IN_LFN_ENTRY + 1;      /* We want to create SFN entry too */

  /* Scan the directory from the very begining for the free directory entries */
  lfn_setup_inode(handle, fnp->f_dirstart, 0);

  free_entries = 0;
  while ((rc = dir_read(fnp)) == 1)
  {
    if (fnp->f_dir.dir_name[0] == DELETED)
    {
      free_entries++;
      if (free_entries == entries_needed)
        break;
    }
    else
      free_entries = 0;
  }
  if (rc == DE_BLKINVLD)
    return LHE_IOERROR;
  /* We have reached the end of the directory here. */

  if (free_entries != entries_needed)
    free_entries = 0;
  while (free_entries != entries_needed)
  {
    rc = dir_read(fnp);
    if (rc == 0)
      free_entries++;
    else if (rc == DE_BLKINVLD)
      return LHE_IOERROR;
    else if (rc == DE_SEEK && extend_dir(fnp) != SUCCESS)
    {
      lfn_allocate_inode();     /* Another hack. */
      return LHE_IOERROR;
    }
  }
  sfn_offset = fnp->f_diroff;
  fnp->f_diroff -= DIRENT_SIZE;

  for (i = entries_needed - 2; i >= 0; i++)
  {
    lfn_name = &lip->name[i * CHARS_IN_LFN_ENTRY];
    unicode_to_lfn(&lfn_name, lfn(fnp));
    fnp->f_dir.dir_attrib = D_LFN;
    if (!dir_write(fnp))
      return LHE_IOERROR;
    fnp->f_diroff -= DIRENT_SIZE;
  }

  fnp->f_diroff = sfn_offset;

  return SUCCESS;
}

COUNT lfn_remove_entries(COUNT handle)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0)
    return LHE_INVLDHNDL;

  if (remove_lfn_entries(fnp) < 0)
    return LHE_IOERROR;

  return SUCCESS;
}

/* Calculate checksum for the 8.3 name */
UBYTE lfn_checksum(UBYTE * sfn_name)
{
  UBYTE sum;
  COUNT i;

  for (sum = 0, i = 11; --i >= 0; sum += *sfn_name++)
    sum = (sum << 7) | (sum >> 1);

  return sum;
}

COUNT ufstrlen(REG UNICODE FAR * s)
{
  REG COUNT cnt = 0;

  while (*s++ != 0)
    ++cnt;
  return cnt;
}

#endif
