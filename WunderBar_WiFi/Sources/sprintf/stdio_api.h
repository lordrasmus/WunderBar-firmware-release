/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/06/01 15:39:39 $
 * $Revision: 1.1 $
 */

#ifndef _EWL_STDIO_API_H
#define _EWL_STDIO_API_H

#include "ansi_parms.h"

#include "size_t.h"

#if _EWL_WIDE_CHAR
	#include <wchar_t.h>
	#include <file_struc.h>
#endif /* _EWL_WIDE_CHAR */

#include "null.h"
#include "eof.h"
#include <stdarg.h>
#include "file_struc.h"

_EWL_BEGIN_NAMESPACE_STD
_EWL_BEGIN_EXTERN_C

#pragma pack(push,4)

//_EWL_IMP_EXP_C size_t _EWL_CDECL	__fread(void *, size_t, size_t, FILE *) _EWL_CANT_THROW;
//_EWL_IMP_EXP_C size_t _EWL_CDECL	__fwrite(const void *, size_t, size_t, FILE *) _EWL_CANT_THROW;
//_EWL_IMP_EXP_C fpos_t _EWL_CDECL   	_ftell(FILE * _EWL_RESTRICT) _EWL_CANT_THROW;
//_EWL_IMP_EXP_C int _EWL_CDECL     	_fseek(FILE *, fpos_t, int) _EWL_CANT_THROW;
//_EWL_IMP_EXP_C int _EWL_CDECL		__get_char(FILE *) _EWL_CANT_THROW;
//_EWL_IMP_EXP_C int _EWL_CDECL		__put_char(int, FILE *) _EWL_CANT_THROW;

#if _EWL_WIDE_CHAR
_EWL_IMP_EXP_C int _EWL_CDECL		__fwide(FILE *, int) _EWL_CANT_THROW;
#endif

void * _EWL_CDECL __FileWrite(void *, const char *, size_t) _EWL_CANT_THROW;
int    _EWL_CDECL __FileRead(void *, int, int) _EWL_CANT_THROW;

#if !_EWL_CONSOLE_FILE_IS_DISK_FILE && _EWL_CONSOLE_SUPPORT
int    _EWL_CDECL __ConsoleRead(void *, int, int) _EWL_CANT_THROW;
void * _EWL_CDECL __ConsoleWrite(void *, const char *, size_t) _EWL_CANT_THROW;
#endif /* !_EWL_CONSOLE_FILE_IS_DISK_FILE && _EWL_CONSOLE_SUPPORT */

//int _EWL_CDECL __ungetc(int, FILE *) _EWL_CANT_THROW;

#if _EWL_WIDE_CHAR
	wchar_t _EWL_CDECL __fgetwc(FILE *) _EWL_CANT_THROW;
	wchar_t _EWL_CDECL __getwchar(void) _EWL_CANT_THROW;
	wchar_t _EWL_CDECL __ungetwc(wchar_t, FILE *) _EWL_CANT_THROW;
	void * 	_EWL_CDECL __wFileWrite(void *, const wchar_t *, size_t) _EWL_CANT_THROW;

#if _EWL_C99
	wint_t 	__wFileRead(void *, wint_t, int) _EWL_CANT_THROW;
#endif /* _EWL_C99 */

#endif /* _EWL_WIDE_CHAR */

_MISRA_EXCEPTION_STDIO_MACROS()

#if _EWL_WIDE_CHAR
	#define __getc(_file) \
		((__fwide((FILE *)_file, -1) >= 0) ? EOF : ((_file)->buffer_len-- ? (int)  *(_file)->buffer_ptr++ : __std(__get_char)((FILE *)_file)))
	#define __putc(_c, _file) \
		((__fwide((FILE *)_file, -1) >= 0) ? EOF : ((_file)->buffer_len-- ? (int) (*(_file)->buffer_ptr++ = (unsigned char)(_c)) : __std(__put_char)(_c, (FILE *)_file)))
#else
	#define __getc(_file)  \
		((_file)->buffer_len-- ? (int)  *(_file)->buffer_ptr++ : __std(__get_char)((FILE *)_file))
	#define __putc(_c, _file) \
		((_file)->buffer_len-- ? (int) (*(_file)->buffer_ptr++ = (unsigned char)(_c)) : __std(__put_char)(_c, (FILE *)_file))
#endif /* _EWL_WIDE_CHAR && _EWL_C99 */

#if _EWL_WFILEIO_AVAILABLE
	_EWL_IMP_EXP_C FILE * _EWL_CDECL __whandle_open(__file_handle handle, const wchar_t * mode) _EWL_CANT_THROW;
	FILE * _EWL_CDECL __whandle_reopen(__file_handle handle, const wchar_t * mode, FILE *) _EWL_CANT_THROW;
#endif /* _EWL_WFILEIO_AVAILABLE */

typedef struct
{
	char * CharStr;
	__std(size_t) MaxCharCount;
	__std(size_t) CharsWritten;
} __OutStrCtrl;

typedef struct
{
	const char * NextChar;
	int    NullCharDetected;
} __InStrCtrl;

#if _EWL_WIDE_CHAR
_EWL_IMP_EXP_C __std(size_t) _EWL_CDECL	__fwrite(const void *, __std(size_t), __std(size_t), FILE *) _EWL_CANT_THROW;
#endif

void * _EWL_CDECL __StringWrite(void *, const char *, size_t) _EWL_CANT_THROW;
int    _EWL_CDECL __StringRead(void *, int, int) _EWL_CANT_THROW;
int    _EWL_CDECL __FileRead(void *File, int ch, int Action);
void * _EWL_CDECL __FileWrite(void *File, const char * Buffer, size_t NumChars);


int __sformatter(int (_EWL_CDECL *ReadProc)(void *a, int b, int c),
						void		 *ReadProcArg,
						const char *format_str,
						va_list		  arg,
						int		  is_secure);

int __pformatter(void *(_EWL_CDECL *WriteProc)(void *a, const char *b, size_t c),
                        void 		 * 	WriteProcArg,
                        const char * 	_EWL_RESTRICT format_str,
                        va_list 		arg,
                        int 			is_secure);

enum __ReadProcActions
{
	__GetAChar,
	__UngetAChar,
	__TestForError
};

#if _EWL_WIDE_CHAR
typedef struct
{
	wchar_t * wCharStr;
	size_t MaxCharCount;
	size_t CharsWritten;
} __wOutStrCtrl;

typedef struct
{
	wchar_t * wNextChar;
	int    wNullCharDetected;
} __wInStrCtrl;

enum __WReadProcActions
{
	__GetAwChar,
	__UngetAwChar,
	__TestForwcsError
};

void * _EWL_CDECL __wStringWrite(void *, const wchar_t *, size_t) _EWL_CANT_THROW;
wint_t __wStringRead(void *, wint_t, int) _EWL_CANT_THROW;

#endif /* _EWL_WIDE_CHAR */

#pragma pack(pop)

_EWL_END_EXTERN_C
_EWL_END_NAMESPACE_STD

#endif /* _EWL_STDIO_API_H */
