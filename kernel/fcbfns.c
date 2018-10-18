/****************************************************************/
/*                                                              */
/*                          fcbfns.c                            */
/*                                                              */
/*           Old CP/M Style Function Handlers for Kernel        */
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
static BYTE *RcsId =
    "$Id: fcbfns.c 1405 2009-05-26 20:44:44Z bartoldeman $";
#endif

#define FCB_SUCCESS     0
#define FCB_ERR_NODATA  1
#define FCB_ERR_SEGMENT_WRAP 2
#define FCB_ERR_EOF     3
#define FCB_ERROR       0xff

STATIC fcb FAR *ExtFcbToFcb(xfcb FAR * lpExtFcb);
STATIC fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer,
                       COUNT * pCurDrive);
STATIC void FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
STATIC void FcbNextRecord(fcb FAR * lpFcb);
STATIC void FcbCalcRec(xfcb FAR * lpXfcb);

#define TestCmnSeps(lpFileName) (*lpFileName && strchr(":;,=+ \t", *lpFileName) != NULL)
#define TestFieldSeps(lpFileName) ((unsigned char)*lpFileName <= ' ' || strchr("/\\\"[]<>|.:;,=+\t", *lpFileName) != NULL)

static dmatch Dmatch;

BYTE FAR *FatGetDrvData(UBYTE drive, UBYTE * pspc, UWORD * bps, UWORD * nc)
{
  static BYTE mdb;
  UWORD spc;

  /* get the data available from dpb                       */
  spc = DosGetFree(drive, NULL, bps, nc);
  if (spc != 0xffff)
  {
    struct dpb FAR *dpbp = get_dpb(drive == 0 ? default_drive : drive - 1);
    /* Point to the media desctriptor for this drive               */
    *pspc = (UBYTE)spc;
    if (dpbp == NULL)
    {
      mdb = spc >> 8;
      spc &= 0xff;
      return &mdb;
    }
    else
    {
      return (BYTE FAR *) & (dpbp->dpb_mdb);
    }
  }
  return NULL;
}

#define PARSE_SKIP_LEAD_SEP     0x01
#define PARSE_DFLT_DRIVE        0x02
#define PARSE_BLNK_FNAME        0x04
#define PARSE_BLNK_FEXT         0x08

#define PARSE_RET_NOWILD        0
#define PARSE_RET_WILD          1
#define PARSE_RET_BADDRIVE      0xff

#ifndef IPL
UWORD FcbParseFname(UBYTE *wTestMode, const BYTE FAR * lpFileName, fcb FAR * lpFcb)
{
  WORD wRetCodeName = FALSE, wRetCodeExt = FALSE;

  /* pjv -- ExtFcbToFcb?                                          */
  
  /* skip leading separators if requested                         */
  if (*wTestMode & PARSE_SKIP_LEAD_SEP)
  {
    while (TestCmnSeps(lpFileName))
      ++lpFileName;
  }

  /* Undocumented "feature," we skip white space anyway           */
  lpFileName = ParseSkipWh(lpFileName);

  /* Now check for drive specification                            */
  /* If drive specified, set to it (when valid) otherwise         */
  /* set to default drive unless leave as-is requested            */
  if (*(lpFileName + 1) == ':')
  {
    /* non-portable construct to be changed                 */
    REG UBYTE Drive = DosUpFChar(*lpFileName) - 'A';

    if (get_cds(Drive) == NULL)
    {
      *wTestMode = PARSE_RET_BADDRIVE;
      return FP_OFF(lpFileName);
    }

    lpFcb->fcb_drive = Drive + 1;
    lpFileName += 2;
  } else if (!(*wTestMode & PARSE_DFLT_DRIVE)) {
    lpFcb->fcb_drive = FDFLT_DRIVE;
  }

  /* Undocumented behavior, set record number & record size to 0  */
  /* per MS-DOS Encyclopedia pp269 no other FCB fields modified   */
  /* except zeroing current block and record size fields          */
  lpFcb->fcb_cublock = lpFcb->fcb_recsiz = 0;

  if (!(*wTestMode & PARSE_BLNK_FNAME))
  {
    fmemset(lpFcb->fcb_fname, ' ', FNAME_SIZE);
  }
  if (!(*wTestMode & PARSE_BLNK_FEXT))
  {
    fmemset(lpFcb->fcb_fext, ' ', FEXT_SIZE);
  }

  /* special cases: '.' and '..' */
  if (*lpFileName == '.')
  {
    lpFcb->fcb_fname[0] = '.';
    ++lpFileName;
    if (*lpFileName == '.')
    {
      lpFcb->fcb_fname[1] = '.';
      ++lpFileName;
    }
    *wTestMode = PARSE_RET_NOWILD;
    return FP_OFF(lpFileName);
  }

  /* Now to format the file name into the string                  */
  lpFileName =
      GetNameField(lpFileName, (BYTE FAR *) lpFcb->fcb_fname, FNAME_SIZE,
                   (BOOL *) & wRetCodeName);

  /* Do we have an extension? If do, format it else return        */
  if (*lpFileName == '.')
    lpFileName =
        GetNameField(++lpFileName, (BYTE FAR *) lpFcb->fcb_fext,
                     FEXT_SIZE, (BOOL *) & wRetCodeExt);

  *wTestMode = (wRetCodeName | wRetCodeExt) ? PARSE_RET_WILD : PARSE_RET_NOWILD;
  return FP_OFF(lpFileName);
}

