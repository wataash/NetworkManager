// SPDX-License-Identifier: LGPL-2.1+
/*
 * Copyright (C) 2012 Colin Walters <walters@verbum.org>.
 * Copyright (C) 2014 Red Hat, Inc.
 */

#ifndef __NM_MACROS_INTERNAL_H__
#define __NM_MACROS_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <gio/gio.h>

/*****************************************************************************/

#define _nm_packed           __attribute__ ((__packed__))
#define _nm_unused           __attribute__ ((__unused__))
#define _nm_used             __attribute__ ((__used__))
#define _nm_pure             __attribute__ ((__pure__))
#define _nm_const            __attribute__ ((__const__))
#define _nm_printf(a,b)      __attribute__ ((__format__ (__printf__, a, b)))
#define _nm_align(s)         __attribute__ ((__aligned__ (s)))
#define _nm_section(s)       __attribute__ ((__section__ (s)))
#define _nm_alignof(type)    __alignof (type)
#define _nm_alignas(type)    _nm_align (_nm_alignof (type))
#define nm_auto(fcn)         __attribute__ ((__cleanup__(fcn)))


/* This is required to make LTO working.
 *
 * See https://gitlab.freedesktop.org/NetworkManager/NetworkManager/merge_requests/76#note_112694
 *     https://gcc.gnu.org/bugzilla/show_bug.cgi?id=48200#c28
 */
#ifndef __clang__
#define _nm_externally_visible __attribute__ ((__externally_visible__))
#else
#define _nm_externally_visible
#endif


#if __GNUC__ >= 7
#define _nm_fallthrough      __attribute__ ((__fallthrough__))
#else
#define _nm_fallthrough
#endif

/*****************************************************************************/

#ifdef thread_local
#define _nm_thread_local thread_local
/*
 * Don't break on glibc < 2.16 that doesn't define __STDC_NO_THREADS__
 * see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53769
 */
#elif __STDC_VERSION__ >= 201112L && !(defined(__STDC_NO_THREADS__) || (defined(__GNU_LIBRARY__) && __GLIBC__ == 2 && __GLIBC_MINOR__ < 16))
#define _nm_thread_local _Thread_local
#else
#define _nm_thread_local __thread
#endif

/*****************************************************************************/

/* most of our code is single-threaded with a mainloop. Hence, we usually don't need
 * any thread-safety. Sometimes, we do need thread-safety (nm-logging), but we can
 * avoid locking if we are on the main-thread by:
 *
 *   - modifications of shared data is done infrequently and only from the
 *     main-thread (nm_logging_setup())
 *   - read-only access is done frequently (nm_logging_enabled())
 *     - from the main-thread, we can do that without locking (because
 *       all modifications are also done on the main thread.
 *     - from other threads, we need locking. But this is expected to be
 *       done infrequently too. Important is the lock-free fast-path on the
 *       main-thread.
 *
 * By defining NM_THREAD_SAFE_ON_MAIN_THREAD you indicate that this code runs
 * on the main-thread. It is by default defined to "1". If you have code that
 * is also used on another thread, redefine the define to 0 (to opt in into
 * the slow-path).
 */
#define NM_THREAD_SAFE_ON_MAIN_THREAD 1

/*****************************************************************************/

#define NM_AUTO_DEFINE_FCN_VOID(CastType, name, func) \
static inline void name (void *v) \
{ \
	func (*((CastType *) v)); \
}

#define NM_AUTO_DEFINE_FCN_VOID0(CastType, name, func) \
static inline void name (void *v) \
{ \
	if (*((CastType *) v)) \
		func (*((CastType *) v)); \
}

#define NM_AUTO_DEFINE_FCN(Type, name, func) \
static inline void name (Type *v) \
{ \
	func (*v); \
}

#define NM_AUTO_DEFINE_FCN0(Type, name, func) \
static inline void name (Type *v) \
{ \
	if (*v) \
		func (*v); \
}

/*****************************************************************************/

/**
 * gs_free:
 *
 * Call g_free() on a variable location when it goes out of scope.
 */
#define gs_free nm_auto(gs_local_free)
NM_AUTO_DEFINE_FCN_VOID0 (void *, gs_local_free, g_free)

/**
 * gs_unref_object:
 *
 * Call g_object_unref() on a variable location when it goes out of
 * scope.  Note that unlike g_object_unref(), the variable may be
 * %NULL.
 */
#define gs_unref_object nm_auto(gs_local_obj_unref)
NM_AUTO_DEFINE_FCN_VOID0 (GObject *, gs_local_obj_unref, g_object_unref)

/**
 * gs_unref_variant:
 *
 * Call g_variant_unref() on a variable location when it goes out of
 * scope.  Note that unlike g_variant_unref(), the variable may be
 * %NULL.
 */
#define gs_unref_variant nm_auto(gs_local_variant_unref)
NM_AUTO_DEFINE_FCN0 (GVariant *, gs_local_variant_unref, g_variant_unref)

/**
 * gs_unref_array:
 *
 * Call g_array_unref() on a variable location when it goes out of
 * scope.  Note that unlike g_array_unref(), the variable may be
 * %NULL.

 */
#define gs_unref_array nm_auto(gs_local_array_unref)
NM_AUTO_DEFINE_FCN0 (GArray *, gs_local_array_unref, g_array_unref)

/**
 * gs_unref_ptrarray:
 *
 * Call g_ptr_array_unref() on a variable location when it goes out of
 * scope.  Note that unlike g_ptr_array_unref(), the variable may be
 * %NULL.

 */
#define gs_unref_ptrarray nm_auto(gs_local_ptrarray_unref)
NM_AUTO_DEFINE_FCN0 (GPtrArray *, gs_local_ptrarray_unref, g_ptr_array_unref)

/**
 * gs_unref_hashtable:
 *
 * Call g_hash_table_unref() on a variable location when it goes out
 * of scope.  Note that unlike g_hash_table_unref(), the variable may
 * be %NULL.
 */
#define gs_unref_hashtable nm_auto(gs_local_hashtable_unref)
NM_AUTO_DEFINE_FCN0 (GHashTable *, gs_local_hashtable_unref, g_hash_table_unref)

/**
 * gs_free_slist:
 *
 * Call g_slist_free() on a variable location when it goes out
 * of scope.
 */
#define gs_free_slist nm_auto(gs_local_free_slist)
NM_AUTO_DEFINE_FCN0 (GSList *, gs_local_free_slist, g_slist_free)

/**
 * gs_unref_bytes:
 *
 * Call g_bytes_unref() on a variable location when it goes out
 * of scope.  Note that unlike g_bytes_unref(), the variable may
 * be %NULL.
 */
#define gs_unref_bytes nm_auto(gs_local_bytes_unref)
NM_AUTO_DEFINE_FCN0 (GBytes *, gs_local_bytes_unref, g_bytes_unref)

/**
 * gs_strfreev:
 *
 * Call g_strfreev() on a variable location when it goes out of scope.
 */
#define gs_strfreev nm_auto(gs_local_strfreev)
NM_AUTO_DEFINE_FCN0 (char **, gs_local_strfreev, g_strfreev)

/**
 * gs_free_error:
 *
 * Call g_error_free() on a variable location when it goes out of scope.
 */
#define gs_free_error nm_auto(gs_local_free_error)
NM_AUTO_DEFINE_FCN0 (GError *, gs_local_free_error, g_error_free)

/**
 * gs_unref_keyfile:
 *
 * Call g_key_file_unref() on a variable location when it goes out of scope.
 */
#define gs_unref_keyfile nm_auto(gs_local_keyfile_unref)
NM_AUTO_DEFINE_FCN0 (GKeyFile *, gs_local_keyfile_unref, g_key_file_unref)

/*****************************************************************************/

#include "nm-glib.h"

/*****************************************************************************/

#define nm_offsetofend(t,m) (G_STRUCT_OFFSET (t,m) + sizeof (((t *) NULL)->m))

/*****************************************************************************/

static inline int nm_close (int fd);

/**
 * nm_auto_free:
 *
 * Call free() on a variable location when it goes out of scope.
 * This is for pointers that are allocated with malloc() instead of
 * g_malloc().
 *
 * In practice, since glib 2.45, g_malloc()/g_free() always wraps malloc()/free().
 * See bgo#751592. In that case, it would be safe to free pointers allocated with
 * malloc() with gs_free or g_free().
 *
 * However, let's never mix them. To free malloc'ed memory, always use
 * free() or nm_auto_free.
 */
NM_AUTO_DEFINE_FCN_VOID0 (void *, _nm_auto_free_impl, free)
#define nm_auto_free nm_auto(_nm_auto_free_impl)

NM_AUTO_DEFINE_FCN0 (GVariantIter *, _nm_auto_free_variant_iter, g_variant_iter_free)
#define nm_auto_free_variant_iter nm_auto(_nm_auto_free_variant_iter)

NM_AUTO_DEFINE_FCN0 (GVariantBuilder *, _nm_auto_unref_variant_builder, g_variant_builder_unref)
#define nm_auto_unref_variant_builder nm_auto(_nm_auto_unref_variant_builder)

#define nm_auto_clear_variant_builder nm_auto(g_variant_builder_clear)

NM_AUTO_DEFINE_FCN0 (GList *, _nm_auto_free_list, g_list_free)
#define nm_auto_free_list nm_auto(_nm_auto_free_list)

NM_AUTO_DEFINE_FCN0 (GChecksum *, _nm_auto_checksum_free, g_checksum_free)
#define nm_auto_free_checksum nm_auto(_nm_auto_checksum_free)

