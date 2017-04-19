/* kbtables.c -- keyboard tables and high level access functions
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

#include <stdlib.h>
#include <string.h>
#include <kb.h>
#include "_kb.h"
#include "_kbtable.hh"
#include "_kbname.hh"

#if defined(KB_LOCK_ALL_START)
KB_LOCK_ALL_START(_libkb_kbtables)
#endif
/* If you use the libkb library in a product, you *must* keep these
 * copyright strings in the executable of your product.
 */
const unsigned kb_version_id = KB_VERSION_ID;
const char kb_version_string[] = KB_VERSION_STRING;
const char kb_version_date[] = KB_VERSION_DATE;
const char kb_copyright[] =
    "\n\n\n"
    "libkb -- a free, advanced and portable low-level keyboard library\n"
    "Copyright (C) 1995-1998 Markus Franz Xaver Johannes Oberhumer\n"
    "<markus.oberhumer@jk.uni-linz.ac.at>\n"
    "\n"
    "libkb version: v" KB_VERSION_STRING ", " KB_VERSION_DATE "\n"
    "libkb build date: " __DATE__ " " __TIME__ "\n" "\n\n";

/* Once again (17 Jan 1996) the Linux kernel sources have stopped
 * me from making things too complicate. As this is a keyboard
 * library and not a re-write of an operating system, I have
 * simplified the tables to suit the basic needs.
 * There are 4 tables that are set up for American keyboards.
 * If you really want to do use local key mappings while the
 * keyboard handler is active, use kb_keypress() and
 * convert the code yourself.
 */

#if !defined(_kb_plain_table)
unsigned short _kb_plain_table[128];
#endif
#if !defined(_kb_shift_table)
unsigned short _kb_shift_table[128];
#endif
#if !defined(_kb_control_table)
unsigned short _kb_control_table[128];
#endif
#if !defined(_kb_alt_table)
unsigned short _kb_alt_table[128];
#endif

/***********************************************************************
// convert kb_keypress() to a keycode
//
// see comments for keycode() in mktables.c
************************************************************************/

unsigned kb_keycode(unsigned k) {
	unsigned scan = k & 0x7f;

	if (KB_ANY_MASK(k, KB_SHIFT_ANY_ALT << 8))
		return _kb_alt_table[scan];
	else if (KB_ANY_MASK(k, KB_SHIFT_ANY_CONTROL << 8))
		return _kb_control_table[scan];
	else if (KB_ANY_MASK(k, KB_SHIFT_ANY_SHIFT << 8))
		return _kb_shift_table[scan];
	else
		return _kb_plain_table[scan];
}

/***********************************************************************
//
************************************************************************/

static unsigned long kb_inkey_keycode(unsigned key) {
	if (key)
		return kb_keycode(key) | ((unsigned long)key << 16);
	else
		return 0;
}

static unsigned kb_getkey_keycode(unsigned key) {
	if (key)
		return kb_keycode(key);
	else
		return 0;
}

/***********************************************************************
//
************************************************************************/

unsigned long kb_inkey(void) {
	unsigned key;
	unsigned long code;

	if (!_kb_mode)
		return kb_os_getkey();
	while ((key = kb_keypress()) != 0) {
		code = kb_inkey_keycode(key);
		if (code != 0)
			return code;
	}
	return 0;
}

unsigned long kb_inkey_i(void) {
	unsigned key;
	unsigned long code;

	if (!_kb_mode)
		return kb_os_getkey();
	while ((key = kb_keypress_i()) != 0) {
		code = kb_inkey_keycode(key);
		if (code != 0)
			return code;
	}
	return 0;
}

unsigned kb_getkey(void) {
	unsigned key, code;

	if (!_kb_mode)
		return kb_os_getkey();
	while ((key = kb_keypress()) != 0) {
		code = kb_getkey_keycode(key);
		if (code != 0)
			return code;
	}
	return 0;
}

unsigned kb_getkey_i(void) {
	unsigned key, code;

	if (!_kb_mode)
		return kb_os_getkey();
	while ((key = kb_keypress_i()) != 0) {
		code = kb_getkey_keycode(key);
		if (code != 0)
			return code;
	}
	return 0;
}

unsigned kb_waitkey(void) {
	if (!_kb_mode)
		return kb_os_waitkey();

	while (!kb_kbhit())
		_kb_usleep(1024);
	return kb_getkey();
}

unsigned kb_waitkey_i(void) {
	if (!_kb_mode)
		return kb_os_waitkey();

	while (!kb_kbhit_i())
		_kb_usleep(1024);
	return kb_getkey_i();
}

/***********************************************************************
// kb_keycode, kb_bios_keycode and kb_os_keycode should return
// the same values - and this on all platforms
************************************************************************/

/* convert a BIOS keycode */
/* see: djgpp1/libsrc/c/dos/getkey.s */
/* see: djgpp2/src/libc/pc_hw/kb */

unsigned kb_bios_keycode(unsigned k) {
	unsigned key = k & 0xff;
	unsigned x = (k >> 8) & 0xff;

	if (key == 0)
		key = 0x100 | x;
	else if (key == 0xe0 && x != 0)
		key = 0x200 | x;

	return key;
}

/* convert a getch() keycode */
unsigned kb_os_keycode(unsigned k1, unsigned k2) {
	if (k2 == 0)
		return k1;

	/* we cannot distinguish between keypad and cursor keys,
	 * so let's assume cursor keys */
	if (k2 >= KB_SCAN_7_PAD && k2 <= KB_SCAN_PERIOD_PAD && _kb_prefix_scancode[k2])
		k2 |= 0x200;
	else
		k2 |= 0x100;

	return k2;
}

/***********************************************************************
//
************************************************************************/

void _kb_init_tables(void) {
#if !defined(_kb_plain_table)
	memcpy(_kb_plain_table, default_plain_table, sizeof(_kb_plain_table));
#endif
#if !defined(_kb_shift_table)
	memcpy(_kb_shift_table, default_shift_table, sizeof(_kb_shift_table));
#endif
#if !defined(_kb_control_table)
	memcpy(_kb_control_table, default_control_table, sizeof(_kb_control_table));
#endif
#if !defined(_kb_alt_table)
	memcpy(_kb_alt_table, default_alt_table, sizeof(_kb_alt_table));
#endif
}

/***********************************************************************
//
************************************************************************/

const char *kb_keyname(int scan) {
	return _kb_key_name[scan & 0x7f];
}

/***********************************************************************
// locking
************************************************************************/

#if defined(KB_LOCK_ALL_END)
KB_LOCK_ALL_END(_libkb_kbtables)
#endif
int _libkb_kbtables_lock(void) {
	int x = 0;

#if defined(KB_LOCK_ALL)
	KB_LOCK_ALL(_libkb_kbtables, x);
#endif

	return x;
}

/*
vi:ts=4
*/
