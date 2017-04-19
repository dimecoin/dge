/* _kbmsdos.h -- low-level MSDOS keyboard installaton
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the keyboard library and is
   subject to change. Applications should only use kb.h.
 */

/***********************************************************************
// keyboard handler
************************************************************************/

#include "_handler.h"

/***********************************************************************
//
************************************************************************/

void kb_update(void) {
	if (!_kb_mode)
		return;

	/* Control-C check */
	if (_kb_flags & KB_FLAG_SIGINT) {
		if (_kb_key[KB_SCAN_C] && (_kb_key[KB_SCAN_LCONTROL] || _kb_key[KB_SCAN_RCONTROL])) {
#if !defined(_KB_NO_SIGNALS)
			raise(SIGINT);
#else
			kb_remove();
			exit(EXIT_SIG(SIGINT));	/* sorry, but what else ? */
#endif
		}
	}
#if !defined(_KB_NO_SIGNALS) && defined(SIGALRM)
	if (_kb_flags & KB_FLAG_EMERGENCY_SIGALRM)
		_kb_signal_alarm_update();
#endif
}

/***********************************************************************
//
************************************************************************/

#if defined(__BORLANDC__)

static void (interrupt far * _oldkeyint) (void) = NULL;

static int _kb_install(void) {
	_oldkeyint = getvect(9);
	setvect(9, _my_keyint);
	return _my_keyint == getvect(9) ? 0 : -1;
}

#pragma argsused
static void _kb_remove(int final) {
	if (_oldkeyint)
		setvect(9, _oldkeyint);
}

#endif				/* __BORLANDC__ */

/***********************************************************************
//
************************************************************************/

#if defined(_MSC_VER) && defined(__KB_MSDOS16)

static void (__cdecl __interrupt __far * _oldkeyint) () = NULL;

static int _kb_install(void) {
	_oldkeyint = _dos_getvect(9);
	_dos_setvect(9, _my_keyint);
	return _my_keyint == _dos_getvect(9) ? 0 : -1;
}

static void _kb_remove(int final) {
	if (_oldkeyint)
		_dos_setvect(9, _oldkeyint);
}

#endif				/* _MSC_VER */

/***********************************************************************
//
************************************************************************/

#if defined(__WATCOMC__)

static void (__interrupt __far * _oldkeyint) () = NULL;

static int _kb_install(void) {
	_oldkeyint = _dos_getvect(9);
	_dos_setvect(9, _my_keyint);
	return _my_keyint == _dos_getvect(9) ? 0 : -1;
}

static void _kb_remove(int final) {
	if (_oldkeyint)
		_dos_setvect(9, _oldkeyint);
}

#endif				/* __WATCOMC__ */

/***********************************************************************
//
************************************************************************/

#if defined(__EMX__)

static int _kb_install(void) {
	if (_osmode != DOS_MODE)
		return -1;

	/* sorry, no interrupt handler possible with emx */
	return -1;
}

static void _kb_remove(int dummy) {
}

#endif				/* __EMX__ */

/***********************************************************************
// djgpp
//
// note: Under DPMI the interrupt is always passed to the
//       protected-mode handler first. But there have been reports about
//       some strange situations under which a (faulty ?) DPMI host
//       issued a real-mode interrupt. So we are paranoid and install
//       a real-mode handler under DPMI as well. It shouldn't get called
//       anyway, but it's there just in case.
//       You can comment out the
//       _go32_dpmi_set_protected_mode_interrupt_vector() call
//       to verify that the real-mode handler is working fine.
//
//       The real-mode handler is needed for djgpp v1. Really.
************************************************************************/

#if defined(__GO32__)

static unsigned long rm_count = 0;

static void _my_rm_keyint(_go32_dpmi_registers * regs) {
	rm_count++;
	_my_keyint(regs);
}

/* See example code in djgpp2/info/libc.inf */

static _go32_dpmi_seginfo _key_pm_oldint;	/* original prot-mode key IRQ */
static _go32_dpmi_seginfo _key_pm_int;	/* prot-mode interrupt segment info */
static _go32_dpmi_seginfo _key_rm_oldint;	/* original real mode key IRQ */
static _go32_dpmi_seginfo _key_rm_int;	/* real mode interrupt segment info */
static _go32_dpmi_registers _key_rm_regs;

