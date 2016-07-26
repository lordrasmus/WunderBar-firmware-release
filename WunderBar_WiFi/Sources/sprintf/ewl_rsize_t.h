/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/06/01 15:39:38 $
 * $Revision: 1.1 $
 */

#if defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__

#ifndef _EWL_RSIZE_T_H
#define _EWL_RSIZE_T_H

#include "size_t.h"

_EWL_BEGIN_NAMESPACE_STD
_EWL_BEGIN_EXTERN_C

	#ifndef _EWL_RSIZE_T_TYPE
		#define _EWL_RSIZE_T_TYPE size_t
	#endif

	#ifndef _EWL_RSIZE_T_DEFINED
		#define _EWL_RSIZE_T_DEFINED
		typedef _EWL_RSIZE_T_TYPE rsize_t;
	#endif

_EWL_END_EXTERN_C
_EWL_END_NAMESPACE_STD

#endif /* _EWL_RSIZE_T_H */

#endif /* __STDC_WANT_LIB_EXT1__ */
