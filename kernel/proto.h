/****************************************************************/
/*                                                              */
/*                          proto.h                             */
/*                                                              */
/*                   Global Function Prototypes                 */
/*                                                              */
/*                   Copyright (c) 1995, 1996                   */
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

#ifdef MAIN
#ifdef VERSION_STRINGS
static BYTE *Proto_hRcsId = "$Id$";
#endif
#endif

/*
 * $Log$
 * Revision 1.24  2001/11/13 23:36:45  bartoldeman
 * Kernel 2025a final changes.
 *
 * Revision 1.23  2001/11/04 19:47:39  bartoldeman
 * kernel 2025a changes: see history.txt
 *
 * Revision 1.22  2001/09/23 20:39:44  bartoldeman
 * FAT32 support, misc fixes, INT2F/AH=12 support, drive B: handling
 *
 * Revision 1.21  2001/08/19 12:58:36  bartoldeman
 * Time and date fixes, Ctrl-S/P, findfirst/next, FCBs, buffers, tsr unloading
 *
 * Revision 1.20  2001/07/24 16:56:29  bartoldeman
 * fixes for FCBs, DJGPP ls, DBLBYTE, dyninit allocation (2024e).
 *
 * Revision 1.19  2001/07/23 12:47:42  bartoldeman
 * FCB fixes and clean-ups, exec int21/ax=4b01, initdisk.c printf
 *
 * Revision 1.18  2001/07/22 01:58:58  bartoldeman
 * Support for Brian's FORMAT, DJGPP libc compilation, cleanups, MSCDEX
 *
 * Revision 1.17  2001/06/03 14:16:18  bartoldeman
 * BUFFERS tuning and misc bug fixes/cleanups (2024c).
 *
 * Revision 1.16  2001/04/29 17:34:40  bartoldeman
 * A new SYS.COM/config.sys single stepping/console output/misc fixes.
 *
 * Revision 1.15  2001/04/21 22:32:53  bartoldeman
 * Init DS=Init CS, fixed stack overflow problems and misc bugs.
 *
 * Revision 1.14  2001/04/16 01:45:26  bartoldeman
 * Fixed handles, config.sys drivers, warnings. Enabled INT21/AH=6C, printf %S/%Fs
 *
 * Revision 1.13  2001/04/15 03:21:50  bartoldeman
 * See history.txt for the list of fixes.
 *
 * Revision 1.12  2001/03/30 19:30:06  bartoldeman
 * Misc fixes and implementation of SHELLHIGH. See history.txt for details.
 *
 * Revision 1.11  2001/03/27 20:27:43  bartoldeman
 * dsk.c (reported by Nagy Daniel), inthndlr and int25/26 fixes by Tom Ehlert.
 *
 * Revision 1.10  2001/03/22 04:55:36  bartoldeman
 * Fix prototypes.
 *
 * Revision 1.9  2001/03/21 02:56:26  bartoldeman
 * See history.txt for changes. Bug fixes and HMA support are the main ones.
 *
 * Revision 1.8  2000/10/30 00:21:15  jimtabor
 * Adding Brian Reifsnyder Fix for Int 25/26
 *
 * Revision 1.7  2000/08/07 22:51:34  jimtabor
 * Remove unused code
 *
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
 * Revision 1.17  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.16  2000/03/17 22:59:04  kernel
 * Steffen Kaiser's NLS changes
 *
 * Revision 1.15  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.14  1999/09/23 04:40:48  jprice
 * *** empty log message ***
 *
 * Revision 1.10  1999/08/25 03:18:09  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.9  1999/05/03 06:25:45  jprice
 * Patches from ror4 and many changed of signed to unsigned variables.
 *
 * Revision 1.8  1999/04/23 04:24:39  jprice
 * Memory manager changes made by ska
 *
 * Revision 1.7  1999/04/16 21:43:40  jprice
 * ror4 multi-sector IO
 *
 * Revision 1.6  1999/04/16 12:21:22  jprice
 * Steffen c-break handler changes
 *
 * Revision 1.5  1999/04/12 03:21:17  jprice
 * more ror4 patches.  Changes for multi-block IO
 *
 * Revision 1.4  1999/04/11 04:33:39  jprice
 * ror4 patches
 *
 * Revision 1.2  1999/04/04 18:51:43  jprice
 * no message
 *
 * Revision 1.1.1.1  1999/03/29 15:41:30  jprice
 * New version without IPL.SYS
 *
 * Revision 1.4  1999/02/08 05:55:57  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.3  1999/02/01 01:48:41  jprice
 * Clean up; Now you can use hex numbers in config.sys. added config.sys screen function to change screen mode (28 or 43/50 lines)
 *
 * Revision 1.2  1999/01/22 04:13:27  jprice
 * Formating
 *
 * Revision 1.1.1.1  1999/01/20 05:51:01  jprice
 * Imported sources
 *
 *
 *   Rev 1.11   06 Dec 1998  8:47:18   patv
 *Expanded due to new I/O subsystem.
 *
 *   Rev 1.10   07 Feb 1998 20:38:00   patv
 *Modified stack fram to match DOS standard
 *
 *   Rev 1.9   22 Jan 1998  4:09:26   patv
 *Fixed pointer problems affecting SDA
 *
 *   Rev 1.8   11 Jan 1998  2:06:22   patv
 *Added functionality to ioctl.
 *
 *   Rev 1.7   04 Jan 1998 23:16:22   patv
 *Changed Log for strip utility
 *
 *   Rev 1.6   03 Jan 1998  8:36:48   patv
 *Converted data area to SDA format
 *
 *   Rev 1.5   16 Jan 1997 12:46:44   patv
 *pre-Release 0.92 feature additions
 *
 *   Rev 1.4   29 May 1996 21:03:40   patv
 *bug fixes for v0.91a
 *
 *   Rev 1.3   19 Feb 1996  3:23:06   patv
 *Added NLS, int2f and config.sys processing
 *
 *   Rev 1.2   01 Sep 1995 17:54:26   patv
 *First GPL release.
 *
 *   Rev 1.1   30 Jul 1995 20:51:58   patv
 *Eliminated version strings in ipl
 *
 *   Rev 1.0   05 Jul 1995 11:32:16   patv
 *Initial revision.
 */

