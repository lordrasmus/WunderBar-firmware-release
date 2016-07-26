/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/07/12 22:31:13 $
 * $Revision: 1.2 $
 */

#ifndef _EWL_CSTDIO
#define _EWL_CSTDIO

#include "ansi_parms.h"

#if !_EWL_USING_CW_C_HEADERS_
	#include <stdio.h>

	#if _EWL_PUT_THIRD_PARTY_C_IN_STD
		#include <ewl_cpp_std_stdio.h>
	#endif
#else

#include "cstddef.h"
#include "null.h"
#include "eof.h"
#include "va_list.h"
#include "ewl_lib_ext1.h"
#include "ewl_rsize_t.h"
#include "file_struc.h"
#include "stdio_api.h"

#if _EWL_POSIX
	#include <stdio.posix.h>
#endif

#pragma pack(push,4)

_EWL_BEGIN_NAMESPACE_STD
_EWL_BEGIN_EXTERN_C

	/** Except that it returns no value, the setbuf function is equivalent to the setvbuf
	 *	function invoked with the values _IOFBF for mode and BUFSIZ for size, or (if buf
	 *	is a null pointer), with the value _IONBF for mode.
	 */
_EWL_IMP_EXP_C void _EWL_CDECL			setbuf(FILE * _EWL_RESTRICT stream, char * _EWL_RESTRICT buf) _EWL_CANT_THROW;

	/** The setvbuf function may be used only after the stream pointed to by stream has
	 *	been associated with an open file and before any other operation (other than an
	 *	unsuccessful call to setvbuf) is performed on the stream. The argument mode
	 *	determines how stream will be buffered, as follows: _IOFBF causes input/output to be
	 *	fully buffered; _IOLBF causes input/output to be line buffered; _IONBF causes
	 *	input/output to be unbuffered. If buf is not a null pointer, the array it points to may be
	 *	used instead of a buffer allocated by the setvbuf function and the argument size
	 *	specifies the size of the array; otherwise, size may determine the size of a buffer
	 *	allocated by the setvbuf function. The contents of the array at any time are
	 *	indeterminate.
	 *	The setvbuf function returns zero on success, or nonzero if an invalid value is given
	 *	for mode or if the request cannot be honored.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			setvbuf(FILE * _EWL_RESTRICT stream, char * _EWL_RESTRICT buf, int mode, size_t size) _EWL_CANT_THROW;

	/** A successful call to the fclose function causes the stream pointed to by stream to be
	 *	flushed and the associated file to be closed. Any unwritten buffered data for the stream
	 *	are delivered to the host environment to be written to the file; any unread buffered data
	 *	are discarded. Whether or not the call succeeds, the stream is disassociated from the file
	 *	and any buffer set by the setbuf or setvbuf function is disassociated from the stream.
	 *	Returns zero if the stream was successfully closed, or EOF if any errors were detected.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fclose(FILE *stream) _EWL_CANT_THROW;

	/** If stream points to an output stream or an update stream in which the most recent
	 *	operation was not input, the fflush function causes any unwritten data for that stream
	 *	to be delivered to the host environment to be written to the file; otherwise, the behavior is
	 *	undefined.
	 *	If the stream is a null pointer, the fflush function performs this flushing action on all
	 *	streams for which the behavior is defined above.
	 *	The fflush function sets the error indicator for the stream and returns EOF if a write
	 *	error occurs, otherwise it returns zero.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fflush(FILE *stream) _EWL_CANT_THROW;

	/** The fscanf function reads input from the stream pointed to by stream, under control
	 *	of the string pointed to by format that specifies the admissible input sequences and how
	 *	they are to be converted for assignment, using subsequent arguments as pointers to the
	 *	objects to receive the converted input. If there are insufficient arguments for the format,
	 *	the behavior is undefined. If the format is exhausted while arguments remain, the excess
	 *	arguments are evaluated (as always) but are otherwise ignored.
	 *	
	 *	The format shall be a multibyte character sequence, beginning and ending in its initial
	 *	shift state. The format is composed of zero or more directives: one or more white-space
	 *	characters, an ordinary multibyte character (neither '%' nor a white-space character), or a
	 *	conversion specification. Each conversion specification is introduced by the character '%'.
	 *
	 *	After the '%', the following appear in sequence:
	 *
	 *		An optional assignment-suppressing character '*'.
	 *
	 *		An optional decimal integer greater than zero that specifies the maximum field width
	 *			(in characters).
	 *
	 *		An optional length modifier that specifies the size of the receiving object.
	 *
	 *		A conversion specifier character that specifies the type of conversion to be applied.
	 *
	 *	The fscanf function executes each directive of the format in turn. If a directive fails, as
	 *	detailed below, the function returns. Failures are described as input failures (due to the
	 *	occurrence of an encoding error or the unavailability of input characters), or matching
	 *	failures (due to inappropriate input).
	 *
	 *	A directive composed of white-space character(s) is executed by reading input up to the
	 *	first non-white-space character (which remains unread), or until no more characters can
	 *	be read.
	 *
	 *	A directive that is an ordinary multibyte character is executed by reading the next
	 *	characters of the stream. If any of those characters differ from the ones composing the
	 *	directive, the directive fails and the differing and subsequent characters remain unread.
	 *	Similarly, if end-of-file, an encoding error, or a read error prevents a character from being
	 *	read, the directive fails.
	 *
	 *	A directive that is a conversion specification defines a set of matching input sequences, as
	 *	described below for each specifier. A conversion specification is executed in the
	 *	following steps:
	 *
	 *	Input white-space characters (as specified by the isspace function) are skipped, unless
	 *	the specification includes a '[', 'c', or 'n' specifier.
	 *
	 *	An input item is read from the stream, unless the specification includes an n specifier. An
	 *	input item is defined as the longest sequence of input characters which does not exceed
	 *	any specified field width and which is, or is a prefix of, a matching input sequence.
	 *	The first character, if any, after the input item remains unread. If the length of the input
	 *	item is zero, the execution of the directive fails; this condition is a matching failure unless
	 *	end-of-file, an encoding error, or a read error prevented input from the stream, in which
	 *	case it is an input failure.
	 *
	 *	Except in the case of a '%' specifier, the input item (or, in the case of a %n directive, the
	 *	count of input characters) is converted to a type appropriate to the conversion specifier. If
	 *	the input item is not a matching sequence, the execution of the directive fails: this
	 *	condition is a matching failure. Unless assignment suppression was indicated by a '*', the
	 *	result of the conversion is placed in the object pointed to by the first argument following
	 *	the format argument that has not already received a conversion result. If this object
	 *	does not have an appropriate type, or if the result of the conversion cannot be represented
	 *	in the object, the behavior is undefined.
	 *
	 *	The length modifiers and their meanings are:
	 *
	 *	 hh Specifies that a following 'd', 'i', 'o', 'u', 'x', 'X', or 'n' conversion specifier applies
	 *		to an argument with type pointer to signed char or unsigned char.
	 *
	 *	 h Specifies that a following 'd', 'i', 'o', 'u', 'x', 'X', or 'n' conversion specifier applies
	 *		to an argument with type pointer to short int or unsigned short int.
	 *
	 *	 l Specifies that a following 'd', 'i', 'o', 'u', 'x', 'X', or 'n' conversion specifier applies
	 *		to an argument with type pointer to long int or unsigned long
	 *		int; that a following 'a', 'A', 'e', 'E', 'f', 'F', 'g', or 'G' conversion specifier applies to
	 *		an argument with type pointer to double; or that a following 'c', 's', or '['
	 *		conversion specifier applies to an argument with type pointer to wchar_t.
	 *
	 *	 ll Specifies that a following 'd', 'i', 'o', 'u', 'x', 'X', or 'n' conversion specifier applies
	 *		to an argument with type pointer to long long int or unsigned
	 *		long long int.
	 *		(Available only when _EWL_C99_PRINTF_SCANF or _EWL_LONGLONG_PRINTF_SCANF is set).
	 *
	 *   j Specifies that a following 'd', 'i', 'o', 'u', 'x', 'X', or 'n' conversion specifier applies
	 *		to an argument with type pointer to intmax_t or uintmax_t.
	 *		(Available only when _EWL_C99_PRINTF_SCANF is set).
	 *
	 *	 z Specifies that a following 'd', 'i', 'o', 'u', 'x', 'X', or 'n' conversion specifier applies
	 *		to an argument with type pointer to size_t or the corresponding signed
	 *		integer type.
	 *		(Available only when _EWL_C99_PRINTF_SCANF is set).
	 *
	 *	 t Specifies that a following 'd', 'i', 'o', 'u', 'x', 'X', or 'n' conversion specifier applies
	 *		to an argument with type pointer to ptrdiff_t or the corresponding
	 *		unsigned integer type.
	 *		(Available only when _EWL_C99_PRINTF_SCANF is set).
	 *
	 * 	 L Specifies that a following 'a', 'A', 'e', 'E', 'f', 'F', 'g', or 'G' conversion specifier
	 *		applies to an argument with type pointer to long double.
	 *		(Available only when _EWL_FLOATING_POINT_PRINTF_SCANF is set).
	 *
	 *	If a length modifier appears with any conversion specifier other than as specified above,
	 *	the behavior is undefined.
	 *
	 *	The conversion specifiers and their meanings are:
	 *
	 *	 d Matches an optionally signed decimal integer, whose format is the same as
	 *		expected for the subject sequence of the strtol function with the value 10
	 *		for the base argument. The corresponding argument shall be a pointer to
	 *		signed integer.
	 *
	 *	 i Matches an optionally signed integer, whose format is the same as expected
	 *		for the subject sequence of the strtol function with the value 0 for the
	 *		base argument. The corresponding argument shall be a pointer to signed
	 *		integer.
	 *
	 *	 o Matches an optionally signed octal integer, whose format is the same as
	 *		expected for the subject sequence of the strtoul function with the value 8
	 *		for the base argument. The corresponding argument shall be a pointer to
	 *		unsigned integer.
	 *
	 *	 u Matches an optionally signed decimal integer, whose format is the same as
	 *		expected for the subject sequence of the strtoul function with the value 10
	 *		for the base argument. The corresponding argument shall be a pointer to
	 *		unsigned integer.
	 *
	 *	 x Matches an optionally signed hexadecimal integer, whose format is the same
	 *		as expected for the subject sequence of the strtoul function with the value
	 *		16 for the base argument. The corresponding argument shall be a pointer to
	 *		unsigned integer.
	 *
	 *	 a,e,f,g Matches an optionally signed floating-point number, infinity, or NaN, whose
	 *		format is the same as expected for the subject sequence of the strtod
	 *		function. The corresponding argument shall be a pointer to floating.
	 *		(Available only when _EWL_FLOATING_POINT_PRINTF_SCANF is set).
	 *
	 *	 c Matches a sequence of characters of exactly the number specified by the field
	 *		width (1 if no field width is present in the directive).
	 *
	 *		If no 'l' length modifier is present, the corresponding argument shall be a
	 *		pointer to the initial element of a character array large enough to accept the
	 *		sequence. No null character is added.
	 *
	 *		If an 'l' length modifier is present, the input shall be a sequence of multibyte
	 *		characters that begins in the initial shift state. Each multibyte character in the
	 *		sequence is converted to a wide character as if by a call to the mbrtowc
	 *		function, with the conversion state described by an mbstate_t object
	 *		initialized to zero before the first multibyte character is converted. The
	 *		corresponding argument shall be a pointer to the initial element of an array of
	 *		wchar_t large enough to accept the resulting sequence of wide characters.
	 *		No null wide character is added.
	 *		(Available only when _EWL_WIDE_CHAR is set).
	 *
	 *	 s Matches a sequence of non-white-space characters.
	 *
	 *		If no 'l' length modifier is present, the corresponding argument shall be a
	 *		pointer to the initial element of a character array large enough to accept the
	 *		sequence and a terminating null character, which will be added automatically.
	 *
	 *		If an 'l' length modifier is present, the input shall be a sequence of multibyte
	 *		characters that begins in the initial shift state. Each multibyte character is
	 *		converted to a wide character as if by a call to the mbrtowc function, with
	 *		the conversion state described by an mbstate_t object initialized to zero
	 *		before the first multibyte character is converted. The corresponding argument
	 *		shall be a pointer to the initial element of an array of wchar_t large enough
	 *		to accept the sequence and the terminating null wide character, which will be
	 *		added automatically.
	 *		(Available only when _EWL_WIDE_CHAR is set).
	 *
	 *	 [ Matches a nonempty sequence of characters from a set of expected characters
	 *		(the scanset).
	 *
	 *		If no 'l' length modifier is present, the corresponding argument shall be a
	 *		pointer to the initial element of a character array large enough to accept the
	 *		sequence and a terminating null character, which will be added automatically.
	 *
	 *		If an 'l' length modifier is present, the input shall be a sequence of multibyte
	 *		characters that begins in the initial shift state. Each multibyte character is
	 *		converted to a wide character as if by a call to the mbrtowc function, with
	 *		the conversion state described by an mbstate_t object initialized to zero
	 *		before the first multibyte character is converted. The corresponding argument
	 *		shall be a pointer to the initial element of an array of wchar_t large enough
	 *		to accept the sequence and the terminating null wide character, which will be
	 *		added automatically.
	 *		(Available only when _EWL_WIDE_CHAR is set).
	 *
	 *		The conversion specifier includes all subsequent characters in the format
	 *		string, up to and including the matching right bracket (]). The characters
	 *		between the brackets (the scanlist) compose the scanset, unless the character
	 *		after the left bracket is a circumflex (^), in which case the scanset contains all
	 *		characters that do not appear in the scanlist between the circumflex and the
	 *		right bracket. If the conversion specifier begins with [] or [^], the right
	 *		bracket character is in the scanlist and the next following right bracket
	 *		character is the matching right bracket that ends the specification; otherwise
	 *		the first following right bracket character is the one that ends the
	 *		specification. If a '- 'character is in the scanlist and is not the first, nor the
	 *		second where the first character is a '^', nor the last character, the behavior is
	 *		implementation-defined.
	 *
	 *	 p Matches an implementation-defined set of sequences, which should be the
	 *		same as the set of sequences that may be produced by the %p conversion of
	 *		the fprintf function. The corresponding argument shall be a pointer to a
	 *		pointer to void. The input item is converted to a pointer value in an
	 *		implementation-defined manner. If the input item is a value converted earlier
	 *		during the same program execution, the pointer that results shall compare
	 *		equal to that value; otherwise the behavior of the %p conversion is undefined.
	 *
	 *	 n No input is consumed. The corresponding argument shall be a pointer to
	 *		signed integer into which is to be written the number of characters read from
	 *		the input stream so far by this call to the fscanf function. Execution of a
	 *		%n directive does not increment the assignment count returned at the
	 *		completion of execution of the fscanf function. No argument is converted,
	 *		but one is consumed. If the conversion specification includes an assignmentsuppressing
	 *		character or a field width, the behavior is undefined.
	 *
	 *	 % Matches a single % character; no conversion or assignment occurs. The
	 *		complete conversion specification shall be %%.
	 *	
	 *	If a conversion specification is invalid, the behavior is undefined.
	 *	
	 *	The conversion specifiers 'A', 'E', 'F', 'G', and 'X' are also valid and behave the same as,
	 *	respectively, 'a', 'e', 'f', 'g', and 'x'.
	 *	
	 *	Trailing white space (including new-line characters) is left unread unless matched by a
	 *	directive. The success of literal matches and suppressed assignments is not directly
	 *	determinable other than via the %n directive.
	 *
	 *	Returns the value of the macro EOF if an input failure occurs before any conversion.
	 *	Otherwise, the function returns the number of input items assigned, which can be fewer
	 *	than provided for, or even zero, in the event of an early matching failure.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fscanf(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;

	/**	The scanf function is equivalent to fscanf with the argument stdin interposed
	 *	before the arguments to scanf.
	 *
	 *	Returns the value of the macro EOF if an input failure occurs before
	 *	any conversion. Otherwise, the scanf function returns the number of input items
	 *	assigned, which can be fewer than provided for, or even zero, in the event of an early
	 *	matching failure.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			scanf(const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;

	/**	The sprintf function is equivalent to fprintf, except that the output is written into
	 *	an array (specified by the arguments) rather than to a stream. A null character is written
	 *	at the end of the characters written; it is not counted as part of the returned value. If
	 *	copying takes place between objects that overlap, the behavior is undefined.
	 *
	 *	Returns the number of characters written in the array, not counting the terminating null
	 *	character, or a neg ative value if an encoding error occurred.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			sprintf(char * _EWL_RESTRICT s, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;

	/**	The snprintf function is equivalent to fprintf, except that the output is written into
	 *	an array (specified by argument s) rather than to a stream. If n is zero, nothing is written,
	 *	and s may be a null pointer. Otherwise, output characters beyond the n-1st are
	 *	discarded rather than being written to the array, and a null character is written at the end
	 *	of the characters actually written into the array. If copying takes place between objects
	 *	that overlap, the behavior is undefined.
	 *
	 *	Returns the number of characters that would have been written had n been sufficiently large,
	 *	not counting the terminating null character, or a negative value if an encoding error occurred.
	 *	Thus, the null-terminated output has been completely written if and only if the returned value
	 *	is nonnegative and less than n.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			snprintf(char * _EWL_RESTRICT s, size_t n, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;

	/**	The sscanf function is equivalent to fscanf, except that input is obtained from a
	 *	string (specified by the argument s) rather than from a stream. Reaching the end of the
	 *	string is equivalent to encountering end-of-file for the fscanf function. If copying
	 *	takes place between objects that overlap, the behavior is undefined.
	 *
	 *	Returns the value of the macro EOF if an input failure occurs before any conversion.
	 *	Otherwise, the sscanf function returns the number of input items assigned, which can be fewer
	 *	than provided for, or even zero, in the event of an early matching failure.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			sscanf(const char * _EWL_RESTRICT s, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;

	/** The vfscanf function is equivalent to fscanf, with the variable argument list
	 *	replaced by arg, which shall have been initialized by the va_start macro (and
	 *	possibly subsequent va_arg calls). The vfscanf function does not invoke the
	 *	va_end macro.
	 *	
	 *	Returns the value of the macro EOF if an input failure occurs before any conversion. 
	 *	Otherwise, the vfscanf function returns the number of input items assigned, which can
	 *	be fewer than provided for, or even zero, in the event of an early matching failure.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			vfscanf(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, va_list va) _EWL_CANT_THROW;

	/**	The vsscanf function is equivalent to sscanf, with the variable argument list
	 *	replaced by arg, which shall have been initialized by the va_start macro (and
	 *	possibly subsequent va_arg calls). The vsscanf function does not invoke the
	 *	va_end macro.
	 *
	 *	Returns the value of the macro EOF if an input failure occurs
	 *	before any conversion. Otherwise, the vsscanf function returns the number of input
	 *	items assigned, which can be fewer than provided for, or even zero, in the event of an
	 *	early matching failure.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			vsscanf(const char * _EWL_RESTRICT s, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;

	/** The vscanf function is equivalent to scanf, with the variable argument list replaced
	 *	by arg, which shall have been initialized by the va_start macro (and possibly
	 *	subsequent va_arg calls). The vscanf function does not invoke the va_end
	 *	macro.
	 *
	 *	Returns the value of the macro EOF if an input failure occurs before any conversion.
	 *	Otherwise, the vscanf function returns the number of input items assigned, which can
	 *	be fewer than provided for, or even zero, in the event of an early matching failure.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			vscanf(const char * _EWL_RESTRICT s, va_list arg) _EWL_CANT_THROW;

	/**	The vfprintf function is equivalent to fprintf, with the variable argument list
	 *	replaced by arg, which shall have been initialized by the va_start macro (and
	 *	possibly subsequent va_arg calls). The vfprintf function does not invoke the
	 *	va_end macro.
	 *
	 *	Returns the number of characters transmitted, or a negative
	 *	value if an output or encoding error occurred.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			vfprintf(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;

	/**	The vprintf function is equivalent to printf, with the variable argument list
	 *	replaced by arg, which shall have been initialized by the va_start macro (and
	 *	possibly subsequent va_arg calls). The vprintf function does not invoke the
	 *	va_end macro.
	 *
	 *	Returns the number of characters transmitted, or a negative value if an output or encoding error occurred.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			vprintf(const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;

	/**	The vsprintf function is equivalent to sprintf, with the variable argument list
	 *	replaced by arg, which shall have been initialized by the va_start macro (and
	 *	possibly subsequent va_arg calls). The vsprintf function does not invoke the
	 *	va_end macro. If copying takes place between objects that overlap, the behavior is
	 *	undefined.
	 *
	 *	Returns the number of characters written in the array, not counting the terminating
	 *	null character, or a neg ative value if an encoding error occurred.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			vsprintf(char * _EWL_RESTRICT s, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;

	/** The vsnprintf function is equivalent to snprintf, with the variable argument list
	 *	replaced by arg, which shall have been initialized by the va_start macro (and
	 *	possibly subsequent va_arg calls). The vsnprintf function does not invoke the
	 *	va_end macro.If copying takes place between objects that overlap, the behavior is
	 *	undefined.
	 *
	 *	Returns the number of characters that would have been written had n been
	 *	sufficiently large, not counting the terminating null character, or a negative
	 *	value if an encoding error occurred. Thus, the null-terminated output has been
	 *	completely written if and only if the returned value is nonnegative and less than n.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			vsnprintf(char * _EWL_RESTRICT s, size_t n, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;

	/**	If the end-of-file indicator for the input stream pointed to by stream is not set and a
	 *	next character is present, the fgetc function obtains that character as an unsigned
	 *	char converted to an int and advances the associated file position indicator for the
	 *	stream (if defined).
	 *
	 *	If the end-of-file indicator for the stream is set, or if the stream is at end-of-file, the
	 *	end-of-file indicator for the stream is set and the fgetc function returns EOF. Otherwise,
	 *	the fgetc function returns the next character from the input stream pointed to by stream.
	 *	If a read error occurs, the error indicator for the stream is set and the fgetc function
	 *	returns EOF.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fgetc(FILE *stream) _EWL_CANT_THROW;

	/**	The fgets function reads at most one less than the number of characters specified by n
	 *	from the stream pointed to by stream into the array pointed to by s. No additional
	 *	characters are read after a new-line character (which is retained) or after end-of-file. A
	 *	null character is written immediately after the last character read into the array.
	 *
	 *	The fgets function returns s if successful. If end-of-file is encountered and no
	 *	characters have been read into the array, the contents of the array remain unchanged and a
	 *	null pointer is returned. If a read error occurs during the operation, the array contents are
	 *	indeterminate and a null pointer is returned.
	 */
