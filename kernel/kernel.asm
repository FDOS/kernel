;
; File:
;                          kernel.asm
; Description:
;                       kernel start-up code
;
;                    Copyright (c) 1995, 1996
;                       Pasquale J. Villani
;                       All Rights Reserved
;
; This file is part of DOS-C.
;
; DOS-C is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version
; 2, or (at your option) any later version.
;
; DOS-C is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
; the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
; $Id: kernel.asm 1705 2012-02-07 08:10:33Z perditionc $
;

                %include "segs.inc"
                %include "stacks.inc"
                %include "ludivmul.inc"


segment PSP

                extern  _ReqPktPtr

STACK_SIZE      equ     384/2           ; stack allocated in words

;************************************************************       
; KERNEL BEGINS HERE, i.e. this is byte 0 of KERNEL.SYS
;************************************************************       

%ifidn __OUTPUT_FORMAT__, obj
..start:
%endif
bootloadunit:		; (byte of short jump re-used)
entry:
                jmp short realentry

;************************************************************       
; KERNEL CONFIGURATION AREA
; this is copied up on the very beginning
; it's a good idea to keep this in sync with KConfig.h
;************************************************************       
                global _LowKernelConfig                                        
_LowKernelConfig:
config_signature:
                db 'CONFIG'             ; constant
                dw configend-configstart; size of config area
                                        ; to be checked !!!

configstart:

DLASortByDriveNo            db 0        ; sort disks by drive order
InitDiskShowDriveAssignment db 1        ;
SkipConfigSeconds           db 2        ;
ForceLBA                    db 0        ;
GlobalEnableLBAsupport      db 1        ;
BootHarddiskSeconds         db 0        ;

; The following VERSION resource must be keep in sync with VERSION.H
Version_OemID               db 0xFD     ; OEM_ID
Version_Major               db 2
Version_Revision            dw 43       ; REVISION_SEQ
Version_Release             dw 1        ; 0=release build, >0=svn#

CheckDebugger:	            db 0        ; 0 = no check, 1 = check, 2 = assume present
Verbose	                    db 0        ; -1 = quiet, 0 = normal, 1 = verbose

PartitionMode				db 0x1f		; bits 0-1: 01=GPT if found, 00=MBR if found, 11=Hybrid, GPT first then MBR, 10=Hybrid, MBR first then GPT
										;           in hybrid mode, EE partitions ignored, drives assigned by GPT or MBR first based on hybrid type
										; bits 2-4: 001=mount ESP (usually FAT32) partition, 010=mount MS Basic partitions, 100=mount unknown partitions
										;           111=attempt to mount all paritions, 110=attempt to mount all but ESP partitions
										; bits 5-7: reserved, 0 else undefined behavior

configend:
kernel_config_size: equ configend - config_signature
	; must be below-or-equal the size of struct _KernelConfig
	;  in the file kconfig.h !

		times (32 - 4) - ($ - $$) db 0
bootloadstack:		dd 0


;************************************************************       
; KERNEL CONFIGURATION AREA END
;************************************************************       


;************************************************************       
; KERNEL real entry (at ~60:20)
;                               
; moves the INIT part of kernel.sys to high memory (~9000:0)
; then jumps there
; to aid debugging, some '123' messages are output
; this area is discardable and used as temporary PSP for the
; init sequence
;************************************************************       

                cpu 8086                ; (keep initial entry compatible)

global realentry
realentry:                              ; execution continues here
	push cs
	pop ds
	xor di, di
	mov byte [di + bootloadunit - $$], bl
	push bp
	mov word [di + bootloadstack - $$], sp
	mov word [di + bootloadstack + 2 - $$], ss
	jmp entry_common


	times 0C0h - ($ - $$) nop	; magic offset (used by exeflat)
entry_common:
%ifndef QUIET
                push ax
                push bx
                pushf              
                mov ax, 0e31h           ; '1' Tracecode - kernel entered
                mov bx, 00f0h                                        
                int 010h
                popf
                pop bx
                pop ax
%endif

	push cs
	pop ds
                jmp     IGROUP:kernel_start

beyond_entry:   times   256-(beyond_entry-entry) db 0
                                        ; scratch area for data (DOS_PSP)
_master_env equ $ - 128
global _master_env

segment INIT_TEXT

%ifdef TEST_FILL_INIT_TEXT
 %macro step 0
 %if _LFSR & 1
  %assign _LFSR (_LFSR >> 1) ^ 0x80200003
 %else
  %assign _LFSR (_LFSR >> 1)
 %endif
 %endmacro

	align 16
 %assign _LFSR 1
 %rep 1024 * 8
	dd _LFSR
	step
 %endrep
%endif

                extern  _FreeDOSmain
                extern  _query_cpu
                
                ;
                ; kernel start-up
                ;
kernel_start:
	cld

%ifndef QUIET
                push bx
                pushf              
                mov ax, 0e32h           ; '2' Tracecode - kernel entered
                mov bx, 00f0h                                        
                int 010h
                popf
                pop bx
%endif


extern _kernel_command_line

		; INP:	ds => entry section, with CONFIG block
		;		and compressed entry help data
		;		(actually always used now)
