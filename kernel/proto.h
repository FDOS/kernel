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
static BYTE *Proto_hRcsId =
    "$Id$";
#endif
#endif

/* blockio.c */
ULONG getblkno(struct buffer FAR *);
VOID setblkno(struct buffer FAR *, ULONG);
struct buffer FAR *getblock(ULONG blkno, COUNT dsk);
struct buffer FAR *getblockOver(ULONG blkno, COUNT dsk);
VOID setinvld(REG COUNT dsk);
BOOL flush_buffers(REG COUNT dsk);
BOOL flush1(struct buffer FAR * bp);
BOOL flush(void);
BOOL fill(REG struct buffer FAR * bp, ULONG blkno, COUNT dsk);
BOOL DeleteBlockInBufferCache(ULONG blknolow, ULONG blknohigh, COUNT dsk);
/* *** Changed on 9/4/00  BER */
UWORD dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks,
              COUNT mode);
/* *** End of change */

/* chario.c */
UCOUNT BinaryCharIO(struct dhdr FAR * dev, UCOUNT n, void FAR * bp, unsigned command, COUNT *err);
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
const char FAR *get_root(const char FAR *);
BOOL check_break(void);
UCOUNT GenericReadSft(sft far * sftp, UCOUNT n, void FAR * bp,
                      COUNT * err, BOOL force_binary);
COUNT SftSeek(sft FAR * sftp, LONG new_pos, COUNT mode);
/*COUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err); */
UCOUNT BinaryReadSft(sft FAR * s, void *bp, COUNT *err);
#define BinaryRead(hndl, bp, err) BinaryReadSft(get_sft(hndl), bp, err)
UCOUNT DosRWSft(sft FAR * s, UCOUNT n, void FAR * bp, COUNT *err, int mode);
#define DosRead(hndl, n, bp, err) DosRWSft(get_sft(hndl), n, bp, err, XFR_READ)
#define DosWrite(hndl, n, bp, err) DosRWSft(get_sft(hndl), n, bp, err, XFR_WRITE)
ULONG DosSeek(COUNT hndl, LONG new_pos, COUNT mode);
long DosOpen(char FAR * fname, unsigned flags, unsigned attrib);
COUNT CloneHandle(unsigned hndl);
long DosDup(unsigned Handle);
COUNT DosForceDup(unsigned OldHandle, unsigned NewHandle);
long DosOpenSft(char FAR * fname, unsigned flags, unsigned attrib);
COUNT DosClose(COUNT hndl);
COUNT DosCloseSft(WORD sft_idx, BOOL commitonly);
#define DosCommit(hndl) DosCloseSft(get_sft_idx(hndl), TRUE)
BOOL DosGetFree(UBYTE drive, UWORD * spc, UWORD * navc,
                UWORD * bps, UWORD * nc);
COUNT DosGetCuDir(UBYTE drive, BYTE FAR * s);
COUNT DosChangeDir(BYTE FAR * s);
COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name);
COUNT DosFindNext(void);
COUNT DosGetFtime(COUNT hndl, date * dp, time * tp);
COUNT DosSetFtimeSft(WORD sft_idx, date dp, time tp);
#define DosSetFtime(hndl, dp, tp) DosSetFtimeSft(get_sft_idx(hndl), (dp), (tp))
COUNT DosGetFattr(BYTE FAR * name);
COUNT DosSetFattr(BYTE FAR * name, UWORD attrp);
UBYTE DosSelectDrv(UBYTE drv);
COUNT DosDelete(BYTE FAR * path, int attrib);
COUNT DosRename(BYTE FAR * path1, BYTE FAR * path2);
COUNT DosRenameTrue(BYTE * path1, BYTE * path2, int attrib);
COUNT DosMkdir(const char FAR * dir);
COUNT DosRmdir(const char FAR * dir);
struct dhdr FAR *IsDevice(const char FAR * FileName);
BOOL IsShareInstalled(void);
COUNT DosLockUnlock(COUNT hndl, LONG pos, LONG len, COUNT unlock);
sft FAR *idx_to_sft(COUNT SftIndex);
COUNT get_sft_idx(UCOUNT hndl);
COUNT DosTruename(const char FAR * src, char FAR * dest);

/*dosidle.asm */
VOID ASMCFUNC DosIdle_int(void);

/* dosnames.c */
int ParseDosName(const char *, char *, BOOL);

