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
    "$Id$";
#endif

#define FCB_SUCCESS     0
#define FCB_ERR_NODATA  1
#define FCB_ERR_SEGMENT_WRAP 2
#define FCB_ERR_EOF     3
#define FCB_ERROR       0xff

STATIC fcb FAR *ExtFcbToFcb(xfcb FAR * lpExtFcb);
STATIC fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer,
                       COUNT * pCurDrive);
STATIC int FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
STATIC void FcbNextRecord(fcb FAR * lpFcb);
STATIC void FcbCalcRec(xfcb FAR * lpXfcb);

#define TestCmnSeps(lpFileName) (*lpFileName && strchr(":<|>+=,", *lpFileName) != NULL)
#define TestFieldSeps(lpFileName) ((unsigned char)*lpFileName <= ' ' || strchr("/\"[]<>|.", *lpFileName) != NULL)

static dmatch Dmatch;

BYTE FAR *FatGetDrvData(UBYTE drive, UWORD * spc, UWORD * bps, UWORD * nc)
{
  static BYTE mdb;
  UWORD navc;

  /* get the data available from dpb                       */
  *nc = 0xffff;                 /* pass 0xffff to skip free count */
  if (DosGetFree(drive, spc, &navc, bps, nc))
  {
    struct cds FAR *cdsp =
      &CDSp[(drive == 0 ? default_drive : drive - 1)];
    /* Point to the media desctriptor for this drive               */
    if (cdsp->cdsFlags & CDSNETWDRV)
    {
      mdb = *spc >> 8;
      *spc &= 0xff;
      return &mdb;
    }
    else
    {
      return (BYTE FAR *) & (cdsp->cdsDpb->dpb_mdb);
    }
  }
  return NULL;
}

#define PARSE_SEP_STOP          0x01
#define PARSE_DFLT_DRIVE        0x02
#define PARSE_BLNK_FNAME        0x04
#define PARSE_BLNK_FEXT         0x08

#define PARSE_RET_NOWILD        0
#define PARSE_RET_WILD          1
#define PARSE_RET_BADDRIVE      0xff

#ifndef IPL
UWORD FcbParseFname(int *wTestMode, const BYTE FAR * lpFileName, fcb FAR * lpFcb)
{
  WORD wRetCodeName = FALSE, wRetCodeExt = FALSE;
  const BYTE FAR * lpFileName2 = lpFileName;

  /* pjv -- ExtFcbToFcb?                                          */
  /* Start out with some simple stuff first.  Check if we are     */
  /* going to use a default drive specificaton.                   */
  if (!(*wTestMode & PARSE_DFLT_DRIVE))
    lpFcb->fcb_drive = FDFLT_DRIVE;
  if (!(*wTestMode & PARSE_BLNK_FNAME))
  {
    fmemset(lpFcb->fcb_fname, ' ', FNAME_SIZE);
  }
  if (!(*wTestMode & PARSE_BLNK_FEXT))
  {
    fmemset(lpFcb->fcb_fext, ' ', FEXT_SIZE);
  }

  /* Undocumented behavior, set record number & record size to 0  */
  lpFcb->fcb_cublock = lpFcb->fcb_recsiz = 0;

  if (!(*wTestMode & PARSE_SEP_STOP))
  {
    lpFileName2 = ParseSkipWh(lpFileName2);
    if (TestCmnSeps(lpFileName2))
      ++lpFileName2;
  }

  /* Undocumented "feature," we skip white space anyway           */
  lpFileName2 = ParseSkipWh(lpFileName2);

  /* Now check for drive specification                            */
  if (*(lpFileName2 + 1) == ':')
  {
    /* non-portable construct to be changed                 */
    REG UBYTE Drive = DosUpFChar(*lpFileName2) - 'A';

    if (Drive >= lastdrive)
    {
      *wTestMode = PARSE_RET_BADDRIVE;
      return lpFileName2 - lpFileName;
    }

    lpFcb->fcb_drive = Drive + 1;
    lpFileName2 += 2;
  }

  /* special cases: '.' and '..' */
  if (*lpFileName2 == '.')
  {
    lpFcb->fcb_fname[0] = '.';
    ++lpFileName2;
    if (*lpFileName2 == '.')
    {
      lpFcb->fcb_fname[1] = '.';
      ++lpFileName2;
    }
    *wTestMode = PARSE_RET_NOWILD;
    return lpFileName2 - lpFileName;
  }

  /* Now to format the file name into the string                  */
  lpFileName2 =
      GetNameField(lpFileName2, (BYTE FAR *) lpFcb->fcb_fname, FNAME_SIZE,
                   (BOOL *) & wRetCodeName);

  /* Do we have an extension? If do, format it else return        */
  if (*lpFileName2 == '.')
    lpFileName2 =
        GetNameField(++lpFileName2, (BYTE FAR *) lpFcb->fcb_fext,
                     FEXT_SIZE, (BOOL *) & wRetCodeExt);

  *wTestMode = (wRetCodeName | wRetCodeExt) ? PARSE_RET_WILD : PARSE_RET_NOWILD;
  return lpFileName2 - lpFileName;
}

