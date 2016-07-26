/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/06/01 15:39:37 $
 * $Revision: 1.1 $
 */

#ifndef _EWL_CSTDDEF
#define _EWL_CSTDDEF

#include "ansi_parms.h"

#if !_EWL_USING_CW_C_HEADERS_
	#include <stddef.h>

	#if _EWL_PUT_THIRD_PARTY_C_IN_STD
		#include <ewl_cpp_std_stddef.h>
	#endif
#else

#include "null.h"
#include "size_t.h"
#include "wchar_t.h"
#include "ewl_lib_ext1.h"
#include "ewl_rsize_t.h"

#ifndef _EWL_OFFSETOF

#if __CWCC__>=0x4301
	#define _EWL_OFFSETOF(type, member)	(__builtin_force_const((__std(size_t)) &(((type *) 0)->member)))
#else
	#define _EWL_OFFSETOF(type, member)	((__std(size_t)) &(((type *) 0)->member))
#endif

#endif

#ifndef offsetof
	#define offsetof(type, member) _EWL_OFFSETOF(type, member)
#endif

_EWL_BEGIN_NAMESPACE_STD
_EWL_BEGIN_EXTERN_C

	#ifndef _EWL_PTRDIFF_T_TYPE
		#define _EWL_PTRDIFF_T_TYPE __typeof__((char*)0-(char*)0)
	#endif

	typedef _EWL_PTRDIFF_T_TYPE ptrdiff_t;

_EWL_END_EXTERN_C
_EWL_END_NAMESPACE_STD

#if defined(_EWL_EXPORT_CNAMES) && defined(__cplusplus) && defined(_EWL_USING_NAMESPACE)
#include <stddef.h>
#endif /*  defined(_EWL_EXPORT_CNAMES) && defined(__cplusplus) && defined(_EWL_USING_NAMESPACE) */

#endif /* _EWL_USING_CW_C_HEADERS_ */

#endif /* _EWL_CSTDDEF */
