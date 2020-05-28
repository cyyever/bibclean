#include <config.h>
#include "xctype.h"
#include "xstat.h"
#include "xstring.h"
#include "xstdlib.h"

RCSID("$Id: chek.c,v 1.10 2017/06/09 14:47:20 beebe Exp beebe $")

#include "yesorno.h"
#include "match.h"			/* must come AFTER yesorno.h */
#include "token.h"
#include "typedefs.h"			/* must come AFTER match.h */

#if defined(HAVE_PATTERNS)
#define PATTERN_MATCHES(string,pattern) (match_pattern(string,pattern) == YES)
#else /* NOT defined(HAVE_PATTERNS) */
#define PATTERN_MATCHES(string,pattern) match_regexp(string,pattern)
#endif /* defined(HAVE_PATTERNS) */

#define	PT_CHAPTER	0		/* index in pattern_names[] */
#define	PT_MONTH	1		/* index in pattern_names[] */
#define	PT_NUMBER	2		/* index in pattern_names[] */
#define	PT_PAGES	3		/* index in pattern_names[] */
#define	PT_VOLUME	4		/* index in pattern_names[] */
#define	PT_YEAR		5		/* index in pattern_names[] */

#if !defined(STD_MAX_TOKEN)
/* This was 1000 in original BibTeX, then changed to 5000 for TeX Live */
/* builds up to 2011, and changed again for 2012--2019 releases to 20000 */
/* #define STD_MAX_TOKEN	((size_t)1000)	*/ /* Standard BibTeX limit */
#define STD_MAX_TOKEN	((size_t)20000)	/* TeX Live 2012--2019 limit */
#endif /* !defined(STD_MAX_TOKEN) */

#define UNKNOWN_CODEN	"??????"
#define MAX_CODEN	(sizeof(UNKNOWN_CODEN)-1)

#define UNKNOWN_ISBN	"??????????"
#define MAX_ISBN	(sizeof(UNKNOWN_ISBN)-1)

#define UNKNOWN_ISBN_13	"?????????????"
#define MAX_ISBN_13	(sizeof(UNKNOWN_ISBN_13)-1)

#define UNKNOWN_ISSN	"????????"
#define MAX_ISSN	(sizeof(UNKNOWN_ISSN)-1)

extern YESorNO	check_values;		/* NO: suppress value checks */
extern char	current_field[];	/* field name */
extern char	current_key[];		/* string value */
extern char	current_value[];	/* string value */
extern NAME_PAIR month_pair[];
extern PATTERN_NAMES pattern_names[];
extern char	shared_string[];
extern FILE	*stdlog;		/* usually stderr */
extern YESorNO	stdlog_on_stdout;	/* NO for separate files */

extern void	error ARGS((const char *msg_));
extern void	ISBN_hyphenate ARGS((/*@out@*/ char *s_, /*@out@*/ char *t_, size_t maxs_));
extern void	ISBN_13_hyphenate ARGS((/*@out@*/ char *s_, /*@out@*/ char *t_, size_t maxs_));
extern void	warning ARGS((const char *msg_));

void		check_chapter ARGS((void));
void		check_DOI ARGS((void));
void		check_CODEN ARGS((void));
void		check_ISBN ARGS((void));
void		check_ISSN ARGS((void));
YESorNO		check_junior ARGS((const char *last_name_));
void		check_key ARGS((void));
void		check_length ARGS((size_t n_));
void		check_month ARGS((void));
void		check_number ARGS((void));
void		check_other ARGS((void));
void		check_pages ARGS((void));
void		check_URL ARGS((void));
void		check_volume ARGS((void));
void		check_year ARGS((void));

static void	bad_CODEN ARGS((char CODEN_[6]));
static void	bad_ISBN ARGS((char ISBN_[11]));
static void	bad_ISBN_13 ARGS((char ISBN_13_[14]));
static void	bad_ISSN ARGS((char ISSN_[9]));
static YESorNO	check_patterns ARGS((PATTERN_TABLE *pt_,const char *value_));
static int	CODEN_character_value ARGS((int c_));
static size_t	copy_element ARGS((char *target_, size_t nt_, const char *source_, size_t ns_));
static void	incomplete_CODEN ARGS((char CODEN_[6]));
static YESorNO	is_CODEN_char ARGS((int c_, size_t n_));
static YESorNO	is_DOI_char ARGS((int c_, size_t n_));
static YESorNO	is_ISBN_char ARGS((int c_, size_t n_));
static YESorNO	is_ISBN_13_char ARGS((int c_, size_t n_));
static YESorNO	is_ISSN_char ARGS((int c_, size_t n_));
static YESorNO	is_URL_char ARGS((int c_, size_t n_));
static void	parse_list ARGS((const char *s,
				 YESorNO (*is_name_char_) ARGS((int c_, size_t n_)),
				 void (*validate_) ARGS((const char *CODEN_, size_t n_))));
static void	parse_element ARGS((/*@out@*/ parse_data *pd_));
static void	parse_separator ARGS((/*@out@*/ parse_data *pd_));
static void	validate_CODEN ARGS((const char *CODEN_, size_t n_));
static void	validate_DOI ARGS((const char *CODEN_, size_t n_));
static void	validate_ISBN ARGS((const char *ISBN_, size_t n_));
static void	validate_ISBN_13 ARGS((const char *ISBN_, size_t n_));
static void	validate_ISSN ARGS((const char *ISSN_, size_t n_));
static void	validate_URL ARGS((const char *CODEN_, size_t n_));
static void	unexpected ARGS((void));

#define elementsof(v)		(sizeof(v) / sizeof(v[0]))

