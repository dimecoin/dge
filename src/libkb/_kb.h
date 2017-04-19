/* _kb.h -- include file used for building 'libkb'
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the keyboard library and is
   subject to change. Applications should only use kb.h.
 */

#ifndef __LIBKB_KB_INTERNAL_H
#define __LIBKB_KB_INTERNAL_H

/* memory locking */
#if defined(__KB_MSDOS32)
#if defined(__DJGPP__) || defined(__WATCOMC__)
#include <kbmlock.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__inline__) && !defined(__GNUC__)
#if defined(__cplusplus)
#define __inline__		inline
#else
#define __inline__		/* nothing */
#endif
#endif

/* make KB_DEBUG numeric if defined */
#if defined(KB_DEBUG) && (KB_DEBUG < 1)
#undef KB_DEBUG
#define KB_DEBUG	1
#endif

/* exit code for a signal */
#define EXIT_SIG(signum)	(128 + ((signum) & 0x7f))

#ifndef HIGH
#define HIGH(x)		((int)(sizeof(x)/sizeof(*(x))))
#endif

#define UNUSED(parm)	(parm = parm)

/***********************************************************************
// virtual keys and shift-states for internal use
************************************************************************/

#define KB_SCAN_UNKNOWN                 0	/* internal - must be 0 */
#define KB_SCAN_MAX_RAW                 0x5d	/* 0x5d -> KB_SCAN_RMENU */

#define KB_SCAN_CONTROL_BREAK_VIRTUAL   101	/* 0x65 - Linux */
#define KB_SCAN_PAUSE_VIRTUAL           119	/* 0x77 - Linux */

/* hopefully the next code doesn't exist on an exotic keyboard... */
#define KB_SCAN_UNUSED_VIRTUAL          120	/* 0x78 */

#define KB_SHIFT_NO_PRESS       0x4000	/* do not generate a key press */
#define KB_SHIFT_VIRTUAL        0x8000	/* one of the virtual keys above */

#define KB_SHIFT_MASK_SHIFT     0x003f	/* mask for a 'normal' shift flag */
#define KB_SHIFT_MASK_TOGGLE    0x0fc0	/* mask for a toggle flag */

/***********************************************************************
// internal tables
************************************************************************/

	extern const unsigned char _kb_prefix_scancode[128];
/**** extern const unsigned char _kb_inverse_prefix_scancode[128]; */

	extern const unsigned short _kb_shift_state_table[128];
/**** extern const unsigned short _kb_unshift_state_table[128]; */

#if 0
	extern unsigned short _kb_plain_table[128];
	extern unsigned short _kb_shift_table[128];
	extern unsigned short _kb_alt_table[128];
	extern unsigned short _kb_control_table[128];
#else
/* these are static const tables now */
#define _kb_plain_table		default_plain_table
#define _kb_shift_table		default_shift_table
#define _kb_alt_table		default_alt_table
#define _kb_control_table	default_control_table
#endif

	void _kb_init_tables(void);

	extern const unsigned kb_version_id;
	extern const char kb_version_string[];
	extern const char kb_version_date[];
	extern const char kb_copyright[];

/***********************************************************************
// locking - this is very important under MSDOS + DPMI
************************************************************************/

/* one lock function for each source file */
	int _libkb_kbhandle_lock(void);
	int _libkb_kbmlock_lock(void);
	int _libkb_kbos_lock(void);
	int _libkb_kbsignal_lock(void);
	int _libkb_kbtables_lock(void);

/* main lock function */
	int _kb_lock(void);
	void _kb_unlock(void);

/***********************************************************************
// misc.
************************************************************************/

	int _kb_signal_install(void);
	int _kb_signal_lock(void);

	int _kb_signal_alarm_install(void);
	void _kb_signal_alarm_update(void);

	void _kb_emergency_remove(int final);

#if defined(__KB_LINUX)

	int _kb_linux_switch_vt(int fd, int vt);
	int _kb_linux_is_switch(int *vt, int vtnum1, int vtnum2);
	int _kb_linux_update(const unsigned char *buf, int len, void (*key_handler) (unsigned char));

#endif

/***********************************************************************
// BIOS keyboard access
************************************************************************/

#if defined(__KB_MSDOS)

/* cmd values for kb_bioskey() */
#define KB_KEYBRD_READ              0	/* read key */
#define KB_NKEYBRD_READ             0x10	/* read key - enhanced */
#define KB_KEYBRD_READY             1	/* check key ready */
#define KB_NKEYBRD_READY            0x11	/* check key ready - enhanced */
#define KB_KEYBRD_SHIFTSTATUS       2	/* get shift status */
#define KB_NKEYBRD_SHIFTSTATUS      0x12	/* get shift status - enhanced */

#endif

	int kb_bioskey(unsigned _cmd);