#define nm_auto_unset_gvalue nm_auto(g_value_unset)

NM_AUTO_DEFINE_FCN_VOID0 (void *, _nm_auto_unref_gtypeclass, g_type_class_unref)
#define nm_auto_unref_gtypeclass nm_auto(_nm_auto_unref_gtypeclass)

NM_AUTO_DEFINE_FCN0 (GByteArray *, _nm_auto_unref_bytearray, g_byte_array_unref)
#define nm_auto_unref_bytearray nm_auto(_nm_auto_unref_bytearray)

static inline void
_nm_auto_free_gstring (GString **str)
{
	if (*str)
		g_string_free (*str, TRUE);
}
#define nm_auto_free_gstring nm_auto(_nm_auto_free_gstring)

static inline void
_nm_auto_close (int *pfd)
{
	if (*pfd >= 0) {
		int errsv = errno;

		(void) nm_close (*pfd);
		errno = errsv;
	}
}
#define nm_auto_close nm_auto(_nm_auto_close)

static inline void
_nm_auto_fclose (FILE **pfd)
{
	if (*pfd) {
		int errsv = errno;

		(void) fclose (*pfd);
		errno = errsv;
	}
}
#define nm_auto_fclose nm_auto(_nm_auto_fclose)

static inline void
_nm_auto_protect_errno (int *p_saved_errno)
{
	errno = *p_saved_errno;
}
#define NM_AUTO_PROTECT_ERRNO(errsv_saved) nm_auto(_nm_auto_protect_errno) _nm_unused const int errsv_saved = (errno)

NM_AUTO_DEFINE_FCN0 (GSource *, _nm_auto_unref_gsource, g_source_unref);
#define nm_auto_unref_gsource nm_auto(_nm_auto_unref_gsource)

NM_AUTO_DEFINE_FCN0 (guint, _nm_auto_remove_source, g_source_remove);
#define nm_auto_remove_source nm_auto(_nm_auto_remove_source)

NM_AUTO_DEFINE_FCN0 (GIOChannel *, _nm_auto_unref_io_channel, g_io_channel_unref)
#define nm_auto_unref_io_channel nm_auto(_nm_auto_unref_io_channel)

NM_AUTO_DEFINE_FCN0 (GMainLoop *, _nm_auto_unref_gmainloop, g_main_loop_unref);
#define nm_auto_unref_gmainloop nm_auto(_nm_auto_unref_gmainloop)

static inline void
_nm_auto_freev (gpointer ptr)
{
	gpointer **p = ptr;
	gpointer *_ptr;

	if (*p) {
		for (_ptr = *p; *_ptr; _ptr++)
			g_free (*_ptr);
		g_free (*p);
	}
}
/* g_free a NULL terminated array of pointers, with also freeing each
 * pointer with g_free(). It essentially does the same as
 * gs_strfreev / g_strfreev(), but not restricted to strv arrays. */
#define nm_auto_freev nm_auto(_nm_auto_freev)

/*****************************************************************************/

/* http://stackoverflow.com/a/11172679 */
#define  _NM_UTILS_MACRO_FIRST(...)                           __NM_UTILS_MACRO_FIRST_HELPER(__VA_ARGS__, throwaway)
#define __NM_UTILS_MACRO_FIRST_HELPER(first, ...)             first

#define  _NM_UTILS_MACRO_REST(...)                            __NM_UTILS_MACRO_REST_HELPER(__NM_UTILS_MACRO_REST_NUM(__VA_ARGS__), __VA_ARGS__)
#define __NM_UTILS_MACRO_REST_HELPER(qty, ...)                __NM_UTILS_MACRO_REST_HELPER2(qty, __VA_ARGS__)
#define __NM_UTILS_MACRO_REST_HELPER2(qty, ...)               __NM_UTILS_MACRO_REST_HELPER_##qty(__VA_ARGS__)
#define __NM_UTILS_MACRO_REST_HELPER_ONE(first)
#define __NM_UTILS_MACRO_REST_HELPER_TWOORMORE(first, ...)    , __VA_ARGS__
#define __NM_UTILS_MACRO_REST_NUM(...) \
    __NM_UTILS_MACRO_REST_SELECT_30TH(__VA_ARGS__, \
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
                TWOORMORE, TWOORMORE, TWOORMORE, ONE, throwaway)
#define __NM_UTILS_MACRO_REST_SELECT_30TH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, ...) a30

/*****************************************************************************/

/* http://stackoverflow.com/a/2124385/354393
 * https://stackoverflow.com/questions/11317474/macro-to-count-number-of-arguments
 */

