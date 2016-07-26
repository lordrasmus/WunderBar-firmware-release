/* EWL
 * Copyright © 1995-2009 Freescale Corporation.  All rights reserved.
 *
 * $Date: 2012/07/12 22:31:13 $
 * $Revision: 1.2 $
 */

/* This header file contains the definitions connected with file structures
 * to separate their definition from the stdio prototypes
 */

#ifndef _EWL_FILE_STRUC_H
#define _EWL_FILE_STRUC_H

#include "ansi_parms.h"

#include "size_t.h"

#if _EWL_WIDE_CHAR
	#include <wchar_t.h>
#endif /* _EWL_WIDE_CHAR */

#pragma pack(push,4)

_EWL_BEGIN_NAMESPACE_STD
_EWL_BEGIN_EXTERN_C

typedef enum __file_kinds
{
	__closed_file,
	__disk_file,
	__console_file,
	__unavailable_file
} __file_kinds;

typedef enum __open_modes
{
	__must_exist,
	__create_if_necessary,
	__create_or_truncate
} __open_modes;

#if _EWL_WIDE_CHAR
typedef enum __file_orientation
{
	__unoriented,
	__char_oriented,
	__wide_oriented
} __file_orientation;
#endif /* _EWL_WIDE_CHAR */

typedef enum __io_modes
{
	__read_mode			= 1U,
	__write_mode		= 2U,
	__read_write_mode	= 3U,
	__append_mode		= 4U
} __io_modes;

typedef struct
{
	unsigned char	open_mode;
	unsigned char	io_mode;
	unsigned char	buffer_mode;
	unsigned char	file_kind;

	#if _EWL_WIDE_CHAR
		unsigned char	file_orientation;
	#endif /* _EWL_WIDE_CHAR */

	unsigned char	binary_io;
}  __file_modes;

typedef enum __io_states
{
	__neutral,
	__writing,
	__reading,
	__rereading
} __io_states;

typedef struct
{
	unsigned char	io_state;
	unsigned char	free_buffer;
	unsigned char	eof;
	unsigned char	error;
}  __file_state;

typedef unsigned long __file_handle;

#if defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL != 0)
	#include <arm/file_struc_aeabi.h>

#else

	/** fpos_t is an object capable of recording all the information
	 *	needed to specify uniquely every position within a file
	 */
//typedef unsigned long fpos_t;

//typedef struct _FILE FILE;

#ifndef _IONBF
#define _IONBF 0
#endif

#ifndef _IOLBF
#define _IOLBF 1
#endif

#ifndef _IOFBF
#define _IOFBF 2
#endif

#ifndef	_EWL_FILEIO_BUFSIZ
	#define _EWL_FILEIO_BUFSIZ 4096
#endif

#endif /* _AEABI_PORTABILITY_LEVEL */

typedef enum __io_results
{
	__no_io_error,
	__io_error,
	__io_EOF
} __io_results;

#if __dest_os == __starcore
typedef void * __ref_con;
#endif
typedef void (* __idle_proc)  (void);
#if __dest_os == __starcore
//typedef int  (* __pos_proc)   (__file_handle file, fpos_t * position, int mode, __ref_con ref_con);
typedef int  (* __io_proc)    (__file_handle file, unsigned char * buff, size_t * count, __ref_con ref_con);
#else
//typedef int  (* __pos_proc)   (__file_handle file, fpos_t * position, int mode);
typedef int  (* __io_proc)    (__file_handle file, unsigned char * buff, size_t * count);
#endif
typedef int  (* __close_proc) (__file_handle file);

#define __ungetc_buffer_size 2

