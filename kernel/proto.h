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

#define INIT

#ifdef IN_INIT_MOD
#define __FAR_WRAPPER(ret, name, proto) \
	ret FAR name proto;	/* will be expanded to `init_call_<name>' */
#else
#define __FAR_WRAPPER(ret, name, proto) \
	ret name proto; \
	ret FAR init_call_##name proto;
#endif

/* blockio.c */
VOID FAR init_buffers(void);
ULONG getblkno(struct buffer FAR *);
VOID setblkno(struct buffer FAR *, ULONG);
struct buffer FAR *getblock(ULONG blkno, COUNT dsk);
BOOL getbuf(struct buffer FAR ** pbp, ULONG blkno, COUNT dsk);
VOID setinvld(REG COUNT dsk);
BOOL flush_buffers(REG COUNT dsk);
BOOL flush1(struct buffer FAR * bp);
BOOL flush(void);
BOOL fill(REG struct buffer FAR * bp, ULONG blkno, COUNT dsk);
BOOL dskxfer(COUNT dsk, ULONG blkno, VOID FAR * buf, UWORD numblocks, COUNT mode);

/* chario.c */
VOID cso(COUNT c);
VOID sto(COUNT c);
VOID mod_sto(REG UCOUNT c);
VOID destr_bs(void);
UCOUNT _sti(void);
VOID con_hold(void);
BOOL con_break(void);
BOOL StdinBusy(void);
VOID KbdFlush(void);
VOID Do_DosIdle_loop(void);
__FAR_WRAPPER(VOID, sti, (keyboard FAR * kp))

sft FAR *get_sft(COUNT);

/* config.c */
INIT VOID PreConfig(VOID);
INIT VOID DoConfig(VOID);
INIT VOID PostConfig(VOID);
INIT BYTE *skipwh(BYTE * s);
INIT BYTE *scan(BYTE * s, BYTE * d);
INIT BOOL isnum(BYTE * pszString);
INIT BYTE *GetNumber(REG BYTE * pszString, REG COUNT * pnNum);
INIT COUNT tolower(COUNT c);
INIT COUNT toupper(COUNT c);
INIT VOID mcb_init(mcb FAR * mcbp, UWORD size);
INIT VOID umcb_init(mcb FAR * mcbp, UWORD size);
INIT VOID strcat(REG BYTE * d, REG BYTE * s);

/* dosfns.c */
BYTE FAR *get_root(BYTE FAR *);
BOOL fnmatch(BYTE FAR *, BYTE FAR *, COUNT, COUNT);
BOOL check_break(void);
UCOUNT GenericRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err,
                   BOOL force_binary);
COUNT SftSeek(sft FAR *sftp, LONG new_pos, COUNT mode);
UCOUNT DosRead(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err);
UCOUNT DosWrite(COUNT hndl, UCOUNT n, BYTE FAR * bp, COUNT FAR * err);
COUNT DosSeek(COUNT hndl, LONG new_pos, COUNT mode, ULONG * set_pos);
COUNT DosCreat(BYTE FAR * fname, COUNT attrib);
COUNT CloneHandle(COUNT hndl);
COUNT DosDup(COUNT Handle);
COUNT DosForceDup(COUNT OldHandle, COUNT NewHandle);
COUNT DosOpen(BYTE FAR * fname, COUNT mode);
COUNT DosClose(COUNT hndl);
VOID DosGetFree(COUNT drive, COUNT FAR * spc, COUNT FAR * navc, COUNT FAR * bps, COUNT FAR * nc);
COUNT DosGetCuDir(COUNT drive, BYTE FAR * s);
COUNT DosChangeDir(BYTE FAR * s);
COUNT DosFindFirst(UCOUNT attr, BYTE FAR * name);
COUNT DosFindNext(void);
COUNT DosGetFtime(COUNT hndl, date FAR * dp, time FAR * tp);
COUNT DosSetFtime(COUNT hndl, date FAR * dp, time FAR * tp);
COUNT DosGetFattr(BYTE FAR * name, UWORD FAR * attrp);
COUNT DosSetFattr(BYTE FAR * name, UWORD FAR * attrp);
BYTE DosSelectDrv(BYTE drv);
COUNT DosDelete(BYTE FAR *path);
COUNT DosRename(BYTE FAR * path1, BYTE FAR * path2);
COUNT DosMkdir(BYTE FAR * dir);
COUNT DosRmdir(BYTE FAR * dir);
struct dhdr FAR * IsDevice(BYTE FAR * FileName);

