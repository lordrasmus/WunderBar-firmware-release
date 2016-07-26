/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/07/12 22:31:13 $
 * $Revision: 1.2 $
 */

#ifndef _EWL_ANSI_PARMS_H

#define _EWL_ANSI_PARMS_H

#include "ewl_c_version.h"

#include "ewlGlobals.h"

#if !defined(_EWL_CONSOLE_SUPPORT)
	#define _EWL_CONSOLE_SUPPORT 0
#endif

#ifndef _EWL_CONSOLE_SUPPORT
	#define _EWL_CONSOLE_SUPPORT 1
#endif

#ifndef _EWL_BUFFERED_CONSOLE
	#define _EWL_BUFFERED_CONSOLE 1
#endif

#ifndef _EWL_CONSOLE_FILE_IS_DISK_FILE
	#define _EWL_CONSOLE_FILE_IS_DISK_FILE 0
#endif

#ifndef _EWL_FILE_CONSOLE_ROUTINES
	#define _EWL_FILE_CONSOLE_ROUTINES 0
#endif

#ifndef _EWL_NULL_CONSOLE_ROUTINES
	#if _EWL_CONSOLE_SUPPORT
		#define _EWL_NULL_CONSOLE_ROUTINES 0
	#else
		#define _EWL_NULL_CONSOLE_ROUTINES 1
	#endif
#endif

#if !defined(_EWL_OS_DISK_FILE_SUPPORT)
	#define _EWL_OS_DISK_FILE_SUPPORT 0
#endif

/* Memory Management configuration */
/* By default use the simplest memory allocation scheme */
#if !defined(_EMBEDDED_WARRIOR_MALLOC) && !defined(_EWL_BAREBOARD_MALLOC) && !defined(_EWL_CLASSIC_MALLOC) && !defined(_EWL_OS_DIRECT_MALLOC)

	#define _EMBEDDED_WARRIOR_MALLOC	0
	#define _EWL_BAREBOARD_MALLOC		0
	#define _EWL_CLASSIC_MALLOC			0
	#define _EWL_OS_DIRECT_MALLOC		0

#endif

#ifndef _EMBEDDED_WARRIOR_MALLOC
	#define _EMBEDDED_WARRIOR_MALLOC	0
#endif

#ifndef _EWL_BAREBOARD_MALLOC
	#define _EWL_BAREBOARD_MALLOC		0
#endif

#ifndef _EWL_CLASSIC_MALLOC
	#define _EWL_CLASSIC_MALLOC			0
#endif

#ifndef _EWL_OS_DIRECT_MALLOC
	#define _EWL_OS_DIRECT_MALLOC		0
#endif

/* assume there is no OS supporting heap management */
#ifndef _EWL_OS_ALLOC_HEAP_SUPPORT
	#define _EWL_OS_ALLOC_HEAP_SUPPORT 	0
#endif

/* time and clock support */

#ifndef _EWL_OS_TIME_SUPPORT
	#define _EWL_OS_TIME_SUPPORT 		0
#endif

#ifndef	_EWL_CLOCK_T_AVAILABLE
	#define _EWL_CLOCK_T_AVAILABLE 		1
#endif

#ifndef _EWL_TIME_T_AVAILABLE
	#define _EWL_TIME_T_AVAILABLE 		1
#endif

#ifndef _EWL_TM_STRUCT_AVAILABLE
	#define _EWL_TM_STRUCT_AVAILABLE 	1
#endif

#ifndef _EWL_TIME_T_IS_LOCALTIME
	#define _EWL_TIME_T_IS_LOCALTIME 	1
#endif

#if _EWL_OS_TIME_SUPPORT && ((!_EWL_CLOCK_T_AVAILABLE)  || (!_EWL_TIME_T_AVAILABLE))
	#error _EWL_CLOCK_T_AVAILABLE and _EWL_TIME_T_AVAILABLE must match _EWL_OS_TIME_SUPPORT
#endif

#ifndef _EWL_THREADSAFE
	#define _EWL_THREADSAFE				0
#endif

#ifndef _EWL_PTHREADS
	#define _EWL_PTHREADS 				0
#endif

#if _EWL_PTHREADS && !_EWL_THREADSAFE
	#error _EWL_PTHREADS and _EWL_THREADSAFE must match
#endif

#ifndef _EWL_LONGLONG
	#if defined(__CWCC__)
		#if __option(longlong)
			#define _EWL_LONGLONG 	1
		#else
			#define _EWL_LONGLONG 	0
		#endif
	#else
		#define _EWL_LONGLONG 		0
	#endif
#endif

#ifndef _EWL_FLOATING_POINT
	#define _EWL_FLOATING_POINT 		0
#endif

