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

#define LOADNGO 0
#define LOAD    1
#define OVERLAY 3

#define LOAD_HIGH 0x80

/* static exe_header ExeHeader;
                           to save some bytes, both static and on stack,
                           we recycle SecPathBuffer                 TE */

#define ExeHeader (*(exe_header *)(SecPathName + 0))
#define TempExeBlock (*(exec_blk *)(SecPathName + sizeof(exe_header)))
#define Shell (SecPathName + sizeof(exe_header) + sizeof(exec_blk))
#define sizeofShell (sizeof SecPathName - sizeof(exe_header) - sizeof(exec_blk))

#ifdef __TURBOC__ /* this is a Borlandism and doesn't work elsewhere */
 #if sizeof(SecPathName) < sizeof(exe_header) + sizeof(exec_blk) + NAMEMAX
  #error No room in SecPathName to be recycled!
 #endif
#endif

#define CHUNK 32256u		/* =8000h-512; this value allows to combine
				   in one int value negative error codes
				   and positive read counters
				*/
#define MAXENV 32768u		/* maximum environment size */

intvec getvec(unsigned char intno)
{
  intvec iv;
  disable();
  iv = *MK_PTR(intvec, 0, 4 * intno);
  enable();
  return iv;
}

void setvec(unsigned char intno, intvec vector)
{
  disable();
  *MK_PTR(intvec, 0, 4 * intno) = vector;
  enable();
}

ULONG SftGetFsize(int sft_idx)
{
  const sft FAR *s = idx_to_sft(sft_idx);

  /* Get the SFT block that contains the SFT      */
  if (FP_OFF(s) == (size_t) -1)
    return DE_INVLDHNDL;

  /* If SFT entry refers to a device, return the date and time of opening */
  if (s->sft_flags & (SFT_FDEVICE | SFT_FSHARED))
  {
    return s->sft_size;
  }

  /* call file system handler                     */
  return dos_getfsize(s->sft_status);
}

/* create a new environment for the process */
STATIC int ChildEnv(seg_t env_seg, seg_t *penv_seg, const char far *path)
{
  size_t env_sz, path_sz;
  int rc;

  /* Every process requires an environment because of argv[0]
     -- 1999/04/21 ska
  */

  /* make complete pathname					*/
  if ((rc = truename(path, PriPathName, CDS_MODE_SKIP_PHYSICAL)) < SUCCESS)
    return rc;

  /* get parent's environment if exec.env_seg == 0		*/
  if (env_seg == 0)
    env_seg = MK_SEG_PTR(const psp, cu_psp)->ps_environ;

  /* count size of environment					*/
  path_sz = strlen(PriPathName) + 5;
  env_sz = 0;
  if (env_seg && *MK_PTR(const char, env_seg, 0))
  {
    do
    {
      env_sz++;
      if (env_sz + path_sz > MAXENV)
        return DE_INVLDENV;
    } while (*MK_PTR(const UWORD, env_seg, env_sz));
  }

  /* allocate space for env + path				*/
  {
    UWORD tmp;
    if ((rc = DosMemAlloc((env_sz + path_sz + 15) / 16,
                          mem_access_mode, penv_seg, &tmp)) != SUCCESS)
      return rc;
  }
  {
    seg_t dst_seg = *penv_seg + 1;

    /* enviornment contains:
       - 0 or more ASCIIZ strings (with variable definitions);
       - empty ASCIIZ string (one null character);
       - 16-bit counter (usually 1);
       - ASCIIZ string with path.
    */
    /* UNDOCUMENTED: with none variables before empty ASCIIZ string,
       environment should get additional null character (ie. empty
       environment contains two null characters). --avb
    */
    fmemcpy(MK_SEG_PTR(char, dst_seg), MK_SEG_PTR(const char, env_seg), env_sz);
    *MK_PTR(UWORD, dst_seg, env_sz) = 0;
    *MK_PTR(UWORD, dst_seg, env_sz + 2) = 1;
    fstrcpy(MK_PTR(char, dst_seg, env_sz + 4), PriPathName);
  }
  return SUCCESS;
}

