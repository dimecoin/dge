/* _handler.h -- keyboard handler
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the keyboard library and is
   subject to change. Applications should only use kb.h.
 */

/***********************************************************************
// int9 keyboard interrupt handler (IRQ1 keyboard data ready)
//
// IMPORTANT NOTE: check your compiler flags so that no
//                 stack overflow checks will be inserted here !
//
// Most of this code should be quite obvious.
//   Prefixed keys (E0) are mapped to unique scancodes.
//
// The Pause key requires some extra handling:
//   I have observed the following:
//     Pressing Pause generates these 3 codes: E1 1D 45
//     Pressing Control+Pause (== Break) generates E0 46 which is
//       mapped to an unique virtual scancode.
//     Pressing both Control keys + Pause generates Pause and not E0 46.
//     The release codes for Pause (E1 80+1D 80+45) and Control+Pause
//       (E0 80+46) are always generated after the press codes !
//       Releasing Pause or Control+Pause generates nothing - this is the
//       reason that there is no scancode for these, only the shift toggles.
//     The Pause key is the only key that generates E1.
//   Implementation:
//     The only thing to care for is the code 1D (KB_SCAN_LCONTROL)
//       in combination with the prefix count.
//       This code is mapped to a special virtual key so that the
//       current shift status won't get touched.
//     Fortunately E0 45 is never generated, so
//       the E1 1D 45 conversion works fine.
//
// The Print key is also somewhat unusual:
//   normally it generates E0 37, but when Alt is pressed then 54
************************************************************************/