_EWL_IMP_EXP_C char * _EWL_CDECL	    fgets(char * _EWL_RESTRICT s, int n, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;

	/**	The fputc function writes the character specified by c (converted to an unsigned
	 *	char) to the output stream pointed to by stream, at the position indicated by the
	 *	associated file position indicator for the stream (if defined), and advances the indicator
	 *	appropriately. If the file cannot support positioning requests, or if the stream was opened
	 *	with append mode, the character is appended to the output stream.
	 *
	 *	Returns the character written. If a write error occurs, the error indicator for the stream
	 *	is set and fputc returns EOF.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fputc(int, FILE *stream) _EWL_CANT_THROW;

	/**	The fputs function writes the string pointed to by s to the stream pointed to by stream.
	 *	The terminating null character is not written.
	 *
	 *	Returns EOF if a write error occurs; otherwise it returns a nonnegative value.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fputs(const char * _EWL_RESTRICT s, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;

	/**	The gets function reads characters from the input stream pointed to by stdin, into the
	 *	array pointed to by s, until end-of-file is encountered or a new-line character is read.
	 *	Any new-line character is discarded, and a null character is written immediately after the
	 *	last character read into the array.
	 *
	 *	Returns s if successful. If end-of-file is encountered and no
	 *	characters have been read into the array, the contents of the array remain unchanged and a
	 *	null pointer is returned. If a read error occurs during the operation, the array contents are
	 *	indeterminate and a null pointer is returned.
	 */
_EWL_IMP_EXP_C char * _EWL_CDECL	    gets(char *s) _EWL_CANT_THROW;

	/**	The puts function writes the string pointed to by s to the stream pointed to by stdout,
	 *	and appends a new-line character to the output. The terminating null character is not written.
	 *	The puts function returns EOF if a write error occurs; otherwise it returns a nonnegative value.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			puts(const char *s) _EWL_CANT_THROW;

	/**	The ungetc function pushes the character specified by c (converted to an unsigned char)
	 *	back onto the input stream pointed to by stream. Pushed-back characters will be
	 *	returned by subsequent reads on that stream in the reverse order of their pushing. A
	 *	successful intervening call (with the stream pointed to by stream) to a file positioning
	 *	function (fseek, fsetpos, or rewind) discards any pushed-back characters for the
	 *	stream. The external storage corresponding to the stream is unchanged.
	 *	
	 *	One character of pushback is guaranteed. If the ungetc function is called too many
	 *	times on the same stream without an intervening read or file positioning operation on that
	 *	stream, the operation may fail.
	 *	
	 *	If the value of c equals that of the macro EOF, the operation fails and the input stream is
	 *	unchanged.
	 *	
	 *	A successful call to the ungetc function clears the end-of-file indicator for the stream.
	 *	The value of the file position indicator for the stream after reading or discarding all
	 *	pushed-back characters shall be the same as it was before the characters were pushed
	 *	back. For a text stream, the value of its file position indicator after a successful call to the
	 *	ungetc function is unspecified until all pushed-back characters are read or discarded.
	 *	For a binary stream, its file position indicator is decremented by each successful call to
	 *	the ungetc function; if its value was zero before a call, it is indeterminate after the
	 *	call.
	 *	
	 *	The ungetc function returns the character pushed back after conversion, or EOF if the
	 *	operation fails.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			ungetc(int c, FILE *stream) _EWL_CANT_THROW;

	/**	The fread function reads, into the array pointed to by ptr, up to nmemb elements
	 *	whose size is specified by size, from the stream pointed to by stream. For each
	 *	object, size calls are made to the fgetc function and the results stored, in the order
	 *	read, in an array of unsigned char exactly overlaying the object. The file position
	 *	indicator for the stream (if defined) is advanced by the number of characters successfully
	 *	read. If an error occurs, the resulting value of the file position indicator for the stream is
	 *	indeterminate. If a partial element is read, its value is indeterminate.
	 *
	 *	Returns the number of elements successfully read, which may be
	 *	less than nmemb if a read error or end-of-file is encountered. If size or nmemb is zero,
	 *	fread returns zero and the contents of the array and the state of the stream remain
	 *	unchanged.
	 */
_EWL_IMP_EXP_C size_t _EWL_CDECL	    fread(void * _EWL_RESTRICT ptr, size_t nmemb, size_t size, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;

	/**	The fwrite function writes, from the array pointed to by ptr, up to nmemb elements
	 *	whose size is specified by size, to the stream pointed to by stream. For each object,
	 *	size calls are made to the fputc function, taking the values (in order) from an array of
	 *	unsigned char exactly overlaying the object. The file position indicator for the
	 *	stream (if defined) is advanced by the number of characters successfully written. If an
	 *	error occurs, the resulting value of the file position indicator for the stream is
	 *	indeterminate.
	 *
	 *	Returns the number of elements successfully written, which will be
	 *	less than nmemb only if a write error is encountered. If size or nmemb is zero,
	 *	fwrite returns zero and the state of the stream remains unchanged.
	 */
_EWL_IMP_EXP_C size_t _EWL_CDECL	    fwrite(const void * _EWL_RESTRICT ptr, size_t nmemb, size_t size, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;

	/**	The fgetpos function stores the current values of the parse state (if any) and file
	 *	position indicator for the stream pointed to by stream in the object pointed to by pos.
	 *	The values stored contain unspecified information usable by the fsetpos function for
	 *	repositioning the stream to its position at the time of the call to the fgetpos function.
	 *
	 *	If successful, the fgetpos function returns zero; on failure, the fgetpos function
	 *	returns nonzero and stores an implementation-defined positive value in errno.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fgetpos(FILE * _EWL_RESTRICT stream, fpos_t * _EWL_RESTRICT pos) _EWL_CANT_THROW;

	/**	The ftell function obtains the current value of the file position indicator for the stream
	 *	pointed to by stream. For a binary stream, the value is the number of characters from
	 *	the beginning of the file. For a text stream, its file position indicator contains unspecified
	 *	information, usable by the fseek function for returning the file position indicator for the
	 *	stream to its position at the time of the ftell call; the difference between two such
	 *	return values is not necessarily a meaningful measure of the number of characters written
	 *	or read.
	 *
	 *	If successful, the ftell function returns the current value of the file position indicator
	 *	for the stream. On failure, the ftell function returns -1L and stores an implementation-defined
	 *	positive value in errno.
	 */
_EWL_IMP_EXP_C long _EWL_CDECL			ftell(FILE *stream) _EWL_CANT_THROW;

	/** The fsetpos function sets the mbstate_t object (if any) and file position indicator
	 *	for the stream pointed to by stream according to the value of the object pointed to by
	 *	pos, which shall be a value obtained from an earlier successful call to the fgetpos
	 *	function on a stream associated with the same file. If a read or write error occurs, the
	 *	error indicator for the stream is set and fsetpos fails.
	 *
	 *	A successful call to the fsetpos function undoes any effects of the ungetc function
	 *	on the stream, clears the end-of-file indicator for the stream, and then establishes the new
	 *	parse state and position. After a successful fsetpos call, the next operation on an
	 *	update stream may be either input or output.
	 *	
	 *	If successful, the fsetpos function returns zero; on failure, the fsetpos function
	 *	returns nonzero and stores an implementation-defined positive value in errno.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fsetpos(FILE *stream, const fpos_t *pos) _EWL_CANT_THROW;

	/**	The fseek function sets the file position indicator for the stream pointed to by stream.
	 *	If a read or write error occurs, the error indicator for the stream is set and fseek fails.
	 *
	 *	For a binary stream, the new position, measured in characters from the beginning of the
	 *	file, is obtained by adding offset to the position specified by whence. The specified
	 *	position is the beginning of the file if whence is SEEK_SET, the current value of the file
	 *	position indicator if SEEK_CUR, or end-of-file if SEEK_END. A binary stream need not
	 *	meaningfully support fseek calls with a whence value of SEEK_END.
	 *
	 *	For a text stream, either offset shall be zero, or offset shall be a value returned by
	 *	an earlier successful call to the ftell function on a stream associated with the same file
	 *	and whence shall be SEEK_SET.
	 *
	 *	After determining the new position, a successful call to the fseek function undoes any
	 *	effects of the ungetc function on the stream, clears the end-of-file indicator for the
	 *	stream, and then establishes the new position. After a successful fseek call, the next
	 *	operation on an update stream may be either input or output.
	 *
	 *	The fseek function returns nonzero only for a request that cannot be satisfied.
	 */
_EWL_IMP_EXP_C int _EWL_CDECL			fseek(FILE *stream, long offset, int whence) _EWL_CANT_THROW;

	/**	The rewind function sets the file position indicator for the stream pointed to by
	 *	stream to the beginning of the file. It is equivalent to (void)fseek(stream, 0L, SEEK_SET)
	 *	except that the error indicator for the stream is also cleared.
	 */
_EWL_IMP_EXP_C void	_EWL_CDECL			rewind(FILE *stream) _EWL_CANT_THROW;

	/**	The clearerr function clears the end-of-file and error indicators for the stream pointed
	 *	to by stream.
	 */
//_EWL_IMP_EXP_C void	_EWL_CDECL  		clearerr(FILE *stream) _EWL_CANT_THROW;

#if _EWL_OS_DISK_FILE_SUPPORT
	/** The perror function maps the error number in the integer expression errno to an
	 *	error message. It writes a sequence of characters to the standard error stream thus: first
	 *	(if s is not a null pointer and the character pointed to by s is not the null character), the
	 *	string pointed to by s followed by a colon (:) and a space; then an appropriate error
	 *	message string followed by a new-line character. The contents of the error message
	 *	strings are the same as those returned by the strerror function with argument errno.
	 */
	_EWL_IMP_EXP_C void	_EWL_CDECL  		perror(const char *s) _EWL_CANT_THROW;
#endif /* _EWL_OS_DISK_FILE_SUPPORT */

#if defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
	_EWL_IMP_EXP_C int _EWL_CDECL		fprintf_s(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		fscanf_s(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		scanf_s(const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		snprintf_s(char * _EWL_RESTRICT s, rsize_t n, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		sprintf_s(char * _EWL_RESTRICT s, rsize_t n, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		sscanf_s(const char * _EWL_RESTRICT s, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		vfprintf_s(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		vfscanf_s(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		vprintf_s(const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		vsnprintf_s(char * _EWL_RESTRICT s, rsize_t, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		vsprintf_s(char * _EWL_RESTRICT s, rsize_t, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		vsscanf_s(const char * _EWL_RESTRICT s, const char * _EWL_RESTRICT format, va_list arg) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C char * _EWL_CDECL	gets_s(char * s, rsize_t n) _EWL_CANT_THROW;
#endif /* __STDC_WANT_LIB_EXT1__ */

#if _EWL_OS_DISK_FILE_SUPPORT
	/** The remove function causes the file whose name is the string pointed to by filename
	 *	to be no longer accessible by that name. A subsequent attempt to open that file using that
	 *	name will fail, unless it is created anew. If the file is open, the behavior of the remove
	 *	function is implementation-defined.
	 *
	 *	Available only if disk IO is supported (_EWL_OS_DISK_FILE_SUPPORT is on).
	 */
	_EWL_IMP_EXP_C int _EWL_CDECL		remove(const char *s) _EWL_CANT_THROW;

	/**	The rename function causes the file whose name is the string pointed to by old to be
	 *	henceforth known by the name given by the string pointed to by new. The file named
	 *	old is no longer accessible by that name. If a file named by the string pointed to by new
	 *	exists prior to the call to the rename function, the behavior is implementation-defined.
	 *
	 *	Available only if disk IO is supported (_EWL_OS_DISK_FILE_SUPPORT is on).
	 */
	_EWL_IMP_EXP_C int _EWL_CDECL		rename(const char *oldname, const char *newname) _EWL_CANT_THROW;

	/**	The tmpnam function generates a string that is a valid file name and that is not the same
	 *	as the name of an existing file. The function is potentially capable of generating
	 *	TMP_MAX different strings, but any or all of them may already be in use by existing files
	 *	and thus not be suitable return values. The tmpnam function generates a different string
	 *	each time it is called.
	 *
	 *	Available only if disk IO is supported (_EWL_OS_DISK_FILE_SUPPORT is on).
	 */
	_EWL_IMP_EXP_C char * _EWL_CDECL	tmpnam(char *s) _EWL_CANT_THROW;

	/**	The tmpfile function creates a temporary binary file that is different from any other
	 *	existing file and that will automatically be removed when it is closed or at program
	 *	termination. If the program terminates abnormally, whether an open temporary file is
	 *	removed is implementation-defined. The file is opened for update with "wb+" mode.
	 *
	 *	Available only if disk IO is supported (_EWL_OS_DISK_FILE_SUPPORT is on).
	 */
	_EWL_IMP_EXP_C FILE * _EWL_CDECL	tmpfile(void) _EWL_CANT_THROW;

	/** The fprintf function writes output to the stream pointed to by stream, under control
	 *	of the string pointed to by format that specifies how subsequent arguments are
	 *	converted for output. If there are insufficient arguments for the format, the behavior is
	 *	undefined. If the format is exhausted while arguments remain, the excess arguments are
	 *	evaluated (as always) but are otherwise ignored. The fprintf function returns when
	 *	the end of the format string is encountered.
	 *
	 *	The format shall be a multibyte character sequence, beginning and ending in its initial
	 *	shift state. The format is composed of zero or more directives: ordinary multibyte
	 *	characters (not %), which are copied unchanged to the output stream; and conversion
	 *	specifications, each of which results in fetching zero or more subsequent arguments,
	 *	converting them, if applicable, according to the corresponding conversion specifier, and
	 *	then writing the result to the output stream.
	 *
	 *	Each conversion specification is introduced by the character %. After the %, the following
	 *	appear in sequence:
	 *
	 *	- Zero or more flags (in any order) that modify the meaning of the conversion
	 *		specification.
	 *
	 *	- An optional minimum field width. If the converted value has fewer characters than the
	 *		field width, it is padded with spaces (by default) on the left (or right, if the left
	 *		adjustment flag, described later, has been given) to the field width. The field width
	 *		takes the form of an asterisk '*' (described later) or a nonnegative decimal integer.
	 *
	 *	- An optional precision that gives the minimum number of digits to appear for the 'd', 'i',
	 *		'o', 'u', 'x', and 'X' conversions, the number of digits to appear after the decimal-point
	 *		character for 'a', 'A', 'e', 'E', 'f', and 'F' conversions, the maximum number of significant
	 *		digits for the 'g' and 'G' conversions, or the maximum number of bytes to be written for
	 *		's' conversions. The precision takes the form of a period (.) followed either by an
	 *		asterisk '*' (described later) or by an optional decimal integer; if only the period is
	 *		specified, the precision is taken as zero. If a precision appears with any other
	 *		conversion specifier, it is ignored.
	 *
	 *	- An optional length modifier that specifies the size of the argument.
	 *
	 *	- A conversion specifier character that specifies the type of conversion to be applied.
	 *	
	 *	As noted above, a field width, or precision, or both, may be indicated by an asterisk. In
	 *	this case, an int argument supplies the field width or precision. The arguments
	 *	specifying field width, or precision, or both, shall appear (in that order) before the
	 *	argument (if any) to be converted. A negative field width argument is taken as a '-' flag
	 *	followed by a positive field width. A neg ative precision argument is taken as if the
	 *	precision were omitted.
	 *	
	 *	The flag characters and their meanings are:
	 *
	 *	 '-' The result of the conversion is left-justified within the field. (It is right-justified if
	 *		this flag is not specified.)
	 *
	 *	 '+' The result of a signed conversion always begins with a plus or minus sign. (It
	 *		begins with a sign only when a negative value is converted if this flag is not
	 *		space If the first character of a signed conversion is not a sign, or if a signed conversion
	 *		results in no characters, a space is prefixed to the result. If the space and '+' flags
	 *		both appear, the space flag is ignored.
	 *	
	 *	 '#' The result is converted to an ‘‘alternative form’’. For 'o' conversion, it increases
	 *		the precision, if and only if necessary, to force the first digit of the result to be a
	 *		zero (if the value and precision are both '0', a single '0' is printed). For 'x' (or 'X')
	 *		conversion, a nonzero result has 0x (or 0X) prefixed to it. For 'a', 'A', 'e', 'E', 'f',
	 *		'F', 'g', and 'G' conversions, the result of converting a floating-point number always
	 *		contains a decimal-point character, even if no digits follow it. (Normally, a
	 *		decimal-point character appears in the result of these conversions only if a digit
	 *		follows it.) For 'g' and 'G' conversions, trailing zeros are not removed from the
	 *		result. For other conversions, it is ignored.
	 *
	 *	 '0' For 'd', 'i', 'o', 'u', 'x', 'X', 'a', 'A', 'e', 'E', 'f', 'F', 'g', and 'G' conversions,
	 *		leading zeros (following any indication of sign or base) are used to pad to the field width
	 *		rather than performing space padding, except when converting an infinity or NaN. If the
	 *		'0' and '-' flags both appear, the '0' flag is ignored. For 'd', 'i', 'o', 'u', 'x', and 'X'
	 *		conversions, if a precision is specified, the '0' flag is ignored. For other
	 *		conversions, it is ignored.
	 *
	 *	 '@' Power Architecture AltiVec extension, the next argument provides vector separator string.
	 *
	 *	The length modifiers and their meanings are:
	 *
	 *	 hh Specifies that a following 'd', 'i', 'o', 'u', 'x', or 'X' conversion specifier
	 *		applies to a signed char or unsigned char argument (the argument will have
	 *		been promoted according to the integer promotions, but its value shall be
	 *		converted to signed char or unsigned char before printing); or that
	 *		a following n conversion specifier applies to a pointer to a signed char
	 *		argument.
	 *		(Available only when _EWL_C99_PRINTF_SCANF is set)
	 *
	 *	 hv Power Architecture AltiVec extension, the argument is of type vector short.
	 *
	 *	 h Specifies that a following  'd', 'i', 'o', 'u', 'x', or 'X' conversion specifier
	 *		applies to a short int or unsigned short int argument (the argument will
	 *		have been promoted according to the integer promotions, but its value shall
	 *		be converted to short int or unsigned short int before printing);
	 *		or that a following n conversion specifier applies to a pointer to a short
	 *		int argument.
	 *
	 *	 l Specifies that a following  'd', 'i', 'o', 'u', 'x', or 'X' conversion 
	 *		specifier applies to a long int or unsigned long int argument; that a following
	 *		'n' conversion specifier applies to a pointer to a long int argument; that a
	 *		following 'c' conversion specifier applies to a wint_t argument; that a
	 *		following 's' conversion specifier applies to a pointer to a wchar_t argument;
	 *		or has no effect on a following 'a', 'A', 'e', 'E', 'f', 'F', 'g', or 'G'
	 *		conversion specifier.
	 *
	 *	 ll Specifies that a following  'd', 'i', 'o', 'u', 'x', or 'X' conversion specifier
	 *		applies to a long long int or unsigned long long int argument; or that a
	 *		following n conversion specifier applies to a pointer to a long long int
	 *		argument.
	 *		(Available only when _EWL_C99_PRINTF_SCANF or _EWL_LONGLONG_PRINTF_SCANF is set)
	 *
	 *	 j Specifies that a following  'd', 'i', 'o', 'u', 'x', or 'X' conversion specifier
	 *		applies to an intmax_t or uintmax_t argument; or that a following n conversion
	 *		specifier applies to a pointer to an intmax_t argument.
	 *		(Available only when _EWL_C99_PRINTF_SCANF is set)
	 *
	 *	 z Specifies that a following  'd', 'i', 'o', 'u', 'x', or 'X' conversion specifier
	 *		applies to a size_t or the corresponding signed integer type argument; or that a
	 *		following n conversion specifier applies to a pointer to a signed integer type
	 *		corresponding to size_t argument.
	 *		(Available only when _EWL_C99_PRINTF_SCANF is set)
	 *
	 *	 t Specifies that a following  'd', 'i', 'o', 'u', 'x', or 'X' conversion specifier
	 *		applies to a ptrdiff_t or the corresponding unsigned integer type argument; or that
	 *		a following n conversion specifier applies to a pointer to a ptrdiff_t argument.
	 *		(Available only when _EWL_C99_PRINTF_SCANF is set)
	 *
	 *	 L Specifies that a following 'a', 'A', 'e', 'E', 'f', 'F', 'g', or 'G' conversion
	 *		specifier applies to a long double argument. If a length modifier appears with any
	 *		conversion specifier other than as specified above, the behavior is undefined.
	 *		(Available only when _EWL_FLOATING_POINT_PRINTF_SCANF is set)
	 *	
	 *	The conversion specifiers and their meanings are:
	 *	
	 *	 d,i The int argument is converted to signed decimal in the style [-]dddd. The
	 *		precision specifies the minimum number of digits to appear; if the value
	 *		being converted can be represented in fewer digits, it is expanded with
	 *		leading zeros. The default precision is 1. The result of converting a zero
	 *		value with a precision of zero is no characters.
	 *		
	 *	o,u,x,X The unsigned int argument is converted to unsigned octal (o), unsigned
	 *		decimal (u), or unsigned hexadecimal notation (x or X) in the style dddd; the
	 *		letters abcdef are used for 'x' conversion and the letters ABCDEF for 'X'
	 *		conversion. The precision specifies the minimum number of digits to appear;
	 *		if the value being converted can be represented in fewer digits, it is expanded
	 *		with leading zeros. The default precision is 1. The result of converting a
	 *		zero value with a precision of zero is no characters.
	 *	
	 *	f,F A double argument representing a floating-point number is converted to
	 *		decimal notation in the style [.]ddd.ddd, where the number of digits after
	 *		the decimal-point character is equal to the precision specification. If the
	 *		precision is missing, it is taken as 6; if the precision is zero and the '#'
	 *		flag is not specified, no decimal-point character appears. If a decimal-point
	 *		character appears, at least one digit appears before it. The value is rounded to
	 *		the appropriate number of digits. A double argument representing an infinity is
	 *		converted in one of the styles [-]inf or [-]infinity which style is
	 *		implementation-defined. A double argument representing a NaN is converted in
	 *		one of the styles [-]nan or [-]nan(n-char-sequence) which style, and the meaning
	 *		of any n-char-sequence, is implementation-defined. The 'F' conversion specifier
	 *		produces INF, INFINITY, or NAN instead of inf, infinity, or nan, respectively.
	 *		(Available only when _EWL_FLOATING_POINT_PRINTF_SCANF is set)
	 *
	 *	e,E A double argument representing a floating-point number is converted in the
	 *		style [.]d.ddd e±}dd, where there is one digit (which is nonzero if the
	 *		argument is nonzero) before the decimal-point character and the number of
	 *		digits after it is equal to the precision; if the precision is missing, it is
	 *		taken as 6; if the precision is zero and the '#' flag is not specified, no
	 *		decimal-point character appears. The value is rounded to the appropriate number
	 *		of digits. The 'E' conversion specifier produces a number with E instead of e
	 *		introducing the exponent. The exponent always contains at least two digits,
	 *		and only as many more digits as necessary to represent the exponent. If the
	 *		value is zero, the exponent is zero.
	 *		A double argument representing an infinity or NaN is converted in the style
	 *		of an 'f' or 'F' conversion specifier.
	 *		(Available only when _EWL_FLOATING_POINT_PRINTF_SCANF is set)
	 *
	 *	g,G A double argument representing a floating-point number is converted in style
	 *		'f' or 'e' (or in style 'F' or 'E' in the case of a G conversion specifier),
	 *		depending on the value converted and the precision. Let P equal the
	 *		precision if nonzero, 6 if the precision is omitted, or 1 if the precision is zero.
	 *		Then, if a conversion with style E would have an exponent of X:
	 *			if P > X >= -4, the conversion is with style f (or F) and precision P - (X + 1).
	 *			otherwise, the conversion is with style e (or E) and precision P - 1.
	 *		Finally, unless the '#' flag is used, any trailing zeros are removed from the
	 *		fractional portion of the result and the decimal-point character is removed if
	 *		there is no fractional portion remaining.
	 *		A double argument representing an infinity or NaN is converted in the style
	 *		of an 'f' or 'F' conversion specifier.
	 *		(Available only when _EWL_FLOATING_POINT_PRINTF_SCANF is set)
	 *	
	 *	a,A A double argument representing a floating-point number is converted in the
	 *		style [-]0xh.hhhh p±d, where there is one hexadecimal digit (which is
	 *		nonzero if the argument is a normalized floating-point number and is
	 *		otherwise unspecified) before the decimal-point character and the number
	 *		of hexadecimal digits after it is equal to the precision; if the precision is
	 *		missing and FLT_RADIX is a power of 2, then the precision is sufficient for
	 *		an exact representation of the value; if the precision is missing and
	 *		FLT_RADIX is not a power of 2, then the precision is sufficient to
	 *		distinguish values of type double, except that trailing zeros may be
	 *		omitted; if the precision is zero and the '#' flag is not specified, no decimal
	 *		point character appears. The letters abcdef are used for a conversion and
	 *		the letters ABCDEF for 'A' conversion. The 'A' conversion specifier produces a
	 *		number with X and P instead of x and p. The exponent always contains at
	 *		least one digit, and only as many more digits as necessary to represent the
	 *		decimal exponent of 2. If the value is zero, the exponent is zero.
	 *		A double argument representing an infinity or NaN is converted in the style
	 *		of an 'f' or 'F' conversion specifier.
	 *		(Available only when _EWL_FLOATING_POINT_PRINTF_SCANF and _EWL_C99_PRINTF_SCANF are set)
	 *
	 *	c If no 'l' length modifier is present, the int argument is converted to an
	 *		unsigned char, and the resulting character is written.
	 *
	 *		If an 'l' length modifier is present, the wint_t argument is converted as if by
	 *		an ls conversion specification with no precision and an argument that points
	 *		to the initial element of a two-element array of wchar_t, the first element
	 *		containing the wint_t argument to the 'lc' conversion specification and the
	 *		second a null wide character.
	 *		(Available only when _EWL_WIDE_CHAR is set)
	 *
	 *	s If no 'l' length modifier is present, the argument shall be a pointer to the initial
	 *		element of an array of character type. Characters from the array are written
	 *		up to (but not including) the terminating null character. If the precision is 
	 *		specified, no more than that many bytes are written. If the precision is not
	 *		specified or is greater than the size of the array, the array shall contain a
	 *		null character.
	 *
	 *		If an 'l' length modifier is present, the argument shall be a pointer to the initial
	 *		element of an array of wchar_t type. Wide characters from the array are
	 *		converted to multibyte characters (each as if by a call to the wcrtomb
	 *		function, with the conversion state described by an mbstate_t object
	 *		initialized to zero before the first wide character is converted) up to and
	 *		including a terminating null wide character. The resulting multibyte
	 *		characters are written up to (but not including) the terminating null character
	 *		(byte). If no precision is specified, the array shall contain a null wide
	 *		character. If a precision is specified, no more than that many bytes are
	 *		written (including shift sequences, if any), and the array shall contain a null
	 *		wide character if, to equal the multibyte character sequence length given by
	 *		the precision, the function would need to access a wide character one past the
	 *		end of the array. In no case is a partial multibyte character written.
	 *		(Available only when _EWL_WIDE_CHAR is set)
	 *	
	 *	p The argument shall be a pointer to void. The value of the pointer is
	 *		converted to a sequence of printing characters, in an implementation-defined
	 *		manner.
	 *
	 *	n The argument shall be a pointer to signed integer into which is written the
	 *		number of characters written to the output stream so far by this call to
	 *		fprintf. No argument is converted, but one is consumed. If the conversion
	 *		specification includes any flags, a field width, or a precision, the behavior is
	 *		undefined.
	 *
	 *	R,r Power Architecture SPE extension, The argument shall be unsigned (signed) fixed
	 *		point.
	 *		(Available only when _EWL_C99_PRINTF_SCANF && _EWL_FLOATING_POINT_PRINTF_SCANF are set)
	 *
	 *	% A % character is written. No argument is converted. The complete conversion
	 *		specification shall be %%.
	 *
	 *	If a conversion specification is invalid, the behavior is undefined. If any argument is
	 *	not the correct type for the corresponding conversion specification, the behavior is
	 *	undefined.
	 *	
	 *	In no case does a nonexistent or small field width cause truncation of a field; if the result
	 *	of a conversion is wider than the field width, the field is expanded to contain the
	 *	conversion result.
	 *
	 *	For a and A conversions, if FLT_RADIX is a power of 2, the value is correctly rounded
	 *	to a hexadecimal floating number with the given precision.
	 *
	 *	Returns the number of characters transmitted, or a negative value if an output or encoding
	 *	error occurred.
	 */
	_EWL_IMP_EXP_C int _EWL_CDECL		fprintf(FILE * _EWL_RESTRICT stream, const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;

	/** The fopen function opens the file whose name is the string pointed to by filename,
	 *	and associates a stream with it.
	 *	The argument mode points to a string. If the string is one of the following, the file is
	 *	open in the indicated mode. Otherwise, NULL is returned.
	 *		r open text file for reading
	 *		w truncate to zero length or create text file for writing
	 *		a append; open or create text file for writing at end-of-file
	 *		rb open binary file for reading
	 *		wb truncate to zero length or create binary file for writing
	 *		ab append; open or create binary file for writing at end-of-file
	 *		r+ open text file for update (reading and writing)
	 *		w+ truncate to zero length or create text file for update
	 *		a+ append; open or create text file for update, writing at end-of-file
	 *		r+b or rb+ open binary file for update (reading and writing)
	 *		w+b or wb+ truncate to zero length or create binary file for update
	 *		a+b or ab+ append; open or create binary file for update, writing at end-of-file
	 *
	 *	Opening a file with read mode ('r' as the first character in the mode argument) fails if
	 *	the file does not exist or cannot be read.
	 *
	 *	Opening a file with append mode ('a' as the first character in the mode argument)
	 *	causes all subsequent writes to the file to be forced to the then current end-of-file,
	 *	regardless of intervening calls to the fseek function. In some implementations, opening
	 *	a binary file with append mode ('b' as the second or third character in the above list of
	 *	mode argument values) may initially position the file position indicator for the stream
	 *	beyond the last data written, because of null character padding.
	 *
	 *	When a file is opened with update mode ('+' as the second or third character in the
	 *	above list of mode argument values), both input and output may be performed on the
	 *	associated stream. However, output shall not be directly followed by input without an
	 *	intervening call to the fflush function or to a file positioning function (fseek,
	 *	fsetpos, or rewind), and input shall not be directly followed by output without an
	 *	intervening call to a file positioning function, unless the input operation encounters endof-
	 *	file. Opening (or creating) a text file with update mode may instead open (or create) a
	 *	binary stream in some implementations.
	 *
	 *	When opened, a stream is fully buffered if and only if it can be determined not to refer to
	 *	an interactive device. The error and end-of-file indicators for the stream are cleared.
	 *
	 *	The fopen function returns a pointer to the object controlling the stream. If the open
	 *	operation fails, fopen returns a null pointer.
	 */
	_EWL_IMP_EXP_C FILE * _EWL_CDECL	fopen(const char * _EWL_RESTRICT filename, const char * _EWL_RESTRICT mode) _EWL_CANT_THROW;

	/**	The freopen function opens the file whose name is the string pointed to by filename
	 *	and associates the stream pointed to by stream with it. The mode argument is used just
	 *	as in the fopen function.
	 *
	 *	If the filename is a null pointer, the freopen function attempts to change the mode of
	 *	the stream to that specified by mode, as if the name of the file currently associated with
	 *	the stream had been used. It is implementation-defined which changes of mode are
	 *	permitted (if any), and under what circumstances.
	 *
	 *	The freopen function first attempts to close any file that is associated with the specified
	 *	stream. Failure to close the file is ignored. The error and end-of-file indicators for the
	 *	stream are cleared.
	 *
	 *	The freopen function returns a null pointer if the open operation fails. Otherwise,
	 *	freopen returns the value of stream.
	 */
	_EWL_IMP_EXP_C FILE * _EWL_CDECL	freopen(const char * _EWL_RESTRICT filename, const char * _EWL_RESTRICT mode, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;
#if defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
	_EWL_IMP_EXP_C errno_t _EWL_CDECL	fopen_s(FILE * _EWL_RESTRICT * _EWL_RESTRICT, const char * _EWL_RESTRICT, const char * _EWL_RESTRICT) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C errno_t _EWL_CDECL	freopen_s(FILE * _EWL_RESTRICT * _EWL_RESTRICT, const char * _EWL_RESTRICT, const char * _EWL_RESTRICT, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C errno_t _EWL_CDECL	tmpfile_s(FILE **) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C errno_t _EWL_CDECL	tmpnam_s(char *, rsize_t) _EWL_CANT_THROW;
#endif /* __STDC_WANT_LIB_EXT1__ */
#if _EWL_WFILEIO_AVAILABLE
	_EWL_IMP_EXP_C int _EWL_CDECL		_wremove(const wchar_t *) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C int _EWL_CDECL		_wrename(const wchar_t *, const wchar_t *) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C wchar_t * _EWL_CDECL	_wtmpnam(wchar_t *) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C FILE * _EWL_CDECL	_wfopen(const wchar_t * _EWL_RESTRICT, const wchar_t * _EWL_RESTRICT) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C FILE * _EWL_CDECL	_wfreopen(const wchar_t * _EWL_RESTRICT, const wchar_t * _EWL_RESTRICT, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;
#if defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
	_EWL_IMP_EXP_C errno_t _EWL_CDECL	_wfopen_s(FILE * _EWL_RESTRICT * _EWL_RESTRICT, const wchar_t * _EWL_RESTRICT, const wchar_t * _EWL_RESTRICT) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C errno_t _EWL_CDECL	_wfreopen_s(FILE * _EWL_RESTRICT * _EWL_RESTRICT, const wchar_t * _EWL_RESTRICT, const wchar_t * _EWL_RESTRICT, FILE * _EWL_RESTRICT stream) _EWL_CANT_THROW;
	_EWL_IMP_EXP_C errno_t _EWL_CDECL	_wtmpnam_s(wchar_t *, rsize_t) _EWL_CANT_THROW;
#endif /* __STDC_WANT_LIB_EXT1__ */
#endif /* _EWL_WFILEIO_AVAILABLE */
#endif /* _EWL_OS_DISK_FILE_SUPPORT */


#if _EWL_CONSOLE_SUPPORT

	/** The printf function is equivalent to fprintf with the argument stdout interposed
	 *	before the arguments to printf.
	 *	Returns the number of characters transmitted, or a negative value if an output or
	 *	encoding error occurred.
	 */
	_EWL_IMP_EXP_C int _EWL_CDECL printf(const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;

	#if defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
	_EWL_IMP_EXP_C int _EWL_CDECL printf_s(const char * _EWL_RESTRICT format, ...) _EWL_CANT_THROW;
	#endif /* __STDC_WANT_LIB_EXT1__ */

#endif /* _EWL_CONSOLE_SUPPORT */

#ifdef __cplusplus

	#if _EWL_THREADSAFE || defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL == 0)

		/**	The getc function is equivalent to fgetc, except that if it is implemented as a macro, it
		 *	may evaluate stream more than once, so the argument should never be an expression
		 *	with side effects.
		 *
		 *	Returns the next character from the input stream pointed to by
		 *	stream. If the stream is at end-of-file, the end-of-file indicator for the stream is set and
		 *	getc returns EOF. If a read error occurs, the error indicator for the stream is set and
		 *	getc returns EOF.
		 */
		_EWL_IMP_EXP_C int _EWL_CDECL		getc(FILE *stream);
		
		/**	The putc function is equivalent to fputc, except that if it is implemented as a macro, it
		 *	may evaluate stream more than once, so that argument should never be an expression
		 *	with side effects.
		 *
		 *	Returns the character written. If a write error occurs, the error
		 *	indicator for the stream is set and putc returns EOF.
		 */
		_EWL_IMP_EXP_C int _EWL_CDECL		putc(int c, FILE *stream);

		_EWL_END_EXTERN_C
	#else
		_EWL_END_EXTERN_C

		/**	The getc function is equivalent to fgetc, except that if it is implemented as a macro, it
		 *	may evaluate stream more than once, so the argument should never be an expression
		 *	with side effects.
		 *
		 *	Returns the next character from the input stream pointed to by
		 *	stream. If the stream is at end-of-file, the end-of-file indicator for the stream is set and
		 *	getc returns EOF. If a read error occurs, the error indicator for the stream is set and
		 *	getc returns EOF.
		 */
		inline int _EWL_CDECL getc(FILE * stream)
			{ return __getc(stream); }

		/**	The putc function is equivalent to fputc, except that if it is implemented as a macro, it
		 *	may evaluate stream more than once, so that argument should never be an expression
		 *	with side effects.
		 *
		 *	Returns the character written. If a write error occurs, the error
		 *	indicator for the stream is set and putc returns EOF.
		 */
		inline int _EWL_CDECL putc(int c, FILE *stream)
			{ return __putc(stream); }
	#endif /* _EWL_THREADSAFE */

	/**	The getchar function is equivalent to getc with the argument stdin.
	 *
	 *	The getchar function returns the next character from the input stream pointed to by
	 *	stdin. If the stream is at end-of-file, the end-of-file indicator for the stream is set and
	 *	getchar returns EOF. If a read error occurs, the error indicator for the stream is set and
	 *	getchar returns EOF.
	 */
	inline int _EWL_CDECL getchar()
		{ return getc(stdin); }

	/**	The putchar function is equivalent to putc with the second argument stdout.
	 *	The putchar function returns the character written. If a write error occurs, the error
	 *	indicator for the stream is set and putchar returns EOF.
	 */
	inline int _EWL_CDECL putchar(int c)
		{ return putc(c, stdout); }

	#if !defined(_AEABI_PORTABILITY_LEVEL) || (_AEABI_PORTABILITY_LEVEL == 0)
		inline int _EWL_CDECL feof(FILE * stream)
			{ return stream->state.eof; }

		inline int _EWL_CDECL ferror(FILE * stream)
			{ return stream->state.error; }
	#endif /* #if !defined(_AEABI_PORTABILITY_LEVEL) || == _AEABI_PORTABILITY_LEVEL == 0 */

#else

	/**	The getc function is equivalent to fgetc, except that if it is implemented as a macro, it
	 *	may evaluate stream more than once, so the argument should never be an expression
	 *	with side effects.
	 *
	 *	Returns the next character from the input stream pointed to by
	 *	stream. If the stream is at end-of-file, the end-of-file indicator for the stream is set and
	 *	getc returns EOF. If a read error occurs, the error indicator for the stream is set and
	 *	getc returns EOF.
	 */
//	_EWL_IMP_EXP_C int _EWL_CDECL		getc(FILE *stream);
//	_EWL_IMP_EXP_C int _EWL_CDECL		putc(int c, FILE *stream);

	#if !(_EWL_THREADSAFE || defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL == 0))

		/**	The getc function is equivalent to fgetc, except that if it is implemented as a macro, it
		 *	may evaluate stream more than once, so the argument should never be an expression
		 *	with side effects.
		 *
		 *	Returns the next character from the input stream pointed to by
		 *	stream. If the stream is at end-of-file, the end-of-file indicator for the stream is set and
		 *	getc returns EOF. If a read error occurs, the error indicator for the stream is set and
		 *	getc returns EOF.
		 */
		#define getc(file) __getc(file)

		/**	The putc function is equivalent to fputc, except that if it is implemented as a macro, it
		 *	may evaluate stream more than once, so that argument should never be an expression
		 *	with side effects.
		 *
		 *	Returns the character written. If a write error occurs, the error
		 *	indicator for the stream is set and putc returns EOF.
		 */
		#define putc(c, file) __putc(c, file)

	#endif /* !(_EWL_THREADSAFE || defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL == 0)) */

	/**	The getchar function is equivalent to getc with the argument stdin.
	 *
	 *	The getchar function returns the next character from the input stream pointed to by
	 *	stdin. If the stream is at end-of-file, the end-of-file indicator for the stream is set and
	 *	getchar returns EOF. If a read error occurs, the error indicator for the stream is set and
	 *	getchar returns EOF.
	 */
//	_EWL_IMP_EXP_C int _EWL_CDECL		getchar(void);

	/**	The putchar function is equivalent to putc with the second argument stdout.
	 *	The putchar function returns the character written. If a write error occurs, the error
	 *	indicator for the stream is set and putchar returns EOF.
	 */
//	_EWL_IMP_EXP_C int _EWL_CDECL		putchar(int c);

//    #define getchar()					__std(getc)(stdin)
//    #define putchar(c)					__std(putc)(c, stdout)

	/**	The feof function tests the end-of-file indicator for the stream pointed to by stream.
	 *	The feof function returns nonzero if and only if the end-of-file indicator is set for
	 *	stream.
	 */
//	_EWL_IMP_EXP_C int _EWL_CDECL		feof  (FILE *stream);

	/** The ferror function tests the error indicator for the stream pointed to by stream.
	 *	The ferror function returns nonzero if and only if the error indicator is set for
	 *	stream.
	 */
//	_EWL_IMP_EXP_C int _EWL_CDECL		ferror(FILE *stream);

	#if !defined(_AEABI_PORTABILITY_LEVEL) || (_AEABI_PORTABILITY_LEVEL == 0)
		#define feof(file)					((file)->state.eof)
		#define ferror(file)				((file)->state.error)
	#endif /* #if !defined(_AEABI_PORTABILITY_LEVEL) || == _AEABI_PORTABILITY_LEVEL == 0 */

#endif /* __cplusplus */

_EWL_END_NAMESPACE_STD

#pragma pack(pop)

#if defined(_EWL_EXPORT_CNAMES) && defined(__cplusplus) && defined(_EWL_USING_NAMESPACE)
#include <stdio.h>
#endif /*  defined(_EWL_EXPORT_CNAMES) && defined(__cplusplus) && defined(_EWL_USING_NAMESPACE) */

#endif /* _EWL_USING_CW_C_HEADERS_ */

#endif /* _EWL_CSTDIO */