#ifndef _EWL_FLOATING_POINT_PRINTF_SCANF
	#define _EWL_FLOATING_POINT_PRINTF_SCANF		0
#endif

#ifndef _EWL_WIDE_CHAR
	#define _EWL_WIDE_CHAR 				0
#endif

#ifndef _EWL_USES_SUN_MATH_LIB
	#define _EWL_USES_SUN_MATH_LIB 		0
#endif

#ifndef _EWL_USES_SUN_SP_MATH_LIB
	#define _EWL_USES_SUN_SP_MATH_LIB 	0
#endif

#ifndef _EWL_USES_SUN_DP_MATH_LIB
	#if _EWL_USES_SUN_MATH_LIB
		#define _EWL_USES_SUN_DP_MATH_LIB 1
	#else
		#define _EWL_USES_SUN_DP_MATH_LIB 0
	#endif
#endif

#if !_EWL_USES_SUN_MATH_LIB && (_EWL_USES_SUN_SP_MATH_LIB || _EWL_USES_SUN_DP_MATH_LIB)
	#error "Macro _EWL_USES_SUN_SP_MATH_LIB and _EWL_USES_SUN_DP_MATH_LIB require _EWL_USES_SUN_MATH_LIB be true!"
#endif

#ifndef _EWL_ACCURATE_BUT_LARGE_ANSI_FP
	#define _EWL_ACCURATE_BUT_LARGE_ANSI_FP	0
#endif

#ifndef _EWL_FLT_EVAL_METHOD
	#define _EWL_FLT_EVAL_METHOD  		0
#endif

#ifndef _EWL_C99
	#define _EWL_C99 					1
#endif

#ifndef _EWL_C99_MATH_LEAK
	#define _EWL_C99_MATH_LEAK			0
#endif

#ifndef _EWL_C_HAS_CPP_EXTENSIONS
	#ifdef __cplusplus
		#define _EWL_C_HAS_CPP_EXTENSIONS 	1
	#else
		#define _EWL_C_HAS_CPP_EXTENSIONS 	0
	#endif
#endif

#ifndef _EWL_POSIX
	#define _EWL_POSIX 					0
#endif

#ifndef _EWL_NEEDS_EXTRAS
	#define _EWL_NEEDS_EXTRAS 			0
#endif

#ifndef _EWL_STRERROR_KNOWS_ERROR_NAMES
	#define _EWL_STRERROR_KNOWS_ERROR_NAMES 1
#endif

#ifndef _EWL_C_LOCALE_ONLY
	#define _EWL_C_LOCALE_ONLY 			0
#endif

#ifndef _EWL_FLOATING_POINT_PRINTF_SCANF
	#define _EWL_FLOATING_POINT_PRINTF_SCANF 		1
#endif

#ifndef _EWL_C99_PRINTF_SCANF
	#define _EWL_C99_PRINTF_SCANF 		1
#endif

#ifndef _EWL_USES_EXIT_PROCESS
	#define _EWL_USES_EXIT_PROCESS 		0
#endif

#ifndef _EWL_USING_CW_C_HEADERS_
	#define _EWL_USING_CW_C_HEADERS_ 	1
#endif

#ifndef _EWL_PUT_THIRD_PARTY_C_IN_STD
	#define _EWL_PUT_THIRD_PARTY_C_IN_STD 0
#endif

#ifndef _EWL_IMP_EXP
	#define _EWL_IMP_EXP
#endif

#ifndef _EWL_IMP_EXP_C
	#define _EWL_IMP_EXP_C _EWL_IMP_EXP
#endif

#ifndef _EWL_IMP_EXP_RUNTIME
	#define _EWL_IMP_EXP_RUNTIME _EWL_IMP_EXP
#endif

#ifndef _EWL_MALLOC_IS_ALTIVEC_ALIGNED
	#define _EWL_MALLOC_IS_ALTIVEC_ALIGNED 0
#endif

#ifndef _EWL_WFILEIO_AVAILABLE
	#define _EWL_WFILEIO_AVAILABLE 		0
#endif

#ifndef _EWL_FLOAT_HEX
	#if _EWL_C99
		#define _EWL_FLOAT_HEX 		1
	#else
		#define _EWL_FLOAT_HEX 		0
	#endif
#endif

#ifndef _EWL_CDECL
	#define _EWL_CDECL
#endif

#ifndef _EWL_MATH_CDECL
	#define _EWL_MATH_CDECL
#endif

#ifndef _EWL_WEAK
		#define _EWL_WEAK		__attribute__((weak))
#endif

#ifndef _EWL_NORETURN
		#define _EWL_NORETURN		__attribute__ ((__noreturn__))
#endif

#ifndef _EWL_NAKED
	#define _EWL_NAKED		__attribute__((naked))