static int _kb_install(void) {
#if 0
	const int ino = 8 + 1;
#else
	const int ino = _go32_info_block.master_interrupt_controller_base + 1;
#endif

#if !defined(__DJGPP__)
	_kb_flags &= ~KB_FLAG_DJGPP_NO_RM;
#endif

	memset(&_key_pm_oldint, 0, sizeof(_key_pm_oldint));
	memset(&_key_pm_int, 0, sizeof(_key_pm_int));
	memset(&_key_rm_oldint, 0, sizeof(_key_rm_oldint));
	memset(&_key_rm_int, 0, sizeof(_key_rm_int));
	memset(&_key_rm_regs, 0, sizeof(_key_rm_regs));

	/* get and allocate pm handler */
	_go32_dpmi_get_protected_mode_interrupt_vector(ino, &_key_pm_oldint);
	_key_pm_int.pm_selector = _go32_my_cs();
	_key_pm_int.pm_offset = (unsigned long)_my_keyint;
	if (_go32_dpmi_allocate_iret_wrapper(&_key_pm_int) != 0)
		goto error;

	/* get and allocate rm handler */
	if (!(_kb_flags & KB_FLAG_DJGPP_NO_RM)) {
		_go32_dpmi_get_real_mode_interrupt_vector(ino, &_key_rm_oldint);
		_key_rm_int.pm_selector = _go32_my_cs();
		_key_rm_int.pm_offset = (unsigned long)_my_rm_keyint;
		if (_go32_dpmi_allocate_real_mode_callback_iret(&_key_rm_int, &_key_rm_regs) != 0)
			goto error;
	}

	/* set pm handler */
	if (_go32_dpmi_set_protected_mode_interrupt_vector(ino, &_key_pm_int) != 0)
		goto error;

	/* set rm handler */
	if (!(_kb_flags & KB_FLAG_DJGPP_NO_RM))
		_go32_dpmi_set_real_mode_interrupt_vector(ino, &_key_rm_int);

	return 0;

 error:
	memset(&_key_pm_oldint, 0, sizeof(_key_pm_oldint));
	memset(&_key_pm_int, 0, sizeof(_key_pm_int));
	memset(&_key_rm_oldint, 0, sizeof(_key_rm_oldint));
	memset(&_key_rm_int, 0, sizeof(_key_rm_int));
	memset(&_key_rm_regs, 0, sizeof(_key_rm_regs));

	return -1;
}

static void _kb_remove(int final) {
#if 0
	const int ino = 8 + 1;
#else
	const int ino = _go32_info_block.master_interrupt_controller_base + 1;
#endif
	int r = 0;

	if (_key_pm_oldint.pm_selector != 0 || _key_pm_oldint.pm_offset != 0)
		r |= _go32_dpmi_set_protected_mode_interrupt_vector(ino, &_key_pm_oldint);
	if (_key_rm_oldint.rm_segment != 0 || _key_rm_oldint.rm_offset != 0)
		r |= _go32_dpmi_set_real_mode_interrupt_vector(ino, &_key_rm_oldint);

	if (final && r == 0) {
		if (_key_pm_int.pm_selector != 0 || _key_pm_int.pm_offset != 0)
			_go32_dpmi_free_iret_wrapper(&_key_pm_int);
		if (_key_rm_int.size)
			_go32_dpmi_free_real_mode_callback(&_key_rm_int);

#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
		if (rm_count)
			fprintf(stderr, "_kb_remove: info: %lu real-mode interrupts\n", rm_count);
#endif

		memset(&_key_pm_oldint, 0, sizeof(_key_pm_oldint));
		memset(&_key_pm_int, 0, sizeof(_key_pm_int));
		memset(&_key_rm_oldint, 0, sizeof(_key_rm_oldint));
		memset(&_key_rm_int, 0, sizeof(_key_rm_int));
	}
}

#endif				/* __GO32__ */

/*
vi:ts=4
*/
