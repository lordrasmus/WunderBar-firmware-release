/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2009/05/14 16:55:29 $
 * $Revision: 1.1 $
 */

#ifndef _EWL_STRTOUL_H
#define _EWL_STRTOUL_H

#include "ansi_parms.h"

#include "cstdio.h"

_EWL_BEGIN_EXTERN_C

	extern unsigned long _EWL_CDECL __strtoul(int base,
								 			  int max_width,
											  int (_EWL_CDECL *ReadProc)(void *, int, int),
											  void * ReadProcArg,
											  int  * chars_scanned,
											  int  * negative,
											  int  * overflow) _EWL_CANT_THROW;

	#if _EWL_LONGLONG
		extern unsigned long long _EWL_CDECL __strtoull(	int		base,
												int		max_width,
												int (_EWL_CDECL *ReadProc)(void *, int, int),
												void * ReadProcArg,
												int	* chars_scanned,
												int	* negative,
												int	* overflow) _EWL_CANT_THROW;
	#endif   /*_EWL_LONGLONG*/

_EWL_END_EXTERN_C

#endif /* _EWL_STRTOUL_H */
