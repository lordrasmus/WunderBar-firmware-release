/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/06/01 15:39:39 $
 * $Revision: 1.1 $
 */

#ifndef _EWL_WCHAR_T_H
#define _EWL_WCHAR_T_H

#include "ansi_parms.h"

#if _EWL_WIDE_CHAR

#ifndef _EWL_WCHAR_T_TYPE
	#if defined(__GNUC__) 
		#define _EWL_WCHAR_T_TYPE __WCHAR_TYPE__
	#elif defined(_GHSPORT_) 
		#define _EWL_WCHAR_T_TYPE long
	#else
		#define _EWL_WCHAR_T_TYPE unsigned short
	#endif /*  __GNUC__ */
#endif /* _EWL_WCHAR_T_TYPE */


#if defined(__CWCC__)
	#if (!defined(__cplusplus)|| !__option(wchar_type))
		#ifdef __cplusplus
			extern "C" {
		#endif

		typedef _EWL_WCHAR_T_TYPE wchar_t;

		#ifdef __cplusplus
			}
		#endif
	#endif /* (!defined(__cplusplus)|| !__option(wchar_type)) */
#endif /* __CWCC__ */

#if defined(__GNUC__) && !defined(__cplusplus) 
	typedef _EWL_WCHAR_T_TYPE wchar_t;
#endif /* defined(__GNUC__)  && !defined(__cplusplus) */

#ifdef __cplusplus
	#ifdef _EWL_USING_NAMESPACE
		namespace std {
	#endif
	extern "C" {
#endif

#if defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL != 0)
 typedef int wint_t;
#else
 typedef wchar_t wint_t;
#endif

#if !defined(_AEABI_PORTABILITY_LEVEL) || (_AEABI_PORTABILITY_LEVEL == 0)

	typedef int 	mbstate_t;
	typedef wchar_t wctype_t;

#else

	#define _AEABI_PORTABLE 1
	typedef struct { unsigned state1, state2;} mbstate_t;
	typedef void * wctype_t;

#endif

typedef wchar_t Wint_t;

#ifndef _EWL_WCHAR_MAX
	#ifdef __GNUC__
		#define _EWL_WCHAR_MAX __WCHAR_MAX__
	#else
		#define _EWL_WCHAR_MAX 0xffffU
	#endif
#endif

#ifndef _EWL_WCHAR_MIN
	#ifdef __GNUC__
		#define _EWL_WCHAR_MIN (-__WCHAR_MAX__-1)
	#else
		#define _EWL_WCHAR_MIN 0
	#endif
#endif

#ifndef WCHAR_MIN
	#define WCHAR_MIN _EWL_WCHAR_MIN
#endif

#ifndef WCHAR_MAX
	#define WCHAR_MAX _EWL_WCHAR_MAX
#endif


#if __dest_os == __win32_os
	#define _WCTYPE_T_DEFINED
#endif

#ifdef __cplusplus
	}
	#ifdef _EWL_USING_NAMESPACE
		}
	#endif
#endif

#endif /* _EWL_WIDE_CHAR */

#endif /* _EWL_WCHAR_T_H */
