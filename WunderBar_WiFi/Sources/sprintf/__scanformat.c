/*
 */

/*	Routines
 *	--------
 *		fscanf
 *		fscanf_s
 *		scanf
 *		scanf_s
 *		sscanf
 *		sscanf_s
 *		vfscanf
 *		vfscanf_s
 *		vsscanf
 *		vsscanf_s
 */


#include "ansi_parms.h"
#include "ewl_rsize_t.h"
#include "stdio_api.h"
#include "strtoul_api.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if _EWL_WIDE_CHAR
#include <wchar.h>
#endif

#if _EWL_FLOATING_POINT_PRINTF_SCANF
#include <strtold_api.h>
#endif

#if !_EWL_LONGLONG
#if (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
#warning "Impossible to perform long long IO when long long support is not available!"
#endif
#undef _EWL_LONGLONG_PRINTF_SCANF
#define _EWL_LONGLONG_PRINTF_SCANF	0
#endif


#include "ewl_misra_types.h"
MISRA_ALLOW_POINTER_CASTS()

#if _EWL_FLOATING_POINT_PRINTF_SCANF
	#include <ansi_fp.h>
	#include <float.h>
#endif /* _EWL_FLOATING_POINT_PRINTF_SCANF */

#define bad_conversion			(char_t)0xFF

enum argument_options
{
	normal_argument,
	char_argument,
	short_argument,
	long_argument

#if _EWL_C99_PRINTF_SCANF
	,
	intmax_argument,
	size_t_argument,
	ptrdiff_argument
#endif /* _EWL_C99_PRINTF_SCANF */
#if _EWL_LONGLONG_PRINTF_SCANF || _EWL_C99_PRINTF_SCANF
	,
	long_long_argument
#endif /* _EWL_LONGLONG_PRINTF_SCANF */

#if _EWL_FLOATING_POINT_PRINTF_SCANF
	,
	double_argument,
	long_double_argument
#endif /* _EWL_FLOATING_POINT_PRINTF_SCANF */
#if _EWL_WIDE_CHAR
	,
	wchar_argument
#endif /* _EWL_WIDE_CHAR */
};

typedef uchar_t char_map[32];

typedef struct
{
	uchar_t					suppress_assignment;
	uchar_t 				field_width_specified;
	enum argument_options 	argument_options;
	char_t					conversion_char;
	int_t					field_width;
#ifdef __ALTIVEC__
	int_t					vec;							/* doing vector conversion						*/
	int_t  					vecCount;						/* number of vec fields to convert (4, 8, 16) 	*/
	char_t 					*vecSeperator;					/* separator string to insert between vec fields*/
	char_t 					*c_vecSeperator;				/* separator string to use for %c vec conversion*/
	int_t	 				vecSepLen;						/* length of *vecSeperator or *c_vecSeperator 	*/
#endif /* __ALTIVEC__ */
	char_map				char_set;
} scan_format;

#ifdef __ALTIVEC__
#ifndef __VEC__
	#define vec_va_arg(ap)  (*(((MWVector128 *) (ap = (char_t *) ((((uint32_t) ap + 15) & ~(15)) + 16))) -1))
	#define va_arg_replacement2(ap, vec_val) (*va_arg(ap, MWVector128 *) = vec_val)
#endif /* __VEC__ */

union MWVector128 {						/* used to get at the vec field values 				*/
	#ifdef __VEC__
	__vector unsigned char		vuc;
	__vector signed char		vsc;
	__vector bool char			vbc;
	__vector unsigned short		vus;
	__vector signed short		vss;
	__vector unsigned long		vul;
	__vector signed long		vsl;
	__vector float				vf;
	__vector pixel				vp;
	#endif /* __VEC__ */
	uchar_t 		 			u8[16];
	int8_t 		 				s8[16];
	uint16_t		 			u16[8];
	int16_t		 				s16[8];
	uint32_t 		 			u32[4];
	int32_t 		 			s32[4];
	f32_t		   		 		f32[4];
};
typedef union MWVector128 MWVector128;
#endif /* __ALTIVEC__ */

MISRA_EXCEPTION_RULE_19_7()
#define set_char_map(map, ch) (map[(uchar_t)ch>>3] |= (uchar_t)(1u << ((uchar_t)ch&7u)))
#define tst_char_map(map, ch) (map[(uchar_t)ch>>3] &  (uchar_t)(1u << ((uchar_t)ch&7u)))
_MISRA_RESTORE()