/* The following code is 8086 dependant				*/
void new_psp(seg_t para, seg_t cur_psp)
{
  psp _seg *p = MK_SEG_PTR(psp, para);

  fmemcpy(p, MK_SEG_PTR(psp, cur_psp), sizeof(psp));

  /* initialize all entries and exits				*/
  p->ps_exit = 0x20cd;		/* CP/M-like exit point:	*/
				/* INT 20 opcode		*/
				/* CP/M-like entry point:	*/
  p->ps_farcall = 0x9a;		/* FAR CALL opcode...		*/
  p->ps_reentry = MK_FP(0xf01d,0xfef0);	/* ...entry address	*/

  /* entry address should point to 0:c0 (INT 30 vector), but
     low word of ps_reentry should also contain "size of first
     segment for .COM file" while preserving the far call;
     in MS-DOS this is F01D:FEF0 --avb				*/

  p->ps_unix[0] = 0xcd;		/* unix style call:		*/
  p->ps_unix[1] = 0x21;		/* INT 21/RETF opcodes		*/
  p->ps_unix[2] = 0xcb;

  /* parent-child relationships					*/
  p->ps_prevpsp = (VFP)-1l;	/* previous psp address		*/

  p->ps_isv22 = getvec(0x22);	/* terminate handler		*/
  p->ps_isv23 = getvec(0x23);	/* break handler		*/
  p->ps_isv24 = getvec(0x24);	/* critical error handler	*/

  /* File System parameters					*/
  p->ps_maxfiles = sizeof p->ps_files; /* size of file table	*/
  p->ps_filetab = p->ps_files;	/* file table address		*/
}

/* !!! cur_psp always equal to cu_psp --avb */
void child_psp(seg_t para, seg_t cur_psp, seg_t beyond)
{
  psp _seg *p;

  new_psp(para, cur_psp);
  p = MK_SEG_PTR(psp, para);

  /* parent-child relationships					*/
  p->ps_parent = cu_psp;	/* parent psp segment		*/

  /* Environment and memory useage parameters			*/
  p->ps_size = beyond;		/* segment of memory beyond	*/
				/* memory allocated to program	*/

  /* File System parameters					*/
  {
    psp _seg *q = MK_SEG_PTR(psp, cur_psp);
    int i;
    /* clone the file table, 0xff=unused			*/
    for (i = 0; i < sizeof p->ps_files; i++)
      p->ps_files[i] = CloneHandle(i) != SUCCESS ? 0xff : q->ps_filetab[i];
  }
}

STATIC void makePSP(seg_t pspseg, seg_t envseg, size_t asize, const char FAR * path)
{
  psp _seg *p = MK_SEG_PTR(psp, pspseg);
  mcb _seg *pspmcb = MK_SEG_PTR(mcb, FP_SEG(p) - 1);

  /* identify the mcb as this functions'			*/
  pspmcb->m_psp = FP_SEG(p);

  /* copy the file name less extension into MCB			*/
  {
    const char FAR *np;
    int i;
    for (np = path;;)		/* find program name after path */
    {
      char ch = *path;
      if (ch == '\0')
        break;
      path++;
      if (ch == ':' || ch == '\\' || ch == '/')
        np = path;		/* remember position after path */
    }
    i = 0;
    do				/* extract program name		*/
    {
      UBYTE ch = *np;
      if (ch == '.' || ch == '\0')
      {
        pspmcb->m_name[i] = '\0';
        break;
      }
      if (ch >= 'a' && ch <= 'z')
        ch -= (UBYTE)('a' - 'A');
      pspmcb->m_name[i] = ch;	/* copy name, without extension	*/
      i++, np++;
    } while (i < 8);
  }

  setvec(0x22, (intvec)MK_FP(user_r->CS, user_r->IP));
  child_psp(FP_SEG(p), cu_psp, FP_SEG(p) + asize);

  /* Patch in env segment, if present, also adjust its MCB	*/
  if (envseg)
  {
    MK_SEG_PTR(mcb, envseg)->m_psp = FP_SEG(p);
    envseg++;
  }
  p->ps_environ = envseg;
}

