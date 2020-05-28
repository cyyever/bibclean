/* config.h.  Generated automatically by configure.  */
/* WARNING: Do NOT edit the config.h file; instead, put any needed
changes in custom.h, so that they can override assumptions made in the
automatically-generated config.h file */

#define STDC_HEADERS 1

/* #undef _ALL_SOURCE */

/* #undef _POSIX_SOURCE */

#define HAVE_STDC 1

/* Define if you want old pattern-less string matching in .ini files */
/* #undef HAVE_OLDCODE */

/* Define if you want standard bibclean-specific pattern in .ini files */
#define HAVE_PATTERNS	1

/* Define if you want regular-expression patterns in .ini files using re_comp() */

/* #undef HAVE_RECOMP */
/* #undef HAVE_REGEXP */

#if !(defined(__cplusplus) || defined(c_plusplus))
/* Define to empty if the keyword does not work, but do nothing in */
/* C++ environment, because autoconf 2.4 gets the wrong answer from */
/* the AC_C_CONST test program under C++ compilation. */
#define const
#endif

/* Define if your free() requires (char*) cast instead of (void*) */
/* #undef FREE_CAST_IS_CHAR_STAR */

/* Define if your compiler recognizes \a as an alert (ASCII BEL) character */
#define HAVE_ALERT_CHAR 1

/* Library functions that we need */

#define HAVE_GETCWD 1
#define HAVE_GETWD 1
#define HAVE_STRCSPN 1
/* #undef HAVE_STRDUP */
#define HAVE_STRSPN 1
#define HAVE_STRSTR 1
#define HAVE_STRTOD 1
#define HAVE_STRTOL 1

/* Header files that we might need */

#define HAVE_CONIO_H 1
#define HAVE_CTYPE_H 1
/* #undef HAVE_DESCRIP_H */
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
/* #undef HAVE_IODEF_H */
#define HAVE_IO_H 1
/* #undef HAVE_JPIDEF_H */
#define HAVE_LIMITS_H 1
/* #undef HAVE_OSFCN_H */
/* #undef HAVE_PWD_H */
#define HAVE_REGEXP_H 1
/* #undef HAVE_REGEX_H */
/* #undef HAVE_RMS_H */
#define HAVE_SGTTY_H 1
/* #undef HAVE_SSDEF_H */
/* #undef HAVE_STAT_H */
/* #undef HAVE_STDDEF_H */
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
/* #undef HAVE_SYS_IOCTL_H */
/* #undef HAVE_SYS_PARAM_H */
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_TERMIOS_H 1
#define HAVE_TERMIO_H 1
#define HAVE_TIME_H 1
/* #undef HAVE_TT2DEF_H */
/* #undef HAVE_TTDEF_H */
/* #undef HAVE_TYPES_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_UNIXIO_H */

#define RCSID(s) static char rcsid[] = s;

#define OS_PCDOS	1

/* NB: some systems have Standard C header files, but compilers that
do not accept function prototypes (e.g. HP HP-UX 9.0.3 /bin/cc) */
#if defined(STDC_HEADERS) && (defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus))
#define ARGS(plist)	plist
#define VOID		void
#else
#define ARGS(plist)	()
#define VOID
#endif

#define SCREEN_LINES	24

/***********************************************************************
The limit on file name lengths is non-standard:

--------	-------------	------------------------------------------
Name		Definition	System
--------	-------------	------------------------------------------
FNMAX		<stdio.h>	PCC-20
MAXPATH		<dir.h>		Turbo C 2.0, C and C++ 3.0, and TopSpeed C
_MAX_PATH	<stdlib.h>	Microsoft C 5.0, 6.0, 7.0 and TopSpeed C
MAXPATHLEN	<sys/param.h>	Sun OS (4.2BSD), 4.3BSD, Gould UTX/32,
				HPUX, KCC-20, AIX (RT, RS, PS/2, 370),
				HP/Apollo DomainOS, DEC Alpha (OSF/1)
PATH_MAX	<stdio.h>	SYS V (Silicon Graphics)
PATH_MAX	<limits.h>	POSIX, DEC Alpha (OSF/1)
FILENAME_MAX	<stdio.h>	Intel RMX, NeXT Mach, Turbo C/C++ 3.0
--------	-------------	------------------------------------------

***********************************************************************/

#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif

#if defined(HAVE_LIMITS_H)
#include <limits.h>
#endif

#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#if !defined(MAXPATHLEN) && defined(FNMAX)
#define MAXPATHLEN FNMAX
#endif

#if !defined(MAXPATHLEN) && defined(MAXPATH)
#define MAXPATHLEN MAXPATH
#endif

#if !defined(MAXPATHLEN) && defined(_MAX_PATH)
#define MAXPATHLEN _MAX_PATH
#endif

#if !defined(MAXPATHLEN) && defined(PATH_MAX)
#define MAXPATHLEN PATH_MAX
#endif

#if !defined(MAXPATHLEN) && defined(FILENAME_MAX)
#define MAXPATHLEN FILENAME_MAX
#endif

#if !defined(MAXPATHLEN) && defined(FNMAX)
#define MAXPATHLEN 1024
#endif

#if defined(OS_PCDOS)
#define SEP_COMP " ;,|"	/* separators between filename components */
#define SEP_PATH "\\"	/* separators between directory path and filename */
#endif

#if defined(OS_UNIX)
#define SEP_COMP " ;:,|" /* separators between filename components */
#define SEP_PATH "/"	/* separators between directory path and filename */
#endif

#include <stdio.h>
#include <custom.h>

#if !defined(HAVE_IOCTL_PROTOTYPE)
#if defined(__cplusplus) || defined(c_plusplus)
/* system files should have: extern "C" {int ioctl ARGS((int, int, ...));} */
#else
extern int ioctl ARGS((int, int, ...));
#endif
#endif

#if defined(FREE_CAST_IS_CHAR_STAR)
#define FREE(p)		free((char*)(p))
#else
#define FREE(p)		free((void*)(p))
#endif
