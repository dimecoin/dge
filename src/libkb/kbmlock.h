/* kbmlock.h -- memory locking interface of the 'libkb' keyboard library
 * Copyright (C) 1995-1998 Markus F.X.J. Oberhumer
 * For conditions of distribution and use, see copyright notice in kb.h
 */

/* locking can be used independently from the rest of the library */

#ifndef __LIBKB_KBMLOCK_H
#define __LIBKB_KBMLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

	int kb_lock_code(void (*start) (void), void (*end) (void));
	int kb_lock_data(const void *start, const void *end);
	int kb_lock_var(const void *addr, unsigned size);

	int kb_unlock_code(void (*start) (void), void (*end) (void));
	int kb_unlock_data(const void *start, const void *end);
	int kb_unlock_var(const void *addr, unsigned size);

	int _kb_lock_module(int do_lock,
			    void (*cs1) (void), void (*ce1) (void),
			    void (*cs2) (void), void (*ce2) (void),
			    const void *ds1, const void *de1,
			    const void *ds2, const void *de2,
			    const void *ds3, const void *de3,
			    const void *ds4, const void *de4,
			    const void *ds5, const void *de5,
			    const void *ds6, const void *de6,
			    const void *ds7, const void *de7, const void *ds8, const void *de8);

/***********************************************************************
// lock code: static + extern
************************************************************************/

#define ___KB_LOCK_ALL_CODE_WITH_SUFFIX(id,suffix) \
	static void id ## _c_static ## suffix (void) { } \
	extern void id ## _c_extern ## suffix (void); \
	       void id ## _c_extern ## suffix (void) { }

#define KB_LOCK_ALL_CODE_START(id) \
	___KB_LOCK_ALL_CODE_WITH_SUFFIX(id,_s)

#define KB_LOCK_ALL_CODE_END(id) \
	___KB_LOCK_ALL_CODE_WITH_SUFFIX(id,_e)

#define KB_LOCK_ALL_CODE(id,x) \
	x |= kb_lock_code(id ## _c_static_s, id ## _c_static_e); \
	x |= kb_lock_code(id ## _c_extern_s, id ## _c_extern_e);

/***********************************************************************
// data: static + extern, unconst + const, initialized + not initialized
************************************************************************/

#define ___KB_LOCK_ALL_DATA_WITH_SUFFIX(id,suffix) \
	static char id ## _d_static ## suffix; \
	extern char id ## _d_extern ## suffix; \
	       char id ## _d_extern ## suffix; \
	static char id ## _i_static ## suffix = 1; \
	extern char id ## _i_extern ## suffix; \
	       char id ## _i_extern ## suffix = 1; \
	static const char id ## _o_static ## suffix; \
	extern const char id ## _o_extern ## suffix; \
	       const char id ## _o_extern ## suffix; \
	static const char id ## _p_static ## suffix = 1; \
	extern const char id ## _p_extern ## suffix; \
	       const char id ## _p_extern ## suffix = 1;

#define KB_LOCK_ALL_DATA_START(id) \
	___KB_LOCK_ALL_DATA_WITH_SUFFIX(id,_s)

#define KB_LOCK_ALL_DATA_END(id) \
	___KB_LOCK_ALL_DATA_WITH_SUFFIX(id,_e)

#define KB_LOCK_ALL_DATA(id,x) \
	x |= kb_lock_data(& id ## _d_static_s, & id ## _d_static_e); \
	x |= kb_lock_data(& id ## _d_extern_s, & id ## _d_extern_e); \
	x |= kb_lock_data(& id ## _i_static_s, & id ## _i_static_e); \
	x |= kb_lock_data(& id ## _i_extern_s, & id ## _i_extern_e); \
	x |= kb_lock_data(& id ## _o_static_s, & id ## _o_static_e); \
	x |= kb_lock_data(& id ## _o_extern_s, & id ## _o_extern_e); \
	x |= kb_lock_data(& id ## _p_static_s, & id ## _p_static_e); \
	x |= kb_lock_data(& id ## _p_extern_s, & id ## _p_extern_e);

/***********************************************************************
// lock code + data
************************************************************************/

#define KB_LOCK_ALL_START(id) \
	KB_LOCK_ALL_CODE_START(id) \
	KB_LOCK_ALL_DATA_START(id)

#define KB_LOCK_ALL_END(id) \
	KB_LOCK_ALL_CODE_END(id) \
	KB_LOCK_ALL_DATA_END(id)

#if 0
#define KB_LOCK_ALL(id,x) \
	KB_LOCK_ALL_CODE(id,x) \
	KB_LOCK_ALL_DATA(id,x)
#else
#define KB_LOCK_ALL(id,x) \
	x |= _kb_lock_module(1,  id ## _c_static_s,   id ## _c_static_e, \
	                         id ## _c_extern_s,   id ## _c_extern_e, \
	                       & id ## _d_static_s, & id ## _d_static_e, \
	                       & id ## _d_extern_s, & id ## _d_extern_e, \
	                       & id ## _i_static_s, & id ## _i_static_e, \
	                       & id ## _i_extern_s, & id ## _i_extern_e, \
	                       & id ## _o_static_s, & id ## _o_static_e, \
	                       & id ## _o_extern_s, & id ## _o_extern_e, \
	                       & id ## _p_static_s, & id ## _p_static_e, \
	                       & id ## _p_extern_s, & id ## _p_extern_e);
#endif

#ifdef __cplusplus
}				/* extern "C" */
#endif
#endif				/* already included */
/*
vi:ts=4
*/
