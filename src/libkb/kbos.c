/* kbos.c -- operating system and BIOS keyboard access
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

#if 0 && defined(__EMX__)
#include <sys/emx.h>		/* must be first include file */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <kb.h>
#include "_kb.h"
#if defined(__GO32__)
#include <gppconio.h>
#undef kbhit			/* want to be able to call kbhit from libc */
int kbhit(void);		/* djgpp v2 */
#endif

#if defined(KB_LOCK_ALL_START)
KB_LOCK_ALL_START(_libkb_kbos)
#endif
/* this file contains lots of #if ... */
/***********************************************************************
// utility functions
************************************************************************/
#if defined(KB_INT86_REGS)
__inline__ void _kb_int86_regs_init(KB_INT86_REGS * regs) {
	memset(regs, 0, sizeof(KB_INT86_REGS));
#if defined(__WATCOMC__)
	regs->w.ds = FP_SEG(regs);
#if defined(__386__) && !defined(__WINDOWS_386__)
	regs->w.fs = regs->w.gs = FP_SEG(regs);
#endif
#endif
}

__inline__ void _kb_int86_regs_init_ax(KB_INT86_REGS * regs, unsigned short ax) {
	_kb_int86_regs_init(regs);
#if defined(__WATCOMC__)
	regs->w.ax = ax;
#else
	regs->x.ax = ax;
#endif
}

#endif				/* KB_INT86_REGS */

/* check if we are running in a Windows DOS box */
int _kb_iswin(void) {
#if !defined(KB_INT86_REGS)
	return 0;
#elif defined(__GO32__) && !defined(__DJGPP__)
	/* go32 doesn't support INT 0x2f, AX=0x1600 */
	return -1;
#else
	KB_INT86_REGS regs;
#if defined(__EMX__)
	if (_osmode != DOS_MODE)
		return 0;
	if ((_emx_env & 0x1000) == 0)	/* if RSX not found */
		return 0;
	if (!KB_USE_INT86())	/* _int86() not allowed */
		return -1;
#endif
	_kb_int86_regs_init_ax(&regs, 0x1600);
	KB_INT86(0x2f, &regs);
	if (regs.h.al == 0 || regs.h.al == 1 || regs.h.al == 0x80 || regs.h.al == 0xff)
		return 0;
	/* return major-version in high byte, minor-version in low byte */
	return (regs.h.al << 8) | regs.h.ah;
#endif				/* KB_INT86_REGS */
}

/* sleep for a very short time */
__inline__ void _kb_usleep(unsigned long usec) {
	if (usec < 1024)
		return;
#if defined(__KB_LINUX) || defined(__DJGPP__)
	usleep(usec);
#elif defined(__EMX__)
	_sleep2(usec / 1000);
#elif defined(_MSC_VER) && defined(__KB_MSDOS16)
	/* FIXME */
#elif defined(__KB_MSDOS)
	delay((int)(usec / 1000));
#endif
}

/***********************************************************************
// BIOS wrapper
// some documentation can be found in djgpp2/src/libc/bios/
************************************************************************/

#if defined(__EMX__) || defined(__GO32__) || defined(__WATCOMC__)

static __inline__ int _my_bioskey(unsigned cmd) {
	KB_INT86_REGS regs;

#if defined(__EMX__)
	/* ((_emx_env & 0x0800) == 0) || "emx: -ac not enabled for _int86()" */
	if (!KB_USE_INT86())
		return 0;
#endif				/* __EMX__ */

	_kb_int86_regs_init(&regs);
	regs.h.ah = cmd;
	KB_INT86(0x16, &regs);

#if defined(__WATCOMC__)
#define x w			/* regs.x.AA -> regs.w.AA */
#endif

	switch (cmd) {
	case KB_KEYBRD_READY:
	case KB_NKEYBRD_READY:
		if (regs.x.flags & 0x40)	/* zero flag set -> no key is waiting */
			return 0;
		else if (regs.x.ax == 0)
			return -1;	/* Control-Break */
		else
			return regs.x.ax;
	case KB_KEYBRD_SHIFTSTATUS:
		return regs.h.al;
	case KB_KEYBRD_READ:
	case KB_NKEYBRD_READ:
	case KB_NKEYBRD_SHIFTSTATUS:
		return regs.x.ax;
	}
	return 0;

#if defined(__WATCOMC__)
#undef x
#endif
}

