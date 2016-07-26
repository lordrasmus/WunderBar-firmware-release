/*****************************************************
 misra.h -  Interface/collection with MISRA declarations.
 *****************************************************/

#ifndef _EWL_MISRA_H_
#define _EWL_MISRA_H_

#include <stdint.h>

_EWL_BEGIN_EXTERN_C

#define __MISRA__

typedef char            char_t;
typedef unsigned char   uchar_t;
typedef int             int_t;
typedef unsigned int    uint_t;
typedef float           f32_t;			/* float */
typedef double          f64_t;			/* double */
typedef long double     ldbl_t;			/* long double */

/*	Global PC-Lint 9 options (this needs to be fixed) */

/*lint -e438 		Warning:	Last value assigned to variable '*' not used */
/*lint -e440 		Warning:	irregularity clause(?) */
/*lint -e441 		Warning:	irregularity clause(?) */
/*lint -e443 		Warning:	irregularity clause(?) */
/*lint -e571 		Warning:	Suspicious cast(?) */
/*lint -e713 		Info:		Loss of precision (assignment) (unsigned * to *) */
/*lint -e732 		Info:		Loss of sign in assignment from * to unsigned * */
/*lint -e734 		Info:		Loss of precision in assignment */
/*lint -e737 		Info:		Loss of sign in promotion from * to unsigned * */
/*lint -e778 		Info:		Constant expression evaluates to 0 in operation ... */
/*lint -e817 		Info:		Conceivably negative subscript (-2) in operator '[' */
/*lint -e838 		Info:		Previously assigned value to variable ... has not been used */

/*lint -estring(974,*finite*)  +stack option is used to dump information for MISRA 2004 Rule 16.2. */
											/* Supress informative notes on stack size, only issue non-conformities. */

/*	these macros silence PC-Lint warnings that come up in Misra compliance testing, they do not have to be listed in the "Misra Compliance Exceptions" list */

#define MISRA_QUIET_MACRO_FUNCDEF()			/*lint -e{123} Lint macro/function define conflict (e.g., "int (getc)(FILE*)") */
#define MISRA_QUIET_NULL_POINTER_ARG()		/*lint -e{418} -e{668} -e{688} Allow (possible) NULL pointer arguments */
#define MISRA_QUIET_UNUSED_SYMBOL()			/*lint --e{550} Possibly (depending on configuration) unused symbol */
#define MISRA_QUIET_SUSPICIOUS_CAST()		/*lint -e{571} Suspicious cast */
#define MISRA_QUIET_UNUSED_ARGS()			/*lint --e{715} Possibly unused function argument(s) */
#define MISRA_QUIET_UNUSED_HEADER()			/*lint --e{766} Unused header file */
#define MISRA_QUIET_ENUM_SWITCH()			/*lint -e{787} -e{788} Not all enumerators used in switch() */
#define MISRA_QUIET_SUSPICIOUS_PTR_CAST()	/*lint -e{826} Suspicious pointer-to-pointer conversion */
#define MISRA_QUIET_UNUSED_MACRO()			/*lint --e{750} Local macro not referenced */
#define MISRA_QUIET_USES_LONG_LONG()		/*lint -e{435} Integral constant has long long precision */

/*	these macros silence PC-Lint warnings that are possibly (to be reviewed) bugs in PC-Lint 9 */

#define MISRA_QUIET_LINT_02()				/*lint -e{775} Non-negative quantity cannot be less than zero */
#define MISRA_QUIET_LINT_03()				/*lint -e{835} A zero has been given as argument to operator '-' (for -0.0) */
#define MISRA_QUIET_LINT_04()				/*lint -e{429} Custodial pointer? */
#define MISRA_QUIET_LINT_05()				/*lint -e{676} Possible negative subscript? */
#define MISRA_QUIET_LINT_06()				/*lint -e{774} Lint unsigned overflow bug */

/*	these macros silence PC-Lint Misra compliance warnings/errors, they have to be listed in the "Misra Compliance Exceptions" list */

#define MISRA_ALLOW_POINTER_CASTS()			/*lint -e920 -e926 -e927 -e928 -e929 MISRA 2004 Rule 11.4: Cast from pointer to pointer/void */
											/* Used to globally disable pointer casting related messages in some source files. */