#define ISBN_DIGIT_VALUE(c)	((((int)(c) == (int)'X') || ((int)(c) == (int)'x')) ? 10 : \
					((int)(c) - (int)'0'))
				/* correct only if digits are valid; */
				/* the code below ensures that */

#define ISSN_DIGIT_VALUE(c)	ISBN_DIGIT_VALUE(c)
				/* ISSN digits are just like ISBN digits */

#if defined(HAVE_STDC)
static void
bad_CODEN(char CODEN[7])
#else /* K&R style */
static void
bad_CODEN(CODEN)
char CODEN[7];
#endif
{
    static const char fmt[] =
	"Invalid checksum for CODEN %c%c%c%c%c%c in ``%%f = %%v''";
    char msg[sizeof(fmt)];

#define XCODEN(n)	(int)((CODEN[n] == '\0') ? '?' : CODEN[n])

    (void)sprintf(msg, fmt,
		  XCODEN(1), XCODEN(2), XCODEN(3), XCODEN(4), XCODEN(5), XCODEN(6));
    warning(msg);	/* should be error(), but some journals might have */
			/* invalid CODENs (some books have invalid ISBNs) */
}


#if defined(HAVE_STDC)
static void
bad_ISBN(char ISBN[11])
#else /* K&R style */
static void
bad_ISBN(ISBN)
char ISBN[11];
#endif
{
#define MAXISBN	(13+1)	/* space for correctly hyphenated ISBN, plus NUL */
    static const char fmt[] = "Invalid checksum for ISBN %s in ``%%f = %%v''";
    char msg[sizeof(fmt)+MAXISBN-1-2];
    char s[MAXISBN];
    char t[MAXISBN];
    size_t n;

    (void)strcpy(s,UNKNOWN_ISBN);
    n = strlen(&ISBN[1]);
    (void)memcpy(s,&ISBN[1],(n > sizeof(s)) ? sizeof(s) : n);
    s[10] = '\0';
    ISBN_hyphenate(s,t,sizeof(s));

    (void)sprintf(msg, fmt, s);
    warning(msg);	/* used to be error(), but some books actually have */
			/* invalid ISBNs */
}


#if defined(HAVE_STDC)
static void
bad_ISBN_13(char ISBN_13[13 + 1])
#else /* K&R style */
static void
bad_ISBN_13(ISBN_13)
char ISBN_13[13 + 1];
#endif
{
#define MAXISBN_13	(13 + 3 + 1)	/* space for correctly hyphenated ISBN_13, plus NUL */
    static const char fmt[] = "Invalid checksum for ISBN_13 %s in ``%%f = %%v''";
    char msg[sizeof(fmt) + MAXISBN_13 - 1 - 2];
    char s[MAXISBN_13];
    char t[MAXISBN_13];
    size_t n;

    (void)strcpy(s,UNKNOWN_ISBN_13);
    n = strlen(&ISBN_13[1]);
    (void)memcpy(s,&ISBN_13[1],(n > sizeof(s)) ? sizeof(s) : n);
    s[13] = '\0';
    ISBN_13_hyphenate(s,t,sizeof(s));

    (void)sprintf(msg, fmt, s);
    warning(msg);	/* used to be error(), but some books actually have */
			/* invalid ISBN_13s */
}


#if defined(HAVE_STDC)
static void
bad_ISSN(char ISSN[9])
#else /* K&R style */
static void
bad_ISSN(ISSN)
char ISSN[9];
#endif
{
    static const char fmt[] =
	"Invalid checksum for ISSN %c%c%c%c-%c%c%c%c in ``%%f = %%v''";
    char msg[sizeof(fmt)];

#define XISSN(n)	(int)((ISSN[n] == '\0') ? '?' : ISSN[n])

    (void)sprintf(msg, fmt, XISSN(1), XISSN(2), XISSN(3), XISSN(4),
		  XISSN(5), XISSN(6), XISSN(7), XISSN(8));
    warning(msg);	/* used to be error(), but some journals might have */
			/* invalid ISSNs (some books have invalid ISBNs) */
}


void
check_chapter(VOID)
{
#if defined(HAVE_OLDCODE)
    size_t k;
    size_t n = strlen(current_value) - 1;

    /* match patterns like "23" and "23-1" */
    for (k = 1; k < n; ++k)
    {	/* omit first and last characters -- they are quotation marks */
	if (!(Isdigit(current_value[k]) || (current_value[k] == '-')))
	    break;
    }
    if (k == n)
	return;
#else /* NOT defined(HAVE_OLDCODE) */
    if (check_patterns(pattern_names[PT_CHAPTER].table,current_value) == YES)
	return;
#endif /* defined(HAVE_OLDCODE) */

    unexpected();
}


void
check_CODEN(VOID)
{
    parse_list(current_value, is_CODEN_char, validate_CODEN);
}


void
check_DOI(VOID)
{
    parse_list(current_value, is_DOI_char, validate_DOI);

    if ( IN_SET(current_value, ' ') ||
	 IN_SET(current_value, ',') ||
	 IN_SET(current_value, ';') )
	warning("Unexpected space or list separator in DOI value ``%v''");
}


