/***********************************************************************

==========
BACKGROUND
==========

This file contains an implementation of limited regular-expression
pattern matching code.  The pattern syntax is simpler, more limited,
and different from normal regular-expression pattern matching syntax.
It is described in more detail below.

The motivation for this new code is that I found considerable
inconsistency in the matching behavior between versions of either
re_comp()/re_exec() or compile()/step() on these systems

	DECstation 3100
	IBM 3090
	IBM PS/2
	IBM RS/6000 AIX 3.2
	NeXT Mach 3.0
	Silicon Graphics IRIX 4.0
	Stardent OS 2.2
	Sun SPARC

That makes use of those regular-expression pattern matching unreliable
across systems.

One possible solution would be to use the GNU re_comp() and re_exec()
from the regexp distribution on prep.ai.mit.edu (as of writing,
pub/gnu/regex-0.11.*).  However, that code is large (5000+ lines), and
its installation uses configuration facilities that only work under
some variants of UNIX, and are completely useless on other operating
systems.

By contrast, the pattern matching code here is quite adequate for
bibclean's needs, and can be expressed in fewer than 140 lines.  In
addition, it provides special handling of TeX control sequences and
braces that would be rather awkward to express in conventional
regular-expression syntax.

If the symbol TEST is defined at compile time, a main program will be
included that can be used for testing patterns supplied from stdin.


==============
PATTERN SYNTAX
==============

The string values to be pattern-matched are tab-free single-line
values delimited by quotation marks.

The patterns are represented by the following special markers:

	a	exactly one letter
	A	one or more letters
	d	exactly one digit
	D	one or more digits
	r	exactly one roman numeral
	R	one or more roman numerals (i.e. a roman number)
	w	exactly one word (one or more letters and digits)
	W	one or more space-separated words, beginning and ending
		with a word
	X	one or more special-separated words, beginning and ending
		with a word
	.	one special character (see SPECIAL_CHARS defined below)
	:	one or more special characters
	<space>	one or more spaces
	\x	exactly one x (x is an character)
	x	exactly the character x (x is anything but aAdDrRwW.:<space>\)

Special characters are a subset of punctuation characters that are
typically used in values.

Note the \<space> represents a single literal space, \\ a single
literal backslash, \a the letter a, \A the letter A, \d the letter d,
\D the letter D, and so on.  Remember to double all backslashes in C
strings: \a must be entered as \\a, and "and" as "\\an\\d".

Each pattern is matched against the entire string and must match
successfully for a YES return from match_pattern().  Consequently,
there is no need for an analogue of ^ and $ in full regular
expressions.  Neither is there provision for matching on arbitrary
sets of characters.  Instead, fixed sets of characters are provided
(conventional regular-expression equivalents are shown in
parentheses):

	digits ([0-9]),
	alphanumerics ([A-Za-z0-9]),
	space ([ \t\f\r\n\v]), and
	special ([][" !#()*+,-./:;?~])

In addition, TeX control sequences of the form
\<one-special-character> or \<letter-sequence> in the string are
ignored in the match, together with any following whitespace.

Braces are also ignored, but not whitespace following them.

Thus "{TR\slash A87}" matches the patterns "AD" and "W", and
"{TR A\slash 87}" matches the patterns "A AD" and "A W".

[29-Jan-1993]
***********************************************************************/

#include <config.h>
#include "xstdbool.h"
#include "xstdlib.h"
#include "xstring.h"
#include "xctype.h"
#include "yesorno.h"
#include "match.h"			/* must come AFTER yesorno.h */

RCSID("$Id: match.c,v 1.10 2017/06/09 14:46:20 beebe Exp beebe $")

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#define RETURN_MATCH_FAILURE(expect) return (match_failure(expect,org_s,s,org_pattern,pattern))

#ifdef MIN
#undef MIN
#endif /* MIN */

#define	MIN(a,b)	((a) < (b) ? (a) : (b))

#define SPECIAL_CHARS	" !#()*+,-./:;?[]~"

#define is_special(c)	IN_SET(SPECIAL_CHARS,(c))

