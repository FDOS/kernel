/****************************************************************/
/*                                                              */
/*                            nls_hc.c                          */
/*                            FreeDOS                           */
/*                                                              */
/*    National Languge Support hardcoded NLS package            */
/*                                                              */
/*                   Copyright (c) 2000                         */
/*                         Steffen Kaiser                       */
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
#define NLS_NO_VARS
#include "nls.h"

#undef NLS_NO_VARS
#define NLS_HARDCODED US
#define NLS_POINTERS 5
#define NLS_FNAMSEPS 14
#define NLS_DBCSENTR 1
#include "nls.h"


/*
 *	Hardcoded NLS package for U.S.A. CP437
 */
struct nlsCharTbl128 nlsUpHardcodedTable = {
	128,            /* upcase table */
	{
			'\x80' ,'\x9a' ,'E' ,'A' ,'\x8e' ,'A' ,'\x8f' ,'\x80'           /* 0 - 7 */
			,'E' ,'E' ,'E' ,'I' ,'I' ,'I' ,'\x8e' ,'\x8f'           /* 8 - 15 */
			,'\x90' ,'\x92' ,'\x92' ,'O' ,'\x99' ,'O' ,'U' ,'U'             /* 16 - 23 */
			,'Y' ,'\x99' ,'\x9a' ,'\x9b' ,'\x9c' ,'\x9d' ,'\x9e' ,'\x9f'            /* 24 - 31 */
			,'A' ,'I' ,'O' ,'U' ,'\xa5' ,'\xa5' ,'\xa6' ,'\xa7'             /* 32 - 39 */
			,'\xa8' ,'\xa9' ,'\xaa' ,'\xab' ,'\xac' ,'\xad' ,'\xae' ,'\xaf'                 /* 40 - 47 */
			,'\xb0' ,'\xb1' ,'\xb2' ,'\xb3' ,'\xb4' ,'\xb5' ,'\xb6' ,'\xb7'                 /* 48 - 55 */
			,'\xb8' ,'\xb9' ,'\xba' ,'\xbb' ,'\xbc' ,'\xbd' ,'\xbe' ,'\xbf'                 /* 56 - 63 */
			,'\xc0' ,'\xc1' ,'\xc2' ,'\xc3' ,'\xc4' ,'\xc5' ,'\xc6' ,'\xc7'                 /* 64 - 71 */
			,'\xc8' ,'\xc9' ,'\xca' ,'\xcb' ,'\xcc' ,'\xcd' ,'\xce' ,'\xcf'                 /* 72 - 79 */
			,'\xd0' ,'\xd1' ,'\xd2' ,'\xd3' ,'\xd4' ,'\xd5' ,'\xd6' ,'\xd7'                 /* 80 - 87 */
			,'\xd8' ,'\xd9' ,'\xda' ,'\xdb' ,'\xdc' ,'\xdd' ,'\xde' ,'\xdf'                 /* 88 - 95 */
			,'\xe0' ,'\xe1' ,'\xe2' ,'\xe3' ,'\xe4' ,'\xe5' ,'\xe6' ,'\xe7'                 /* 96 - 103 */
			,'\xe8' ,'\xe9' ,'\xea' ,'\xeb' ,'\xec' ,'\xed' ,'\xee' ,'\xef'                 /* 104 - 111 */
			,'\xf0' ,'\xf1' ,'\xf2' ,'\xf3' ,'\xf4' ,'\xf5' ,'\xf6' ,'\xf7'                 /* 112 - 119 */
			,'\xf8' ,'\xf9' ,'\xfa' ,'\xfb' ,'\xfc' ,'\xfd' ,'\xfe' ,'\xff'                 /* 120 - 127 */
	}
};
struct nlsCharTbl128 nlsFnameUpHardcodedTable = {
	128,            /* file name upcase table */
	{
			'\x80' ,'\x9a' ,'E' ,'A' ,'\x8e' ,'A' ,'\x8f' ,'\x80'           /* 0 - 7 */
			,'E' ,'E' ,'E' ,'I' ,'I' ,'I' ,'\x8e' ,'\x8f'           /* 8 - 15 */
			,'\x90' ,'\x92' ,'\x92' ,'O' ,'\x99' ,'O' ,'U' ,'U'             /* 16 - 23 */
			,'Y' ,'\x99' ,'\x9a' ,'\x9b' ,'\x9c' ,'\x9d' ,'\x9e' ,'\x9f'            /* 24 - 31 */
			,'A' ,'I' ,'O' ,'U' ,'\xa5' ,'\xa5' ,'\xa6' ,'\xa7'             /* 32 - 39 */
			,'\xa8' ,'\xa9' ,'\xaa' ,'\xab' ,'\xac' ,'\xad' ,'\xae' ,'\xaf'                 /* 40 - 47 */
			,'\xb0' ,'\xb1' ,'\xb2' ,'\xb3' ,'\xb4' ,'\xb5' ,'\xb6' ,'\xb7'                 /* 48 - 55 */
			,'\xb8' ,'\xb9' ,'\xba' ,'\xbb' ,'\xbc' ,'\xbd' ,'\xbe' ,'\xbf'                 /* 56 - 63 */
			,'\xc0' ,'\xc1' ,'\xc2' ,'\xc3' ,'\xc4' ,'\xc5' ,'\xc6' ,'\xc7'                 /* 64 - 71 */
			,'\xc8' ,'\xc9' ,'\xca' ,'\xcb' ,'\xcc' ,'\xcd' ,'\xce' ,'\xcf'                 /* 72 - 79 */
			,'\xd0' ,'\xd1' ,'\xd2' ,'\xd3' ,'\xd4' ,'\xd5' ,'\xd6' ,'\xd7'                 /* 80 - 87 */
			,'\xd8' ,'\xd9' ,'\xda' ,'\xdb' ,'\xdc' ,'\xdd' ,'\xde' ,'\xdf'                 /* 88 - 95 */
			,'\xe0' ,'\xe1' ,'\xe2' ,'\xe3' ,'\xe4' ,'\xe5' ,'\xe6' ,'\xe7'                 /* 96 - 103 */
			,'\xe8' ,'\xe9' ,'\xea' ,'\xeb' ,'\xec' ,'\xed' ,'\xee' ,'\xef'                 /* 104 - 111 */
			,'\xf0' ,'\xf1' ,'\xf2' ,'\xf3' ,'\xf4' ,'\xf5' ,'\xf6' ,'\xf7'                 /* 112 - 119 */
			,'\xf8' ,'\xf9' ,'\xfa' ,'\xfb' ,'\xfc' ,'\xfd' ,'\xfe' ,'\xff'                 /* 120 - 127 */
	}
};
struct nlsFnamTermUS nlsFnameTermHardcodedTable = {
	22,             /* size of permittable character structure */
	1,              /* reserved */
	'\x00' ,'\xff', /* first/last permittable character */
	0,              /* reserved */
	'\x00' ,' ',    /* first/last excluded character */
	2,              /* reserved */
	14,             /* number of separators */
	{               /* separators */
			'.' ,'"' ,'/' ,'\\','[' ,']' ,':' ,'|',                 /* 0 - 7 */
			'<' ,'>' ,'+' ,'=' ,';' ,','            /* 8 - 13 */
	}
};
struct nlsCharTbl256 nlsCollHardcodedTable = {
	256,            /* collating sequence table */
	{
			'\x00' ,'\x01' ,'\x02' ,'\x03' ,'\x04' ,'\x05' ,'\x06' ,'\x07'          /* 0 - 7 */
			,'\x08' ,'\x09' ,'\x0a' ,'\x0b' ,'\x0c' ,'\x0d' ,'\x0e' ,'\x0f'                 /* 8 - 15 */
			,'\x10' ,'\x11' ,'\x12' ,'\x13' ,'\x14' ,'\x15' ,'\x16' ,'\x17'                 /* 16 - 23 */
			,'\x18' ,'\x19' ,'\x1a' ,'\x1b' ,'\x1c' ,'\x1d' ,'\x1e' ,'\x1f'                 /* 24 - 31 */
			,' ' ,'!' ,'"' ,'#' ,'$' ,'%' ,'&' ,'\''                /* 32 - 39 */
			,'(' ,')' ,'*' ,'+' ,',' ,'-' ,'.' ,'/'                 /* 40 - 47 */
			,'0' ,'1' ,'2' ,'3' ,'4' ,'5' ,'6' ,'7'                 /* 48 - 55 */
			,'8' ,'9' ,':' ,';' ,'<' ,'=' ,'>' ,'?'                 /* 56 - 63 */
			,'@' ,'A' ,'B' ,'C' ,'D' ,'E' ,'F' ,'G'                 /* 64 - 71 */
			,'H' ,'I' ,'J' ,'K' ,'L' ,'M' ,'N' ,'O'                 /* 72 - 79 */
			,'P' ,'Q' ,'R' ,'S' ,'T' ,'U' ,'V' ,'W'                 /* 80 - 87 */
			,'X' ,'Y' ,'Z' ,'[' ,'\\',']' ,'^' ,'_'                 /* 88 - 95 */
			,'`' ,'A' ,'B' ,'C' ,'D' ,'E' ,'F' ,'G'                 /* 96 - 103 */
			,'H' ,'I' ,'J' ,'K' ,'L' ,'M' ,'N' ,'O'                 /* 104 - 111 */
			,'P' ,'Q' ,'R' ,'S' ,'T' ,'U' ,'V' ,'W'                 /* 112 - 119 */
			,'X' ,'Y' ,'Z' ,'{' ,'|' ,'}' ,'~' ,'\x7f'              /* 120 - 127 */
			,'C' ,'U' ,'E' ,'A' ,'A' ,'A' ,'A' ,'C'                 /* 128 - 135 */
			,'E' ,'E' ,'E' ,'I' ,'I' ,'I' ,'A' ,'A'                 /* 136 - 143 */
			,'E' ,'A' ,'A' ,'O' ,'O' ,'O' ,'U' ,'U'                 /* 144 - 151 */
			,'Y' ,'O' ,'U' ,'$' ,'$' ,'$' ,'$' ,'$'                 /* 152 - 159 */
			,'A' ,'I' ,'O' ,'U' ,'N' ,'N' ,'\xa6' ,'\xa7'           /* 160 - 167 */
			,'?' ,'\xa9' ,'\xaa' ,'\xab' ,'\xac' ,'!' ,'"' ,'"'             /* 168 - 175 */
			,'\xb0' ,'\xb1' ,'\xb2' ,'\xb3' ,'\xb4' ,'\xb5' ,'\xb6' ,'\xb7'                 /* 176 - 183 */
			,'\xb8' ,'\xb9' ,'\xba' ,'\xbb' ,'\xbc' ,'\xbd' ,'\xbe' ,'\xbf'                 /* 184 - 191 */
			,'\xc0' ,'\xc1' ,'\xc2' ,'\xc3' ,'\xc4' ,'\xc5' ,'\xc6' ,'\xc7'                 /* 192 - 199 */
			,'\xc8' ,'\xc9' ,'\xca' ,'\xcb' ,'\xcc' ,'\xcd' ,'\xce' ,'\xcf'                 /* 200 - 207 */
			,'\xd0' ,'\xd1' ,'\xd2' ,'\xd3' ,'\xd4' ,'\xd5' ,'\xd6' ,'\xd7'                 /* 208 - 215 */
			,'\xd8' ,'\xd9' ,'\xda' ,'\xdb' ,'\xdc' ,'\xdd' ,'\xde' ,'\xdf'                 /* 216 - 223 */
			,'\xe0' ,'S' ,'\xe2' ,'\xe3' ,'\xe4' ,'\xe5' ,'\xe6' ,'\xe7'            /* 224 - 231 */
			,'\xe8' ,'\xe9' ,'\xea' ,'\xeb' ,'\xec' ,'\xed' ,'\xee' ,'\xef'                 /* 232 - 239 */
			,'\xf0' ,'\xf1' ,'\xf2' ,'\xf3' ,'\xf4' ,'\xf5' ,'\xf6' ,'\xf7'                 /* 240 - 247 */
			,'\xf8' ,'\xf9' ,'\xfa' ,'\xfb' ,'\xfc' ,'\xfd' ,'\xfe' ,'\xff'                 /* 248 - 255 */
	}
};
struct nlsDBCSUS nlsDBCSHardcodedTable = {
	0,              /* no DBC support */
	0,              /* DBC end marker */
};