void
check_inodes(VOID)
{
    struct stat buflog;
    struct stat bufout;

    stdlog_on_stdout = YES;			/* assume the worst initially */

    (void)fstat(fileno(stdlog),&buflog);
    (void)fstat(fileno(stdout),&bufout);

#if OS_UNIX
    stdlog_on_stdout = ((buflog.st_dev == bufout.st_dev) &&
			(buflog.st_ino == bufout.st_ino)) ? YES : NO;
#endif /* OS_UNIX */

#if OS_PCDOS
    /* No inodes, so use other fields instead */
    stdlog_on_stdout = ((buflog.st_dev == bufout.st_dev) &&
			(buflog.st_mode == bufout.st_mode) &&
			(buflog.st_size == bufout.st_size) &&
			(buflog.st_ctime == bufout.st_ctime)) ? YES : NO;
#endif /* OS_PCDOS */

#if OS_VAXVMS
    /* Inode field is 3 separate values */
    stdlog_on_stdout = ((buflog.st_dev == bufout.st_dev) &&
			(buflog.st_ino[0] == bufout.st_ino[0]) &&
			(buflog.st_ino[1] == bufout.st_ino[1]) &&
			(buflog.st_ino[2] == bufout.st_ino[2])) ? YES : NO;
#endif /* OS_VAXVMS */

}


void
check_ISBN(VOID)
{
    char t[MAX_TOKEN_SIZE];

    /* Supply correct hyphenation for all ISBNs */
    ISBN_hyphenate(current_value,t,sizeof(t)/sizeof(t[0]));

    parse_list(current_value, is_ISBN_char, validate_ISBN);
}


void
check_ISBN_13(VOID)
{
    char t[MAX_TOKEN_SIZE];

    /* Supply correct hyphenation for all ISBN-13s */
    ISBN_13_hyphenate(current_value, t, sizeof(t) / sizeof(t[0]));

    parse_list(current_value, is_ISBN_13_char, validate_ISBN_13);
}


void
check_ISSN(VOID)
{
    parse_list(current_value, is_ISSN_char, validate_ISSN);
}


void
check_ISSN_L(VOID)
{
    parse_list(current_value, is_ISSN_char, validate_ISSN);

    if (strlen(current_value) != 11)	/* "1234-5689" */
	warning("Unexpected ISSN-L field length in ``%v''");
}


#if defined(HAVE_STDC)
YESorNO
check_junior(const char *last_name)
#else /* K&R style */
YESorNO
check_junior(last_name)
const char *last_name;
#endif
{				/* return YES: name is Jr.-like, else: NO */
    int b_level;		/* brace level */
    static const char *juniors[] =
    {				/* name parts that parse like "Jr." */
	"Jr",
	"Jr.",
	"Sr",
	"Sr.",
	"SJ",
	"S.J.",
	"S. J.",
	(const char*)NULL,	/* list terminator */
    };
    int k;			/* index into juniors[] */
    int n;			/* index into last_name[] */

    for (n = 0, b_level = 0; last_name[n] != '\0'; ++n)
    {				/* check for "Smith, Jr" and "Smith Jr" and */
	switch (last_name[n])	/* convert to "{Smith, Jr}" and "{Smith Jr}" */
	{
	case '{':
	    b_level++;
	    break;

	case '}':
	    b_level--;
	    break;

	case ',':
	    if (b_level == 0)
		return (YES);
	    break;

	case '\t':
	case ' ':		/* test for Jr.-like name */
	    if (b_level == 0)
	    {
		for (k = 0; juniors[k] != (const char*)NULL; ++k)
		{
		    if (strnicmp(&last_name[n+1],juniors[k],strlen(juniors[k]))
			== 0)
			return (YES);
		}			/* end for (k...) */
		if (strcspn(&last_name[n+1],"IVX") == 0)
		    return (YES); /* probably small upper-case Roman number */
	    }
	    break;

	default:
	    break;
	}				/* end switch (last_name[n]) */
    }					/* end for (n = 0,...) */
    return (NO);
}


void
check_key(VOID)
{
    int k;				/* index into pattern_names[] */

    for (k = 0; pattern_names[k].name != (const char*)NULL; ++k)
    {
	if (stricmp(pattern_names[k].name,current_key) == 0)
	{				/* then found the required table */
	    if (check_patterns(pattern_names[k].table,current_key) == NO)
		warning("Unexpected citation key ``%k''");
	    return;
	}
    }
}


#if defined(HAVE_STDC)
void
check_length(size_t n)
#else /* K&R style */
void
check_length(n)
size_t n;
#endif
{
    if ((check_values == YES) && (n >= STD_MAX_TOKEN))
	warning("String length exceeds standard BibTeX limit for ``%f'' entry");
}


void
check_month(VOID)
{
    size_t n;

    n = strlen(current_value);

    if (n == 3)			/* check for match against standard abbrevs */
    {
	int m;			/* month index */

	for (m = 0; month_pair[m].old_name != (const char*)NULL; ++m)
	{
	    if (stricmp(month_pair[m].new_name,current_value) == 0)
		return;
	}
    }

    /* Hand coding for the remaining patterns is too ugly to contemplate,
       so we only provide the checking when real pattern matching is
       available. */

#if !defined(HAVE_OLDCODE)
    if (check_patterns(pattern_names[PT_MONTH].table,current_value) == YES)
	return;
#endif /* !defined(HAVE_OLDCODE) */

    unexpected();
}


void
check_number(VOID)
{
#if defined(HAVE_OLDCODE)
    size_t k;
    size_t n = strlen(current_value) - 1;

    /* We expect the value string to match the regexp "[0-9a-zA-Z---,/ ()]+
    to handle values like "UMIACS-TR-89-11, CS-TR-2189, SRC-TR-89-13",
    "RJ 3847 (43914)", "{STAN-CS-89-1256}", "UMIACS-TR-89-3.1, CS-TR-2177.1",
    "TR\#89-24", "23", "23-27", and "3+4". */

    for (k = 1; k < n; ++k)
    {	/* omit first and last characters -- they are quotation marks */
	if (!(     Isalnum(current_value[k])
		|| Isspace(current_value[k]) || (current_value[k] == '-')
		|| (current_value[k] == '+') || (current_value[k] == ',')
		|| (current_value[k] == '.') || (current_value[k] == '/')
		|| (current_value[k] == '#') || (current_value[k] == '\\')
		|| (current_value[k] == '(') || (current_value[k] == ')')
		|| (current_value[k] == '{') || (current_value[k] == '}') ))
	    break;
    }
    if (k == n)
	return;
#else /* NOT defined(HAVE_OLDCODE) */
    if (check_patterns(pattern_names[PT_NUMBER].table,current_value) == YES)
	return;
#endif /* defined(HAVE_OLDCODE) */

    unexpected();
}


