/****************************************************************/
/*                                                              */
/*                           lfnapi.c                           */
/*                                                              */
/*       Directory access functions for LFN aid API             */
/*                Module is under construction!                 */
/****************************************************************/

#include "portab.h"
#include "globals.h"

#ifdef VERSION_STRINGS
static BYTE *lfnaidRcsId = "$Id$";
#endif

#if 0
#define E_INVLDHNDL  -1
#define E_NOFREEHNDL -2
#define E_IOERROR    -3
#define E_INVLDDRV   -4

COUNT lfn_allocate_inode()
{
  f_node_ptr fnp = get_f_node();
  struct cds FAR *cdsp;
  if (fnp == 0) return E_NOFREEHNDL;

  cdsp = &CDSp->cds_table[default_drive];

  if (cdsp->cdsDpb == 0)
    {
      release_f_node(fnp);
      return E_INVLDDRV;
    }

  fnp->f_dpb = cdsp->cdsDpb;

  if (media_check(fnp->f_dpb) < 0)
    {
      release_f_node(fnp);
      return E_INVLDDRV;
    }

  fnp->f_dsize = DIRENT_SIZE * fnp->f_dpb->dpb_dirents;

  return xlt_fnp(fnp);
}

COUNT lfn_free_inode(COUNT handle)
{
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0) return E_INVLDHNDL;

  release_f_node(fnp);
}

COUNT lfn_to_unicode_chunk(UNICODE FAR **nptr, UNICODE FAR **uptr,
                           UCOUNT *index, COUNT count)
{
  COUNT j;

  for (j = 0; j < count; j++, *uptr++, *index++, *nptr++)
    {
      **nptr = **uptr;
      if (**uptr == 0) return 0;
    }

  return 1;
}

COUNT lfn_to_unicode(UNICODE FAR *name, UBYTE FAR *lfn_entry, UCOUNT *index)
{
  COUNT j;
  UNICODE FAR *uptr;
  UNICODE FAR *nptr = name;

  uptr = (UNICODE FAR *)&lfn_entry[1];
  if (!lfn_to_unicode_chunk(&nptr, &uptr, index, 5)) return 0;
  uptr = (UNICODE FAR *)&lfn_entry[0xe];
  if (!lfn_to_unicode_chunk(&nptr, &uptr, index, 6)) return 0;
  uptr = (UNICODE FAR *)&lfn_entry[0x1c];
  if (!lfn_to_unicode_chunk(&nptr, &uptr, index, 2)) return 0;

  return 1;
}

COUNT lfn_dir_read(COUNT handle, lfn_inode_ptr lip)
{
  COUNT index = 0; /* index of the first non-filled char in the long file
                      name string */
  ULONG original_diroff;
  f_node_ptr fnp = xlt_fd(handle);
  if (fnp == 0 || fnp->f_count <= 0) return E_INVLDHNDL;

  if (lfnp->l_dirstart == 0)
    {
    }

  original_diroff = fnp->f_diroff;

  lip->name[0] = 0;

  while (TRUE)
    {
      if (dir_read(fnp) <= 0) return E_IOERROR;	    
      if (fnp->f_dir.dir_name[0] != DELETED && fnp->f_dir.dir_name[0] != '\0' 
	  && fnp->f_dir.dir_attrib != D_LFN)
        {
	  fmemcpy(lip->l_dir, fnp->f_dir, sizeof(struct dirent));
          lip->l_diroff = fnp->f_diroff;
	  lip->l_dirstart = fnp->f_dirstart;
          break;
        }
    }
  fnp->f_diroff = original_diroff - DIRENT_SIZE;

  while (TRUE)
    {
      if (fnp->f_diroff == 0) break;
      fnp->f_diroff -= 2*DIRENT_SIZE;
      if (dir_read(fnp) <= 0) return E_IOERROR;
      if (fnp->f_dir.dir_name[0] == '\0'
          || fnp->f_dir.dir_name[0] == DELETED) break;
      if (!lfn_to_unicode(lip->name,
			  (UBYTE FAR *)fnp->f_dir, index)) break;
    }

  if (lip->name[0] == 0)
    {
      ConvertName83ToNameSZ(lip->name, lip->l_dir.dir_name);
    }

  return SUCCESS;
}

/* Calculate checksum for the 8.3 name */
UBYTE lfn_checksum(char *name)
{
  UBYTE sum;
  COUNT i;

  for (sum = 0, i = 11; --i >= 0; sum += *name++)
    sum = (sum << 7) | (sum >> 1);

  return sum;
}
#endif