#ifdef __ALTIVEC__
static const char_t * parse_format(const char_t * format_string, va_list * arg, scan_format * format)
#else
static const char_t * parse_format(const char_t * format_string, scan_format * format)
#endif /* __ALTIVEC__ */
{
	const char_t *	s = format_string;
	int_t				c;
	int_t				flag_found, invert;
#ifdef __ALTIVEC__
	int_t				vec_sep_ok = 1;
	scan_format			f = {0u, 0u, normal_argument, 0, INT_MAX, 0, 0, "", "", 0, {0u}};
#else
	scan_format			f = {0u, 0u, normal_argument, 0, INT_MAX, {0u}};
#endif /* __ALTIVEC__ */

	if ((c = *++s) == '%') {
		f.conversion_char = (char_t)c;
		*format = f;
		MISRA_EXCEPTION_RULE_14_7()
		return(s + 1);
	}

	if (c == '*') {
		f.suppress_assignment = 1u;
		c = *++s;
	}

#ifdef __ALTIVEC__
	GetWidth:			 			/* goto here when we see we had a vec sep char_t flag */
#endif /* __ALTIVEC__ */

	if (isdigit(c)) {
		f.field_width = 0;

		do {
			f.field_width = (f.field_width * 10) + (c - '0');
			c = *++s;
		} while (isdigit(c));

		if (f.field_width == 0) {
			f.conversion_char = bad_conversion;
			*format = f;
			MISRA_EXCEPTION_RULE_14_7()
			return(s + 1);
		}

		f.field_width_specified = 1u;
	}

#ifdef __ALTIVEC__
	if (c == 'v')
	{
		f.vec = 1;
		c = *++s;
	}
#endif /* __ALTIVEC__ */

	flag_found = 1;

	switch (c)
	{
		case 'h':
			f.argument_options = short_argument;

			if (s [1] == 'h')
			{
				f.argument_options = char_argument;
				c = *++s; /* move past second h */
			}
#ifdef __ALTIVEC__
			else
				f.vecCount = 8;
#endif /* __ALTIVEC__ */

			break;

		case 'l':

			f.argument_options = long_argument;

#if _EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF
			if (s [1] == 'l')
			{
				f.argument_options = long_long_argument;
				c = *++s; /* move past second l */
			}
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */

#ifdef __ALTIVEC__
	#if _EWL_LONGLONG_PRINTF_SCANF
			else
	#endif /* _EWL_LONGLONG_PRINTF_SCANF */
				f.vecCount = 4;
#endif /* __ALTIVEC__ */

			break;

#if _EWL_FLOATING_POINT_PRINTF_SCANF

		case 'L':
			f.argument_options = long_double_argument;
			break;

#endif /* _EWL_FLOATING_POINT_PRINTF_SCANF */

#if _EWL_C99_PRINTF_SCANF

		case 'j':

			f.argument_options = intmax_argument;
			break;

		case 't':

			f.argument_options = ptrdiff_argument;
			break;

		case 'z':

			f.argument_options = size_t_argument;
			break;

#endif /* _EWL_C99_PRINTF_SCANF */

		default:
			flag_found = 0;
#ifdef __ALTIVEC__
			f.vecCount = 16;
#endif /* __ALTIVEC__ */
			break;
	}

	if (flag_found) {
		c = *++s;
	}

#ifdef __ALTIVEC__
	if (!f.vec && c == 'v' && f.vecCount != 0) {
		f.vec = 1;
		c = *++s;
	}
#endif /* __ALTIVEC__ */

	f.conversion_char = (char_t)c;

	switch (c)
	{
		case 'd':
		case 'i':
		case 'u':
		case 'o':
		case 'x':
		case 'X':

#if _EWL_FLOATING_POINT_PRINTF_SCANF
			if (f.argument_options == long_double_argument)
			{
				f.conversion_char = bad_conversion;
				break;
			}
#endif /* _EWL_FLOATING_POINT_PRINTF_SCANF */

			break;

#if _EWL_FLOATING_POINT_PRINTF_SCANF
	#if _EWL_C99_PRINTF_SCANF
        case 'a':
        case 'A':
	#endif /* _EWL_C99_PRINTF_SCANF */

 		case 'F':
		case 'f':
		case 'e':
		case 'E':
		case 'g':
		case 'G':

			if ((f.argument_options == char_argument)
				|| (f.argument_options == short_argument)
	#if _EWL_C99_PRINTF_SCANF
				|| (f.argument_options == intmax_argument)
				|| (f.argument_options == size_t_argument)
				|| (f.argument_options == ptrdiff_argument)
	#endif /* _EWL_C99_PRINTF_SCANF */
	#if _EWL_LONGLONG_PRINTF_SCANF || _EWL_C99_PRINTF_SCANF
				|| (f.argument_options == long_long_argument)
	#endif /* _EWL_LONGLONG_PRINTF_SCANF */
				) {
				f.conversion_char = bad_conversion;
				break;
			}

			/* 'l' before float identifier means double */

			if (f.argument_options == long_argument) {
				f.argument_options = double_argument;
			}
			break;
#endif /* _EWL_FLOATING_POINT_PRINTF_SCANF */

		case 'p':

#ifdef __ALTIVEC__
			if (!f.vec)
#endif /* __ALTIVEC__ */

/* dsp56800 uses 16-bit pointers*/
#if (__dest_os == __m56800_os) || (__dest_os == __m56800E_os)

	#ifdef __m56800E_lmm__
    		f.argument_options = long_argument;
	#else
			f.argument_options = short_argument;
	#endif

#else
    		f.argument_options = long_argument;
#endif

    		f.conversion_char  = 'x';
    		break;

		case 'c':
#ifdef __ALTIVEC__
			if (!f.vec) {
#endif /* __ALTIVEC__ */
#if _EWL_WIDE_CHAR
			if (f.argument_options == long_argument) {
				f.argument_options = wchar_argument;
			} else
#endif /* _EWL_WIDE_CHAR */
			{
				if (f.argument_options != normal_argument) {
					f.conversion_char = bad_conversion;
				}
			}
#ifdef __ALTIVEC__
			}
#endif /* __ALTIVEC__ */

			break;

		case 's':

#if _EWL_WIDE_CHAR
			if (f.argument_options == long_argument) {
				f.argument_options = wchar_argument;
			} else
#endif /* _EWL_WIDE_CHAR */
			{
				if (f.argument_options != normal_argument) {
					f.conversion_char = bad_conversion;
				}
			}

			{
				uint_t		i;
				uchar_t *	p = f.char_set;

				for (i = sizeof(f.char_set); i; --i) {
					*p++ = 0xFFu;
				}

				f.char_set[1] = 0xC1u;	/* sets bits for HT, LF, VT, FF, and CR chars to zero */
				f.char_set[4] = 0xFEu;	/* set bit for Sp char_t to zero */
			}

			break;

		case 'n':

			break;

		case '[':
#if _EWL_WIDE_CHAR
			if (f.argument_options == long_argument) {
				f.argument_options = wchar_argument;
			} else
#endif /* _EWL_WIDE_CHAR */
			{
				if (f.argument_options != normal_argument) {
					f.conversion_char = bad_conversion;
				}
			}

			c = *++s;

			invert = 0;

			if (c == '^') {
				invert = 1;
				c = *++s;
			}

			if (c == ']') {
				set_char_map(f.char_set, ']');
				c = *++s;
			}

			while (c && (c != ']')) {
				int_t d;
				set_char_map(f.char_set, c);

				MISRA_EXCEPTION_RULE_12_4()
				if ((*(s+1) == '-') && ((d = *(s+2)) != 0) && (d != ']')) {
					while (++c <= d) {
						set_char_map(f.char_set, c);
					}
					c = *(s += 3);
				} else {
					c = *++s;
				}
			}

			if (!c) {
				f.conversion_char = bad_conversion;
				break;
			}

			if (invert) {
				uint_t		i;
				uchar_t *	p = f.char_set;

				for (i = sizeof(f.char_set); i; --i) {
					*p = (uchar_t)~*p;
					++p;
				}
			}

			break;

		default:
#ifdef __ALTIVEC__
			if (!f.vec && vec_sep_ok) {
				vec_sep_ok = 0;
				if (c == '@') {
					f.vecSeperator = va_arg(*arg, char_t *);
					f.vecSepLen    = strlen(f.vecSeperator);
					while (f.vecSepLen > 0 && *f.vecSeperator == ' ') {
						++f.vecSeperator;
						--f.vecSepLen;
					}
					while (f.vecSepLen > 0 && f.vecSeperator[f.vecSepLen-1] == ' ') {
						--f.vecSepLen;
					}
				} else {
					f.vecSeperator = (char_t *)s;
					f.vecSepLen    = 1;
				}
				f.c_vecSeperator = f.vecSeperator;
				c = *++s;
				if (!f.suppress_assignment && c == '*') {
					f.suppress_assignment = 1;
					c = *++s;
				}
				goto GetWidth;
			}
#endif /* __ALTIVEC__ */

			f.conversion_char = bad_conversion;
			break;
	}

	*format = f;

#if 0 /* EWL debugging code */
#if _EWL_OS_DISK_FILE_SUPPORT
	fprintf(stderr, "scan_format:\n"
					"  suppress_assignment   = %d\n"
					"  field_width_specified = %d\n"
					"  argument_options      = %d\n"
					"  conversion_char       = %c\n"
					"  field_width           = %ld\n"
					"  vec                   = %ld\n"
					"  vecCount              = %ld\n"
					"  vecSeperator          = %s\n"
					"  vecSepLen             = %ld\n",
					f.suppress_assignment,
					f.field_width_specified,
					f.argument_options,
					f.conversion_char,
					f.field_width,
					f.vec,
					f.vecCount,
					f.vecSeperator == 0 ? "<null>" : f.vecSeperator,
					f.vecSepLen);
#endif /* _EWL_OS_DISK_FILE_SUPPORT */
#endif /* 0 */

	return(s + 1);
}

