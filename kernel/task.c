/****************************************************************/
/*                                                              */
/*                           task.c                             */
/*                                                              */
/*                 Task Manager for DOS Processes               */
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

#define toupper(c)	((c) >= 'a' && (c) <= 'z' ? (c) + ('A' - 'a') : (c))

#define LOADNGO 0
#define LOAD    1
#define OVERLAY 3

#define LOAD_HIGH 0x80

static exe_header header;

#define CHUNK 32256
#define MAXENV 32768u
#define ENV_KEEPFREE 83         /* keep unallocated by environment variables */
        /* The '65' added to nEnvSize does not cover the additional stuff:
           + 2 bytes: number of strings
           + 80 bytes: maximum absolute filename
           + 1 byte: '\0'
           -- 1999/04/21 ska */

#ifndef PROTO
COUNT ChildEnv(exec_blk FAR *, UWORD *, char far *);
#else
COUNT ChildEnv();
#endif

LONG doslseek(COUNT fd, LONG foffset, COUNT origin)
{
  LONG set_pos;
  DosSeek(fd, foffset, origin, (ULONG *) & set_pos);
  return set_pos;
}

LONG DosGetFsize(COUNT hndl)
{
  sft FAR *s;
/*  sfttbl FAR *sp;*/

  /* Get the SFT block that contains the SFT      */
  if ((s = get_sft(hndl)) == (sft FAR *) - 1)
    return DE_INVLDHNDL;

  /* If this is not opened another error          */
  if (s->sft_count == 0)
    return DE_ACCESS;

  /* If SFT entry refers to a device, return the date and time of opening */
  if (s->sft_flags & (SFT_FDEVICE | SFT_FSHARED))
  {
    return s->sft_size;
  }

  /* call file system handler                     */
  return dos_getfsize(s->sft_status);
}

COUNT ChildEnv(exec_blk FAR * exp, UWORD * pChildEnvSeg,
               char far * pathname)
{
  BYTE FAR *pSrc;
  BYTE FAR *pDest;
  UWORD nEnvSize;
  COUNT RetCode;
/*  UWORD MaxEnvSize;                                                                                                                                           not used -- 1999/04/21 ska */
  psp FAR *ppsp = MK_FP(cu_psp, 0);

  /* create a new environment for the process             */
  /* copy parent's environment if exec.env_seg == 0       */

  pSrc = exp->exec.env_seg ?
      MK_FP(exp->exec.env_seg, 0) : MK_FP(ppsp->ps_environ, 0);

#if 0
  /* Every process requires an environment because of argv[0]
     -- 1999/04/21 ska */
  */if (!pSrc)                  /* no environment to copy */
  {
    *pChildEnvSeg = 0;
    return SUCCESS;
  }
#endif

  nEnvSize = 1;
  /* This loop had not counted the very last '\0'
     -- 1999/04/21 ska */
  if (pSrc)
  {                             /* if no environment is available, one byte is required */

    for (nEnvSize = 0;; nEnvSize++)
    {
      /* Test env size and abort if greater than max          */
      if (nEnvSize >= MAXENV - ENV_KEEPFREE)
        return DE_INVLDENV;

      if (*(UWORD FAR *) (pSrc + nEnvSize) == 0)
        break;
    }
    nEnvSize += 2;              /* account for trailing \0\0 */
  }

  /* allocate enough space for env + path                 */
  if ((RetCode = DosMemAlloc(long2para(nEnvSize + ENV_KEEPFREE),
                             mem_access_mode, (seg FAR *) pChildEnvSeg,
                             NULL /*(UWORD FAR *) MaxEnvSize ska */ )) < 0)
    return RetCode;
  pDest = MK_FP(*pChildEnvSeg + 1, 0);

  /* fill the new env and inform the process of its       */
  /* location throught the psp                            */

  /* copy the environment */
  if (pSrc)
  {
    fmemcpy(pDest, pSrc, nEnvSize);
    pDest += nEnvSize;
  }
  else
    *pDest++ = '\0';            /* create an empty environment */

  /* initialize 'extra strings' count */
  *((UWORD FAR *) pDest) = 1;
  pDest += sizeof(UWORD) / sizeof(BYTE);

  /* copy complete pathname */
  if ((RetCode = truename(pathname, pDest, TRUE)) != SUCCESS)
  {
    return RetCode;
  }

  /* Theoretically one could either:
     + resize the already allocated block to best-fit behind the pathname, or
     + generate the filename into a temporary buffer to allocate only the
     minimum required environment -- 1999/04/21 ska */

  return SUCCESS;
}

