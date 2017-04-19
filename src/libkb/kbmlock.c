/* kbmlock.c -- memory locking
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

/* memory locking is currently implemented for djgpp v2 and Watcom C32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kb.h>
#include <kbmlock.h>
#include "_kb.h"

/* see also: DPMI specification
 *           djgpp2/src/libc/go32/gopint.c
 *           djgpp2/src/libc/dpmi/api/d0600.s
 */

#if defined(KB_LOCK_ALL_START)
KB_LOCK_ALL_START(_libkb_kbmlock)
#endif
typedef struct {
	int selector;
	unsigned long address;
	long size;
	unsigned long linear_address;
} kb_lockaddr_t;

/***********************************************************************
// DPMI support for Watcom C32 (djgpp v2 compatible)
************************************************************************/

#if defined(__WATCOMC__) && defined(__KB_MSDOS32)

typedef struct {
	unsigned long handle;	/* 0, 2 */
	unsigned long size;	/* or count *//* 4, 6 */
	unsigned long address;	/* 8, 10 */
} __dpmi_meminfo;

/* it looks like every base address is 0 under DOS/4GW and PMODE/W */

/* DPMI 0.9 AX=0006 */
static
int __dpmi_get_segment_base_address(int _selector, unsigned long *_addr) {
	KB_INT86_REGS regs;
	_kb_int86_regs_init_ax(&regs, 0x0006);

	regs.w.bx = (unsigned short)_selector;
	regs.w.flags = 0x01;	/* be paranoid */
	KB_INT86(0x31, &regs);
	if (regs.w.flags & 0x01)	/* error if carry flag set */
		return -1;
	*_addr = ((unsigned long)regs.w.cx << 16) | regs.w.dx;
	return 0;
}

static
int watcom_c32_do_lock(__dpmi_meminfo * _info, unsigned short ax) {
	KB_INT86_REGS regs;
	_kb_int86_regs_init_ax(&regs, ax);

	regs.w.bx = (unsigned short)(_info->address >> 16);
	regs.w.cx = (unsigned short)(_info->address);
	regs.w.si = (unsigned short)(_info->size >> 16);
	regs.w.di = (unsigned short)(_info->size);

	regs.w.flags = 0x01;	/* be paranoid */
	KB_INT86(0x31, &regs);
	return (regs.w.flags & 0x01 ? -1 : 0);	/* error if carry flag set */
}

/* DPMI 0.9 AX=0600 */
static
int __dpmi_lock_linear_region(__dpmi_meminfo * _info) {
	return watcom_c32_do_lock(_info, 0x0600);
}

/* DPMI 0.9 AX=0601 */
static
int __dpmi_unlock_linear_region(__dpmi_meminfo * _info) {
	return watcom_c32_do_lock(_info, 0x0601);
}

#endif				/* __WATCOMC__ */

/***********************************************************************
// low level locking - djgpp v2, Watcom C32
************************************************************************/

#if defined(__KB_MSDOS32)
#if defined(__DJGPP__) || defined(__WATCOMC__)

#define KB_HAVE_LOCK

static int _kb_lock_lockaddr(const kb_lockaddr_t * l, int do_lock) {
	int r;
	__dpmi_meminfo memregion;

	memregion.handle = 0;
	memregion.address = l->linear_address;
	memregion.size = l->size;

	if (l->size <= 0)
		r = -1;
	else if (do_lock)
		r = __dpmi_lock_linear_region(&memregion);
	else
		r = __dpmi_unlock_linear_region(&memregion);

#if defined(KB_DEBUG) && (KB_DEBUG >= 3)
	fprintf(stderr, "libkb lockinfo: %-6s %04x:%08lx (0x%08lx), "
		"%6ld bytes: %d\n", do_lock ? "lock" : "unlock",
		l->selector, l->address, l->linear_address, l->size, r);
#endif

	return r;
}

static
int _kb_init_lockaddr(kb_lockaddr_t * l, int seg, unsigned long lockaddr, long locksize, int code) {
	int r;

	l->selector = seg;
#if defined(__DJGPP__)
	if (l->selector == -1)
		l->selector = code ? _go32_my_cs() : _go32_my_ds();
#endif
	l->address = lockaddr;
	l->size = locksize;
	l->linear_address = 0;

	r = __dpmi_get_segment_base_address(l->selector, &l->linear_address);
	if (r == 0)
		l->linear_address += l->address;
	else
		l->linear_address = 0;

#if defined(KB_DEBUG) && (KB_DEBUG >= 4)
	fprintf(stderr, "libkb lockinfo: %-6s %04x:%08lx (0x%08lx), "
		"%6ld bytes\n", "", l->selector, l->address, l->linear_address, l->size);
#endif

	if (r != 0)
		l->size = -1;
	return l->size > 0 ? 0 : -1;
}