/***********************************************************************
// low level includes and defines
************************************************************************/

#if ( ( defined(__BORLANDC__) || defined(__TURBOC__) ) && defined(__MSDOS__) )
#include <dos.h>
#include <bios.h>
#define KB_DISABLE()		disable()
#define KB_ENABLE()		enable()
#define KB_INP8(a)		inportb(a)
#define KB_OUTP8(a,b)		outportb(a,b)
#define KB_INT86_REGS		union REGS
#define KB_INT86(n,r)		int86(n,r,r)
#elif defined(__EMX__)
#include <dos.h>
#include <sys/hw.h>
#include <os2.h>
#if 1
#define KB_DISABLE()		/* not allowed under emx */
#define KB_ENABLE()		/* not allowed under emx */
#else
#define KB_DISABLE()	__asm__ __volatile__("cli \n")
#define KB_ENABLE()		__asm__ __volatile__("sti ; cld \n")
#endif
#define KB_INP8(a)		_inp8(a)
#define KB_OUTP8(a,b)		_outp8(a,b)
#define KB_INT86_REGS		union REGS
#define KB_INT86(n,r)		_int86(n,r,r)
#define KB_USE_INT86() \
		(_osmode == DOS_MODE && (_emx_env & 0x0800))	/* _int86() allowed ? */
#elif defined(__DJGPP__)
#if !defined(__GO32__)
#error check your djgpp configuration
#endif
#include <dos.h>
#include <bios.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <unistd.h>
#if 1
#define KB_DISABLE()	__asm__ __volatile__("cli \n")
#define KB_ENABLE()		__asm__ __volatile__("sti ; cld \n")
#else
	/* don't use this - makes problems in a Windows 3.1 DOS box ! */
#define KB_DISABLE()	__dpmi_get_and_disable_virtual_interrupt_state()
#define KB_ENABLE() 	__dpmi_get_and_enable_virtual_interrupt_state()
#endif
#define KB_INP8(a)		inportb(a)
#define KB_OUTP8(a,b)		outportb(a,b)
#define KB_INT86_REGS		__dpmi_regs
#define KB_INT86(n,r)		__dpmi_int(n,r)
#elif defined(__GO32__)
#include <dos.h>
#include <bios.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#define _KB_NO_SIGNALS		/* djgpp v1 has no useable signals */
#if 1
#define KB_DISABLE()	__asm__ __volatile__("cli \n")
#define KB_ENABLE()		__asm__ __volatile__("sti ; cld \n")
#else
#define KB_DISABLE()	disable()
#define KB_ENABLE()		enable()
#endif
#define KB_INP8(a)		inportb(a)
#define KB_OUTP8(a,b)		outportb(a,b)
#define KB_INT86_REGS		union REGS
#define KB_INT86(n,r)		int86(n,r,r)
#elif defined(_MSC_VER) && defined(MSDOS)
#include <dos.h>
#include <bios.h>
#define KB_DISABLE()		_disable()
#define KB_ENABLE()		_enable()
#define KB_INP8(a)		inp(a)
#define KB_OUTP8(a,b)		outp(a,b)
#define KB_INT86_REGS		union REGS
#define KB_INT86(n,r)		int86(n,r,r)
#elif defined(__WATCOMC__) // 	&& defined(MSDOS)
#include <dos.h>
#include <bios.h>
#include <i86.h>
#define KB_DISABLE()		_disable()
#define KB_ENABLE()		_enable()
#define KB_INP8(a)		inp(a)
#define KB_OUTP8(a,b)		outp(a,b)
#define KB_INT86_REGS		union REGPACK
#define KB_INT86(n,r)		intr(n,r)
#elif defined(__KB_LINUX)
#include <unistd.h>
#define KB_INP8(a)		/* nothing */
#define KB_OUTP8(a,b)		/* nothing */
#if 1
#define KB_DISABLE()		/* not allowed and not needed under Linux */
#define KB_ENABLE()		/* not allowed and not needed under Linux */
#else
#include <asm/system.h>		/* probably only legal for kernel building */
#define KB_DISABLE()	cli()
#define KB_ENABLE()		sti()
#endif
#define KB_SIGNAME(n)		(_sys_siglist[n])
#else
#error unsupported compiler
#endif

#if !defined(KB_DISABLE)
#define KB_DISABLE()		/* nothing */
#endif
#if !defined(KB_ENABLE)
#define KB_ENABLE()		/* nothing */
#endif

#if defined(KB_INT86_REGS)
	void _kb_int86_regs_init(KB_INT86_REGS * regs);
	void _kb_int86_regs_init_ax(KB_INT86_REGS * regs, unsigned short ax);
#endif

#ifdef __cplusplus
}				/* extern "C" */
#endif
#endif				/* already included */
/*
vi:ts=4
*/