/* blockio.c */
ULONG getblkno(struct buffer FAR *);
VOID setblkno(struct buffer FAR *, ULONG);
struct buffer FAR *getblock    (ULONG blkno, COUNT dsk);
struct buffer FAR *getblockOver(ULONG blkno, COUNT dsk);
VOID setinvld(REG COUNT dsk);
BOOL flush_buffers(REG COUNT dsk);
BOOL flush1(struct buffer FAR * bp);
BOOL flush(void);
BOOL fill(REG struct buffer FAR * bp, ULONG blkno, COUNT dsk);
BOOL DeleteBlockInBufferCache(ULONG blknolow, ULONG blknohigh, COUNT dsk);
/* *** Changed on 9/4/00  BER */
UWORD dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks, COUNT mode);
/* *** End of change */

/* chario.c */
VOID sto(COUNT c);
VOID cso(COUNT c);
VOID mod_cso(REG UCOUNT c);
VOID destr_bs(void);
UCOUNT _sti(BOOL check_break);
VOID con_hold(void);
BOOL con_break(void);
BOOL StdinBusy(void);
VOID KbdFlush(void);
VOID Do_DosIdle_loop(void);
UCOUNT sti_0a(keyboard FAR * kp);
UCOUNT sti(keyboard * kp);

sft FAR *get_sft(UCOUNT);

/* dosfns.c */
#ifdef WITHFAT32
struct dpb FAR *GetDriveDPB(UBYTE drive, COUNT *rc);
#endif
BYTE FAR *get_root(BYTE FAR *);
BOOL fnmatch(BYTE FAR *, BYTE FAR *, COUNT, COUNT);
BOOL check_break(void);
UCOUNT GenericReadSft(sft far *sftp, UCOUNT n, BYTE FAR * bp, COUNT FAR * err,
		   BOOL force_binary);