#endif

#ifndef _EWL_INLINE
	#if defined(__GNUC__)
		#define _EWL_INLINE			__attribute__((weak))
	#elif defined(_GHSPORT_)
		#define _EWL_INLINE 		inline
	#else
		#define _EWL_INLINE 		__inline
	#endif
#endif

#ifndef _EWL_DO_NOT_INLINE
	#if defined(__GNUC__)
		#define _EWL_DO_NOT_INLINE 	__attribute__((__noinline__))
	#elif defined(_GHSPORT_)
		#define _EWL_DO_NOT_INLINE
	#else
		#define _EWL_DO_NOT_INLINE 	__attribute__((noinline))
	#endif
#endif

#ifndef _EWL_LOCALDATA
	#define _EWL_LOCALDATA(_a) 			_a
#endif

#ifndef _EWL_LOCALDATA_AVAILABLE
	#define _EWL_LOCALDATA_AVAILABLE 	1
#endif

#ifndef _EWL_RESTRICT
	#if defined(__CWCC__)
		#if __option(c99)
			#define _EWL_RESTRICT	restrict
		#else
			#define _EWL_RESTRICT   __restrict
		#endif
	#elif defined(__GNUC__)
		#define _EWL_RESTRICT   __restrict
	#else
		#define _EWL_RESTRICT
	#endif
#endif

#ifndef _EWL_COMPLEX_SUPPORT
	#if defined(__CWCC__) && __CWCC__>=0x4100
		#if __has_feature(C99_COMPLEX) && __option(c99)
			#define _EWL_COMPLEX_SUPPORT 1
		#else
			#define _EWL_COMPLEX_SUPPORT 0
		#endif
	#elif defined(__GNUC__) && (__STDC_VERSION__ >= 199901L)
		#define _EWL_COMPLEX_SUPPORT	1
	#else
		#define _EWL_COMPLEX_SUPPORT	0
	#endif
#endif

/*
   This macro is set to MATH_ERRNO (the value 1) by default.  If a particular platform
   has different math requirements, it should set _EWL_MATH_ERRHANDLING to the appropriate
   value in the platform prefix file.  C99 requires that MATH_ERRHANDLING (and thus
   _EWL_MATH_ERRHANDLING) be a value of MATH_ERRNO (1), MATH_ERREXCEPT (2), or
   MATH_ERRNO | MATH_ERREXCEPT (3).  No other values are valid.
*/

#ifndef _EWL_MATH_ERRHANDLING
	#ifndef _EWL_C99
		#define _EWL_MATH_ERRHANDLING 3		/* Define to MATH_ERRNO | MATH_ERREXCEPT by default */
	#else
		#define _EWL_MATH_ERRHANDLING 1		/* Define to MATH_ERRNO by default */
	#endif
#endif

#ifndef _EWL_LITTLE_ENDIAN
	#if defined(__CWCC__)
		#if __option(little_endian)
			#define _EWL_LITTLE_ENDIAN 1
		#else
			#define _EWL_LITTLE_ENDIAN 0
		#endif
	#elif defined(__GNUC__)
		#if defined(__arm__)
				#if defined(__ARMEL__)
					#define _EWL_LITTLE_ENDIAN 1
				#else
					#define _EWL_LITTLE_ENDIAN 0
				#endif
		#endif
	#else
		//other architecture: please define your default
		#define _EWL_LITTLE_ENDIAN	0
	#endif
#endif

#ifndef _EWL_TR1_NAMESPACE
	#ifdef _EWL_NO_CPP_NAMESPACE
		#define _EWL_TR1_NAMESPACE 0
	#else
		#define _EWL_TR1_NAMESPACE 1
	#endif
#endif  /* _EWL_TR1_NAMESPACE */

#ifdef __cplusplus
	#define _EWL_BEGIN_EXTERN_C 	extern "C" {
	#define _EWL_END_EXTERN_C 		}
	#define _EWL_BEGIN_EXTERN_CPP 	extern "C++" {
	#define _EWL_END_EXTERN_CPP 		}

	#ifdef _EWL_USING_NAMESPACE
		#define _EWL_BEGIN_NAMESPACE_STD namespace std {
		#define _EWL_END_NAMESPACE_STD }

		#define __std(ref) 			::std::ref
		#define __global() 			::
	#else
		#define _EWL_BEGIN_NAMESPACE_STD
		#define _EWL_END_NAMESPACE_STD

		#define __std(ref) 			ref
		#define __global()
	#endif
#else
	#define _EWL_BEGIN_EXTERN_C
	#define _EWL_END_EXTERN_C

	#define _EWL_BEGIN_NAMESPACE_STD
	#define _EWL_END_NAMESPACE_STD

	#define __std(ref)				ref
	#define __global()