void
check_other(VOID)
{
    int k;				/* index into pattern_names[] */

    for (k = 0; pattern_names[k].name != (const char*)NULL; ++k)
    {
	if (stricmp(pattern_names[k].name,current_field) == 0)
	{				/* then found the required table */
	    if (check_patterns(pattern_names[k].table,current_value) == NO)
		unexpected();
	    return;
	}
    }
}


void
check_pages(VOID)
{
    /* Need to handle "B721--B729" as well as "721--729"; some
       physics journals use an initial letter in page number. */

#if defined(HAVE_OLDCODE)
    int number = 1;
    size_t k;
    size_t n = strlen(current_value) - 1;

    /* We expect the value string to match the regexps [0-9]+ or
       [0-9]+--[0-9]+ */
    for (k = 1; k < n; ++k)
    {	/* omit first and last characters -- they are quotation marks */
	switch (current_value[k])
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    if (number > 2)
	    {
		warning("More than 2 page numbers in ``%f = %v''");
		return;
	    }
	    break;

	case '-':
	    number++;
	    if (current_value[k+1] != '-')	/* expect -- */
	    {
		warning(
		    "Use en-dash, --, to separate page numbers in ``%f = %v''");
		return;
	    }
	    ++k;
	    if (current_value[k+1] == '-')	/* should not have --- */
	    {
		warning(
		    "Use en-dash, --, to separate page numbers in ``%f = %v''");
		return;
	    }
	    break;

	case ',':
	    number++;
	    break;

	default:
	    unexpected();
	    return;
	}
    }
#else /* NOT defined(HAVE_OLDCODE) */
    if (check_patterns(pattern_names[PT_PAGES].table,current_value) == YES)
	return;
#endif /* defined(HAVE_OLDCODE) */

    unexpected();
}


#if (defined(HAVE_PATTERNS) || defined(HAVE_REGEXP) || defined(HAVE_RECOMP))

#if defined(HAVE_STDC)
static YESorNO
check_patterns(PATTERN_TABLE* pt,const char *value)
#else /* K&R style */
static YESorNO
check_patterns(pt,value)
PATTERN_TABLE* pt;
const char *value;
#endif
{
    /* Return YES if current_value[] matches a pattern, or there are no
       patterns, and NO if there is a match failure.  Any message
       associated with a successfully-matched pattern is printed before
       returning. */

    int k;

    for (k = 0; k < pt->current_size; ++k)
    {
	if (PATTERN_MATCHES(value,pt->patterns[k].pattern))
	{
	    if (pt->patterns[k].message != (const char*)NULL)
	    {
		if (pt->patterns[k].message[0] == '?') /* special error flag */
		    error(pt->patterns[k].message + 1);
		else		/* just normal warning */
		    warning(pt->patterns[k].message);
	    }
	    return (YES);
	}
    }
    return ((pt->current_size == 0) ? YES : NO);
}
#endif /* (defined(HAVE_PATTERNS) || defined(HAVE_REGEXP) ||
	   defined(HAVE_RECOMP)) */


void
check_URL(VOID)
{
    parse_list(current_value, is_URL_char, validate_URL);
}


void
check_volume(VOID)
{
#if defined(HAVE_OLDCODE)
    size_t k;
    size_t n = strlen(current_value) - 1;

    /* Match patterns like "27", "27A", "27/3", "27A 3", "SMC-13", "VIII",
       "B", "{IX}", "1.2", "Special issue A", and  "11 and 12".  However,
       NEVER match pattern like "11(5)", since that is probably an erroneous
       incorporation of issue number into the volume value. */

    for (k = 1; k < n; ++k)
    {	/* omit first and last characters -- they are quotation marks */
	if (!(     Isalnum(current_value[k])
		|| (current_value[k] == '-')
		|| (current_value[k] == '/')
		|| (current_value[k] == '.')
		|| Isspace(current_value[k])
		|| (current_value[k] == '{')
		|| (current_value[k] == '}') ))
	{
	    unexpected();
	    return;
	}
    }
#else /* NOT defined(HAVE_OLDCODE) */
    if (check_patterns(pattern_names[PT_VOLUME].table,current_value) == YES)
	return;
#endif /* defined(HAVE_OLDCODE) */

    unexpected();
}


void
check_year(VOID)
{
    char *p;
    char *q;
    long year;

#if defined(HAVE_OLDCODE)
    size_t k;
    size_t n;

    /* We expect the value string to match the regexp [0-9]+ */
    for (k = 1, n = strlen(current_value) - 1; k < n; ++k)
    {	/* omit first and last characters -- they are quotation marks */
	if (!Isdigit(current_value[k]))
	{
	    warning("Non-digit found in field value of ``%f = %v''");
	    return;
	}
    }
#else /* NOT defined(HAVE_PATTERNS) */
    if (check_patterns(pattern_names[PT_YEAR].table,current_value) == YES)
	return;
    unexpected();
#endif /* defined(HAVE_PATTERNS) */

    for (p = current_value; (*p != '\0') ; ) /* now validate all digit strings */
    {
	if (Isdigit(*p))	/* then have digit string */
	{			/* now make sure year is `reasonable' */
	    year = strtol(p,&q,10);
	    if ((year < 1800L) || (year > 2099L))
		warning("Suspicious year in ``%f = %v''");
	    p = q;
	}
	else			/* ignore other characters */
	    p++;
    }
}


