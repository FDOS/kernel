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
static BYTE *RcsId = "$Id$";
#endif

/*
 * $Log$
 * Revision 1.6  2000/08/06 05:50:17  jimtabor
 * Add new files and update cvs with patches and changes
 *
 * Revision 1.5  2000/06/21 18:16:46  jimtabor
 * Add UMB code, patch, and code fixes
 *
 * Revision 1.4  2000/05/26 19:25:19  jimtabor
 * Read History file for Change info
 *
 * Revision 1.3  2000/05/25 20:56:21  jimtabor
 * Fixed project history
 *
 * Revision 1.2  2000/05/08 04:30:00  jimtabor
 * Update CVS to 2020
 *
 * Revision 1.1.1.1  2000/05/06 19:34:53  jhall1
 * The FreeDOS Kernel.  A DOS kernel that aims to be 100% compatible with
 * MS-DOS.  Distributed under the GNU GPL.
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

#define FCB_SUCCESS     0
#define FCB_ERR_NODATA  1
#define FCB_ERR_EOF     3
#define FCB_ERR_WRITE   1
#define D_ALL   D_NORMAL | D_RDONLY | D_HIDDEN | D_SYSTEM | D_DIR | D_ARCHIVE

#ifdef PROTO
fcb FAR *ExtFcbToFcb(xfcb FAR * lpExtFcb);
fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer, COUNT * pCurDrive);
void FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
sft FAR *FcbGetSft(COUNT SftIndex);
VOID FcbNextRecord(fcb FAR * lpFcb);
sft FAR *FcbGetFreeSft(WORD FAR * sft_idx);
BOOL FcbCalcRec(xfcb FAR * lpXfcb);
VOID MoveDirInfo(dmatch FAR * lpDmatch, struct dirent FAR * lpDir);
#else
fcb FAR *ExtFcbToFcb();
fcb FAR *CommonFcbInit();
void FcbNameInit();
sft FAR *FcbGetSft();
VOID FcbNextRecord();
sft FAR *FcbGetFreeSft();
BOOL FcbCalcRec();
VOID MoveDirInfo();
#endif

static dmatch Dmatch;

VOID FatGetDrvData(COUNT drive, COUNT FAR * spc, COUNT FAR * bps,
                   COUNT FAR * nc, BYTE FAR ** mdp)
{
  struct dpb *dpbp;

  printf("FGDD\n");

  /* first check for valid drive                                  */
    if ((drive < 0) || (drive > (lastdrive -1)) || (drive > NDEVS))
  {
    *spc = -1;
    return;
  }

  /* next - "log" in the drive                                    */
  drive = (drive == 0 ? default_drive : drive - 1);
	if (CDSp->cds_table[drive].cdsFlags & CDSNETWDRV) {
		printf("FatGetDrvData not yet supported over network drives\n");
		*spc = -1;
		return;
	}
  dpbp = (struct dpb *)CDSp->cds_table[drive].cdsDpb;
  dpbp->dpb_flags = -1;
  if ((media_check(dpbp) < 0))
  {
    *spc = -1;
    return;
  }

  /* get the data vailable from dpb                       */
  *nc = dpbp->dpb_size;
  *spc = dpbp->dpb_clsmask + 1;
  *bps = dpbp->dpb_secsize;

  /* Point to the media desctriptor fotr this drive               */
  *mdp = &(dpbp->dpb_mdb);
}

#define PARSE_SEP_STOP          0x01
#define PARSE_DFLT_DRIVE        0x02
#define PARSE_BLNK_FNAME        0x04
#define PARSE_BLNK_FEXT         0x08

#define PARSE_RET_NOWILD        0
#define PARSE_RET_WILD          1
#define PARSE_RET_BADDRIVE      0xff

