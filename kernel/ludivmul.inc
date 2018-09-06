;  this one adapted from elks, http://elks.sourceforge.net
;  multiply cx:bx * dx:ax, result in dx:ax        

%macro LMULU 0

        push    si
        push    cx
	mov	si, ax   ; save _ax in si
        mov     ax, bx   ; cx:ax = _cx:_bx
	mul	dx       ; dx:ax = _bx*_dx (forget dx)
        xchg    cx, ax   ; cx = low(_dx*_bx)
	mul	si       ; dx:ax = _cx*_ax (forget dx)
	add	cx, ax   ; cx = low(_cx*_ax + _dx*_bx)
	mov	ax, si   ; restore _ax
	mul	bx       ; dx:ax = _bx*_ax
        add     dx, cx   ; dx = high(_bx*_ax)+low(_cx*_ax + _dx*_bx)
        pop     cx
        pop     si
	ret

%endmacro

;  divide dx:ax / cx:bx, quotient in dx:ax, remainder in cx:bx

%macro LDIVMODU 0
; this one is adapted from an assembly gem:
; gem writer: Norbert Juffa, norbert.juffa@amd.com

; Dividing 64-bit unsigned integers Assembler / 80386

;   Here is a division routine for dividing two 64-bit unsigned integers. 
;   I derived it by modifying some old
;   16-bit code for dividing 32-bit integers that I did several years ago for a 
;   Turbo-Pascal replacement library.
;   If a 64-bit signed integer division is needed, appropriate shell code for 
;   this routine can easily be written.
;
;   (adapted back to 32-bit by Bart Oldeman ;-))
;
; __U4D divides two unsigned long numbers, the dividend and the divisor
; resulting in a quotient and a remainder.
;
; input:
;   dx:ax = dividend
;   cx:bx = divisor
;
; output:
;   dx:ax = quotient of division of dividend by divisor
;   cx:bx = remainder of division of dividend by divisor
;
; destroys:
;   flags
;
%ifdef STDCALL
	push bp
	mov bp, sp
	mov ax, [bp+6]
	mov dx, [bp+8]
	mov bx, [bp+10]
	mov cx, [bp+12]
	pop bp
%endif

	test    cx, cx             ; divisor > 2^16-1 ?
	jnz     %%big_divisor      ; yes, divisor > 2^16-1
	cmp     dx, bx             ; only one division needed ? (cx = 0)
	jb      %%one_div          ; yes, one division sufficient


	xchg    cx, ax             ; save dividend-lo in cx, ax=0
	xchg    ax, dx             ; get dividend-hi in ax, dx=0
	div     bx                 ; quotient-hi in ax
	xchg    ax, cx             ; cx = quotient-hi, ax =dividend-lo

%%one_div:    
	div     bx                 ; ax = quotient-lo
	mov     bx, dx             ; bx = remainder-lo
	mov     dx, cx             ; dx = quotient-hi(quotient in dx:ax)
	xor     cx, cx             ; cx = remainder-hi (rem. in cx:bx)
	ret

%%big_divisor:
	push    si                 ; save temp
	push    di                 ;  variables
	push    dx                 ; save
	push    ax                 ;  dividend
	mov     si, bx             ; divisor now in
	mov     di, cx             ;  di:si and cx:bx
%%shift_loop:
	shr     dx, 1              ; shift both
	rcr     ax, 1              ;  dividend
	shr     cx, 1              ;   and divisor
	rcr     bx, 1              ;    right by 1 bit
        jnz     %%shift_loop       ;     loop if di non-zero (rcr does not touch ZF)
	div     bx                 ; compute quotient dx:ax>>x / cx:bx>>x (stored in ax; remainder in dx not used)
	pop     bx                 ; get dividend lo-word
	mov     cx, ax             ; save quotient
        mul     di                 ; quotient * divisor hi-word (low only)
	pop     dx                 ; dividend high
	sub     dx,ax              ; dividend high - divisor high * quotient, no overflow (carry/borrow) possible here
	push    dx                 ; save dividend high
        mov     ax, cx             ; ax=quotient
	mul     si                 ; quotient * divisor lo-word
	sub     bx, ax             ; dividend-lo - (quot.*divisor-lo)-lo
	mov     ax, cx             ; get quotient
	pop     cx                 ; restore dividend hi-word
	sbb     cx, dx             ; subtract (divisor-lo * quot.)-hi from dividend-hi
	sbb     dx, dx             ; 0 if remainder > 0, else FFFFFFFFh
	and     si, dx             ; nothing to add
	and     di, dx             ;  back if remainder positive di:si := di:si(cx:bx) & dx:dx
	add     bx, si             ; correct remainder           cx:bx += di:si
	adc     cx, di             ;  and
	add     ax, dx             ;   quotient if necessary           ax += dx
	xor     dx, dx             ; clear hi-word of quot (ax<=FFFFh) dx := 0
	pop     di                 ; restore temp  
	pop     si                 ;  variables
	ret

%endmacro

%macro LSHLU 0
	pop bx
	popargs {dx,ax},cx
	push bx
	jcxz %%ret
%%loop:	shl ax, 1
	rcl dx, 1
	loop %%loop
%%ret:	ret
%endmacro

%macro LSHRU 0
	pop bx
	popargs {dx,ax},cx
	push bx
	jcxz %%ret
%%loop:	shr dx, 1
	rcr ax, 1
	loop %%loop
%%ret:	ret
%endmacro
