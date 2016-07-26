/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/06/01 15:39:39 $
 * $Revision: 1.1 $
 */

#ifndef _EWL_VA_LIST_H
#define _EWL_VA_LIST_H

_EWL_BEGIN_NAMESPACE_STD
_EWL_BEGIN_EXTERN_C

#if defined(__GNUC__)
	typedef __builtin_va_list va_list;
	#define _EWL_VA_LIST_TYPE va_list;
	#define _EWL_VA_LIST_DEFINED
#elif defined(_GHSPORT_)
	#define _EWL_VA_LIST_DEFINED
#else

	#ifndef _EWL_VA_LIST_TYPE
		#define _EWL_VA_LIST_TYPE char *
	#endif

	#ifndef _EWL_VA_LIST_DEFINED
		typedef _EWL_VA_LIST_TYPE va_list;
		#define _EWL_VA_LIST_DEFINED
	#endif
#endif /* defined(__GNUC__) */

_EWL_END_EXTERN_C
_EWL_END_NAMESPACE_STD

#endif /* _EWL_VA_LIST_H */