struct nlsInfoBlockUS nlsInfo = {
	(char FAR*)NULL			/*fname*/
	,437				/*sysCodePage*/
	,(struct nlsPackage FAR*)&nlsInfo.chain	/* actPkg */
#ifdef NLS_CACHE_DATA
	,(struct nlsCharTbl FAR*)&nlsFnameUpHardcodedTable
	,(struct nlsCharTbl FAR*)&nlsUpHardcodedTable
#endif
	, /* hardcoded nlsPackageUS */ {
		(struct nlsPackage FAR*)NULL		/* nxt */
		,0					/* MUX calling flags */
		, /* Extended Country Information */ {
			   1,				/* subfct */
			   0x26,			/* size */
                1,              /* country code */
                437,            /* code page */
				0,      /* date format */
				{
						/* currency string */
						'$','\x00','\x00','\x00','\x00',                /* 0 - 4 */
				},
				{               /* thousand separator */
						',' ,'\x00'             /* 0 - 1 */
				},
				{               /* decimal point */
						'.' ,'\x00'             /* 0 - 1 */
				},
				{               /* date separator */
						'-' ,'\x00'             /* 0 - 1 */
				},
				{               /* time separator */
						':' ,'\x00'             /* 0 - 1 */
				},
				0,              /* currency format */
				2,              /* currency prescision */
				0,              /* time format */
				CharMapSrvc,    /* upcase function */
				{               /* data separator */
						',','\x00'              /* 0 - 1 */
				}
		}
		, 'Y', 'N'			/* yes / no */
		, 5		/* num of subfunctions */
		, /* subfunctions */ {
			{ 2, (VOID FAR*)&nlsUpHardcodedTable }			/* #0 */
			, { 4, (VOID FAR*)&nlsFnameUpHardcodedTable }	/* #1 */
			, { 5, (VOID FAR*)&nlsFnameTermHardcodedTable }	/* #2 */
			, { 6, (VOID FAR*)&nlsCollHardcodedTable }		/* #3 */
			, { 7, (VOID FAR*)&nlsDBCSHardcodedTable }		/* #4 */
		}
	}
};