/* The following code is 8086 dependant                         */
VOID new_psp(psp FAR * p, int psize)
{
  REG COUNT i;
  psp FAR *q = MK_FP(cu_psp, 0);

  /* Clear out new psp first                              */
  fmemset(p, 0, sizeof(psp));

  /* initialize all entries and exits                     */
  /* CP/M-like exit point                                 */
  p->ps_exit = 0x20cd;

  /* CP/M-like entry point - jump to special entry        */
  p->ps_farcall = 0xea;
  p->ps_reentry = cpm_entry;
  /* unix style call - 0xcd 0x21 0xcb (int 21, retf)      */
  p->ps_unix[0] = 0xcd;
  p->ps_unix[1] = 0x21;
  p->ps_unix[2] = 0xcb;

  /* Now for parent-child relationships                   */
  /* parent psp segment                                   */
  p->ps_parent = cu_psp;
  /* previous psp pointer                                 */
  p->ps_prevpsp = q;

  /* Environment and memory useage parameters             */
  /* memory size in paragraphs                            */
  p->ps_size = psize;
  /* environment paragraph                                */
  p->ps_environ = 0;
  /* process dta                                          */
  p->ps_dta = (BYTE FAR *) (&p->ps_cmd_count);

  /* terminate address                                    */
  p->ps_isv22 = getvec(0x22);
  /* break address                                        */
  p->ps_isv23 = getvec(0x23);
  /* critical error address                               */
  p->ps_isv24 = getvec(0x24);

  /* File System parameters                               */
  /* user stack pointer - int 21                          */
  p->ps_stack = q->ps_stack;
  /* file table - 0xff is unused                          */

  for (i = 0; i < 20; i++)
    p->ps_files[i] = 0xff;

  /* maximum open files                                   */
  p->ps_maxfiles = 20;
  /* open file table pointer                              */
  p->ps_filetab = p->ps_files;

  /* clone the file table                                 */
  if (p != q)
  {
    REG COUNT i;

    for (i = 0; i < 20; i++)
    {
      if (q->ps_filetab[i] != 0xff && CloneHandle(i) >= 0)
        p->ps_filetab[i] = q->ps_filetab[i];
      else
        p->ps_filetab[i] = 0xff;
    }
  }

  /* first command line argument                          */
  p->ps_fcb1.fcb_drive = 0;
  fmemset(p->ps_fcb1.fcb_fname, ' ', FNAME_SIZE + FEXT_SIZE);
  /* second command line argument                         */
  p->ps_fcb2.fcb_drive = 0;
  fmemset(p->ps_fcb2.fcb_fname, ' ', FNAME_SIZE + FEXT_SIZE);

  /* local command line                                   */
  p->ps_cmd_count = 0;          /* command tail                 */
  p->ps_cmd[0] = 0;             /* command tail                 */
  if (RootPsp == (seg) ~ 0)
    RootPsp = FP_SEG(p);
}

STATIC UWORD patchPSP(UWORD pspseg, UWORD envseg, exec_blk FAR * exb,
                      BYTE FAR * fnam)
{
  psp FAR *psp;
  mcb FAR *pspmcb;
  int i;
  BYTE FAR *np;

  pspmcb = MK_FP(pspseg, 0);
  ++pspseg;
  psp = MK_FP(pspseg, 0);

  /* complete the psp by adding the command line and FCBs     */
  fmemcpy(psp->ps_cmd, exb->exec.cmd_line->ctBuffer, 127);
  if (FP_OFF(exb->exec.fcb_1) != 0xffff)
  {
    fmemcpy(&psp->ps_fcb1, exb->exec.fcb_1, 16);
    fmemcpy(&psp->ps_fcb2, exb->exec.fcb_2, 16);
  }
  psp->ps_cmd_count = exb->exec.cmd_line->ctCount;

  /* identify the mcb as this functions'                  */
  pspmcb->m_psp = pspseg;
  /* Patch in environment segment, if present, also adjust its MCB */
  if (envseg)
  {
    psp->ps_environ = envseg + 1;
    ((mcb FAR *) MK_FP(envseg, 0))->m_psp = pspseg;
  }
  else
    psp->ps_environ = 0;

  /* use the file name less extension - left adjusted and */
  np = fnam;
  for (;;)
  {
    switch (*fnam++)
    {
      case '\0':
        goto set_name;
      case ':':
      case '/':
      case '\\':
        np = fnam;
    }
  }
set_name:
  for (i = 0; i < 8 && np[i] != '.' && np[i] != '\0'; i++)
  {
    pspmcb->m_name[i] = toupper(np[i]);
  }
  if (i < 8)
    pspmcb->m_name[i] = '\0';

  /* return value: AX value to be passed based on FCB values */
  return ((psp->ps_fcb1.fcb_drive < lastdrive &&
           CDSp->cds_table[psp->ps_fcb1.fcb_drive].
           cdsFlags & CDSVALID) ? 0 : 0xff) + ((psp->ps_fcb2.fcb_drive <
                                                lastdrive
                                                && CDSp->cds_table[psp->
                                                                   ps_fcb2.
                                                                   fcb_drive].
                                                cdsFlags & CDSVALID) ? 0 :
                                               0xff) * 0x100;
}