initialise_command_line_buffer:
	mov dx, I_GROUP
	mov es, dx

	mov bl, [bootloadunit]
	lds si, [bootloadstack]		; -> original ss:sp - 2
	lea ax, [si + 2]		; ax = original sp
	mov si, word [si]		; si = original bp

		; Note that the kernel command line buffer in
		;  the init data segment is pre-initialised to
		;  hold 0x00 0xFF in the first two bytes. This
		;  is used to indicate no command line present,
		;  as opposed to an empty command line which
		;  will hold 0x00 0x00.
		; If any of the branches to .none are taken then
		;  the buffer is not modified so it retains the
		;  0x00 0xFF contents.
	cmp si, 114h			; buffer fits below ss:bp ?
	jb .none			; no -->
	cmp word [si - 14h], "CL"	; signature passed to us ?
	jne .none			; no -->
	lea si, [si - 114h]		; -> command line buffer
	cmp ax, si			; stack top starts below-or-equal buffer ?
	ja .none			; no -->
	mov di, _kernel_command_line	; our buffer
	mov cx, 255
	xor ax, ax
	push di
	rep movsb		; copy up to 255 bytes
	stosb			; truncate
	pop di
	mov ch, 1		; cx = 256
	repne scasb		; scan for terminator
	rep stosb		; clear remainder of buffer
				; (make sure we do not have 0x00 0xFF
				;  even if the command line given is
				;  actually the empty string)
.none:

                cli
                mov     ss, dx
                mov     sp, init_tos
                int     12h             ; move init text+data to higher memory
                mov     cl,6
                shl     ax,cl           ; convert kb to para
                mov     dx,15 + INITSIZE
                mov     cl,4
                shr     dx,cl
                sub     ax,dx
                mov     es,ax
                mov     dx,INITTEXTSIZE ; para aligned
                shr     dx,cl
                add     ax,dx
                mov     ss,ax           ; set SS to init data segment
                sti                     ; now enable them
                mov     ax,cs
                mov     dx,__HMATextEnd ; para aligned
                shr     dx,cl
%ifdef WATCOM
                add     ax,dx
%endif
                mov     ds,ax
                mov     si,-2 + INITSIZE; word aligned
                lea     cx,[si+2]
                mov     di,si
                shr     cx,1
                std                     ; if there's overlap only std is safe
                rep     movsw

                                        ; move HMA_TEXT to higher memory
                sub     ax,dx
                mov     ds,ax           ; ds = HMA_TEXT
                mov     ax,es
                sub     ax,dx
                mov     es,ax           ; es = new HMA_TEXT

                mov     si,-2 + __HMATextEnd
                lea     cx,[si+2]
                mov     di,si
                shr     cx,1
                rep     movsw
                
                cld
%ifndef WATCOM                          ; for WATCOM: CS equal for HMA and INIT
                add     ax,dx
                mov     es,ax           ; otherwise CS -> init_text
%endif
                push    es
                mov     ax,cont
                push    ax
                retf
cont:           ; Now set up call frame
                mov     ds,[cs:_INIT_DGROUP]
                mov     bp,sp           ; and set up stack frame for c

%ifndef QUIET
                push bx
                pushf              
                mov ax, 0e33h           ; '3' Tracecode - kernel entered
                mov bx, 00f0h                                        
                int 010h
                popf
                pop bx
%endif

                mov     byte [_BootDrive],bl ; tell where we came from

;!!             int     11h
;!!             mov     cl,6
;!!             shr     al,cl
;!!             inc     al
;!!                mov     byte [_NumFloppies],al ; and how many

                call _query_cpu
%if XCPU != 86
 %if XCPU < 186 || (XCPU % 100) != 86 || (XCPU / 100) > 9
  %fatal Unknown CPU level defined
 %endif
                cmp     al, (XCPU / 100)
                jb      cpu_abort       ; if CPU not supported -->

                cpu XCPU
%endif
                mov     [_CPULevel], al

initialise_kernel_config:
extern _InitKernelConfig

	mov ax, ss			; => init data segment
	mov si, 60h
	mov ds, si			; => entry section
	mov si, _LowKernelConfig	; -> our CONFIG block
	mov es, ax			; => init data segment
	mov di, _InitKernelConfig	; -> init's CONFIG block buffer
	mov cx, kernel_config_size / 2	; size that we support
	rep movsw			; copy it over
%if kernel_config_size & 1
	movsb				; allow odd size
%endif
	mov ds, ax			; => init data segment

check_debugger_present:
extern _debugger_present

	mov al, 1			; assume debugger present
	cmp byte [di - kernel_config_size + (CheckDebugger - config_signature)], 1
	ja .skip_ints_00_06		; 2 means assume present
	jb .absent			; 0 means assume absent
	clc				; 1 means check
	int3				; break to debugger
	jc .skip_ints_00_06
		; The debugger should set CY here to indicate its
		;  presence. The flag set is checked later to skip
		;  overwriting the interrupt vectors 00h, 01h, 03h,
		;  and 06h. This logic is taken from lDOS init.
.absent:
	xor ax, ax			; no debugger present
.skip_ints_00_06:
	mov byte [_debugger_present], al

           jmp     _FreeDOSmain

%if XCPU != 86
        cpu 8086

cpu_abort:
        mov ah, 0Fh
        int 10h                 ; get video mode, bh = active page

        call .first             ; print string that follows (address pushed by call)

%define LOADNAME "FreeDOS"
        db 13,10                ; (to emit a blank line after the tracecodes)
        db 13,10
        db LOADNAME, " load error: An 80", '0'+(XCPU / 100)
        db   "86 processor or higher is required by this build.",13,10
        db "To use ", LOADNAME, " on this processor please"
        db  " obtain a compatible build.",13,10
        db 13,10
        db "Press any key to reboot.",13,10
        db 0

