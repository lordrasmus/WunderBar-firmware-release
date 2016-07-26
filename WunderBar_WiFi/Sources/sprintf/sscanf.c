/* EWL
 * Copyright © 1995-2007 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2013/02/08 10:47:50 $
 * $Revision: 1.2 $
 */

/*	Routines
 *	--------
 *		sscanf
 */

#include "ansi_parms.h"
_MISRA_EXCEPTION_RULE_19_6()
//#ifdef __STDC_WANT_LIB_EXT1__
// enforce __STDC_WANT_LIB_EXT1__ support
//#undef __STDC_WANT_LIB_EXT1__
//#endif
_MISRA_RESTORE()
//#define __STDC_WANT_LIB_EXT1__ 1

#include "ewl_misra_types.h"
#include <stdarg.h>
#include <stdio.h>


MISRA_EXCEPTION_RULE_16_1()
int_t _EWL_CDECL sscanf(const char_t * _EWL_RESTRICT s, const char_t * _EWL_RESTRICT format, ...)
{
	va_list args;

	va_start( args, format );
	return(vsscanf(s, format, args));
}