MISRA_EXCEPTION_RULE_16_2()
int_t __sformatter(int_t (_EWL_CDECL *ReadProc)(void *a, int_t b, int_t c),
				void * ReadProcArg,
				const char_t * format_str,
				va_list arg,
				int_t is_secure)
{
	int_t			num_chars, chars_read, items_assigned, conversions;
	int_t			base, negative, overflow;
	int_t 			rval = 0;
	const char_t *	format_ptr;
	char_t			format_char;
	int_t           c;
	scan_format		format;
	int32_t			long_num = 0;
	uint32_t		u_long_num = 0UL;
	char_t			needs_conversions = 0;
#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
	int64_t  		long_long_num = 0LL;
	uint64_t 	 	u_long_long_num = 0ULL;
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
#if _EWL_FLOATING_POINT_PRINTF_SCANF
	ldbl_t			long_double_num;
#endif /* _EWL_FLOATING_POINT_PRINTF_SCANF */

	char_t *		arg_ptr;
	int_t			elem_valid = 0;
	size_t			elem_maxsize = 0u;
	int_t 			match_failure = 0;
	int_t 			terminate  = 0;

#ifdef __ALTIVEC__
	int_t vecIndex;						/* counts vector elements */
	MWVector128 vec_val;

	#define GOTO_GET_REMAINING_VEC_FIELDS(lbl) 										\
		if (!match_failure)	{														\
			if ((c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar)) != EOF && ++vecIndex < format.vecCount) {		\
				int_t sepSeen = (format.vecSepLen == 0);						\
				if (!sepSeen) {													\
					int_t i = 0;												\
					while (isspace(c)) {										\
						c = (*ReadProc)(ReadProcArg, 0, 1);						\
						++chars_read;											\
					}															\
					while (c != EOF) {											\
						sepSeen = (c == format.vecSeperator[i]);				\
						if (!sepSeen) {											\
							break;												\
						}														\
						c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar);		\
						++chars_read;											\
						if (++i >= format.vecSepLen) {							\
							break;												\
						}														\
					}															\
				}																\
				if (sepSeen && c != EOF) {										\
					while (isspace(c)) {										\
						c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar);		\
						++chars_read;											\
					}															\
					(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);		\
					goto lbl;													\
				}																\
			} else {															\
				(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);			\
			}																	\
		}