/* error.c */
VOID dump(void);
VOID panic(BYTE * s);
VOID fatal(BYTE * err_msg);

/* fatdir.c */
VOID dir_init_fnode(f_node_ptr fnp, CLUSTER dirstart);
f_node_ptr dir_open(const char *dirname);
COUNT dir_read(REG f_node_ptr fnp);
BOOL dir_write(REG f_node_ptr fnp);
VOID dir_close(REG f_node_ptr fnp);
COUNT dos_findfirst(UCOUNT attr, BYTE * name);
COUNT dos_findnext(void);
void ConvertName83ToNameSZ(BYTE FAR * destSZ, BYTE FAR * srcFCBName);
int FileName83Length(BYTE * filename83);

/* fatfs.c */
ULONG clus2phys(CLUSTER cl_no, struct dpb FAR * dpbp);
long dos_open(char * path, unsigned flag, unsigned attrib);
BOOL fcbmatch(const char *fcbname1, const char *fcbname2);
BOOL fcmp_wild(const char * s1, const char * s2, unsigned n);
VOID touc(BYTE * s, COUNT n);
COUNT dos_close(COUNT fd);
COUNT dos_commit(COUNT fd);
COUNT dos_delete(BYTE * path, int attrib);
COUNT dos_rmdir(BYTE * path);
COUNT dos_rename(BYTE * path1, BYTE * path2, int attrib);
date dos_getdate(void);
time dos_gettime(void);
COUNT dos_getftime(COUNT fd, date FAR * dp, time FAR * tp);
COUNT dos_setftime(COUNT fd, date dp, time tp);
ULONG dos_getfsize(COUNT fd);
BOOL dos_setfsize(COUNT fd, LONG size);
COUNT dos_mkdir(BYTE * dir);
BOOL last_link(f_node_ptr fnp);
COUNT map_cluster(REG f_node_ptr fnp, COUNT mode);
UCOUNT rwblock(COUNT fd, VOID FAR * buffer, UCOUNT count, int mode);
COUNT dos_read(COUNT fd, VOID FAR * buffer, UCOUNT count);
COUNT dos_write(COUNT fd, const VOID FAR * buffer, UCOUNT count);
LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin);
CLUSTER dos_free(struct dpb FAR * dpbp);
BOOL dir_exists(char * path);

VOID trim_path(BYTE FAR * s);

COUNT dos_cd(struct cds FAR * cdsp, BYTE * PathName);

f_node_ptr get_f_node(void);
VOID release_f_node(f_node_ptr fnp);
VOID dos_setdta(BYTE FAR * newdta);
COUNT dos_getfattr_fd(COUNT fd);
COUNT dos_getfattr(BYTE * name);
COUNT dos_setfattr(BYTE * name, UWORD attrp);
COUNT media_check(REG struct dpb FAR * dpbp);
f_node_ptr xlt_fd(COUNT fd);
COUNT xlt_fnp(f_node_ptr fnp);
struct dhdr FAR * select_unit(COUNT drive);

/* fattab.c */
void read_fsinfo(struct dpb FAR * dpbp);
void write_fsinfo(struct dpb FAR * dpbp);
UCOUNT link_fat(struct dpb FAR * dpbp, CLUSTER Cluster1,
                REG CLUSTER Cluster2);
CLUSTER next_cluster(struct dpb FAR * dpbp, REG CLUSTER ClusterNum);

/* fcbfns.c */
VOID DosOutputString(BYTE FAR * s);
int DosCharInputEcho(VOID);
int DosCharInput(VOID);
VOID DosDirectConsoleIO(iregs FAR * r);
VOID DosCharOutput(COUNT c);
VOID DosDisplayOutput(COUNT c);
BYTE FAR *FatGetDrvData(UBYTE drive, UWORD * spc, UWORD * bps,
                   UWORD * nc);
UWORD FcbParseFname(int *wTestMode, const BYTE FAR *lpFileName, fcb FAR * lpFcb);
const BYTE FAR *ParseSkipWh(const BYTE FAR * lpFileName);
BOOL TestCmnSeps(BYTE FAR * lpFileName);
BOOL TestFieldSeps(BYTE FAR * lpFileName);
const BYTE FAR *GetNameField(const BYTE FAR * lpFileName, BYTE FAR * lpDestField,
                       COUNT nFieldSize, BOOL * pbWildCard);
