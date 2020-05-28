#ifndef XSTDIO_H_DEFINED_
#define XSTDIO_H_DEFINED_

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#if defined(__LCC__) || !defined(HAVE_FILENO_DECL)
extern int	(fileno)(FILE *);	/* fileno() not in lcc <stdio.h> */
#endif /* defined(__LCC__) || !defined(HAVE_FILENO_DECL) */

/* There are several variants of the name of the variable that records
   the longest acceptable file name, so create our own instead. */

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

#if !defined(MAXPATHLEN) && defined(NAME_MAX)
#define MAXPATHLEN NAME_MAX
#endif

/* Hewlett-Packard HP-UX 10.20 has FILENAME_MAX == NAME_MAX == 14, but
   the file system actually supports names up to 255 characters in length,
   so override ridiculously small values! */
#if !defined(MAXPATHLEN) || (MAXPATHLEN < 255)
#undef MAXPATHLEN
#define MAXPATHLEN 255
#endif

#endif /* XSTDIO_H_DEFINED_ */