#if defined(HAVE_STDC)
static int
CODEN_character_value(int c)
#else /* K&R style */
static int
CODEN_character_value(c)
int c;
#endif
{
    if (((int)'a' <= c) && (c <= (int)'z'))
	return ((c - (int)'a' + 1));
    else if (((int)'A' <= c) && (c <= (int)'Z'))
	return ((c - (int)'A' + 1));
    else if (((int)'1' <= c) && (c <= (int)'9'))
	return ((c - (int)'1' + 27));
    else if (c == (int)'0')
	return (36);
    else
	return (-1);
}


#if defined(HAVE_STDC)
static size_t
copy_element(char *target, size_t nt, const char *source, size_t ns)
#else /* K&R style */
static size_t
copy_element(target, nt, source, ns)
char *target;
size_t nt;
const char *source;
size_t ns;
#endif
{    /* Copy source[] into target[], excluding spaces and hyphens, and add a */
     /* trailing NUL.  Return the number of characters left in source[], */
     /* after ignoring trailing spaces and hyphens. */
    size_t ks;
    size_t kt;

    for (ks = 0, kt = 0; (ks < ns) && (kt < (nt - 1)); ++ks)
    {
	if (!((source[ks] == '-') || Isspace(source[ks])))
	    target[kt++] = source[ks];
    }
    target[kt] = '\0';

    for ( ; (source[ks] == '-') || Isspace(source[ks]); ++ks)
	continue;		 /* skip trailing space and hyphens */

    return (size_t)(ns - ks);
}


#if defined(HAVE_STDC)
static void
incomplete_CODEN(char CODEN[7])
#else /* K&R style */
static void
incomplete_CODEN(CODEN)
char CODEN[7];
#endif
{
    static const char fmt[] =
	"Incomplete CODEN %c%c%c%c%c should be %c%c%c%c%c%c in ``%%f = %%v''";
    char msg[sizeof(fmt)];

    (void)sprintf(msg, fmt, CODEN[1], CODEN[2], CODEN[3], CODEN[4], CODEN[5],
		  CODEN[1], CODEN[2], CODEN[3], CODEN[4], CODEN[5], CODEN[6]);
    warning(msg);	/* should be error(), but some journals might have */
			/* invalid CODENs (some books have invalid ISBNs) */
}


#if defined(HAVE_STDC)
static YESorNO
is_CODEN_char(int c, size_t n)
#else /* K&R style */
static YESorNO
is_CODEN_char(c,n)
int c;
size_t n;
#endif
{
    static size_t n_significant = 0;
		/* number of significant chars already seen in current CODEN */

    /* CODENs match [A-Z]-*[A-Z]-*[A-Z]-*[A-Z]-*[A-Z]-*[A-Z0-9], but we
       also allow lower-case letters. */

    if (n == 0)				/* start new CODEN */
	n_significant = 0;

    /* embedded hyphens are accepted, but are not significant */
    if ((n_significant > 0) && (c == (int)'-'))
	return (YES);
    else if ((n_significant < 5) && Isalpha(c))
    {
	n_significant++;
	return (YES);
    }
    else if ((n_significant >= 5) && Isalnum(c)) /* sixth char can be a digit */
    {
	n_significant++;
	return (YES);
    }

    return (NO);
}


#if defined(HAVE_STDC)
static YESorNO
is_DOI_char(int c, size_t n)
#else /* K&R style */
static YESorNO
is_DOI_char(c,n)
int c;
size_t n;
#endif
{
    return (Isprint(c) ? YES : NO);    /* DOIs match any printable string */
}


#if defined(HAVE_STDC)
static YESorNO
is_ISBN_char(int c, size_t n)
#else /* K&R style */
static YESorNO
is_ISBN_char(c,n)
int c;
size_t n;
#endif
{
    static size_t n_significant = 0;
		/* number of significant chars already seen in current CODEN */

    /* ISBNs match
	[0-9][- ]*[0-9][- ]*[0-9][- ]*[0-9][- ]*[0-9][- ]*
	[0-9][- ]*[0-9][- ]*[0-9][- ]*[0-9][- ]*[0-9xX]
    */

    if (n == 0)				/* start new ISBN */
	n_significant = 0;

    /* embedded hyphens and space are accepted, but are not significant */
    if ((n_significant > 0) && ((c == (int)'-') || Isspace(c)))
	return (YES);
    else if ((n_significant < 9) && Isdigit(c))
    {
	n_significant++;
	return (YES);
    }
    else if ((n_significant >= 9) && (Isdigit(c) || (c == (int)'X') || (c == (int)'x')))
    {					/* tenth character may be [0-9xX] */
	n_significant++;
	return (YES);
    }

    return (NO);
}


#if defined(HAVE_STDC)
static YESorNO
is_ISBN_13_char(int c, size_t n)
#else /* K&R style */
static YESorNO
is_ISBN_13_char(c,n)
int c;
size_t n;
#endif
{
    return (is_ISBN_char(c, n));
}