YESorNO	debug_match_failures = NO; 	/* YES: debug value pattern match failures */
static const char	*next_s ARGS((const char *s_));
bool			is_roman ARGS((int c_));
static YESorNO		match_failure ARGS((const char *expect,
					    const char *org_s, const char *s,
					    const char *org_pattern,
					    const char *pattern));
static void		match_warning ARGS((const char *name,
					    const char *org_s, const char *s));
extern void		warning ARGS((const char *msg_));


#if defined(HAVE_STDC)
YESorNO
match_pattern(const char *s, const char *pattern)
#else /* K&R style */
YESorNO
match_pattern(s,pattern)
const char *s;
const char *pattern;
#endif
{
    const char *org_s;
    const char *org_pattern;

    org_s = s;
    org_pattern = pattern;

    s = next_s(s-1);
    for ( ; (*pattern != '\0'); ++pattern)
    {
	switch(*pattern)
	{
	case 'a':			/* single letter */
	    if (!Isalpha((int)*s))
		RETURN_MATCH_FAILURE("single letter");
	    s = next_s(s);
	    break;

	case 'A':			/* one or more letters */
	    if (!Isalpha((int)*s))
		RETURN_MATCH_FAILURE("one or more letters");
	    while (Isalpha((int)*s))
		s = next_s(s);
	    break;

	case 'd':
	    if (!Isdigit((int)*s))	/* single digit */
		RETURN_MATCH_FAILURE("single digit");
	    s = next_s(s);
	    break;

	case 'D':			/* one or more digits */
	    if (!Isdigit((int)*s))
		RETURN_MATCH_FAILURE("one or more digits");
	    while (Isdigit((int)*s))
		s = next_s(s);
	    break;

	case 'r':			/* single roman numeral */
	    if (!is_roman((int)*s))
		RETURN_MATCH_FAILURE("single roman numeral");
	    s = next_s(s);
	    break;

	case 'R':			/* one or more roman numerals */
	    if (!is_roman((int)*s))
		RETURN_MATCH_FAILURE("one or more roman numerals");
	    while (is_roman((int)*s))
		s = next_s(s);
	    break;

	case 'w':			/* one word (letters and digits) */
	    if (!Isalnum((int)*s))
		RETURN_MATCH_FAILURE("one word (letters and digits)");
	    while (Isalnum((int)*s))
		s = next_s(s);
	    break;

	case 'W':			/* one or more space-separated words */
	    if (!Isalnum((int)*s))
		RETURN_MATCH_FAILURE("one or more space-separated words");
	    while (Isalnum((int)*s))		/* parse first word */
		s = next_s(s);
	    for (;;)
	    {
		if (!Isspace((int)*s))
		    break;
		while (Isspace((int)*s))	/* parse separators */
		    s = next_s(s);
		while (Isalnum((int)*s))	/* parse another word */
		    s = next_s(s);
	    }
	    break;

	case 'X':		/* one or more special-separated words */
	    if (!Isalnum((int)*s))
		RETURN_MATCH_FAILURE("one or more special-separated words");
	    while (Isalnum((int)*s))		/* parse first word */
		s = next_s(s);
	    for (;;)
	    {
		if (!is_special(*s))
		    break;
		while (is_special(*s))	/* parse separators */
		    s = next_s(s);
		while (Isalnum((int)*s))	/* parse another word */
		    s = next_s(s);
	    }
	    break;

	case ' ':			/* one or more whitespace characters */
	    if (!Isspace((int)*s))
		RETURN_MATCH_FAILURE("one or more whitespace characters");
	    while (Isspace((int)*s))
		s = next_s(s);
	    break;

	case '.':			/* exactly one special character */
	    if (!is_special(*s))
		RETURN_MATCH_FAILURE("exactly one special character");
	    s = next_s(s);		/* [07-Mar-1999] bug fix: missing before bibclean 2.12 */
	    break;

	case ':':			/* one or more special characters */
	    if (!is_special(*s))
		RETURN_MATCH_FAILURE("one or more special characters");
	    while (is_special(*s))
		s = next_s(s);
	    break;

	case '\\':			/* literal next character */
	    pattern++;
	    /* fall through to exact match test */
	    /*@fallthrough@*/ /*FALLTHROUGH*/

	default:			/* anything else: exact match */
	    if (*pattern != *s)
		RETURN_MATCH_FAILURE("anything else: exact match");
	    s = next_s(s);
	}				/* end switch */
    }					/* end for (; ;) */
    if (*s == '\0')
	return (YES);
    else
	RETURN_MATCH_FAILURE("end of string");
}


