/* kbsignal.c -- signal handling
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <kb.h>
#include "_kb.h"

#if defined(KB_LOCK_ALL_START)
KB_LOCK_ALL_START(_libkb_kbsignal)
#endif
/***********************************************************************
// wrapper for djgpp v1
************************************************************************/
#if defined(_KB_NO_SIGNALS)
const int _kb_nsig = 0;
const int _kb_sigmin = 0;
const int _kb_sigmax = -1;

#if !defined(SIG_ERR)
#define SIG_ERR	NULL
#endif

int _kb_signal_install(void) {
	return 0;		/* installation ok */
}

static void _my_signal_handler(int signum) {
	_kb_emergency_remove(1);
	exit(EXIT_SIG(signum));
}

const kb_sighandler_t kb_signal_handler = _my_signal_handler;

kb_sighandler_t kb_signal(int signum, kb_sighandler_t handler) {
	return SIG_ERR;
}

kb_sighandler_t kb_get_chained_signal(int signum) {
	return SIG_ERR;
}

kb_sighandler_t kb_set_chained_signal(int signum, kb_sighandler_t h) {
	return SIG_ERR;
}

#endif				/* _KB_NO_SIGNALS */

/***********************************************************************
// IMPORTANT NOTE: check your compiler flags so that no
//                 stack overflow checks will be inserted here !
************************************************************************/

#if !defined(_KB_NO_SIGNALS)

/* don't waste memory */
#if defined(__DJGPP__)
#define KB_SIGMIN	288
#else
#define KB_SIGMIN	0
#endif

/* NSIG is the highest defined signal number + 1 */
#if !defined(NSIG)
#if defined(_NSIG)
#define NSIG	_NSIG
#elif defined(SIGMAX)
#define NSIG	(SIGMAX+1)
#elif defined(_SIGMAX)
#define NSIG	(_SIGMAX+1)
#else
#define NSIG	(32 + KB_SIGMIN)
#endif
#endif

const int _kb_nsig = NSIG;
const int _kb_sigmin = KB_SIGMIN;
const int _kb_sigmax = NSIG - 1;

/* sigaction() gives us more control than signal() */
#if defined(__KB_LINUX)
#define _KB_USE_SIGACTION_GET
#elif defined(__EMX__)
#define _KB_USE_SIGACTION_GET
#define _KB_USE_SIGACTION_SET
#endif

/***********************************************************************
// wrappers for sigaction/signal
************************************************************************/

#if defined(_KB_USE_SIGACTION_GET)
static kb_sighandler_t kb_signal_get_sa(int signum, struct sigaction *sa) {
	if (sa == NULL)
		return SIG_ERR;
	if (sigaction(signum, NULL, sa) != 0)
		return SIG_ERR;
	return sa->sa_handler;
}
#endif

#if defined(_KB_USE_SIGACTION_SET)
static kb_sighandler_t kb_signal_set_sa(int signum, kb_sighandler_t h, struct sigaction *sa) {
	kb_sighandler_t oldh;

	if (sa == NULL)
		return SIG_ERR;
	if (sigaction(signum, NULL, sa) != 0)
		return SIG_ERR;
	oldh = sa->sa_handler;
	if (oldh == SIG_ERR || h == SIG_ERR)
		return SIG_ERR;
	sa->sa_handler = h;
#if defined(__EMX__)		/* use the emx signal processing model */
	sa->sa_flags &= ~(SA_SYSV);
	sa->sa_flags |= (SA_ACK);
#endif
	if (sigaction(signum, sa, NULL) != 0)
		return SIG_ERR;
	return oldh;
}
#endif

static kb_sighandler_t kb_signal_get(int signum) {
#if defined(_KB_USE_SIGACTION_GET)
	struct sigaction sa;
	return kb_signal_get_sa(signum, &sa);
#else
	kb_sighandler_t h, n;

	n = SIG_DFL;
	h = signal(signum, n);	/* get and set handler */
	if (h == SIG_ERR)	/* error in signal() call */
		return SIG_ERR;
	if (h != n)
		if (signal(signum, h) == SIG_ERR)	/* restore handler */
			return SIG_ERR;
	return h;
#endif
}

static kb_sighandler_t kb_signal_set(int signum, kb_sighandler_t h) {
#if defined(_KB_USE_SIGACTION_SET)
	struct sigaction sa;
	return kb_signal_set_sa(signum, h, &sa);
#else
	return signal(signum, h);
#endif
}

/***********************************************************************
// vars for the old (chained) signal handlers
************************************************************************/

/* don't waste memory */
#define old_sigh		(__old_sigh - KB_SIGMIN)
#define sig_installed	(__sig_installed - KB_SIGMIN)