static void load_transfer(seg_t ds, exec_blk *ep, int mode)
{
  UWORD fcbcode;
  psp _seg *p = MK_SEG_PTR(psp, ds);
  {
    psp _seg *q = MK_SEG_PTR(psp, cu_psp);
    p->ps_parent = FP_SEG(q);
    p->ps_prevpsp = q;
    q->ps_stack = (BYTE FAR *)user_r;
  }
  user_r->FLAGS &= ~FLG_CARRY;

  cu_psp = FP_SEG(p);
  /* process dta                                          */
  dta = &p->ps_cmd;

  /* complete the psp by adding the command line and FCBs	*/
  /* UNDOCUMENTED: MS-DOS copies sizeof(CommandTail) bytes without
     checking memory contents and fixing wrong (>7Fh) length field;
     FCBs also copied without checking address validness --avb
  */
  fmemcpy(&p->ps_fcb1, ep->exec.fcb_1, 12); /* drive+name+ext */
  fmemcpy(&p->ps_fcb2, ep->exec.fcb_2, 12);
  fmemcpy(&p->ps_cmd, ep->exec.cmd_line, sizeof(CommandTail));

  /* AX value to be passed based on FCB values */
  fcbcode = (get_cds1(p->ps_fcb1.fcb_drive) ? 0 : 0xff) |
	    (get_cds1(p->ps_fcb2.fcb_drive) ? 0 : 0xff00);

  /* Transfer control to the executable                   */
  if (mode == LOADNGO)
  {
    /* build the user area on the stack                     */
    iregs FAR *irp = (iregs FAR *)ep->exec.stack - 1;

    /* start allocating REGs (as in MS-DOS - some demos expect them so --LG) */
    /* see http://www.beroset.com/asm/showregs.asm */
    irp->AX =
    irp->BX = fcbcode;
    irp->DX =
    irp->ES =
    irp->DS = FP_SEG(p);
    irp->CS = FP_SEG(ep->exec.start_addr);
    irp->SI =
    irp->IP = FP_OFF(ep->exec.start_addr);
    irp->DI = FP_OFF(ep->exec.stack);
    irp->BP = 0x91e; /* this is more or less random but some programs
                        expect 0x9 in the high byte of BP!! */
    irp->CX = 0xFF;
    irp->FLAGS = 0x200;

    if (InDOS)
      --InDOS;
    exec_user(irp, 1);

    /* We should never be here
       panic("KERNEL RETURNED!!!"); */
  }

  /* mode == LOAD */
  ep->exec.stack -= sizeof(UWORD);
  *(UWORD FAR *)ep->exec.stack = fcbcode;
}

/* Now find out how many paragraphs are available
   considering a threshold, trying HIGH then LOW */
STATIC int ExecMemLargest(UWORD *asize, UWORD threshold)
{
  int rc;
  if (mem_access_mode & 0x80)
  {
    mem_access_mode &= ~0x80;
    mem_access_mode |= 0x40;
    rc = DosMemLargest(asize);
    mem_access_mode &= ~0x40;
    /* less memory than the .COM/.EXE file has:
       try low memory first */
    if (rc != SUCCESS || *asize < threshold)
      rc = DosMemLargest(asize);
    mem_access_mode |= 0x80;
  }
  else
    rc = DosMemLargest(asize);
  return (*asize < threshold ? DE_NOMEM : rc);
}

STATIC int ExecMemAlloc(UWORD size, seg *para, UWORD *asize)
{
  /* We can still get an error on first fit if the above  */
  /* returned size was a best fit case                    */
  /* ModeLoadHigh = 80 = try high, then low               */
  int rc = DosMemAlloc(size, mem_access_mode, para, asize);

  if (rc != SUCCESS)
  {
    if (rc == DE_NOMEM)
    {
      rc = DosMemAlloc(0, LARGEST, para, asize);
      if ((mem_access_mode & 0x80) && (rc != SUCCESS))
      {
        mem_access_mode &= ~0x80;
        rc = DosMemAlloc(0, LARGEST, para, asize);
        mem_access_mode |= 0x80;
      }
    }
  }
  else
  {
    /* with no error, we got exactly what we asked for      */
    *asize = size;
  }

  /* This should never happen, but ... */
  if (rc == SUCCESS && *asize < size)
  {
    DosMemFree(*para);
    return DE_NOMEM;
  }
  return rc;
}