UBYTE FcbReadWrite(xfcb FAR *, UCOUNT, int);
UBYTE FcbGetFileSize(xfcb FAR * lpXfcb);
void FcbSetRandom(xfcb FAR * lpXfcb);
UBYTE FcbRandomBlockIO(xfcb FAR * lpXfcb, COUNT nRecords, int mode);
UBYTE FcbRandomIO(xfcb FAR * lpXfcb, int mode);
UBYTE FcbOpen(xfcb FAR * lpXfcb, unsigned flags);
int FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
UBYTE FcbDelete(xfcb FAR * lpXfcb);
UBYTE FcbRename(xfcb FAR * lpXfcb);
UBYTE FcbClose(xfcb FAR * lpXfcb);
void FcbCloseAll(void);
UBYTE FcbFindFirstNext(xfcb FAR * lpXfcb, BOOL First);

/* ioctl.c */
COUNT DosDevIOctl(lregs * r);

/* memmgr.c */
seg far2para(VOID FAR * p);
seg long2para(ULONG size);
VOID FAR *add_far(VOID FAR * fp, ULONG off);
VOID FAR *adjust_far(const void FAR * fp);
COUNT DosMemAlloc(UWORD size, COUNT mode, seg FAR * para,
                  UWORD FAR * asize);
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
char * ASMCFUNC strcpy(char * d, const char * s);
void ASMCFUNC fmemcpyBack(void FAR * d, const void FAR * s, size_t n);
void ASMCFUNC fmemcpy(void FAR * d, const void FAR * s, size_t n);
void ASMCFUNC fstrcpy(char FAR * d, const char FAR * s);
void * ASMCFUNC memcpy(void *d, const void * s, size_t n);
void ASMCFUNC fmemset(void FAR * s, int ch, size_t n);
void * ASMCFUNC memset(void * s, int ch, size_t n);

int ASMCFUNC memcmp(const void *m1, const void *m2, size_t n);
int ASMCFUNC fmemcmp(const void FAR *m1, const void FAR *m2, size_t n);

/* lfnapi.c */
COUNT lfn_allocate_inode(VOID);
COUNT lfn_free_inode(COUNT handle);

COUNT lfn_setup_inode(COUNT handle, ULONG dirstart, ULONG diroff);

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
VOID DosUpString(char FAR * str);
VOID DosUpFMem(VOID FAR * str, unsigned len);
unsigned char DosUpFChar(unsigned char ch);
VOID DosUpFString(char FAR * str);
COUNT DosGetData(int subfct, UWORD cp, UWORD cntry, UWORD bufsize,
                 VOID FAR * buf);
#ifndef DosGetCountryInformation
COUNT DosGetCountryInformation(UWORD cntry, VOID FAR * buf);
#endif
#ifndef DosSetCountry
COUNT DosSetCountry(UWORD cntry);
#endif
COUNT DosGetCodepage(UWORD * actCP, UWORD * sysCP);
COUNT DosSetCodepage(UWORD actCP, UWORD sysCP);
UWORD ASMCFUNC syscall_MUX14(DIRECT_IREGS);

/* prf.c */
VOID put_console(COUNT c);
int CDECL printf(CONST BYTE * fmt, ...);
int CDECL sprintf(BYTE * buff, CONST BYTE * fmt, ...);
VOID hexd(char *title, VOID FAR * p, COUNT numBytes);

/* strings.c */
size_t ASMCFUNC strlen(const char * s);
size_t ASMCFUNC fstrlen(const char FAR * s);
char FAR * ASMCFUNC _fstrcpy(char FAR * d, const char FAR * s);
char * ASMCFUNC strncpy(char * d, const char * s, size_t l);
int ASMCFUNC strcmp(const char * d, const char * s);
int ASMCFUNC fstrcmp(const char FAR * d, const char FAR * s);
int ASMCFUNC fstrncmp(const char FAR * d, const char FAR * s, size_t l);
int ASMCFUNC strncmp(const char * d, const char * s, size_t l);
void ASMCFUNC fstrncpy(char FAR * d, const char FAR * s, size_t l);
char * ASMCFUNC strchr(const char * s, int c);
char FAR * ASMCFUNC fstrchr(const char FAR * s, int c);
void FAR * ASMCFUNC fmemchr(const void FAR * s, int c, size_t n);

/* sysclk.c */
COUNT BcdToByte(COUNT x);
COUNT BcdToWord(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);
COUNT ByteToBcd(COUNT x);
LONG WordToBcd(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);