#if defined(HAVE_STDC)
static YESorNO
match_failure(const char *expect, const char *org_s, const char *s,
	      const char *org_pattern, const char *pattern)
#else /* K&R style */
static YESorNO
match_failure(expect,org_s,s,org_pattern,pattern)
const char *expect;
const char *org_s;
const char *s;
const char *org_pattern;
const char *pattern;
#endif
{	/* maybe print a warning if -debug-match-failures was specified; always return NO */
    if (debug_match_failures == YES)
    {
#define FORMAT "Pattern does not match %.50s"
	char msg[sizeof(FORMAT) + 50 + 1];

	(void)sprintf(msg,FORMAT,expect);
#undef FORMAT

#ifndef TEST
	warning(msg);
#endif
	match_warning("String ", org_s, s);
	match_warning("Pattern", org_pattern, pattern);
    }
    return (NO);
}


#if defined(HAVE_STDC)
static void
match_warning(const char *name, const char *org_s, const char *s)
#else /* K&R style */
static void
match_warning(name,org_s,s)
const char *name;
const char *org_s;
const char *s;
#endif
{
    char	*msg;

#define DOTS		"..."
#define FORMAT		"%s so far: [%s%.*s], remaining: [%.*s%s]"
#define MAX_FRAG	((size_t)25)	/* longest string fragment allowed */

    msg = (char*)malloc(strlen(FORMAT) + strlen(name) +
			MIN((size_t)(s - org_s),MAX_FRAG) +
			strlen(DOTS) + MIN(strlen(s),MAX_FRAG) +
			strlen(DOTS) + 1);
    if (msg != (char*)NULL)
    {
	const char	*dots1;
	const char	*dots2;

	if ((size_t)(s - org_s) > MAX_FRAG)
	{
	    org_s = s - MAX_FRAG;
	    dots1 = DOTS;
	}
	else
	{
	    dots1 = "";
	}
	if (strlen(s) > MAX_FRAG)
	    dots2 = DOTS;
	else
	    dots2 = "";
	(void)sprintf(msg,FORMAT, name, dots1, (int)(s - org_s), org_s,
		      (int)MIN(strlen(s),MAX_FRAG), s, dots2);
#ifdef TEST
	(void)fprintf(stderr,"%s\n", msg);
#else
	warning(msg);
#endif
	FREE(msg);
    }
#undef FORMAT
}


#if defined(HAVE_STDC)
static const char *
next_s(const char *s)
#else /* K&R style */
static const char *
next_s(s)
const char *s;
#endif
{
    /* find next position in s, ignoring braces and ignoring TeX control
       sequences and any space that follows them */
    for (++s; (*s != '\0'); )
    {
	switch (*s)
	{
	case '\\':			/* TeX control sequence */
	    ++s;			/* look at next character */
	    if (Isalpha((int)*s))		/* \<one-or-more-letters> */
	    {
		while (Isalpha((int)*s))
		    ++s;
	    }
	    else			/* \<non-letter> */
		++s;
	    while (Isspace((int)*s))		/* advance over trailing whitespace */
	        ++s;			/* since TeX does too */
	    break;

	case '{':
	case '}':
	    ++s;
	    break;

	case BIBTEX_HIDDEN_DELIMITER:	/* ignore delimited inline comment */
	    for (++s; (*s != '\0'); ++s)
	    {
		if (*s == BIBTEX_HIDDEN_DELIMITER)
		{
		    ++s;
		    break;
		}
	    }
	    break;

	default:
	    return (s);
	}				/* end switch */
    }					/* end for */
    return (s);
}

#ifdef TEST
#define MAXLINE 256