COUNT SftSeek(sft FAR *sftp, LONG new_pos, COUNT mode);
/* COUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err); */
#define GenericRead(hndl, n, bp, err, t) GenericReadSft(get_sft(hndl), n, bp, err, t)
#define DosRead(hndl, n, bp, err) GenericRead(hndl, n, bp, err, FALSE)
#define DosReadSft(sftp, n, bp, err) GenericReadSft(sftp, n, bp, err, FALSE)
UCOUNT DosWriteSft(sft FAR *sftp, UCOUNT n, BYTE FAR * bp, COUNT FAR * err);
#define DosWrite(hndl, n, bp, err) DosWriteSft(get_sft(hndl), n, bp, err)
COUNT DosSeek(COUNT hndl, LONG new_pos, COUNT mode, ULONG * set_pos);
COUNT DosCreat(BYTE FAR * fname, COUNT attrib);
COUNT DosCreatSft(BYTE * fname, COUNT attrib);
COUNT CloneHandle(COUNT hndl);
COUNT DosDup(COUNT Handle);
COUNT DosForceDup(COUNT OldHandle, COUNT NewHandle);
COUNT DosOpen(BYTE FAR * fname, COUNT mode);
COUNT DosOpenSft(BYTE * fname, COUNT mode);
COUNT DosClose(COUNT hndl);
COUNT DosCloseSft(WORD sft_idx);
BOOL DosGetFree(UBYTE drive, UCOUNT FAR * spc, UCOUNT FAR * navc, UCOUNT FAR * bps, UCOUNT FAR * nc);
COUNT DosGetExtFree(BYTE FAR *DriveString, struct xfreespace FAR *xfsp);
COUNT DosGetCuDir(UBYTE drive, BYTE FAR * s);
COUNT DosChangeDir(BYTE FAR * s);
COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name);
COUNT DosFindNext(void);
COUNT DosGetFtime(COUNT hndl, date FAR * dp, time FAR * tp);
COUNT DosSetFtimeSft(WORD sft_idx, date dp, time tp);
#define DosSetFtime(hndl, dp, tp) DosSetFtimeSft(get_sft_idx(hndl), (dp), (tp))
COUNT DosGetFattr(BYTE FAR * name);
COUNT DosSetFattr(BYTE FAR * name, UWORD attrp);
UBYTE DosSelectDrv(UBYTE drv);
COUNT DosDelete(BYTE FAR *path);
COUNT DosRename(BYTE FAR * path1, BYTE FAR * path2);
COUNT DosRenameTrue(BYTE * path1, BYTE * path2);
COUNT DosMkdir(BYTE FAR * dir);
COUNT DosRmdir(BYTE FAR * dir);
struct dhdr FAR * IsDevice(BYTE FAR * FileName);
BOOL IsShareInstalled(void);
COUNT DosLockUnlock(COUNT hndl, LONG pos, LONG len, COUNT unlock);
sft FAR *idx_to_sft(COUNT SftIndex);
COUNT get_sft_idx(UCOUNT hndl);

/*dosidle.asm */
VOID ASMCFUNC DosIdle_int(void);

/* dosnames.c */
VOID SpacePad(BYTE *, COUNT);
COUNT ParseDosName(BYTE *, COUNT *, BYTE *, BYTE *, BYTE *, BOOL);
/* COUNT ParseDosPath(BYTE *, COUNT *, BYTE *, BYTE FAR *); */

/* dsk.c */
COUNT FAR ASMCFUNC blk_driver(rqptr rp);
ddt *getddt(int dev);

/* error.c */
VOID dump(void);
VOID panic(BYTE * s);
VOID fatal(BYTE * err_msg);
COUNT char_error(request * rq, struct dhdr FAR * lpDevice);
COUNT block_error(request * rq, COUNT nDrive, struct dhdr FAR * lpDevice);

/* fatdir.c */
VOID dir_init_fnode(f_node_ptr fnp, CLUSTER dirstart);
f_node_ptr dir_open(BYTE * dirname);
COUNT dir_read(REG f_node_ptr fnp);
BOOL dir_write(REG f_node_ptr fnp);
VOID dir_close(REG f_node_ptr fnp);
COUNT dos_findfirst(UCOUNT attr, BYTE * name);
COUNT dos_findnext(void);
void ConvertName83ToNameSZ(BYTE FAR *destSZ, BYTE FAR *srcFCBName);
int FileName83Length(BYTE *filename83);