#if defined(__DJGPP__)
#define KB_INIT_LOCKADDR(l,addr,size,code) \
			_kb_init_lockaddr(l,-1,(unsigned long)(addr),size,code)
#elif defined(__WATCOMC__)
#define KB_INIT_LOCKADDR(l,addr,size,code) \
			_kb_init_lockaddr(l,FP_SEG(addr),FP_OFF(addr),size,code)
#endif

#endif
#endif				/* __KB_MSDOS32 */

/***********************************************************************
// init a kb_lockaddr_t
************************************************************************/

static
int _kb_init_lock_code(kb_lockaddr_t * l, void (*start) (void), void (*end) (void)) {
	l->size = -1;
	if (start == NULL || end == NULL || start == end)
		return -1;

#if defined(KB_INIT_LOCKADDR)
	/* warning: ANSI C forbids ordered comparisons of pointers to functions */
	{
		unsigned long s = (unsigned long)start;
		unsigned long e = (unsigned long)end;
		if (e > s)
			return KB_INIT_LOCKADDR(l, start, e - s, 1);
		else if (s > e)
			return KB_INIT_LOCKADDR(l, end, s - e, 1);
		else
			return -1;
	}
#else
	l->size = 0;
	return 0;
#endif
}

static
int _kb_init_lock_data(kb_lockaddr_t * l, const void *start, const void *end) {
	l->size = -1;
	if (start == NULL || end == NULL || start == end)
		return -1;

#if defined(KB_INIT_LOCKADDR)
	if (end > start)
		return KB_INIT_LOCKADDR(l, start, (const char *)end - (const char *)start, 0);
	else if (start > end)
		return KB_INIT_LOCKADDR(l, end, (const char *)start - (const char *)end, 0);
	else
		return -1;
#else
	l->size = 0;
	return 0;
#endif
}

/***********************************************************************
// high level locking
************************************************************************/

int kb_lock_code(void (*start) (void), void (*end) (void)) {
	kb_lockaddr_t l;

	if (_kb_init_lock_code(&l, start, end) != 0)
		return -1;
#if defined(KB_HAVE_LOCK)
	return _kb_lock_lockaddr(&l, 1);
#else
	return 0;
#endif
}

int kb_lock_data(const void *start, const void *end) {
	kb_lockaddr_t l;

	if (_kb_init_lock_data(&l, start, end) != 0)
		return -1;
#if defined(KB_HAVE_LOCK)
	return _kb_lock_lockaddr(&l, 1);
#else
	return 0;
#endif
}

int kb_lock_var(const void *addr, unsigned size) {
	return kb_lock_data(addr, (const char *)addr + size);
}

/***********************************************************************
// high level unlocking
************************************************************************/

int kb_unlock_code(void (*start) (void), void (*end) (void)) {
	kb_lockaddr_t l;

	if (_kb_init_lock_code(&l, start, end) != 0)
		return -1;
#if defined(KB_HAVE_LOCK)
	return _kb_lock_lockaddr(&l, 0);
#else
	return 0;
#endif
}

int kb_unlock_data(const void *start, const void *end) {
	kb_lockaddr_t l;

	if (_kb_init_lock_data(&l, start, end) != 0)
		return -1;
#if defined(KB_HAVE_LOCK)
	return _kb_lock_lockaddr(&l, 0);
#else
	return 0;
#endif
}

int kb_unlock_var(const void *addr, unsigned size) {
	return kb_unlock_data(addr, (const char *)addr + size);
}

/***********************************************************************
// merge overlapping regions
//
// We try to lock as few different regions as possible.
************************************************************************/

#if defined(KB_HAVE_LOCK)

static int _kb_merge2(kb_lockaddr_t * l1, kb_lockaddr_t * l2) {
	unsigned long end1, end2;

	if (l1->size <= 0 || l2->size <= 0)
		return 0;

	end1 = l1->linear_address + l1->size;
	end2 = l2->linear_address + l2->size;

	/* try to merge l2 into l1 */
	if (l1->linear_address <= l2->linear_address && end1 + 1 >= l2->linear_address) {
		if (end2 > end1)
			l1->size = end2 - l1->linear_address;
		l2->size = 0;
		return 1;
	}

	/* try to merge l1 into l2 */
	if (l2->linear_address <= l1->linear_address && end2 + 1 >= l1->linear_address) {
		if (end1 > end2)
			l2->size = end1 - l2->linear_address;
		l1->size = 0;
		return 1;
	}

	return 0;
}