/*dosidle.asm */
VOID DosIdle_int(void);

/* dosnames.c */
VOID SpacePad(BYTE *, COUNT);
COUNT ParseDosName(BYTE FAR *, COUNT *, BYTE *, BYTE *, BYTE *, BOOL);
COUNT ParseDosPath(BYTE FAR *, COUNT *, BYTE *, BYTE FAR *);

/* dsk.c */
COUNT blk_driver(rqptr rp);

/* error.c */
VOID dump(void);
VOID panic(BYTE * s);
__FAR_WRAPPER(VOID, fatal, (BYTE * err_msg))
COUNT char_error(request * rq, struct dhdr FAR * lpDevice);
COUNT block_error(request * rq, COUNT nDrive, struct dhdr FAR * lpDevice);

/* fatdir.c */
struct f_node FAR *dir_open(BYTE FAR * dirname);
COUNT dir_read(REG struct f_node FAR * fnp);
COUNT dir_write(REG struct f_node FAR * fnp);
VOID dir_close(REG struct f_node FAR * fnp);
COUNT dos_findfirst(UCOUNT attr, BYTE FAR * name);
COUNT dos_findnext(void);

/* fatfs.c */
__FAR_WRAPPER(COUNT, dos_open, (BYTE FAR * path, COUNT flag))
BOOL fcmp(BYTE FAR * s1, BYTE FAR * s2, COUNT n);
BOOL fcmp_wild(BYTE FAR * s1, BYTE FAR * s2, COUNT n);
VOID touc(BYTE FAR * s, COUNT n);
__FAR_WRAPPER(COUNT, dos_close, (COUNT fd))
COUNT dos_creat(BYTE FAR * path, COUNT attrib);
COUNT dos_delete(BYTE FAR * path);
COUNT dos_rmdir(BYTE FAR * path);
COUNT dos_rename(BYTE FAR * path1, BYTE FAR * path2);
__FAR_WRAPPER(date, dos_getdate, (void))
__FAR_WRAPPER(time, dos_gettime, (void))
COUNT dos_getftime(COUNT fd, date FAR * dp, time FAR * tp);
COUNT dos_setftime(COUNT fd, date FAR * dp, time FAR * tp);
LONG dos_getcufsize(COUNT fd);
LONG dos_getfsize(COUNT fd);
BOOL dos_setfsize(COUNT fd, LONG size);
COUNT dos_mkdir(BYTE FAR * dir);
BOOL last_link(struct f_node FAR * fnp);
COUNT map_cluster(REG struct f_node FAR * fnp, COUNT mode);
UCOUNT readblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err);
UCOUNT writeblock(COUNT fd, VOID FAR * buffer, UCOUNT count, COUNT * err);
__FAR_WRAPPER(COUNT, dos_read, (COUNT fd, VOID FAR * buffer, UCOUNT count))
COUNT dos_write(COUNT fd, VOID FAR * buffer, UCOUNT count);
LONG dos_lseek(COUNT fd, LONG foffset, COUNT origin);
UWORD dos_free(struct dpb *dpbp);

VOID trim_path(BYTE FAR * s);

COUNT dos_cd(struct cds FAR * cdsp, BYTE FAR * PathName);

struct f_node FAR *get_f_node(void);
VOID release_f_node(struct f_node FAR * fnp);
VOID dos_setdta(BYTE FAR * newdta);
COUNT dos_getfattr(BYTE FAR * name, UWORD FAR * attrp);
COUNT dos_setfattr(BYTE FAR * name, UWORD FAR * attrp);
COUNT media_check(REG struct dpb *dpbp);
struct f_node FAR *xlt_fd(COUNT fd);
COUNT xlt_fnp(struct f_node FAR * fnp);
struct dhdr FAR *select_unit(COUNT drive);

/* fattab.c */
UCOUNT link_fat(struct dpb *dpbp, UCOUNT Cluster1, REG UCOUNT Cluster2);
UCOUNT link_fat16(struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2);
UCOUNT link_fat12(struct dpb *dpbp, UCOUNT Cluster1, UCOUNT Cluster2);
UWORD next_cluster(struct dpb *dpbp, REG UCOUNT ClusterNum);
UWORD next_cl16(struct dpb *dpbp, REG UCOUNT ClusterNum);
UWORD next_cl12(struct dpb *dpbp, REG UCOUNT ClusterNum);