/* fatfs.c */
ULONG clus2phys(CLUSTER cl_no, struct dpb FAR *dpbp);
COUNT dos_open(BYTE * path, COUNT flag);
BOOL fcmp(BYTE * s1, BYTE * s2, COUNT n);
BOOL fcmp_wild(BYTE FAR * s1, BYTE FAR * s2, COUNT n);
VOID touc(BYTE * s, COUNT n);
COUNT dos_close(COUNT fd);
COUNT dos_creat(BYTE * path, COUNT attrib);
COUNT dos_delete(BYTE * path);
COUNT dos_rmdir(BYTE * path);
COUNT dos_rename(BYTE * path1, BYTE * path2);
date dos_getdate(void);
time dos_gettime(void);
COUNT dos_getftime(COUNT fd, date FAR * dp, time FAR * tp);
COUNT dos_setftime(COUNT fd, date dp, time tp);
LONG dos_getcufsize(COUNT fd);
LONG dos_getfsize(COUNT fd);
BOOL dos_setfsize(COUNT fd, LONG size);
COUNT dos_mkdir(BYTE * dir);
BOOL last_link(f_node_ptr fnp);
COUNT map_cluster(REG f_node_ptr fnp, COUNT mode);
UCOUNT readblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err);
UCOUNT writeblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err);
COUNT dos_read(COUNT fd, VOID FAR * buffer, UCOUNT count);
COUNT dos_write(COUNT fd, VOID FAR * buffer, UCOUNT count);
LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin);
CLUSTER dos_free(struct dpb FAR *dpbp);

VOID trim_path(BYTE FAR * s);

COUNT dos_cd(struct cds FAR * cdsp, BYTE * PathName);

f_node_ptr get_f_node(void);
VOID release_f_node(f_node_ptr fnp);
VOID dos_setdta(BYTE FAR * newdta);
COUNT dos_getfattr(BYTE * name);
COUNT dos_setfattr(BYTE * name, UWORD attrp);
COUNT media_check(REG struct dpb FAR *dpbp);
f_node_ptr xlt_fd(COUNT fd);
COUNT xlt_fnp(f_node_ptr fnp);
struct dhdr FAR *select_unit(COUNT drive);
#ifdef WITHFAT32
VOID bpb_to_dpb(bpb FAR *bpbp, REG struct dpb FAR * dpbp, BOOL extended);
#else
VOID bpb_to_dpb(bpb FAR *bpbp, REG struct dpb FAR * dpbp);
#endif

/* fattab.c */
void read_fsinfo(struct dpb FAR *dpbp);
void write_fsinfo(struct dpb FAR *dpbp);
UCOUNT link_fat(struct dpb FAR *dpbp, CLUSTER Cluster1, REG CLUSTER Cluster2);
UCOUNT link_fat32(struct dpb FAR *dpbp, CLUSTER Cluster1, CLUSTER Cluster2);
UCOUNT link_fat16(struct dpb FAR *dpbp, CLUSTER Cluster1, CLUSTER Cluster2);
UCOUNT link_fat12(struct dpb FAR *dpbp, CLUSTER Cluster1, CLUSTER Cluster2);
CLUSTER next_cluster(struct dpb FAR *dpbp, REG CLUSTER ClusterNum);
CLUSTER next_cl32(struct dpb FAR *dpbp, REG CLUSTER ClusterNum);
CLUSTER next_cl16(struct dpb FAR *dpbp, REG CLUSTER ClusterNum);
CLUSTER next_cl12(struct dpb FAR *dpbp, REG CLUSTER ClusterNum);

