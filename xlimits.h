#ifndef XLIMITS_H_DEFINED_
#define XLIMITS_H_DEFINED_

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

/* At least one system (NeXT Mach 3.3) was missing these values, so
   make sure they are defined.  For bibclean, their exact values are
   not critical; they just need to be large relative to the size of
   strings processed. */

#if !defined(INT_MAX)
#define INT_MAX 2147483647
#endif

#if !defined(LONG_MAX)
#define LONG_MAX 2147483647L
#endif

#endif /* XLIMITS_H_DEFINED_ */