//struct _FILE
//{
//	__file_handle		handle;
//	__file_modes		mode;
//	__file_state		state;
//
//	#if _EWL_OS_DISK_FILE_SUPPORT
//		unsigned char	is_dynamically_allocated;
//	#endif /* _EWL_OS_DISK_FILE_SUPPORT */
//
//	unsigned char		char_buffer;
//	unsigned char		char_buffer_overflow;
//	unsigned char		ungetc_buffer[__ungetc_buffer_size];
//
//	#if _EWL_WIDE_CHAR
//		wchar_t			ungetwc_buffer[__ungetc_buffer_size];
//	#endif /* _EWL_WIDE_CHAR */
//
//	unsigned long		position;
//	unsigned char *		buffer;
//	unsigned long		buffer_size;
//	unsigned char *		buffer_ptr;
//	unsigned long		buffer_len;
//	unsigned long		buffer_alignment;
//	unsigned long		saved_buffer_len;
//	unsigned long		buffer_pos;
//	__pos_proc			position_proc;
//	__io_proc			read_proc;
//	__io_proc			write_proc;
//	__close_proc		close_proc;
//#if __dest_os == __starcore
//	__ref_con			ref_con;
//#endif
//	#if _EWL_OS_DISK_FILE_SUPPORT
//		struct _FILE *	next_file_struct;
//	#endif /* _EWL_OS_DISK_FILE_SUPPORT */
//};


#ifndef stdin
#define stdin  	(&__std(__files[0]))
#endif

#ifndef stdout
#define stdout	(&__std(__files[1]))
#endif

#ifndef stderr
#define stderr	(&__std(__files[2]))
#endif

//extern _EWL_IMP_EXP_C struct _FILE __files[];

	/** BUFSIZ is constant expression that is the size of the buffer used by the
	 *	setbuf function.
	 */
#define BUFSIZ _EWL_FILEIO_BUFSIZ

	/** FOPEN_MAX is aconstant expression that is the minimum number of files that
	 *	the implementation guarantees can be open simultaneously
	 */
#if !(defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL != 0))
#if !_EWL_OS_DISK_FILE_SUPPORT
	#if !_EWL_CONSOLE_SUPPORT
		#ifndef FOPEN_MAX
			#define FOPEN_MAX		0
		#endif
	#else
		#ifndef FOPEN_MAX
			#define FOPEN_MAX		3
		#endif
	#endif
	#define _STATIC_FILES		3
#else
	#define FOPEN_MAX			35
	#define _STATIC_FILES		4
#endif
#endif /* !(defined(_AEABI_PORTABILITY_LEVEL) && (_AEABI_PORTABILITY_LEVEL != 0)) */

#ifndef _EWL_FILENAME_MAX
#define _EWL_FILENAME_MAX		256
#endif

	/** FILENAME_MAX is a constant expression that is the size needed for an array of
	 *	char large enough to hold the longest file name string that the implementation
	 *	guarantees can be opened.
	 *
	 * 	Applies only when DISK IO is supported.
	 */
#ifndef FILENAME_MAX
#define FILENAME_MAX			_EWL_FILENAME_MAX
#endif

	/** L_tmpnam is a constant expression that is the size needed for an array of
	 *	char large enough to hold a temporary file name string generated by the tmpnam
	 *	function.
	 *
	 * 	Applies only when DISK IO is supported.
	 */
#ifndef L_tmpnam
#define	L_tmpnam				_EWL_FILENAME_MAX
#endif

	/** TMP_MAX is a constant expression that is the maximum number of unique
	 *	file names that can be generated by the tmpnam function.
	 *
	 * 	Applies only when DISK IO is supported.
	 */
#ifndef TMP_MAX
#define TMP_MAX					256
#endif

#if defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
	#define	L_tmpnam_s			_EWL_FILENAME_MAX
	#define TMP_MAX_S			TMP_MAX
#endif /* __STDC_WANT_LIB_EXT1__ */

	/** SEEK_SET, SEEK_CUR, SEEK_END expand to integer constant expressions with
	 *	distinct values, suitable for use as the third argument to the fseek function.
	 *	The specified position is the beginning of the file if whence is SEEK_SET,
	 *	the current value of the file position indicator if SEEK_CUR, or
	 *	end-of-file if SEEK_END.
	 */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

_EWL_END_EXTERN_C
_EWL_END_NAMESPACE_STD

#pragma pack(pop)

#endif /* _EWL_FILE_STRUC_H */