#define NO_WARNING	(const char *)NULL

MATCH_PATTERN month_patterns[] =
{
    {"aaa",			"oct"},
    {"aaa # \" D\"",		"oct # \" 10\""},
    {"aaa # \" D--D\"",		"oct # \" 20--24\""},
    {"\"D \" # aaa",		"\"10 \" # oct"},
    {"\"D--D \" # aaa",		"\"10--24 \" # oct"},
    {"aaa # \"\" # aaa",	"jul # \"\\emdash \" # aug"},
    {"aaa # \"--\" # aaa",	"jul # \"--\" # aug"},
    {"aaa # \" -- \" # aaa",	"jul # \" -- \" # aug"},
    {"aaa # \"/\" # aaa",	"jul # \"/\" # aug"},
    {"aaa # \" A \" # aaa",	"jul # \" and \" # aug"},
    {(const char*)NULL,		NO_WARNING},
};

MATCH_PATTERN number_patterns[] =
{
    {"\"A AD\"",		"PN LPS5001"},
    {"\"A D(D)\"",		"RJ 34(49)"},
    {"\"A D\"",			"XNSS 288811"},
    {"\"A D\\.D\"",		"Version 3.20"},
    {"\"A-A-D-D\"",		"UMIAC-TR-89-11"},
    {"\"A-A-D\"",		"CS-TR-2189"},
    {"\"A-A-D\\.D\"",		"CS-TR-21.7"},
    {"\"A-AD-D\"",		"TN-K\\slash 27-70"},
    {"\"A-D D\"",		"PB-251 845"},
    {"\"A-D-D\"",		"ANL-30-74"},
    {"\"A-D\"",			"TR-2189"},
    {"\"AD-D-D\"",		"GG24-3611-00"},
    {"\"AD-D\"",		"SP43-29"},
    {"\"AD\"",			"LPS0064"},
    {"\"A\\#D-D\"",		"TR\\#89-24 ????"},
    {"\"D  D\"",		"23 \\& 24"},
    {"\"D \\an\\d D\"",		"11 and 12"},
    {"\"D+D\"",			"3+4"},
    {"\"D-D\"",			"23-27"},
    {"\"D/D\"",			"23/27"},
    {"\"DA\"",			"23A"},
    {"\"D\"",			"23"},
    {"\"D\\.D\"",		"3.4"},
    {"\"W-W W\"",		"AERE-R 12329"},
    {"\"W-W-WW-W\"",		"OSU-CISRC-4\\slash 87-TR9"},
    {"\"W\"",			"Computer Science Report 100"},
    {"\"X\"",			"TR/AB/3-43.7-3/AB"},
    {(const char*)NULL,		NO_WARNING},
};

MATCH_PATTERN pages_patterns[] =
{
    {"\"D\"",			"23"},
    {"\"aD\"",			"L23"},
    {"\"D--D\"",		"23--27"},
    {"\"aD--aD\"",		"L23--L27"},
    {"\"D, D\"",		"23, 27"},
    {"\"aD, aD\"",		"L23, L27"},
    {"\"D, D, D\"",		"23, 27, 45"},
    {"\"aD, aD, aD\"",		"L23, L27, L45"},
    {"\"D, D, D, D\"",		"23, 27, 45, 98"},
    {"\"aD, aD, aD, aD\"",	"L23, L27, L45, L98"},
    {"\"R + D\"",		"viii + 445"},
    {"\"R + D, w D w\"",	"viii + 445, with 30 illustrations"},
    {"\"D, w D w\"",		"239, with 27 illustrations"},
    {"\"D--D, D--D\"",		"23--27, 29--32"},
    {"\"D--D, D--D, D--D\"",	"23--27, 29--32, 35--37"},
    {"\"aD--aD, aD--aD\"",	"L23--L27, L29--L32"},
    {"\"aD--aD, aD--aD, aD--aD\"", "L23--L27, L29--L32, L35--L37"},
    {(const char*)NULL,		NO_WARNING},
};