/* fcbfns.c */
VOID DosOutputString(BYTE FAR * s);
int DosCharInputEcho(VOID);
int DosCharInput(VOID);
VOID DosDirectConsoleIO(iregs FAR * r);
VOID DosCharOutput(COUNT c);
VOID DosDisplayOutput(COUNT c);
VOID FatGetDrvData(UCOUNT drive, UCOUNT FAR * spc, UCOUNT FAR * bps, UCOUNT FAR * nc, BYTE FAR ** mdp);
WORD FcbParseFname(int wTestMode, BYTE FAR ** lpFileName, fcb FAR * lpFcb);
BYTE FAR *ParseSkipWh(BYTE FAR * lpFileName);
BOOL TestCmnSeps(BYTE FAR * lpFileName);
BOOL TestFieldSeps(BYTE FAR * lpFileName);
BYTE FAR *GetNameField(BYTE FAR * lpFileName, BYTE FAR * lpDestField, COUNT nFieldSize, BOOL * pbWildCard);
BOOL FcbRead(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbWrite(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbGetFileSize(xfcb FAR * lpXfcb);
BOOL FcbSetRandom(xfcb FAR * lpXfcb);
BOOL FcbCalcRec(xfcb FAR * lpXfcb);
BOOL FcbRandomBlockRead(xfcb FAR * lpXfcb, COUNT nRecords, COUNT * nErrorCode);
BOOL FcbRandomBlockWrite(xfcb FAR * lpXfcb, COUNT nRecords, COUNT * nErrorCode);
BOOL FcbRandomIO(xfcb FAR * lpXfcb, COUNT * nErrorCode,
                 BOOL (*FcbFunc)(xfcb FAR *, COUNT *));
BOOL FcbCreate(xfcb FAR * lpXfcb);
void FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
BOOL FcbOpen(xfcb FAR * lpXfcb);
BOOL FcbDelete(xfcb FAR * lpXfcb);
BOOL FcbRename(xfcb FAR * lpXfcb);
BOOL FcbClose(xfcb FAR * lpXfcb);
VOID FcbCloseAll(VOID);
BOOL FcbFindFirst(xfcb FAR * lpXfcb);
BOOL FcbFindNext(xfcb FAR * lpXfcb);

/* ioctl.c */
COUNT DosDevIOctl(iregs FAR * r);

/* memmgr.c */
seg far2para(VOID FAR * p);
seg long2para(ULONG size);
VOID FAR *add_far(VOID FAR * fp, ULONG off);
VOID FAR *adjust_far(VOID FAR * fp);
COUNT DosMemAlloc(UWORD size, COUNT mode, seg FAR * para, UWORD FAR * asize);
COUNT DosMemLargest(UWORD FAR * size);
COUNT DosMemFree(UWORD para);
COUNT DosMemChange(UWORD para, UWORD size, UWORD * maxSize);
COUNT DosMemCheck(void);
COUNT FreeProcessMem(UWORD ps);
COUNT DosGetLargestBlock(UWORD FAR * block);
VOID show_chain(void);
VOID DosUmbLink(BYTE n);
VOID mcb_print(mcb FAR * mcbp);

/* misc.c */
/*
VOID scopy(REG BYTE * s, REG BYTE * d);
#define strcpy(d, s)    scopy(s, d)
VOID fbcopy(REG VOID FAR * s, REG VOID FAR * d, REG COUNT n);

*/
VOID strcpy(REG BYTE * d, REG BYTE * s);
#define scopy(s, d)    strcpy(d,s)
VOID ASMCFUNC fmemcpy(REG VOID FAR * d, REG VOID FAR * s, REG COUNT n);
#define fbcopy(s, d, n)    fmemcpy(d,s,n)


/*VOID fscopy(REG BYTE FAR * s, REG BYTE FAR * d);*/
VOID ASMCFUNC fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s);
#define fscopy(s,d) fstrcpy(d,s)

VOID ASMCFUNC fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s);

/*VOID bcopy(REG BYTE * s, REG BYTE * d, REG COUNT n);*/
void ASMCFUNC memcpy(REG void * d, REG VOID * s, REG COUNT n);
#define bcopy(s,d,n) memcpy(d,s,n)

void ASMCFUNC fmemset(REG VOID FAR * s, REG int ch, REG COUNT n);
void ASMCFUNC memset(REG VOID     * s, REG int ch, REG COUNT n);

