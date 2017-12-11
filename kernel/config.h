/****************************************************************/
/*                                                              */
/*                          config.h                            */
/*                            DOS-C                             */
/*                                                              */
/*             Global data structures and declarations          */
/*                                                              */
/*                   Copyright (c) 2000                         */
/*                     Steffen Kaiser                           */
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

struct config {     /* Configuration variables */
  UBYTE cfgDosDataUmb;
  BYTE cfgBuffers;           /* number of buffers in the system */
  UBYTE cfgFiles;            /* number of available files */
  UBYTE cfgFilesHigh;
  UBYTE cfgFcbs;             /* number of available FCBs */
  UBYTE cfgProtFcbs;         /* number of protected FCBs */
  BYTE *cfgInit;             /* init of command.com */
  BYTE *cfgInitTail;         /* command.com's tail */
  UBYTE cfgLastdrive;        /* last drive */
  UBYTE cfgLastdriveHigh;
  BYTE cfgStacks;            /* number of stacks */
  BYTE cfgStacksHigh;
  UWORD cfgStackSize;        /* stacks size for each stack */
  UBYTE cfgP_0_startmode;    /* load command.com high or not */
  unsigned ebda2move;        /* value for switches=/E:nnnn */
};