const BYTE FAR * ParseSkipWh(const BYTE FAR * lpFileName)
{
  while (*lpFileName == ' ' || *lpFileName == '\t')
    ++lpFileName;
  return lpFileName;
}


const BYTE FAR * GetNameField(const BYTE FAR * lpFileName, BYTE FAR * lpDestField,
                       COUNT nFieldSize, BOOL * pbWildCard)
{
  COUNT nIndex = 0;
  BYTE cFill = ' ';

  while (*lpFileName != '\0' && !TestFieldSeps(lpFileName)
         && nIndex < nFieldSize)
  {
    /* convert * into multiple ? for remaining length of field    */
    if (*lpFileName == '*')
    {
      *pbWildCard = TRUE;
      cFill = '?';
      ++lpFileName;
      break;
    }
    /* include ? as-is but flag for return purposes wildcard used */
    if (*lpFileName == '?')
      *pbWildCard = TRUE;
    
    /* store uppercased character, and advance to next char       */
    *lpDestField++ = DosUpFChar(*lpFileName++);
    ++nIndex;
  }

  /* Blank out remainder of field on exit                         */
  fmemset(lpDestField, cFill, nFieldSize - nIndex);
  return lpFileName;
}

STATIC VOID FcbNextRecord(fcb FAR * lpFcb)
{
  if (++lpFcb->fcb_curec >= 128)
  {
    lpFcb->fcb_curec = 0;
    ++lpFcb->fcb_cublock;
  }
}

STATIC ULONG FcbRec(fcb FAR *lpFcb)
{
  return ((ULONG) lpFcb->fcb_cublock * 128) + lpFcb->fcb_curec;
}