/* lfnapi.c */
COUNT lfn_allocate_inode(VOID);
COUNT lfn_free_inode(COUNT handle);

COUNT lfn_setup_inode(COUNT handle, CLUSTER dirstart, ULONG diroff);

COUNT lfn_create_entries(COUNT handle, lfn_inode_ptr lip);
COUNT lfn_remove_entries(COUNT handle);

COUNT lfn_dir_read(COUNT handle, lfn_inode_ptr lip);
COUNT lfn_dir_write(COUNT handle);

/* nls.c */
BYTE DosYesNo(unsigned char ch);
#ifndef DosUpMem
VOID DosUpMem(VOID FAR * str, unsigned len);
#endif
unsigned char ASMCFUNC DosUpChar(unsigned char ch);
VOID DosUpString(char FAR *str);
VOID DosUpFMem(VOID FAR *str, unsigned len);
unsigned char DosUpFChar(unsigned char ch);
VOID DosUpFString(char FAR *str);
COUNT DosGetData(int subfct, UWORD cp, UWORD cntry
   , UWORD bufsize, VOID FAR * buf);
#ifndef DosGetCountryInformation
COUNT DosGetCountryInformation(UWORD cntry, VOID FAR *buf);
#endif
#ifndef DosSetCountry
COUNT DosSetCountry(UWORD cntry);
#endif
COUNT DosGetCodepage(UWORD FAR* actCP, UWORD FAR* sysCP);
COUNT DosSetCodepage(UWORD actCP, UWORD sysCP);
UWORD ASMCFUNC syscall_MUX14(DIRECT_IREGS);

/* prf.c */
VOID put_console(COUNT c);
WORD printf(CONST BYTE * fmt,...);
WORD sprintf(BYTE * buff, CONST BYTE * fmt, ...);
VOID hexd(char *title,VOID FAR *p,COUNT numBytes);

/* strings.c */
COUNT ASMCFUNC strlen(REG BYTE * s);
COUNT ASMCFUNC fstrlen(REG BYTE FAR * s);
VOID ASMCFUNC _fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s);
VOID ASMCFUNC strncpy(REG BYTE * d, REG BYTE * s, COUNT l);
COUNT ASMCFUNC strcmp(REG BYTE * d, REG BYTE * s);
COUNT ASMCFUNC fstrcmp(REG BYTE FAR * d, REG BYTE FAR * s);
COUNT ASMCFUNC fstrncmp(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l);
COUNT ASMCFUNC strncmp(REG BYTE * d, REG BYTE * s, COUNT l);
/*
void fsncopy(REG BYTE FAR * s, REG BYTE FAR * d, COUNT l);
#define fstrncpy(d,s,l) fsncopy(s,d,l)
*/
void ASMCFUNC fstrncpy(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l);
#define fsncopy(s,d,l) fstrncpy(d,s,l)

BYTE * ASMCFUNC strchr(BYTE * s, BYTE c);

/* sysclk.c */
WORD FAR ASMCFUNC clk_driver(rqptr rp);
COUNT BcdToByte(COUNT x);
COUNT BcdToWord(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);
COUNT ByteToBcd(COUNT x);
LONG WordToBcd(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);

/* syspack.c */
#ifdef NONNATIVE
VOID getdirent(BYTE FAR * vp, struct dirent FAR * dp);
VOID putdirent(struct dirent FAR * dp, BYTE FAR * vp);
#else
#define getdirent(vp, dp) fmemcpy(dp, vp, sizeof(struct dirent))
#define putdirent(dp, vp) fmemcpy(vp, dp, sizeof(struct dirent))
#endif

/* syscon.c */
WORD con_driver(rqptr rp);
VOID break_handler(void);

/* systime.c */
VOID DosGetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp, BYTE FAR * hdp);
COUNT DosSetTime(BYTE h, BYTE m, BYTE s, BYTE hd);
VOID DosGetDate(BYTE FAR * wdp, BYTE FAR * mp, BYTE FAR * mdp, COUNT FAR * yp);
COUNT DosSetDate(UWORD Month, UWORD DayOfMonth, UWORD Year);

