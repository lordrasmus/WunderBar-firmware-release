/* EWL
 * Copyright © 1995-2007 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2013/02/08 10:47:50 $
 * $Revision: 1.2 $
 */

/*	Routines
 *	--------
 *		vsscanf
 */

#include "ansi_parms.h"
_MISRA_EXCEPTION_RULE_19_6()
//#ifdef __STDC_WANT_LIB_EXT1__
/* enforce __STDC_WANT_LIB_EXT1__ support */
//#undef __STDC_WANT_LIB_EXT1__
//#endif
_MISRA_RESTORE()
//#define __STDC_WANT_LIB_EXT1__ 1

#include "ewl_misra_types.h"
#include "stdio_api.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


int_t _EWL_CDECL __StringRead(void * isc, int_t ch, int_t Action)
{
	char_t	RetVal;

	__InStrCtrl * Iscp = (__InStrCtrl*)isc;
	switch(Action)
	{
		case __GetAChar:
			RetVal = *(Iscp->NextChar);
			if (RetVal == '\0') {
				Iscp->NullCharDetected = 1;
				MISRA_EXCEPTION_RULE_14_7()
				return EOF;
			} else {
				Iscp->NextChar++;
				MISRA_EXCEPTION_RULE_14_7()
				return (int_t)(uchar_t)RetVal;
			}

		case __UngetAChar:
			if (!Iscp->NullCharDetected) {
				Iscp->NextChar--;
			} else {
				Iscp->NullCharDetected = 0;
			}
			MISRA_EXCEPTION_RULE_14_7()
			return ch;

		case __TestForError:
			MISRA_EXCEPTION_RULE_14_7()
			return Iscp->NullCharDetected;

		default:
			MISRA_EXCEPTION_RULE_14_7()
			return 0;
	}
}

int_t _EWL_CDECL vsscanf(const char_t * _EWL_RESTRICT s, const char_t * _EWL_RESTRICT format, va_list arg)
{
	__InStrCtrl isc;

	MISRA_EXCEPTION_RULE_11_4()
	isc.NextChar         = (char_t *)s;
	if ((s == NULL) || (*isc.NextChar == '\0')) {
		MISRA_EXCEPTION_RULE_14_7()
		return(EOF);
	}
	isc.NullCharDetected = 0;

	return(__sformatter(__StringRead, (void *)&isc, format, arg, 0));
}