UBYTE FcbReadWrite(xfcb FAR * lpXfcb, UCOUNT recno, int mode)
{
  ULONG lPosit;
  long nTransfer;
  fcb FAR *lpFcb;
  unsigned size;
  unsigned long bigsize;
  unsigned recsiz;

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  recsiz = lpFcb->fcb_recsiz;
  bigsize = (ULONG)recsiz * recno;
  if (bigsize > 0xffff)
    return FCB_ERR_SEGMENT_WRAP;
  size = (unsigned)bigsize;

  if (FP_OFF(dta) + size < FP_OFF(dta))
    return FCB_ERR_SEGMENT_WRAP;

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lPosit = FcbRec(lpFcb) * recsiz;
  if ((CritErrCode = -SftSeek(lpFcb->fcb_sftno, lPosit, 0)) != SUCCESS)
    return FCB_ERR_NODATA;

  /* Do the read                                                  */
  nTransfer = DosRWSft(lpFcb->fcb_sftno, size, dta, mode & ~XFR_FCB_RANDOM);
  if (nTransfer < 0)
    CritErrCode = -(int)nTransfer;

  /* Now find out how we will return and do it.                   */
  if (mode & XFR_WRITE)
    lpFcb->fcb_fsize = SftGetFsize(lpFcb->fcb_sftno);

  /* if end-of-file, then partial read should count last record */
  if (mode & XFR_FCB_RANDOM && recsiz > 0)
    lpFcb->fcb_rndm += ((unsigned)nTransfer + recsiz - 1) / recsiz;
  size -= (unsigned)nTransfer;
  if (size == 0)
  {
    FcbNextRecord(lpFcb);
    return FCB_SUCCESS;
  }
  size %= lpFcb->fcb_recsiz;
  if (mode & XFR_READ && size > 0)
  {
    fmemset((char FAR *)dta + (unsigned)nTransfer, 0, size);
    FcbNextRecord(lpFcb);
    return FCB_ERR_EOF;
  }
  return FCB_ERR_NODATA;
}

UBYTE FcbGetFileSize(xfcb FAR * lpXfcb)
{
  int FcbDrive, sft_idx;
  unsigned recsiz;

  /* Build a traditional DOS file name                            */
  fcb FAR *lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  recsiz = lpFcb->fcb_recsiz;

  /* check for a device                                           */
  if (!lpFcb || IsDevice(SecPathName) || (recsiz == 0))
    return FCB_ERROR;

  sft_idx = (short)DosOpenSft(SecPathName, O_LEGACY | O_RDONLY | O_OPEN, 0);
  if (sft_idx >= 0)
  {
    ULONG fsize;

    /* Get the size                                         */
    fsize = SftGetFsize(sft_idx);

    /* compute the size and update the fcb                  */
    lpFcb->fcb_rndm = (fsize + (recsiz - 1)) / recsiz;

    /* close the file and leave                             */
    if ((CritErrCode = -DosCloseSft(sft_idx, FALSE)) == SUCCESS)
      return FCB_SUCCESS;
  }
  else
    CritErrCode = -sft_idx;
  return FCB_ERROR;
}

void FcbSetRandom(xfcb FAR * lpXfcb)
{
  /* Convert to fcb if necessary                                  */
  fcb FAR *lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to. */
  lpFcb->fcb_rndm = FcbRec(lpFcb);
}

void FcbCalcRec(xfcb FAR * lpXfcb)
{

  /* Convert to fcb if necessary                                  */
  fcb FAR *lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lpFcb->fcb_cublock = (UWORD)(lpFcb->fcb_rndm / 128);
  lpFcb->fcb_curec = (UBYTE)lpFcb->fcb_rndm & 127;
}

UBYTE FcbRandomBlockIO(xfcb FAR * lpXfcb, UWORD *nRecords, int mode)
{
  UBYTE nErrorCode;
  fcb FAR *lpFcb;
  unsigned long old;

  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  old = lpFcb->fcb_rndm;
  nErrorCode = FcbReadWrite(lpXfcb, *nRecords, mode);
  *nRecords = (UWORD)(lpFcb->fcb_rndm - old);

  /* Now update the fcb                                           */
  FcbCalcRec(lpXfcb);

  return nErrorCode;
}

UBYTE FcbRandomIO(xfcb FAR * lpXfcb, int mode)
{
  UWORD uwCurrentBlock;
  UBYTE ucCurrentRecord;
  UBYTE nErrorCode;
  fcb FAR *lpFcb;

  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  uwCurrentBlock = lpFcb->fcb_cublock;
  ucCurrentRecord = lpFcb->fcb_curec;

  nErrorCode = FcbReadWrite(lpXfcb, 1, mode);

  lpFcb->fcb_cublock = uwCurrentBlock;
  lpFcb->fcb_curec = ucCurrentRecord;
  return nErrorCode;
}