/* syspack.c */
#ifdef NONNATIVE
VOID getdirent(UBYTE FAR * vp, struct dirent FAR * dp);
VOID putdirent(struct dirent FAR * dp, UBYTE FAR * vp);
#else
#define getdirent(vp, dp) fmemcpy(dp, vp, sizeof(struct dirent))
#define putdirent(dp, vp) fmemcpy(vp, dp, sizeof(struct dirent))
#endif

/* systime.c */
VOID DosGetTime(UBYTE * hp, UBYTE * mp, UBYTE * sp, UBYTE * hdp);
COUNT DosSetTime(BYTE h, BYTE m, BYTE s, BYTE hd);
VOID DosGetDate(UBYTE * wdp, UBYTE * mp, UBYTE * mdp, UWORD * yp);
COUNT DosSetDate(UWORD Month, UWORD DayOfMonth, UWORD Year);

const UWORD *is_leap_year_monthdays(UWORD year);
UWORD DaysFromYearMonthDay(UWORD Year, UWORD Month, UWORD DayOfMonth);

/* task.c */
VOID new_psp(psp FAR * p, int psize);
VOID return_user(void);
COUNT DosExec(COUNT mode, exec_blk FAR * ep, BYTE FAR * lp);
ULONG DosGetFsize(COUNT hndl);
VOID InitPSP(VOID);

/* newstuff.c */
int SetJFTSize(UWORD nHandles);
int DosMkTmp(BYTE FAR * pathname, UWORD attr);
COUNT get_verify_drive(const char FAR * src);
COUNT truename(const char FAR * src, char * dest, COUNT t);

/* network.c */
COUNT ASMCFUNC remote_doredirect(UWORD b, UCOUNT n, UWORD d, VOID FAR * s,
                                 UWORD i, VOID FAR * data);
COUNT ASMCFUNC remote_printset(UWORD b, UCOUNT n, UWORD d, VOID FAR * s,
                               UWORD i, VOID FAR * data);
COUNT ASMCFUNC remote_rename(VOID);
COUNT ASMCFUNC remote_delete(VOID);
COUNT ASMCFUNC remote_chdir(VOID);
COUNT ASMCFUNC remote_mkdir(VOID);
COUNT ASMCFUNC remote_rmdir(VOID);
COUNT ASMCFUNC remote_close_all(VOID);
COUNT ASMCFUNC remote_process_end(VOID);
COUNT ASMCFUNC remote_flushall(VOID);
COUNT ASMCFUNC remote_findfirst(VOID FAR * s);
COUNT ASMCFUNC remote_findnext(VOID FAR * s);
COUNT ASMCFUNC remote_getfattr(VOID);
COUNT ASMCFUNC remote_getfree(VOID FAR * s, VOID * d);
COUNT ASMCFUNC remote_open(sft FAR * s, COUNT mode);
int ASMCFUNC remote_extopen(sft FAR * s, unsigned attr);
LONG ASMCFUNC remote_lseek(sft FAR * s, LONG new_pos);
UCOUNT ASMCFUNC remote_read(sft FAR * s, UCOUNT n, COUNT * err);
UCOUNT ASMCFUNC remote_write(sft FAR * s, UCOUNT n, COUNT * err);
COUNT ASMCFUNC remote_creat(sft FAR * s, COUNT attr);
COUNT ASMCFUNC remote_setfattr(COUNT attr);
COUNT ASMCFUNC remote_printredir(UCOUNT dx, UCOUNT ax);
COUNT ASMCFUNC remote_commit(sft FAR * s);
COUNT ASMCFUNC remote_close(sft FAR * s);
COUNT ASMCFUNC QRemote_Fn(char FAR * d, const char FAR * s);

UWORD get_machine_name(BYTE FAR * netname);
VOID set_machine_name(BYTE FAR * netname, UWORD name_num);

/* procsupt.asm */
VOID ASMCFUNC exec_user(iregs FAR * irp);

/* new by TE */

/*
    assert at compile time, that something is true.
    
    use like 
        ASSERT_CONST( SECSIZE == 512) 
        ASSERT_CONST( (BYTE FAR *)x->fcb_ext - (BYTE FAR *)x->fcbname == 8)
*/

#define ASSERT_CONST(x) { typedef struct { char _xx[x ? 1 : -1]; } xx ; }

/*
 * Log: proto.h,v 
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