#define MISRA_EXCEPTION_LONG_NAME()			/*lint -e{621} MISRA 2004 Rule 1.2, 1.4 and 5.1: Allow long internal name */
											/* Used to allow long file names, e.g., "__ewl_generic_count_leading_zero32". */
#define MISRA_EXCEPTION_RULE_1_2a()			/*lint -e{920} MISRA 2004 Rule 1.2: Cast from pointer to void */
											/* Used to silence "function result not used" warnings. */
#define MISRA_EXCEPTION_RULE_1_2b()			/*lint -e{931} MISRA 2004 Rule 1.2: Both sides have side effects */
											/* Used to generate more efficient copy code, e.g., "if (*p1++ != *p2++)". */
#define MISRA_EXCEPTION_RULE_1_2c()			/*lint -e{740} MISRA 2004 Rule 1.2: Unusual pointer cast (incompatible indirect types) */
											/* Used in vararg handling or to generate floating point values from bit patterns. */
#define MISRA_EXCEPTION_RULE_1_2d()			/*lint -e{414} MISRA 2004 Rule 1.2: Division by zero */
											/* Used to generate FP NaN results. */
#define MISRA_EXCEPTION_RULE_9_1()			/*lint -e{644} -e{771} MISRA 2004 Rule 9.1: Possible uninitialized symbol */
											/* Used when the symbol usage is consistent with its initialization */
#define MISRA_EXCEPTION_RULE_10_1()			/*lint -e{960} MISRA 2004 Rule 10.1: converting expressions */
											/* Used to allow implicit and explicit arithmetic conversions. */
#define MISRA_EXCEPTION_RULE_10_2()			/*lint -e{960} MISRA 2004 Rule 10.2: Complex returns */
											/* Used to allow conversions in return statements, e.g., "return NAN;". */
#define MISRA_EXCEPTION_RULE_10_3()			/*lint -e{960} MISRA 2004 Rule 10.3: Cast of expressions */
											/* Used to allow casting of expressions, e.g., "(int32_t)(x_i & 0x7F800000UL);". */
#define MISRA_EXCEPTION_RULE_10_4()			/*lint -e{960} MISRA 2004 Rule 10.4: cast of floating point */
											/* Used to allow casting of floating point expressions. */
#define MISRA_EXCEPTION_RULE_10_5()			/*lint -e{701} MISRA 2004 Rule 10.5: Shift left of signed quantity */
											/* Used to allow shifting of signed expressions, e.g., "a << 1". */
#define MISRA_EXCEPTION_RULE_11_3()			/*lint -e{923} MISRA 2004 Rule 11.3: Cast pointer/non-pointer */
											/* Used in pointer alignment computations, e.g., "((uint32_t)ptr & 3UL). */
#define MISRA_EXCEPTION_RULE_11_4()			/*lint -e{926} -e{927} -e{928} -e{929} MISRA 2004 Rule 11.4: cast from pointer to pointer */
											/* Used for opaque data structure accessing or floating point bit pattern manipulations, e.g., "(struct _FILE *)_file". */
#define MISRA_EXCEPTION_RULE_12_4()			/*lint -e{960} MISRA 2004 Rule 12.4: side effect righthand of && or || */
											/* Used to allow sideeffect in logical expressions, e.g., "if (isnan(x) || isnan(y)) ...". */
#define MISRA_EXCEPTION_RULE_12_7()			/*lint -e{703} -e{704} -e{960} MISRA 2004 Rule 12.7: Bitwise shift operator applied to signed underlying type */
											/* Used to allow >>, e.g., "ex >>= 23;". */
#define MISRA_EXCEPTION_RULE_12_7a()		/*lint -e{960} MISRA 2004 Rule 12.7: Bitwise operator (and(&),  or(|)) applied to signed underlying type */
											/* Used to allow bitwise "&" on signed types, e.g., "intpart & 1L". */
#define MISRA_EXCEPTION_RULE_12_7b()		/*lint -e{514} -e{960} MISRA 2004 Rule 12.7: Bitwise operator xor (^) used in logical expressions or with signed types */
											/* Used to allow '^' as xor in logical expresisons, e.g., "sign = ((_x < 0) ^ (y < 0)) ? -1 : 1;". */