/* FcbOpen and FcbCreate
   Expects lpXfcb to point to a valid, unopened FCB, containing file name to open (create)
   Create will attempt to find the file name in the current directory, if found truncates
     setting file size to 0, otherwise if does not exist will create the new file; the
     FCB is filled in same as the open call.
   On any error returns FCB_ERROR
   On success returns FCB_SUCCESS, and sets the following fields (other non-system reserved ones left unchanged)
     drive identifier (fcb_drive) set to actual drive (1=A, 2=B, ...; always >0 if not device)
     current block number (fcb_cublock) to 0
     file size (fcb_fsize) value from directory entry (0 if create)
     record size (fcb_recsiz) to 128; set to 0 for devices
     time & date (fcb_time & fcb_date) values from directory entry
     fcb_sftno, fcb_attrib_hi/_lo, fcb_strtclst, fcb_dirclst/off_unused are for internal use (system reserved)
*/
UBYTE FcbOpen(xfcb FAR * lpXfcb, unsigned flags)
{
  sft FAR *sftp;
  COUNT sft_idx, FcbDrive;
  unsigned attr = 0;

  /* Build a traditional DOS file name                            */
  fcb FAR *lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  if ((flags & O_CREAT) && lpXfcb->xfcb_flag == 0xff)
    /* pass attribute without constraints (dangerous for directories) */
    attr = lpXfcb->xfcb_attrib;

  sft_idx = (short)DosOpenSft(SecPathName, flags, attr);
  if (sft_idx < 0)
  {
    CritErrCode = -sft_idx;
    return FCB_ERROR;
  }

  sftp = idx_to_sft(sft_idx);
  sftp->sft_mode |= O_FCB;

  lpFcb->fcb_sftno = sft_idx;
  lpFcb->fcb_cublock = 0;
  /* should not be cleared, programs e.g. GEM depend on these values remaining unchanged
  lpFcb->fcb_curec = 0;
  lpFcb->fcb_rndm = 0;
  */
  
  lpFcb->fcb_recsiz = 0;      /* true for devices   */
  if (!(sftp->sft_flags & SFT_FDEVICE)) /* check for a device */
  {
    lpFcb->fcb_drive = FcbDrive;
    lpFcb->fcb_recsiz = 128;
  }
  lpFcb->fcb_fsize = sftp->sft_size;
  lpFcb->fcb_date = sftp->sft_date;
  lpFcb->fcb_time = sftp->sft_time;
  return FCB_SUCCESS;
}


STATIC fcb FAR *ExtFcbToFcb(xfcb FAR * lpExtFcb)
{
  if (*((UBYTE FAR *) lpExtFcb) == 0xff)
    sda_lpFcb = &lpExtFcb->xfcb_fcb;
  else
    sda_lpFcb = (fcb FAR *) lpExtFcb;
  return sda_lpFcb;
}

STATIC fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer,
                              COUNT * pCurDrive)
{
  fcb FAR *lpFcb;

  /* convert to fcb if needed first                               */
  sda_lpFcb = lpFcb = ExtFcbToFcb(lpExtFcb);

  /* Build a traditional DOS file name                            */
  FcbNameInit(lpFcb, pszBuffer, pCurDrive);
  /* and return the fcb pointer                                   */
  return lpFcb;
}

STATIC void FcbNameInit(fcb FAR * lpFcb, BYTE * szBuffer, COUNT * pCurDrive)
{
  BYTE *pszBuffer = szBuffer;

  /* Build a traditional DOS file name                            */
  *pCurDrive = default_drive + 1;
  if (lpFcb->fcb_drive != 0)
  {
    *pCurDrive = lpFcb->fcb_drive;
    pszBuffer[0] = 'A' + lpFcb->fcb_drive - 1;
    pszBuffer[1] = ':';
    pszBuffer += 2;
  }
  ConvertName83ToNameSZ(pszBuffer, lpFcb->fcb_fname);
}

