;  these are adapted from elks, http://elks.sourceforge.net
;  multiply cx:bx * dx:ax, result in dx:ax        

%macro LMULU 0

        push    si
        push    cx
	mov	si,ax   ; save _ax in si
        mov     ax,bx   ; cx:ax = _cx:_bx
	mul	dx      ; dx:ax = _bx*_dx (forget dx)
        xchg    cx,ax   ; cx = low(_dx*_bx)
	mul	si      ; dx:ax = _cx*_ax (forget dx)
	add	cx,ax   ; cx = low(_cx*_ax + _dx*_bx)
	mov	ax,si   ; restore _ax
	mul	bx      ; dx:ax = _bx*_ax
        add     dx,cx   ; dx = high(_bx*_ax)+low(_cx*_ax + _dx*_bx)
        pop     cx
        pop     si
	ret

%endmacro

;  divide dx:ax / cx:bx, quotient in dx:ax, remainder in cx:bx

%macro LDIVMODU 0
; ldivmod.s - 32 over 32 to 32 bit division and remainder for 8086

; ludivmod( dividend bx:ax, divisor di:cx ) [ unsigned quot di:cx, rem bx:ax ]

; dx is not preserved


; NB negatives are handled correctly, unlike by the processor
; divison by zero does not trap


; let dividend = a, divisor = b, quotient = q, remainder = r
;	a = b * q + r  mod 2^32
; where:

; if b = 0, q = 0 and r = a

; otherwise, q and r are uniquely determined by the requirements:
; r has the same sign as b and absolute value smaller than that of b, i.e.
;	0 <= r < b

        push    di
        mov     di,cx
        mov     cx,bx
        mov     bx,dx

	push	bp
	push	si
	mov	bp,sp
	push	di		; remember b
	push	cx

%assign b0	-4
%assign b16	-2

	test	di,di
	jne	%%divlarge
	test	cx,cx
	je	%%return          ; divzero: return q = 0 and r = a
	cmp	bx,cx
	jae	%%divlarge	; would overflow
	mov	dx,bx		; a in dx:ax
	div	cx
	xchg	cx,ax		; q in di:cx, junk in ax
	xchg	ax,dx		; r in ax, junk in dx
	mov	bx,di		; r in bx:ax
	jmp	%%return

%%divlarge:
	mov	si,di		; w in si:dx, initially b from di:cx
	mov	dx,cx
	xor	cx,cx		; q in di:cx, initially 0
	mov	di,cx

; r in bx:ax, initially a
; use di:cx rather than dx:cx in order to
; have dx free for a byte pair later

	cmp	si,bx
	jb	%%loop1
	ja	%%return		; finished if b > r
	cmp	dx,ax
	ja	%%return

; rotate w (= b) to greatest dyadic multiple of b <= r

%%loop1:
	shl	dx,1		; w = 2*w
	rcl	si,1
	jc	%%loop1_exit	; w was > r counting overflow (unsigned)
	cmp	si,bx		; while w <= r (unsigned)
	jb	%%loop1
	ja	%%loop1_exit
	cmp	dx,ax
	jbe	%%loop1		; else exit with carry clear for rcr

%%loop1_exit:
	rcr	si,1
	rcr	dx,1

%%loop2:
	shl	cx,1		; q = 2*q
	rcl	di,1
	cmp	si,bx		; if w <= r
	jb	%%loop2_over
	ja	%%loop2_test
	cmp	dx,ax
	ja	%%loop2_test

%%loop2_over:
	add	cx,1		; q++
	adc	di,0
	sub	ax,dx		; r = r-w
	sbb	bx,si

%%loop2_test:
	shr	si,1		; w = w/2
	rcr	dx,1
	cmp	si,[bp+b16]	; while w >= b
	ja	%%loop2
	jb	%%return
	cmp	dx,[bp+b0]
	jae	%%loop2

%%return:
	mov	sp,bp
	pop	si
	pop	bp

        mov     dx,bx
        mov     bx,ax
        mov     ax,cx
        mov     cx,dx
        mov     dx,di
        pop     di

	ret

%endmacro