MATCH_PATTERN volume_patterns[] =
{
    {"\"D\"",			"27"},
    {"\"DA\"",			"27A"},
    {"\"D/D\"",			"27/3"},
    {"\"DA D\"",		"27A 3"},
    {"\"w-D\"",			"SMC-13"},
    {"\"A\"",			"VIII"},
    {"\"D\\.D\"",		"1.2"},
    {"\"D \\an\\d D\"",		"11 and 12"},
    {"\"W\"",			"Special issue A"},
    {(const char*)NULL,		NO_WARNING},
};

MATCH_PATTERN year_patterns[] =
{
    {"\"DDDD\"",		NO_WARNING},
    {"\"DDDD,WDDDD\"",		NO_WARNING},
    {"\"DDDD, DDDD, DDDD\"",	NO_WARNING},
    {"\"18dd, 18dd, 18dd\"",	"1889, 1890, 1891"},
    {"\"18dd, 18dd\"",		"1889, 1890"},
    {"\"18dd--d\"",		"1891--2"},
    {"\"18dd\"",		"1892"},
    {"\"18dda18dd\"",		"{\\noopsort{1885a}}1885"},
    {"\"19dd (19dd)\"",		"1989 (1990)" },
    {"\"19dd, 19dd, 19dd\"",	"1989, 1990, 1991"},
    {"\"19dd, 19dd\"",		"1989, 1990"},
    {"\"19dd--d\"",		"1991--2"},
    {"\"19dd\" # \"--\"",	"\"1989\" # \"\\unskip--\""},
    {"\"19dd\"",		"1992"},
    {"\"19dda19dd\"",		"{\\noopsort{1985a}}1985"},
    {"\"200d--d\"",		"2001--2"},
    {"\"200d\"",		"2009"},
    {(const char*)NULL,		NO_WARNING},
};

static long	line_number;

int		main ARGS((int argc,char* argv[]));
static void	process ARGS((const char *line_, MATCH_PATTERN patterns_[]));

#if defined(HAVE_STDC)
int
main(int argc, char* argv[])
#else /* K&R style */
int
main(argc,argv)
int argc;
char* argv[];
#endif
{
    char line[MAXLINE];

    /* Input lines should look like
       		key = "value",
       where key is one of: month, number, pages, volume, or
       year. Lines with any other key values are ignored. */

    line_number = 0L;
    while (fgets(line,MAXLINE,stdin) != (char*)NULL)
    {
	char *p;

	line_number++;

	p = strchr(line,'\0');
	if (p != (char *)NULL)
	{
	    for (--p; (p >= &line[0]) && (Isspace((int)*p) || (*p == ',')); --p)
		*p = '\0';	/* remove trailing whitespace and commas */
	    for (p = line; Isspace((int)*p); ++p)
       		continue;
	    if (strncmp(p,"month",4) == 0)
		process(line,month_patterns);
	    else if (strncmp(p,"number",6) == 0)
		process(line,number_patterns);
	    else if (strncmp(p,"pages",4) == 0)
		process(line,pages_patterns);
	    else if (strncmp(p,"volume",6) == 0)
		process(line,volume_patterns);
	    else if (strncmp(p,"year",4) == 0)
		process(line,year_patterns);
	    else
       		printf("%%%% %ld [%-24s]: %s\n", line_number, line, "ignored");
	}
    }

    exit (EXIT_SUCCESS);
    return (EXIT_SUCCESS);
}


#if defined(HAVE_STDC)
static void
process(const char *line, MATCH_PATTERN patterns[])
#else /* K&R style */
static void
process(line,patterns)
const char *line;
MATCH_PATTERN patterns[];
#endif
{
    int k;
    const char *p;

    p = strchr(line,'\"');

    if (p != (char *)NULL)
	line = p;

    for (k = 0; patterns[k].pattern != (const char*)NULL; ++k)
    {
	if (match_pattern(line,patterns[k].pattern) == YES)
	{
	    if (patterns[k].message != NO_WARNING)
		printf("%%%% %ld [%-24s]: matches %s\n", line_number, line,
		       patterns[k].message);
	    return;
	}
    }

    printf("?? %ld [%-24s]: illegal value\n", line_number, line);
}
#endif /* TEST */