UBYTE FcbDelete(xfcb FAR * lpXfcb)
{
  COUNT FcbDrive;
  UBYTE result = FCB_SUCCESS;
  void FAR *lpOldDta = dta;

  /* Build a traditional DOS file name                            */
  CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  /* check for a device                                           */
  if (IsDevice(SecPathName))
  {
    result = FCB_ERROR;
  }
  else
  {
    int attr = (lpXfcb->xfcb_flag == 0xff ? lpXfcb->xfcb_attrib : D_ALL);
    dmatch Dmatch;

    dta = &Dmatch;
    if ((CritErrCode = -DosFindFirst(attr, SecPathName)) != SUCCESS)
    {
      result = FCB_ERROR;
    }
    else do
    {
      SecPathName[0] = 'A' + FcbDrive - 1; 
      SecPathName[1] = ':';
      strcpy(&SecPathName[2], Dmatch.dm_name);
      if (DosDelete(SecPathName, attr) != SUCCESS)
      {
        result = FCB_ERROR;
        break;
      }
    }
    while ((CritErrCode = -DosFindNext()) == SUCCESS);
  }
  dta = lpOldDta;
  return result;
}

UBYTE FcbRename(xfcb FAR * lpXfcb)
{
  rfcb FAR *lpRenameFcb;
  COUNT FcbDrive;
  UBYTE result = FCB_SUCCESS;
  void FAR *lpOldDta = dta;

  /* Build a traditional DOS file name                            */
  lpRenameFcb = (rfcb FAR *) CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);

  /* check for a device                                           */
  if (IsDevice(SecPathName))
  {
    result = FCB_ERROR;
  }
  else
  {
    dmatch Dmatch;
    COUNT rc;

    wAttr = (lpXfcb->xfcb_flag == 0xff ? lpXfcb->xfcb_attrib : D_ALL);
    dta = &Dmatch;
    if ((CritErrCode = -DosFindFirst(wAttr, SecPathName)) != SUCCESS)
    {
      result = FCB_ERROR;
    }
    else do
    {
      /* 'A:' + '.' + '\0' */
      BYTE loc_szBuffer[2 + FNAME_SIZE + 1 + FEXT_SIZE + 1];
      fcb LocalFcb;
      BYTE *pToName;
      const BYTE FAR *pFromPattern = Dmatch.dm_name;
      int i;
      UBYTE mode = 0;

      FcbParseFname(&mode, pFromPattern, &LocalFcb);
      /* Overlay the pattern, skipping '?'            */
      /* I'm cheating because this assumes that the   */
      /* struct alignments are on byte boundaries     */
      pToName = LocalFcb.fcb_fname;
      pFromPattern = lpRenameFcb->renNewName;
      for (i = 0; i < FNAME_SIZE + FEXT_SIZE; i++)
      {
        if (*pFromPattern != '?')
          *pToName = *pFromPattern;
        pToName++;
        pFromPattern++;
      }

      SecPathName[0] = 'A' + FcbDrive - 1;
      SecPathName[1] = ':';
      strcpy(&SecPathName[2], Dmatch.dm_name);
      rc = truename(SecPathName, PriPathName, 0);

      if (rc < SUCCESS || (rc & IS_DEVICE))
      {
        result = FCB_ERROR;
        break;
      }
      /* now to build a dos name again                */
      LocalFcb.fcb_drive = FcbDrive;
      FcbNameInit(&LocalFcb, loc_szBuffer, &FcbDrive);
      rc = truename(loc_szBuffer, SecPathName, 0);
      if (rc < SUCCESS || (rc & (IS_NETWORK|IS_DEVICE)) == IS_DEVICE
        || DosRenameTrue(PriPathName, SecPathName, wAttr) != SUCCESS)
      {
        result = FCB_ERROR;
        break;
      }
    }
    while ((CritErrCode = -DosFindNext()) == SUCCESS);
  }
  dta = lpOldDta;
  return result;
}

/* TE:the MoveDirInfo() is now done by simply copying the dirEntry into the FCB
   this prevents problems with ".", ".." and saves code
   BO:use global SearchDir, as produced by FindFirst/Next
*/

