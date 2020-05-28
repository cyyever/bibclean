#ifndef CUSTOM_H_DEFINED_
#define CUSTOM_H_DEFINED_

/***********************************************************************
This file is included by config.h, but is not part of that file
because it must be protected from autoconf twiddling.  Its purpose is
to provide workarounds for limitations of autoconf.
***********************************************************************/

/* Make a preliminary sanity check on which pattern matching we will use */

#if defined(HAVE_REGEXP)
#if defined(HAVE_RECOMP) || defined(HAVE_PATTERNS) || defined(HAVE_OLDCODE)
?? Define only one of HAVE_OLDCODE, HAVE_PATTERNS, HAVE_REGEXP, and HAVE_RECOMP
#endif
#endif

#if defined(HAVE_RECOMP)
#if defined(HAVE_REGEXP) || defined(HAVE_PATTERNS) || defined(HAVE_OLDCODE)
?? Define only one of HAVE_OLDCODE, HAVE_PATTERNS, HAVE_REGEXP, and HAVE_RECOMP
#endif
#endif

#if defined(HAVE_PATTERNS)
#if defined(HAVE_RECOMP) || defined(HAVE_REGEXP) || defined(HAVE_OLDCODE)
?? Define only one of HAVE_OLDCODE, HAVE_PATTERNS, HAVE_REGEXP, and HAVE_RECOMP
#endif
#endif

#if defined(HAVE_OLDCODE)
#if defined(HAVE_PATTERNS) || defined(HAVE_RECOMP) || defined(HAVE_REGEXP)
?? Define only one of HAVE_OLDCODE, HAVE_PATTERNS, HAVE_REGEXP, and HAVE_RECOMP
#endif
#endif

#if !(defined(HAVE_REGEXP) || defined(HAVE_RECOMP))
#if !(defined(HAVE_PATTERNS) || defined(HAVE_OLDCODE))
#define HAVE_PATTERNS	1
#endif
#endif

/* Ensure that only ONE of HAVE_TERMIOS_H, HAVE_TERMIO_H, and HAVE_SGTTY_H
is defined, with preference in that order */

#if defined(HAVE_TERMIOS_H)
#undef HAVE_TERMIO_H
#undef HAVE_SGTTY_H
#else
#if defined(HAVE_TERMIO_H)
#undef HAVE_TERMIOS_H
#undef HAVE_SGTTY_H
#endif
#endif /* defined(HAVE_TERMIOS_H) */

#if !defined(HAVE_TERMIOS_H) && !defined(HAVE_TERMIO_H) && !defined(HAVE_SGTTY_H)
#if defined(sun)
/* Sun acc compiler produces warnings from inclusion of <sgtty.h>, causing
configure to conclude that it is not available */
#define HAVE_SGTTY_H	1
#endif
#endif

#if (defined(__LCC__) || defined(__cplusplus) || defined(c_plusplus))
/* lcc is a strictly Standard C conforming compiler.  However, because
of an lcc 3.4b header file bug on Sun Solaris 2.x, configure will
conclude that STDC_HEADERS and HAVE_STDC should NOT be defined.  We
therefore override any choices made in config.h.

If a C++ compiler is used, override possibly erroneous conclusions
made by configure about header files and prototypes. */

#undef STDC_HEADERS
#define STDC_HEADERS 1

#undef HAVE_STDC
#define HAVE_STDC 1
#endif

#undef ARGS
#define ARGS(plist)	plist

#define RCSID(s) /*@unused@*/ static char rcsid[] = s;

#undef VOID
#define VOID		void

/* Although Microsoft C 6.0 and 7.0 define _MSC_VER, compiler versions
5.0 and 5.1 do not, so the only distinguishing symbol left is M_I86,
which Watcom C unfortunately also defines, sigh... */

#if defined(M_I86) && !defined(__WATCOMC__)
#define CONST		/* bug workaround for IBM PC Microsoft C compilers */
#else
#define CONST const
#endif

#include "xstdio.h"

#if defined(_WIN32)
#define OS_PCDOS	1
#define OS_UNIX		0
#define OS_VAXVMS	0
#elif defined(_VMS)
#define OS_PCDOS	0
#define OS_UNIX		0
#define OS_VAXVMS	1
#else
#define OS_PCDOS	0
#define OS_UNIX		1
#define OS_VAXVMS	0
#endif

#if OS_PCDOS

#define SEP_COMP " ;,|"	/* separators between filename components */
#define SEP_PATH "\\"	/* separators between directory path and filename */

#elif OS_UNIX

#define SEP_COMP " ;:,|" /* separators between filename components */
#define SEP_PATH "/"	/* separators between directory path and filename */

#elif OS_VAXVMS

#define SEP_COMP " ;,|"	/* separators between filename components */
#define SEP_PATH ":]"	/* separators between directory path and filename */
			/* first char is what we default to */
#endif

#if defined(FREE_CAST_IS_CHAR_STAR)
#define FREE(p)		free((char*)(p))
#else
#define FREE(p)		free((void*)(p))
#endif

#define IN_SET(set, c)	(strchr((set), (int)(c)) != (char *)NULL)

/***********************************************************************
Put any other local customizations here, when configure does not
automatically generate the config.h settings that you want.  Please
report them back to the package author, <beebe@math.utah.edu>, for
inclusion in future releases.
***********************************************************************/

#endif /* CUSTOM_H_DEFINED_ */
