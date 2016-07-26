/* EWL
 * Copyright © 2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/11/16 04:12:56 $
 * $Revision: 1.6 $
 */

#ifndef __ansi_prefix__
#define __ansi_prefix__

#include "os_enum.h"

#define __dest_os __arm_bare

#define _USE_AEABI_PRIVATE_  1

/*********************************************/
/* 		OS Specific Configuration Flags		 */
/*********************************************/

#if defined(__GNUC__)
#define __option(x)			x
#define ANSI_strict        	__STRICT_ANSI__
#define unsigned_char      	__CHAR_UNSIGNED__
#define optimize_for_size  	__OPTIMIZE_SIZE__
#define sfp_emulation	   	_SOFT_FLOAT
#define dont_inline        	__NO_INLINE__
#define little_endian	   	__ARMEL__
#define double_is_4_bytes	0
#define floatingpoint		_EWL_FLOATING_POINT
#define __thumb			__thumb__
#endif

/* Console IO Support Configuration Flags */

#ifndef _EWL_CONSOLE_SUPPORT
	#define _EWL_CONSOLE_SUPPORT	1
#endif

#ifndef _EWL_BUFFERED_CONSOLE
#define _EWL_BUFFERED_CONSOLE			1
#endif

#ifndef _EWL_BUFFERED_CONSOLE_SIZE
#define _EWL_BUFFERED_CONSOLE_SIZE 		64
#endif

#ifndef _EWL_CONSOLE_FILE_IS_DISK_FILE
#define _EWL_CONSOLE_FILE_IS_DISK_FILE	0
#endif

#ifndef _EWL_FILE_CONSOLE_ROUTINES
#define _EWL_FILE_CONSOLE_ROUTINES 		0
#endif

#ifndef _EWL_NULL_CONSOLE_ROUTINES
	#if _EWL_CONSOLE_SUPPORT
		#define _EWL_NULL_CONSOLE_ROUTINES 0
	#else
		#define _EWL_NULL_CONSOLE_ROUTINES 1
	#endif
#endif

#if !defined(__STDC_WANT_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 				1
#endif

#if !defined(_lint)
#define _EWL_C99_TC2_FENV					1
#endif

#define _EWL_C99_MATH_LEAK                  1

/* File IO Support Configuration Flags */

#ifndef _EWL_OS_DISK_FILE_SUPPORT
	#define _EWL_OS_DISK_FILE_SUPPORT	0
#endif

/*	File and console I/O configurations */

#if !defined(TMP_MAX) && ! defined(_AEABI_PORTABILITY_LEVEL)

	#define TMP_MAX						32767
#endif

/* Time Support Configuration Flags */

#ifndef _EWL_OS_TIME_SUPPORT
	#define _EWL_OS_TIME_SUPPORT	1
#endif

#ifndef _EWL_CLOCK_T_AVAILABLE
	#define _EWL_CLOCK_T_AVAILABLE 	1
#endif

#ifndef _EWL_TIME_T_AVAILABLE
	#define _EWL_TIME_T_AVAILABLE	1
#endif

#if _EWL_OS_TIME_SUPPORT && ((!_EWL_CLOCK_T_AVAILABLE)  || (!_EWL_TIME_T_AVAILABLE))
	#error _EWL_CLOCK_T_AVAILABLE and _EWL_CLOCK_T_AVAILABLE must match _EWL_OS_TIME_SUPPORT
#endif

/* Thread Safety Configuration Flags */

#ifndef _EWL_THREADSAFE
	#define _EWL_THREADSAFE 0
#endif

#ifndef _EWL_PTHREADS
	#define _EWL_PTHREADS 0
#endif

#if _EWL_PTHREADS && !_EWL_THREADSAFE
	#error _EWL_PTHREADS and _EWL_THREADSAFE must match
#endif

#ifndef _EWL_LOCALDATA_AVAILABLE
	#define _EWL_LOCALDATA_AVAILABLE 0
#endif

/* Memory Allocation Flags */

/*  See alloc.c for more details on configuration options */

#ifndef _EWL_OS_ALLOC_SUPPORT
	#define _EWL_OS_ALLOC_SUPPORT 		0
#endif

#ifndef _EWL_HEAP_EXTERN_PROTOTYPES
	#define _EWL_HEAP_EXTERN_PROTOTYPES \
		extern char_t __heap_addr[]; \
		extern char_t __heap_size[];
#endif

#ifndef _EWL_HEAP_START
	#define _EWL_HEAP_START __heap_addr
#endif

#ifndef _EWL_HEAP_SIZE
	#define _EWL_HEAP_SIZE __heap_size
#endif

/* #define _EWL_NO_MALLOC_DESIGN_2 */ /* lower overhead but slower allocation */

/* #define _EWL_NO_FIX_MALLOC_POOLS */ /* For heaps that are less than 64k */


/********************************************/
/* 		Additional Portability Flags 		*/
/********************************************/

