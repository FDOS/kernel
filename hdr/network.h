/****************************************************************/
/*                                                              */
/*                           network.h                          */
/*                                                              */
/*                        DOS Networking                        */
/*                                                              */
/*                       October 10, 1999                       */
/*                                                              */
/*                      Copyright (c) 1999                      */
/*                          James Tabor                         */
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

/* Defines for remote access functions */
#define REM_RMDIR       0x1101
#define REM_MKDIR       0x1103
#define REM_CHDIR       0x1105
#define REM_CLOSE       0x1106
#define REM_FLUSH       0x1107
#define REM_READ        0x1108
#define REM_WRITE       0x1109
#define REM_LOCK        0x110a
#define REM_UNLOCK      0x110b
#define REM_GETSPACE    0x110c
#define REM_SETATTR     0x110e
#define REM_GETATTRZ    0x110f
#define REM_RENAME      0x1111
#define REM_DELETE      0x1113
#define REM_OPEN        0x1116
#define REM_CREATE      0x1117
#define REM_CRTRWOCDS   0x1118
#define REM_FND1WOCDS   0x1119
#define REM_FINDFIRST   0x111B
#define REM_FINDNEXT    0x111C
#define REM_CLOSEALL    0x111d
#define REM_DOREDIRECT  0x111e
#define REM_PRINTSET    0x111f
#define REM_FLUSHALL    0x1120
#define REM_LSEEK       0x1121
#define REM_PROCESS_END 0x1122
#define REM_FILENAME    0x1123
#define REM_PRINTREDIR  0x1125
#define REM_EXTOC       0x112e

struct rgds {
  UWORD r_spc;
  UWORD r_navc;
  UWORD r_bps;
  UWORD r_nc;
};

struct remote_fileattrib {
  UWORD rfa_file;               /* File Attributes  */
  union {
    ULONG rfa_filesize;         /* file size */
    struct {
      UWORD rfa_filesize_lo;    /* DI Low */
      UWORD rfa_filesize_hi;    /* BX High */
    } _split_rfa_fz;
  } rfa_fz_union;
  UWORD rfa_time;
  UWORD rfa_date;
};