#if defined(HAVE_STDC)
static YESorNO
is_ISSN_char(int c, size_t n)
#else /* K&R style */
static YESorNO
is_ISSN_char(c,n)
int c;
size_t n;
#endif
{
    static size_t n_significant = 0;
		/* number of significant chars already seen in current CODEN */

    /* ISSNs match
	[0-9][- ]*[0-9][- ]*[0-9][- ]*[0-9][- ]*
	[0-9][- ]*[0-9][- ]*[0-9][- ]*[0-9xX]
    */

    if (n == 0)				/* start new ISSN */
	n_significant = 0;

    /* embedded hyphens and space are accepted, but are not significant */
    if ((n_significant > 0) && ((c == (int)'-') || Isspace(c)))
	return (YES);
    else if ((n_significant < 7) && Isdigit(c))
    {
	n_significant++;
	return (YES);
    }
    else if ((n_significant >= 7) && (Isdigit(c) || (c == (int)'X') || (c == (int)'x')))
    {					/* eighth character may be [0-9xX] */
	n_significant++;
	return (YES);
    }

    return (NO);
}


#if defined(HAVE_STDC)
static YESorNO
is_URL_char(int c, size_t n)
#else /* K&R style */
static YESorNO
is_URL_char(c,n)
int c;
size_t n;
#endif
{
    return (Isprint(c) ? YES : NO);    /* URLs match any printable string */
}


#if defined(HAVE_STDC)
static void
parse_list(const char *s, YESorNO (*is_name_char) ARGS((int c, size_t n)),
	   void (*validate) ARGS((const char *s, size_t n)))
#else /* K&R style */
static void
parse_list(s, is_name_char, validate)
const char *s;
YESorNO (*is_name_char) ARGS((int c, size_t n));
void (*validate) ARGS((const char *s, size_t n));
#endif
{
    parse_data pd;

    /*******************************************************************
       Parse a list of CODEN, ISBN, or ISSN elements, according to the
       grammar:

	       LIST : NAME
		      | NAME SEPARATOR LIST

	       SEPARATOR : [not-a-token-char]+ | (nested balanced parentheses)

	       NAME : SEPARATOR* NAME'

	       NAME' : [token-char]+

       This simple, and permissive, grammar accepts any strings that
       contain sequences of zero or more CODEN, ISBN, or ISSN
       elements, separated by one or more of characters which are not
       themselves legal element characters.  The first element in the
       list may be preceded by any number of non-element characters.
       Comments are supported as arbitrary strings inside balanced
       parentheses, allowing lists like

		"0-387-97621-4 (invalid ISBN checksum), 3-540-97621-3"

		"0020-0190 (1982--1990), 0733-8716 (1991--)"

		"0-8493-0190-4 (set), 0-8493-0191-2 (v. 1),
		 0-8493-0192-0 (v. 2), 0-8493-0193-9 (v. 3)"

       The distinction between NAME' and SEPARATOR characters is made
       by the argument function, (*is_name_char)(), and the validation
       of the elements is done by the argument function (*validate)().

       This generality makes it possible for the same code to be
       reused for at least CODEN, ISBN, and ISSN values, and possibly
       others in future versions of this program.

       Tokens are not copied from the list, so no additional dynamic
       string storage is required.
    *******************************************************************/

    pd.s = s;
    pd.is_name_char = is_name_char;

    for (;;)
    {
	parse_separator(&pd);		/* may produce a zero-length token */
	parse_element(&pd);
	if (pd.token_length == 0)	/* no more tokens in list */
	    return;
	(*validate)(pd.token, pd.token_length);
    }
}


#if defined(HAVE_STDC)
static void
parse_element(/*@out@*/ parse_data *pd)
#else /* K&R style */
static void
parse_element(pd)
/*@out@*/ parse_data *pd;
#endif
{
    size_t n;

    for (n = 0, pd->token = pd->s; (*pd->s != '\0') && ((*pd->is_name_char)((int)*pd->s,n) == YES);
	 n++, pd->s++)
	continue;

    pd->token_length = (size_t)(pd->s - pd->token);
}


#if defined(HAVE_STDC)
static void
parse_separator(/*@out@*/ parse_data *pd)
#else /* K&R style */
static void
parse_separator(pd)
/*@out@*/ parse_data *pd;
#endif
{
    size_t n;
    int paren_level;			/* parenthesis level */

    pd->token = pd->s;

    for (n = 0, paren_level = 0;
	 ((*pd->s != '\0') && (((*pd->is_name_char)((int)*pd->s,n) == NO) || (paren_level > 0)));
	n++, pd->s++)
    {
	if (*pd->s == '(')
	    paren_level++;
	else if (*pd->s == ')')
	{
	    paren_level--;
	    if (paren_level == 0)
	        n = 0;
	}
    }

    pd->token_length = (size_t)(pd->s - pd->token);
    if (paren_level != 0)
	warning("Non-zero parenthesis level in ``%f = %v''");
}


static void
unexpected(VOID)
{
    warning("Unexpected value in ``%f = %v''");
}