#ifndef _EWL_COMPLEX_SUPPORT
	#define _EWL_COMPLEX_SUPPORT 0
#endif

#ifndef _EWL_LONGLONG
	#define _EWL_LONGLONG 1
#endif

#ifndef _EWL_FLOATING_POINT
	#define _EWL_FLOATING_POINT 0
#endif

#ifndef _EWL_WIDE_CHAR
	#define _EWL_WIDE_CHAR 0
#endif

#ifndef _EWL_USES_SUN_MATH_LIB
	#define _EWL_USES_SUN_MATH_LIB 		1
	#define _EWL_USES_SUN_SP_MATH_LIB 	1
	#ifndef _EWL_USES_SUN_DP_MATH_LIB
	#define _EWL_USES_SUN_DP_MATH_LIB 	1
	#endif
#endif

#ifndef _EWL_POSIX
	#define _EWL_POSIX 0
#endif

#ifndef _EWL_NEEDS_EXTRAS
	#define _EWL_NEEDS_EXTRAS 0
#endif

#define _EWL_BF_ORDER

#ifndef _EWL_IEC_559_ADDITIONS
	#define _EWL_IEC_559_ADDITIONS			1
#endif

#if defined(__GNUC__)
#ifndef _EWL_USE_INLINE
	#define _EWL_USE_INLINE		0
#endif
#endif

/************************************************/
/* 		Flags to reduce code and data size 		*/
/************************************************/

#ifndef _EMBEDDED_WARRIOR_CTYPE
#define _EMBEDDED_WARRIOR_CTYPE 			1
#endif

#ifndef _EWL_STRERROR_KNOWS_ERROR_NAMES
	#define _EWL_STRERROR_KNOWS_ERROR_NAMES 0
#endif

#ifndef _EWL_ASSERT_DISPLAYS_FUNC
	#define _EWL_ASSERT_DISPLAYS_FUNC	0
#endif

#ifndef _EMBEDDED_WARRIOR_HAS_NO_LOCALE
	#define _EMBEDDED_WARRIOR_HAS_NO_LOCALE	1
#endif

#ifndef _EWL_C_LOCALE_ONLY
	#define _EWL_C_LOCALE_ONLY	1
#endif

#ifndef _EMBEDDED_WARRIOR_MALLOC
	#define _EMBEDDED_WARRIOR_MALLOC		1	/* 1: for reduced code and data size by reducing EWL functionality */
#endif

#ifndef _EWL_C99
	#define _EWL_C99	0
#endif

#ifndef _EWL_ACCURATE_BUT_LARGE_ANSI_FP
    #define _EWL_ACCURATE_BUT_LARGE_ANSI_FP	0
#endif

#ifndef __EWL_ROUND_DECIMAL_ANSI_FP__
	#define __EWL_ROUND_DECIMAL_ANSI_FP__ 	1
#endif

#if !_EWL_C99 && !_EWL_C_LOCALE_ONLY && !_EMBEDDED_WARRIOR_HAS_NO_LOCALE
	#error _EWL_C_LOCALE_ONLY must be turned on if _EWL_C99 is off
#endif

#ifndef _EWL_FLOATING_POINT_IO
	#define _EWL_FLOATING_POINT_IO	0
#endif

#ifndef _EWL_C_HAS_CPP_EXTENSIONS
	#define _EWL_C_HAS_CPP_EXTENSIONS 		1
#endif

#ifndef _EWL_C99_PRINTF_SCANF
	#define _EWL_C99_PRINTF_SCANF 			0
#endif

#ifndef _EWL_FLOATING_POINT_PRINTF_SCANF
	#define _EWL_FLOATING_POINT_PRINTF_SCANF			0	/* 0: for reduced code and data size by reducing EWL functionality */
#endif

#define _EWL_C99_MATH_LEAK                  1

#ifndef _EWL_USES_EXIT_PROCESS
	#define _EWL_USES_EXIT_PROCESS 1
#endif

#ifndef _EWL_FLT_EVAL_METHOD
	#define _EWL_FLT_EVAL_METHOD  0
#endif

#define _EWL_FLOAT_HEX 						0

#if defined(__cplusplus)
    #define _EWL_USING_NAMESPACE
#endif

#define __ANSI_OVERLOAD__

#define _EWL_INTEGRAL_MATH

/************************************************/
/* 		ARM EABI								*/
/************************************************/

#ifndef _AEABI_PORTABILITY_LEVEL
	#define _AEABI_PORTABILITY_LEVEL 0
#endif

#if _AEABI_PORTABILITY_LEVEL && (!_lint)
	typedef long long __int64;
#endif

#if __GNUC__
#define _EWL_DOUBLE_SIZE	(__SIZEOF_DOUBLE__ * 8)
#else
#define _EWL_DOUBLE_SIZE 			64
#endif

#endif /*	__ansi_prefix__	  */
