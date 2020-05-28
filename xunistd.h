#ifndef XUNISTD_H_DEFINED_
#define XUNISTD_H_DEFINED_

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_LIBC_H
#include <libc.h>			/* e.g. NeXT Mach 3.x system */
#endif

#if defined(__NeXT__) && defined(__GNUG__)
/* g++ on NeXT has its own libc.h which just includes stdlib.h,
but that lacks prototypes for three functions that we need.  The
vendor-provided libc.h has them. */
extern "C" {
extern int		access ARGS((const char *, int));
extern int		ioctl ARGS((int, long, ...));
extern unsigned int	sleep ARGS((unsigned int seconds));
};
#endif

#endif /* XUNISTD_H_DEFINED_ */