#if defined(HAVE_STDC)
static void
validate_CODEN(const char *the_CODEN, size_t n)
#else
static void
validate_CODEN(the_CODEN, n)
const char *the_CODEN;
size_t n;
#endif
{
    int checksum;
    char CODEN[1 + MAX_CODEN + 1];	/* saved CODEN for error messages */
					/* (use slots 1..6 instead of 0..5) */
    size_t k;				/* index into CODEN[] */
    size_t nleft;

#define CODEN_CHECK_CHARACTER(n)	"9ABCDEFGHIJKLMNOPQRSTUVWXYZ2345678"[n]

    /*******************************************************************
       CODEN values are 6-character strings from the set [A-Z0-9],
       with a check digit stored in the 6th position given by

	   (11*N1 + 7*N2 + 5*N3 + 3*N4 + 1*N5) mod 34 == X

       where the Nk are 1..26 for A..Z, and 27..36 for 1..9..0.
       However, the checksum X (in 0..33) is represented by the
       corresponding character in the different 34-character range
       [9A-Z2-8], which excludes digits 0 and 1 to avoid confusion
       with letters O and I.

       In library catalogs, the 6th CODEN digit is often omitted, so
       when we find it missing in a CODEN value string, we print a
       warning to tell the user what it should be.  However, we
       intentionally do NOT insert it into the bibclean output,
       because the value string may be corrupted, instead of just
       truncated.

       The largest possible sum above is 11*36 + 7*36 + 5*36 + 3*36 +
       1*36 = 36*(11 + 7 + 5 + 3 + 1) = 36*27 = 972, corresponding to
       the CODEN value 00000T, since 972 mod 34 = 20, which maps to
       the letter T.  In reality, the limit is lower than this,
       because the initial CODEN character is always alphabetic; the
       largest usable CODEN would then be Z0000, which has a checksum
       of 11*26 + 7*36 + 5*36 + 3*36 + 1*36 = 36*(11 + 7 + 5 + 3 + 1)
       - 10*11 = 862.  Even 16-bit (short) integers are adequate for
       this computation.

       Old CODEN values may be stored with a hyphen between the 4th
       and 5th characters, e.g. "JACS-A" and "JACS-AT", as well as
       just "JACSA" and "JACSAT".  Unlike ISBN and ISSN values, spaces
       are NOT used inside CODEN values.
    *******************************************************************/

    (void)strcpy(&CODEN[1], UNKNOWN_CODEN);
    nleft = copy_element(&CODEN[1], sizeof(CODEN)-1, the_CODEN, n);

    for (checksum = 0, k = 1; CODEN[k] != '\0'; ++k)
    {
	if (k < MAX_CODEN)
	{
	    static int multiplier[] = { 0, 11, 7, 5, 3, 1 };

	    checksum += CODEN_character_value((int)CODEN[k]) * multiplier[k];
	}
	else if (k == MAX_CODEN)
	{
	    if (CODEN_CHECK_CHARACTER(checksum % 34) != CODEN[k])
		bad_CODEN(CODEN);
	}
    }				/* end for (loop over CODEN[]) */

    if (strlen(&CODEN[1]) == (MAX_CODEN - 1))
    {		/* check digit omitted, so tell the user what it should be */
	CODEN[MAX_CODEN] = CODEN_CHECK_CHARACTER(checksum % 34);
	incomplete_CODEN(CODEN);
    }
    else if ((strlen(&CODEN[1]) != MAX_CODEN) || (nleft > 0))
	bad_CODEN(CODEN);
}


#if defined(HAVE_STDC)
static void
validate_DOI(const char *the_DOI, size_t n)
#else
static void
validate_DOI(the_DOI, n)
const char *the_DOI;
size_t n;
#endif
{
    /*
    ** Typical input:
    **
    **     the_DOI = "\"http://dx.doi.org/10.1000/a.b.c\""
    **
    ** Notice that the delimiting quotation marks are part of the
    ** string value, so we must skip the first character in matching
    ** against members of the list of valid prefixes.
    */

    int k, match_count;
    static const char *doi_prefix_list[] =
    {	/* the DOI organization introduced new prefixes in 2016 */
	"http://doi.org/",
	"http://dx.doi.org/",	/* original single-prefix style (2000--2016) */
	"https://doi.org/",
	"https://dx.doi.org/",
#if defined(DOI_RAW_VALID)
	"10."
#endif
    };

    for (k = 0, match_count = 0; k < elementsof(doi_prefix_list); ++k)
    {
	if (strncmp(&the_DOI[1], doi_prefix_list[k], strlen(doi_prefix_list[k])) == 0)
	    match_count++;
    }

    if (match_count == 0)
	warning("Unexpected prefix in DOI value ``%v''");
}


#if defined(HAVE_STDC)
static void
validate_ISBN(const char *the_ISBN, size_t n)
#else
static void
validate_ISBN(the_ISBN, n)
const char *the_ISBN;
size_t n;
#endif
{
    int checksum;
    char ISBN[1 + MAX_ISBN + 1];	/* saved ISBN for error messages */
					/* (use slots 1..10 instead of 0..9) */
    size_t k;				/* index into ISBN[] */
    size_t nleft;

    /*******************************************************************
       ISBN numbers are 10-character values from the set [0-9Xx], with
       a checksum given by

		(sum(k=1:9) digit(k) * k) mod 11 == digit(10)

       where digits have their normal value, X (or x) as a digit has
       value 10, and spaces and hyphens are ignored.  The sum is
       bounded from above by 10*(1 + 2 + ... + 9) = 450, so even short
       (16-bit) integers are sufficient for the accumulation.

       ISBN digits are grouped into four parts separated by space or
       hyphen: countrygroupnumber-publishernumber-booknumber-checkdigit.
    *******************************************************************/

    (void)strcpy(&ISBN[1],UNKNOWN_ISBN);
    nleft = copy_element(&ISBN[1], sizeof(ISBN)-1, the_ISBN, n);

    for (checksum = 0, k = 1; ISBN[k] != '\0'; ++k)
    {
	if (k < MAX_ISBN)
	    checksum += ISBN_DIGIT_VALUE(ISBN[k]) * k;
	else if (k == MAX_ISBN)
	{
	    if ((checksum % 11) != ISBN_DIGIT_VALUE(ISBN[k]))
		bad_ISBN(ISBN);
	}
    } 					/* end for (loop over ISBN[]) */

    if ((strlen(&ISBN[1]) != MAX_ISBN) || (nleft > 0))
	bad_ISBN(ISBN);
}