#endif

/*
	Set up no throw specifications for C headers when compiled under C++
*/
#if !defined(_EWL_NO_THROW_SPECS)
	#ifndef _EWL_NO_THROW
		#if defined(__cplusplus)
			#define _EWL_NO_THROW	throw()
		#else
			#define _EWL_NO_THROW
		#endif
	#endif
	#ifndef _EWL_CANT_THROW
		#if defined(__GNUC__)
			#define _EWL_CANT_THROW __attribute__((__nothrow__))
		#elif defined(_GHSPORT_)
			#define _EWL_CANT_THROW
		#else
			#define _EWL_CANT_THROW __attribute__((nothrow))
		#endif
	#endif
#else
	#ifndef _EWL_NO_THROW
		#define _EWL_NO_THROW
	#endif
	#ifndef _EWL_CANT_THROW
		#define _EWL_CANT_THROW
	#endif
#endif

#ifndef _EWL_ATTRIBUTE_CONST
	#define _EWL_ATTRIBUTE_CONST __attribute__((const))
#endif

#if !defined(_EWL_USE_INLINE)
	#if defined(__CWCC__)
		#if !__option(dont_inline)
			#define _EWL_USE_INLINE		1
		#endif
	#elif defined(__GNUC__)
		#define _EWL_USE_INLINE			1
	#else
		#define _EWL_USE_INLINE			0
	#endif
#endif /* !defined(_EWL_USE_INLINE) */

#ifndef __has_intrinsic
	#define __has_intrinsic(_a)		0
#endif

#ifndef _AEABI_PORTABILITY_LEVEL
	#define _AEABI_PORTABILITY_LEVEL  	0
#endif


#ifndef _EWL_BF_ORDER
	/* this is a hook for __attribute(bitfield order) which allows
	 * to define for a structure a default bitfield bit ordering,
	 * typically this allows the struct to be bit ordering agnostic.
	 */
	#define _EWL_BF_ORDER
#endif

#ifndef _EWL_IEC_559_ADDITIONS
	#define _EWL_IEC_559_ADDITIONS 0
#endif

#ifndef _EWL_INCOMPLETE_IEEE_EXCEPTION_MODEL
	/*
		Define to 1 if your hardware floating point instructions
		generate inaccuracies for multiplication and addition
		of subnormals, infinities, and very big or tiny
		normals.  In math functions that have issues with these
		numbers, we have substituted calls to fma and fmaf.  fma
		and fmaf have been modified to handle these numbers in
		software.  If the fma and/or fmaf functions simply map
		to a hardware instruction, this macro will have no
		discernible effect.
	*/
	#define _EWL_INCOMPLETE_IEEE_EXCEPTION_MODEL 0
#endif

#ifndef _EWL_SUPPORTS_PRINTING_QUIET_NANS
	#define _EWL_SUPPORTS_PRINTING_QUIET_NANS 1
#endif

/*	Misra EXCEPTIONS that are used in headers files, see also <ewl_misra_types.h> */

#define _MISRA_QUIET_DO_WHILE_0()			/*lint -e{717} allow "do { ... } while(0)" */

#define _MISRA_EXCEPTION_STD_TYPE()			/*lint -e(970) [MISRA 2004 Rule 6.3] Use of modifier or type outside of a typedef */
#define _MISRA_EXCEPTION_RULE_19_6()		/*lint -save -e960 MISRA 2004 Rule 19.6: Use of '#undef' is discouraged */
#define _MISRA_EXCEPTION_FLOAT_CAST()		/*lint -e(740) -e(826) -e(929) -e(970) exceptions to allow bit pattern -> floating point casts */
#define _MISRA_EXCEPTION_CMATH_MACROS()		/*lint -emacro(506 970, signbit, fpclassify) */
#define _MISRA_EXCEPTION_STDIO_MACROS()		/*lint -emacro(929 960 970, __getc, __putc) */
#define _MISRA_EXCEPTION_MATHAPI_MACROS()	/*lint -emacro(929, GET_DOUBLE_HI_WORD, GET_DOUBLE_UHI_WORD, GET_DOUBLE_LO_WORD, GET_DOUBLE_ULO_WORD) */
#define _MISRA_EXCEPTION_MATHAPISP_MACROS()	/*lint -emacro(929, GET_FLOAT_WORD, GET_FLOAT_UWORD, SET_FLOAT_WORD, SET_FLOAT_UWORD) */
#define _MISRA_EXCEPTION_STDARG_MACROS()	/*lint -emacro( (826) , __va_start) -emacro(923 926 928 970, __va_start) */
#define _MISRA_RESTORE()					/*lint -restore Used to restore saved options */

#endif /* ifndef _EWL_ANSI_PARMS_H */