const BYTE FAR * ParseSkipWh(const BYTE FAR * lpFileName)
{
  while (*lpFileName == ' ' || *lpFileName == '\t')
    ++lpFileName;
  return lpFileName;
}

#if 0                           /* defined above */
BOOL TestCmnSeps(BYTE FAR * lpFileName)
{
  BYTE *pszTest, *pszCmnSeps = ":<|>+=,";

  for (pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
    if (*lpFileName == *pszTest)
      return TRUE;
  return FALSE;
}
#endif

#if 0
BOOL TestFieldSeps(BYTE FAR * lpFileName)
{
  BYTE *pszTest, *pszCmnSeps = "/\"[]<>|.";

  /* Another non-portable construct                               */
  if (*lpFileName <= ' ')
    return TRUE;

  for (pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
    if (*lpFileName == *pszTest)
      return TRUE;
  return FALSE;
}
#endif

const BYTE FAR * GetNameField(const BYTE FAR * lpFileName, BYTE FAR * lpDestField,
                       COUNT nFieldSize, BOOL * pbWildCard)
{
  COUNT nIndex = 0;
  BYTE cFill = ' ';

  while (*lpFileName != '\0' && !TestFieldSeps(lpFileName)
         && nIndex < nFieldSize)
  {
    if (*lpFileName == ' ')
      break;
    if (*lpFileName == '*')
    {
      *pbWildCard = TRUE;
      cFill = '?';
      ++lpFileName;
      break;
    }
    if (*lpFileName == '?')
      *pbWildCard = TRUE;
    *lpDestField++ = DosUpFChar(*lpFileName++);
    ++nIndex;
  }

  /* Blank out remainder of field on exit                         */
  fmemset(lpDestField, cFill, nFieldSize - nIndex);
  return lpFileName;
}

STATIC VOID FcbNextRecord(fcb FAR * lpFcb)
{
  if (++lpFcb->fcb_curec > 128)
  {
    lpFcb->fcb_curec = 0;
    ++lpFcb->fcb_cublock;
  }
}

STATIC ULONG FcbRec(VOID)
{
  return ((ULONG) lpFcb->fcb_cublock * 128) + lpFcb->fcb_curec;
}

UBYTE FcbReadWrite(xfcb FAR * lpXfcb, UCOUNT recno, int mode)
{
  sft FAR *s;
  ULONG lPosit;
  UCOUNT nTransfer;
  BYTE far * FcbIoPtr = dta + recno * lpFcb->fcb_recsiz;

  if ((ULONG)recno * lpFcb->fcb_recsiz >= 0x10000ul ||
      FP_OFF(FcbIoPtr) < FP_OFF(dta))
    return FCB_ERR_SEGMENT_WRAP;                         

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);
    
  /* Get the SFT block that contains the SFT      */
  if ((s = idx_to_sft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FCB_ERR_NODATA;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return FCB_ERR_NODATA;
    
  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lPosit = FcbRec() * lpFcb->fcb_recsiz;
  if ((CritErrCode = -SftSeek(s, lPosit, 0)) != SUCCESS)
    return FCB_ERR_NODATA;

  /* Do the read                                                  */
  nTransfer = DosRWSft(s, lpFcb->fcb_recsiz, FcbIoPtr, &CritErrCode, mode);
  CritErrCode = -CritErrCode;
  
  /* Now find out how we will return and do it.                   */
  if (nTransfer == lpFcb->fcb_recsiz)
  {
    if (mode == XFR_WRITE) lpFcb->fcb_fsize = s->sft_size;
    FcbNextRecord(lpFcb);
    return FCB_SUCCESS;
  }
  if (mode == XFR_READ && nTransfer > 0)
  {
    fmemset(FcbIoPtr + nTransfer, 0, lpFcb->fcb_recsiz - nTransfer);
    FcbNextRecord(lpFcb);
    return FCB_ERR_EOF;
  }
  return FCB_ERR_NODATA;
}

UBYTE FcbGetFileSize(xfcb FAR * lpXfcb)
{
  COUNT FcbDrive, hndl;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);

  /* check for a device                                           */
  if (!lpFcb || IsDevice(SecPathName) || (lpFcb->fcb_recsiz == 0))
    return FCB_ERROR;

  hndl = (short)DosOpen(SecPathName, O_LEGACY | O_RDONLY | O_OPEN, 0);
  if (hndl >= 0)
  {
    ULONG fsize;

    /* Get the size                                         */
    fsize = DosGetFsize(hndl);

    /* compute the size and update the fcb                  */
    lpFcb->fcb_rndm = fsize / lpFcb->fcb_recsiz;
    if ((fsize % lpFcb->fcb_recsiz) != 0)
      ++lpFcb->fcb_rndm;

    /* close the file and leave                             */
    if ((CritErrCode = -DosClose(hndl)) == SUCCESS)
      return FCB_SUCCESS;
  }
  else
    CritErrCode = -hndl;
  return FCB_ERROR;
}

void FcbSetRandom(xfcb FAR * lpXfcb)
{
  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to. */
  lpFcb->fcb_rndm = FcbRec();
}

void FcbCalcRec(xfcb FAR * lpXfcb)
{

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lpFcb->fcb_cublock = lpFcb->fcb_rndm / 128;
  lpFcb->fcb_curec = lpFcb->fcb_rndm & 127;
}

UBYTE FcbRandomBlockIO(xfcb FAR * lpXfcb, COUNT nRecords, int mode)
{
  UCOUNT recno = 0;
  UBYTE nErrorCode;

  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  do
    nErrorCode = FcbReadWrite(lpXfcb, recno++, mode);
  while ((--nRecords > 0) && (nErrorCode == 0));

  /* Now update the fcb                                           */
  lpFcb->fcb_rndm = FcbRec();

  return nErrorCode;
}

UBYTE FcbRandomIO(xfcb FAR * lpXfcb, int mode)
{
  UWORD uwCurrentBlock;
  UBYTE ucCurrentRecord;
  UBYTE nErrorCode;

  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  uwCurrentBlock = lpFcb->fcb_cublock;
  ucCurrentRecord = lpFcb->fcb_curec;

  nErrorCode = FcbReadWrite(lpXfcb, 0, mode);

  lpFcb->fcb_cublock = uwCurrentBlock;
  lpFcb->fcb_curec = ucCurrentRecord;
  return nErrorCode;
}

/* merged fcbOpen and FcbCreate - saves ~200 byte */
UBYTE FcbOpen(xfcb FAR * lpXfcb, unsigned flags)
{
  sft FAR *sftp;
  COUNT sft_idx, FcbDrive;
  unsigned attr = 0;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  if (lpFcb == NULL)
    return FCB_ERROR;

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
  sftp->sft_mode |= SFT_MFCB;

  lpFcb->fcb_sftno = sft_idx;
  lpFcb->fcb_curec = 0;
  lpFcb->fcb_rndm = 0;
  
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
    return &lpExtFcb->xfcb_fcb;
  else
    return (fcb FAR *) lpExtFcb;
}

STATIC fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer,
                              COUNT * pCurDrive)
{
  fcb FAR *lpFcb;

  /* convert to fcb if needed first                               */
  lpFcb = ExtFcbToFcb(lpExtFcb);

  /* Build a traditional DOS file name                            */
  if (FcbNameInit(lpFcb, pszBuffer, pCurDrive) < SUCCESS)
    return NULL;

  /* and return the fcb pointer                                   */
  return lpFcb;
}

int FcbNameInit(fcb FAR * lpFcb, BYTE * szBuffer, COUNT * pCurDrive)
{
  BYTE loc_szBuffer[2 + FNAME_SIZE + 1 + FEXT_SIZE + 1];        /* 'A:' + '.' + '\0' */
  BYTE *pszBuffer = loc_szBuffer;

  /* Build a traditional DOS file name                            */
  if (lpFcb->fcb_drive != 0)
  {
    *pCurDrive = lpFcb->fcb_drive;
    pszBuffer[0] = 'A' + lpFcb->fcb_drive - 1;
    pszBuffer[1] = ':';
    pszBuffer += 2;
  }
  else
  {
    *pCurDrive = default_drive + 1;
  }
  ConvertName83ToNameSZ(pszBuffer, (BYTE FAR *) lpFcb->fcb_fname);
  return truename(loc_szBuffer, szBuffer, CDS_MODE_CHECK_DEV_PATH);
}

UBYTE FcbDelete(xfcb FAR * lpXfcb)
{
  COUNT FcbDrive;
  UBYTE result = FCB_SUCCESS;
  BYTE FAR *lpOldDta = dta;

  /* Build a traditional DOS file name                            */
  CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  /* check for a device                                           */
  if (lpFcb == NULL || IsDevice(SecPathName))
  {
    result = FCB_ERROR;
  }
  else
  {
    int attr = (lpXfcb->xfcb_flag == 0xff ? lpXfcb->xfcb_attrib : D_ALL);
    dmatch Dmatch;

    dta = (BYTE FAR *) & Dmatch;
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
  BYTE FAR *lpOldDta = dta;

  /* Build a traditional DOS file name                            */
  lpRenameFcb = (rfcb FAR *) CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);

  /* check for a device                                           */
  if (lpRenameFcb == NULL || IsDevice(SecPathName))
  {
    result = FCB_ERROR;
  }
  else
  {
    dmatch Dmatch;
    COUNT result;

    wAttr = (lpXfcb->xfcb_flag == 0xff ? lpXfcb->xfcb_attrib : D_ALL);
    dta = (BYTE FAR *) & Dmatch;
    if ((CritErrCode = -DosFindFirst(wAttr, SecPathName)) != SUCCESS)
    {
      result = FCB_ERROR;
    }
    else do
    {
      fcb LocalFcb;
      BYTE *pToName;
      const BYTE FAR *pFromPattern = Dmatch.dm_name;
      int i = 0;

      FcbParseFname(&i, pFromPattern, &LocalFcb);
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
      result = truename(SecPathName, PriPathName, 0);

      if (result < SUCCESS || (result & IS_DEVICE))
      {
        result = FCB_ERROR;
        break;
      }
      /* now to build a dos name again                */
      LocalFcb.fcb_drive = FcbDrive;
      result = FcbNameInit((fcb FAR *) & LocalFcb, SecPathName, &FcbDrive);
      if (result < SUCCESS || (!(result & IS_NETWORK) && (result & IS_DEVICE)))
      {
        result = FCB_ERROR;
        break;
      }

      if (DosRenameTrue(PriPathName, SecPathName, wAttr) != SUCCESS)
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
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* An already closed FCB can be closed again without error */
  if (lpFcb->fcb_sftno == (BYTE) 0xff)
    return FCB_SUCCESS;

  /* Get the SFT block that contains the SFT      */
  if ((s = idx_to_sft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FCB_ERROR;

  /* change time and set file size                */
  s->sft_size = lpFcb->fcb_fsize;
  if (!(s->sft_flags & SFT_FSHARED))
    dos_setfsize(s->sft_status, lpFcb->fcb_fsize);
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
    if ((sftp->sft_mode & SFT_MFCB) && sftp->sft_psp == cu_psp)
      DosCloseSft(idx, FALSE);
}

UBYTE FcbFindFirstNext(xfcb FAR * lpXfcb, BOOL First)
{
  BYTE FAR *lpDir;
  COUNT FcbDrive;
  psp FAR *lpPsp = MK_FP(cu_psp, 0);

  /* First, move the dta to a local and change it around to match */
  /* our functions.                                               */
  lpDir = (BYTE FAR *) dta;
  dta = (BYTE FAR *) & Dmatch;

  /* Next initialze local variables by moving them from the fcb   */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  if (lpFcb == NULL)
    return FCB_ERROR;

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
    dta = lpPsp->ps_dta;
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
  dta = lpPsp->ps_dta;
  return FCB_SUCCESS;
}
#endif

/*
 * Log: fcbfns.c,v - for newer entries see "cvs log fcbfns.c"
 *
 * Revision 1.7  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.6  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.5  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.4  1999/09/23 04:40:46  jprice
 * *** empty log message ***
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:42:15  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/09 02:54:23  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/04 03:18:37  jprice
 * Formating.  Added comments.
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
 *
 *    Rev 1.7   06 Dec 1998  8:44:10   patv
 * Expanded fcb functions for new I/O subsystem.
 *
 *    Rev 1.6   04 Jan 1998 23:14:38   patv
 * Changed Log for strip utility
 *
 *    Rev 1.5   03 Jan 1998  8:36:02   patv
 * Converted data area to SDA format
 *
 *    Rev 1.4   16 Jan 1997 12:46:38   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.3   29 May 1996 21:15:14   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.2   01 Sep 1995 17:48:44   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:50:26   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:06:06   patv
 * Initial revision.
 */