COUNT DosComLoader(BYTE FAR * namep, exec_blk FAR * exp, COUNT mode)
{
  COUNT rc
      /* err     */
      /*,env_size */ ;
  COUNT nread;
  UWORD mem;
  UWORD env, asize;
  BYTE FAR *sp;
  psp FAR *p;
  psp FAR *q = MK_FP(cu_psp, 0);
  iregs FAR *irp;
  LONG com_size;

  int ModeLoadHigh = mode & 0x80;
  UBYTE UMBstate = uppermem_link;

  mode &= 0x7f;

  if (mode != OVERLAY)
  {

    if ((rc = ChildEnv(exp, &env, namep)) != SUCCESS)
    {
      return rc;
    }

    /* COMFILES will always be loaded in largest area. is that true TE */

    /* Now find out how many paragraphs are available       */
    if ((rc = DosMemLargest((seg FAR *) & asize)) != SUCCESS)
    {
      DosMemFree(env);
      return rc;
    }
    com_size = asize;

    if (ModeLoadHigh)
    {
      DosUmbLink(1);            /* link in UMB's */
    }

    /* Allocate our memory and pass back any errors         */
    if ((rc =
         DosMemAlloc((seg) com_size, mem_access_mode, (seg FAR *) & mem,
                     (UWORD FAR *) & asize)) < 0)
    {
      if (rc == DE_NOMEM)
      {
        if ((rc =
             DosMemAlloc(0, LARGEST, (seg FAR *) & mem,
                         (UWORD FAR *) & asize)) < 0)
        {
          DosMemFree(env);
          return rc;
        }
        /* This should never happen, but ... */
        if (asize < com_size)
        {
          DosMemFree(mem);
          DosMemFree(env);
          return rc;
        }
      }
      else
      {
        DosMemFree(env);        /* env may be 0 */
        return rc;
      }
    }
    ++mem;
  }
  else
    mem = exp->load.load_seg;

  if (ModeLoadHigh)
  {
    DosUmbLink(UMBstate);       /* restore link state */
  }

#ifdef DEBUG
  printf("DosComLoader. Loading '%S' at %04x\n", namep, mem);
#endif
  /* Now load the executable                              */
  /* If file not found - error                            */
  /* NOTE - this is fatal because we lost it in transit   */
  /* from DosExec!                                        */
  /* if ((rc = DosOpen(namep, 0)) < 0)
    fatal("(DosComLoader) com file lost in transit");*/

  /* do it in 32K chunks                                  */
  if ((com_size = DosGetFsize(rc)) != 0)
  {
    if (mode == OVERLAY)        /* memory already allocated */
      sp = MK_FP(mem, 0);
    else
    {                           /* test the filesize against the allocated memory */

      sp = MK_FP(mem, sizeof(psp));

      /* This is a potential problem, what to do with .COM files larger than
         the allocated memory?
         MS DOS always only loads the very first 64KB - sizeof(psp) bytes.
         -- 1999/04/21 ska */

      /* BUG !! in case of LH, memory may be smaller then 64K TE */

      if (com_size > ((LONG) asize << 4))       /* less memory than the .COM file has */
        com_size = (LONG) asize << 4;
    }
    do
    {
      nread = DosRead(rc, CHUNK, sp, &UnusedRetVal);
      sp = add_far((VOID FAR *) sp, (ULONG) nread);
    }
    while ((com_size -= nread) > 0 && nread == CHUNK);
  }
  DosClose(rc);

  if (mode == OVERLAY)
    return SUCCESS;

  /* point to the PSP so we can build it                  */
  p = MK_FP(mem, 0);
  setvec(0x22, MK_FP(user_r->CS, user_r->IP));
  new_psp(p, mem + asize);

  asize = patchPSP(mem - 1, env, exp, namep);   /* asize=fcbcode for ax */

  /* Transfer control to the executable                   */
  p->ps_parent = cu_psp;
  p->ps_prevpsp = (BYTE FAR *) MK_FP(cu_psp, 0);
  q->ps_stack = (BYTE FAR *) user_r;
  user_r->FLAGS &= ~FLG_CARRY;
  cu_psp = mem;
  dta = p->ps_dta;

  switch (mode)
  {
    case LOADNGO:
      {
        /*  BUG !!
           this works only, if COMSIZE >= 64K
           in case of LH, this is not necessarily true
         */

        *((UWORD FAR *) MK_FP(mem, 0xfffe)) = (UWORD) 0;

        /* build the user area on the stack                     */
        irp = MK_FP(mem, (0xfffe - sizeof(iregs)));

        /* start allocating REGs                                */
        irp->ES = irp->DS = mem;
        irp->CS = mem;
        irp->IP = 0x100;
        irp->AX = asize;        /* fcbcode */
        irp->BX = irp->CX = irp->DX = irp->SI = irp->DI = irp->BP = 0;
        irp->FLAGS = 0x200;

        if (InDOS)
          --InDOS;
        exec_user(irp);

        /* We should never be here          
           fatal("KERNEL RETURNED!!!");                    */
        break;
      }
    case LOAD:
      exp->exec.stack = MK_FP(mem, 0xfffe);
      *((UWORD FAR *) exp->exec.stack) = asize;
      exp->exec.start_addr = MK_FP(mem, 0x100);
      return SUCCESS;
  }

  return DE_INVLDFMT;
}