COUNT DosComLoader(BYTE FAR * namep, exec_blk * exp, COUNT mode, COUNT fd)
{
  UWORD mem;
  UWORD env, asize;
  UWORD com_size;
    {
      ULONG com_size_long = SftGetFsize(fd);
      /* maximally 64k - 256 bytes stack -
         256 bytes psp */
      com_size = ((UWORD)min(com_size_long, 0xfe00u) >> 4) + 0x10;
    }

    if ((mode & 0x7f) != OVERLAY)
    {
      COUNT rc;
      UBYTE UMBstate = uppermem_link;
      UBYTE orig_mem_access = mem_access_mode;
      
      if (mode & 0x80)
      {
        mem_access_mode |= 0x80;
        DosUmbLink(1);            /* link in UMB's */
      }
      
      /* COMFILES will always be loaded in largest area. is that true TE */
      /* yes, see RBIL, int21/ah=48 -- Bart */
      if ((rc = ChildEnv(exp->exec.env_seg, &env, namep)) == SUCCESS)
      {
        if ((rc = ExecMemLargest(&asize, com_size)) != SUCCESS ||
            /* Allocate our memory and pass back any errors */
            (rc = ExecMemAlloc(asize, &mem, &asize)) != SUCCESS)
          DosMemFree(env);
      }

      if (mode & 0x80)
      {
        DosUmbLink(UMBstate);       /* restore link state */
        mem_access_mode = orig_mem_access;
        mode &= 0x7f;
      }

      if (rc != SUCCESS)
        return rc;

#ifdef DEBUG
  printf("DosComLoader. Loading '%S' at %04x\n", namep, mem);
#endif
      ++mem;
    }

  /* Now load the executable                              */
    /* rewind to start */
    SftSeek(fd, 0, 0);
    /* MS DOS always only loads the very first 64KB - sizeof(psp) bytes.
       -- 1999/04/21 ska */
    /* read everything, but at most 64K - sizeof(PSP)             */
    /* !!! should be added check for reading success --avb */
    DosRWSft(fd, 0xff00, mode == OVERLAY /* memory already allocated */
                                ? MK_FP(exp->load.load_seg, 0)
                                : MK_FP(mem, sizeof(psp)), XFR_READ);
    DosCloseSft(fd, FALSE);

  if (mode != OVERLAY)
  {
    makePSP(mem, env, asize, namep);
    /* set asize to end of segment */
    if (asize > 0x1000)
      asize = 0x1000;
    if (asize < 0x11)
      return DE_NOMEM;
    asize -= 0x11;
    /* CP/M compatibility--size of first segment for .COM files
       while preserving the far call to 0:00c0 +
       copy in HMA at ffff:00d0 */
    MK_SEG_PTR(psp, mem)->ps_reentry = MK_FP(0xc - asize, asize << 4);
    asize <<= 4;
    asize += 0x10e;
    exp->exec.stack = MK_FP(mem, asize);
    exp->exec.start_addr = MK_FP(mem, 0x100);
    *MK_PTR(UWORD, mem, asize) = 0;
    load_transfer(mem, exp, mode);
  }
  return SUCCESS;
}

void return_user(void)
{
  /* restore parent                                       */
  psp _seg *p = MK_SEG_PTR(psp, cu_psp);

  /* When process returns - restore the isv               */
  setvec(0x22, p->ps_isv22);
  setvec(0x23, p->ps_isv23);
  setvec(0x24, p->ps_isv24);

  /* And free all process memory if not a TSR return      */
  network_redirector(REM_PROCESS_END);
  /* might be a good idea to do that after closing
     but doesn't help NET either TE */

  if (!tsr)
  {
    REG COUNT i;
    network_redirector(REM_CLOSEALL);
    for (i = 0; i < p->ps_maxfiles; i++)
    {
      DosClose(i);
    }
    FcbCloseAll();
    FreeProcessMem(FP_SEG(p));
  }

  {
    iregs FAR *irp = (iregs FAR *)MK_SEG_PTR(psp, cu_psp = p->ps_parent)->ps_stack;
    irp->CS = FP_SEG(p->ps_isv22);
    irp->IP = FP_OFF(p->ps_isv22);

    if (InDOS)
      --InDOS;
    exec_user(irp, 0);
  }
}