#define MISRA_EXCEPTION_RULE_13_1()			/*lint -e{820} MISRA 2004 Rule 13.1: Boolean test of a parenthesized assignment */
											/* To be removed: Used to generate more efficient code, e.g., "while ((*q++ = *p++)) ... ". */
#define MISRA_EXCEPTION_RULE_13_3()			/*lint -e{777} MISRA 2004 Rule 13.3: Testing floats for equality */
											/* Used to allow floating point equality checks, e.g., "if (x == -INFINITY) ...". */
#define MISRA_EXCEPTION_RULE_13_7()			/*lint -e{506} -e{774} -e{845} MISRA 2004 Rules 13.7 and 14.1: Constant value logical expressions */
											/* Used to allow configuration dependent constant expressions, e.g., "if ((uint_t)math_errhandling & (uint_t)MATH_ERRNO) ...". */ 
											/* 506: * MISRA 2004 Rules 13.7 and 14.1: Constant value Boolean */
											/* 774: * Boolean within 'if' always evaluates to true */
											/* 845: * The right argument to operator '&&' is certain to be 0 */
#define MISRA_EXCEPTION_RULE_14_4()			/*lint -e{801} MISRA 2004 Rule 14.4: goto stmt */
											/* Used to allow "goto" statements , e.g., for code sharing. */ 
#define MISRA_EXCEPTION_RULE_14_5()			/*lint -e{960} MISRA 2004 Rule 14.5: continue stmt */
											/* Used to allow "continue" statements. */ 
#define MISRA_EXCEPTION_RULE_14_6()			/*lint -e{960} MISRA 2004 Rule 14.6: More than one break terminates loop */
											/* Used to allow more than one "break;" in a loop. */											
#define MISRA_EXCEPTION_RULE_14_7()			/*lint -e{904} MISRA 2004 Rule 14.7: Return statement before end of function */
											/* Used to allow more than one "return;" in a function */											
#define MISRA_EXCEPTION_RULE_15_2()			/*lint -e{616} -e{825} MISRA 2004 Rule 15.2: fall-throught */
											/* Used to allow fall-throught in switch statements. */
#define MISRA_EXCEPTION_RULE_16_1()			/*lint -e{960} MISRA 2004 Rule 16.1: Variable arg list */
											/* Used to allow variable argument lists, e.g., "int printf(const char* format, ...);". */
#define MISRA_EXCEPTION_RULE_16_2()			/*lint -e974 MISRA 2004 Rule 16.2: Functions shall not call themselves, directly or indirectly */											
											/* Used to allow calls through function pointers*/
#define MISRA_EXCEPTION_RULE_16_7()			/*lint -e{818} MISRA 2004 Rule 16.7: Pointer parameter could be declared as pointing to const */
											/* Used when implementing a C std routine which could use const arguments. */
#define MISRA_EXCEPTION_RULE_17_3()			/*lint -e{946} -e{923} -e{947} MISRA 2004 Rule 17.2 and 17.3: Pointer operations */
											/* Used when pointer arithmetic is required, e.g. "buffer_len = file->buffer_ptr - file->buffer;" . */
#define MISRA_EXCEPTION_RULE_19_7()			/*lint -save -e961 MISRA 2004 Rule 19.7: Function-like macro */
											/* Used when a function-like macro is defined. */
#define MISRA_EXCEPTION_RULE_20_4()			/*lint -e{586} MISRA 2004 Rule 20.4: malloc shall not be used */
											/* Used when malloc() has to be used. */
#define MISRA_EXCEPTION_RULE_20_5()			/*lint -e{586} MISRA 2004 Rule 20.5: errno shall not be used */
											/* Used when errno has to be used. */
#define MISRA_EXCEPTION_RULE_20_8()			/*lint -e{586} MISRA 2004 Rule 20.8: raise shall not be used */
											/* Used when raise() has to be used. */
#define MISRA_EXCEPTION_RULE_20_9()			/*lint -e829 MISRA 2004 Rule 20.9: <stdio.h> shall not be used */
											/* Used when <stdio.h> has to be included. */