#if defined(_KB_USE_SIGACTION_GET)
static struct sigaction __old_sigh[NSIG - KB_SIGMIN];
#define OLD_SIGH(signum)	(old_sigh[signum].sa_handler)
#else
static kb_sighandler_t __old_sigh[NSIG - KB_SIGMIN];
#define OLD_SIGH(signum)	(old_sigh[signum])
#endif

static unsigned char __sig_installed[NSIG - KB_SIGMIN];

/***********************************************************************
// our signal handler
************************************************************************/

static void _my_signal_handler(int signum) {
	int err;

#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
	/* remove keyboard handler before doing anything else */
	_kb_emergency_remove(0);

	/* file I/O is somewhat dangerous within a signal handler ... */
	fprintf(stderr, "libkb: received signal %d%s\n", signum,
		(signum < KB_SIGMIN || signum >= NSIG) ? "  STRANGE !!!" : "");
	fflush(stderr);
#endif

	/* remove keyboard handler */
	_kb_emergency_remove(1);

	/* sanity check */
	if (signum < KB_SIGMIN || signum >= NSIG)
		return;

	/* restore old signal handler */
	if (sig_installed[signum] != 0x01) {	/* should not happen */
		err = (signal(signum, SIG_DFL) == SIG_ERR);
#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
		fprintf(stderr, "libkb: signal %d not expected !!!\n", signum);
		fflush(stderr);
#endif
	}
#if defined(_KB_USE_SIGACTION_GET)
	else if (sigaction(signum, &old_sigh[signum], NULL) != 0)
		err = 1;
#else
	else if (signal(signum, old_sigh[signum]) == SIG_ERR)
		err = 1;
#endif
	else
		err = 0;

#if defined(__EMX__)		/* emx signal processing model */
	signal(signum, SIG_ACK);	/* ack. signal */
#endif

	/* chain old signal handler - this should terminate the program */
	if (!err) {
		sig_installed[signum] = 0;	/* no longer active */
#if defined(KB_DEBUG) && (KB_DEBUG >= 2)
		fprintf(stderr, "libkb: chaining signal %d\n", signum);
		fflush(stderr);
#endif
		raise(signum);
	}

	/* if we return from here, the application should be aware
	 * that the keyboard handler was removed.
	 */
}

/* improve source code readability (at least a little bit :-) */
#define MY_HANDLER		_my_signal_handler

/* make accessible */
const kb_sighandler_t kb_signal_handler = MY_HANDLER;

/***********************************************************************
// install a signal handler (compatible to signal())
************************************************************************/

kb_sighandler_t kb_signal(int signum, kb_sighandler_t handler) {
	kb_sighandler_t h;

	if (signum < KB_SIGMIN || signum >= NSIG || handler == SIG_ERR)
		return SIG_ERR;
	h = kb_signal_get(signum);
	if (h == SIG_ERR) {
		sig_installed[signum] = 0x81;
		return SIG_ERR;
	}

	if (h == MY_HANDLER && sig_installed[signum] != 0x01) {
		/* strange: someone installed my handler without my knowledge */
		sig_installed[signum] = 0x01;
	}

	if (h == handler)	/* no change */
		return h;

	if (handler != MY_HANDLER) {	/* not my handler */
		if (kb_signal_set(signum, handler) != h)
			return SIG_ERR;
		if (handler == SIG_IGN)
			sig_installed[signum] = 0x20;
		else
			sig_installed[signum] = 0x10;
		return h;
	}

	/* are you still here ? :-) */
	/* assert(handler == MY_HANDLER); */

	/* store current signal information */
#if defined(_KB_USE_SIGACTION_GET)
	if (kb_signal_get_sa(signum, &old_sigh[signum]) != h)
		return SIG_ERR;
#else
	old_sigh[signum] = h;
#endif

	/* if the signal is ignored, we ignore it as well */
	if (h == SIG_IGN) {
		sig_installed[signum] = 0x02;
		return h;
	}

	/* now the time has come to install our handler */
	if (kb_signal_set(signum, handler) != h) {
		sig_installed[signum] = 0x82;
		return SIG_ERR;
	}

	/* everything ok */
	sig_installed[signum] = 0x01;
	return h;
}

/***********************************************************************
// install all signal handlers
// note: a handler cannot be installed for SIGKILL, so
//       don't "kill -9" as this will not restore the keyboard
************************************************************************/