#ifndef IPL
WORD FcbParseFname(int wTestMode, BYTE FAR ** lpFileName, fcb FAR * lpFcb)
{
  COUNT nIndex;
  WORD wRetCode = PARSE_RET_NOWILD;

  /* pjv -- ExtFcbToFcb?                                          */
  /* Start out with some simple stuff first.  Check if we are     */
  /* going to use a default drive specificaton.                   */
  if (!(wTestMode & PARSE_DFLT_DRIVE))
    lpFcb->fcb_drive = FDFLT_DRIVE;
  if (!(wTestMode & PARSE_BLNK_FNAME))
  {
    for (nIndex = 0; nIndex < FNAME_SIZE; ++nIndex)
      lpFcb->fcb_fname[nIndex] = ' ';
  }
  if (!(wTestMode & PARSE_BLNK_FEXT))
  {
    for (nIndex = 0; nIndex < FEXT_SIZE; ++nIndex)
      lpFcb->fcb_fext[nIndex] = ' ';
  }

  /* Undocumented behavior, set record number & record size to 0  */
  lpFcb->fcb_cublock = lpFcb->fcb_recsiz = 0;

  if (!(wTestMode & PARSE_SEP_STOP))
  {
    *lpFileName = ParseSkipWh(*lpFileName);
    if (TestCmnSeps(*lpFileName))
      ++ * lpFileName;
  }

  /* Undocumented "feature," we skip white space anyway           */
  *lpFileName = ParseSkipWh(*lpFileName);

  /* Now check for drive specification                            */
  if (*(*lpFileName + 1) == ':')
  {
    REG BYTE Drive = DosUpFChar(**lpFileName);

    /* non-portable construct to be changed                 */
    if (Drive < 'A' || Drive > 'Z')
      return PARSE_RET_BADDRIVE;
    Drive -= ('A' - 1);
    if (Drive > (lastdrive -1))
      return PARSE_RET_BADDRIVE;
    else
      lpFcb->fcb_drive = Drive;
    *lpFileName += 2;
  }

  /* Now to format the file name into the string                  */
  *lpFileName = GetNameField(*lpFileName, (BYTE FAR *) lpFcb->fcb_fname, FNAME_SIZE, (BOOL *) & wRetCode);

  /* Do we have an extension? If do, format it else return        */
  if (**lpFileName == '.')
    *lpFileName = GetNameField(++*lpFileName, (BYTE FAR *) lpFcb->fcb_fext, FEXT_SIZE, (BOOL *) & wRetCode);

  return wRetCode ? PARSE_RET_WILD : PARSE_RET_NOWILD;
}

BYTE FAR *ParseSkipWh(BYTE FAR * lpFileName)
{
  while (*lpFileName == ' ' || *lpFileName == '\t')
    ++lpFileName;
  return lpFileName;
}