/* fcbfns.c */
VOID DosOutputString(BYTE FAR * s);
int DosCharInputEcho(VOID);
int DosCharInput(VOID);
VOID DosDirectConsoleIO(iregs FAR * r);
VOID DosCharOutput(COUNT c);
VOID DosDisplayOutput(COUNT c);
VOID FatGetDrvData(COUNT drive, COUNT FAR * spc, COUNT FAR * bps, COUNT FAR * nc, BYTE FAR ** mdp);
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
BOOL FcbRandomRead(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbRandomWrite(xfcb FAR * lpXfcb, COUNT * nErrorCode);
BOOL FcbCreate(xfcb FAR * lpXfcb);
void FcbNameInit(fcb FAR * lpFcb, BYTE * pszBuffer, COUNT * pCurDrive);
BOOL FcbOpen(xfcb FAR * lpXfcb);
BOOL FcbDelete(xfcb FAR * lpXfcb);
BOOL FcbRename(xfcb FAR * lpXfcb);
void MoveDirInfo(dmatch FAR * lpDmatch, struct dirent FAR * lpDir);
BOOL FcbClose(xfcb FAR * lpXfcb);
BOOL FcbFindFirst(xfcb FAR * lpXfcb);
BOOL FcbFindNext(xfcb FAR * lpXfcb);

/* initoem.c */
UWORD init_oem(void);

/* inthndlr.c */
VOID INRPT far got_cbreak(void);	/* procsupt.asm */
VOID INRPT far int20_handler(iregs UserRegs);
VOID INRPT far int21_handler(iregs UserRegs);
VOID far int21_entry(iregs UserRegs);
VOID int21_service(iregs far * r);
VOID INRPT FAR int22_handler(void);
VOID INRPT FAR int23_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
VOID INRPT FAR int24_handler(void);
VOID INRPT FAR low_int25_handler(void);
VOID INRPT FAR low_int26_handler(void);
VOID int25_handler();
VOID int26_handler();
VOID INRPT FAR int27_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);
VOID INRPT FAR int28_handler(void);
VOID INRPT FAR int2a_handler(void);
VOID INRPT FAR int2f_handler(void);
VOID INRPT FAR empty_handler(void);

/* ioctl.c */
COUNT DosDevIOctl(iregs FAR * r, COUNT FAR * err);

/* main.c */
INIT VOID main(void);
INIT BOOL init_device(struct dhdr FAR * dhp, BYTE FAR * cmdLine, COUNT mode, COUNT top);

/* memmgr.c */
seg far2para(VOID FAR * p);
seg long2para(LONG size);
VOID FAR *add_far(VOID FAR * fp, ULONG off);
VOID FAR *adjust_far(VOID FAR * fp);
__FAR_WRAPPER(COUNT, DosMemAlloc,
              (UWORD size, COUNT mode, seg FAR * para, UWORD FAR * asize))
COUNT DosMemLargest(UWORD FAR * size);
COUNT DosMemFree(UWORD para);
COUNT DosMemChange(UWORD para, UWORD size, UWORD * maxSize);
COUNT DosMemCheck(void);
COUNT FreeProcessMem(UWORD ps);
COUNT DosGetLargestBlock(UWORD FAR * block);
VOID show_chain(void);
VOID mcb_print(mcb FAR * mcbp);
VOID _fmemcpy(BYTE FAR * d, BYTE FAR * s, REG COUNT n);

/* misc.c */
__FAR_WRAPPER(VOID, scopy, (REG BYTE * s, REG BYTE * d))
VOID fscopy(REG BYTE FAR * s, REG BYTE FAR * d);
VOID fsncopy(BYTE FAR * s, BYTE FAR * d, REG COUNT n);
VOID bcopy(REG BYTE * s, REG BYTE * d, REG COUNT n);
__FAR_WRAPPER(VOID, fbcopy, (REG VOID FAR * s, REG VOID FAR * d, REG COUNT n))

/* nls.c */
COUNT extCtryInfo(int subfct, UWORD codepage,
	UWORD cntry, UWORD bufsize, UBYTE FAR * buf);