VOID return_user(void)
{
  psp FAR *p, FAR * q;
  REG COUNT i;
  iregs FAR *irp;
/*  long j;*/

  /* restore parent                                       */
  p = MK_FP(cu_psp, 0);

  /* When process returns - restore the isv               */
  setvec(0x22, p->ps_isv22);
  setvec(0x23, p->ps_isv23);
  setvec(0x24, p->ps_isv24);

  /* And free all process memory if not a TSR return      */
  remote_process_end();         /* might be a good idea to do that after closing
                                   but doesn't help NET either TE */
  if (!tsr)
  {
    remote_close_all();
    for (i = 0; i < p->ps_maxfiles; i++)
    {
      DosClose(i);
    }
    FcbCloseAll();
    FreeProcessMem(cu_psp);
  }

  cu_psp = p->ps_parent;
  q = MK_FP(cu_psp, 0);
  dta = q->ps_dta;

  irp = (iregs FAR *) q->ps_stack;

  irp->CS = FP_SEG(p->ps_isv22);
  irp->IP = FP_OFF(p->ps_isv22);

  if (InDOS)
    --InDOS;
  exec_user((iregs FAR *) q->ps_stack);
}

COUNT DosExeLoader(BYTE FAR * namep, exec_blk FAR * exp, COUNT mode)
{
  COUNT rc;
  /*err,     */
  /*env_size, */

  UWORD mem, env, asize, start_seg;

  int ModeLoadHigh = mode & 0x80;
  UBYTE UMBstate = uppermem_link;

  mode &= 0x7f;

  /* Clone the environement and create a memory arena     */
  if (mode != OVERLAY)
  {
    if ((rc = ChildEnv(exp, &env, namep)) != SUCCESS)
      return rc;
  }
  else
    mem = exp->load.load_seg;

  {
    ULONG image_size;
    ULONG image_offset;
    LONG exe_size;
    mcb FAR *mp;

    /* compute image offset from the header                 */
    image_offset = (ULONG) header.exHeaderSize * 16;

    /* compute image size by removing the offset from the   */
    /* number pages scaled to bytes plus the remainder and  */
    /* the psp                                              */
    /*  First scale the size                                */
    image_size = (ULONG) header.exPages * 512;
    /* remove the offset                                    */
    image_size -= image_offset;

    /* and finally add in the psp size                      */
    if (mode != OVERLAY)
      image_size += sizeof(psp);        /*TE 03/20/01 */

    if (mode != OVERLAY)
    {
      if (ModeLoadHigh)
      {
        DosUmbLink(1);          /* link in UMB's */
        mem_access_mode |= ModeLoadHigh;
      }

      /* Now find out how many paragraphs are available       */
      if ((rc = DosMemLargest((seg FAR *) & asize)) != SUCCESS)
      {
        DosMemFree(env);
        return rc;
      }

      exe_size = (LONG) long2para(image_size) + header.exMinAlloc;

      /* + long2para((LONG) sizeof(psp)); ?? see above
         image_size += sizeof(psp) -- 1999/04/21 ska */
      if (exe_size > asize && (mem_access_mode & 0x80))
      {
        /* First try low memory */
        mem_access_mode &= ~0x80;
        rc = DosMemLargest((seg FAR *) & asize);
        mem_access_mode |= 0x80;
        if (rc != SUCCESS)
        {
          DosMemFree(env);
          return rc;
        }
      }
      if (exe_size > asize)
      {
        DosMemFree(env);
        return DE_NOMEM;
      }
      exe_size = (LONG) long2para(image_size) + header.exMaxAlloc;
      /* + long2para((LONG) sizeof(psp)); ?? -- 1999/04/21 ska */
      if (exe_size > asize)
        exe_size = asize;

      /* TE if header.exMinAlloc == header.exMaxAlloc == 0,
         DOS will allocate the largest possible memory area
         and load the image as high as possible into it.
         discovered (and after that found in RBIL), when testing NET */

      if ((header.exMinAlloc | header.exMaxAlloc) == 0)
        exe_size = asize;

      /* /// Removed closing curly brace.  We should not attempt to allocate
         memory if we are overlaying the current process, because the new
         process will simply re-use the block we already have allocated.
         This was causing execl() to fail in applications which use it to
         overlay (replace) the current exe file with a new one.
         Jun 11, 2000 - rbc
         } */

      /* Allocate our memory and pass back any errors         */
      /* We can still get an error on first fit if the above  */
      /* returned size was a bet fit case                     */
      /* ModeLoadHigh = 80 = try high, then low                   */
      if ((rc =
           DosMemAlloc((seg) exe_size, mem_access_mode | ModeLoadHigh,
                       (seg FAR *) & mem, (UWORD FAR *) & asize)) < 0)
      {
        if (rc == DE_NOMEM)
        {
          if ((rc =
               DosMemAlloc(0, LARGEST, (seg FAR *) & mem,
                           (UWORD FAR *) & asize)) < 0)
          {
            DosMemFree(env);
            return rc;
          }
          /* This should never happen, but ... */
          if (asize < exe_size)
          {
            DosMemFree(mem);
            DosMemFree(env);
            return rc;
          }
        }
        else
        {
          DosMemFree(env);
          return rc;
        }
      }
      else
        /* with no error, we got exactly what we asked for      */
        asize = exe_size;

#ifdef DEBUG
      printf("DosExeLoader. Loading '%S' at %04x\n", namep, mem);
#endif

/* /// Added open curly brace and "else" clause.  We should not attempt
       to allocate memory if we are overlaying the current process, because
       the new process will simply re-use the block we already have allocated.
       This was causing execl() to fail in applications which use it to
       overlay (replace) the current exe file with a new one.
       Jun 11, 2000 - rbc */
    }
    else
      asize = exe_size;
/* /// End of additions.  Jun 11, 2000 - rbc */

    if (ModeLoadHigh)
    {
      mem_access_mode &= ~ModeLoadHigh; /* restore old situation */
      DosUmbLink(UMBstate);     /* restore link state */
    }

    if (mode != OVERLAY)
    {
      /* memory found large enough - continue processing      */
      mp = MK_FP(mem, 0);
      ++mem;
    }
    else
      mem = exp->load.load_seg;

    /* create the start seg for later computations          */
    if (mode == OVERLAY)
      start_seg = mem;
    else
    {
      start_seg = mem + long2para((LONG) sizeof(psp));
    }

    /* Now load the executable                              */
    /* If file not found - error                            */
    /* NOTE - this is fatal because we lost it in transit   */
    /* from DosExec!                                        */
#if 0
    if ((rc = DosOpen(namep, 0)) < 0)
    {
      fatal("(DosExeLoader) exe file lost in transit");
    }
#endif
    /* offset to start of image                             */
    if (doslseek(rc, image_offset, 0) != image_offset)
    {
      if (mode != OVERLAY)
      {
        DosMemFree(--mem);
        DosMemFree(env);
      }
      return DE_INVLDDATA;
    }

    /* read in the image in 32K chunks                      */
    if (mode != OVERLAY)
    {
      exe_size = image_size - sizeof(psp);
    }
    else
      exe_size = image_size;

    if (exe_size > 0)
    {
      UCOUNT nBytesRead;
      BYTE FAR *sp;

      if (mode != OVERLAY)
      {
        if ((header.exMinAlloc == 0) && (header.exMaxAlloc == 0))
        {
          /* then the image should be placed as high as possible */
          start_seg = start_seg + mp->m_size - (image_size + 15) / 16;
        }
      }

      sp = MK_FP(start_seg, 0x0);

      do
      {
        nBytesRead =
            DosRead((COUNT) rc,
                    (COUNT) (exe_size < CHUNK ? exe_size : CHUNK),
                    (VOID FAR *) sp, &UnusedRetVal);
        sp = add_far((VOID FAR *) sp, (ULONG) nBytesRead);
        exe_size -= nBytesRead;
      }
      while (nBytesRead && exe_size > 0);
    }
  }

  {                             /* relocate the image for new segment                   */
    COUNT i;
    UWORD reloc[2];
    seg FAR *spot;

    doslseek(rc, (LONG) header.exRelocTable, 0);
    for (i = 0; i < header.exRelocItems; i++)
    {
      if (DosRead
          (rc, sizeof(reloc), (VOID FAR *) & reloc[0],
           &UnusedRetVal) != sizeof(reloc))
      {
        return DE_INVLDDATA;
      }
      if (mode == OVERLAY)
      {
        spot = MK_FP(reloc[1] + mem, reloc[0]);
        *spot += exp->load.reloc;
      }
      else
      {
        /*      spot = MK_FP(reloc[1] + mem + 0x10, reloc[0]); */
        spot = MK_FP(reloc[1] + start_seg, reloc[0]);
        *spot += start_seg;
      }
    }
  }

  /* and finally close the file                           */
  DosClose(rc);

  /* exit here for overlay                                */
  if (mode == OVERLAY)
    return SUCCESS;

  {
    psp FAR *p;
    psp FAR *q = MK_FP(cu_psp, 0);

    /* point to the PSP so we can build it                  */
    p = MK_FP(mem, 0);
    setvec(0x22, MK_FP(user_r->CS, user_r->IP));
    new_psp(p, mem + asize);

    asize = patchPSP(mem - 1, env, exp, namep); /* asize = fcbcode */

    /* Transfer control to the executable                   */
    p->ps_parent = cu_psp;
    p->ps_prevpsp = (BYTE FAR *) MK_FP(cu_psp, 0);
    q->ps_stack = (BYTE FAR *) user_r;
    user_r->FLAGS &= ~FLG_CARRY;

    switch (mode)
    {
      case LOADNGO:
        {
          /* build the user area on the stack                     */
          iregs FAR *irp = MK_FP(header.exInitSS + start_seg,
                                 ((header.exInitSP -
                                   sizeof(iregs)) & 0xffff));

          /* start allocating REGs                                */
          /* Note: must match es & ds memory segment              */
          irp->ES = irp->DS = mem;
          irp->CS = header.exInitCS + start_seg;
          irp->IP = header.exInitIP;
          irp->AX = asize;      /* asize = fcbcode    */
          irp->BX = irp->CX = irp->DX = irp->SI = irp->DI = irp->BP = 0;
          irp->FLAGS = 0x200;

          cu_psp = mem;
          dta = p->ps_dta;

          if (InDOS)
            --InDOS;
          exec_user(irp);
          /* We should never be here     
             fatal("KERNEL RETURNED!!!");                    */
          break;
        }

      case LOAD:
        cu_psp = mem;
        exp->exec.stack =
            MK_FP(header.exInitSS + start_seg, header.exInitSP);
        *((UWORD FAR *) exp->exec.stack) = asize;       /* fcbcode */
        exp->exec.start_addr =
            MK_FP(header.exInitCS + start_seg, header.exInitIP);
        return SUCCESS;
    }
  }
  return DE_INVLDFMT;
}

