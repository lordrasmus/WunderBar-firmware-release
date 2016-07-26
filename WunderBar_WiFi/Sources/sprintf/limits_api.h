/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/07/06 06:32:33 $
 * $Revision: 1.2 $
 */

#ifndef _EWL_LIMITS_API_H
#define _EWL_LIMITS_API_H

#if _AEABI_PORTABILITY_LEVEL
	#include <arm/limits_aeabi.h>
#endif

#ifndef _EWL_CHAR_BIT
	#define _EWL_CHAR_BIT				8
#endif

#ifndef _EWL_SIZEOF_SHRT
	#define _EWL_SIZEOF_SHRT			2
#endif

#ifndef _EWL_SIZEOF_INT
	#define _EWL_SIZEOF_INT				4
#endif

#ifndef _EWL_SIZEOF_LONG
	#define _EWL_SIZEOF_LONG			4
#endif

#ifndef _EWL_MB_LEN_MAX
	#define _EWL_MB_LEN_MAX				3
#endif

#if _EWL_LONGLONG
	#ifndef _EWL_SIZEOF_LONG_LONG
		#define _EWL_SIZEOF_LONG_LONG	8
	#endif
#endif /* _EWL_LONGLONG */

#ifndef _EWL_UNSIGNED_CHAR
	#ifdef __CWCC__
		#if __option(unsigned_char)
			#define _EWL_UNSIGNED_CHAR	1
		#else
			#define _EWL_UNSIGNED_CHAR	0
		#endif
	#elif defined(__GNUC__)
		#if __CHAR_UNSIGNED__
			#define _EWL_UNSIGNED_CHAR	1
		#else
			#define _EWL_UNSIGNED_CHAR	0
		#endif
	#else
		#define _EWL_UNSIGNED_CHAR		0
	#endif /* __CWCC__ */
#endif /* _EWL_UNSIGNED_CHAR */


#if _EWL_CHAR_BIT == 8
	#define _EWL_SCHAR_MAX				0x7f
	#define _EWL_UCHAR_MAX				0xff
#elif _EWL_CHAR_BIT == 16
	#define _EWL_SCHAR_MAX				0x7fff
	#define _EWL_UCHAR_MAX				0xffff
#elif _EWL_CHAR_BIT == 24
	#define _EWL_SCHAR_MAX				0x7fffff
	#define _EWL_UCHAR_MAX				0xffffff
#else
	#error unsupported size for _EWL_CHAR_BIT
#endif

#define _EWL_SCHAR_MIN					(-_EWL_SCHAR_MAX - 1)

#if _EWL_UNSIGNED_CHAR
	#define _EWL_CHAR_MIN				0
	#define _EWL_CHAR_MAX				_EWL_UCHAR_MAX
#else
	#define _EWL_CHAR_MIN				_EWL_SCHAR_MIN
	#define _EWL_CHAR_MAX				_EWL_SCHAR_MAX
#endif

#if _EWL_SIZEOF_SHRT*_EWL_CHAR_BIT == 16
	#define _EWL_SHRT_MAX				0x7fff
	#define _EWL_USHRT_MAX				0xffff
#elif _EWL_SIZEOF_SHRT*_EWL_CHAR_BIT == 24
	#define _EWL_SHRT_MAX				0x7fffff
	#define _EWL_USHRT_MAX				0xffffff
#else
	#error unsupported size for sizeof short
#endif

#define _EWL_SHRT_MIN					(-_EWL_SHRT_MAX - 1)

#if _EWL_SIZEOF_INT*_EWL_CHAR_BIT == 16
	#define _EWL_INT_MAX				0x7fff
	#define _EWL_UINT_MAX				0xffffU
#elif _EWL_SIZEOF_INT*_EWL_CHAR_BIT == 24
	#define _EWL_INT_MAX				0x7fffff
	#define _EWL_UINT_MAX				0xffffffU
#elif _EWL_SIZEOF_INT*_EWL_CHAR_BIT == 32
	#define _EWL_INT_MAX				0x7fffffff
	#define _EWL_UINT_MAX				0xffffffffU
#else
	#error unsupported size for sizeof int
#endif

#define _EWL_INT_MIN					(-_EWL_INT_MAX - 1)

#if _EWL_SIZEOF_LONG*_EWL_CHAR_BIT == 24
	#define _EWL_LONG_MAX				0x7fffffL
	#define _EWL_ULONG_MAX				0xffffffUL
#elif _EWL_SIZEOF_LONG*_EWL_CHAR_BIT == 32
	#define _EWL_LONG_MAX				0x7fffffffL
	#define _EWL_ULONG_MAX				0xffffffffUL
#else
	#error unsupported size for sizeof long
#endif

#define _EWL_LONG_MIN					(-_EWL_LONG_MAX - 1)

#if _EWL_LONGLONG
	#if _EWL_SIZEOF_LONG_LONG*_EWL_CHAR_BIT == 24
		#define _EWL_LLONG_MAX			0x7fffffLL
		#define _EWL_ULLONG_MAX			0xffffffULL
	#elif _EWL_SIZEOF_LONG_LONG*_EWL_CHAR_BIT == 32
		#define _EWL_LLONG_MAX			0x7fffffffLL
		#define _EWL_ULLONG_MAX			0xffffffffULL
	#elif _EWL_SIZEOF_LONG_LONG*_EWL_CHAR_BIT == 64
		#define _EWL_LLONG_MAX			0x7fffffffffffffffLL
		#define _EWL_ULLONG_MAX			0xffffffffffffffffULL
	#else
		#error unsupported size for sizeof long long
	#endif

	#define _EWL_LLONG_MIN				(-_EWL_LLONG_MAX - 1)
#endif /* _EWL_LONGLONG && _EWL_C99 */

#endif /* _EWL_LIMITS_API_H */