BYTE yesNo(unsigned char ch);
unsigned char upChar(unsigned char ch);
VOID upString(unsigned char FAR * str);
VOID upMem(unsigned char FAR * str, unsigned len);
unsigned char upFChar(unsigned char ch);
VOID upFString(unsigned char FAR * str);
VOID upFMem(unsigned char FAR * str, unsigned len);
COUNT setCountryCode(UWORD cntry);
COUNT getCountryInformation(UWORD cntry, BYTE FAR *buf);
COUNT getCodePage(UWORD FAR* actCP, UWORD FAR*sysCP);
COUNT setCodePage(UWORD actCP, UWORD sysCP);

/* prf.c */
VOID put_console(COUNT c);
__FAR_WRAPPER(WORD, printf, (CONST BYTE * fmt,...))
WORD sprintf(BYTE * buff, CONST BYTE * fmt,...);

/* strings.c */
__FAR_WRAPPER(COUNT, strlen, (REG BYTE * s))
COUNT fstrlen(REG BYTE FAR * s);
VOID _fstrcpy(REG BYTE FAR * d, REG BYTE FAR * s);
VOID strncpy(REG BYTE * d, REG BYTE * s, COUNT l);
__FAR_WRAPPER(COUNT, strcmp, (REG BYTE * d, REG BYTE * s))
COUNT fstrcmp(REG BYTE FAR * d, REG BYTE FAR * s);
COUNT fstrncmp(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l);
COUNT strncmp(REG BYTE * d, REG BYTE * s, COUNT l);
VOID fstrncpy(REG BYTE FAR * d, REG BYTE FAR * s, COUNT l);
BYTE *strchr(BYTE * s, BYTE c);

/* sysclk.c */
WORD clk_driver(rqptr rp);
COUNT BcdToByte(COUNT x);
COUNT BcdToWord(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);
COUNT ByteToBcd(COUNT x);
LONG WordToBcd(BYTE * x, UWORD * mon, UWORD * day, UWORD * yr);

/* syscon.c */
WORD con_driver(rqptr rp);
VOID break_handler(void);
VOID INRPT FAR int29_handler(int es, int ds, int di, int si, int bp, int sp, int bx, int dx, int cx, int ax, int ip, int cs, int flags);

/* syspack.c */
VOID getdirent(BYTE FAR * vp, struct dirent FAR * dp);
VOID putdirent(struct dirent FAR * dp, BYTE FAR * vp);

/* systime.c */
VOID DosGetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp, BYTE FAR * hdp);
COUNT DosSetTime(BYTE FAR * hp, BYTE FAR * mp, BYTE FAR * sp, BYTE FAR * hdp);
VOID DosGetDate(BYTE FAR * wdp, BYTE FAR * mp, BYTE FAR * mdp, COUNT FAR * yp);
COUNT DosSetDate(BYTE FAR * mp, BYTE FAR * mdp, COUNT FAR * yp);

/* task.c */
COUNT ChildEnv(exec_blk FAR * exp, UWORD * pChildEnvSeg, char far * pathname);
VOID new_psp(psp FAR * p, int psize);
VOID return_user(void);
__FAR_WRAPPER(COUNT, DosExec, (COUNT mode, exec_blk FAR * ep, BYTE FAR * lp))
__FAR_WRAPPER(VOID, InitPSP, (VOID))

VOID FAR p_0(void);

/* irqstack.asm */
VOID init_stacks(VOID FAR * stack_base, COUNT nStacks, WORD stackSize);

/* newstuff.c */
int SetJFTSize(UWORD nHandles);
int DosMkTmp(BYTE FAR * pathname, UWORD attr);
COUNT get_verify_drive(char FAR * src);
COUNT truename(char FAR * src, char FAR * dest, COUNT t);

/* network.c */
COUNT int2f_Remote_call(UWORD func, UWORD b, UCOUNT n, UWORD d, VOID FAR * s, UWORD i, VOID FAR * data);
COUNT QRemote_Fn(char FAR * s, char FAR * d);

UWORD get_machine_name(BYTE FAR * netname);
VOID set_machine_name(BYTE FAR * netname, UWORD name_num);
UCOUNT Remote_RW(UWORD func, UCOUNT n, BYTE FAR * bp, sft FAR * s, COUNT FAR * err);
COUNT Remote_find(UWORD func, BYTE FAR * name, REG dmatch FAR * dmp);

/* procsupt.asm */
VOID INRPT FAR exec_user(iregs FAR * irp);

#define strcpy(d, s)	scopy(s, d)

