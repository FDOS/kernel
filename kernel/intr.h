/*
*/

struct REGPACK {
	unsigned r_ax, r_bx, r_cx, r_dx;
	unsigned r_bp, r_di, r_si, r_ds, r_es, r_flags;
};

extern void intr(int intrnr, struct REGPACK *rp);
extern void FAR int3();
