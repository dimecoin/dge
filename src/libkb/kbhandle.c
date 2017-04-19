/* kbhandle.c -- MSDOS keyboard interrupt handler / Linux keyboard handler
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

#include <kb.h>
#include "_kb.h"

#if defined(KB_LOCK_ALL_START)
KB_LOCK_ALL_START(_libkb_kbhandle)
#endif
#if defined(__KB_MSDOS)
#define KB_ACKINT()		KB_OUTP8(0x20, 0x20)	/* ack. interrupt */
#else
#define KB_ACKINT()		/* no interrupt */
#endif
/***********************************************************************
// globals
************************************************************************/
volatile unsigned char _kb_key[128];	/* the key flags */
volatile unsigned short _kb_shift = 0;	/* shift state */
volatile int _kb_keys_pressed = 0;	/* number of keys pressed */
volatile unsigned short _kb_last_key = 0;	/* last key pressed */

int _kb_mode = 0;		/* do we have a keyboard handler? */
unsigned long _kb_flags = 0;	/* flags from kb_install() */

kb_handler_callback_t kb_handler_callback = NULL;
kb_handler_callback_t kb_handler_mediumraw_callback = NULL;

/***********************************************************************
// static variables
// These are not volatile because all access outside the
//   interrupt handler uses KB_DISABLE().
************************************************************************/

static unsigned char _t_prefix = 0;
static char _emergency_done = 0;	/* don't recurse here */

#define KEY_BUFFER_SIZE 256	/* store up to 256 keys for kb_keypress() */

static unsigned char _key_buffer[2 * KEY_BUFFER_SIZE];
#define _key_buffer_start	(&_key_buffer[0])
#define _key_buffer_end		(&_key_buffer[HIGH(_key_buffer)])

static unsigned char *_key_buffer_head = _key_buffer_start;
static unsigned char *_key_buffer_tail = _key_buffer_start;

/* debugging and error reporting */

#if defined(KB_DEBUG)
static unsigned char _kb_port_buffer[128];
#define _kb_port_buffer_start	(&_kb_port_buffer[0])
#define _kb_port_buffer_end		(&_kb_port_buffer[sizeof(_kb_port_buffer)])
static unsigned char *_kb_port_buffer_head = _kb_port_buffer_start;
#endif

/* I've just (07 Jan 1996) taken a look at the Linux kernel source
 * and come to the conclusion that reporting all unknown scan-codes
 * as a bug is probably counterproductive.
 * It seems that there exist lots of keyboards that produce lots
 * of codes we are simply not interested in.
 * I have removed all error reporting, the port values give us
 * enough information if we want to add something new.
 */

/***********************************************************************
// reset all vars except _kb_flags
************************************************************************/

static void _kb_clear(void) {
	KB_DISABLE();
	memset((void *)_kb_key, 0, sizeof(_kb_key));	/* cast away volatile */
	_kb_shift = 0;
	_kb_keys_pressed = 0;
	_kb_last_key = 0;
	kb_handler_callback = NULL;
	kb_handler_mediumraw_callback = NULL;

	_t_prefix = 0;

	_key_buffer_head = _key_buffer_tail = _key_buffer_start;
	KB_ENABLE();
}

/***********************************************************************
//
************************************************************************/

int kb_kbhit() {
	int c;

	if (!_kb_mode)
		return kb_os_kbhit();

	kb_update();
	KB_DISABLE();
	c = (_key_buffer_head != _key_buffer_tail);
	KB_ENABLE();

	return c;
}

int kb_kbhit_i() {
	if (!_kb_mode)
		return kb_os_kbhit();
	kb_update();
	return (_key_buffer_head != _key_buffer_tail);
}

void kb_clearkeys(void) {
	kb_update();
	KB_DISABLE();
	_key_buffer_tail = _key_buffer_head;
	KB_ENABLE();
}

void kb_clearkeys_i(void) {
	kb_update();
	_key_buffer_tail = _key_buffer_head;
}

/***********************************************************************
// basic keypress access.
// this function returns all relevant information:
//   the scancode and the low bits of the shift status
// kb_getkey() and kb_inkey() sit on top of this function.
************************************************************************/

unsigned kb_keypress(void) {
	unsigned char scan, shift;
	unsigned k;

	if (!_kb_mode)
		return 0;

	kb_update();

	KB_DISABLE();
	if (_key_buffer_head == _key_buffer_tail) {
		KB_ENABLE();
		return 0;
	}

	scan = *_key_buffer_tail++;	/* get scan code */
	shift = *_key_buffer_tail++;	/* get shift code */
	if (_key_buffer_tail >= _key_buffer_end)
		_key_buffer_tail = _key_buffer_start;
	KB_ENABLE();

	k = scan | ((unsigned)shift << 8);
	return k;
}