.display:
        mov ah, 0Eh
        mov bl, 07h             ; page in bh, bl = colour for some modes
        int 10h                 ; write character (may change bp!)

        db 0A8h                 ; [test al,imm8] skip "pop si" [=imm8] after the first iteration 
.first:
        pop si                  ; (first iteration only) get message address from stack
        cs lodsb                ; get character
        test al, al             ; zero ?
        jnz .display            ; no, display and get next character -->

        xor ax, ax
        xor dx, dx
        int 13h                 ; reset floppy disks
        xor ax, ax
        mov dl, 80h
        int 13h                 ; reset hard disks

                                ; this "test ax, imm16" opcode is used to
        db 0A9h                 ; skip "sti" \ "hlt" [=imm16] during first iteration
.wait:
        sti
        hlt                     ; idle while waiting for keystroke
        mov ah, 01h
        int 16h                 ; get keystroke
        jz .wait                ; none available, loop -->

        mov ah, 00h
        int 16h                 ; remove keystroke from buffer

        int 19h                 ; reboot
        jmp short $             ; (in case it returns, which it shouldn't)

        cpu XCPU
%endif        ; XCPU != 86


segment INIT_TEXT_END


;************************************************************       
; KERNEL CODE AREA END
; the NUL device
;************************************************************       

segment CONST

                ;
                ; NUL device strategy
                ;
                global  _nul_strtgy
                extern GenStrategy
_nul_strtgy:
                jmp LGROUP:GenStrategy

                ;
                ; NUL device interrupt
                ;
                global  _nul_intr
_nul_intr:
                push    es
                push    bx
                mov     bx,LGROUP
                mov     es,bx
                les     bx,[es:_ReqPktPtr]  ;es:bx--> rqheadr
                cmp     byte [es:bx+2],4    ;if read, set 0 read
                jne     no_nul_read
                mov     word [es:bx+12h],0
no_nul_read:
                or      word [es:bx+3],100h ;set "done" flag
                pop     bx
                pop     es
                retf

segment _LOWTEXT

                ; low interrupt vectors 10h,13h,15h,19h,1Bh
                ; these need to be at 0070:0100 (see RBIL memory.lst)
                global _intvec_table
_intvec_table:  db 10h
                dd 0
                ; used by int13 handler and get/set via int 2f/13h
                global  _BIOSInt13 ; BIOS provided disk handler 
                global  _UserInt13 ; actual disk handler used by kernel
                db 13h
_BIOSInt13:     dd 0  
                db 15h
                dd 0
                ; used for cleanup on reboot
                global  _BIOSInt19
                db 19h
_BIOSInt19:     dd 0
                db 1Bh
                dd 0
                ; default to using BIOS provided disk handler
                db 13h
_UserInt13:     dd 0 

                ; floppy parameter table
                global _int1e_table
_int1e_table:   times 0eh db 0

;************************************************************       
; KERNEL FIXED DATA AREA 
;************************************************************       


segment _FIXED_DATA

; Because of the following bytes of data, THIS MODULE MUST BE THE FIRST
; IN THE LINK SEQUENCE.  THE BYTE AT DS:0004 determines the SDA format in
; use.  A 0 indicates MS-DOS 3.X style, a 1 indicates MS-DOS 4.0-6.X style.
                global  DATASTART
DATASTART:
                global  _DATASTART
_DATASTART:
dos_data        db      0
                dw      kernel_start
                db      0               ; padding
                dw      1               ; Hardcoded MS-DOS 4.0+ style

                times (0eh - ($ - DATASTART)) db 0
                global  _NetBios
_NetBios        dw      0               ; NetBios Number

                times (26h - 0ch - ($ - DATASTART)) db 0

; Globally referenced variables - WARNING: DO NOT CHANGE ORDER
; BECAUSE THEY ARE DOCUMENTED AS UNDOCUMENTED (?) AND HAVE
; MANY MULTIPLEX PROGRAMS AND TSRs ACCESSING THEM
                global  _NetRetry
_NetRetry       dw      3               ;-000c network retry count
                global  _NetDelay
_NetDelay       dw      1               ;-000a network delay count
                global  _DskBuffer
_DskBuffer      dd      -1              ;-0008 current dos disk buffer
                global  _inputptr
_inputptr       dw      0               ;-0004 Unread con input
                global  _first_mcb
_first_mcb      dw      0               ;-0002 Start of user memory
                global  _DPBp
                global  MARK0026H
; A reference seems to indicate that this should start at offset 26h.
MARK0026H       equ     $
_DPBp           dd      -1              ; 0000 First drive Parameter Block
                global  _sfthead
_sfthead        dd      0               ; 0004 System File Table head
                global  _clock
_clock          dd      0               ; 0008 CLOCK$ device
                global  _syscon
_syscon         dw      _con_dev,LGROUP ; 000c console device
                global  _maxsecsize
_maxsecsize     dw      512             ; 0010 maximum bytes/sector of any block device
                dd      0               ; 0012 pointer to buffers info structure
                global  _CDSp
_CDSp           dd      0               ; 0016 Current Directory Structure
                global  _FCBp
_FCBp           dd      0               ; 001a FCB table pointer
                global  _nprotfcb
_nprotfcb       dw      0               ; 001e number of protected fcbs
                global  _nblkdev
_nblkdev        db      0               ; 0020 number of block devices
                global  _lastdrive
_lastdrive      db      0               ; 0021 value of last drive
                global  _nul_dev
_nul_dev:           ; 0022 device chain root
                extern  _con_dev
                dw      _con_dev, LGROUP
                                        ; next is con_dev at init time.  
                dw      8004h           ; attributes = char device, NUL bit set
                dw      _nul_strtgy
                dw      _nul_intr
                db      'NUL     '
                global  _njoined
_njoined        db      0               ; 0034 number of joined devices
                dw      0               ; 0035 DOS 4 near pointer to special names (always zero in DOS 5) [setver precursor]
                global  _setverPtr
_setverPtr      dw      0,0             ; 0037 setver list (far pointer, set by setver driver)
                dw      0               ; 003B cs offset for fix a20
                dw      0               ; 003D psp of last umb exec
                global _LoL_nbuffers
_LoL_nbuffers   dw      1               ; 003F number of buffers
                dw      1               ; 0041 size of pre-read buffer
                global  _BootDrive
_BootDrive      db      1               ; 0043 drive we booted from   

                global  _CPULevel
_CPULevel       db      0               ; 0044 cpu type (MSDOS >0 indicates dword moves ok, ie 386+)
                                        ; unless compatibility issues arise FD uses
                                        ; 0=808x, 1=18x, 2=286, 3=386+
                                        ; see cpu.asm, use >= as may add checks for 486 ...

                dw      0               ; 0045 Extended memory in KBytes
buf_info:               
                global  _firstbuf
_firstbuf       dd      0               ; 0047 disk buffer chain
                dw      0               ; 004B Number of dirty buffers
                dd      0               ; 004D pre-read buffer
                dw      0               ; 0051 number of look-ahead buffers
                global  _bufloc
_bufloc         db      0               ; 0053 00=conv 01=HMA
                global  _deblock_buf
_deblock_buf    dd      0               ; 0054 deblock buffer
                times 3 db 0            ; 0058 unknown
                dw      0               ; 005B unknown
                db      0, 0FFh, 0      ; 005D unknown
                global _VgaSet
_VgaSet         db      0               ; 0060 unknown
                dw      0               ; 0061 unknown
                global  _uppermem_link
_uppermem_link  db      0               ; 0063 upper memory link flag
_min_pars       dw      0               ; 0064 minimum paragraphs of memory 
                                        ;      required by program being EXECed
                global  _uppermem_root
_uppermem_root  dw      0ffffh          ; 0066 dmd_upper_root (usually 9fff)
_last_para      dw      0               ; 0068 para of last mem search
SysVarEnd:

;; FreeDOS specific entries
;; all variables below this point are subject to relocation.
;; programs should not rely on any values below this point!!!

                global  _os_setver_minor
_os_setver_minor        db      0
                global  _os_setver_major
_os_setver_major        db      5
                global  _os_minor
_os_minor       db      0
                global  _os_major              
_os_major       db      5
_rev_number     db      0
                global  _version_flags         
_version_flags  db      0

                global  os_release
                extern  _os_release
os_release      dw      _os_release

%IFDEF WIN31SUPPORT
                global  _winStartupInfo, _winInstanced
_winInstanced    dw 0 ; set to 1 on WinInit broadcast, 0 on WinExit broadcast
_winStartupInfo:
                dw 0 ; structure version (same as windows version)
                dd 0 ; next startup info structure, 0:0h marks end
                dd 0 ; far pointer to name virtual device file or 0:0h
                dd 0 ; far pointer, reference data for virtual device driver
 %ifnidni __OUTPUT_FORMAT__, elf
                dw instance_table,seg instance_table ; array of instance data
 %else
                dw instance_table ; array of instance data
global _winseg1
_winseg1:	dw 0
 %endif
instance_table: ; should include stacks, Win may auto determine SDA region
                ; we simply include whole DOS data segment
 %ifnidni __OUTPUT_FORMAT__, elf
                dw seg _DATASTART, 0 ; [SEG:OFF] address of region's base
                dw _markEndInstanceData wrt seg _DATASTART ; size in bytes
 %else
global _winseg2
_winseg2:	dw 0
                dw 0 ; [SEG:OFF] address of region's base
global _winseg3
_winseg3:	dw 0 ; size in bytes
 %endif
                dd 0 ; 0 marks end of table
                dw 0 ; and 0 length for end of instance_table entry
                global  _winPatchTable
_winPatchTable: ; returns offsets to various internal variables
                dw 0x0006      ; DOS version, major# in low byte, eg. 6.00
                dw save_DS     ; where DS stored during int21h dispatch
                dw save_BX     ; where BX stored during int21h dispatch
                dw _InDOS      ; offset of InDOS flag
                dw _MachineId  ; offset to variable containing MachineID
                dw _CritPatch  ; offset of to array of offsets to patch
                               ; NOTE: this points to a null terminated
                               ; array of offsets of critical section bytes
                               ; to patch, for now we can just point this
                               ; to an empty table
                               ; ie we just point to a 0 word to mark end
                dw _uppermem_root ; seg of last arena header in conv memory
                                  ; this matches MS DOS's location, but 
                                  ; do we have the same meaning?
%ENDIF ; WIN31SUPPORT

;;  The first 5 sft entries appear to have to be at DS:00cc
                times (0cch - ($ - DATASTART)) db 0
                global _firstsftt
_firstsftt:             
                dd -1                   ; link to next
                dw 5                    ; count 
                times 5*59 db 0         ; reserve space for the 5 sft entries
                db 0                    ; pad byte so next value on even boundary        

; Some references seem to indicate that this data should start at 01fbh in
; order to maintain 100% MS-DOS compatibility.
                times (01fbh - ($ - DATASTART)) db 0

                global  MARK01FBH
MARK01FBH       equ     $
                global  _local_buffer   ; local_buffer is 256 bytes long
                                        ; so it overflows into kb_buf!!
        ; only when kb_buf is used, local_buffer is limited to 128 bytes.
_local_buffer:  times 128 db 0
                global  _kb_buf
_kb_buf db      128,0                   ; initialise buffer to empty
                times 128+1 db 0   ; room for 128 byte readline + LF
;
; Variables that follow are documented as part of the DOS 4.0-6.X swappable
; data area in Ralf Browns Interrupt List #56
;
; this byte is used for ^P support
                global  _PrinterEcho
_PrinterEcho    db      0               ;-34 -  0 = no printer echo, ~0 echo
                global  _verify_ena
_verify_ena     db      0               ; ~0, write with verify

; this byte is used for TABs (shared by all char device writes??)
                global _scr_pos
_scr_pos        db      0               ; Current Cursor Column
                global  _switchar
_switchar       db      '/'             ;-31 - switch char
                global  _mem_access_mode
_mem_access_mode db     0               ;-30 -  memory allocation strategy
                global  sharing_flag
sharing_flag    db      0               ; 00 = sharing module not loaded
                                        ; 01 = sharing module loaded, but
                                        ;      open/close for block devices
                                        ;      disabled
                                        ; FF = sharing module loaded,
                                        ;      open/close for block devices
                                        ;      enabled (not implemented)
                global  _net_set_count
_net_set_count   db      1               ;-28 -  count the name below was set
                global  _net_name
_net_name       db      '               ' ;-27 - 15 Character Network Name
                db      00                ; Terminating 0 byte


;
;       Variables contained the the "STATE_DATA" segment contain
;       information about the STATE of the current DOS Process. These
;       variables must be preserved regardless of the state of the INDOS
;       flag.
;
;       All variables that appear in "STATE_DATA" **MUST** be declared
;       in this file as the offsets from the INTERNAL_DATA variable are
;       critical to the DOS applications that modify this data area.
;
;
                global  _ErrorMode, _InDOS
                global  _CritErrLocus, _CritErrCode
                global  _CritErrAction, _CritErrClass
                global  _CritErrDev, _CritErrDrive
                global  _dta
                global  _cu_psp, _default_drive
                global  _break_ena
                global  _return_code
                global  _internal_data

; ensure offset of critical patch table remains fixed, some programs hard code offset
                times (0315h - ($ - DATASTART)) db 0
                global  _CritPatch
_CritPatch      dw      0               ;-11 zero list of patched critical
                dw      0               ;    section variables
                dw      0               ;    DOS puts 0d0ch here but some
                dw      0               ;    progs really write to that addr.
                dw      0               ;-03 - critical patch list terminator
                db      90h             ;-01 - unused, NOP pad byte
_internal_data:              ; <-- Address returned by INT21/5D06
_ErrorMode      db      0               ; 00 - Critical Error Flag
_InDOS          db      0               ; 01 - Indos Flag
_CritErrDrive   db      0               ; 02 - Drive on write protect error
_CritErrLocus   db      0               ; 03 - Error Locus
_CritErrCode    dw      0               ; 04 - DOS format error Code
_CritErrAction  db      0               ; 06 - Error Action Code
_CritErrClass   db      0               ; 07 - Error Class
_CritErrDev     dd      0               ; 08 - Failing Device Address
_dta            dd      0               ; 0C - current DTA
_cu_psp         dw      0               ; 10 - Current PSP
break_sp        dw      0               ; 12 - used in int 23
_return_code    dw      0               ; 14 - return code from process
_default_drive  db      0               ; 16 - Current Drive
_break_ena      db      1               ; 17 - Break Flag (default TRUE)
                db      0               ; 18 - flag, code page switching
                db      0               ; 19 - flag, copy of 18 on int 24h abort

                global  _swap_always, _swap_indos
_swap_always:

                global  _Int21AX
_Int21AX        dw      0               ; 1A - AX from last Int 21

                global  owning_psp, _MachineId
owning_psp      dw      0               ; 1C - owning psp
_MachineId      dw      0               ; 1E - remote machine ID
                dw      0               ; 20 - First usable mcb
                dw      0               ; 22 - Best usable mcb
                dw      0               ; 24 - Last usable mcb
                dw      0               ; 26 - memory size in paragraphs
                dw      0               ; 28 - unknown
                db      0               ; 2A - unknown
                db      0               ; 2B - unknown
                db      0               ; 2C - unknown
                global  _break_flg
_break_flg      db      0               ; 2D - Program aborted by ^C
                db      0               ; 2E - unknown
                db      0               ; 2F - not referenced
                global  _DayOfMonth
_DayOfMonth     db      1               ; 30 - day of month
                global  _Month
_Month          db      1               ; 31 - month
                global  _YearsSince1980
_YearsSince1980 dw      0               ; 32 - year since 1980
daysSince1980   dw      0FFFFh          ; 34 - number of days since epoch
                                        ; force rebuild on first clock read
                global  _DayOfWeek
_DayOfWeek      db      2               ; 36 - day of week
_console_swap   db      0               ; 37 console swapped during read from dev
                global  _dosidle_flag        
_dosidle_flag   db      1               ; 38 - safe to call int28 if nonzero
                global  _abort_progress
_abort_progress db      0               ; 39 - abort in progress
                global  _CharReqHdr
_CharReqHdr:
                global  _ClkReqHdr
_ClkReqHdr      times 30 db 0      ; 3A - Device driver request header
                dd      0               ; 58 - pointer to driver entry
                global  _MediaReqHdr
_MediaReqHdr    times 22 db 0      ; 5C - Device driver request header
                global  _IoReqHdr
_IoReqHdr       times 30 db 0      ; 72 - Device driver request header
                times 6 db 0       ; 90 - unknown
                global  _ClkRecord
_ClkRecord      times 6 db 0       ; 96 - CLOCK$ transfer record
                dw      0               ; 9C - unknown
                global  __PriPathBuffer
__PriPathBuffer times 80h db 0     ; 9E - buffer for file name
                global  __SecPathBuffer
__SecPathBuffer times 80h db 0     ;11E - buffer for file name
                global  _sda_tmp_dm
_sda_tmp_dm     times 21 db 0      ;19E - 21 byte srch state
                global  _SearchDir
_SearchDir      times 32 db 0      ;1B3 - 32 byte dir entry
                global  _TempCDS
_TempCDS        times 88 db 0      ;1D3 - TemporaryCDS buffer
                global  _DirEntBuffer
_DirEntBuffer   times 32 db 0      ;22B - space enough for 1 dir entry
                global  _wAttr
_wAttr          dw      0               ;24B - extended FCB file attribute


                global  _SAttr
_SAttr          db      0           ;24D - Attribute Mask for Dir Search
                global  _OpenMode
_OpenMode       db      0           ;24E - File Open Attribute

                times 3 db 0
                global  _Server_Call
_Server_Call    db      0           ;252 - Server call Func 5D sub 0
                db      0
                ; Pad to 05CCh
                times (25ch - ($ - _internal_data)) db 0

                global  _term_type      ; used by break and critical error
_term_type      db      0               ;25C -  handlers during termination
		; ecm: 00h = normal terminate,
		;	01h = control-c terminate,
		;	02h = critical error abort,
		;	03h = TSR terminate
                db      0               ;25D - padding
                global  term_psp
term_psp        dw  0                   ;25E - 0??
                global  int24_esbp
int24_esbp      times 2 dw 0       ;260 - pointer to criticalerr DPB
                global  _user_r, int21regs_off, int21regs_seg
_user_r:
int21regs_off   dw      0               ;264 - pointer to int21h stack frame
int21regs_seg   dw      0               ;       i.e. points stack frame (SS:SP) with user registers when int21h called
                global  critical_sp
critical_sp     dw      0               ;268 - critical error internal stack, store SP during int24h
                global  current_ddsc
current_ddsc    times 2 dw 0            ;26A - pointer to DPB for ???

diskbuf_seg     dw      0               ;26E - segment of disk buffer
                dd      0               ;270 - saving partial cluster number??? - see https://faydoc.tripod.com/structures/16/1690.htm
				dw      0
				dw      0               ;276 - temp word
				db      0               ;278 - media id returned by AH=1Bh,1Ch
				db      0               ;279 - unused

                ; Pad to 059ah
                times (27ah - ($ - _internal_data)) db 0
                global  current_device
current_device  times 2 dw 0       ;27A - point to device header if filename is character device
                global  _lpCurSft
_lpCurSft       times 2 dw 0       ;27e - Current SFT
                global  _current_ldt
_current_ldt     times 2 dw 0       ;282 - Current CDS
                global  _sda_lpFcb
_sda_lpFcb      times 2 dw 0       ;286 - pointer to callers FCB
                global  _current_sft_idx
_current_sft_idx    dw      0               ;28A - SFT index for next open
                                        ; used by MS NET
				dw      0               ;28C - temp file handler
				dd      0               ;28E - pointer to JFT entry (for file being opened) in process handle table

                ; Pad to 05b2h
                times (292h - ($ - _internal_data)) db 0
                dw      __PriPathBuffer  ; 292 - "sda_WFP_START" offset in DOS DS of first filename argument
                dw      __SecPathBuffer  ; 294 - "sda_REN_WFP" offset in DOS DS of second filename argument
				dw      0ffffh           ; 296 - 0xffff or offset of last component in filename

                ; Pad to 05ceh
                times (2aeh - ($ - _internal_data)) db 0
                global  _current_filepos
_current_filepos times 2 dw 0       ;2AE - current offset in file

                ; Pad to 05eah
                times (2cah - ($ - _internal_data)) db 0
                ;global _save_BX
                ;global _save_DS
save_BX                 dw      0       ;2CA - unused by FreeDOS, for Win3.x
save_DS                 dw      0       ;      compatibility, match MS's positions
                        dw      0       ;      store DS,BX on entry to int21h (ie caller's values)
                global  _prev_user_r
                global  prev_int21regs_off
                global  prev_int21regs_seg
_prev_user_r:
prev_int21regs_off      dw      0       ;2D0 - pointer to prev int 21 frame (see offset 264h)
prev_int21regs_seg      dw      0

                ; Pad to 05fdh, 2D4 through 2E4 used for int21h/ax=6C00h
                times (2ddh - ($ - _internal_data)) db 0
                global  _ext_open_action
                global  _ext_open_attrib
                global  _ext_open_mode
_ext_open_action dw 0                   ;2DD - extended open action
_ext_open_attrib dw 0                   ;2DF - extended open attrib
_ext_open_mode   dw 0                   ;2E1 - extended open mode
open_filename    dd 0                   ;2E3 pointer to filename to open (see ax=6C00h)

                ; Pad to 0620h
                times (300h - ($ - _internal_data)) db 0

                global apistk_bottom
apistk_bottom:
                ; use bottom of error stack as scratch buffer
                ;  - only used during int 21 call
                global  _sda_tmp_dm_ren
_sda_tmp_dm_ren:times 21 db 0x90   ;300 - 21 byte srch state for rename
                global  _SearchDir_ren
_SearchDir_ren: times 32 db 0x90   ;315 - 32 byte dir entry for rename

                ; stacks are made to initialize to no-ops so that high-water
                ; testing can be performed
                times STACK_SIZE*2-($-apistk_bottom) db 0x90
				; critical error stack 331 bytes?, disk stack 384 bytes, character i/o 384 bytes
                ;300 - Error Processing Stack
                global  _error_tos
_error_tos:
                times STACK_SIZE dw 0x9090 ;480 - Disk Function Stack
                global  _disk_api_tos
_disk_api_tos:
                times STACK_SIZE dw 0x9090 ;600 - Char Function Stack
                global  _char_api_tos
_char_api_tos:
apistk_top:
                db      0               ;780 device driver look-ahead (printer), see ah=64h
_VolChange      db      0               ;781 - volume change
_VirtOpen       db      0               ;782 - virtual open flag

                ; 783h to 788h are fastseek drive, 1st cluster, logical cluster, returned logical cluster
				; 78Ah dw ?  temp location of SYSINIT

                ; controlled variables end at offset 78Ch so pad to end
                times (78ch - ($ - _internal_data)) db 0

; MSDOS 7.1+ with FAT32 SDA extended
										;78Ch - 47 bytes of ???
                times (7bbh - ($ - _internal_data)) db 0
absrdwrflg		db 		0				;7bbh - 0=int 25h/26h absolute read/write, 1=int 21h/ax=7305h
				times 12 dw 0           ;7bch to 7d2h, high word of 32bit values, see offsets 0x 2a2, 29c, 2bc, 29a, 276, 244, & registers ebx,edx,edi,ecx
				times  3 dw 0           ; ???

;
; end of controlled variables
;

segment _BSS
;!!                global  _NumFloppies
;!!_NumFloppies resw    1
;!!intr_dos_stk resw    1
;!!intr_dos_seg resw    1


; mark front and end of bss area to clear
segment IB_B
    global __ib_start
__ib_start:
segment IB_E
    global __ib_end
__ib_end:
        ;; do not clear the other init BSS variables + STACK: too late.

; kernel startup stack
                global  init_tos
                resw 512
init_tos:
; the last paragraph of conventional memory might become an MCB
                resb 16
                global __init_end
__init_end:
init_end:        

segment _DATA
; blockdev private stack
                global  blk_stk_top
                times 256 dw 0
blk_stk_top:

; clockdev private stack
                global  clk_stk_top
                times 128 dw 0
clk_stk_top:

; int2fh private stack
                global  int2f_stk_top
                times 128 dw 0
int2f_stk_top:

; Dynamic data:
; member of the DOS DATA GROUP
; and marks definitive end of all used data in kernel data segment
;

segment _DATAEND

_swap_indos:
; we don't know precisely what needs to be swapped before this, so set it here.
; this is just after FIXED_DATA+BSS+DATA and before (D)CONST+BSS
; probably, the clock and block stacks and disktransferbuffer should go past
; _swap_indos but only if int2a ah=80/81 (critical section start/end)
; are called upon entry and exit of the device drivers

                times 96 dw 0x9090 ; Process 0 Stack
                global  _p_0_tos
_p_0_tos:

segment DYN_DATA

        global _Dyn
_Dyn:
        DynAllocated dw 0
		;times DynAllocated db ? ; allocated at runtime, need to add DynAllocated to _markEndInstanceData

global _markEndInstanceData
_markEndInstanceData:  ; mark end of DOS data seg we say needs instancing

        
segment ID_B
    global __INIT_DATA_START
__INIT_DATA_START:
segment ID_E
    global __INIT_DATA_END
__INIT_DATA_END:


segment INIT_TEXT_START
                global  __InitTextStart
__InitTextStart:                    ; and c version

segment INIT_TEXT_END
                global  __InitTextEnd
__InitTextEnd:                      ; and c version

;
; start end end of HMA area

segment HMA_TEXT_START
                global __HMATextAvailable
__HMATextAvailable:
                global  __HMATextStart
__HMATextStart:   
 
; 
; the HMA area is filled with 1eh+3(=sizeof VDISK) = 33 byte dummy data,
; so nothing will ever be below 0xffff:0031
;
segment HMA_TEXT
begin_hma:              
                times 10h db 0   ; filler [ffff:0..ffff:10]
                times 20h db 0
                db 0

; to minimize relocations
                global _DGROUP_
_DGROUP_        dw DGROUP

%ifdef WATCOM
;               32 bit multiplication + division
global __U4M
__U4M:
                LMULU
global __U4D
__U4D:
                LDIVMODU
%endif

%ifdef gcc
%macro ULONG_HELPERS 1
global %1udivsi3
%1udivsi3:      call %1ldivmodu
                ret 8

global %1umodsi3
%1umodsi3:      call %1ldivmodu
                mov dx, cx
                mov ax, bx
                ret 8

%1ldivmodu:     LDIVMODU

global %1ashlsi3
%1ashlsi3:      LSHLU

global %1lshrsi3
%1lshrsi3:      LSHRU
%endmacro
                ULONG_HELPERS ___
%endif

                times 0xd0 - ($-begin_hma) db 0
                ; reserve space for far jump to cp/m routine
                times 5 db 0

;End of HMA segment                
segment HMA_TEXT_END
                global  __HMATextEnd
__HMATextEnd:                   ; and c version



; The default stack (_TEXT:0) will overwrite the data area, so I create a dummy
; stack here to ease debugging. -- ror4

segment _STACK  class(STACK) nobits stack



    

segment CONST
        ; dummy interrupt return handlers

                global _int22_handler
                global _int28_handler
                global _int2a_handler
                global _empty_handler
_int22_handler:         
_int28_handler:
_int2a_handler:
_empty_handler:
                iret
    

global _initforceEnableA20
initforceEnableA20:
                call near forceEnableA20
                retf   

    global __HMARelocationTableStart
__HMARelocationTableStart:   

                global  _int2f_handler
                extern  reloc_call_int2f_handler
_int2f_handler: jmp 0:reloc_call_int2f_handler
                call near forceEnableA20

                global  _int20_handler
                extern  reloc_call_int20_handler
_int20_handler: jmp 0:reloc_call_int20_handler
                call near forceEnableA20

                global  _int21_handler
                extern  reloc_call_int21_handler
_int21_handler: jmp 0:reloc_call_int21_handler
                call near forceEnableA20


                global  _low_int25_handler
                extern  reloc_call_low_int25_handler
_low_int25_handler: jmp 0:reloc_call_low_int25_handler
                call near forceEnableA20

                global  _low_int26_handler
                extern  reloc_call_low_int26_handler
_low_int26_handler: jmp 0:reloc_call_low_int26_handler
                call near forceEnableA20

                global  _int27_handler
                extern  reloc_call_int27_handler
_int27_handler: jmp 0:reloc_call_int27_handler
                call near forceEnableA20

                global  _int0_handler
                extern  reloc_call_int0_handler
_int0_handler:  jmp 0:reloc_call_int0_handler
                call near forceEnableA20

                global  _int6_handler
                extern  reloc_call_int6_handler
_int6_handler:  jmp 0:reloc_call_int6_handler
                call near forceEnableA20

                global  _int19_handler
                extern  reloc_call_int19_handler
_int19_handler: jmp 0:reloc_call_int19_handler
                call near forceEnableA20

                global  _cpm_entry
                extern  reloc_call_cpm_entry
_cpm_entry:     jmp 0:reloc_call_cpm_entry
                call near forceEnableA20

                global  _reloc_call_blk_driver
                extern  _blk_driver
_reloc_call_blk_driver:
                jmp 0:_blk_driver
                call near forceEnableA20

                global  _reloc_call_clk_driver
                extern  _clk_driver
_reloc_call_clk_driver:
                jmp 0:_clk_driver
                call near forceEnableA20

                global  _CharMapSrvc ; in _DATA (see AARD)
                extern  _reloc_call_CharMapSrvc
_CharMapSrvc:   jmp 0:_reloc_call_CharMapSrvc
                call near forceEnableA20

                global _init_call_p_0
                extern reloc_call_p_0
_init_call_p_0: jmp  0:reloc_call_p_0
                call near forceEnableA20


   global __HMARelocationTableEnd
__HMARelocationTableEnd:    

;
; if we were lucky, we found all entries from the outside to the kernel.
; if not, BUMS
;
;
; this routine makes the HMA area available. PERIOD.
; must conserve ALL registers
; will be only ever called, if HMA (DOS=HIGH) is enabled.
; for obvious reasons it should be located at the relocation table
;

    global _ENABLEA20
_ENABLEA20:
    mov ah,5
UsingXMSdriver:

	global _XMS_Enable_Patch
_XMS_Enable_Patch:		; SMC: patch to nop (90h) to enable use of XMS
	retf

    push bx
    call 0:0			; (immediate far address patched)
    global _XMSDriverAddress
_XMSDriverAddress: equ $ - 4	; XMS driver, if detected
    pop  bx
    retf

    global _DISABLEA20
_DISABLEA20:
    mov ah,6
    jmp short UsingXMSdriver


    global forceEnableA20
forceEnableA20:

    push ds
    push es
    push ax
	push si
	push di
	push cx
	pushf
	cld

.retry:
	xor si, si		; = 0000h
	mov ds, si		; => low memory (IVT)
	dec si			; = FFFFh
	mov es, si		; => HMA at offset 10h
	inc si			; back to 0, -> IVT entry 0 and 1
	mov di, 10h		; -> HMA, or wrapping around to 0:0
	mov cx, 4
	repe cmpsw		; compare up to 4 words
	je .enable

.success:
	popf
	pop cx
	pop di
	pop si
    pop ax
    pop es
    pop ds
    retn

.enable:
		; ok, we have to enable A20 (at least seems so)
	push cs			; make far call stack frame
	call _ENABLEA20
	jmp short .retry


; global f*cking compatibility issues:
;
; very old brain dead software (PKLITE, copyright 1990)
; forces us to execute with A20 disabled
;

	global _ExecUserDisableA20
_ExecUserDisableA20:
    push ax
	push cs			; make far call stack frame
	call _DISABLEA20	; (no-op if not in HMA, patched otherwise)
    pop ax
    iret


; Default Int 24h handler -- always returns fail
; so we have not to relocate it (now)
;
FAIL            equ     03h

                global  _int24_handler
_int24_handler: mov     al,FAIL
                iret

;
; this makes some things easier
;

segment _LOWTEXT
                global _TEXT_DGROUP
_TEXT_DGROUP dw DGROUP

segment INIT_TEXT
                global _INIT_DGROUP
_INIT_DGROUP dw DGROUP

%ifdef gcc
                ULONG_HELPERS _init_
%endif