COUNT DosExeLoader(BYTE FAR * namep, exec_blk * exp, COUNT mode, COUNT fd)
{
  UWORD mem, env, start_seg, asize;
  UWORD image_size;

    /* compute image size by removing the offset from the   */
    /* number pages scaled to bytes plus the remainder and  */
    /* the psp                                              */
    /*  First scale the size and remove the offset          */
    if (ExeHeader.exPages >= 2048)
      return DE_INVLDDATA; /* we're not able to get >=1MB in dos memory */
    image_size = ExeHeader.exPages * 32 - ExeHeader.exHeaderSize;

    /* We should not attempt to allocate memory if we are overlaying
       the current process, because the new process will simply re-use
       the block we already have allocated. This was causing execl() to
       fail in applications which use it to overlay (replace) the current
       exe file with a new one. Jun 11, 2000 --rbc */
    
    if ((mode & 0x7f) != OVERLAY)
    {
      UBYTE UMBstate = uppermem_link;
      UBYTE orig_mem_access = mem_access_mode;
      COUNT rc;
      
      /* Clone the environement and create a memory arena     */
      if (mode & 0x80)
      {
        DosUmbLink(1);          /* link in UMB's */
        mem_access_mode |= 0x80;
      }
      
      if ((rc = ChildEnv(exp->exec.env_seg, &env, namep)) == SUCCESS)
      {
        image_size += sizeof(psp) / 16;        /*TE 03/20/01 */
        /* Now find out how many paragraphs are available       */
        if ((rc = ExecMemLargest(&asize, image_size + ExeHeader.exMinAlloc)) == SUCCESS)
        {
          unsigned max_size = image_size + ExeHeader.exMaxAlloc;
          /* second test is for overflow (avoiding longs) --
             exMaxAlloc can be high */
          if (max_size > asize || max_size < image_size ||
              /* TE if ExeHeader.exMinAlloc == ExeHeader.exMaxAlloc == 0,
                 DOS will allocate the largest possible memory area
                 and load the image as high as possible into it.
                 discovered (and after that found in RBIL), when testing NET */
              (ExeHeader.exMinAlloc | ExeHeader.exMaxAlloc) == 0)
            max_size = asize;
          /* Allocate our memory and pass back any errors */
          rc = ExecMemAlloc(max_size, &mem, &asize);
        }
        if (rc != SUCCESS)
          DosMemFree(env);
      }
      
      if (mode & 0x80)
      {
        mem_access_mode = orig_mem_access; /* restore old situation */
        DosUmbLink(UMBstate);     /* restore link state */
      }

      if (rc != SUCCESS)
        return rc;
      
      mode &= 0x7f; /* forget about high loading from now on */
      
#ifdef DEBUG
      printf("DosExeLoader. Loading '%S' at %04x\n", namep, mem);
#endif
      /* memory found large enough - continue processing      */
    }

    /* Now load the executable                              */
    /* offset to start of image                             */
    if (SftSeek(fd, ExeHeader.exHeaderSize * 16UL, 0) != SUCCESS)
    {
      if (mode != OVERLAY)
      {
        DosMemFree(mem);
        DosMemFree(env);
      }
      return DE_INVLDDATA;
    }
    
    /* create the start seg for later computations          */
    start_seg = exp->load.load_seg;
    if (mode != OVERLAY)
    {
      start_seg = mem + 1 + sizeof(psp) / 16;
      if ((ExeHeader.exMinAlloc | ExeHeader.exMaxAlloc) == 0)
        /* then the image should be placed as high as possible */
        start_seg += MK_SEG_PTR(const mcb, mem)->m_size - image_size;
      image_size -= sizeof(psp) / 16;
    }

  /* read in the image in 32256 chunks                      */
  {
    seg_t sp = start_seg;
    do
    {
      int toRead = CHUNK;
      if (image_size < CHUNK/16)
      {
        toRead = image_size*16;
        image_size = CHUNK/16;
      }
      if ((int)DosRWSft(fd, toRead, MK_FP(sp, 0), XFR_READ) < toRead)
        break;
      sp += CHUNK/16;
    } while (image_size -= CHUNK/16);
  }

  /* relocate the image for new segment				*/
  SftSeek(fd, ExeHeader.exRelocTable, 0);
  {
    unsigned i;
    for (i = ExeHeader.exRelocItems; i; i--)
    {
      UWORD reloc[2];
      if (DosRWSft(fd, sizeof reloc, reloc, XFR_READ) != sizeof reloc)
      {
        if (mode != OVERLAY)
        {
          DosMemFree(mem);
          DosMemFree(env);
        }
        return DE_INVLDDATA;
      }
      *MK_PTR(seg_t, reloc[1] + start_seg, reloc[0]) += start_seg;
    }
  }

  /* and finally close the file                           */
  DosCloseSft(fd, FALSE);

  /* exit here for overlay                                */
  if (mode != OVERLAY)
  {
    mem++;
    makePSP(mem, env, asize, namep);
    exp->exec.stack =
      MK_FP(ExeHeader.exInitSS + start_seg, ExeHeader.exInitSP);
    exp->exec.start_addr =
      MK_FP(ExeHeader.exInitCS + start_seg, ExeHeader.exInitIP);

    /* Transfer control to the executable                   */
    load_transfer(mem, exp, mode);
  }
  return SUCCESS;
}