#define NM_NARG(...) \
         _NM_NARG(, ##__VA_ARGS__, _NM_NARG_RSEQ_N())
#define _NM_NARG(...) \
         _NM_NARG_ARG_N(__VA_ARGS__)
#define _NM_NARG_ARG_N( \
          _0, \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define _NM_NARG_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

/*****************************************************************************/

#if defined (__GNUC__)
#define _NM_PRAGMA_WARNING_DO(warning)       G_STRINGIFY(GCC diagnostic ignored warning)
#elif defined (__clang__)
#define _NM_PRAGMA_WARNING_DO(warning)       G_STRINGIFY(clang diagnostic ignored warning)
#endif

/* you can only suppress a specific warning that the compiler
 * understands. Otherwise you will get another compiler warning
 * about invalid pragma option.
 * It's not that bad however, because gcc and clang often have the
 * same name for the same warning. */

#if defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#define NM_PRAGMA_WARNING_DISABLE(warning) \
        _Pragma("GCC diagnostic push") \
        _Pragma(_NM_PRAGMA_WARNING_DO(warning))
#elif defined (__clang__)
#define NM_PRAGMA_WARNING_DISABLE(warning) \
        _Pragma("clang diagnostic push") \
        _Pragma(_NM_PRAGMA_WARNING_DO("-Wunknown-warning-option")) \
        _Pragma(_NM_PRAGMA_WARNING_DO(warning))
#else
#define NM_PRAGMA_WARNING_DISABLE(warning)
#endif

#if defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#define NM_PRAGMA_WARNING_REENABLE \
    _Pragma("GCC diagnostic pop")
#elif defined (__clang__)
#define NM_PRAGMA_WARNING_REENABLE \
    _Pragma("clang diagnostic pop")
#else
#define NM_PRAGMA_WARNING_REENABLE
#endif

/*****************************************************************************/

/**
 * NM_G_ERROR_MSG:
 * @error: (allow-none): the #GError instance
 *
 * All functions must follow the convention that when they
 * return a failure, they must also set the GError to a valid
 * message. For external API however, we want to be extra
 * careful before accessing the error instance. Use NM_G_ERROR_MSG()
 * which is safe to use on NULL.
 *
 * Returns: the error message.
 **/
static inline const char *
NM_G_ERROR_MSG (GError *error)
{
	return error ? (error->message ?: "(null)") : "(no-error)"; \
}

/*****************************************************************************/

/* macro to return strlen() of a compile time string. */
#define NM_STRLEN(str)     ( sizeof (""str"") - 1 )

/* returns the length of a NULL terminated array of pointers,
 * like g_strv_length() does. The difference is:
 *  - it operats on arrays of pointers (of any kind, requiring no cast).
 *  - it accepts NULL to return zero. */
#define NM_PTRARRAY_LEN(array) \
	({ \
		typeof (*(array)) *const _array = (array); \
		gsize _n = 0; \
		\
		if (_array) { \
			_nm_unused gconstpointer _type_check_is_pointer = _array[0]; \
			\
			while (_array[_n]) \
				_n++; \
		} \
		_n; \
	})

/* Note: @value is only evaluated when *out_val is present.
 * Thus,
 *    NM_SET_OUT (out_str, g_strdup ("hallo"));
 * does the right thing.
 */
#define NM_SET_OUT(out_val, value) \
	G_STMT_START { \
		typeof(*(out_val)) *_out_val = (out_val); \
		\
		if (_out_val) { \
			*_out_val = (value); \
		} \
	} G_STMT_END

/*****************************************************************************/

#ifndef _NM_CC_SUPPORT_AUTO_TYPE
#if (defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9 )))
#define _NM_CC_SUPPORT_AUTO_TYPE 1
#else
#define _NM_CC_SUPPORT_AUTO_TYPE 0
#endif
#endif

#ifndef _NM_CC_SUPPORT_GENERIC
/* In the meantime, NetworkManager requires C11 and _Generic() should always be available.
 * However, shared/nm-utils may also be used in VPN/applet, which possibly did not yet
 * bump the C standard requirement. Leave this for the moment, but eventually we can
 * drop it. */
#if (defined (__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9 ))) || (defined (__clang__))
#define _NM_CC_SUPPORT_GENERIC 1
#else
#define _NM_CC_SUPPORT_GENERIC 0
#endif
#endif

#if _NM_CC_SUPPORT_AUTO_TYPE
#define _nm_auto_type __auto_type
#endif

#if _NM_CC_SUPPORT_GENERIC
#define _NM_CONSTCAST_FULL_1(type, obj_expr, obj) \
	(_Generic ((obj_expr), \
	           const void        *const: ((const type *) (obj)), \
	           const void        *     : ((const type *) (obj)), \
	                 void        *const: ((      type *) (obj)), \
	                 void        *     : ((      type *) (obj)), \
	           const type        *const: ((const type *) (obj)), \
	           const type        *     : ((const type *) (obj)), \
	                 type        *const: ((      type *) (obj)), \
	                 type        *     : ((      type *) (obj))))
#define _NM_CONSTCAST_FULL_2(type, obj_expr, obj, alias_type2) \
	(_Generic ((obj_expr), \
	           const void        *const: ((const type *) (obj)), \
	           const void        *     : ((const type *) (obj)), \
	                 void        *const: ((      type *) (obj)), \
	                 void        *     : ((      type *) (obj)), \
	           const alias_type2 *const: ((const type *) (obj)), \
	           const alias_type2 *     : ((const type *) (obj)), \
	                 alias_type2 *const: ((      type *) (obj)), \
	                 alias_type2 *     : ((      type *) (obj)), \
	           const type        *const: ((const type *) (obj)), \
	           const type        *     : ((const type *) (obj)), \
	                 type        *const: ((      type *) (obj)), \
	                 type        *     : ((      type *) (obj))))
#define _NM_CONSTCAST_FULL_3(type, obj_expr, obj, alias_type2, alias_type3) \
	(_Generic ((obj_expr), \
	           const void        *const: ((const type *) (obj)), \
	           const void        *     : ((const type *) (obj)), \
	                 void        *const: ((      type *) (obj)), \
	                 void        *     : ((      type *) (obj)), \
	           const alias_type2 *const: ((const type *) (obj)), \
	           const alias_type2 *     : ((const type *) (obj)), \
	                 alias_type2 *const: ((      type *) (obj)), \
	                 alias_type2 *     : ((      type *) (obj)), \
	           const alias_type3 *const: ((const type *) (obj)), \
	           const alias_type3 *     : ((const type *) (obj)), \
	                 alias_type3 *const: ((      type *) (obj)), \
	                 alias_type3 *     : ((      type *) (obj)), \
	           const type        *const: ((const type *) (obj)), \
	           const type        *     : ((const type *) (obj)), \
	                 type        *const: ((      type *) (obj)), \
	                 type        *     : ((      type *) (obj))))
#define _NM_CONSTCAST_FULL_4(type, obj_expr, obj, alias_type2, alias_type3, alias_type4) \
	(_Generic ((obj_expr), \
	           const void        *const: ((const type *) (obj)), \
	           const void        *     : ((const type *) (obj)), \
	                 void        *const: ((      type *) (obj)), \
	                 void        *     : ((      type *) (obj)), \
	           const alias_type2 *const: ((const type *) (obj)), \
	           const alias_type2 *     : ((const type *) (obj)), \
	                 alias_type2 *const: ((      type *) (obj)), \
	                 alias_type2 *     : ((      type *) (obj)), \
	           const alias_type3 *const: ((const type *) (obj)), \
	           const alias_type3 *     : ((const type *) (obj)), \
	                 alias_type3 *const: ((      type *) (obj)), \
	                 alias_type3 *     : ((      type *) (obj)), \
	           const alias_type4 *const: ((const type *) (obj)), \
	           const alias_type4 *     : ((const type *) (obj)), \
	                 alias_type4 *const: ((      type *) (obj)), \
	                 alias_type4 *     : ((      type *) (obj)), \
	           const type        *const: ((const type *) (obj)), \
	           const type        *     : ((const type *) (obj)), \
	                 type        *const: ((      type *) (obj)), \
	                 type        *     : ((      type *) (obj))))
#define _NM_CONSTCAST_FULL_x(type, obj_expr, obj, n, ...)   (_NM_CONSTCAST_FULL_##n (type, obj_expr, obj,                        ##__VA_ARGS__))
#define _NM_CONSTCAST_FULL_y(type, obj_expr, obj, n, ...)   (_NM_CONSTCAST_FULL_x   (type, obj_expr, obj, n,                     ##__VA_ARGS__))
#define NM_CONSTCAST_FULL(   type, obj_expr, obj,    ...)   (_NM_CONSTCAST_FULL_y   (type, obj_expr, obj, NM_NARG (dummy, ##__VA_ARGS__), ##__VA_ARGS__))
#else
#define NM_CONSTCAST_FULL(   type, obj_expr, obj,    ...)   ((type *) (obj))
#endif

#define NM_CONSTCAST(type, obj, ...) \
	NM_CONSTCAST_FULL(type, (obj), (obj), ##__VA_ARGS__)

#if _NM_CC_SUPPORT_GENERIC
#define NM_UNCONST_PTR(type, arg) \
	_Generic ((arg), \
	          const type *: ((type *) (arg)), \
	                type *: ((type *) (arg)))
#else
#define NM_UNCONST_PTR(type, arg) \
	((type *) (arg))
#endif

#if _NM_CC_SUPPORT_GENERIC
#define NM_UNCONST_PPTR(type, arg) \
	_Generic ((arg), \
	          const type *     *: ((type **) (arg)), \
	                type *     *: ((type **) (arg)), \
	          const type *const*: ((type **) (arg)), \
	                type *const*: ((type **) (arg)))
#else
#define NM_UNCONST_PPTR(type, arg) \
	((type **) (arg))
#endif

#define NM_GOBJECT_CAST(type, obj, is_check, ...) \
	({ \
		const void *_obj = (obj); \
		\
		nm_assert (_obj || (is_check (_obj))); \
		NM_CONSTCAST_FULL (type, (obj), _obj, GObject, ##__VA_ARGS__); \
	})

#define NM_GOBJECT_CAST_NON_NULL(type, obj, is_check, ...) \
	({ \
		const void *_obj = (obj); \
		\
		nm_assert (is_check (_obj)); \
		NM_CONSTCAST_FULL (type, (obj), _obj, GObject, ##__VA_ARGS__); \
	})

#if _NM_CC_SUPPORT_GENERIC
/* returns @value, if the type of @value matches @type.
 * This requires support for C11 _Generic(). If no support is
 * present, this returns @value directly.
 *
 * It's useful to check the let the compiler ensure that @value is
 * of a certain type. */
#define _NM_ENSURE_TYPE(type, value) (_Generic ((value), type: (value)))
#define _NM_ENSURE_TYPE_CONST(type, value) (_Generic ((value), \
                                                      const type      : ((const type) (value)), \
                                                      const type const: ((const type) (value)), \
                                                            type      : ((const type) (value)), \
                                                            type const: ((const type) (value))))
#else
#define _NM_ENSURE_TYPE(type, value) (value)
#define _NM_ENSURE_TYPE_CONST(type, value) ((const type) (value))
#endif

#if _NM_CC_SUPPORT_GENERIC
/* these macros cast (value) to
 *  - "const char **"      (for "MC", mutable-const)
 *  - "const char *const*" (for "CC", const-const)
 * The point is to do this cast, but only accepting pointers
 * that are compatible already.
 *
 * The problem is, if you add a function like g_strdupv(), the input
 * argument is not modified (CC), but you want to make it work also
 * for "char **". C doesn't allow this form of casting (for good reasons),
 * so the function makes a choice like g_strdupv(char**). That means,
 * every time you want to call it with a const argument, you need to
 * explicitly cast it.
 *
 * These macros do the cast, but they only accept a compatible input
 * type, otherwise they will fail compilation.
 */
#define NM_CAST_STRV_MC(value) \
	(_Generic ((value), \
	           const char *     *: (const char *     *) (value), \
	                 char *     *: (const char *     *) (value), \
	                       void *: (const char *     *) (value)))
#define NM_CAST_STRV_CC(value) \
	(_Generic ((value), \
	           const char *const*: (const char *const*) (value), \
	           const char *     *: (const char *const*) (value), \
	                 char *const*: (const char *const*) (value), \
	                 char *     *: (const char *const*) (value), \
	                 const void *: (const char *const*) (value), \
	                       void *: (const char *const*) (value)))
#else
#define NM_CAST_STRV_MC(value) ((const char *     *) (value))
#define NM_CAST_STRV_CC(value) ((const char *const*) (value))
#endif

#if _NM_CC_SUPPORT_GENERIC
#define NM_PROPAGATE_CONST(test_expr, ptr) \
	(_Generic ((test_expr), \
	           const typeof (*(test_expr)) *: ((const typeof (*(ptr)) *) (ptr)), \
	                                 default: (_Generic ((test_expr), \
	                                                     typeof (*(test_expr)) *: (ptr)))))
#else
#define NM_PROPAGATE_CONST(test_expr, ptr) (ptr)
#endif

/* with the way it is implemented, the caller may or may not pass a trailing
 * ',' and it will work. However, this makes the macro unsuitable for initializing
 * an array. */
#define NM_MAKE_STRV(...) \
	((const char *const[(sizeof (((const char *const[]) { __VA_ARGS__ })) / sizeof (const char *)) + 1]) { __VA_ARGS__ })

/*****************************************************************************/

#define _NM_IN_SET_EVAL_1( op, _x, y)           (_x == (y))
#define _NM_IN_SET_EVAL_2( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_1  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_3( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_2  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_4( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_3  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_5( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_4  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_6( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_5  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_7( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_6  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_8( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_7  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_9( op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_8  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_10(op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_9  (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_11(op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_10 (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_12(op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_11 (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_13(op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_12 (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_14(op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_13 (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_15(op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_14 (op, _x, __VA_ARGS__)
#define _NM_IN_SET_EVAL_16(op, _x, y, ...)      (_x == (y)) op _NM_IN_SET_EVAL_15 (op, _x, __VA_ARGS__)

#define _NM_IN_SET_EVAL_N2(op, _x, n, ...)      (_NM_IN_SET_EVAL_##n(op, _x, __VA_ARGS__))
#define _NM_IN_SET_EVAL_N(op, type, x, n, ...)                      \
    ({                                                              \
        type _x = (x);                                              \
                                                                    \
        /* trigger a -Wenum-compare warning */                      \
        nm_assert (TRUE || _x == (x));                              \
                                                                    \
        !!_NM_IN_SET_EVAL_N2(op, _x, n, __VA_ARGS__);               \
    })

#define _NM_IN_SET(op, type, x, ...)        _NM_IN_SET_EVAL_N(op, type, x, NM_NARG (__VA_ARGS__), __VA_ARGS__)

/* Beware that this does short-circuit evaluation (use "||" instead of "|")
 * which has a possibly unexpected non-function-like behavior.
 * Use NM_IN_SET_SE if you need all arguments to be evaluated. */
#define NM_IN_SET(x, ...)                   _NM_IN_SET(||, typeof (x), x, __VA_ARGS__)

/* "SE" stands for "side-effect". Contrary to NM_IN_SET(), this does not do
 * short-circuit evaluation, which can make a difference if the arguments have
 * side-effects. */
#define NM_IN_SET_SE(x, ...)                _NM_IN_SET(|,  typeof (x), x, __VA_ARGS__)

/* the *_TYPED forms allow to explicitly select the type of "x". This is useful
 * if "x" doesn't support typeof (bitfields) or you want to gracefully convert
 * a type using automatic type conversion rules (but not forcing the conversion
 * with a cast). */
#define NM_IN_SET_TYPED(type, x, ...)       _NM_IN_SET(||, type,       x, __VA_ARGS__)
#define NM_IN_SET_SE_TYPED(type, x, ...)    _NM_IN_SET(|,  type,       x, __VA_ARGS__)

/*****************************************************************************/

static inline gboolean
_NM_IN_STRSET_streq (const char *x, const char *s)
{
	return s && strcmp (x, s) == 0;
}

#define _NM_IN_STRSET_EVAL_1( op, _x, y)        _NM_IN_STRSET_streq (_x, y)
#define _NM_IN_STRSET_EVAL_2( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_1  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_3( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_2  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_4( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_3  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_5( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_4  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_6( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_5  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_7( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_6  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_8( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_7  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_9( op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_8  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_10(op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_9  (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_11(op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_10 (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_12(op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_11 (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_13(op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_12 (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_14(op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_13 (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_15(op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_14 (op, _x, __VA_ARGS__)
#define _NM_IN_STRSET_EVAL_16(op, _x, y, ...)   _NM_IN_STRSET_streq (_x, y) op _NM_IN_STRSET_EVAL_15 (op, _x, __VA_ARGS__)

#define _NM_IN_STRSET_EVAL_N2(op, _x, n, ...)   (_NM_IN_STRSET_EVAL_##n(op, _x, __VA_ARGS__))
#define _NM_IN_STRSET_EVAL_N(op, x, n, ...)                       \
    ({                                                            \
        const char *_x = (x);                                     \
        (   ((_x == NULL) && _NM_IN_SET_EVAL_N2    (op, ((const char *) NULL), n, __VA_ARGS__)) \
         || ((_x != NULL) && _NM_IN_STRSET_EVAL_N2 (op, _x,                    n, __VA_ARGS__)) \
        ); \
    })

/* Beware that this does short-circuit evaluation (use "||" instead of "|")
 * which has a possibly unexpected non-function-like behavior.
 * Use NM_IN_STRSET_SE if you need all arguments to be evaluated. */
#define NM_IN_STRSET(x, ...)               _NM_IN_STRSET_EVAL_N(||, x, NM_NARG (__VA_ARGS__), __VA_ARGS__)

/* "SE" stands for "side-effect". Contrary to NM_IN_STRSET(), this does not do
 * short-circuit evaluation, which can make a difference if the arguments have
 * side-effects. */
#define NM_IN_STRSET_SE(x, ...)            _NM_IN_STRSET_EVAL_N(|, x, NM_NARG (__VA_ARGS__), __VA_ARGS__)

#define NM_STRCHAR_ALL(str, ch_iter, predicate) \
	({ \
		gboolean _val = TRUE; \
		const char *_str = (str); \
		\
		if (_str) { \
			for (;;) { \
				const char ch_iter = _str[0]; \
				\
				if (ch_iter != '\0') { \
					if (predicate) {\
						_str++; \
						continue; \
					} \
					_val = FALSE; \
				} \
				break; \
			} \
		} \
		_val; \
	})

#define NM_STRCHAR_ANY(str, ch_iter, predicate) \
	({ \
		gboolean _val = FALSE; \
		const char *_str = (str); \
		\
		if (_str) { \
			for (;;) { \
				const char ch_iter = _str[0]; \
				\
				if (ch_iter != '\0') { \
					if (predicate) { \
						; \
					} else { \
						_str++; \
						continue; \
					} \
					_val = TRUE; \
				} \
				break; \
			} \
		} \
		_val; \
	})

/*****************************************************************************/

/* NM_CACHED_QUARK() returns the GQuark for @string, but caches
 * it in a static variable to speed up future lookups.
 *
 * @string must be a string literal.
 */
#define NM_CACHED_QUARK(string) \
	({ \
		static GQuark _nm_cached_quark = 0; \
		\
		(G_LIKELY (_nm_cached_quark != 0) \
			? _nm_cached_quark \
			: (_nm_cached_quark = g_quark_from_static_string (""string""))); \
	})

/* NM_CACHED_QUARK_FCN() is essentially the same as G_DEFINE_QUARK
 * with two differences:
 * - @string must be a quoted string-literal
 * - @fcn must be the full function name, while G_DEFINE_QUARK() appends
 *   "_quark" to the function name.
 * Both properties of G_DEFINE_QUARK() are non favorable, because you can no
 * longer grep for string/fcn -- unless you are aware that you are searching
 * for G_DEFINE_QUARK() and omit quotes / append _quark(). With NM_CACHED_QUARK_FCN(),
 * ctags/cscope can locate the use of @fcn (though it doesn't recognize that
 * NM_CACHED_QUARK_FCN() defines it).
 */
#define NM_CACHED_QUARK_FCN(string, fcn) \
GQuark \
fcn (void) \
{ \
	return NM_CACHED_QUARK (string); \
}

/*****************************************************************************/

static inline int
nm_strcmp0 (const char *s1, const char *s2)
{
	int c;

	/* like g_strcmp0(), but this is inlinable.
	 *
	 * Also, it is guaranteed to return either -1, 0, or 1. */
	if (s1 == s2)
		return 0;
	if (!s1)
		return -1;
	if (!s2)
		return 1;
	c = strcmp (s1, s2);
	if (c < 0)
		return -1;
	if (c > 0)
		return 1;
	return 0;
}

static inline gboolean
nm_streq (const char *s1, const char *s2)
{
	return strcmp (s1, s2) == 0;
}

static inline gboolean
nm_streq0 (const char *s1, const char *s2)
{
	return    (s1 == s2)
	       || (s1 && s2 && strcmp (s1, s2) == 0);
}

#define NM_STR_HAS_PREFIX(str, prefix) \
	({ \
		const char *const _str = (str); \
		\
		_str && (strncmp ((str), ""prefix"", NM_STRLEN (prefix)) == 0); \
	})

#define NM_STR_HAS_SUFFIX(str, suffix) \
	({ \
		const char *_str; \
		gsize _l; \
		\
		(   (_str = (str)) \
		 && ((_l = strlen (_str)) >= NM_STRLEN (suffix)) \
		 && (memcmp (&_str[_l - NM_STRLEN (suffix)], \
		             ""suffix"", \
		             NM_STRLEN (suffix)) == 0)); \
	})

/*****************************************************************************/

static inline GString *
nm_gstring_prepare (GString **l)
{
	if (*l)
		g_string_set_size (*l, 0);
	else
		*l = g_string_sized_new (30);
	return *l;
}

static inline GString *
nm_gstring_add_space_delimiter (GString *str)
{
	if (str->len > 0)
		g_string_append_c (str, ' ');
	return str;
}

static inline const char *
nm_str_not_empty (const char *str)
{
	return str && str[0] ? str : NULL;
}

static inline char *
nm_strdup_not_empty (const char *str)
{
	return str && str[0] ? g_strdup (str) : NULL;
}

static inline char *
nm_str_realloc (char *str)
{
	gs_free char *s = str;

	/* Returns a new clone of @str and frees @str. The point is that @str
	 * possibly points to a larger chunck of memory. We want to freshly allocate
	 * a buffer.
	 *
	 * We could use realloc(), but that might not do anything or leave
	 * @str in its memory pool for chunks of a different size (bad for
	 * fragmentation).
	 *
	 * This is only useful when we want to keep the buffer around for a long
	 * time and want to re-allocate a more optimal buffer. */

	return g_strdup (s);
}

/*****************************************************************************/

#define NM_PRINT_FMT_QUOTED(cond, prefix, str, suffix, str_else) \
	(cond) ? (prefix) : "", \
	(cond) ? (str) : (str_else), \
	(cond) ? (suffix) : ""
#define NM_PRINT_FMT_QUOTE_STRING(arg) NM_PRINT_FMT_QUOTED((arg), "\"", (arg), "\"", "(null)")

/*****************************************************************************/

/* glib/C provides the following kind of assertions:
 *   - assert() -- disable with NDEBUG
 *   - g_return_if_fail() -- disable with G_DISABLE_CHECKS
 *   - g_assert() -- disable with G_DISABLE_ASSERT
 * but they are all enabled by default and usually even production builds have
 * these kind of assertions enabled. It also means, that disabling assertions
 * is an untested configuration, and might have bugs.
 *
 * Add our own assertion macro nm_assert(), which is disabled by default and must
 * be explicitly enabled. They are useful for more expensive checks or checks that
 * depend less on runtime conditions (that is, are generally expected to be true). */

#ifndef NM_MORE_ASSERTS
#define NM_MORE_ASSERTS 0
#endif

#if NM_MORE_ASSERTS
#define nm_assert(cond) G_STMT_START { g_assert (cond); } G_STMT_END
#define nm_assert_se(cond) G_STMT_START { if (G_LIKELY (cond)) { ; } else { g_assert (FALSE && (cond)); } } G_STMT_END
#define nm_assert_not_reached() G_STMT_START { g_assert_not_reached (); } G_STMT_END
#else
#define nm_assert(cond) G_STMT_START { if (FALSE) { if (cond) { } } } G_STMT_END
#define nm_assert_se(cond) G_STMT_START { if (G_LIKELY (cond)) { ; } } G_STMT_END
#define nm_assert_not_reached() G_STMT_START { ; } G_STMT_END
#endif

/*****************************************************************************/

#define NM_GOBJECT_PROPERTIES_DEFINE_BASE(...) \
typedef enum { \
	PROP_0, \
	__VA_ARGS__ \
	_PROPERTY_ENUMS_LAST, \
} _PropertyEnums; \
static GParamSpec *obj_properties[_PROPERTY_ENUMS_LAST] = { NULL, }

#define NM_GOBJECT_PROPERTIES_DEFINE_NOTIFY(obj_type, obj_properties, property_enums_type, prop_0) \
static inline void \
_nm_gobject_notify_together_impl (obj_type *obj, guint n, const property_enums_type *props) \
{ \
	const gboolean freeze_thaw = (n > 1); \
	\
	nm_assert (G_IS_OBJECT (obj)); \
	nm_assert (n > 0); \
	\
	if (freeze_thaw) \
		g_object_freeze_notify ((GObject *) obj); \
	while (n-- > 0) { \
		const property_enums_type prop = *props++; \
		\
		if (prop != prop_0) { \
			nm_assert ((gsize) prop < G_N_ELEMENTS (obj_properties)); \
			nm_assert (obj_properties[prop]); \
			g_object_notify_by_pspec ((GObject *) obj, obj_properties[prop]); \
		} \
	} \
	if (freeze_thaw) \
		g_object_thaw_notify ((GObject *) obj); \
} \
\
static inline void \
_notify (obj_type *obj, property_enums_type prop) \
{ \
	_nm_gobject_notify_together_impl (obj, 1, &prop); \
} \

#define NM_GOBJECT_PROPERTIES_DEFINE(obj_type, ...) \
NM_GOBJECT_PROPERTIES_DEFINE_BASE (__VA_ARGS__); \
NM_GOBJECT_PROPERTIES_DEFINE_NOTIFY (obj_type, obj_properties, _PropertyEnums, PROP_0)

/* invokes _notify() for all arguments (of type _PropertyEnums). Note, that if
 * there are more than one prop arguments, this will involve a freeze/thaw
 * of GObject property notifications. */
#define nm_gobject_notify_together(obj, ...) \
	_nm_gobject_notify_together_impl (obj, NM_NARG (__VA_ARGS__), (const _PropertyEnums[]) { __VA_ARGS__ })

/*****************************************************************************/

#define _NM_GET_PRIVATE(self, type, is_check, ...) (&(NM_GOBJECT_CAST_NON_NULL (type, (self), is_check, ##__VA_ARGS__)->_priv))
#if _NM_CC_SUPPORT_AUTO_TYPE
#define _NM_GET_PRIVATE_PTR(self, type, is_check, ...) \
	({ \
		_nm_auto_type _self = NM_GOBJECT_CAST_NON_NULL (type, (self), is_check, ##__VA_ARGS__); \
		\
		NM_PROPAGATE_CONST (_self, _self->_priv); \
	})
#else
#define _NM_GET_PRIVATE_PTR(self, type, is_check, ...) (NM_GOBJECT_CAST_NON_NULL (type, (self), is_check, ##__VA_ARGS__)->_priv)
#endif

/*****************************************************************************/

static inline gpointer
nm_g_object_ref (gpointer obj)
{
	/* g_object_ref() doesn't accept NULL. */
	if (obj)
		g_object_ref (obj);
	return obj;
}
#define nm_g_object_ref(obj) ((typeof (obj)) nm_g_object_ref (obj))

static inline void
nm_g_object_unref (gpointer obj)
{
	/* g_object_unref() doesn't accept NULL. Usully, we workaround that
	 * by using g_clear_object(), but sometimes that is not convenient
	 * (for example as as destroy function for a hash table that can contain
	 * NULL values). */
	if (obj)
		g_object_unref (obj);
}

/* Assigns GObject @obj to destination @pp, and takes an additional ref.
 * The previous value of @pp is unrefed.
 *
 * It makes sure to first increase the ref-count of @obj, and handles %NULL
 * @obj correctly.
 * */
#define nm_g_object_ref_set(pp, obj) \
	({ \
		typeof (*(pp)) *const _pp = (pp); \
		typeof (*_pp) const _obj = (obj); \
		typeof (*_pp) _p; \
		gboolean _changed = FALSE; \
		\
		nm_assert (!_pp || !*_pp || G_IS_OBJECT (*_pp)); \
		nm_assert (!_obj || G_IS_OBJECT (_obj)); \
		\
		if (   _pp \
		    && ((_p = *_pp) != _obj)) { \
			nm_g_object_ref (_obj); \
			*_pp = _obj; \
			nm_g_object_unref (_p); \
			_changed = TRUE; \
		} \
		_changed; \
	})

#define nm_clear_pointer(pp, destroy) \
	({ \
		typeof (*(pp)) *_pp = (pp); \
		typeof (*_pp) _p; \
		gboolean _changed = FALSE; \
		\
		if (   _pp \
		    && (_p = *_pp)) { \
			_nm_unused gconstpointer _p_check_is_pointer = _p; \
			\
			*_pp = NULL; \
			/* g_clear_pointer() assigns @destroy first to a local variable, so that
			 * you can call "g_clear_pointer (pp, (GDestroyNotify) destroy);" without
			 * gcc emitting a warning. We don't do that, hence, you cannot cast
			 * "destroy" first.
			 *
			 * On the upside: you are not supposed to cast fcn, because the pointer
			 * types are preserved. If you really need a cast, you should cast @pp.
			 * But that is hardly ever necessary. */ \
			(destroy) (_p); \
			\
			_changed = TRUE; \
		} \
		_changed; \
	})

/* basically, replaces
 *   g_clear_pointer (&location, g_free)
 * with
 *   nm_clear_g_free (&location)
 *
 * Another advantage is that by using a macro and typeof(), it is more
 * typesafe and gives you for example a compiler warning when pp is a const
 * pointer or points to a const-pointer.
 */
#define nm_clear_g_free(pp) \
	nm_clear_pointer (pp, g_free)

#define nm_clear_g_object(pp) \
	nm_clear_pointer (pp, g_object_unref)

/**
 * nm_clear_error:
 * @err: a pointer to pointer to a #GError.
 *
 * This is like g_clear_error(). The only difference is
 * that this is an inline function.
 */
static inline void
nm_clear_error (GError **err)
{
	if (err && *err) {
		g_error_free (*err);
		*err = NULL;
	}
}

/* Patch g_clear_error() to use nm_clear_error(), which is inlineable
 * and visible to the compiler. For example gs_free_error attribute only
 * frees the error after checking that it's not %NULL. So, in many cases
 * the compiler knows that gs_free_error has no effect and can optimize
 * the call away. By making g_clear_error() inlineable, we give the compiler
 * more chance to detect that the function actually has no effect. */
#define g_clear_error(ptr) nm_clear_error(ptr)

static inline gboolean
nm_clear_g_source (guint *id)
{
	guint v;

	if (   id
	    && (v = *id)) {
		*id = 0;
		g_source_remove (v);
		return TRUE;
	}
	return FALSE;
}

static inline gboolean
nm_clear_g_signal_handler (gpointer self, gulong *id)
{
	gulong v;

	if (   id
	    && (v = *id)) {
		*id = 0;
		g_signal_handler_disconnect (self, v);
		return TRUE;
	}
	return FALSE;
}

static inline gboolean
nm_clear_g_variant (GVariant **variant)
{
	GVariant *v;

	if (   variant
	    && (v = *variant)) {
		*variant = NULL;
		g_variant_unref (v);
		return TRUE;
	}
	return FALSE;
}

static inline gboolean
nm_clear_g_cancellable (GCancellable **cancellable)
{
	GCancellable *v;

	if (   cancellable
	    && (v = *cancellable)) {
		*cancellable = NULL;
		g_cancellable_cancel (v);
		g_object_unref (v);
		return TRUE;
	}
	return FALSE;
}

/* If @cancellable_id is not 0, clear it and call g_cancellable_disconnect().
 * @cancellable may be %NULL, if there is nothing to disconnect.
 *
 * It's like nm_clear_g_signal_handler(), except that it uses g_cancellable_disconnect()
 * instead of g_signal_handler_disconnect().
 *
 * Note the warning in glib documentation about dead-lock and what g_cancellable_disconnect()
 * actually does. */
static inline gboolean
nm_clear_g_cancellable_disconnect (GCancellable *cancellable, gulong *cancellable_id)
{
	gulong id;

	if (   cancellable_id
	    && (id = *cancellable_id) != 0) {
		*cancellable_id = 0;
		g_cancellable_disconnect (cancellable, id);
		return TRUE;
	}
	return FALSE;
}

/*****************************************************************************/

/* GVariantType is basically a C string. But G_VARIANT_TYPE() is not suitable
 * to initialize a static variable (because it evaluates a function check that
 * the string is valid). Add an alternative macro that does the plain cast.
 *
 * Here you loose the assertion check that G_VARIANT_TYPE() to ensure the
 * string is valid. */
#define NM_G_VARIANT_TYPE(fmt) ((const GVariantType *) (""fmt""))

static inline GVariant *
nm_g_variant_ref (GVariant *v)
{
	if (v)
		g_variant_ref (v);
	return v;
}

static inline GVariant *
nm_g_variant_ref_sink (GVariant *v)
{
	if (v)
		g_variant_ref_sink (v);
	return v;
}

static inline void
nm_g_variant_unref (GVariant *v)
{
	if (v)
		g_variant_unref (v);
}

static inline GVariant *
nm_g_variant_take_ref (GVariant *v)
{
	if (v)
		g_variant_take_ref (v);
	return v;
}

/*****************************************************************************/

/* Determine whether @x is a power of two (@x being an integer type).
 * Basically, this returns TRUE, if @x has exactly one bit set.
 * For negative values and zero, this always returns FALSE. */
#define nm_utils_is_power_of_two(x) ({ \
		typeof(x) __x = (x); \
		\
		(    (__x > ((typeof(__x)) 0)) \
		 && ((__x & (__x - (((typeof(__x)) 1)))) == ((typeof(__x)) 0))); \
	})

#define NM_DIV_ROUND_UP(x, y) \
	({ \
		const typeof(x) _x = (x); \
		const typeof(y) _y = (y); \
		\
		(_x / _y + !!(_x % _y)); \
	})

/*****************************************************************************/

#define NM_UTILS_LOOKUP_DEFAULT(v)            return (v)
#define NM_UTILS_LOOKUP_DEFAULT_WARN(v)       g_return_val_if_reached (v)
#define NM_UTILS_LOOKUP_DEFAULT_NM_ASSERT(v)  { nm_assert_not_reached (); return (v); }
#define NM_UTILS_LOOKUP_ITEM(v, n)            (void) 0; case v: return (n); (void) 0
#define NM_UTILS_LOOKUP_STR_ITEM(v, n)        NM_UTILS_LOOKUP_ITEM(v, ""n"")
#define NM_UTILS_LOOKUP_ITEM_IGNORE(v)        (void) 0; case v: break; (void) 0
#define NM_UTILS_LOOKUP_ITEM_IGNORE_OTHER()   (void) 0; default: break; (void) 0

#define _NM_UTILS_LOOKUP_DEFINE(scope, fcn_name, lookup_type, result_type, unknown_val, ...) \
scope result_type \
fcn_name (lookup_type val) \
{ \
	switch (val) { \
		(void) 0, \
		__VA_ARGS__ \
		(void) 0; \
	}; \
	{ unknown_val; } \
}

#define NM_UTILS_LOOKUP_STR_DEFINE(fcn_name, lookup_type, unknown_val, ...) \
	_NM_UTILS_LOOKUP_DEFINE (, fcn_name, lookup_type, const char *, unknown_val, __VA_ARGS__)
#define NM_UTILS_LOOKUP_STR_DEFINE_STATIC(fcn_name, lookup_type, unknown_val, ...) \
	_NM_UTILS_LOOKUP_DEFINE (static, fcn_name, lookup_type, const char *, unknown_val, __VA_ARGS__)

/* Call the string-lookup-table function @fcn_name. If the function returns
 * %NULL, the numeric index is converted to string using a alloca() buffer.
 * Beware: this macro uses alloca(). */
#define NM_UTILS_LOOKUP_STR_A(fcn_name, idx) \
	({ \
		typeof (idx) _idx = (idx); \
		const char *_s; \
		\
		_s = fcn_name (_idx); \
		if (!_s) { \
			_s = g_alloca (30); \
			\
			g_snprintf ((char *) _s, 30, "(%lld)", (long long) _idx); \
		} \
		_s; \
	})

/*****************************************************************************/

/* check if @flags has exactly one flag (@check) set. You should call this
 * only with @check being a compile time constant and a power of two. */
#define NM_FLAGS_HAS(flags, check)  \
    ( G_STATIC_ASSERT_EXPR ((check) > 0 && ((check) & ((check) - 1)) == 0), NM_FLAGS_ANY ((flags), (check)) )

#define NM_FLAGS_ANY(flags, check)  ( ( ((flags) & (check)) != 0       ) ? TRUE : FALSE )
#define NM_FLAGS_ALL(flags, check)  ( ( ((flags) & (check)) == (check) ) ? TRUE : FALSE )

#define NM_FLAGS_SET(flags, val)  ({ \
		const typeof(flags) _flags = (flags); \
		const typeof(flags) _val = (val); \
		\
		_flags | _val; \
	})