#define MISRA_EXCEPTION_RULE_20_11()		/*lint -e{586} MISRA 2004 Rule 20.11: abort shall not be used */
											/* Used when abort() has to be used. */
#define MISRA_EXCEPTION_RULE_20_12()		/*lint -e{586} MISRA 2004 Rule 20.12: mktime shall not be used */
											/* Used when mktime() has to be used. */

/* These are also define in <ansi_params.h> because they have to be used in header files */

#define _MISRA_QUIET_DO_WHILE_0()			/*lint -e{717} Allow "do { ... } while(0)" */

#define _MISRA_EXCEPTION_STD_TYPE()			/*lint -e(970) MISRA 2004 Rule 6.3: Use of modifier or type outside of a typedef */
											/* Used when standard types/qualifiers have to used in headers. */
#define _MISRA_EXCEPTION_RULE_19_6()		/*lint -save -e960 MISRA 2004 Rule 19.6: Use of '#undef' is discouraged */
											/* Used when #undef has to used to control compilation. */
#define _MISRA_EXCEPTION_FLOAT_CAST()		/*lint -e(740) -e(826) -e(929) -e(970) Exceptions to allow bit pattern -> floating point casts */
											/* Used to generate floating point values from bit patterns. */
											/* 740: * MISRA 2004 Rule 1.2: unusual pointer casts */
											/* 826: * unusual pointer cast */
											/* 970: * MISRA 2004 Rule 6.3: Use of modifier or type outside of a typedef */
#define _MISRA_EXCEPTION_CMATH_MACROS()		/*lint -emacro(506 970, signbit, fpclassify) */
											/* Used to implement signbit and fpclassify macros. */
											/* 506: * MISRA 2004 Rules 13.7 and 14.1: Constant value Boolean  */
											/* 970: * MISRA 2004 Rule 6.3: Use of modifier or type outside of a typedef */
#define _MISRA_EXCEPTION_STDIO_MACROS()		/*lint -emacro(929 960 970, __getc, __putc) */
											/* Used to implement __getc and __putc macros. */
											/* 929: * MISRA 2004 Rule 11.4: Cast from pointer to pointer */
											/* 960: * MISRA 2004 Rule 10.1: Cast/conversion of complex integer expression */
											/* 970: * MISRA 2004 Rule 6.3: Use of modifier or type outside of a typedef */
#define _MISRA_EXCEPTION_MATHAPI_MACROS()	/*lint -emacro(929, GET_DOUBLE_HI_WORD, GET_DOUBLE_UHI_WORD, GET_DOUBLE_LO_WORD, GET_DOUBLE_ULO_WORD) */
											/* Used to implement GET_DOUBLE_HI_WORD, GET_DOUBLE_UHI_WORD, GET_DOUBLE_LO_WORD, GET_DOUBLE_ULO_WORD macros (to access double hi/lo words). */
											/* 929: * MISRA 2004 Rule 11.4: Cast from pointer to pointer */
#define _MISRA_EXCEPTION_MATHAPISP_MACROS()	/*lint -emacro(929, GET_FLOAT_WORD, GET_FLOAT_UWORD, SET_FLOAT_WORD, SET_FLOAT_UWORD) */
											/* Used to implement GET_FLOAT_WORD, GET_FLOAT_UWORD, SET_FLOAT_WORD, SET_FLOAT_UWORD macros (to access floats as words). */
											/* 929: * MISRA 2004 Rule 11.4: Cast from pointer to pointer */
#define _MISRA_EXCEPTION_STDARG_MACROS()	/*lint -emacro( (826) , __va_start) -emacro(923 926 928 970, __va_start) */
											/* Used to implement __va_start */
											/* 826: * Suspicious pointer-to-pointer conversion */                                										
											/* 923: * MISRA 2004 Rule 11.3: Cast pointer/non-pointer */     											
											/* 929: * MISRA 2004 Rule 11.4: Cast from pointer to pointer */	  			
											/* 970: * MISRA 2004 Rule 6.3: Use of modifier or type outside of a typedef */											
#define _MISRA_RESTORE()					/*lint -restore Used to restore saved options */

_EWL_END_EXTERN_C

#endif /* _EWL_MISRA_H_ */