/* mode = LOAD or EXECUTE
   ep = EXE block
   lp = filename to load (string)

   leb = local copy of exe block
 */
COUNT DosExec(COUNT mode, exec_blk FAR * ep, BYTE FAR * lp)
{
  COUNT rc;
  COUNT fd;

  if ((mode & 0x7f) > 3 || (mode & 0x7f) == 2)
    return DE_INVLDFMT; 

  fmemcpy(&TempExeBlock, ep, sizeof(exec_blk));
  /* If file not found - free ram and return error        */

  if (IsDevice(lp) ||        /* we don't want to execute C:>NUL */
      (fd = (short)DosOpenSft(lp, O_LEGACY | O_OPEN | O_RDONLY, 0)) < 0)
  {
    return DE_FILENOTFND;
  }
  
  rc = (int)DosRWSft(fd, sizeof(exe_header), &ExeHeader, XFR_READ);

  if (rc == sizeof(exe_header) &&
      (ExeHeader.exSignature == MAGIC || ExeHeader.exSignature == OLD_MAGIC))
  {
    rc = DosExeLoader(lp, &TempExeBlock, mode, fd);
  }
  else if (rc != 0)
  {
    rc = DosComLoader(lp, &TempExeBlock, mode, fd);
  }

  DosCloseSft(fd, FALSE);

  if (mode == LOAD && rc == SUCCESS)
    fmemcpy(ep, &TempExeBlock, sizeof(exec_blk));

  return rc;
}

#include "config.h" /* config structure definition */

/* start process 0 (the shell) */
void ASMCFUNC P_0(const struct config FAR *);
#ifdef __WATCOMC__
# pragma aux (cdecl) P_0 aborts
#endif

void ASMCFUNC P_0(const struct config FAR *Config)
{
  int mode = Config->cfgP_0_startmode;

  const char FAR *p = MK_PTR(const char, FP_SEG(Config), Config->cfgShell);
  PStr endp = Shell;
  while ((*endp = *p++) != '\0' &&
         ++endp < Shell + sizeofShell - 4); /* 4 for 0,ctCount and "\r\0" */

  for (;;) /* endless shell load loop - reboot or shut down to exit it! */
  {
    PStr tailp = Shell - 1;

    *endp = '\r', endp[1] = '\0'; /* terminate command line */
    endp += 2;

    /* find end of command name */
    do tailp++; while ((UBYTE)*tailp > ' ' && *tailp != '/');

    /* shift tail to right by 2 to make room for '\0' and ctCount */
    {
      PStr p = endp;
      do
      {
        p--;
        p[2] = p[0];
      } while (p > tailp);
    }

    /* terminate name */
    *tailp = '\0';

    /* init length of command line tail (ctCount field) */
    tailp++;
    *tailp = (UBYTE)(endp - tailp - 1); /* without "\r\0" */

    {
      exec_blk exb;
      exb.exec.env_seg = DOS_PSP + 8;
      exb.exec.cmd_line = (CommandTail *)tailp;
      /*exb.exec.fcb_1 = exb.exec.fcb_2 = NULL;*/ /* unimportant */

#ifdef DEBUG
      printf("Process 0 starting: %s%s\n\n", Shell, tailp + 1);
#endif
      res_DosExec(mode, &exb, Shell);
    }

    /* failure or exit */
    put_string("\nBad or missing Command Interpreter\n"
                 "Enter the full shell command line:\n");
    put_string(Shell);
    *endp = '\n'; /* replace "\r\0" by "\n\0" */
    put_string(++tailp);
    endp = Shell + res_read(STDIN, Shell, sizeofShell) - 2; /* exclude "\r\n" */
  }
}