unsigned kb_keypress_i(void) {
	unsigned char scan, shift;
	unsigned k;

	if (!_kb_mode)
		return 0;

	kb_update();

	if (_key_buffer_head == _key_buffer_tail)
		return 0;
	scan = *_key_buffer_tail++;	/* get scan code */
	shift = *_key_buffer_tail++;	/* get shift code */
	if (_key_buffer_tail >= _key_buffer_end)
		_key_buffer_tail = _key_buffer_start;

	k = scan | ((unsigned)shift << 8);
	return k;
}

unsigned kb_keypeek(void) {
	unsigned char scan, shift;
	unsigned k;

	if (!_kb_mode)
		return 0;

	kb_update();

	KB_DISABLE();
	if (_key_buffer_head == _key_buffer_tail) {
		KB_ENABLE();
		return 0;
	}

	scan = *_key_buffer_tail;	/* get scan code */
	shift = *(_key_buffer_tail + 1);	/* get shift code */
	KB_ENABLE();

	k = scan | ((unsigned)shift << 8);
	return k;
}

unsigned kb_keypeek_i(void) {
	unsigned char scan, shift;
	unsigned k;

	if (!_kb_mode)
		return 0;

	kb_update();

	if (_key_buffer_head == _key_buffer_tail)
		return 0;
	scan = *_key_buffer_tail;	/* get scan code */
	shift = *(_key_buffer_tail + 1);	/* get shift code */

	k = scan | ((unsigned)shift << 8);
	return k;
}

/***********************************************************************
// low level stuff
************************************************************************/

#if defined(__KB_MSDOS)
#include "_kbmsdos.h"
#elif defined(__KB_LINUX)
#include "_kblinux.h"
#endif

/***********************************************************************
// high level installation
************************************************************************/

int kb_init(void) {
	_kb_init_tables();
	if (!_kb_mode)
		_kb_clear();
	return 0;
}

int kb_install(unsigned long flags) {
	if (_kb_mode)
		return 0;

	_kb_flags = flags;
	_kb_flags &= 0xffff;
#if defined(KB_DEBUG)
	fprintf(stderr, "kb_install 1: 0x%08lx\n", _kb_flags);
#endif

	if (!(_kb_flags & KB_FLAG_NO_ATEXIT)) {
		static unsigned char atexit_done = 0;

		if (atexit_done)
			_kb_flags |= KB_FLAG_ATEXIT_DONE;
		else if (atexit(kb_remove) == 0) {
			_kb_flags |= KB_FLAG_ATEXIT_DONE;
			atexit_done = 1;
		} else
			_kb_flags |= KB_FLAG_ATEXIT_FAILED;
	}

	if (!(_kb_flags & KB_FLAG_NO_LOCK))
		if (_kb_lock() == 0)
			_kb_flags |= KB_FLAG_LOCK_DONE;
		else
			_kb_flags |= KB_FLAG_LOCK_FAILED;

	/* link version information into executeable */
	_kb_shift |= (kb_version_string[0] & kb_version_date[0] & kb_copyright[0]);
	kb_init();
	KB_DISABLE();
	if (_kb_install() == 0)
		_kb_mode = 1;
	KB_ENABLE();

#if !defined(_KB_NO_SIGNALS)
	/* install signals only if handler could be installed */
	if (_kb_mode)
		if (!(_kb_flags & KB_FLAG_NO_SIGNAL))
			if (_kb_signal_install() == 0)
				_kb_flags |= KB_FLAG_SIGNAL_DONE;
			else
				_kb_flags |= KB_FLAG_SIGNAL_FAILED;
#endif

#if !defined(_KB_NO_SIGNALS) && defined(SIGALRM)
	if (_kb_mode)
		if (_kb_flags & KB_FLAG_EMERGENCY_SIGALRM)
			if (_kb_signal_alarm_install() != 0)
				_kb_flags &= ~(KB_FLAG_EMERGENCY_SIGALRM);	/* failed */
#else
	_kb_flags &= ~(KB_FLAG_EMERGENCY_SIGALRM);
#endif

#if defined(KB_DEBUG)
	fflush(stdout);
	fprintf(stderr, "kb_install 2: 0x%08lx, %s\n", _kb_flags, _kb_mode ? "ok" : "FAILED");
	fflush(stderr);
#endif

	return _kb_mode ? 0 : -1;
}

/***********************************************************************
// high level removal
************************************************************************/

typedef void (*kb_cleanup_t) (void);
static kb_cleanup_t kb_cleanup_func1 = NULL, kb_cleanup_func2 = NULL;
static kb_cleanup_t kb_emergency_cleanup_func1 = NULL, kb_emergency_cleanup_func2 = NULL;

void kb_set_cleanup(void (*before) (void), void (*after) (void)) {
	kb_cleanup_func1 = before;
	kb_cleanup_func2 = after;
}