#if defined(HAVE_STDC)
static void
validate_ISBN_13(const char *the_ISBN_13, size_t n)
#else
static void
validate_ISBN_13(the_ISBN_13, n)
const char *the_ISBN_13;
size_t n;
#endif
{
    int checksum;
    char ISBN_13[1 + MAX_ISBN_13 + 1];	/* saved ISBN_13 for error messages */
					/* (use slots 1..13 instead of 0..12) */
    size_t k;				/* index into ISBN_13[] */
    size_t nleft;

    /*******************************************************************
       ISBN_13 numbers are 13-character values from the set [0-9Xx], with
       a final checksum digit given by

		rem = (sum(k=1:12) digit(k) * weight(k)) mod 10
		weight(k) = if (k odd) then 1 else 3
		digit(13) = if (rem == 0) then 0 else (10 - rem)

       where digits have their normal value, X (or x) as a digit has
       value 10, and spaces and hyphens are ignored.  The sum is
       bounded from above by 3*(9 + 9 + ... + 9) = 324, so even
       short (16-bit) integers are sufficient for the accumulation.

       ISBN_13 digits are grouped into five parts separated by space
       or hyphen:

	   978-countrygroupnumber-publishernumber-booknumber-checkdigit.

       The initial prefix changes to 979 when the 978 group is
       exhausted.
    *******************************************************************/

    (void)strcpy(&ISBN_13[1],UNKNOWN_ISBN_13);
    nleft = copy_element(&ISBN_13[1], sizeof(ISBN_13)-1, the_ISBN_13, n);

    for (checksum = 0, k = 1; ISBN_13[k] != '\0'; ++k)
    {
	size_t weight;

	weight = (k & 1) ? 1 : 3;

	if (k < MAX_ISBN_13)
	    checksum += ISBN_DIGIT_VALUE(ISBN_13[k]) * weight ;
	else if (k == MAX_ISBN_13)
	{
	    size_t digit_13, rem;

	    rem = checksum % 10;
	    digit_13 = (rem == 0) ? 0 : (10 - rem);

	    if (digit_13 != ISBN_DIGIT_VALUE(ISBN_13[k]))
		bad_ISBN_13(ISBN_13);
	}
    } 					/* end for (loop over ISBN_13[]) */

    if ((strlen(&ISBN_13[1]) != MAX_ISBN_13) || (nleft > 0))
	bad_ISBN_13(ISBN_13);
}


#if defined(HAVE_STDC)
static void
validate_ISSN(const char *the_ISSN, size_t n)
#else
static void
validate_ISSN(the_ISSN, n)
const char *the_ISSN;
size_t n;
#endif
{
    long checksum;
    char ISSN[1 + MAX_ISSN + 1];	/* saved ISSN for error messages */
					/* (use slots 1..8 instead of 0..7) */
    size_t k;				/* index into ISSN[] */
    size_t nleft;

    /*******************************************************************
       ISSN numbers are 8-character values from the set [0-9Xx], with
       a checksum given by

		(sum(k=1:7) digit(k) * (k+2)) mod 11 == digit(8)

       where digits have their normal value, X (or x) as a digit has
       value 10, and spaces and hyphens are ignored.  The sum is
       bounded from above by 10*(3 + 4 + ... + 9) = 420, so even short
       (16-bit) integers are sufficient for the accumulation.

       ISSN digits are grouped into two 4-digit parts separated by
       space or hyphen.
    *******************************************************************/

    (void)strcpy(&ISSN[1],UNKNOWN_ISSN);
    nleft = copy_element(&ISSN[1], sizeof(ISSN)-1, the_ISSN, n);

    for (checksum = 0L, k = 1; (ISSN[k] != '\0'); ++k)
    {
	if (k < MAX_ISSN)
	    checksum += (long)(ISSN_DIGIT_VALUE(ISSN[k]) * (k + 2));
	else if (k == MAX_ISSN)
	{
	    if ((checksum % 11L) != ISSN_DIGIT_VALUE(ISSN[k]))
		bad_ISSN(ISSN);
	}
    } 					/* end for (loop over ISSN[]) */

    if ((strlen(&ISSN[1]) != MAX_ISSN) || (nleft > 0))
	bad_ISSN(ISSN);
}

#if defined(HAVE_STDC)
static void
validate_URL(const char *the_URL, size_t n)
#else
static void
validate_URL(the_URL, n)
const char *the_URL;
size_t n;
#endif
{
    char *p;

    p = stristr(the_URL, "://");

    if (p == (char *)NULL)
	warning("Expected protocol://... in URL value ``%v%''");
    else
    {
	if ( ((p - the_URL) >= 3) && (strncmp(&p[-3], "ftp", 3) == 0) )
	    /* NO-OP */ ;
	else if ( ((p - the_URL) >= 4) && (strncmp(&p[-4], "http", 4) == 0) )
	    /* NO-OP */ ;
	else if ( ((p - the_URL) >= 5) && (strncmp(&p[-5], "https", 5) == 0) )
	    /* NO-OP */ ;
	else
	    warning("Unexpected protocol://... in URL value ``%v'': normally ftp://... or http://... or https://...");

	if (stristr(the_URL, "doi.org/") != (char *)NULL)
	    warning("Unexpected DOI in URL value ``%v'': move to separate DOI = \"...\" key/value in this entry");
	else if ( (stristr(the_URL, ".com/10.") != (char *)NULL) ||
		  (stristr(the_URL, ".edu/10.") != (char *)NULL) ||
		  (stristr(the_URL, ".gov/10.") != (char *)NULL) ||
		  (stristr(the_URL, ".mil/10.") != (char *)NULL) ||
		  (stristr(the_URL, ".net/10.") != (char *)NULL) ||
		  (stristr(the_URL, ".org/10.") != (char *)NULL) )
	    warning("Possible DOI in URL value ``%v'': if so, move to separate DOI = \"...\" key/value in this entry");
    }
}
