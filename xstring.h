#ifndef XSTRING_H_DEFINED_
#define XSTRING_H_DEFINED_

#ifdef HAVE_STRING_H
#include <string.h>
#else
#endif

extern void	Memmove ARGS((void *target_, const void *source_, size_t n_));
extern void	*Memset ARGS((void *target_, int value_, size_t n_));
extern char	*Strdup ARGS((const char *));
extern int	stricmp ARGS((const char *, const char *));
extern char	*stristr ARGS((const char *s_, const char *sub_));
extern int	strnicmp ARGS((const char *s1_, const char *s2_, size_t n_));

#define STREQUAL(a,b)	(strcmp(a,b) == 0)
#define STRGREATER(a,b)	(strcmp(a,b) > 0)

#endif /* XSTRING_H_DEFINED_ */