void kb_set_emergency_cleanup(void (*before) (void), void (*after) (void)) {
	kb_emergency_cleanup_func1 = before;
	kb_emergency_cleanup_func2 = after;
}

static __inline__ void kb_call_cleanup(kb_cleanup_t * k) {
	if (k != NULL && *k != NULL) {
		kb_cleanup_t c = *k;
		*k = NULL;
		(*c) ();	/* <-- call the cleanup function */
	}
}

void kb_remove(void) {
	if (!_kb_mode)
		return;

	if (_kb_flags & KB_FLAG_EMERGENCY_SIGALRM)
		_kb_signal_alarm_pause();

	kb_call_cleanup(&kb_cleanup_func1);

#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
	fprintf(stderr, "kb_remove 1\n");
#endif

	if (_kb_mode) {
		KB_DISABLE();
		_kb_mode = 0;
		_kb_remove(1);
		KB_ENABLE();
		_kb_clear();
		_kb_unlock();
#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
		fprintf(stderr, "kb_remove 2: keyboard removed\n");
		fflush(stderr);
#endif
	}

	kb_call_cleanup(&kb_cleanup_func2);
}

/* Note: this is called within an interrupt during emergency exit
 *       or from our signal handler.
 * If final is true, the call is assumed to conform to a
 *   Last In First Out (LIFO) resource allocation policy.
 * If final is false, the remove routine will be called a second time.
 *   There may be multiple non-final calls, we are really paranoid
 *   about resetting the keyboard because other libraries might
 *   restore the keyboard as well and ignore any LIFO strategies
 *   like signal chaining.
 */
void _kb_emergency_remove(int final) {
	if (!_kb_mode)
		return;

	if (_kb_flags & KB_FLAG_EMERGENCY_SIGALRM)
		_kb_signal_alarm_pause();

	if (!final) {
		/* we are in a dangerous situation and just want to make sure
		 * the keyboard is useable. We're expecting another call,
		 * the 'official' final remove, to happen soon afterwards.
		 */
		KB_DISABLE();
		_kb_remove(final);
		KB_ENABLE();
#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
		fprintf(stderr, "_kb_emergency_remove non FINAL: keyboard removed\n");
		fflush(stderr);
#endif
		return;
	}

	kb_call_cleanup(&kb_emergency_cleanup_func1);

	if (_kb_mode) {
		KB_DISABLE();
		_kb_mode = 0;
		_kb_remove(final);
		KB_ENABLE();
#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
		fprintf(stderr, "_kb_emergency_remove: keyboard removed\n");
		fflush(stderr);
#endif
	}

	kb_call_cleanup(&kb_emergency_cleanup_func2);
}

/***********************************************************************
// lock ANY addresses which may be seen by the keyboard interrupt
************************************************************************/

static int _kb_locked = 0;

int _kb_lock(void) {
	int x = 0;

	if (_kb_locked)
		return 0;

	/* lock all modules */
	x |= _libkb_kbhandle_lock();
	x |= _libkb_kbmlock_lock();
	x |= _libkb_kbos_lock();
	x |= _libkb_kbsignal_lock();
	x |= _libkb_kbtables_lock();

	if (x == 0)
		_kb_locked = 1;
	return _kb_locked ? 0 : -1;
}

void _kb_unlock(void) {
	if (!_kb_locked)
		return;

	/* do nothing - memory will be unlocked by the OS or extender
	 * when the program terminates. */

	/* _kb_locked = 0; */
}

/***********************************************************************
// debugging and error reporting
************************************************************************/

void _kb_port_debug(void) {
#if defined(KB_DEBUG)
	unsigned char *p;
	/* we'd better go volatile here */
	unsigned char **volatile bh = &_kb_port_buffer_head;

	if (!_kb_mode)
		return;

	for (p = _kb_port_buffer_start; p < *bh; p++) {
		unsigned c = *p;
		static unsigned prefix = 0;

		if (c >= 0xe0 && c <= 0xe1)
			prefix = c - (0xe0 - 1);
		else {
			if (prefix) {
				printf("%02x+", prefix + (0xe0 - 1));
				prefix--;
			}

			if (c < 0xe0 && (c & 0x80))
				printf("80+%02x ", c & 0x7f);
			else
				printf("%02x ", c);
		}
	}

	/* reset buffer pointer */
	_kb_port_buffer_head = _kb_port_buffer_start;
#endif
}

/***********************************************************************
// locking
************************************************************************/

#if defined(KB_LOCK_ALL_END)
KB_LOCK_ALL_END(_libkb_kbhandle)
#endif
int _libkb_kbhandle_lock(void) {
	int x = 0;

#if defined(KB_LOCK_ALL)
	KB_LOCK_ALL(_libkb_kbhandle, x);
#endif

	return x;
}

/*
vi:ts=4
*/