/* FIXME: which signals ? */
/* We catch all signals that cause an exit by default (aka almost all) */
static const short signals_to_catch[] = {
	SIGABRT,		/* ANSI */
#if defined(SIGALRM)
	SIGALRM,		/* POSIX.1 */
#endif
#if defined(SIGBREAK)
	SIGBREAK,
#endif
#if defined(SIGBUS)
	SIGBUS,
#endif
	SIGFPE,			/* ANSI */
#if defined(SIGHUP)
	SIGHUP,			/* POSIX.1 */
#endif
	SIGILL,			/* ANSI */
	SIGINT,			/* ANSI */
#if defined(SIGIOT)
	SIGIOT,
#endif
#if defined(SIGKILL)
	SIGKILL,		/* we can set a handler for djgpp v2 */
#endif
#if defined(SIGPIPE)
	SIGPIPE,		/* POSIX.1 */
#endif
#if defined(SIGPROF)
	SIGPROF,
#endif
#if defined(SIGPWR)
	SIGPWR,
#endif
#if defined(SIGQUIT)
	SIGQUIT,		/* POSIX.1 */
#endif
	SIGSEGV,		/* ANSI */
#if defined(SIGTERM)
	SIGTERM,		/* ANSI */
#endif
#if defined(SIGTRAP)
	SIGTRAP,
#endif
#if defined(SIGXCPU)
	SIGXCPU,
#endif
#if defined(SIGXFSZ)
	SIGXFSZ,
#endif
#if defined(SIGVTALRM)
	SIGVTALRM,
#endif
};

int _kb_signal_install(void) {
	int i;
	int saved_errno;

	saved_errno = errno;

	for (i = 0; i < HIGH(signals_to_catch); i++) {
		int signum;
		kb_sighandler_t h;

		signum = signals_to_catch[i];
		h = kb_signal(signum, MY_HANDLER);

#if defined(KB_DEBUG) && (KB_DEBUG >= 3)
		fprintf(stderr, "libkb info: handler for signal %2d", signum);
		fprintf(stderr, ": 0x%02x, %s", sig_installed[signum], h == SIG_ERR ? "FAILED" : "ok");
#if defined(KB_SIGNAME)
		fprintf(stderr, "  (%s)", KB_SIGNAME(signum));
#endif
		fprintf(stderr, "\n");
#endif
		h = h;
	}

#if defined(KB_DEBUG)
	fprintf(stderr, "libkb info: installed a signal handler for these signals:\n");
	for (i = KB_SIGMIN; i < NSIG; i++)
		if (sig_installed[i] == 0x01)
			fprintf(stderr, "%d ", i);
	fprintf(stderr, "\n");
	fflush(stderr);
#endif

	errno = saved_errno;

	/* return ok */
	return 0;
}

/***********************************************************************
// signal chaining
************************************************************************/

kb_sighandler_t kb_get_chained_signal(int signum) {
	kb_sighandler_t h;

	if (signum < KB_SIGMIN || signum >= NSIG)
		return SIG_ERR;
	if (sig_installed[signum] != 0x01)
		return SIG_ERR;

	h = OLD_SIGH(signum);
	/* sanity check - may not happen */
	if (h == SIG_IGN || h == SIG_ERR || h == MY_HANDLER)
		return SIG_ERR;
	return h;
}

kb_sighandler_t kb_set_chained_signal(int signum, kb_sighandler_t handler) {
	kb_sighandler_t h;

	/* cannot chain to SIG_IGN */
	if (handler == SIG_IGN || handler == SIG_ERR)
		return SIG_ERR;
	/* cannot chain to the signal handler itself */
	if (handler == MY_HANDLER)
		return SIG_ERR;

	h = kb_get_chained_signal(signum);
	if (h == SIG_ERR)
		return SIG_ERR;

	OLD_SIGH(signum) = handler;
	return h;
}

#endif				/* !_KB_NO_SIGNALS */

/***********************************************************************
// emergency SIGALRM
************************************************************************/

#if !defined(_KB_NO_SIGNALS) && defined(SIGALRM)

static int running = 0;

int _kb_signal_alarm_install(void) {
	if (sig_installed[SIGALRM] != 0x01)
		return -1;
	if (_kb_signal_alarm_resume() < 0)
		return -1;
	return 0;
}

void _kb_signal_alarm_update(void) {
	if (running)
		alarm(30);
}

int _kb_signal_alarm_pause(void) {
	int old_running = running;

	if (!(_kb_mode && (_kb_flags & KB_FLAG_EMERGENCY_SIGALRM)))
		return -1;

	alarm(0);
	running = 0;
	return old_running;
}

int _kb_signal_alarm_resume(void) {
	int old_running = running;

	if (!(_kb_mode && (_kb_flags & KB_FLAG_EMERGENCY_SIGALRM)))
		return -1;

	running = 1;
	_kb_signal_alarm_update();
	return old_running;
}

#else

int _kb_signal_alarm_pause(void) {
	return -1;
}

int _kb_signal_alarm_resume(void) {
	return -1;
}

#endif

/***********************************************************************
// locking
************************************************************************/

#if defined(KB_LOCK_ALL_END)
KB_LOCK_ALL_END(_libkb_kbsignal)
#endif
int _libkb_kbsignal_lock(void) {
	int x = 0;

#if defined(KB_LOCK_ALL)
	KB_LOCK_ALL(_libkb_kbsignal, x);
#endif

	return x;
}

/*
vi:ts=4
*/