#if ( defined(__BORLANDC__) || defined(__TURBOC__) ) && defined(__KB_MSDOS16)
static void interrupt far _my_keyint() {
	unsigned char _t_scan;
	unsigned short _t_shift;

#elif defined(_MSC_VER) && defined(__KB_MSDOS16)
static void __cdecl __interrupt __far _my_keyint() {
	unsigned char _t_scan;
	unsigned short _t_shift;

#elif defined(__EMX__) && defined(__KB_MSDOS)
static void _my_keyint() {
	unsigned char _t_scan;
	unsigned short _t_shift;

#elif defined(__DJGPP__) && defined(__KB_MSDOS)
static void _my_keyint(_go32_dpmi_registers * regs) {
	unsigned char _t_scan;
	unsigned short _t_shift;

	UNUSED(regs);

#elif defined(__GO32__) && defined(__KB_MSDOS)
	/* stop djgpp v1 messing up with stack reads */
static unsigned char _t_scan;
static unsigned short _t_shift;
static void _my_keyint(_go32_dpmi_registers * regs) {
	UNUSED(regs);

#elif defined(__WATCOMC__) && defined(__KB_MSDOS)
static void __interrupt __far _my_keyint(union INTPACK regs) {
	unsigned char _t_scan;
	unsigned short _t_shift;

#elif defined(__KB_LINUX)

	/* note: this is NOT an interrupt handler */
#if defined(KB_LINUX_MEDIUMRAW)
static void _my_mediumraw_handler(unsigned char _t_scan)
#else
static void _my_raw_handler(unsigned char _t_scan)
#endif
{
	unsigned short _t_shift;

#else
#error unsupported compiler
#endif

#if defined(__KB_MSDOS)
	/* read keyboard byte */
_t_scan = KB_INP8(0x60);

	/* tell the XT keyboard controller to clear the key */
    /* this is only necessary on XT's */
#if defined(__KB_MSDOS16)
{
	/* TODO: do we have to care about slow I/O on fast machines ? */
	unsigned char _t_tmp = KB_INP8(0x61);
	KB_OUTP8(0x61, _t_tmp | 0x80);
	KB_OUTP8(0x61, _t_tmp & 0x7f);
}
#endif
#endif

#if !defined(KB_LINUX_MEDIUMRAW)
if (kb_handler_callback)
	kb_handler_callback(_t_scan);
#endif

#if defined(KB_DEBUG)
	/* store port value in buffer */
*_kb_port_buffer_head++ = _t_scan;
if (_kb_port_buffer_head >= _kb_port_buffer_end)
	_kb_port_buffer_head = _kb_port_buffer_start;
#endif

#if !defined(KB_LINUX_MEDIUMRAW)
if (_t_scan >= 0xe0) {
	if (_t_scan <= 0xe1) {	/* E0, E1: key prefix */
		_t_prefix = _t_scan - (0xe0 - 1);	/* prefix count */
	} else if (_t_scan == 0xff) {	/* handle overflow */
		/* try it: press a lot of keys */
		_t_prefix = 0;
		_kb_shift |= KB_SHIFT_OVERFLOW;
	} else {
		/* ignore unknown keys */
		_t_prefix = 0;
		_kb_shift |= KB_SHIFT_UNKNOWN;
	}
} else
#endif

if (_t_scan & 0x80) {		/* key was released */
	_t_scan &= 0x7f;
#if !defined(KB_LINUX_MEDIUMRAW)
	if (_t_scan > KB_SCAN_MAX_RAW) {
		_t_scan = KB_SCAN_UNKNOWN;
		_t_prefix = 0;
	} else if (_t_prefix > 0) {	/* convert scancode */
		if (--_t_prefix > 0)
			_t_scan = (_t_scan == KB_SCAN_LCONTROL) ? KB_SCAN_UNUSED_VIRTUAL : KB_SCAN_UNKNOWN;
		else
			_t_scan = _kb_prefix_scancode[_t_scan];
	}
#endif

	if (kb_handler_mediumraw_callback)
		kb_handler_mediumraw_callback(_t_scan | 0x80);

	_t_shift = _kb_shift_state_table[_t_scan];

	if (_t_shift & KB_SHIFT_UNKNOWN)
		_kb_shift |= KB_SHIFT_UNKNOWN;
	else if (_kb_key[_t_scan]) {
		_kb_key[_t_scan] = 0;
		_t_shift &= KB_SHIFT_MASK_SHIFT;
		_kb_shift &= ~_t_shift;	/* clear bit */
		--_kb_keys_pressed;
	}
}

else {				/* key was pressed */

#if !defined(KB_LINUX_MEDIUMRAW)
	if (_t_scan > KB_SCAN_MAX_RAW) {
		_t_scan = KB_SCAN_UNKNOWN;
		_t_prefix = 0;
	} else if (_t_prefix > 0) {	/* convert scancode */
		if (--_t_prefix > 0)
			_t_scan = (_t_scan == KB_SCAN_LCONTROL) ? KB_SCAN_UNUSED_VIRTUAL : KB_SCAN_UNKNOWN;
		else
			_t_scan = _kb_prefix_scancode[_t_scan];
	}
#endif

	if (kb_handler_mediumraw_callback)
		kb_handler_mediumraw_callback(_t_scan);

	_t_shift = _kb_shift_state_table[_t_scan];

	if (_t_shift & KB_SHIFT_UNKNOWN)
		_kb_shift |= KB_SHIFT_UNKNOWN;
	else if (!_kb_key[_t_scan]) {
		_kb_shift ^= _t_shift;	/* toggle shift status */
		if (!(_t_shift & KB_SHIFT_VIRTUAL)) {	/* do not count virtual keys */
			_kb_key[_t_scan] = 1;
			_kb_last_key = _t_scan | (_kb_shift << 8);
#if (USHRT_MAX > 0xffff)
			_kb_last_key &= 0xffff;
#endif
			++_kb_keys_pressed;
		}
	} else if (_kb_flags & KB_FLAG_REPEAT_OFF)
		_t_shift |= KB_SHIFT_NO_PRESS;

	if (!(_t_shift & KB_SHIFT_NO_PRESS)) {	/* if this key stores a press */
		/* store keypress: scan code + low bits of shift state */
		*_key_buffer_head++ = _t_scan;
		*_key_buffer_head++ = (unsigned char)_kb_shift;

		/* adjust pointers */
		if (_key_buffer_head >= _key_buffer_end)
			_key_buffer_head = _key_buffer_start;
		if (_key_buffer_head == _key_buffer_tail) {	/* buffer full */
			_key_buffer_tail += 2;
			if (_key_buffer_tail >= _key_buffer_end)
				_key_buffer_tail = _key_buffer_start;
		}
	}

	/* Emergency exit.
	 * Raising a signal or exiting within an interrupt handler is probably
	 * no good idea. The DPMI specs also say that an application should not
	 * terminate during an interrupt that is reflected from real-mode
	 * to protected-mode.
	 * But during development it could be worthwile to try it - if your
	 * program locks, a reboot would be necessary anyway.
	 * It seems to work with Borland C/Watcom C/djgpp though.
	 * note: both control keys have to be pressed
	 */
	if (_kb_flags & KB_FLAG_EMERGENCY_EXIT) {
		if (!_emergency_done && _kb_key[KB_SCAN_C] &&
		    _kb_key[KB_SCAN_LCONTROL] && (_kb_key[KB_SCAN_RCONTROL] || _kb_key[KB_SCAN_RMENU])) {
			_emergency_done = 1;
#if defined(__DJGPP__)
			/* Internal function that raises SIGINT immediately
			 * after the end of this interrupt.
			 * Do not ignore SIGINT when using emergency-exit !
			 */
			__asm__ __volatile__("  movb $0x79,%%al \n"
					     "  call ___djgpp_hw_exception \n":::"%eax", "%ebx", "%ecx", "%edx",
					     "%esi", "%edi", "memory", "cc");
#else
			KB_ACKINT();	/* ack. interrupt */
			_kb_emergency_remove(0);
#if !defined(_KB_NO_SIGNALS)
			raise(SIGINT);
#if defined(SIGBREAK)
			raise(SIGBREAK);	/* try this if SIGINT returns */
#endif
#endif
			exit(EXIT_SIG(SIGINT));	/* exit if signal handlers return */
#endif
		}
	}
}

KB_ACKINT();			/* ack. interrupt */
}

/*
vi:ts=4
*/