#endif /* __ALTIVEC__ */

	format_ptr     = format_str;
	chars_read     = 0;
	items_assigned = 0;
	conversions    = 0;

	MISRA_EXCEPTION_RULE_10_1()
	MISRA_EXCEPTION_RULE_12_4()
	while (!terminate && ((format_char = *format_ptr) != 0)) {
		if (isspace((int_t)format_char)) {
			do {
				format_char = *++format_ptr;
			} while (isspace((int_t)format_char));

			if (!match_failure) {
				while (isspace((c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar)))) {
					++chars_read;
				}
				if (c == EOF) {
					if (!is_secure) {
					    MISRA_EXCEPTION_RULE_14_4()
						goto exit;
					} else {
						match_failure = 1;
					}
				}
				(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
			}
			MISRA_EXCEPTION_RULE_14_5()
			continue;
		}

		if ((format_char != '%') && (!match_failure)) {
			if ((c = (char_t)(*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar)) != format_char) {
				(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
				if (!is_secure) {
					MISRA_EXCEPTION_RULE_14_4()
					goto exit;
				} else {
					match_failure = 1;
					++format_ptr;
					MISRA_EXCEPTION_RULE_14_5()
					continue;
				}
			}

			++chars_read;
			++format_ptr;
			MISRA_EXCEPTION_RULE_14_5()
			continue;
		}
#ifndef __ALTIVEC__
		format_ptr = parse_format(format_ptr, &format);
#else
		format_ptr = parse_format(format_ptr, (va_list *)&arg, &format);

		if (format.vec) {
			vecIndex 	 	= 0;
			vec_val.u32[0]	= 0;
			vec_val.u32[1]	= 0;
			vec_val.u32[2]	= 0;
			vec_val.u32[3]	= 0;

			/* Leading input blanks skipped only in vector cases */
			if (!match_failure) {
				while (isspace(c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar))) {
					++chars_read;
				}
				(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
			}
		} else
#endif /* __ALTIVEC__ */

#if !defined(__m56800E__)

		if (!format.suppress_assignment && (format.conversion_char != '%')) {
			arg_ptr = va_arg(arg, char_t *);
		} else
#endif /* __m56800E__ */
        {
			arg_ptr = 0;
		}

		MISRA_EXCEPTION_RULE_12_4()
		if ((format.conversion_char != 'n') && (!match_failure) && (*ReadProc)(ReadProcArg, 0, (int_t)__TestForError)) {
			if (!is_secure) {
				MISRA_EXCEPTION_RULE_14_4()
				goto exit;
			} else {
				match_failure = 1;
			}
		}
		switch (format.conversion_char)
		{
			case 'd':

				base = 10;
				MISRA_EXCEPTION_RULE_14_4()
				goto signed_int;

			case 'i':

				base = 0;

			signed_int:

				needs_conversions = 1;
				
				if (match_failure) {
					long_num = 0;

#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
					long_long_num = 0;
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */

				} else {

#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
	                if ((format.argument_options == long_long_argument)
	#if _EWL_C99_PRINTF_SCANF
	                	||	(format.argument_options == intmax_argument)
	#endif /* _EWL_C99_PRINTF_SCANF */
						)
					{
	                   u_long_long_num =
	                      __strtoull(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
	                } else
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
					{
					u_long_num
						= __strtoul(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
					}

					if (!num_chars) {
						if (!is_secure) {
							MISRA_EXCEPTION_RULE_14_4()
							goto exit;
						} else {
							match_failure = 1;
							long_num = 0;
#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
							long_long_num = 0;
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
							MISRA_EXCEPTION_RULE_14_4()
							goto signed_int_assign;
						}
					}

					chars_read += num_chars;
#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
	                if ((format.argument_options == long_long_argument)
	#if _EWL_C99_PRINTF_SCANF
	                	||	(format.argument_options == intmax_argument)
	#endif /* _EWL_C99_PRINTF_SCANF */
						) {
						if (negative && ((uint64_t)u_long_long_num != 0x8000000000000000ULL)) {
							long_long_num = -(int64_t)u_long_long_num;
						} else {
							long_long_num = (int64_t)u_long_long_num;
						}
	                } else
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
					{
						if (negative && ((uint32_t)u_long_num != 0x80000000UL)) {
							long_num = -(int32_t)u_long_num;
						} else {
							long_num = (int32_t)u_long_num;
						}
					}
				}

			signed_int_assign:
#ifdef __ALTIVEC__
				if (format.vec) {
					if (!format.suppress_assignment) {
						switch (format.argument_options) {
							case normal_argument: vec_val.s8 [vecIndex] = long_num; break;
							case short_argument:  vec_val.s16[vecIndex] = long_num; break;
							case long_argument:   vec_val.s32[vecIndex] = long_num; break;
						}
						GOTO_GET_REMAINING_VEC_FIELDS(signed_int);

						#ifdef __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif /* __VEC__ */
						if (!match_failure) {
							++items_assigned;
						}
					} else {
						GOTO_GET_REMAINING_VEC_FIELDS(signed_int);
					}
				}
				else
#endif /* __ALTIVEC__ */

#if defined(__m56800E__)
			{
				int_t * arg_iptr = 0;
				if (format.argument_options == char_argument) {
					arg_ptr = va_arg(arg, char_t *);
				} else {
					arg_iptr = va_arg(arg, int_t *);
				}

				switch (format.argument_options) {
					case normal_argument:
						if (arg_iptr) 	{(*((int_t *) arg_iptr)) = long_num;}
						break;
					case short_argument:
						if (arg_iptr) 	{(*((int16_t *) arg_iptr)) = long_num;}
						break;
					case long_argument:
						if (arg_iptr) 	{(*((int32_t *) arg_iptr)) = long_num;}
						break;
					case char_argument:
						if (arg_ptr) 	{(*((int8_t *) arg_ptr)) = long_num;}
						break;
	#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
					case long_long_argument:
						if (arg_iptr) 	{(*((int64_t *) arg_iptr)) = long_long_num;}
						break;
	#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
				}
				if (!match_failure) {
					++items_assigned;
				}
			}
#else /* !__m56800E__ */

				if (arg_ptr) {
					MISRA_QUIET_ENUM_SWITCH()
					MISRA_QUIET_SUSPICIOUS_PTR_CAST()
					switch (format.argument_options) {
						case normal_argument:  * (int_t *)      arg_ptr = long_num; break;
						case char_argument:    * (int8_t *)     arg_ptr = (int8_t)long_num; break;
						case short_argument:   * (int16_t *)    arg_ptr = (int16_t)long_num; break;
						case long_argument:    * (int32_t *)    arg_ptr = long_num; break;
	#if _EWL_C99_PRINTF_SCANF
		#if _EWL_LONGLONG_PRINTF_SCANF
						case intmax_argument:  * (intmax_t *)   arg_ptr = long_long_num; break;
		#else
						case intmax_argument:  * (intmax_t *)   arg_ptr = long_num; break;
		#endif /* _EWL_LONGLONG_PRINTF_SCANF */
						case size_t_argument:  * (size_t *)     arg_ptr = (uint_t)long_num; break;
						case ptrdiff_argument: * (ptrdiff_t *)  arg_ptr = long_num; break;
	#endif /* _EWL_C99_PRINTF_SCANF */
	#if _EWL_LONGLONG_PRINTF_SCANF
                        case long_long_argument: * (int64_t *)  arg_ptr = long_long_num; break;
	#endif /* _EWL_LONGLONG_PRINTF_SCANF */
						default: break;
					}
					if (!match_failure) {
						++items_assigned;
					}
				}
#endif /* __m56800E__ */
				++conversions;

				break;

			case 'o':
				base = 8;
				MISRA_EXCEPTION_RULE_14_4()
				goto unsigned_int;

			case 'u':
				base = 10;
				MISRA_EXCEPTION_RULE_14_4()
				goto unsigned_int;

			case 'x':
			case 'X':
				base = 16;

			unsigned_int:

				needs_conversions = 1;
				
				if (match_failure) {
					u_long_num = 0u;
#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
					u_long_long_num = 0uLL;
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
				}
				else
				{
#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
	                if ((format.argument_options == long_long_argument)
	#if _EWL_C99_PRINTF_SCANF
	                	||	(format.argument_options == intmax_argument)
	#endif /* _EWL_C99_PRINTF_SCANF */
						) {
	                   u_long_long_num =
	                      __strtoull(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
	                } else
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
					{
					u_long_num
						= __strtoul(base, format.field_width, ReadProc, ReadProcArg, &num_chars, &negative, &overflow);
					}

					if (!num_chars) {
						if (!is_secure) {
							MISRA_EXCEPTION_RULE_14_4()
							goto exit;
						} else {
							match_failure = 1;
							u_long_num = 0uL;
#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
							u_long_long_num = 0uLL;
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
							MISRA_EXCEPTION_RULE_14_4()
							goto unsigned_int_assign;
						}
					}

					chars_read += num_chars;

					if (negative) {
#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
	                if (format.argument_options == long_long_argument) {
	                	MISRA_EXCEPTION_RULE_10_3()
						if ((uint64_t)u_long_long_num != 0x8000000000000000ULL) {
							u_long_long_num = (uint64_t)-(int64_t)u_long_long_num;
						} 
					} else
#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */
					{
	                	MISRA_EXCEPTION_RULE_10_3()
						if ((uint32_t)u_long_num != 0x80000000UL) {
							u_long_num = (uint32_t)-(int32_t)u_long_num;
						}
					}
					}
				}

			unsigned_int_assign:
#ifdef __ALTIVEC__
				if (format.vec) {
					if (!format.suppress_assignment) {
						switch (format.argument_options) {
							case normal_argument: vec_val.u8 [vecIndex] = u_long_num; break;
							case short_argument:  vec_val.u16[vecIndex] = u_long_num; break;
							case long_argument:   vec_val.u32[vecIndex] = u_long_num; break;
						}
						GOTO_GET_REMAINING_VEC_FIELDS(unsigned_int);

						#ifdef __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif /* __VEC__ */
						if (!match_failure) {
							++items_assigned;
						}
					} else {
						GOTO_GET_REMAINING_VEC_FIELDS(unsigned_int);
					}
				}
				else
#endif /* __ALTIVEC__ */
#if defined(__m56800E__)
			{
				int_t * arg_iptr = 0;
				if (format.argument_options == char_argument) {
					arg_ptr = va_arg(arg,char_t *);
				} else {
					arg_iptr = va_arg(arg,int_t *);
				}

				switch (format.argument_options) {
					case normal_argument:
						if (arg_iptr) 	(*((uint_t *) arg_iptr)) = u_long_num;
						break;
					case short_argument:
						if (arg_iptr) 	(*((uint16_t *) arg_iptr)) = u_long_num;
						break;
					case long_argument:
						if (arg_iptr) 	(*((uint32_t *) arg_iptr)) = u_long_num;
						break;
					case char_argument:
						if (arg_ptr) 	(*((uchar_t *) arg_ptr)) = u_long_num;
						break;
	#if _EWL_C99_PRINTF_SCANF && _EWL_LONGLONG_PRINTF_SCANF
					case long_long_argument:
						if (arg_iptr) 	(*((uint64_t *) arg_iptr)) = u_long_long_num;
						break;
	#endif /* _EWL_C99_PRINTF_SCANF && _EWL_LONGLONG_PRINTF_SCANF */
				}
				if (!match_failure) {
					++items_assigned;
				}
			}
#else /* !__m56800E__ */
				if (arg_ptr) {
					MISRA_QUIET_ENUM_SWITCH()
					MISRA_QUIET_SUSPICIOUS_PTR_CAST()
					switch (format.argument_options) {
						case normal_argument:  * (uint_t *)   arg_ptr = u_long_num; break;
						case char_argument:    * (uchar_t *)  arg_ptr = (uchar_t) u_long_num; break;
						case short_argument:   * (uint16_t *) arg_ptr = (uint16_t)u_long_num; break;
						case long_argument:    * (uint32_t *) arg_ptr = (uint32_t) u_long_num; break;

	#if _EWL_C99_PRINTF_SCANF
		#if _EWL_LONGLONG_PRINTF_SCANF
						case intmax_argument:  * (intmax_t *) arg_ptr = (int64_t)u_long_long_num; break;
		#else
						case intmax_argument:  * (intmax_t *) arg_ptr = u_long_num; break;
		#endif /* _EWL_LONGLONG_PRINTF_SCANF */
						case size_t_argument:  * (size_t *)   arg_ptr = u_long_num; break;
						case ptrdiff_argument: * (ptrdiff_t *) arg_ptr = (int_t)u_long_num; break;
	#endif /* _EWL_C99_PRINTF_SCANF */
	#if _EWL_LONGLONG_PRINTF_SCANF
                        case long_long_argument: * (uint64_t *) arg_ptr = u_long_long_num; break;
	#endif /* _EWL_LONGLONG_PRINTF_SCANF */
					}

					if (!match_failure) {
						++items_assigned;
					}
				}
#endif /* __m56800E__ */

				++conversions;

				break;

#if _EWL_FLOATING_POINT_PRINTF_SCANF
	#if _EWL_C99_PRINTF_SCANF
			case 'a':
			case 'A':
	#endif /* _EWL_C99_PRINTF_SCANF */
			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
	#ifdef __ALTIVEC__
			flt:
	#endif
				needs_conversions = 1;
				
				if (match_failure) {
					long_double_num = NAN;
				} else {
					long_double_num = __strtold(format.field_width, ReadProc, ReadProcArg, &num_chars, &overflow);
					if (!num_chars) {
						if (!is_secure) {
							MISRA_EXCEPTION_RULE_14_4()
							goto exit;
						} else {
							match_failure = 1;
							long_double_num = NAN;
							MISRA_EXCEPTION_RULE_14_4()
							goto assign_float;
						}
					}
					chars_read += num_chars;
				}

			assign_float:
	#ifdef __ALTIVEC__
				if (format.vec) {
					if (!format.suppress_assignment) {
						vec_val.f32[vecIndex] = long_double_num;

						if (format.argument_options == normal_argument) {
							format.vecCount = 4;
						}
						GOTO_GET_REMAINING_VEC_FIELDS(flt);

						#ifdef __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif /* __VEC__ */
						if (!match_failure) {
							++items_assigned;
						}
					}
					else {
						GOTO_GET_REMAINING_VEC_FIELDS(flt);
					}
				} else
	#endif /* __ALTIVEC__ */

	#if defined(__m56800E__)
			{
				float * arg_fptr;
	#define	arg_ptr	arg_fptr
				arg_fptr = va_arg(arg, float *);
	#endif /* __m56800E__ */
				if (arg_ptr) {
					MISRA_QUIET_ENUM_SWITCH()
					MISRA_QUIET_SUSPICIOUS_PTR_CAST()
					switch (format.argument_options) {
						case normal_argument:      * (f32_t *)  arg_ptr = (f32_t)long_double_num; break;
						case double_argument:      * (f64_t *)  arg_ptr = long_double_num; break;
						case long_double_argument: * (ldbl_t *) arg_ptr = long_double_num; break;
					}

					if (!match_failure) {
						++items_assigned;
					}
				}
	#if defined(__m56800E__)
	#undef	arg_ptr
			}
	#endif /* __m56800E__ */
				++conversions;
				break;

#endif /*_EWL_FLOATING_POINT_PRINTF_SCANF */
			case 'c':
				if (!format.field_width_specified) {
					format.field_width = 1;
				}
#ifdef __ALTIVEC__
				if (format.vec) {
					if (!format.suppress_assignment) {
						char_t *ptr;
						int_t width;

						if (vecIndex == 0) {
							format.vecSeperator = format.c_vecSeperator;
							format.vecSepLen    = *format.c_vecSeperator ? 1 : 0;
						}
					str1:
						switch (format.argument_options) {
							case long_argument:
								if (format.field_width > 4) {
									goto exit;
								}
								ptr = (char_t *)&vec_val.u8[4*vecIndex + 4 - format.field_width];
								break;
							case short_argument:
								if (format.field_width > 2) {
									goto exit;
								}
								ptr = (char_t *)&vec_val.u8[2*vecIndex + 2 - format.field_width];
								break;
							case normal_argument:
								if (format.field_width > 1) {
									goto exit;
								}
								ptr = (char_t *)&vec_val.u8[vecIndex];
								break;
						}

						num_chars = 0;
						width = format.field_width;
						while (width-- && (c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar)) != EOF) {
							*ptr++ = c;
							++num_chars;
						}

						if (!num_chars) {
							MISRA_EXCEPTION_RULE_14_4()
							goto exit;
						}

						chars_read += num_chars;
						GOTO_GET_REMAINING_VEC_FIELDS(str1);
						#if __VEC__
						*va_arg(arg, MWVector128 *) = vec_val;
						#else
						va_arg_replacement2(arg, vec_val);
						#endif
						++items_assigned;
					} else {
						int_t width;
					str2:
						num_chars = 0;
						width = format.field_width;
						while (format.field_width-- &&
								((rval = ((*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar))) != EOF)) {
							c = rval;
							++num_chars;
						}
						c = rval;

						if (!num_chars) {
							goto exit;
						}

						GOTO_GET_REMAINING_VEC_FIELDS(str2);
					}
				}
				else
#endif /* __ALTIVEC__ */
#if defined(__m56800E__)
                if(format.suppress_assignment) {
				    arg_ptr = 0;
				} else {
					arg_ptr = va_arg(arg, char_t *);
				}
#endif /* __m56800E__ */
				if (arg_ptr) {
#if __STDC_WANT_LIB_EXT1__
					if (is_secure) {
						elem_valid = 1;
						elem_maxsize = va_arg(arg, rsize_t);
					}
#endif
					num_chars = 0;
					if (match_failure) {
						if (elem_maxsize > 0U) {
							*arg_ptr = 0;
						}
						MISRA_EXCEPTION_RULE_14_5()
						continue;
					} else {
						char_t * arg_start = arg_ptr;
						
						MISRA_EXCEPTION_RULE_12_4()
						MISRA_EXCEPTION_RULE_10_1()
						while (format.field_width-- &&
							(!is_secure || ((elem_valid = ((int_t)elem_maxsize > num_chars)) != 0)) &&
							((rval = ((*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar))) != EOF)) {
							c = rval;
#if _EWL_WIDE_CHAR
							if (format.argument_options == wchar_argument) {
								char_t tc = (char_t)c;
								MISRA_QUIET_SUSPICIOUS_PTR_CAST()
								(void)mbtowc(((wchar_t *)arg_ptr), &tc, 1U);
								c = tc;
								arg_ptr += sizeof(wchar_t);
							} else
#endif /* _EWL_WIDE_CHAR */
							{
								*arg_ptr++ = (char_t)c;
							}
							++num_chars;
						}

						c = rval;
						if (!num_chars || (is_secure && !elem_valid)) {
							if (!is_secure) {
								MISRA_EXCEPTION_RULE_14_4()
								goto exit;
							} else {
								match_failure = 1;
								if (elem_maxsize > 0U) {
									*arg_start = 0;
								}
								MISRA_EXCEPTION_RULE_14_5()
								continue;
							}
						}
						chars_read += num_chars;
						++items_assigned;
					}
				} else {
					num_chars = 0;
					MISRA_EXCEPTION_RULE_12_4()
					MISRA_EXCEPTION_RULE_10_1()
					while (format.field_width-- &&
						((rval = ((*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar))) != EOF)) {
						c = (char_t)rval;
						++num_chars;
					}
					c = (char_t)rval;
					if (!num_chars) {
						MISRA_EXCEPTION_RULE_14_4()
						goto exit;
					}
				}
				++conversions;
				break;

			case '%':
			
				needs_conversions = 1;
			
#ifdef __ALTIVEC__
				if (format.vec) {
					if (!is_secure) {
						goto exit;
					} else {
						match_failure = 1;
						continue;
					}
				}
#endif /* __ALTIVEC__ */
							
				if (match_failure) {
					MISRA_EXCEPTION_RULE_14_5()
					continue;
				} else {
					while (isspace((c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar)))) {
						++chars_read;
					}

					if (c != '%') {
						(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
						if (!is_secure) {
							MISRA_EXCEPTION_RULE_14_4()
							goto exit;
						} else {
							match_failure = 1;
							MISRA_EXCEPTION_RULE_14_5()
							continue;
						}
					}
					++chars_read;
				}
				break;

			case 's':
			
				needs_conversions = 1;
			
#ifdef __ALTIVEC__
				if (format.vec) {
					if (!is_secure) {
						goto exit;
					} else {
						match_failure = 1;
						continue;
					}
				}
#endif /* __ALTIVEC__ */
							
				if (!match_failure) {
					c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar);
					while(isspace(c)) {
						++chars_read;
						c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar);
					}

					(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
				}
			/* Fall Thru */
			MISRA_EXCEPTION_RULE_15_2()
			case '[':
#ifdef __ALTIVEC__
				if (format.vec) {
					if (!is_secure) {
						goto exit;
					} else {
						match_failure = 1;
						continue;
					}
				}
#endif /* __ALTIVEC__ */
#if defined(__m56800E__)
                if(format.suppress_assignment) {
				    arg_ptr = 0;
				} else {
					arg_ptr = va_arg(arg, char_t *);
				}
#endif /* __m56800E__ */
				if (arg_ptr) {
#if __STDC_WANT_LIB_EXT1__
					if (is_secure) {
						elem_valid = 1;
						MISRA_QUIET_SUSPICIOUS_PTR_CAST()
						elem_maxsize = va_arg(arg, rsize_t) - 1U;
					}
#endif

					num_chars = 0;

					if (match_failure) {
						if (elem_maxsize > 0U) {
							*arg_ptr = 0;
						}
						MISRA_EXCEPTION_RULE_14_5()
						continue;
					} else {
						char_t * arg_start = arg_ptr;
						MISRA_EXCEPTION_RULE_10_1()
						MISRA_EXCEPTION_RULE_12_4()
						while (format.field_width-- &&
							(!is_secure || ((elem_valid = ((int_t)elem_maxsize >= num_chars)) != 0)) &&
							((rval = ((*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar))) != EOF)) {
							c = rval;

							if (!tst_char_map(format.char_set, c)) {
								break;
							}
#if _EWL_WIDE_CHAR
							if (format.argument_options == wchar_argument) {
								char_t tc = (char_t) c;
								MISRA_QUIET_SUSPICIOUS_PTR_CAST()
								(void)mbtowc(((wchar_t *)arg_ptr), &tc, 1U);
								c = tc;
								arg_ptr += sizeof(wchar_t);
							} else
#endif /* _EWL_WIDE_CHAR */
							{
								*arg_ptr++ = (char_t)c;
							}
							++num_chars;
						}
						c = (char_t)rval;

						if (!num_chars || (is_secure && !elem_valid)) {
							(void)(*ReadProc)(ReadProcArg, (int_t)c, (int_t)__UngetAChar);
							if (!is_secure) {
								MISRA_EXCEPTION_RULE_14_4()
								goto exit;
							} else {
								match_failure = 1;
								if (elem_maxsize > 0U) {
									*arg_start = 0;
								}
								MISRA_EXCEPTION_RULE_14_5()
								continue;
							}
						}

						chars_read += num_chars;

#if _EWL_WIDE_CHAR
						if (format.argument_options == wchar_argument) {
							MISRA_QUIET_SUSPICIOUS_PTR_CAST()
							*(wchar_t*)arg_ptr = L'\0';
						} else
#endif /* _EWL_WIDE_CHAR */
						{
							*arg_ptr = 0;
						}

						++items_assigned;
					}
				} else {
					num_chars = 0;
					MISRA_EXCEPTION_RULE_12_4()
					while (format.field_width-- &&
						((rval = ((*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar))) != (int_t)EOF)) {
						c = rval;
						if (!tst_char_map(format.char_set, c)) {
							break;
						}

						++num_chars;
					}
					c = rval;

					if (!num_chars) {
						(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
						break;
					}
					chars_read += num_chars;
				}

				if (format.field_width >= 0) {
					(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
				}

				++conversions;

				break;

			case 'n':

#if defined(__m56800E__)
			{
				int_t * arg_iptr = 0;
				if (format.argument_options == char_argument) {
					arg_ptr = va_arg(arg,char_t *);
				} else {
					arg_iptr = va_arg(arg,int_t *);
				}

				switch (format.argument_options)
				{
					case normal_argument:
						if (arg_iptr) {
							(*((int_t *) arg_iptr)) = chars_read;
						}
						break;
					case short_argument:
						if (arg_iptr) {
							(*((int16_t *) arg_iptr)) = chars_read;
						}
						break;
					case long_argument:
						if (arg_iptr) {
							(*((int32_t *) arg_iptr)) = chars_read;
						}
						break;
					case char_argument:
						if (arg_ptr) {
							(*((char_t *) arg_ptr)) = chars_read;
						}
						break;
	#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
					case long_long_argument:
						if (arg_iptr) {
							(*((int64_t *) arg_iptr)) = chars_read;
						}
						break;
	#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)*/
				}
			}
#else /* !__m56800E__ */
				if (arg_ptr) {
					MISRA_QUIET_ENUM_SWITCH()
					MISRA_QUIET_SUSPICIOUS_PTR_CAST()
					switch (format.argument_options) {
						case normal_argument: * (int_t   *) arg_ptr = chars_read; break;
						case short_argument:  * (int16_t *) arg_ptr = (int16_t)chars_read; break;
						case long_argument:   * (int32_t *) arg_ptr = chars_read; break;
						case char_argument:   *             arg_ptr = (char_t)chars_read; break;

	#if _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF)
                        case long_long_argument: * (int64_t *) arg_ptr = chars_read; break;
	#endif /* _EWL_LONGLONG && (_EWL_C99_PRINTF_SCANF || _EWL_LONGLONG_PRINTF_SCANF) */

					}
#endif /* __m56800E__ */
				}
				MISRA_EXCEPTION_RULE_14_5()
				continue;

			case bad_conversion:
			default:
				MISRA_EXCEPTION_RULE_14_4()
				goto exit;
		}

	}


exit:

	if ((*ReadProc)(ReadProcArg, 0, (int_t)__TestForError) && (conversions == 0)) {
		MISRA_EXCEPTION_RULE_14_7()
		return(EOF);
	}

	if ((conversions == 0) && needs_conversions)
	{
		c = (*ReadProc)(ReadProcArg, 0, (int_t)__GetAChar);
		if ((*ReadProc)(ReadProcArg, 0, (int_t)__TestForError))
		{
			MISRA_EXCEPTION_RULE_14_7()
			return (EOF);
		}
		(void)(*ReadProc)(ReadProcArg, c, (int_t)__UngetAChar);
	}

	return(items_assigned);

	MISRA_EXCEPTION_RULE_16_7()
}

