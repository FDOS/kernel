; this one adapted from elks, http://elks.sourceforge.net
; multiply cx:bx * dx:ax, result in dx:ax
; optimized by Arkady Belousov:
;   dx:ax * cx:bx
; = xh:xl * yh:yl
; = xh:xl*yh*w + xh:xl*yl
; = [xh*yh*w*w +] (xl*yh + xh*yl)*w + xl*yl

%macro LMULU 0

	push	cx
	push	si
	xchg	si,ax		; si=xl			(XCHG instead MOV)
	xchg	ax,dx		; ax=xh			(XCHG instead MOV)
	mul	bx		; dx:ax=xh*yl (forget dx)
	xchg	cx,ax		; cx=low(xh*yl), ax=yh
	mul	si		; dx:ax=xl*yh (forget dx)
	add	cx,ax		; cx=low(xl*yh+xh*yl)
	xchg	ax,si		; ax=xl			(XCHG instead MOV)
	mul	bx		; dx:ax=xl*yl
	add	dx,cx
	pop	si
	pop	cx
	ret

%endmacro

;  divide dx:ax / cx:bx, quotient in dx:ax, remainder in cx:bx

%macro LDIVMODU 0
; this one is adapted from an assembly gem:
; gem writer: Norbert Juffa, norbert.juffa@amd.com
; Dividing 64-bit unsigned integers Assembler / 80386
;   (adapted back to 32-bit by Bart Oldeman ;-))
;   ...bugfixed and optimized by Arkady Belousov.

; This macro divides two unsigned long numbers, the dividend and the divisor
; resulting in a quotient and a remainder.
;
; input:
;   dx:ax = dividend (x=xh:xl)
;   cx:bx = divisor  (y=yh:yl)
; output:
;   dx:ax = quotient of division of dividend by divisor (q=x/y)
;   cx:bx = remainder of division of dividend by divisor (r=x%y)
; destroys:
;   flags
;
%if XCPU < 386

	jcxz	%%div3216	; cx=0 -> divisor < 2^16

	push	si		; save temp
	 push	di		;  variables

	push	dx		; save
	 push	ax		;  dividend x
	mov	si,bx		; si=yl
	 mov	di,cx		; di:si=cx:bx=y

%%shift_loop:
	shr	dx,1		; shift both
	 rcr	ax,1		;  divisor and
	shr	cx,1		;   and dividend
	 rcr	bx,1		;    right by 1 bit (rcr preserves ZF)
	jnz	%%shift_loop	;     until zero in cx (divisor < 2^16)
	div	bx		; ax=quotient q, di:si=y

	mov	cx,ax		; cx=q
	mul	di		; dx:ax=q*yh (forget dx)
	xchg	bx,ax		; bx=low(q*yh)	(XCHG instead MOV)
	mov	ax,cx		; ax=q
	mul	si		; dx:ax=q*yl
	add	dx,bx		; dx:ax=q*y, cx=q

	pop	bx		; bx=xl
	sub	bx,ax		; bx=xl-low(q*y)
	xchg	ax,cx		; ax=q		(XCHG instead MOV)
	pop	cx		; cx=xh
	sbb	cx,dx		; cx:bx=x-q*y=remainder r, ax=q

	jae	%%div_done	; if remainder < 0
	add	bx,si
	 adc	cx,di		; correct remainder (r+=y)
	dec	ax		;  and quotient (q-=1)
%%div_done:
	xor	dx,dx		; dx:ax=0:q=q

	pop	di		; restore temp
	pop	si		;  variables
	ret

; dx:ax=x, bx=y, cx=0
; x=xh:xl=xh*w+xl=[xh/y]*y*w+xh%y*w+xl=[xh/y]*y*w+xt
; w=2^16, xh=x/w, xl=x%w, xt=xh%y*w+xl
; remainder =  x%y  =          xt%y
; quotient  = [x/y] = [xh/y]*w+xt/y

%%div3216:
	cmp	dx,bx		; xh < y ?
	jb	%%one_div	; yes, one division sufficient

	xchg	cx,ax		; ax=0, cx=xl
	xchg	ax,dx		; dx:ax=0:xh, cx=xl
	div	bx		; ax=xh/y, dx=xh%y, cx=xl
	xchg	ax,cx		; dx:ax=xh%y*w+xl=xt, cx=xh/y

%%one_div:
	div	bx		; ax=xt/y, dx=xt%d=x%d, cx=xh/y
	mov	bx,dx		; bx=x%d
	mov	dx,cx		; dx:ax=xh/y*w+xt/y=x/y
	xor	cx,cx		; cx:bx=x%d
	ret

%else	; XCPU >= 386 (Svilen Stoianov and Luchezar Georgiev, Varna, Bulgaria)

	push	eax		; save eax.high
	 pop	ax
	push	edx		; save edx.high
	 push	ax
	 pop	eax		; eax=x
	push	ecx		; save ecx.high
	 push	bx
	 pop	ecx		; ecx=y

	xor	edx,edx
	div	ecx		; eax=q, edx=r

	push	edx
	 pop	bx
	 pop	ecx		; restore ecx.high, cx:bx=r
	push	eax
	 pop	ax
	 pop	edx		; restore edx.high, dx:ax=q
	push	ax
	 pop	eax		; restore eax.high
	ret

%endif
%endmacro
