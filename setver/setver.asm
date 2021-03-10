;
; Initially written by Ricardo Hanke
; Released under the terms of the GNU General Public License.
; See the file 'COPYING' in the main directory for details.
;
; This driver loads the list of special programs into memory.
;

section    .text


COMMAND    equ   2
STATUS     equ   3
DRIVEREND  equ   14

FAKETABLE  equ   37h


header     dw    -1, -1
           dw    1000000000000000b
           dw    strategy
           dw    interrupt
           db    "SETVERXX"

           dw    signature

paramblock dd    0


strategy:  mov   [cs:paramblock + 0], bx
           mov   [cs:paramblock + 2], es
           retf


interrupt: push  ax
           push  bx
           push  di
           push  es
           pushf

           les   di, [paramblock]
           mov   bl, [es:di + COMMAND]
           cmp   bl, 0
           jne   error

           mov   ax, table
           add   ax, [length]

           mov   word [es:di + DRIVEREND + 0], ax
           mov   word [es:di + DRIVEREND + 2], cs

           mov   ah, 52h
           int   21h

           mov   word [es:bx + FAKETABLE + 0], table
           mov   word [es:bx + FAKETABLE + 2], cs

           xor   ax, ax
           jmp   exit

error:     mov   ax, 8003h

exit:      or    ax, 0100h
           mov   [es:di + STATUS], ax

           popf
           pop   es
           pop   di
           pop   bx
           pop   ax

           retf


signature  db    'SDIR'
length     dw    1

table      ;db    11, "SHOWVER.EXE",  5, 00
           ;db    12, "BUGGYPRG.EXE", 3, 31

           db    0
