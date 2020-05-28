#ifndef CH_H_DEFINED_
#define CH_H_DEFINED_

#define BIBTEX_COMMENT_PREFIX	((int)'%')	/* comment character in BibTeX files */
			/* (I hope this will be standard in BibTeX 1.0) */

#define BYTE_VAL(c)	((unsigned int)((c) & 0xff))

#define CTL(x)		((int)(x) & 037)	/* make ASCII control character */

#define CH_BACKSPACE	CTL('H')

#if defined(HAVE_ALERT_CHAR)
#define CH_BELL		((int)'\a')
#else
#define CH_BELL		((int)'\007')
#endif

#define CH_DELETE	((int)'\177')

#define CH_ESCAPE	((int)'\033')

#define CH_LINE_KILL	CTL('U')

#define CH_NUL		((int)'\000')

#define CH_REPRINT	CTL('R')

#define CH_WORD_ERASE	CTL('W')

#define COMMENT_PREFIX	((int)'%')	/* comment character in initialization files */

#define	ERROR_PREFIX	"??"	/* this prefixes all error messages */

#define LINEBREAK	((int)CTL('n')) /* line break character */

#define META(x)		((int)(x) | 0200)	/* make GNU Emacs meta character */

#define PARBREAK	CTL('p') /* paragraph break character */

#define WARNING_PREFIX	"%%"	/* this prefixes all warning messages */

#endif /* CH_H_DEFINED_ */