static
int _kb_merge_lockaddr(kb_lockaddr_t l[], int n) {
	int i, j;
	int found;

	/* a simple 'bubblemerge' (like bubblesort) */

	do {
		found = 0;

		for (i = 0; i < n - 1; i++)
			for (j = i + 1; j < n; j++)
				if (_kb_merge2(&l[i], &l[j]))
					found = 1;

	} while (found);

	return 0;
}

#if defined(KB_DEBUG) && (KB_DEBUG >= 4)
static
void _kb_print_lockaddr(const kb_lockaddr_t ll[], int n, const char *s) {
	int i;
	const kb_lockaddr_t *l = ll;

	for (i = 0; i < n; i++, l++)
		if (l->size > 0)
			fprintf(stderr, "libkb lockinfo: %-6s %04x:%08lx (0x%08lx), "
				"%6ld bytes\n", s, l->selector, l->address, l->linear_address, l->size);
}
#endif

#endif				/* KB_HAVE_LOCK */

/***********************************************************************
// lock a module, merge overlapping regions
************************************************************************/

#if defined(KB_HAVE_LOCK)

static
int __kb_lock_module(int do_lock, int cn, int dn, void (*code[])(void), const void *data[], kb_lockaddr_t l[]) {
	int x = 0;
	int i, n = 0;

	for (i = 0; i < cn + dn; i++)
		l[i].size = -1;

	for (i = 0; i < 2 * cn; i += 2)
		if (code[i] != NULL || code[i + 1] != NULL)
			x |= _kb_init_lock_code(&l[n++], code[i], code[i + 1]);

	for (i = 0; i < 2 * dn; i += 2)
		if (data[i] != NULL || data[i + 1] != NULL)
			x |= _kb_init_lock_data(&l[n++], data[i], data[i + 1]);

	if (x != 0)
		return -1;

	for (i = 0; i < n; i++)
		if (l[i].size < 0)
			return -1;

#if defined(KB_DEBUG) && (KB_DEBUG >= 4)
	_kb_print_lockaddr(l, n, "1");
#endif
	_kb_merge_lockaddr(l, n);
#if defined(KB_DEBUG) && (KB_DEBUG >= 4)
	_kb_print_lockaddr(l, n, "2");
#endif

	for (i = 0; i < n; i++)
		if (l[i].size > 0)
			x |= _kb_lock_lockaddr(&l[i], do_lock);

	return x == 0 ? 0 : -1;
}

#endif				/* KB_HAVE_LOCK */

#if defined(__BORLANDC__) && !defined(KB_HAVE_LOCK)
#pragma argsused
#endif
int _kb_lock_module(int do_lock,
		    void (*cs1) (void), void (*ce1) (void),
		    void (*cs2) (void), void (*ce2) (void),
		    const void *ds1, const void *de1,
		    const void *ds2, const void *de2,
		    const void *ds3, const void *de3,
		    const void *ds4, const void *de4,
		    const void *ds5, const void *de5,
		    const void *ds6, const void *de6,
		    const void *ds7, const void *de7, const void *ds8, const void *de8) {
#if defined(KB_HAVE_LOCK)
	void (*code[2 * 2]) (void);
	const void *data[8 * 2];
	kb_lockaddr_t l[2 + 8];
	int i;

	i = 0;
	code[i++] = cs1;
	code[i++] = ce1;
	code[i++] = cs2;
	code[i++] = ce2;

	i = 0;
	data[i++] = ds1;
	data[i++] = de1;
	data[i++] = ds2;
	data[i++] = de2;
	data[i++] = ds3;
	data[i++] = de3;
	data[i++] = ds4;
	data[i++] = de4;
	data[i++] = ds5;
	data[i++] = de5;
	data[i++] = ds6;
	data[i++] = de6;
	data[i++] = ds7;
	data[i++] = de7;
	data[i++] = ds8;
	data[i++] = de8;

	return __kb_lock_module(do_lock, 2, 8, code, data, l);
#else
	return 0;
#endif
}

/***********************************************************************
// locking (lock the locking code :-)
************************************************************************/

#if defined(KB_LOCK_ALL_END)
KB_LOCK_ALL_END(_libkb_kbmlock)
#endif
int _libkb_kbmlock_lock(void) {
	int x = 0;

#if defined(KB_LOCK_ALL)
	KB_LOCK_ALL(_libkb_kbmlock, x);
#endif

	return x;
}

/*
vi:ts=4
*/