UBYTE FcbClose(xfcb FAR * lpXfcb)
{
  sft FAR *s;

  /* Convert to fcb if necessary                                  */
  fcb FAR *lpFcb = ExtFcbToFcb(lpXfcb);

  /* An already closed FCB can be closed again without error */
  if (lpFcb->fcb_sftno == (BYTE) 0xff)
    return FCB_SUCCESS;

  /* Get the SFT block that contains the SFT      */
  if ((s = idx_to_sft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FCB_ERROR;

  /* change time and set file size                */
  s->sft_size = lpFcb->fcb_fsize;
  if (!(s->sft_flags & SFT_FSHARED))
    dos_merge_file_changes(lpFcb->fcb_sftno);
  DosSetFtimeSft(lpFcb->fcb_sftno, lpFcb->fcb_date, lpFcb->fcb_time);
  if ((CritErrCode = -DosCloseSft(lpFcb->fcb_sftno, FALSE)) == SUCCESS)
  {
    lpFcb->fcb_sftno = (BYTE) 0xff;
    return FCB_SUCCESS;
  }
  return FCB_ERROR;
}

/* close all files the current process opened by FCBs */
VOID FcbCloseAll()
{
  COUNT idx = 0;
  sft FAR *sftp;

  for (idx = 0; (sftp = idx_to_sft(idx)) != (sft FAR *) - 1; idx++)
    if ((sftp->sft_mode & O_FCB) && sftp->sft_psp == cu_psp)
      DosCloseSft(idx, FALSE);
}

UBYTE FcbFindFirstNext(xfcb FAR * lpXfcb, BOOL First)
{
  void FAR *orig_dta = dta;
  BYTE FAR *lpDir;
  COUNT FcbDrive;
  fcb FAR *lpFcb;

  /* First, move the dta to a local and change it around to match */
  /* our functions.                                               */
  lpDir = dta;
  dta = &Dmatch;

  /* Next initialze local variables by moving them from the fcb   */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  /* Reconstrct the dirmatch structure from the fcb - doesn't hurt for first */
  Dmatch.dm_drive = lpFcb->fcb_sftno;

  fmemcpy(Dmatch.dm_name_pat, lpFcb->fcb_fname, FNAME_SIZE + FEXT_SIZE);
  DosUpFMem((BYTE FAR *) Dmatch.dm_name_pat, FNAME_SIZE + FEXT_SIZE);
  
  Dmatch.dm_attr_srch = wAttr;
  Dmatch.dm_entry = lpFcb->fcb_strtclst;
  Dmatch.dm_dircluster = lpFcb->fcb_dirclst;

  wAttr = D_ALL;
  
  if ((xfcb FAR *) lpFcb != lpXfcb)
  {
    wAttr = lpXfcb->xfcb_attrib;
    fmemcpy(lpDir, lpXfcb, 7);
    lpDir += 7;
  }

  CritErrCode = -(First ? DosFindFirst(wAttr, SecPathName) : DosFindNext());
  if (CritErrCode != SUCCESS)
  {
    dta = orig_dta;
    return FCB_ERROR;
  }

  *lpDir++ = FcbDrive;
  fmemcpy(lpDir, &SearchDir, sizeof(struct dirent));
  
  lpFcb->fcb_dirclst = (UWORD) Dmatch.dm_dircluster;
  lpFcb->fcb_strtclst = Dmatch.dm_entry;
  
/*
  This is undocumented and seen using Pcwatch and Ramview.
  The First byte is the current directory count and the second seems
  to be the attribute byte.
 */
  lpFcb->fcb_sftno = Dmatch.dm_drive;   /* MSD seems to save this @ fcb_date. */
#if 0
  lpFcb->fcb_cublock = Dmatch.dm_entry;
  lpFcb->fcb_cublock *= 0x100;
  lpFcb->fcb_cublock += wAttr;
#endif
  dta = orig_dta;
  return FCB_SUCCESS;
}
#endif