UWORD  *is_leap_year_monthdays(UWORD year);
UWORD DaysFromYearMonthDay(UWORD Year, UWORD Month, UWORD DayOfMonth);



/* task.c */
COUNT ChildEnv(exec_blk FAR * exp, UWORD * pChildEnvSeg, char far * pathname);
VOID new_psp(psp FAR * p, int psize);
VOID return_user(void);
COUNT DosExec(COUNT mode, exec_blk FAR * ep, BYTE FAR * lp);
LONG DosGetFsize(COUNT hndl);
VOID InitPSP(VOID);

/* newstuff.c */
int SetJFTSize(UWORD nHandles);
int DosMkTmp(BYTE FAR * pathname, UWORD attr);
COUNT get_verify_drive(char FAR * src);
COUNT ASMCFUNC truename(char FAR * src, char FAR * dest, COUNT t);

/* network.c */
COUNT ASMCFUNC remote_doredirect(UWORD b, UCOUNT n, UWORD d, VOID FAR * s, UWORD i, VOID FAR * data);
COUNT ASMCFUNC remote_printset(UWORD b, UCOUNT n, UWORD d, VOID FAR * s, UWORD i, VOID FAR * data);
COUNT ASMCFUNC remote_rename(VOID);
COUNT ASMCFUNC remote_delete(VOID);
COUNT ASMCFUNC remote_chdir(VOID);
COUNT ASMCFUNC remote_mkdir(VOID);
COUNT ASMCFUNC remote_rmdir(VOID);
COUNT ASMCFUNC remote_close_all(VOID);
COUNT ASMCFUNC remote_process_end(VOID);
COUNT ASMCFUNC remote_flushall(VOID);
COUNT ASMCFUNC remote_findfirst(VOID FAR *s);
COUNT ASMCFUNC remote_findnext(VOID FAR *s);
COUNT ASMCFUNC remote_getfattr(VOID);
COUNT ASMCFUNC remote_getfree(VOID FAR *s, VOID *d);
COUNT ASMCFUNC remote_open(sft FAR *s, COUNT mode);
LONG ASMCFUNC remote_lseek(sft FAR *s, LONG new_pos);
UCOUNT ASMCFUNC remote_read(sft FAR *s, UCOUNT n, COUNT * err);
UCOUNT ASMCFUNC remote_write(sft FAR *s, UCOUNT n, COUNT * err);
COUNT ASMCFUNC remote_creat(sft FAR *s, COUNT attr);
COUNT ASMCFUNC remote_setfattr(COUNT attr);
COUNT ASMCFUNC remote_printredir(UCOUNT dx, UCOUNT ax);
COUNT ASMCFUNC remote_close(sft FAR *s);
COUNT ASMCFUNC QRemote_Fn(char FAR * s, char FAR * d);

UWORD get_machine_name(BYTE FAR * netname);
VOID set_machine_name(BYTE FAR * netname, UWORD name_num);

/* procsupt.asm */
VOID ASMCFUNC exec_user(iregs FAR * irp);

/* detect.c */
unsigned long FAR is_dosemu(void);


/* new by TE */

/*
    assert at compile time, that something is true.
    
    use like 
        ASSERT_CONST( SECSIZE == 512) 
        ASSERT_CONST( (BYTE FAR *)x->fcb_ext - (BYTE FAR *)x->fcbname == 8)
*/

#define ASSERT_CONST(x) { typedef struct { char _xx[x ? 1 : -1]; } xx ; }

#if defined(WATCOM) && 0
ULONG FAR ASMCFUNC MULULUS(ULONG mul1, UWORD mul2); /* MULtiply ULong by UShort */
ULONG FAR ASMCFUNC MULULUL(ULONG mul1, ULONG mul2); /* MULtiply ULong by ULong */
ULONG FAR ASMCFUNC DIVULUS(ULONG mul1, UWORD mul2); /* DIVide ULong by UShort */
ULONG FAR ASMCFUNC DIVMODULUS(ULONG mul1, UWORD mul2,UWORD *rem); /* DIVide ULong by UShort */
#endif
