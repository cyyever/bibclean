#ifndef XSTDBOOL_H_DEFINED_
#define XSTDBOOL_H_DEFINED_

/*
** ISO C99 (ISO/IEC 9899:TC3 September 2007) requires that <stdbool.h>
** define these four macros:
**
**	#define bool				_Bool
**	#define true				1
**	#define false				0
**	#define __bool_true_false_are_defined	1
**
** Most systems hide those definitions unless (__STDC_VERSION__ - 0)
** >= 199901L.
**
** The type _Bool is an unsigned integer type that tests on many
** systems show almost always is a 1-byte value (56 / 58), or rarely,
** a 4-byte value (2 / 58) (on Apple Mac OS X 10.5.8 PowerPC
** (code-named Leopard)).
**
** ISO C++98 (ISO/IEC 14882 1998-09-01) took a different approach.  It
** defines __cplusplus >= 199711L, and requires that bool, true, and
** false be literals (NOT preprocessor definitions) known to the
** compiler, and that bool cannot be further qualified (signed,
** unsigned, short, long, long long, ...).  It says that sizeof(bool)
** is implementation-defined, and is not required to be 1.  Because
** the three literals are native to the language, no header file is
** needed to use them.
**
** Thus, the underlying type is _Bool in C99, and bool in C++, but
** both should be identical.  Outside this header file and config.h,
** we use only bool, true, and false.  Here, we have to deal with
** compilers that lack <stdbool.h> but recognize _Bool, or have
** <stdbool.h> but not _Bool, as well as c89, traditional cc, c99, and
** C++ environments.
*/

#if defined(__cplusplus)

/*
** Because 1999 ISO Standard C requires that bool, true, and false be
** macros, guarantee that also in a C++ environment.
*/

#define _Bool	bool
#define true	true
#define false	false

#elif defined(HAVE_STDBOOL_H)

#undef	bool
#undef	true
#undef	false

#if defined(__LCLINT__)
typedef enum { false, true } bool;	/* because bool is not known to lclint or splint */
/* NB: lclint and splint will need:  -booltype bool -booltrue true -boolfalse false */
#endif

#include <stdbool.h>

#if !defined(HAVE__BOOL)
typedef int _Bool;		/* some systems have <stdbool.h> but no _Bool, sigh... */
#endif

#if !defined(__bool_true_false_are_defined)

/*
** Systems that have <stdbool.h> usually make it effectively empty in
** pre-C99 environments.  However, if it does define bool, true, and
** false, it has to declare another testable symbol that we can use.
*/

#define	bool	_Bool
#define	true	1
#define	false	0

#endif /* !defined(__bool_true_false_are_defined) */

#else /* NOT defined(HAVE_STDBOOL_H) and NOT defined(__cplusplus) */

/*
** Some C compilers now recognize _Bool as a language keyword, and
** reject attempts to create a typedef for it, but still lack
** <stdbool.h>.  We therefore remap that keyword to a private type.
*/

#define _Bool my_boolean_t

typedef enum { __false = 0, __true = 1 } _Bool;

#define true	__true
#define false	__false
#define bool	_Bool

#endif /* defined(__cplusplus) */

#undef __bool_true_false_are_defined
#define __bool_true_false_are_defined 1

#endif /* XSTDBOOL_H_DEFINED_ */