#define _KB_BIOSKEY(x)	_my_bioskey(x)

#endif

#if defined(__KB_MSDOS)
#if defined(__BORLANDC__) || defined(__TURBOC__) || defined(__GO32__)
#ifndef _KB_BIOSKEY
#define _KB_BIOSKEY(x)	bioskey(x)
#endif
#endif
#if defined(_MSC_VER) || defined(__WATCOMC__)
#ifndef _KB_BIOSKEY
#define _KB_BIOSKEY(x)	_bios_keybrd(x)
#endif
#endif
#endif

/***********************************************************************
// BIOS level - lowest and (hopefully) fastest access level
// If not applicable, OS level is used
************************************************************************/

__inline__ int kb_bioskey(unsigned cmd) {
#if defined(_KB_BIOSKEY)
	return _KB_BIOSKEY(cmd);
#else
	return 0;
#endif
}

int kb_bios_kbhit(void) {
	if (_kb_mode)
		return kb_kbhit();

#if !defined(_KB_BIOSKEY)
	return kb_os_kbhit();
#elif defined(__GO32__)
	return kbhit();		/* this uses BIOS and is damned fast */
#else
#if defined(__EMX__)
	if (!KB_USE_INT86())
		return kb_os_kbhit();	/* _int86() not allowed */
#endif
	return (_KB_BIOSKEY(KB_NKEYBRD_READY) != 0);
#endif
}

unsigned kb_bios_getkey(void) {
	if (_kb_mode)
		return kb_getkey();

	if (!kb_bios_kbhit())
		return 0;

#if !defined(_KB_BIOSKEY)
	return kb_os_getkey();
#elif defined(__GO32__)
	return getxkey();	/* this uses BIOS */
#else
#if defined(__EMX__)
	if (!KB_USE_INT86())
		return kb_os_getkey();	/* _int86() not allowed */
#endif
	return kb_bios_keycode(_KB_BIOSKEY(KB_NKEYBRD_READ));
#endif
}

/***********************************************************************
// emx getch() wrapper
// returns 0 if no key is available
************************************************************************/

#if defined(__EMX__)

static int _last_key = -1;

static __inline__ int _my_read_kbd(int eat) {
	if (_last_key == -1)
		_last_key = _read_kbd(0, 0, 0);	/* echo, wait, sig */
	if (_last_key == -1)
		return 0;	/* no key available */
	if (eat) {
		int k = _last_key;
		_last_key = -1;
		return k;
	} else
		return 1;
}

#define _my_kbhit()		_my_read_kbd(0)
#define _my_getch()		_my_read_kbd(1)

#endif				/* __EMX__ */

/***********************************************************************
// Linux getch() wrapper
// returns 0 if no key is available
//
// adapted from code written by Tommy Frandsen and Harm Hanemaayer
// see svgalib 1.2.x: src/vgamisc.c
//
// This code is slow and doesn't work very well because stdin
// is only temporary changed for each attempt to read a key.
// It works better if you use kb_os_waitkey().
************************************************************************/

#if defined(__KB_LINUX)

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

/* use a buffer for keys, this works better */
static unsigned char _key_buffer[128];
static const int _key_size = HIGH(_key_buffer);
static int _key_index = 0;

/* read keyboard file, fill buffer */
static __inline__ void _my_read_kbd(int fd) {
	if (_key_index < _key_size) {
		int n = read(fd, &_key_buffer[_key_index], _key_size - _key_index);
		if (n > 0)
			_key_index += n;
	}
}

/* wait for a key, fill buffer */
static __inline__ void _my_read_kbd_and_wait(int fd, unsigned long usec) {
	struct timeval tv, *tvp;
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	if (usec == (unsigned long)-1)
		tvp = NULL;	/* wait forever */
	else {
		tv.tv_sec = usec / 1000000;
		tv.tv_usec = usec % 1000000;
		tvp = &tv;
	}
	if (select(fd + 1, &fds, NULL, NULL, tvp) > 0)
		_my_read_kbd(fd);
}