#define NM_FLAGS_UNSET(flags, val)  ({ \
		const typeof(flags) _flags = (flags); \
		const typeof(flags) _val = (val); \
		\
		_flags & (~_val); \
	})

#define NM_FLAGS_ASSIGN(flags, val, assign)  ({ \
		const typeof(flags) _flags = (flags); \
		const typeof(flags) _val = (val); \
		\
		(assign) \
			? _flags | (_val) \
			: _flags & (~_val); \
	})

/*****************************************************************************/

#define _NM_BACKPORT_SYMBOL_IMPL(version, return_type, orig_func, versioned_func, args_typed, args) \
return_type versioned_func args_typed; \
_nm_externally_visible return_type versioned_func args_typed \
{ \
    return orig_func args; \
} \
return_type orig_func args_typed; \
__asm__(".symver "G_STRINGIFY(versioned_func)", "G_STRINGIFY(orig_func)"@"G_STRINGIFY(version))

#define NM_BACKPORT_SYMBOL(version, return_type, func, args_typed, args) \
_NM_BACKPORT_SYMBOL_IMPL(version, return_type, func, _##func##_##version, args_typed, args)

/*****************************************************************************/

/* mirrors g_ascii_isspace() and what we consider spaces in general. */
#define NM_ASCII_SPACES "\t\n\f\r "

#define nm_str_skip_leading_spaces(str) \
	({ \
		typeof (*(str)) *_str_sls = (str); \
		_nm_unused const char *const _str_type_check = _str_sls; \
		\
		if (_str_sls) { \
			while (g_ascii_isspace (_str_sls[0])) \
				_str_sls++; \
		} \
		_str_sls; \
	})

static inline char *
nm_strstrip (char *str)
{
	/* g_strstrip doesn't like NULL. */
	return str ? g_strstrip (str) : NULL;
}

static inline const char *
nm_strstrip_avoid_copy (const char *str, char **str_free)
{
	gsize l;
	char *s;

	nm_assert (str_free && !*str_free);

	if (!str)
		return NULL;

	str = nm_str_skip_leading_spaces (str);
	l = strlen (str);
	if (   l == 0
	    || !g_ascii_isspace (str[l - 1]))
		return str;
	while (   l > 0
	       && g_ascii_isspace (str[l - 1]))
		l--;

	s = g_new (char, l + 1);
	memcpy (s, str, l);
	s[l] = '\0';
	*str_free = s;
	return s;
}

#define nm_strstrip_avoid_copy_a(alloca_maxlen, str, out_str_free) \
	({ \
		const char *_str_ssac = (str); \
		char **_out_str_free_ssac = (out_str_free); \
		\
		G_STATIC_ASSERT_EXPR ((alloca_maxlen) > 0); \
		\
		nm_assert ( _out_str_free_ssac || ((alloca_maxlen) > (str ? strlen (str) : 0u))); \
		nm_assert (!_out_str_free_ssac || !*_out_str_free_ssac); \
		\
		if (_str_ssac) { \
			_str_ssac = nm_str_skip_leading_spaces (_str_ssac); \
			if (_str_ssac[0] != '\0') { \
				gsize _l = strlen (_str_ssac); \
				\
				if (g_ascii_isspace (_str_ssac[--_l])) { \
					while (   _l > 0 \
					       && g_ascii_isspace (_str_ssac[_l - 1])) { \
						_l--; \
					} \
					_str_ssac = nm_strndup_a ((alloca_maxlen), _str_ssac, _l, _out_str_free_ssac); \
				} \
			} \
		} \
		\
		_str_ssac; \
	})

/* g_ptr_array_sort()'s compare function takes pointers to the
 * value. Thus, you cannot use strcmp directly. You can use
 * nm_strcmp_p().
 *
 * Like strcmp(), this function is not forgiving to accept %NULL. */
static inline int
nm_strcmp_p (gconstpointer a, gconstpointer b)
{
	const char *s1 = *((const char **) a);
	const char *s2 = *((const char **) b);

	return strcmp (s1, s2);
}

/*****************************************************************************/

/* Taken from systemd's UNIQ_T and UNIQ macros. */

#define NM_UNIQ_T(x, uniq) G_PASTE(__unique_prefix_, G_PASTE(x, uniq))
#define NM_UNIQ __COUNTER__

/*****************************************************************************/

/* glib's MIN()/MAX() macros don't have function-like behavior, in that they evaluate
 * the argument possibly twice.
 *
 * Taken from systemd's MIN()/MAX() macros. */

#define NM_MIN(a, b) __NM_MIN(NM_UNIQ, a, NM_UNIQ, b)
#define __NM_MIN(aq, a, bq, b) \
	({ \
		typeof (a) NM_UNIQ_T(A, aq) = (a); \
		typeof (b) NM_UNIQ_T(B, bq) = (b); \
		((NM_UNIQ_T(A, aq) < NM_UNIQ_T(B, bq)) ? NM_UNIQ_T(A, aq) : NM_UNIQ_T(B, bq)); \
	})

#define NM_MAX(a, b) __NM_MAX(NM_UNIQ, a, NM_UNIQ, b)
#define __NM_MAX(aq, a, bq, b) \
	({ \
		typeof (a) NM_UNIQ_T(A, aq) = (a); \
		typeof (b) NM_UNIQ_T(B, bq) = (b); \
		((NM_UNIQ_T(A, aq) > NM_UNIQ_T(B, bq)) ? NM_UNIQ_T(A, aq) : NM_UNIQ_T(B, bq)); \
	})

#define NM_CLAMP(x, low, high) __NM_CLAMP(NM_UNIQ, x, NM_UNIQ, low, NM_UNIQ, high)
#define __NM_CLAMP(xq, x, lowq, low, highq, high) \
	({ \
		typeof(x)NM_UNIQ_T(X,xq) = (x); \
		typeof(low) NM_UNIQ_T(LOW,lowq) = (low); \
		typeof(high) NM_UNIQ_T(HIGH,highq) = (high); \
		\
		( (NM_UNIQ_T(X,xq) > NM_UNIQ_T(HIGH,highq)) \
		  ? NM_UNIQ_T(HIGH,highq) \
		  : (NM_UNIQ_T(X,xq) < NM_UNIQ_T(LOW,lowq)) \
		     ? NM_UNIQ_T(LOW,lowq) \
		     : NM_UNIQ_T(X,xq)); \
	})

#define NM_MAX_WITH_CMP(cmp, a, b) \
	({ \
		typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		\
		(  ((cmp (_a, _b)) >= 0) \
		 ? _a \
		 : _b); \
	})

/* evaluates to (void) if _A or _B are not constant or of different types */
#define NM_CONST_MAX(_A, _B) \
	(__builtin_choose_expr ((   __builtin_constant_p (_A) \
	                         && __builtin_constant_p (_B) \
	                         && __builtin_types_compatible_p (typeof (_A), typeof (_B))), \
	                        ((_A) > (_B)) ? (_A) : (_B),                            \
	                        ((void)  0)))

/*****************************************************************************/

#define nm_g_slice_free(ptr) \
	g_slice_free (typeof (*(ptr)), ptr)

/*****************************************************************************/

/* like g_memdup(). The difference is that the @size argument is of type
 * gsize, while g_memdup() has type guint. Since, the size of container types
 * like GArray is guint as well, this means trying to g_memdup() an
 * array,
 *    g_memdup (array->data, array->len * sizeof (ElementType))
 * will lead to integer overflow, if there are more than G_MAXUINT/sizeof(ElementType)
 * bytes. That seems unnecessarily dangerous to me.
 * nm_memdup() avoids that, because its size argument is always large enough
 * to contain all data that a GArray can hold.
 *
 * Another minor difference to g_memdup() is that the glib version also
 * returns %NULL if @data is %NULL. E.g. g_memdup(NULL, 1)
 * gives %NULL, but nm_memdup(NULL, 1) crashes. I think that
 * is desirable, because @size MUST be correct at all times. @size
 * may be zero, but one must not claim to have non-zero bytes when
 * passing a %NULL @data pointer.
 */
static inline gpointer
nm_memdup (gconstpointer data, gsize size)
{
	gpointer p;

	if (size == 0)
		return NULL;
	p = g_malloc (size);
	memcpy (p, data, size);
	return p;
}

#define nm_malloc_maybe_a(alloca_maxlen, bytes, to_free) \
	({ \
		const gsize _bytes = (bytes); \
		typeof (to_free) _to_free = (to_free); \
		typeof (*_to_free) _ptr; \
		\
		G_STATIC_ASSERT_EXPR ((alloca_maxlen) <= 500); \
		nm_assert (_to_free && !*_to_free); \
		\
		if (_bytes <= (alloca_maxlen)) { \
			_ptr = g_alloca (_bytes); \
		} else { \
			_ptr = g_malloc (_bytes); \
			*_to_free = _ptr; \
		}; \
		\
		_ptr; \
	})

#define nm_malloc0_maybe_a(alloca_maxlen, bytes, to_free) \
	({ \
		const gsize _bytes = (bytes); \
		typeof (to_free) _to_free = (to_free); \
		typeof (*_to_free) _ptr; \
		\
		G_STATIC_ASSERT_EXPR ((alloca_maxlen) <= 500); \
		nm_assert (_to_free && !*_to_free); \
		\
		if (_bytes <= (alloca_maxlen)) { \
			_ptr = g_alloca (_bytes); \
			memset (_ptr, 0, _bytes); \
		} else { \
			_ptr = g_malloc0 (_bytes); \
			*_to_free = _ptr; \
		}; \
		\
		_ptr; \
	})

#define nm_memdup_maybe_a(alloca_maxlen, data, size, to_free) \
	({ \
		const gsize _size = (size); \
		typeof (to_free) _to_free_md = (to_free); \
		typeof (*_to_free_md) _ptr_md = NULL; \
		\
		nm_assert (_to_free_md && !*_to_free_md); \
		\
		if (_size > 0u) { \
			_ptr_md = nm_malloc_maybe_a ((alloca_maxlen), _size, _to_free_md); \
			memcpy (_ptr_md, (data), _size); \
		} \
		\
		_ptr_md; \
	})

static inline char *
_nm_strndup_a_step (char *s, const char *str, gsize len)
{
	NM_PRAGMA_WARNING_DISABLE ("-Wstringop-truncation");
	NM_PRAGMA_WARNING_DISABLE ("-Wstringop-overflow");
	if (len > 0)
		strncpy (s, str, len);
	s[len] = '\0';
	return s;
	NM_PRAGMA_WARNING_REENABLE;
	NM_PRAGMA_WARNING_REENABLE;
}

/* Similar to g_strndup(), however, if the string (including the terminating
 * NUL char) fits into alloca_maxlen, this will alloca() the memory.
 *
 * It's a mix of strndup() and strndupa(), but deciding based on @alloca_maxlen
 * which one to use.
 *
 * In case malloc() is necessary, @out_str_free will be set (this string
 * must be freed afterwards). It is permissible to pass %NULL as @out_str_free,
 * if you ensure that len < alloca_maxlen.
 *
 * Note that just like g_strndup(), this always returns a buffer with @len + 1
 * bytes, even if strlen(@str) is shorter than that (NUL terminated early). We fill
 * the buffer with strncpy(), which means, that @str is copied up to the first
 * NUL character and then filled with NUL characters. */
#define nm_strndup_a(alloca_maxlen, str, len, out_str_free) \
	({ \
		const gsize _alloca_maxlen_snd = (alloca_maxlen); \
		const char *const _str_snd = (str); \
		const gsize _len_snd = (len); \
		char **const _out_str_free_snd = (out_str_free); \
		char *_s_snd; \
		\
		G_STATIC_ASSERT_EXPR ((alloca_maxlen) <= 300); \
		\
		if (   _out_str_free_snd \
		    && _len_snd >= _alloca_maxlen_snd) { \
			_s_snd = g_malloc (_len_snd + 1); \
			*_out_str_free_snd = _s_snd; \
		} else { \
			g_assert (_len_snd < _alloca_maxlen_snd); \
			_s_snd = g_alloca (_len_snd + 1); \
		} \
		_nm_strndup_a_step (_s_snd, _str_snd, _len_snd); \
	})

/*****************************************************************************/

/* generic macro to convert an int to a (heap allocated) string.
 *
 * Usually, an inline function nm_strdup_int64() would be enough. However,
 * that cannot be used for guint64. So, we would also need nm_strdup_uint64().
 * This causes subtle error potential, because the caller needs to ensure to
 * use the right one (and compiler isn't going to help as it silently casts).
 *
 * Instead, this generic macro is supposed to handle all integers correctly. */
#if _NM_CC_SUPPORT_GENERIC
#define nm_strdup_int(val) \
	_Generic ((val), \
	          char:               g_strdup_printf ("%d",   (int)                (val)), \
	          \
	          signed char:        g_strdup_printf ("%d",   (signed)             (val)), \
	          signed short:       g_strdup_printf ("%d",   (signed)             (val)), \
	          signed:             g_strdup_printf ("%d",   (signed)             (val)), \
	          signed long:        g_strdup_printf ("%ld",  (signed long)        (val)), \
	          signed long long:   g_strdup_printf ("%lld", (signed long long)   (val)), \
	          \
	          unsigned char:      g_strdup_printf ("%u",   (unsigned)           (val)), \
	          unsigned short:     g_strdup_printf ("%u",   (unsigned)           (val)), \
	          unsigned:           g_strdup_printf ("%u",   (unsigned)           (val)), \
	          unsigned long:      g_strdup_printf ("%lu",  (unsigned long)      (val)), \
	          unsigned long long: g_strdup_printf ("%llu", (unsigned long long) (val))  \
	)
#else
#define nm_strdup_int(val) \
	(  (   sizeof (val) == sizeof (guint64) \
	    && ((typeof (val)) -1) > 0) \
	 ? g_strdup_printf ("%"G_GUINT64_FORMAT, (guint64) (val)) \
	 : g_strdup_printf ("%"G_GINT64_FORMAT, (gint64) (val)))
#endif

/*****************************************************************************/

static inline guint
nm_encode_version (guint major, guint minor, guint micro)
{
	/* analog to the preprocessor macro NM_ENCODE_VERSION(). */
	return (major << 16) | (minor << 8) | micro;
}

static inline void
nm_decode_version (guint version, guint *major, guint *minor, guint *micro)
{
	*major = (version & 0xFFFF0000u) >> 16;
	*minor = (version & 0x0000FF00u) >>  8;
	*micro = (version & 0x000000FFu);
}

/*****************************************************************************/

/* taken from systemd's DECIMAL_STR_MAX()
 *
 * Returns the number of chars needed to format variables of the
 * specified type as a decimal string. Adds in extra space for a
 * negative '-' prefix (hence works correctly on signed
 * types). Includes space for the trailing NUL. */
#define NM_DECIMAL_STR_MAX(type) \
    (2+(sizeof(type) <= 1 ? 3 : \
        sizeof(type) <= 2 ? 5 : \
        sizeof(type) <= 4 ? 10 : \
        sizeof(type) <= 8 ? 20 : sizeof(int[-2*(sizeof(type) > 8)])))

/*****************************************************************************/

/* if @str is NULL, return "(null)". Otherwise, allocate a buffer using
 * alloca() of and fill it with @str. @str will be quoted with double quote.
 * If @str is longer then @trunc_at, the string is truncated and the closing
 * quote is instead '^' to indicate truncation.
 *
 * Thus, the maximum stack allocated buffer will be @trunc_at+3. The maximum
 * buffer size must be a constant and not larger than 300. */
#define nm_strquote_a(trunc_at, str) \
	({ \
		const char *const _str = (str); \
		\
		(_str \
			? ({ \
				const gsize _trunc_at = (trunc_at); \
				const gsize _strlen_trunc = NM_MIN (strlen (_str), _trunc_at); \
				char *_buf; \
				\
				G_STATIC_ASSERT_EXPR ((trunc_at) <= 300); \
				\
				_buf = g_alloca (_strlen_trunc + 3); \
				_buf[0] = '"'; \
				memcpy (&_buf[1], _str, _strlen_trunc); \
				_buf[_strlen_trunc + 1] = _str[_strlen_trunc] ? '^' : '"'; \
				_buf[_strlen_trunc + 2] = '\0'; \
				_buf; \
			}) \
			: "(null)"); \
	})

#define nm_sprintf_buf(buf, format, ...) \
	({ \
		char * _buf = (buf); \
		int _buf_len; \
		\
		/* some static assert trying to ensure that the buffer is statically allocated.
		 * It disallows a buffer size of sizeof(gpointer) to catch that. */ \
		G_STATIC_ASSERT (G_N_ELEMENTS (buf) == sizeof (buf) && sizeof (buf) != sizeof (char *)); \
		_buf_len = g_snprintf (_buf, sizeof (buf), \
		                       ""format"", ##__VA_ARGS__); \
		nm_assert (_buf_len < sizeof (buf)); \
		_buf; \
	})

/* it is "unsafe" because @bufsize must not be a constant expression and
 * there is no check at compiletime. Regardless of that, the buffer size
 * must not be larger than 300 bytes, as this gets stack allocated. */
#define nm_sprintf_buf_unsafe_a(bufsize, format, ...) \
	({ \
		char *_buf; \
		int _buf_len; \
		typeof (bufsize) _bufsize = (bufsize); \
		\
		nm_assert (_bufsize <= 300); \
		\
		_buf = g_alloca (_bufsize); \
		_buf_len = g_snprintf (_buf, _bufsize, \
		                       ""format"", ##__VA_ARGS__); \
		nm_assert (_buf_len >= 0 && _buf_len < _bufsize); \
		_buf; \
	})

#define nm_sprintf_bufa(bufsize, format, ...) \
	({ \
		G_STATIC_ASSERT_EXPR ((bufsize) <= 300); \
		nm_sprintf_buf_unsafe_a ((bufsize), format, ##__VA_ARGS__); \
	})

/* aims to alloca() a buffer and fill it with printf(format, name).
 * Note that format must not contain any format specifier except
 * "%s".
 * If the resulting string would be too large for stack allocation,
 * it allocates a buffer with g_malloc() and assigns it to *p_val_to_free. */
#define nm_construct_name_a(format, name, p_val_to_free) \
	({ \
		const char *const _name = (name); \
		char **const _p_val_to_free = (p_val_to_free); \
		const gsize _name_len = strlen (_name); \
		char *_buf2; \
		\
		nm_assert (_p_val_to_free && !*_p_val_to_free); \
		if (   NM_STRLEN (format) <= 290 \
		    && _name_len < (gsize) (290 - NM_STRLEN (format))) \
			_buf2 = nm_sprintf_buf_unsafe_a (NM_STRLEN (format) + _name_len, format, _name); \
		else { \
			_buf2 = g_strdup_printf (format, _name); \
			*_p_val_to_free = _buf2; \
		} \
		(const char *) _buf2; \
	})

/*****************************************************************************/

/**
 * The boolean type _Bool is C99 while we mostly stick to C89. However, _Bool is too
 * convenient to miss and is effectively available in gcc and clang. So, just use it.
 *
 * Usually, one would include "stdbool.h" to get the "bool" define which aliases
 * _Bool. We provide this define here, because we want to make use of it anywhere.
 * (also, stdbool.h is again C99).
 *
 * Using _Bool has advantages over gboolean:
 *
 * - commonly _Bool is one byte large, instead of gboolean's 4 bytes (because gboolean
 *   is a typedef for int). Especially when having boolean fields in a struct, we can
 *   thereby easily save some space.
 *
 * - _Bool type guarantees that two "true" expressions compare equal. E.g. the following
 *   will not work:
 *        gboolean v1 = 1;
 *        gboolean v2 = 2;
 *        g_assert_cmpint (v1, ==, v2); // will fail
 *   For that, we often to use !! to coerce gboolean values to 0 or 1:
 *        g_assert_cmpint (!!v2, ==, TRUE);
 *   With _Bool type, this will be handled properly by the compiler.
 *
 * - For structs, we might want to safe even more space and use bitfields:
 *       struct s1 {
 *           gboolean v1:1;
 *       };
 *   But the problem here is that gboolean is signed, so that
 *   v1 will be either 0 or -1 (not 1, TRUE). Thus, the following
 *   fails:
 *      struct s1 s = { .v1 = TRUE, };
 *      g_assert_cmpint (s1.v1, ==, TRUE);
 *   It will however work just fine with bool/_Bool while retaining the
 *   notion of having a boolean value.
 *
 * Also, add the defines for "true" and "false". Those are nicely highlighted by the editor
 * as special types, contrary to glib's "TRUE"/"FALSE".
 */

#ifndef bool
#define bool _Bool
#define true    1
#define false   0
#endif

#ifdef _G_BOOLEAN_EXPR
/* g_assert() uses G_LIKELY(), which in turn uses _G_BOOLEAN_EXPR().
 * As glib's implementation uses a local variable _g_boolean_var_,
 * we cannot do
 *   g_assert (some_macro ());
 * where some_macro() itself expands to ({g_assert(); ...}).
 * In other words, you cannot have a g_assert() inside a g_assert()
 * without getting a -Werror=shadow failure.
 *
 * Workaround that by re-defining _G_BOOLEAN_EXPR()
 **/
#undef  _G_BOOLEAN_EXPR
#define __NM_G_BOOLEAN_EXPR_IMPL(v, expr) \
	({ \
		int NM_UNIQ_T(V, v); \
		\
		if (expr) \
			NM_UNIQ_T(V, v) = 1; \
		else \
			NM_UNIQ_T(V, v) = 0; \
		NM_UNIQ_T(V, v); \
	})
#define _G_BOOLEAN_EXPR(expr) __NM_G_BOOLEAN_EXPR_IMPL (NM_UNIQ, expr)
#endif

/*****************************************************************************/

/**
 * nm_steal_int:
 * @p_val: pointer to an int type.
 *
 * Returns: *p_val and sets *p_val to zero the same time.
 *   Accepts %NULL, in which case also numeric 0 will be returned.
 */
#define nm_steal_int(p_val) \
	({ \
		typeof (p_val) const _p_val = (p_val); \
		typeof (*_p_val) _val = 0; \
		\
		if (   _p_val \
		    && (_val = *_p_val)) { \
			*_p_val = 0; \
		} \
		_val; \
	})

static inline int
nm_steal_fd (int *p_fd)
{
	int fd;

	if (   p_fd
	    && ((fd = *p_fd) >= 0)) {
		*p_fd = -1;
		return fd;
	}
	return -1;
}

/**
 * nm_close:
 *
 * Like close() but throws an assertion if the input fd is
 * invalid.  Closing an invalid fd is a programming error, so
 * it's better to catch it early.
 */
static inline int
nm_close (int fd)
{
	int r;

	r = close (fd);
	nm_assert (r != -1 || fd < 0 || errno != EBADF);
	return r;
}

#define NM_PID_T_INVAL ((pid_t) -1)

/*****************************************************************************/

NM_AUTO_DEFINE_FCN_VOID0 (GMutex *, _nm_auto_unlock_g_mutex, g_mutex_unlock)

#define nm_auto_unlock_g_mutex nm_auto (_nm_auto_unlock_g_mutex)

#define _NM_G_MUTEX_LOCKED(lock, uniq) \
	nm_auto_unlock_g_mutex GMutex *NM_UNIQ_T(nm_lock, uniq) = (lock)

#define NM_G_MUTEX_LOCKED(lock) \
	_NM_G_MUTEX_LOCKED (lock, NM_UNIQ)

/*****************************************************************************/

#endif /* __NM_MACROS_INTERNAL_H__ */
