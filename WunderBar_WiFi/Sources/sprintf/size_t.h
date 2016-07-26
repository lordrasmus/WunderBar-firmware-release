/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/06/01 15:39:39 $
 * $Revision: 1.1 $
 */

#ifndef _EWL_SIZE_T_H
#define _EWL_SIZE_T_H

#include "ansi_parms.h"

_EWL_BEGIN_NAMESPACE_STD
_EWL_BEGIN_EXTERN_C

	#ifndef _EWL_SIZE_T_TYPE
		#define _EWL_SIZE_T_TYPE __typeof__(sizeof(0))
	#endif

	#ifndef _EWL_SIZE_T_DEFINED
		#define _EWL_SIZE_T_DEFINED
		typedef _EWL_SIZE_T_TYPE size_t;
	#endif

_EWL_END_EXTERN_C
_EWL_END_NAMESPACE_STD

#if __cplusplus
using std::size_t;
#endif

#endif /* _EWL_SIZE_T_H */