BOOL TestCmnSeps(BYTE FAR * lpFileName)
{
  BYTE *pszTest,
   *pszCmnSeps = ":<|>+=,";

  for (pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
    if (*lpFileName == *pszTest)
      return TRUE;
  return FALSE;
}

BOOL TestFieldSeps(BYTE FAR * lpFileName)
{
  BYTE *pszTest,
   *pszCmnSeps = "/\"[]<>|.";

  /* Another non-portable construct                               */
  if (*lpFileName <= ' ')
    return TRUE;

  for (pszTest = pszCmnSeps; *pszTest != '\0'; ++pszTest)
    if (*lpFileName == *pszTest)
      return TRUE;
  return FALSE;
}

BYTE FAR *GetNameField(BYTE FAR * lpFileName, BYTE FAR * lpDestField,
                       COUNT nFieldSize, BOOL * pbWildCard)
{
  COUNT nIndex = 0;
  BYTE cFill = ' ';

  *pbWildCard = FALSE;
  while (*lpFileName != '\0' && !TestFieldSeps(lpFileName) && nIndex < nFieldSize)
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
  for (; nIndex < nFieldSize; ++nIndex)
    *lpDestField++ = cFill;
  return lpFileName;
}

static sft FAR *FcbGetSft(COUNT SftIndex)
{
  /* Get the SFT block that contains the SFT      */
  for (lpCurSft = sfthead; lpCurSft != (sfttbl FAR *) - 1;
       lpCurSft = lpCurSft->sftt_next)
  {
    if (SftIndex < lpCurSft->sftt_count)
      break;
    else
      SftIndex -= lpCurSft->sftt_count;
  }

  /* If not found, return an error                */
  if (lpCurSft == (sfttbl FAR *) - 1)
    return (sft FAR *) - 1;

  /* finally, point to the right entry            */
  return (sft FAR *) & (lpCurSft->sftt_table[SftIndex]);
}

static VOID FcbNextRecord(fcb FAR * lpFcb)
{
  if (++lpFcb->fcb_curec > 128)
  {
    lpFcb->fcb_curec = 0;
    ++lpFcb->fcb_cublock;
  }
}

BOOL FcbRead(xfcb FAR * lpXfcb, COUNT * nErrorCode)
{
  sft FAR *s;
  LONG lPosit;
  COUNT nRead;
  psp FAR *p = MK_FP(cu_psp, 0);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Get the SFT block that contains the SFT      */
  if ((s = FcbGetSft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FALSE;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return FALSE;

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lPosit = ((lpFcb->fcb_cublock * 128) + lpFcb->fcb_curec)
      * lpFcb->fcb_recsiz;
  if (SftSeek(s, lPosit, 0) != SUCCESS)
  {
    *nErrorCode = FCB_ERR_EOF;
    return FALSE;
  }

  if (s->sft_flags & SFT_FSHARED)
  {
    nRead = Remote_RW(REM_READ, lpFcb->fcb_recsiz, p->ps_dta, s, nErrorCode);
  }
  else
  {

    /* Do the read                                                  */
    nRead = dos_read(s->sft_status, p->ps_dta, lpFcb->fcb_recsiz);
  }

  /* Now find out how we will return and do it.                   */
  if (nRead == lpFcb->fcb_recsiz)
  {
    *nErrorCode = FCB_SUCCESS;
    FcbNextRecord(lpFcb);
    return TRUE;
  }
  else if (nRead < 0)
  {
    *nErrorCode = FCB_ERR_EOF;
    return TRUE;
  }
  else if (nRead == 0)
  {
    *nErrorCode = FCB_ERR_NODATA;
    return FALSE;
  }
  else
  {
    COUNT nIdx,
      nCount;
    BYTE FAR *lpDta;

    nCount = lpFcb->fcb_recsiz - nRead;
    lpDta = (BYTE FAR *) & (p->ps_dta[nRead]);
    for (nIdx = 0; nIdx < nCount; nIdx++)
      *lpDta++ = 0;
    *nErrorCode = FCB_ERR_EOF;
    FcbNextRecord(lpFcb);
    return FALSE;
  }
}

BOOL FcbWrite(xfcb FAR * lpXfcb, COUNT * nErrorCode)
{
  sft FAR *s;
  LONG lPosit;
  COUNT nWritten;
  psp FAR *p = MK_FP(cu_psp, 0);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Get the SFT block that contains the SFT      */
  if ((s = FcbGetSft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FALSE;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return FALSE;

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lPosit = ((lpFcb->fcb_cublock * 128) + lpFcb->fcb_curec)
      * lpFcb->fcb_recsiz;
  if (SftSeek(s, lPosit, 0) != SUCCESS)
  {
    *nErrorCode = FCB_ERR_EOF;
    return FALSE;
  }

  if (s->sft_flags & SFT_FSHARED)
  {
    nWritten = Remote_RW(REM_WRITE, lpFcb->fcb_recsiz, p->ps_dta, s, nErrorCode);
  }
  else
  {

    /* Do the read                                                  */
    nWritten = dos_write(s->sft_status, p->ps_dta, lpFcb->fcb_recsiz);
    s->sft_size = dos_getcufsize(s->sft_status);
  }

  /* Now find out how we will return and do it.                   */
  if (nWritten == lpFcb->fcb_recsiz)
  {
    lpFcb->fcb_fsize = s->sft_size;
    FcbNextRecord(lpFcb);
    *nErrorCode = FCB_SUCCESS;
    return TRUE;
  }
  else if (nWritten <= 0)
  {
    *nErrorCode = FCB_ERR_WRITE;
    return TRUE;
  }
  *nErrorCode = FCB_ERR_WRITE;
  return FALSE;
}

BOOL FcbGetFileSize(xfcb FAR * lpXfcb)
{
  COUNT FcbDrive,
    FileNum;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, PriPathName, &FcbDrive);

  /* check for a device                                           */
  if (IsDevice(PriPathName) || (lpFcb->fcb_recsiz == 0))
  {
    return FALSE;
  }
  FileNum = dos_open(PriPathName, O_RDONLY);
  if (FileNum >= 0)
  {
    LONG fsize;

    /* Get the size                                         */
    fsize = dos_getfsize(FileNum);

    /* compute the size and update the fcb                  */
    lpFcb->fcb_rndm = fsize / lpFcb->fcb_recsiz;
    if ((fsize % lpFcb->fcb_recsiz) != 0)
      ++lpFcb->fcb_rndm;

    /* close the file and leave                             */
    return dos_close(FileNum) == SUCCESS;
  }
  else
    return FALSE;
}

BOOL FcbSetRandom(xfcb FAR * lpXfcb)
{
  LONG lPosit;

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lpFcb->fcb_rndm = (lpFcb->fcb_cublock * 128)
      + lpFcb->fcb_curec;

  return TRUE;
}

BOOL FcbCalcRec(xfcb FAR * lpXfcb)
{
  LONG lPosit;

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Now update the fcb and compute where we need to position     */
  /* to.                                                          */
  lpFcb->fcb_cublock = lpFcb->fcb_rndm / 128;
  lpFcb->fcb_curec = lpFcb->fcb_rndm % 128;

  return TRUE;
}

BOOL FcbRandomBlockRead(xfcb FAR * lpXfcb, COUNT nRecords, COUNT * nErrorCode)
{
  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  do
    FcbRead(lpXfcb, nErrorCode);
  while ((--nRecords > 0) && (*nErrorCode == 0));

  /* Now update the fcb                                           */
  lpFcb->fcb_rndm = lpFcb->fcb_cublock * 128 + lpFcb->fcb_curec;

  return TRUE;
}

BOOL FcbRandomBlockWrite(xfcb FAR * lpXfcb, COUNT nRecords, COUNT * nErrorCode)
{
  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  do
    FcbWrite(lpXfcb, nErrorCode);
  while ((--nRecords > 0) && (*nErrorCode == 0));

  /* Now update the fcb                                           */
  lpFcb->fcb_rndm = lpFcb->fcb_cublock * 128 + lpFcb->fcb_curec;

  return TRUE;
}

BOOL FcbRandomRead(xfcb FAR * lpXfcb, COUNT * nErrorCode)
{
  UWORD uwCurrentBlock;
  UBYTE ucCurrentRecord;

  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  uwCurrentBlock = lpFcb->fcb_cublock;
  ucCurrentRecord = lpFcb->fcb_curec;

  FcbRead(lpXfcb, nErrorCode);

  lpFcb->fcb_cublock = uwCurrentBlock;
  lpFcb->fcb_curec = ucCurrentRecord;
  return TRUE;
}

BOOL FcbRandomWrite(xfcb FAR * lpXfcb, COUNT * nErrorCode)
{
  UWORD uwCurrentBlock;
  UBYTE ucCurrentRecord;

  FcbCalcRec(lpXfcb);

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  uwCurrentBlock = lpFcb->fcb_cublock;
  ucCurrentRecord = lpFcb->fcb_curec;

  FcbWrite(lpXfcb, nErrorCode);

  lpFcb->fcb_cublock = uwCurrentBlock;
  lpFcb->fcb_curec = ucCurrentRecord;
  return TRUE;
}

static sft FAR *FcbGetFreeSft(WORD FAR * sft_idx)
{
  WORD sys_idx = 0;
  sfttbl FAR *sp;

  /* Get the SFT block that contains the SFT      */
  for (sp = sfthead; sp != (sfttbl FAR *) - 1; sp = sp->sftt_next)
  {
    REG WORD i;

    for (i = 0; i < sp->sftt_count; i++)
    {
      if (sp->sftt_table[i].sft_count == 0)
      {
        *sft_idx = sys_idx + i;
        return (sft FAR *) & sp->sftt_table[sys_idx + i];
      }
    }
    sys_idx += i;
  }

  /* If not found, return an error                */
  return (sft FAR *) - 1;
}

BOOL FcbCreate(xfcb FAR * lpXfcb)
{
  WORD sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  COUNT FcbDrive;

  /* get a free system file table entry                           */
  if ((sftp = FcbGetFreeSft((WORD FAR *) & sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, PriPathName, &FcbDrive);

  /* check for a device                                           */
  dhp = IsDevice(PriPathName);
  if (dhp)
  {
        sftp->sft_count += 1;
        sftp->sft_mode = O_RDWR;
        sftp->sft_attrib = 0;
        sftp->sft_flags =
            ((dhp->dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF;
        sftp->sft_psp = cu_psp;
        fbcopy(lpFcb->fcb_fname, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
        sftp->sft_dev = dhp;
        lpFcb->fcb_sftno = sft_idx;
        lpFcb->fcb_curec = 0;
        lpFcb->fcb_recsiz = 0;
        lpFcb->fcb_fsize = 0;
        lpFcb->fcb_date = dos_getdate();
        lpFcb->fcb_time = dos_gettime();
        lpFcb->fcb_rndm = 0;
        return TRUE;
  }
  sftp->sft_status = dos_creat(PriPathName, 0);
  if (sftp->sft_status >= 0)
  {
    lpFcb->fcb_drive = FcbDrive;
    lpFcb->fcb_sftno = sft_idx;
    lpFcb->fcb_curec = 0;
    lpFcb->fcb_recsiz = 128;
    lpFcb->fcb_fsize = 0;
    lpFcb->fcb_date = dos_getdate();
    lpFcb->fcb_time = dos_gettime();
    lpFcb->fcb_rndm = 0;
    sftp->sft_count += 1;
    sftp->sft_mode = O_RDWR;
    sftp->sft_attrib = 0;
    sftp->sft_flags = 0;
    sftp->sft_psp = cu_psp;
    fbcopy((BYTE FAR *) & lpFcb->fcb_fname, (BYTE FAR *) & sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
    return TRUE;
  }
  else
    return FALSE;
}

static fcb FAR *ExtFcbToFcb(xfcb FAR * lpExtFcb)
{
  if (*((UBYTE FAR *) lpExtFcb) == 0xff)
    return &lpExtFcb->xfcb_fcb;
  else
    return (fcb FAR *) lpExtFcb;
}

static fcb FAR *CommonFcbInit(xfcb FAR * lpExtFcb, BYTE * pszBuffer,
                              COUNT * pCurDrive)
{
  BYTE FAR *lpszFcbFname,
   *lpszFcbFext;
  COUNT nDrvIdx,
    nFnameIdx,
    nFextIdx;
  fcb FAR *lpFcb;

  /* convert to fcb if needed first                               */
  lpFcb = ExtFcbToFcb(lpExtFcb);

  /* Build a traditional DOS file name                            */
  FcbNameInit(lpFcb, pszBuffer, pCurDrive);

  /* and return the fcb pointer                                   */
  return lpFcb;
}

void FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive)
{
  BYTE FAR *lpszFcbFname,
    FAR * lpszFcbFext;
  COUNT nDrvIdx,
    nFnameIdx,
    nFextIdx;

  /* Build a traditional DOS file name                            */
  lpszFcbFname = (BYTE FAR *) lpFcb->fcb_fname;
  if (lpFcb->fcb_drive != 0)
  {
    *pCurDrive = lpFcb->fcb_drive;
    pszBuffer[0] = 'A' + lpFcb->fcb_drive - 1;
    pszBuffer[1] = ':';
    nDrvIdx = 2;
  }
  else
  {
    *pCurDrive = default_drive + 1;
    nDrvIdx = 0;
  }

  for (nFnameIdx = 0; nFnameIdx < FNAME_SIZE; nFnameIdx++)
  {
    if (*lpszFcbFname != ' ')
      pszBuffer[nDrvIdx + nFnameIdx] = *lpszFcbFname++;
    else
      break;
  }

  lpszFcbFext = (BYTE FAR *) lpFcb->fcb_fext;
  if (*lpszFcbFext != ' ')
  {
    pszBuffer[nDrvIdx + nFnameIdx++] = '.';
    for (nFextIdx = 0; nFextIdx < FEXT_SIZE; nFextIdx++)
    {
      if (*lpszFcbFext != ' ')
        pszBuffer[nDrvIdx + nFnameIdx + nFextIdx] =
            *lpszFcbFext++;
      else
        break;
    }
  }
  else
    nFextIdx = 0;
  pszBuffer[nDrvIdx + nFnameIdx + nFextIdx] = '\0';
}

BOOL FcbOpen(xfcb FAR * lpXfcb)
{
  WORD sft_idx;
  sft FAR *sftp;
  struct dhdr FAR *dhp;
  COUNT FcbDrive;

  /* get a free system file table entry                           */
  if ((sftp = FcbGetFreeSft((WORD FAR *) & sft_idx)) == (sft FAR *) - 1)
    return DE_TOOMANY;

  /* Build a traditional DOS file name                            */
  lpFcb = CommonFcbInit(lpXfcb, PriPathName, &FcbDrive);

  /* check for a device                                           */
  dhp = IsDevice(PriPathName);
  if (dhp )
  {
        sftp->sft_count += 1;
        sftp->sft_mode = O_RDWR;
        sftp->sft_attrib = 0;
        sftp->sft_flags =
            ((dhp->dh_attr & ~SFT_MASK) & ~SFT_FSHARED) | SFT_FDEVICE | SFT_FEOF;
        sftp->sft_psp = cu_psp;
        fbcopy(lpFcb->fcb_fname, sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
        sftp->sft_dev = dhp;
        lpFcb->fcb_sftno = sft_idx;
        lpFcb->fcb_curec = 0;
        lpFcb->fcb_recsiz = 0;
        lpFcb->fcb_fsize = 0;
        lpFcb->fcb_date = dos_getdate();
        lpFcb->fcb_time = dos_gettime();
        lpFcb->fcb_rndm = 0;
        return TRUE;
  }
  fbcopy((BYTE FAR *) & lpFcb->fcb_fname, (BYTE FAR *) & sftp->sft_name, FNAME_SIZE + FEXT_SIZE);
  if ((FcbDrive < 0) || (FcbDrive > (lastdrive -1))) {
    return DE_INVLDDRV;
  }
  if (CDSp->cds_table[FcbDrive].cdsFlags & CDSNETWDRV) {
    COUNT result;
    lpCurSft = (sfttbl FAR *)sftp;
    result = int2f_Remote_call(REM_OPEN, 0, 0, 0, (VOID FAR *) sftp, 0, MK_FP(0, O_RDWR));
    result = -result;
    sftp->sft_status = result;
  }
  else {
  sftp->sft_status = dos_open(PriPathName, O_RDWR);
    sftp->sft_size = dos_getfsize(sftp->sft_status);
    dos_getftime(sftp->sft_status,
	    (date FAR *) & sftp->sft_date,
	    (time FAR *) & sftp->sft_time);
		  
  }
  if (sftp->sft_status >= 0)
  {
    lpFcb->fcb_drive = FcbDrive;
    lpFcb->fcb_sftno = sft_idx;
    lpFcb->fcb_curec = 0;
    lpFcb->fcb_recsiz = 128;
    lpFcb->fcb_fsize = sftp->sft_size;
    lpFcb->fcb_date = sftp->sft_date;
    lpFcb->fcb_time = sftp->sft_time;
    lpFcb->fcb_rndm = 0;
    sftp->sft_count += 1;
    sftp->sft_mode = O_RDWR;
    sftp->sft_attrib = 0;
    sftp->sft_flags = 0;
    sftp->sft_psp = cu_psp;
    return TRUE;
  }
  else
    return FALSE;
}

BOOL FcbDelete(xfcb FAR * lpXfcb)
{
  COUNT FcbDrive;

  /* Build a traditional DOS file name                            */
  CommonFcbInit(lpXfcb, PriPathName, &FcbDrive);

  if ((FcbDrive < 0) || (FcbDrive > (lastdrive -1))) {
    return DE_INVLDDRV;
  }
  current_ldt = &CDSp->cds_table[FcbDrive];
  if (CDSp->cds_table[FcbDrive].cdsFlags & CDSNETWDRV) {
    return -int2f_Remote_call(REM_DELETE, 0, 0, 0, 0, 0, 0);
  }

  /* check for a device                                           */
  if (IsDevice(PriPathName))
  {
    return FALSE;
  }
  else
  {
    BYTE FAR *lpOldDta = dta;
    dmatch Dmatch;

    dta = (BYTE FAR *) & Dmatch;
    if (dos_findfirst(D_ALL, PriPathName[1] == ':' ? &PriPathName[2] : PriPathName) != SUCCESS)
    {
      dta = lpOldDta;
      return FALSE;
    }
    do
    {
      if (dos_delete(Dmatch.dm_name) != SUCCESS)
      {
        dta = lpOldDta;
        return FALSE;
      }
    }
    while (dos_findnext() == SUCCESS);
    dta = lpOldDta;
    return TRUE;
  }
}

BOOL FcbRename(xfcb FAR * lpXfcb)
{
  rfcb FAR *lpRenameFcb;
  COUNT FcbDrive;

  /* Build a traditional DOS file name                            */
  lpRenameFcb = (rfcb FAR *) CommonFcbInit(lpXfcb, PriPathName, &FcbDrive);

  /* check for a device                                           */
  if (IsDevice(PriPathName))
  {
    return FALSE;
  }
  else
  {
    BYTE FAR *lpOldDta = dta;
    dmatch Dmatch;

    dta = (BYTE FAR *) & Dmatch;
    if (dos_findfirst(D_ALL, PriPathName[1] == ':' ? &PriPathName[2] : PriPathName) != SUCCESS)
    {
      dta = lpOldDta;
      return FALSE;
    }

    do
    {
      fcb LocalFcb;
      BYTE *pToName,
       *pszFrom;
      BYTE FAR *pFromPattern;
      COUNT nIndex;

      /* First, expand the find match into fcb style  */
      /* file name entry                              */
      /* Fill with blanks first                       */
      for (pToName = LocalFcb.fcb_fname, nIndex = 0;
           nIndex < FNAME_SIZE; nIndex++)
      {
        *pToName++ = ' ';
      }
      for (pToName = LocalFcb.fcb_fext, nIndex = 0;
           nIndex < FEXT_SIZE; nIndex++)
      {
        *pToName++ = ' ';
      }

      /* next move in the file name while overwriting */
      /* the filler blanks                            */
      pszFrom = Dmatch.dm_name;
      pToName = LocalFcb.fcb_fname;
      for (nIndex = 0; nIndex < FNAME_SIZE; nIndex++)
      {
        if (*pszFrom != 0 && *pszFrom != '.')
          *pToName++ = *pszFrom++;
        else if (*pszFrom == '.')
        {
          ++pszFrom;
          break;
        }
        else
          break;
      }

      if (*pszFrom != '\0')
      {
        pToName = LocalFcb.fcb_fext;
        for (nIndex = 0; nIndex < FEXT_SIZE; nIndex++)
        {
          if (*pszFrom != '\0')
            *pToName++ = *pszFrom++;
          else
            break;
        }
      }

      /* Overlay the pattern, skipping '?'            */
      /* I'm cheating because this assumes that the   */
      /* struct alignments are on byte boundaries     */
      pToName = LocalFcb.fcb_fname;
      for (pFromPattern = lpRenameFcb->renNewName,
           nIndex = 0; nIndex < FNAME_SIZE + FEXT_SIZE; nIndex++)
      {
        if (*pFromPattern != '?')
          *pToName++ = *pFromPattern++;
        else
          ++pFromPattern;
      }

      /* now to build a dos name again                */
      LocalFcb.fcb_drive = 0;
      FcbNameInit((fcb FAR *) & LocalFcb, PriPathName, &FcbDrive);

      if (dos_rename(Dmatch.dm_name,
       PriPathName[1] == ':' ? &PriPathName[2] : PriPathName) != SUCCESS)
      {
        dta = lpOldDta;
        return FALSE;
      }
    }
    while (dos_findnext() == SUCCESS);
    dta = lpOldDta;
    return TRUE;
  }
}

void MoveDirInfo(dmatch FAR * lpDmatch, struct dirent FAR * lpDir)
{
  BYTE FAR *lpToName,
    FAR * lpszFrom;
  COUNT nIndex;

  /* First, expand the find match into dir style  */
  /* file name entry                              */
  /* Fill with blanks first                       */
  for (lpToName = lpDir->dir_name, nIndex = 0;
       nIndex < FNAME_SIZE; nIndex++)
  {
    *lpToName++ = ' ';
  }
  for (lpToName = lpDir->dir_ext, nIndex = 0;
       nIndex < FEXT_SIZE; nIndex++)
  {
    *lpToName++ = ' ';
  }

  /* next move in the file name while overwriting */
  /* the filler blanks                            */
  lpszFrom = lpDmatch->dm_name;
  lpToName = lpDir->dir_name;
  for (nIndex = 0; nIndex < FNAME_SIZE; nIndex++)
  {
    if (*lpszFrom != 0 && *lpszFrom != '.')
      *lpToName++ = *lpszFrom++;
    else
      break;
  }

  if (*lpszFrom != '\0')
  {
    if (*lpszFrom == '.')
      ++lpszFrom;
    lpToName = lpDir->dir_ext;
    for (nIndex = 0; nIndex < FEXT_SIZE; nIndex++)
    {
      if (*lpszFrom != '\0')
        *lpToName++ = *lpszFrom++;
      else
        break;
    }
  }
  for (nIndex = 0; nIndex < 10; nIndex++)
    lpDir->dir_reserved[nIndex] = 0;
  lpDir->dir_attrib = lpDmatch->dm_attr_fnd;
  lpDir->dir_time = lpDmatch->dm_time;
  lpDir->dir_date = lpDmatch->dm_date;
  lpDir->dir_start = lpDmatch->dm_cluster;
  lpDir->dir_size = lpDmatch->dm_size;
}

BOOL FcbClose(xfcb FAR * lpXfcb)
{
  sft FAR *s;

  /* Convert to fcb if necessary                                  */
  lpFcb = ExtFcbToFcb(lpXfcb);

  /* Get the SFT block that contains the SFT      */
  if ((s = FcbGetSft(lpFcb->fcb_sftno)) == (sft FAR *) - 1)
    return FALSE;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return FALSE;

  if (s->sft_flags & SFT_FSHARED)
  {
    int2f_Remote_call(REM_CLOSE, 0, 0, 0, (VOID FAR *) s, 0, 0);
  }

  /* now just drop the count if a device, else    */
  /* call file system handler                     */
  if (s->sft_flags & SFT_FDEVICE)
  {
    s->sft_count -= 1;
    return TRUE;
  }
  else
  {
    s->sft_count -= 1;
    if (s->sft_count > 0)
      return SUCCESS;
    else
    {
      /* change time and set file size                */
      dos_setftime(s->sft_status,
                   (date FAR *) & lpFcb->fcb_date,
                   (time FAR *) & lpFcb->fcb_time);
      dos_setfsize(s->sft_status,
                   lpFcb->fcb_fsize);
      return dos_close(s->sft_status) == SUCCESS;
    }
  }
}

BOOL FcbFindFirst(xfcb FAR * lpXfcb)
{
  BYTE FAR *lpOldDta;
  BYTE FAR *lpDir;
  COUNT nIdx,
    FcbDrive;
  psp FAR *lpPsp = MK_FP(cu_psp, 0);

  /* First, move the dta to a local and change it around to match */
  /* our functions.                                               */
  lpDir = (BYTE FAR *) dta;
  dta = (BYTE FAR *) & Dmatch;

  /* Next initialze local variables by moving them from the fcb   */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  if (lpXfcb->xfcb_flag == 0xff)
  {
    wAttr = lpXfcb->xfcb_attrib;
    fbcopy(lpXfcb, lpDir, 7);
    lpDir += 7;
  }
  else
    wAttr = D_ALL;

  *lpDir++ = FcbDrive;


  if (dos_findfirst(wAttr, SecPathName) != SUCCESS)
  {
    dta = lpPsp->ps_dta;
    return FALSE;
  }

  MoveDirInfo((dmatch FAR *) & Dmatch, (struct dirent FAR *)lpDir);

  lpFcb->fcb_dirclst = Dmatch.dm_cluster;
  lpFcb->fcb_diroff = Dmatch.dm_entry;

/*
    This is undocumented and seen using Pcwatch and Ramview.
    The First byte is the current directory count and the second seems
    to be the attribute byte.
 */
  lpFcb->fcb_sftno = Dmatch.dm_drive; /* MSD seems to save this @ fcb_date.*/
#if 0
  lpFcb->fcb_cublock = Dmatch.dm_entry;
  lpFcb->fcb_cublock *= 0x100;
  lpFcb->fcb_cublock += wAttr;
#endif

  dta = lpPsp->ps_dta;
  return TRUE;
}

BOOL FcbFindNext(xfcb FAR * lpXfcb)
{
  BYTE FAR *lpOldDta;
  BYTE FAR *lpDir;
  COUNT nIdx,
    FcbDrive;
  psp FAR *lpPsp = MK_FP(cu_psp, 0);

  /* First, move the dta to a local and change it around to match */
  /* our functions.                                               */
  lpDir = (BYTE FAR *) dta;
  dta = (BYTE FAR *) & Dmatch;

  /* Next initialze local variables by moving them from the fcb   */
  lpFcb = CommonFcbInit(lpXfcb, SecPathName, &FcbDrive);
  if ((xfcb FAR *) lpFcb != lpXfcb)
  {
    wAttr = lpXfcb->xfcb_attrib;
    fbcopy(lpXfcb, lpDir, 7);
    lpDir += 7;
  }
  else
    wAttr = D_ALL;

  /* Reconstrct the dirmatch structure from the fcb               */
  *lpDir++ = FcbDrive;
  Dmatch.dm_drive = lpFcb->fcb_sftno;

  fbcopy(lpFcb->fcb_fname, (BYTE FAR *) Dmatch.dm_name_pat, FNAME_SIZE + FEXT_SIZE);
  DosUpFMem((BYTE FAR *) Dmatch.dm_name_pat, FNAME_SIZE + FEXT_SIZE);

  Dmatch.dm_attr_srch = wAttr;
  Dmatch.dm_entry = lpFcb->fcb_diroff;
  Dmatch.dm_cluster = lpFcb->fcb_dirclst;

  if (dos_findnext() != SUCCESS)
  {
    dta = lpPsp->ps_dta;
    CritErrCode = 0x12;
    return FALSE;
  }

  MoveDirInfo((dmatch FAR *) & Dmatch, (struct dirent FAR *)lpDir);
  lpFcb->fcb_dirclst = Dmatch.dm_cluster;
  lpFcb->fcb_diroff = Dmatch.dm_entry;

  lpFcb->fcb_sftno = Dmatch.dm_drive;
#if 0
  lpFcb->fcb_cublock = Dmatch.dm_entry;
  lpFcb->fcb_cublock *= 0x100;
  lpFcb->fcb_cublock += wAttr;
#endif

  dta = lpPsp->ps_dta;
  return TRUE;
}
#endif