/* read keyboard, fill buffer */
static __inline__ void _my_read_a_key(unsigned long wait) {
	struct termio zap, original;
	int fd = fileno(stdin);

	if (ioctl(fd, TCGETA, &original) != 0)	/* Get termio */
		return;
	zap = original;
	zap.c_cc[VMIN] = 0;	/* Modify termio  */
	zap.c_cc[VTIME] = 0;
	zap.c_lflag = 0;
	if (ioctl(fd, TCSETA, &zap) != 0)	/* Set new termio */
		return;
	if (wait)
		_my_read_kbd_and_wait(fd, wait);
	else
		_my_read_kbd(fd);
	ioctl(fd, TCSETA, &original);	/* Restore termio */
}

/* get a key from the keyboard buffer or read from file */
static unsigned _my_get_key(int eat, unsigned long wait) {
	unsigned k;

	if (_key_index <= 0) {
		_my_read_a_key(wait);
		if (_key_index <= 0)
			return 0;	/* no key available */
	}
	k = _key_buffer[0];
	if (eat) {
		_key_index--;
		if (_key_index > 0)
			memmove(&_key_buffer[0], &_key_buffer[1], _key_index);
	}
	return k;
}

/* this is the 'public' entry point */
static unsigned _linux_getkey(int eat, unsigned long wait) {
	unsigned k = _my_get_key(eat, wait);

#if 0
	/* TODO: convert Esc sequences to our portable keycode.
	 *       how can this be done in a way that works on all terminals ?
	 */
	if (eat && k == 27) {
		unsigned k1 = _my_get_key(eat, wait);
		unsigned k2 = _my_get_key(eat, wait);

		if (k2 == 68)	/* cursor left */
			k = 0x24b;
		else if (k2 == 67)	/* cursor right */
			k = 0x24d;
		else if (k2 == 65)	/* cursor up */
			k = 0x248;
		else if (k2 == 66)	/* cursor down */
			k = 0x250;
		else
			k = 0x200 | k2;
	}
#endif

	return k;
}

#define _my_kbhit()		_linux_getkey(0,0)
#define _my_getch()		_linux_getkey(1,0)

unsigned kb_os_waitkey(void) {
	return _linux_getkey(1, (unsigned long)-1);
}

#endif				/* __KB_LINUX */

/***********************************************************************
// OS level
************************************************************************/

int kb_os_kbhit(void) {
	if (_kb_mode)
		return kb_kbhit();

#if defined(__EMX__)
	/* use BIOS if possible, cause it's much faster */
	if (KB_USE_INT86())
		return kb_bios_kbhit();
	return _my_kbhit() != 0;
#elif defined(__KB_LINUX)
	return _my_kbhit() != 0;
#else
	return kbhit() != 0;
#endif
}

/* do NOT use BIOS, but be compatible */

#if !defined(_my_getch)
#define _my_getch()	getch()
#endif

unsigned kb_os_getkey(void) {
	unsigned k;

	if (_kb_mode)
		return kb_getkey();

	if (!kb_os_kbhit())
		return 0;

	k = _my_getch();
#if defined(__KB_MSDOS)
	if (k != 0)
		return kb_os_keycode(k, 0);
	k = _my_getch();	/* read again */
	return kb_os_keycode(0, k);
#elif defined(__KB_LINUX)
	return k;
#endif
}

/***********************************************************************
// emx+gcc lacks a real kbhit() and getch()
************************************************************************/

#if defined(__EMX__)

int kbhit(void) {
	return kb_os_kbhit();
}

int getch(void) {
	if (_kb_mode) {
		int k = kb_getkey();
		return k == 0 ? -1 : k;
	}
	return _my_getch();
}

#endif				/* __EMX__ */

/***********************************************************************
// wait for a key
************************************************************************/

#if defined(__KB_MSDOS)

unsigned kb_os_waitkey(void) {
	while (!kb_os_kbhit())
		_kb_usleep(5 * 1024u);
	return kb_os_getkey();
}

#endif

/***********************************************************************
// locking
************************************************************************/

#if defined(KB_LOCK_ALL_END)
KB_LOCK_ALL_END(_libkb_kbos)
#endif
int _libkb_kbos_lock(void) {
	int x = 0;

#if defined(KB_LOCK_ALL)
	KB_LOCK_ALL(_libkb_kbos, x);
#endif

	return x;
}

/*
vi:ts=4
*/