/* mode = LOAD or EXECUTE
   ep = EXE block
   lp = filename to load (string)

   leb = local copy of exe block
 */
COUNT DosExec(COUNT mode, exec_blk FAR * ep, BYTE FAR * lp)
{
  COUNT rc;
  exec_blk leb;

/*  BYTE FAR *cp;*/
  BOOL bIsCom = FALSE;

  fmemcpy(&leb, ep, sizeof(exec_blk));
  /* If file not found - free ram and return error        */

  if ((rc = DosOpen(lp, 0)) < 0)
  {
    return DE_FILENOTFND;
  }

  if (DosRead(rc, sizeof(exe_header), (VOID FAR *) & header, &UnusedRetVal)
      != sizeof(exe_header))
  {
    bIsCom = TRUE;
  }
  DosClose(rc);

  if (bIsCom || header.exSignature != MAGIC)
  {
    rc = DosComLoader(lp, &leb, mode);
  }
  else
  {
    rc = DosExeLoader(lp, &leb, mode);
  }
  if (mode == LOAD && rc == SUCCESS)
    fmemcpy(ep, &leb, sizeof(exec_blk));

  return rc;
}

/*
 * Log: task.c,v - for newer log entries do "cvs log task.c"
 *
 * Revision 1.8  2000/03/31 05:40:09  jtabor
 * Added Eric W. Biederman Patches
 *
 * Revision 1.7  2000/03/09 06:07:11  kernel
 * 2017f updates by James Tabor
 *
 * Revision 1.6  1999/08/25 03:18:10  jprice
 * ror4 patches to allow TC 2.01 compile.
 *
 * Revision 1.5  1999/04/23 04:24:39  jprice
 * Memory manager changes made by ska
 *
 * Revision 1.4  1999/04/16 00:53:33  jprice
 * Optimized FAT handling
 *
 * Revision 1.3  1999/04/11 04:33:39  jprice
 * ror4 patches
 *
 * Revision 1.2  1999/03/29 17:05:09  jprice
 * ror4 changes
 *
 * Revision 1.1.1.1  1999/03/29 15:41:41  jprice
 * New version without IPL.SYS
 *
 * Revision 1.5  1999/02/08 05:55:58  jprice
 * Added Pat's 1937 kernel patches
 *
 * Revision 1.4  1999/02/04 03:14:07  jprice
 * Formating.  Added comments.
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
 *    Rev 1.15   06 Dec 1998  8:46:28   patv
 * Bug fixes.
 *
 *    Rev 1.14   07 Feb 1998 20:38:32   patv
 * Modified stack fram to match DOS standard
 *
 *    Rev 1.13   31 Jan 1998 14:39:20   patv
 * Corrected type in load high code.
 *
 *    Rev 1.12   31 Jan 1998 14:02:52   patv
 * Added load high in memory option in DosExeLoader.
 *
 *    Rev 1.11   22 Jan 1998 22:17:14   patv
 * Eliminated warnings.
 *
 *    Rev 1.10   22 Jan 1998 21:31:36   patv
 * Corrected short .COM problem.
 *
 *    Rev 1.9   04 Jan 1998 23:15:16   patv
 * Changed Log for strip utility
 *
 *    Rev 1.8   22 Jan 1997 13:18:14   patv
 * pre-0.92 Svante Frey bug fixes.
 *
 *    Rev 1.7   16 Jan 1997 12:46:56   patv
 * pre-Release 0.92 feature additions
 *
 *    Rev 1.5   29 Aug 1996 13:07:22   patv
 * Bug fixes for v0.91b
 *
 *    Rev 1.4   29 May 1996 21:03:36   patv
 * bug fixes for v0.91a
 *
 *    Rev 1.3   19 Feb 1996  3:21:48   patv
 * Added NLS, int2f and config.sys processing
 *
 *    Rev 1.2   01 Sep 1995 17:54:22   patv
 * First GPL release.
 *
 *    Rev 1.1   30 Jul 1995 20:51:58   patv
 * Eliminated version strings in ipl
 *
 *    Rev 1.0   02 Jul 1995  8:34:06   patv
 * Initial revision.
 */
