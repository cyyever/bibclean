#ifndef XSTAT_H_DEFINED_
#define XSTAT_H_DEFINED_

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>		/* needed on MIPS RC6280 RISCos systems */
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#else
#ifdef HAVE_STAT_H
#include <stat.h>
#endif
#endif

#endif /* XSTAT_H_DEFINED_ */
