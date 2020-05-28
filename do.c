#include <config.h>
#include "xctype.h"
#include "xlimits.h"
#include "xstdbool.h"
#include "xstdlib.h"
#include "xstring.h"
#include "xunistd.h"

RCSID("$Id: do.c,v 1.12 2014/04/03 18:02:49 beebe Exp beebe $")

#include "ch.h"
#include "delete.h"
#include "keybrd.h"
#include "pattern.h"
#include "token.h"
#include "toklst.h"
#include "yesorno.h"
#include "match.h"			/* must come AFTER yesorno.h */
#include "typedefs.h"			/* must come AFTER match.h */

#define	APPEND_CHAR(s,n,c)	(s[n] = (char)c, s[n+1] = (char)'\0')
					/* append c and NUL to s[] */

#if !defined(BIBCLEAN_SUFFIX)
#define BIBCLEAN_EXT		"BIBCLEANEXT" /* environment variable */
#endif

#define EMPTY_STRING(s)	(s[0] = (char)'\0', s)
				/* for return (EMPTY_STRING(foo))*/

#define GETDEFAULT(envname,default) \
	((getenv(envname) != (char *)NULL) ? getenv(envname) : default)

#if !defined(INITFILE_EXT)
#define INITFILE_EXT	".ini"	/* file extension for initialization files */
#endif

#define is_fieldvalueseparator(c)	(((int)(c) == (int)'=') || ((int)(c) == (int)':'))

#define KEEP_PREAMBLE_SPACES()	((in_preamble == YES) && \
				 (keep_preamble_spaces == YES))

#define KEEP_STRING_SPACES()	((in_string == YES) && \
				 (keep_string_spaces == YES))

#if defined(MAX)
#undef MAX
#endif

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))

#if !defined(MAX_LINE)
#define MAX_LINE	10240	/* maximum line length in initialization file */
#endif /* !defined(MAX_LINE) */

#define NOOP				/* dummy statement */

#if OS_PCDOS
#define OPEN_MODE_R	"rb"		/* bibliography files are read in binary mode */
					/* so that we can handle Ctl-Z properly */
#else
#define OPEN_MODE_R	"r"
#endif

#define SKIP_SPACE(p)	while (Isspace((unsigned char)*p)) ++p
#define TABLE_CHUNKS	25	/* how many table entries to allocate at once */
#define TOLOWER(c)      (Isupper((unsigned char)(c)) ? \
			tolower(((unsigned char)(c))) : (int)(((unsigned char)(c))))

NAME_PAIR month_pair[] =
{
	{"January",		"jan"},
	{"February",		"feb"},
	{"March",		"mar"},
	{"April",		"apr"},
	{"May",			"may"},
	{"June",		"jun"},
	{"July",		"jul"},
	{"August",		"aug"},
	{"September",		"sep"},
	{"October",		"oct"},
	{"November",		"nov"},
	{"December",		"dec"},

	{"Jan.",		"jan"},
	{"Feb.",		"feb"},
	{"Mar.",		"mar"},
	{"Apr.",		"apr"},
	{"Jun.",		"jun"},
	{"Jul.",		"jul"},
	{"Aug.",		"aug"},
	{"Sep.",		"sep"},
	{"Sept.",		"sep"},
	{"Oct.",		"oct"},
	{"Nov.",		"nov"},
	{"Dec.",		"dec"},

	{"Jan",			"jan"},
	{"Feb",			"feb"},
	{"Mar",			"mar"},
	{"Apr",			"apr"},
	{"Jun",			"jun"},
	{"Jul",			"jul"},
	{"Aug",			"aug"},
	{"Sep",			"sep"},
	{"Sept",		"sep"},
	{"Oct",			"oct"},
	{"Nov",			"nov"},
	{"Dec",			"dec"},

	{(const char*)NULL,	(const char*)NULL},
};

#if !defined(MAX_KEYWORD)
#define MAX_KEYWORD	200		/* about 8 times the default size */
#endif

static NAME_PAIR field_pair[MAX_KEYWORD] =
{                                   /* field name case change table */
    { "ansi-standard-number",		"ANSI-standard-number" },
    { "book-doi",			"book-DOI" }, /* Digital Object Identifier */
    { "book-url",			"book-URL" }, /*  WWW: Uniform Resource Locator */
    { "coden",				"CODEN" },
					/* 5- and 6-letter standard journal identifier */
    { "doi",				"DOI" }, /* Digital Object Identifier */
    { "ieee-standard-number",		"IEEE-standard-number" },
    { "isbn",				"ISBN" },
					/* old-style (10-digit) International Standard Book Number */
    { "journal-url",			"journal-URL" }, /*  WWW: Uniform Resource Locator */
    { "isbn-13",			"ISBN-13" },
					/* new-style (13-digit)  International Standard Book Number */
    { "iso-standard-number",		"ISO-standard-number" },
    { "issn",				"ISSN" },
					/* International Standard Serials Number */
    { "issn-l",				"ISSN-L" },
					/* linking International Standard Serials Number */
    { "lccn",				"LCCN" },
					/* Library of Congress catalog number */
    { "mrclass",			"MRclass" }, /* Math Reviews class */
    { "mrnumber",			"MRnumber" }, /* Math Reviews number */
    { "mrreviewer",			"MRreviewer" }, /* Math Reviews reviewer */
    { "uri",				"URI" },
					/* WWW: Uniform Resource Identifier */
    { "url",				"URL" },
					/* WWW: Uniform Resource Locator */
    { "urn",				"URN" },
					/* WWW: Uniform Resource Name */
    { "xxansi-standard-number",		"xxANSI-standard-number" },
    { "xxcoden",			"xxCODEN" },
    { "xxdoi",				"xxDOI" },
    { "xxieee-standard-number",		"xxIEEE-standard-number" },
    { "xxisbn",				"xxISBN" },
    { "xxiso-standard-number",		"xxISO-standard-number" },
    { "xxissn",				"xxISSN" },
    { "xxmrclass",			"xxMRclass" },
    { "xxmrnumber",			"xxMRnumber" },
    { "xxmrreviewer",			"xxMRreviewer" },
    { "xxuri",				"xxURI" },
    { "xxurl",				"xxURL" },
    { "xxurn",				"xxURN" },
    { "zmclass",			"ZMclass" },
    { "zmnumber",			"ZMnumber" },
    { "zmreviewer",			"ZMreviewer" },
    { (const char*)NULL,	    (const char*)NULL },
};

extern YESorNO	align_equals;		/* NO: left-adjust equals */
extern int	at_level;		/* @ nesting level */
extern int	brace_level;		/* curly brace nesting level */
extern YESorNO	brace_math;		/* NO: leave mixed-case math text untouched */
extern YESorNO	check_values;		/* NO: suppress value checks */
extern int	close_char;		/* BibTeX entry closing; may */
					/* be right paren or brace */
extern char	current_entry_name[];	/* entry name */
extern char	current_field[];	/* field name */
extern char	current_key[];		/* string value */
extern char	current_value[];	/* string value */
extern YESorNO	delete_empty_values; 	/* YES: delete empty values */
extern YESorNO	discard_next_comma; 	/* YES: deleting field/value */
extern YESorNO	eofile;			/* set to YES at end-of-file */
extern int	field_indentation;
extern FILE	*fpin;			/* input file pointer */
extern YESorNO	in_preamble; 		/* YES: in @Preamble{...} */
extern YESorNO	in_string;		/* YES: in @String{...} */
extern YESorNO	in_value;		/* YES: in value string */
extern YESorNO	is_parbreak;		/* get_next_non_blank() sets */
extern YESorNO	keep_linebreaks;	/* YES: keep linebreaks in values */
extern YESorNO	keep_parbreaks;		/* YES: keep parbreaks in values */
extern YESorNO	keep_preamble_spaces;	/* YES: keep spaces in @Preamble{} */
extern YESorNO	keep_spaces;		/* YES: keep spaces in values */
extern YESorNO	keep_string_spaces;	/* YES: keep spaces in @String{} */
extern int	non_white_chars;	/* used to test for legal @ */
extern YESorNO	parbreaks;		/* NO: parbreaks forbidden */
					/* in strings and entries */
extern PATTERN_NAMES pattern_names[];
extern YESorNO	prettyprint;		/* NO: do lexical analysis */
extern char	*program_name;		/* set to argv[0] */
extern YESorNO	print_patterns;		/* YES: print value patterns */
extern YESorNO	read_initialization_files;/* -[no-]read-init-files sets */
extern YESorNO	remove_OPT_prefixes; 	/* YES: remove OPT prefix */
extern YESorNO	rflag;			/* YES: resynchronizing */
extern YESorNO	Scribe;			/* Scribe format input */
extern char	shared_string[];
extern FILE	*stdlog;		/* usually stderr */
extern IO_PAIR	token_start;		/* used for # line output */
extern IO_PAIR	the_entry;		/* used in error messages */
extern IO_PAIR	the_file;		/* used in error messages */
extern IO_PAIR	the_value;		/* used in error messages */
extern int	value_indentation;
extern YESorNO	wrapping;		/* NO: verbatim output */

void		do_files ARGS((int argc_, char *argv_[]));
void		do_keyword_file ARGS((/*@null@*/ const char *pathlist, /*@null@*/ const char *name));
void		do_other ARGS((void));
void		do_print_keyword_table ARGS((void));

extern YESorNO	apply_function ARGS((const char *option_,
		    OPTION_FUNCTION_ENTRY table_[]));
extern void	check_length ARGS((size_t n_));
extern void	check_key ARGS((void));
extern void	check_chapter ARGS((void));
extern void	check_CODEN ARGS((void));
extern void	check_DOI ARGS((void));
extern void	check_ISBN ARGS((void));
extern void	check_ISBN_13 ARGS((void));
extern void	check_ISSN ARGS((void));
extern void	check_ISSN_L ARGS((void));
extern void	check_month ARGS((void));
extern void	check_number ARGS((void));
extern void	check_pages ARGS((void));
extern void	check_other ARGS((void));
extern void	check_URL ARGS((void));
extern void	check_volume ARGS((void));
extern void	check_year ARGS((void));
extern void	do_args ARGS((int argc_, char *argv_[]));
extern void	do_initfile ARGS((/*@null@*/ const char *pathlist_,/*@null@*/ const char *name_));
extern void	error ARGS((const char *msg_));
/*@noreturn@*/	extern void	fatal ARGS((const char *msg_));
/*@null@*/	extern char	*findfile ARGS((/*@null@*/ const char *pathlist_, /*@null@*/ const char *name_));
extern void	finish ARGS((int status));
extern void	fix_math_spacing ARGS((void));
extern void	fix_month ARGS((void));
extern void	fix_namelist ARGS((void));
extern void	fix_pages ARGS((void));
extern void	fix_title ARGS((void));
extern void	free_pattern_table_entries ARGS((PATTERN_TABLE *pt_));
extern int	get_char ARGS((void));
/*@null@*/extern char	*get_line ARGS((FILE *fp_));
extern int	get_linebreak ARGS((void));
extern int	get_next_non_blank ARGS((void));
extern char	*initialization_file_name;
extern bool	is_idchar ARGS((int c_));
extern bool	is_optionprefix ARGS((int c));
extern void	out_at ARGS((void));
extern void	out_c ARGS((int c_));
extern void	out_flush ARGS((void));
extern void	out_newline ARGS((void));
extern void	out_s ARGS((const char *s_));
extern void	out_spaces ARGS((int n_));
extern void	out_string ARGS((token_t type_, const char *token_));
extern void	out_token ARGS((token_t type_, const char *token_));
extern void	out_with_error ARGS((const char *s_,const char *msg_));
extern void	out_with_parbreak_error ARGS((char *s_));
extern void	put_back ARGS((int c_));
extern FILE	*tfopen ARGS((const char *filename_, const char *mode_));
extern void	warning ARGS((const char *msg_));

YESorNO		German_style = NO;		/* YES: " inside braced string
						value obeys german.sty style */

static long	space_count = 0L; /* count of spaces in do_optional_space()  */
static char	Scribe_close_delims[] = "}])>'\"`";
static char	Scribe_open_delims[]  = "{[(<'\"`";

static void	add_keyword ARGS((const char *the_old, const char *the_new));
static void	add_one_keyword ARGS((const char *the_old, const char *the_new,
				      size_t where));
static void	add_one_pattern ARGS((PATTERN_TABLE *pt_,
		    const char *fieldname_, const char *pattern_,
		    /*@null@*/ const char *msg_));
static void	add_pattern ARGS((const char *fieldname_, const char *pattern_,
		    /*@null@*/ const char *msg_));
static void	append_value ARGS((const char *s_));
static void	do_at ARGS((void));
static void	do_BibTeX_entry ARGS((void));
static void	do_BibTeX_value ARGS((void));
static void	do_BibTeX_value_1 ARGS((void));
static void	do_BibTeX_value_2 ARGS((void));
static void	do_close_brace ARGS((void));
static void	do_comma ARGS((void));
static void	do_entry_name ARGS((void));
static void	do_equals ARGS((void));
static void	do_escapes ARGS((char *s_));
static void	do_field ARGS((void));
static YESorNO	do_field_value_pair ARGS((void));
static void	do_fileinit ARGS((const char *bibfilename_));
static void	do_group ARGS((void));
static void	do_key_name ARGS((void));
static void	do_new_pattern ARGS((char *s_));
static void	do_newline ARGS((void));
static void	do_one_file ARGS((FILE *fp_));
static void	do_open_brace ARGS((void));
static void	do_optional_inline_comment ARGS((void));
static void	do_optional_space ARGS((void));
static void	do_preamble ARGS((void));
static void	do_preamble_2 ARGS((void));
static void	do_Scribe_block_comment ARGS((void));
static void	do_Scribe_close_delimiter ARGS((void));
static void	do_Scribe_comment ARGS((void));
static void	do_Scribe_entry ARGS((void));
static void	do_Scribe_open_delimiter ARGS((void));
static void	do_Scribe_separator ARGS((void));
static void	do_Scribe_value ARGS((void));
static void	do_single_arg ARGS((char *s_));
static void	do_space ARGS((void));
static void	do_string ARGS((void));
static void	do_string_2 ARGS((void));
static void	enlarge_table ARGS((PATTERN_TABLE *table_));
static void	flush_inter_entry_space ARGS((void));
static char	*get_braced_string ARGS((void));
static char	*get_digit_string ARGS((void));
static char	*get_identifier_string ARGS((void));
static char	*get_inline_comment ARGS((void));
static char	*get_optional_space ARGS((void));
static int	get_parbreak ARGS((void));
static char	*get_quoted_string ARGS((void));
static char	*get_Scribe_delimited_string ARGS((void));
static char	*get_Scribe_identifier_string ARGS((void));
static char	*get_Scribe_string ARGS((void));
static char	*get_simple_string ARGS((void));
/*@null@*/ static char	*get_token ARGS((/*@null@*/ char *s_, char **nextp_,
		    const char *terminators_));
static void	new_entry ARGS((void));
static void	new_io_pair ARGS((IO_PAIR *pair_));
static void	new_position ARGS((POSITION *position_));
static void	out_close_brace ARGS((void));
static void	out_comma ARGS((void));
static void	out_complex_value ARGS((void));
static void	out_equals ARGS((void));
static void	out_field ARGS((void));
static void	out_open_brace ARGS((void));
static void	out_other ARGS((const char *s_));
static void	out_value ARGS((void));
static void	prt_pattern ARGS((const char *fieldname_, /*@null@*/ const char *pattern_,
		    /*@null@*/ const char *msg_));
static void	put_back_string ARGS((const char *s_));
static void	trim_value ARGS((void));


#if defined(HAVE_STDC)
static void
add_keyword(const char *the_old, const char *the_new)
#else /* K&R style */
static void
add_keyword(the_old, the_new)
const char *the_old;
const char *the_new;
#endif
{
    /* Search the keyword_range[] table circularly from the last search
       position for the next non-empty slot matching the_old, and
       install the new pair (the_old, the_new) there.  Otherwise,
       add the pair at the end, if enough space remains. */

    static int error_count = 0;
    size_t k;
    static size_t start = (size_t) 0;

    /* Silently ignore invalid keyword pairs */

    if (the_old == (const char *)NULL)
	return;
    else if (the_new == (const char *)NULL)
	return;

    if (the_old[0] == '-')
	start = 0;	/* because deletions must always find the first match */

    for (k = start; (k < MAX_KEYWORD) && (field_pair[k].old_name != (const char *)NULL); ++k)
    {
	if (STREQUAL(field_pair[k].old_name, the_old))
	{
	    add_one_keyword(the_old, the_new, k);
	    start = k;
	    return;
	}
	else if ((the_old[0] == '-') && STREQUAL(field_pair[k].old_name, the_old + 1))
	{	/* then `delete' this entry by changing its begin prefix to start with a hyphen */
	    field_pair[k].old_name = Strdup(the_old);
	    start = k;
	    return;
	}
    }

    /* If we fell through, then restart the search in the beginning of the table */

    for (k = 0; (k < start) && (field_pair[k].old_name != (const char *)NULL); ++k)
    {
	if (STREQUAL(field_pair[k].old_name, the_old))
	{
	    add_one_keyword(the_old, the_new, k);
	    start = k;
	    return;
	}
	else if ((the_old[0] == '-') && STREQUAL(field_pair[k].old_name, the_old + 1))
	{	/* then `delete' this entry by changing its begin prefix to start with a hyphen */
	    field_pair[k].old_name = Strdup(the_old);
	    start = k;
	    return;
	}
    }

    /* If we fell through, then add the new entry at the first deleted
       entry, or after the last used entry */
    for (k = 0; ((k < MAX_KEYWORD) &&
	  (field_pair[k].old_name != (const char *)NULL) &&
	  (field_pair[k].old_name[0] != '\0'));
	 ++k)
	continue;

    if (k < (MAX_KEYWORD - 1))	/* then have space to store this new entry */
    {
	start = k;
	add_one_keyword(the_old, the_new, k);
    }
    else if (++error_count == 1)	/* no more than one error message */
	(void)fprintf(stdlog,
		      "More than %lu keywords fills internal table\n",
		      (unsigned long)MAX_KEYWORD);
}


#if defined(HAVE_STDC)
static void
add_one_keyword(const char *the_old, const char *the_new, size_t where)
#else /* K&R style */
static void
add_one_keyword(the_old, the_new, where)
const char *the_old;
const char *the_new;
size_t where;
#endif
{	/* add an entry at slot where, without bounds checking */

    if (field_pair[where].old_name != (const char *)NULL)
	FREE(field_pair[where].old_name);

    if (field_pair[where].new_name != (const char *)NULL)
	FREE(field_pair[where].new_name);

    field_pair[where].old_name = (the_old == (const char *)NULL) ? the_old :
	Strdup(the_old);
    field_pair[where].new_name = (the_new == (const char *)NULL) ? the_new :
	Strdup(the_new);
}


#if defined(HAVE_STDC)
static void
add_one_pattern(PATTERN_TABLE *pt, const char *fieldname, const char *pattern,
    /*@null@*/ const char *message)
#else /* K&R style */
static void
add_one_pattern(pt,fieldname,pattern,message)
PATTERN_TABLE *pt;
const char *fieldname;
const char *pattern;
/*@null@*/ const char *message;
#endif
{
    if (STREQUAL(pattern,""))		/* then clear pattern table */
	free_pattern_table_entries(pt);
    else				/* otherwise add new pattern */
    {
	int m;				/* index into pt->patterns[] */

	if (pt->current_size == pt->maximum_size) /* then table full */
	    enlarge_table(pt);

	for (m = 0; m < pt->current_size; ++m)
	{
	    /* Make sure this is not a duplicate; if it is, and its message */
	    /* is the same, then we just ignore the request.  Duplicates */
	    /* are possible when the user and system search paths overlap. */
	    if (STREQUAL(pattern,pt->patterns[m].pattern))
	    {			    /* duplicate pattern found */
		if (((pt->patterns[m].message) != (char*)NULL)
		    && (message != (char*)NULL) &&
		    STREQUAL(message,pt->patterns[m].message))
		    return;	    /* messages duplicate too */

		if (pt->patterns[m].message != (const char *)NULL)
		    FREE(pt->patterns[m].message);

		pt->patterns[m].message =
		    (message == (char*)NULL) ? message :
				(const char*)Strdup(message);
				    /* replace message string */
		prt_pattern(fieldname,pattern,message);

		return;
	    }
	}

	/* We have a new and distinct pattern and message, so save them */
	pt->patterns[pt->current_size].pattern = Strdup(pattern);
	pt->patterns[pt->current_size++].message =
	    (message == (char*)NULL) ? message : (const char*)Strdup(message);
    }

    prt_pattern(fieldname,pattern,message);
}


#if defined(HAVE_STDC)
static void
add_pattern(const char *fieldname, const char *pattern, /*@null@*/ const char *message)
#else /* K&R style */
static void
add_pattern(fieldname,pattern,message)
const char *fieldname;
const char *pattern;
/*@null@*/ const char *message;
#endif
{
    int k;				/* index into pattern_names[] */

    for (k = 0; pattern_names[k].name != (const char*)NULL; ++k)
    {					/* find the correct pattern table */
	if (stricmp(pattern_names[k].name,fieldname) == 0)
	{				/* then found the required table */
	    add_one_pattern(pattern_names[k].table,fieldname,pattern,message);
	    return;
	}
    }

    /* If we get here, then the pattern name is not in the built-in list,
       so create a new entry in pattern_names[] if space remains */

    if (k >= (int)((MAX_PATTERN_NAMES - 1)))
    {					/* too many pattern types */
	(void)fprintf(stdlog,
	    "%s Out of memory for pattern name [%s] -- pattern ignored\n",
	    WARNING_PREFIX, fieldname);
    }
    else
    {					/* sufficient table space remains */
	pattern_names[k].name = Strdup(fieldname); /* add new table entry */
	pattern_names[k].table = (PATTERN_TABLE*)malloc(sizeof(PATTERN_TABLE));
	if (pattern_names[k].table == (PATTERN_TABLE*)NULL)
	    fatal("Out of memory for pattern tables");
	pattern_names[k].table->patterns = (MATCH_PATTERN*)NULL;
	pattern_names[k].table->current_size = 0;
	pattern_names[k].table->maximum_size = 0;

	add_one_pattern(pattern_names[k].table,fieldname,pattern,message);

	pattern_names[k+1].name = (char*)NULL; /* mark new end of table */
	pattern_names[k+1].table = (PATTERN_TABLE*)NULL;
    }
}


#if defined(HAVE_STDC)
static void
append_value(const char *s)
#else /* K&R style */
static void
append_value(s)
const char *s;
#endif
{
    size_t n_cv = strlen(current_value);
    size_t n_s = strlen(s);

    if ((n_cv + n_s) < MAX_TOKEN)
	(void)strcpy(&current_value[n_cv],s);
    else			/* string too long; concatenate into parts */
    {
	out_s(current_value);
	(void)strcpy(current_value,s);
	out_with_error(" # ","Value too long for field ``%f''");
    }
}


static void
do_at(VOID)			/* parse @name{...} */
{
    int c;

    token_start = the_file;	/* remember location of token start */

    c = get_char();
    the_entry = the_file;
    if ((non_white_chars == 1) && (c == (int)'@'))
    {
	at_level++;
	out_at();
	if (brace_level != 0)
	{
	    error(
	"@ begins line, but brace level is not zero after entry ``@%e{%k,''");
	    brace_level = 0;
	}
    }
    else if (c != EOF)
    {
	out_c(c);
	out_with_error("", "Expected @name{...} after entry ``@%e{%k,''");
    }
}


static void
do_BibTeX_entry(VOID)
{
    /*************************************************************
     Parse a BibTeX entry, one of:
       @entry-name{key,field=value,field=value,...,}
       @string{name=value}
       @preamble{...}
    *************************************************************/

    new_entry();

    do_at();
    if ((rflag == YES) || (eofile == YES)) return;

    do_optional_space();

    if (prettyprint == YES)
	out_c(DELETE_WHITESPACE);	/* discard any space that we found */

    do_entry_name();
    if (rflag == YES) return;

    if (STREQUAL(current_entry_name,"Include"))
	do_group();
    else if (STREQUAL(current_entry_name,"Preamble"))
	do_preamble();
    else if (STREQUAL(current_entry_name,"String"))
	do_string();
    else			/* expect @name{key, field = value, ... } */
    {
	do_optional_space();

	if (prettyprint == YES)
	    out_c(DELETE_WHITESPACE);	/* discard any space that we found */

	do_open_brace();
	if (rflag == YES) return;

	do_optional_space();

	do_key_name();
	if (rflag == YES) return;

	do_optional_space();

	do_comma();
	if (rflag == YES) return;

	do_optional_space();

	while (do_field_value_pair() == YES)
	{
	    do_optional_space();
	    do_comma();		/* this supplies any missing optional comma */
	    if ((rflag == YES) || (eofile == YES)) return;
	    do_optional_space();
	}
	if (rflag == YES) return;

	do_optional_space();

	do_close_brace();
    }
    flush_inter_entry_space();
}


/***********************************************************************
BibTeX field values can take several forms, as illustrated by this
simple BNF grammar:

BibTeX-value-string:
	simple-string |
	simple-string # BibTeX-value-string

simple-string:
	"quoted string" |
	{braced-string} |
	digit-sequence	|
	alpha-sequence	|
***********************************************************************/

static void
do_BibTeX_value(VOID)		/* process BibTeX value string */
{
    if (prettyprint == YES)
	do_BibTeX_value_1();
    else
	do_BibTeX_value_2();
}


static void
do_BibTeX_value_1(VOID)		/* process BibTeX value string */
{				/* for prettyprinted output */
    /* In order to support string value checking, we need to collect
       complete values, including intervening inline comments, which
       we bracket by magic delimiters so they can be ignored during
       pattern matching, and restored on output.   Space between
       values is simply discarded. */

    int c;

    the_value = the_file;
    current_value[0] = '\0';
    append_value(get_simple_string());

    do_optional_inline_comment();

    while ((c = get_char(), c) == (int)'#')
    {
	if (KEEP_PREAMBLE_SPACES())
	    append_value("#");
	else if (KEEP_STRING_SPACES())
	    append_value("#");
	else
	    append_value(" # ");
	do_optional_inline_comment();
	append_value(get_simple_string());
	do_optional_inline_comment();
    }
    put_back(c);
    out_value();
}


static void
do_BibTeX_value_2(VOID)		/* process BibTeX value string */
{				/* for lexical analysis output */
    int c;

    the_value = the_file;

    (void)strcpy(current_value,get_simple_string());
    out_string((current_value[0] == '"') ? TOKEN_VALUE : TOKEN_ABBREV,
	       current_value);

    do_optional_space();

    while ((c = get_char(), c) == (int)'#')
    {
	out_string(TOKEN_SPACE," ");
	out_string(TOKEN_SHARP,"#");
	out_string(TOKEN_SPACE," ");

	do_optional_space();

	(void)strcpy(current_value,get_simple_string());
	out_string((current_value[0] == '"') ? TOKEN_VALUE : TOKEN_ABBREV,
		current_value);

	do_optional_space();
    }
    put_back(c);
}


static void
do_close_brace(VOID)	/* parse level 1 closing brace or parenthesis */
{
    int c;

    c = get_char();
    if (c == EOF)
	return;
    else if (c == close_char)
    {
	if (c == (int)')')
	    brace_level--;	/* get_char() could not do this for us */
	out_close_brace();	/* standardize parenthesis to brace */
	if (brace_level != 0)
	    out_with_error("",
	"Non-zero brace level after @name{...} processed.  Last key = ``%k''");
    }
    else			/* raise error and try to resynchronize */
    {
	out_c(c);
	out_with_error("",
	    "Expected closing brace or parenthesis in entry ``@%e{%k,''");
    }
}


static void
do_comma(VOID)
{
    int c;

    /* Parse a comma, or an optional comma before a closing brace or
       parenthesis;  an omitted legal comma is supplied explicitly.
       A newline is output after the comma so that field = value
       pairs appear on separate lines. */

    the_value = the_file;

    c = get_char();
    if (c == EOF)
	NOOP;
    else if (c == (int)',')
    {
	if (discard_next_comma == NO)
	{
	    out_comma();
	    out_newline();
	}
    }
    else if (c == close_char)
    {			/* supply missing comma for last field = value pair*/
	if (c == (int)')')
	    brace_level--;	/* get_char() could not do this for us */
	if (brace_level == 0)	/* reached end of bibliography entry */
	{
	    if (c == (int)')')
		brace_level++;	/* put_back() could not do this for us */
	    put_back(c);
	    if (discard_next_comma == NO)
	    {
		out_comma();
		out_newline();
	    }
	}
	else			/* no comma, and still in bibliography entry */
	{
	    out_c(c);
	    out_with_error("","Non-zero brace level after @name{...} \
processed.  Last entry = ``@%e{%k,''");
	}
    }
    else			/* raise error and try to resynchronize */
    {
	out_c(c);
	out_with_error("", "Expected comma after last field ``%f''");
    }
    discard_next_comma = NO;
}


static void
do_entry_name(VOID)		/* process BibTeX entry name */
{
    int c;
    size_t k;
    int n;
    static NAME_PAIR entry_pair[] =
    {					/* entry name case change table */
	{ "Deathesis",		"DEAthesis" },
	{ "Inbook",		"InBook" },
	{ "Incollection",	"InCollection" },
	{ "Inproceedings",	"InProceedings" },
	{ "Mastersthesis",	"MastersThesis" },
	{ "Phdthesis",		"PhdThesis" },
	{ "Techreport",		"TechReport" },
    };

    token_start = the_file;	/* remember location of token start */

    for (k = 0; ((c = get_char(), c) != EOF) && is_idchar(c); ++k)
    {				/* store capitalized entry name */
	if ((k == 0) && !Isalpha(c))
	    error("Non-alphabetic character begins an entry name");
	if ((k == 0) && Islower(c))
	    c = toupper(c);
	else if ((k > 0) && Isupper(c))
	    c = tolower(c);
	if ((parbreaks == NO) && (is_parbreak == YES))
	{
	    APPEND_CHAR(current_entry_name,k,c);
	    out_with_parbreak_error(current_entry_name);
	    return;
	}
	if (k >= MAX_TOKEN)
	{
	    APPEND_CHAR(current_entry_name,k,c);
	    out_with_error(current_entry_name, "@entry_name too long");
	    return;

	}
	current_entry_name[k] = (char)c;
    }
    current_entry_name[k] = (char)'\0';
    if (c != EOF)
	put_back(c);

    /* Substitute a few entry names that look better in upper case */
    for (n = 0; n < (int)(sizeof(entry_pair)/sizeof(entry_pair[0])); ++n)
	if (STREQUAL(current_entry_name,entry_pair[n].old_name))
	    (void)strcpy(current_entry_name,entry_pair[n].new_name);

    if (prettyprint == YES)
	out_s(current_entry_name);
    else if (STREQUAL(current_entry_name,"Include"))
	out_token(TOKEN_INCLUDE, current_entry_name);
    else if (STREQUAL(current_entry_name,"Preamble"))
	out_token(TOKEN_PREAMBLE, current_entry_name);
    else if (STREQUAL(current_entry_name,"String"))
	out_token(TOKEN_STRING, current_entry_name);
    else
	out_token(TOKEN_ENTRY, current_entry_name);
    check_length(k);
}


static void
do_equals(VOID)			/* process = in field = value */
{
    int c;

    the_value = the_file;

    token_start = the_file;	/* remember location of token start */

    c = get_char();
    if (c == EOF)
	NOOP;
    else if (c == (int)'=')
	out_equals();
    else
    {
	out_c(c);
	out_with_error("", "Expected \"=\" after field ``%f''");
    }
    out_spaces((int)(value_indentation - the_file.output.column_position));
				/* supply leading indentation */
}


#if defined(HAVE_STDC)
static void
do_escapes(char *s)
#else /* K&R style */
static void
do_escapes(s)
char *s;
#endif
{					/* reduce escape sequences in s[] */
    int base;				/* number base for strtol() */
    char *endptr;			/* pointer returned by strtol() */
    char *p;				/* pointer into output s[] */

    if (s == (char*)NULL)		/* nothing to do if no string */
	return;

    for (p = s ; (*s  != '\0'); ++s)
    {
	if (*s == '\\')			/* have escaped character */
	{
	    base = 8;			/* base is tentatively octal */
	    switch (*++s)
	    {
	    case 'a':	*p++ = (char)CTL('G'); break;
	    case 'b':	*p++ = (char)CTL('H'); break;
	    case 'f':	*p++ = (char)CTL('L'); break;
	    case 'n':	*p++ = (char)CTL('J'); break;
	    case 'r':	*p++ = (char)CTL('M'); break;
	    case 't':	*p++ = (char)CTL('I'); break;
	    case 'v':	*p++ = (char)CTL('K'); break;

	    case '0':
		if (TOLOWER(s[1]) == (int)'x') /* 0x means hexadecimal */
		    base = 16;
		/* FALLTHROUGH */ /*@fallthrough@*/
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
		*p++ = (char)strtol((const char*)s,&endptr,base);
		s = endptr - 1;	/* point to last used character */
		break;

	    default:			/* \x becomes x for all other x */
		*p++ = *s;
		break;
	    }
	}
	else				/* not escaped, so just copy it */
	    *p++ = *s;
    }
    *p = '\0';				/* terminate final string */
}


#if defined(HAVE_STDC)
static void
do_fileinit(const char *bibfilename)	/* process one initialization file */
#else /* K&R style */
static void
do_fileinit(bibfilename)		/* process one initialization file */
const char *bibfilename;
#endif
{
    char *p;
    const char *ext;

    ext = GETDEFAULT(BIBCLEAN_EXT,INITFILE_EXT);

    if (strrchr(bibfilename,'.') != (char*)NULL) /* then have file extension */
    {   /* convert foo.bib to foo.ini and then process it as an init file */
	if ((p = (char*)malloc(strlen(bibfilename) + strlen(ext) + 1))
	    != (char*)NULL)
	{
	    (void)strcpy(p,bibfilename);
	    (void)strcpy(strrchr(p,'.'),ext);
	    do_initfile((char*)NULL,p);
	    FREE(p);
	}
    }
}


static void
do_field(VOID)			/* process BibTeX field name */
{
    int c;
    size_t k;

    the_value = the_file;
    token_start = the_file;	/* remember location of token start */

    for (k = 0, c = get_char(); (c != EOF) && is_idchar(c);
	c = get_char(), k++)
    {
	if (k >= MAX_TOKEN)
	{
	    APPEND_CHAR(current_field,k,c);
	    out_with_error(current_field, "Entry field name too long");
	    return;
	}
	else if ((k == 0) && !Isalpha(c))
	    error("Non-alphabetic character begins a field name");

	current_field[k] = (char)(((in_string == NO) && Isupper(c))
	    ? tolower(c) : c);
    }

    if (c != EOF)
	put_back(c);

    current_field[k] = (char)'\0';

    if (in_string == NO)		/* @String{...} contents untouched */
    {
	int n;

	/* Substitute a few field names that look better in upper case */
	for (n = 0;
	     (n < MAX_KEYWORD) && (field_pair[n].old_name != (const char*)NULL); ++n)
	{
	    if (STREQUAL(current_field,field_pair[n].old_name))
	    {
		(void)strcpy(current_field,field_pair[n].new_name);
		break;
	    }
	}
	if (strncmp("opt",current_field,3) == 0)

	{				/* Emacs bibtex.el expects OPT */
	    (void)strncpy(current_field,"OPT",3);
	}
    }

    if (k > 0)
	out_field();

    check_length(k);
}


static YESorNO
do_field_value_pair(VOID)		/* process field = value pair */
{
    if (eofile == YES) return (NO);

    do_field();
    if ((rflag == YES) || (eofile == YES) || (current_field[0] == '\0'))
	return (NO);

    space_count = 0L;		/* examined in do_Scribe_separator() */
    do_optional_space();	/* and set here */

    if (Scribe == YES)
	do_Scribe_separator();
    else
	do_equals();
    if ((rflag == YES) || (eofile == YES)) return (NO);

    do_optional_space();

    if (Scribe == YES)
	do_Scribe_value();
    else
	do_BibTeX_value();
    if ((rflag == YES) || (eofile == YES)) return (NO);

    return (YES);
}


#if defined(HAVE_STDC)
void
do_files(int argc, char *argv[])
#else /* K&R style */
void
do_files(argc,argv)
int argc;
char *argv[];
#endif
{
    FILE *fp;
    int k;				/* index into argv[] */

    k = argc;				/* to remove optimizer complaints about unused argument */

    if (argv[1] == (char*)NULL)	/* no files specified, so use stdin */
    {
	the_file.input.filename = "stdin";
	do_one_file(stdin);
    }
    else			/* else use command-line files left in argv[] */
    {
	for (k = 1; argv[k] != (char*)NULL; ++k)
	{
	    if (STREQUAL(argv[k],"-"))
	    {
		/* A filename of "-" is conventionally interpreted in
		   the UNIX world as a synonym for stdin, since that
		   system otherwise lacks true filenames for stdin,
		   stdout, and stdlog.	We process stdin with
		   do_one_file(), but never close it so that subsequent
		   read attempts will silently, and harmlessly, fail
		   at end-of-file. */
		the_file.input.filename = "stdin";
		do_one_file(stdin);
	    }
	    else if ((fp = tfopen(argv[k], OPEN_MODE_R)) == (FILE*)NULL)
	    {
		(void)fprintf(stdlog,
		    "\n%s Ignoring open failure on file [%s]\n",
		    ERROR_PREFIX, argv[k]);
		perror("perror() says");
	    }
	    else			/* open succeeded, so process file */
	    {
		if (k > 1)		/* supply blank line between */
		    out_newline();	/* entries at file boundaries */
		the_file.input.filename = argv[k];
		if (read_initialization_files == YES)
		    do_fileinit(the_file.input.filename);
		do_one_file(fp);
		(void)fclose(fp);	/* close to save file resources */
	    }
	}
    }
}


static void
do_group(VOID)                  /* copy a braced group verbatim */
{
    int c;
    char *s = shared_string;	/* memory-saving device */

    do_optional_space();

    if (prettyprint == YES)
    {
	out_c(DELETE_WHITESPACE);	/* discard any space that we found */
	do_open_brace();

	if (rflag == YES) return;

	out_c(DELETE_WHITESPACE);	/* discard any space that we found */

	while ((c = get_char(), c) != EOF)
	{
	    if ((brace_level == 1) && (close_char == (int)')') && (c == close_char))
	    {			/* end of @entry(...) */
		brace_level = 0;
		c = (int)'}';
	    }

	    if ((non_white_chars == 1) && (c == (int)'@'))
		error("@ begins line, but brace level is not zero after \
entry ``@%e{%k,''");

	    if ((brace_level == 0) && (c == (int)'}'))
	    {
		out_c(DELETE_WHITESPACE); /* discard any space that we found */
		out_close_brace();
	    }
	    else
		out_c(c);

	    if (brace_level == 0)
		break;
	}
    }
    else			/* prettyprint == NO */
    {				/* output entire braced group as one literal*/
	size_t k;		/* index into s[] */

	token_start = the_file;	/* remember location of token start */
	c = get_char();

	if (c == (int)'{')
	    close_char = (int)'}';
	else if (c == (int)'(')
	{
	    close_char = (int)')';
	    brace_level++;	/* get_char() could not do this for us */
	}
	else			/* raise error and try to resynchronize */
	{
	    s[0] = (char)c;
	    s[1] = '\0';
	    out_token(TOKEN_LITERAL,s);
	    error(
	    "Expected open brace or parenthesis.  Last entry = ``@%e{%k,''");
	    return;
	}
	s[0] = '{';		/* standardize to outer braces */

	for (k = 1; c != EOF;)
	{
	    c = get_char();

	    if (k >= MAX_TOKEN)
	    {
		error("Braced literal string too long for entry ``%e''");
		s[k] = '\0';
		out_token(TOKEN_LITERAL, s);
		return;
	    }

	    s[k++] = (char)c;

	    if ((c == close_char) && (c == (int)')'))
		brace_level--;	/* get_char() could not do this for us */

	    if (brace_level == 0)
		break;		/* here's the normal loop exit */
	}

	s[k-1] = '}';		/* standardize to outer braces */
	s[k] = '\0';		/* terminate string */
	out_token(TOKEN_LITERAL, s);
    }
}


#if defined(HAVE_STDC)
void
do_initfile(/*@null@*/ const char *pathlist, /*@null@*/ const char *name)
#else /* K&R style */
void
do_initfile(pathlist,name)
/*@null@*/ const char *pathlist;
/*@null@*/ const char *name;
#endif
{

    FILE *fp;
    char *p;

    if ((initialization_file_name = findfile(pathlist,name)) == (char*)NULL)
	return;				/* silently ignore missing files */

    if ((fp = tfopen(initialization_file_name,"r")) == (FILE*)NULL)
	return;				/* silently ignore missing files */

    while ((p = get_line(fp)) != (char *)NULL)
    {					/* process init file lines */
	SKIP_SPACE(p);
	if (*p == '\0')
	    NOOP;
	else if (*p == COMMENT_PREFIX)	
	    NOOP;
	else if (is_optionprefix((int)*p))
	    do_single_arg(p);		/* then expect -option [value] */
	else
	    do_new_pattern(p);		/* else expect field = "value" */
    }
    (void)fclose(fp);
}


static void
do_key_name(VOID)		/* process BibTeX citation key */
{
    int c;
    size_t k;

    token_start = the_file;	/* remember location of token start */

    for (k = 0, c = get_char();	(c != EOF) && (c != (int)',') && !Isspace(c);
	c = get_char(), k++)
    {
	if (k >= MAX_TOKEN)
	{
	    APPEND_CHAR(current_key,k,c);
	    out_with_error(current_key, "Citation key too long");
	    return;
	}
	current_key[k] = (char)c;
    }
    current_key[k] = (char)'\0';
    if (c != EOF)
	put_back(c);
    if (check_values == YES)
	check_key();
    out_string(TOKEN_KEY, current_key);
    check_length(k);
}


#if defined(HAVE_STDC)
void
do_keyword_file(/*@null@*/ const char *pathlist, /*@null@*/ const char *name)
#else /* K&R style */
void
do_keyword_file(pathlist,name)
/*@null@*/ const char *pathlist;
/*@null@*/ const char *name;
#endif
{
    FILE *fp;
    char *p;
    const char *keyword_file;	/* name of current keyword file */

    if (name == (const char*)NULL)
	return;

    if ((keyword_file = findfile(pathlist,name)) == (char*)NULL)
	return;				/* silently ignore missing files */

    if ((fp = tfopen(keyword_file,"r")) == (FILE*)NULL)
	return;				/* silently ignore missing files */

    /* The keyword file is expected to look like the output of
       -print-keyword-table: lines are (1) blank or empty, (2) comments
       from percent to end-of-line, or (3) pairs of whitespace-separated
       (input-key,output-key) values. */
    while ((p = get_line(fp)) != (char*)NULL)
    {
#define TOKEN_SEPARATORS	" \t"
	const char *the_old;
	const char *the_new;
	char *comment;

	comment = strchr(p, BIBTEX_COMMENT_PREFIX);
	if (comment != (const char*)NULL)
	    *comment = '\0';		/* then discard comment text */

	the_old = strtok(p, TOKEN_SEPARATORS);
	if (the_old == (const char*)NULL)
	    continue;			/* ignore blank or empty lines */
	if (*the_old == (char)BIBTEX_COMMENT_PREFIX)
	    continue;			/* ignore comment lines */
	the_new = strtok((char*)NULL, TOKEN_SEPARATORS);
	if (the_new == (const char*)NULL)
	{
	    (void)fprintf(stdlog,
			  "Expected output-key after input-key [%s] in keyword file [%s]\n",
			  the_old, keyword_file);
	    continue;
	}
#if defined(DEBUG)
	(void)fprintf(stdlog,
		      "DEBUG:\t[%s]\t[%s]\t[%s]\n",
		      keyword_file,
		      the_old,
		      the_new);
#endif
	add_keyword(the_old, the_new);
    }
    (void)fclose(fp);
#undef TOKEN_SEPARATORS
}


#if defined(HAVE_STDC)
static void
do_new_pattern(char *s)
#else /* K&R style */
static void
do_new_pattern(s)
char *s;
#endif
{
    char *field;
    char *p;
    char *value;

    p = s;

    /*******************************************************************
      We expect s[] to contain
	field = "value"
	field : "value"
	field   "value"
	field = "value" "message"
	field : "value" "message"
	field   "value" "message"
      Empty lines are silently ignored.
    *******************************************************************/

    field = get_token(p,&p,"=: \t\v\f");

    if (field == (char*)NULL)
	return;				/* then we have an empty line */

    if (p != (char*)NULL)		/* then we have more text */
    {
	YESorNO saw_space;

	saw_space = Isspace(*p) ? YES : NO;
	SKIP_SPACE(p);

	if ((saw_space == YES) || is_fieldvalueseparator(*p))
	{
	    if (is_fieldvalueseparator(*p))
		++p;			/* then move past separator */

	    SKIP_SPACE(p);

	    if (*p == '"')		/* then have quoted value */
	    {
		value = get_token(p,&p," \t\v\f");

		if (value != (char*)NULL)
		{
		    SKIP_SPACE(p);

		    if (*p == '"')	/* then have quoted message */
		    {
			add_pattern(field,value,get_token(p,&p," \t\v\f"));
			return;
		    }
		    else if ((*p == '\0') || (*p == (char)COMMENT_PREFIX))
		    {			/* have end of string s[] */
			add_pattern(field,value,(char*)NULL);
			return;
		    }
		}
	    }
	}
    }

    (void)fprintf(stdlog,"%s Bad line [%s] in initialization file [%s]\n",
	    ERROR_PREFIX, s, initialization_file_name);

    finish(EXIT_FAILURE);
}

static void
do_newline(VOID)
{
    int c;

    /* Newlines are standardized by bibclean inside bibliographic entries, */
    /* so we only output a newline here if we are outside such an entry. */

    c = get_char();
    if (c == (int)'\n')
    {
	if (brace_level == 0)
	    out_newline();
	else if (KEEP_PREAMBLE_SPACES())
	    out_newline();
	else if (KEEP_STRING_SPACES())
	    out_newline();
    }
    else
	put_back(c);
}


#if defined(HAVE_STDC)
static void
do_one_file(FILE *fp)		/* process one input file on fp */
#else /* K&R style */
static void
do_one_file(fp)			/* process one input file on fp */
FILE *fp;
#endif
{
    fpin = fp;			/* save file pointer globally for get_char() */

    new_io_pair(&the_file);
    eofile = NO;
    new_entry();
    while (eofile == NO)
    {
	do_optional_space();
	do_other();
	if (Scribe == YES)
	    do_Scribe_entry();
	else
	    do_BibTeX_entry();
    }
    if (prettyprint == YES)
    {
	out_c(DELETE_WHITESPACE); /* discard all trailing space */
	out_c((int)'\n');	/* supply final newline */
    }
    out_flush();		/* flush all buffered output */
    if (brace_level != 0)
	error("Non-zero brace level at end-of-file");
}


static void
do_open_brace(VOID)		/* process open brace or parenthesis */
{
    int c;

    c = get_char();

    if (c == EOF)
	return;
    else if (c == (int)'{')
    {
	close_char = (int)'}';
	out_open_brace();
    }
    else if (c == (int)'(')
    {
	close_char = (int)')';
	brace_level++;		/* get_char() could not do this for us */
	out_open_brace();	/* standardize parenthesis to brace */
    }
    else			/* raise error and try to resynchronize */
    {
	out_c(c);
	out_with_error("",
	    "Expected open brace or parenthesis.  Last entry = ``@%e{%k,''");
    }
}


static void
do_optional_inline_comment(VOID)
{
    size_t n;
    char *s;

    for (;;)
    {
	s = get_optional_space();
	switch ((int)s[0])
	{
	case BIBTEX_COMMENT_PREFIX:
	    n = strlen(s);
	    Memmove(s+1,s,n);
	    s[0] = BIBTEX_HIDDEN_DELIMITER;
	    s[n+1] = BIBTEX_HIDDEN_DELIMITER;
	    s[n+2] = '\0';
	    append_value(s);
	    break;

	case '\n':		/* newline or */
	case ' ':		/* horizontal space token */
	case '\f':
	case '\r':
	case '\t':
	case '\v':
	    if (KEEP_PREAMBLE_SPACES())
		append_value(s);
	    else if (KEEP_STRING_SPACES())
		append_value(s);
	    break;		/* else discard white space */

	default:		/* no more space or inline comments */
	    return;		/* here's the loop exit */
	}
    }
}


static void
do_optional_space(VOID)
{ /* skip over optional horizontal space, newline, and in-line comments */
    YESorNO save_wrapping;
    char *s;

    for (;;)
    {
	s = get_optional_space();
	switch (s[0])
	{
	case '\n':		/* newline token */
	    space_count++;
	    put_back((int)s[0]);
	    do_newline();
	    break;

	case ' ':		/* horizontal space token */
	case '\f':
	case '\r':
	case '\t':
	case '\v':
	    space_count++;
	    put_back((int)s[0]);
	    do_space();
	    break;

	case BIBTEX_COMMENT_PREFIX: /* in-line comment token */
	    save_wrapping = wrapping;
	    wrapping = NO;	/* inline comments are never line wrapped */
	    out_string(TOKEN_INLINE,s);
	    wrapping = save_wrapping;
	    break;

	default:		/* not optional space */
	    return;		/* here's the loop exit */
	}
    }
}


void
do_other(VOID)			/* copy non-BibTeX text verbatim */
{
    int c;			/* current input character */
    size_t k;			/* index into s[] */
    YESorNO save_wrapping;
    char *s = shared_string;	/* memory-saving device */

    save_wrapping = wrapping;
    wrapping = NO;

    /* For the purposes of lexical analysis (-no-prettyprint), we
       collect complete lines, rather than single characters. */

    for (k = 0, s[0] = (char)'\0'; (c = get_char(), c) != EOF; )
    {
	if ((c == (int)'@') && (non_white_chars == 1))
	{			/* new entry found */
	    put_back(c);
	    break;
	}
	if (k >= MAX_TOKEN)
	{		/* buffer full, empty it and start a new one */
	    APPEND_CHAR(s,k,c);
	    out_other(s);
	    k = 0;
	}
	else if (c == (int)'\n')	/* end of line */
	{
	    s[k] = (char)'\0';
	    out_other(s);	/* output line contents */
	    out_newline();	/* and then separate newline token */
	    k = 0;
	}
	else if (Isspace(s[0]))	/* then collecting whitespace */
	{
	    if (Isspace(c))	/* still collecting whitespace */
		s[k++] = (char)c;
	    else		/* end of whitespace */
	    {
		s[k] = (char)'\0';
		out_other(s);	/* output whitespace token */
		k = 0;
		s[k++] = (char)c; /* and start new one */
	    }
	}
	else
	    s[k++] = (char)c;
    }
    s[k] = (char)'\0';
    out_other(s);
    wrapping = save_wrapping;
}


static void
do_preamble(VOID)
{
    in_preamble = YES;
    do_preamble_2();
    in_preamble = NO;
}


static void
do_preamble_2(VOID)
{
    do_optional_space();

    if (prettyprint == YES)
	out_c(DELETE_WHITESPACE);	/* discard any space that we found */

    do_open_brace();
    if (rflag == YES) return;
    do_optional_space();

    do_BibTeX_value();
    if (rflag == YES) return;

    do_optional_space();
    do_close_brace();
}


void
do_print_keyword_table(VOID)
{
    size_t k;

    (void)fprintf(stdlog, "%%%%%% keyword mappings\n");
    for (k = 0; (field_pair[k].old_name != (const char *)NULL); ++k)
    {
	/* We intentionally include `deleted' entries (beginning with a hyphen), so
	   as not to conceal information from the user. */
	(void)fprintf(stdlog, "%-31s\t%-31s\n",
		      field_pair[k].old_name, field_pair[k].new_name);
    }
}


static void
do_Scribe_block_comment(VOID)
{
    char *p;

    p = get_Scribe_string();		/* expect to get "comment" */

    if (stricmp(p,"\"comment\"") == 0)
    {					/* found start of @Begin{comment} */
	int c;
	int k;
	char s[3+1];			/* to hold "end" */

	int b_level = 0;		/* brace level */

	for (k = 6; k > 0; --k)
	    out_c(DELETE_CHAR);		/* delete "@Begin" from output */
					/* that was output by do_entry_name() */
	out_s("@Comment{");		/* convert to BibTeX `comment' */

	while ((c = get_char(), c) != EOF)
	{
	    switch (c)
	    {
	    case '@':			/* lookahead for "@End" */
		s[0] = (char)get_char();
		s[1] = (char)get_char();
		s[2] = (char)get_char();
		s[3] = (char)'\0';

		if (stricmp(s,"end") == 0)
		{			/* then we have @End */
		    p = get_Scribe_string(); /* so get what follows */

		    if (stricmp(p,"\"Comment\"") == 0)
		    {
			out_close_brace();/* found @End{comment}, so finish
					   conversion to @Comment{...} */
			return;		/* block comment conversion done! */
		    }
		    else		/* false alarm, just stuff lookahead */
		    {			/* back into input stream */
			put_back_string(p);
			put_back_string(s);
		    }
		}
		else			/* lookahead was NOT "@End" */
		    put_back_string(s);

		break;

	    case '{':
		b_level++;
		break;

	    case '}':
		if (b_level <= 0)
		    out_open_brace(); /* keep output braces balanced */
		else
		    b_level--;

		break;
	    }				/* end switch(c) */

	    out_c(c);			/* copy one comment character */
	}				/* end while ((c = ...)) */
    }
    else				/* was not @Begin{comment} after all */
	put_back_string(p);
}


static void
do_Scribe_close_delimiter(VOID)
{
    int c;
    static char fmt[] = "Expected Scribe close delimiter `%c' [8#%03o], but \
found `%c' [8#%03o] instead for field ``%%f''";
    char msg[sizeof(fmt)];

    c = get_char();
    if ((parbreaks == NO) && (is_parbreak == YES))
    {
	APPEND_CHAR(msg,0,c);
	out_with_parbreak_error(msg);
	return;
    }
    if (c == EOF)
	return;
    else if (c == close_char)
    {
	if (c == (int)')')
	    brace_level--;	/* get_char() could not do this for us */
	out_close_brace();	/* standardize parenthesis to brace */
    }
    else			/* raise error and try to resynchronize */
    {
	out_c(c);
	(void)sprintf(msg, fmt, close_char, BYTE_VAL(close_char),
	    (Isprint(c) ? c : (int)'?'), BYTE_VAL(c));
	out_with_error("", msg);
    }
}


static void
do_Scribe_comment(VOID)
{
    int c;
    int b_level = 0;		    /* brace level */

    /* BibTeX does not yet have a comment syntax, so we just output the
       Scribe comment in braces, ensuring that internal braces are balanced. */

    do_optional_space();

    do_Scribe_open_delimiter();		/* this outputs an opening brace */
    if (rflag == YES) return;

    for (c = get_char(); (c != EOF) && (c != close_char); c = get_char())
    {
	if (c == (int)'{')
	    b_level++;
	else if (c == (int)'}')
	{
	    b_level--;
	    if (b_level < 0)
	    {
		out_open_brace(); /* force matching internal braces */
		b_level++;
	    }
	}
	out_c(c);
    }
    for (; b_level > 0; b_level--)
	out_close_brace();	/* force matching internal braces */

    out_close_brace();
}


static void
do_Scribe_entry(VOID)
{
    /*************************************************************
     Parse a Scribe entry, one of:
       @entry-name{key,field=value,field=value,...,}
       @string{name=value}
       @comment{...}
       @begin{comment}...@end{comment}
     The = separator in field/value pairs may also be a space or
     a slash.
     Any of the seven Scribe delimiters can be used to surround
     the value(s) following @name, and to surround values of
     field value pairs.
    *************************************************************/

    new_entry();

    do_at();
    if ((rflag == YES) || (eofile == YES)) return;

    do_optional_space();

    if (prettyprint == YES)
	out_c(DELETE_WHITESPACE);	/* discard any space that we found */

    do_entry_name();
    if (rflag == YES) return;

    if (STREQUAL(current_entry_name,"Comment"))
	do_Scribe_comment();
    else if (STREQUAL(current_entry_name,"Begin"))
	do_Scribe_block_comment();
    else if (STREQUAL(current_entry_name,"String"))
	do_string();
    else
    {
	int save_close_char;

	do_optional_space();

	if (prettyprint == YES)
	    out_c(DELETE_WHITESPACE);	/* discard any space that we found */

	do_Scribe_open_delimiter();
	if (rflag == YES) return;
	save_close_char = close_char;

	brace_level = 1;		/* get_char() cannot do this for us */

	do_optional_space();

	do_key_name();
	if (rflag == YES) return;

	do_optional_space();

	do_comma();
	if (rflag == YES) return;

	do_optional_space();

	while (do_field_value_pair() == YES)
	{
	    do_optional_space();
	    do_comma();		/* this supplies any missing optional comma */
	    if ((rflag == YES) || (eofile == YES)) return;
	    do_optional_space();
	}
	if (rflag == YES) return;

	do_optional_space();

	close_char = save_close_char;
	do_Scribe_close_delimiter();
    }
    flush_inter_entry_space();
}


static void
do_Scribe_open_delimiter(VOID)		/* process open delimiter */
{
    int c;
    char *p;

    c = get_char();

    if (c == EOF)
	return;
    else
    {
	p = strchr(Scribe_open_delims,c);
	if (p == (char*)NULL)
	{
	    out_c(c);
	    out_with_error("",
      "Expected Scribe open delimiter, one of { [ ( < ' \" ` for field ``%f''");
	    return;
	}
	close_char = (int)Scribe_close_delims[(int)(p - Scribe_open_delims)];
	out_open_brace();	/* standardize open delimiter to brace */
    }
}


static void
do_Scribe_separator(VOID)
{
    int c;
    YESorNO saw_space;

    the_value = the_file;

    saw_space = (space_count > 0L) ? YES : NO;

    c = get_char();
    if ((parbreaks == NO) && (is_parbreak == YES))
    {
	char msg[2];
	APPEND_CHAR(msg,0,c);
	out_with_parbreak_error(msg);
	return;
    }
    if (c == EOF)
	NOOP;
    else if ((c == (int)'=') || (c == (int)'/'))
	out_equals();
    else if (saw_space == YES)	/* have field value with no binary operator */
    {
	out_equals();		/* supply the missing = operator */
	put_back(c);		/* this is first character of value string */
    }
    else			/* looks like run-together fieldvalue */
    {
	out_c(c);
	out_with_error("",
	   "Expected Scribe separator \"=\", \"/\", or \" \" for field ``%f''");
    }
    out_spaces((int)(value_indentation - the_file.output.column_position));
				/* supply leading indentation */
}


/***********************************************************************
Scribe field values can take several forms, as illustrated by this
simple BNF grammar:

Scribe-value-string:
	<open-delimiter><not-open-or-close-delimiter>*<close-delimiter> |
	<digit><letter-or-digit-or-dot>*

***********************************************************************/

static void
do_Scribe_value(VOID)			/* process Scribe value string */
{
    the_value = the_file;
    (void)strcpy(current_value,get_Scribe_string());
    if ((rflag == YES) || (eofile == YES))
	out_s(current_value);
    else
	out_value();
}


#if defined(HAVE_STDC)
static void
do_single_arg(char *s)
#else /* K&R style */
static void
do_single_arg(s)
char *s;
#endif
{					/* expect -option or -option value */
    char *temp_argv[4];			/* "program" "-option" "value" NULL */
    int temp_argc;			/* temporary argument count */

    temp_argv[0] = program_name;	/* 0th argument always program name */
    temp_argv[1] = get_token(s,&s," \t\v\f"); /* option */
    temp_argv[2] = get_token(s,&s," \t\v\f"); /* value */
    temp_argv[3] = (char *)NULL;
    temp_argc = (temp_argv[2] == (char*)NULL) ? 2 : 3;
    do_args(temp_argc,temp_argv);
}


static void
do_space(VOID)
{
    int c;
    char *s = shared_string;	/* memory-saving device */
    size_t k;			/* index into s[] */

    token_start = the_file;	/* remember location of token start */

    c = get_char();
    s[0] = '\0';
    for (k = 0; (c != EOF) && Isspace(c) && (c != (int)'\n'); )
    {
	if (k >= MAX_TOKEN)
	{		/* split long comments into multiple ones */
	    s[k] = '\0';
	    if (prettyprint == NO)
		out_token(TOKEN_SPACE,s);
	    else if (keep_spaces == YES)
		out_s(s);
	    else if (KEEP_PREAMBLE_SPACES())
		out_s(s);
	    else if (KEEP_STRING_SPACES())
		out_s(s);
	    /* else discard: spaces are standardized during prettyprinting */
	    k = 0;
	}
	s[k++] = (char)c;
	c = get_char();
    }
    s[k] = '\0';		/* terminate token string */

    if (prettyprint == NO)
	out_token(TOKEN_SPACE,s);
    else if (keep_spaces == YES)
	out_s(s);
    else if (KEEP_PREAMBLE_SPACES())
	out_s(s);
    else if (KEEP_STRING_SPACES())
	out_s(s);
    /* else discard: spaces are standardized during prettyprinting */

    put_back(c);		/* restore lookahead */
}


static void
do_string(VOID)			/* process @String{abbrev = "value"} */
{
    in_string = YES;
    do_string_2();
    in_string = NO;
}


static void
do_string_2(VOID)			/* process @String{abbrev = "value"} */
{
    do_optional_space();

    if (prettyprint == YES)
	out_c(DELETE_WHITESPACE);	/* discard any space that we found */

    do {				/* one trip loop */
	do_open_brace();
	if (rflag == YES) break;

	do_optional_space();

	if (do_field_value_pair() == NO)
	    break;
	if (rflag == YES) break;

	do_optional_space();

	do_close_brace();
	if (rflag == YES) break;
    } while (0);
}


#if defined(HAVE_STDC)
static void
enlarge_table(PATTERN_TABLE *table)
#else /* K&R style */
static void
enlarge_table(table)
PATTERN_TABLE *table;
#endif
{
    if (table->maximum_size == 0)
	table->patterns = (MATCH_PATTERN*)malloc(sizeof(MATCH_PATTERN) *
		TABLE_CHUNKS);
    else
	table->patterns = (MATCH_PATTERN*)realloc((char*)table->patterns,
		sizeof(MATCH_PATTERN) * (table->maximum_size + TABLE_CHUNKS));
					/* NB: Sun C++ requires (char*) cast */
    if (table->patterns == (MATCH_PATTERN*)NULL)
	fatal("Out of memory for pattern table space");
    table->maximum_size += TABLE_CHUNKS;
}


static void
flush_inter_entry_space(VOID) /* standardize to 1 blank line between entries */
{
    int c;

    if (keep_spaces == YES)
	return;

    put_back((c = get_next_non_blank()));
    if (c != EOF)
	out_newline();
    out_newline();
}


void
free_keyword_table(VOID)
{
    int k;

    for (k = 0; k < MAX_KEYWORD; ++k)
    {
	if (field_pair[k].old_name != (const char *)NULL)
	{
	    FREE(field_pair[k].old_name);
	    field_pair[k].old_name = (const char *)NULL;
	}

	if (field_pair[k].new_name != (const char *)NULL)
	{
	    FREE(field_pair[k].new_name);
	    field_pair[k].new_name = (const char *)NULL;
	}
    }
}


static char *
get_braced_string(VOID)
{
    int b_level = 0;		/* brace level */
    int c;			/* current input character */
    size_t k;			/* index into s[] */
    size_t n;			/* index into t[] */
    char *s = shared_string;	/* memory-saving device */
    char t[MAX_TOKEN_SIZE];	/* working area for braced string */

    in_value = YES;
    for (c = get_char(), k = 0, *s = '\0'; c != EOF; )
    {
	if ((parbreaks == NO) && (is_parbreak == YES))
	{
	    put_back(c);		/* so this comes AFTER error message */
	    out_with_parbreak_error(s);
	    in_value = NO;
	    return (EMPTY_STRING(s));
	}
	else if (k >= MAX_TOKEN)
	{
	    APPEND_CHAR(s,k,c);
	    out_with_error(s, "BibTeX string too long for field ``%f''");
	    in_value = NO;
	    return (EMPTY_STRING(s));
	}
	else
	{
	    if (c == (int)'\n')
		c = get_parbreak();
	    else if (c == (int)'\f')
	    {
		if (keep_linebreaks == NO)
		    c = (keep_parbreaks == YES) ? PARBREAK : (int)' ';
	    }
	    else if (Isspace(c))
		c = (int)' ';		/* change whitespace to real space */
	    else if (c == (int)'{')
		b_level++;
	    else if (c == (int)'}')
		b_level--;
	    else if (c == (int)'\\')		/* \\<NEWLINE> -> \\<LINEBREAK> */
	    {
		s[k++] = (char)c;
		c = get_char();
		if (c == (int)'\\')		/* found \\ */
		{
		    s[k++] = (char)c;
		    c = get_linebreak();
		}
		else			/* \<NON-BACKSLASH> -> \ */
		{
		    put_back(c);
		    c = (int)'\0';
		}
	    }
	    if (c != (int)'\0')
		s[k++] = (char)c;
	    if (b_level == 0)
		break;			/* here's the loop exit */
	    c = Isspace(c) ? get_next_non_blank() : get_char();
	}
    }
    s[k] = (char)'\0';

    /* Now convert braced string to quoted string */

    for (b_level = 0, k = 0, n = 0; (s[k] != '\0') && (n < (MAX_TOKEN_SIZE-2)); ++k)
    {
	if (s[k] == '{')
	    b_level++;
	else if (s[k] == '}')
	    b_level--;
	if ((s[k] == '"') && (b_level == 1))	/* k > 0 if this is true */
	{					/* so we can omit that check */
	    if (German_style == YES)
	    {
		switch (s[k+1])
		{
		case '"':
		case '-':
		case '<':
		case '>':
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		case '\'':
		case '`':
		case 'a':
		case 'c':
		case 'e':
		case 'f':
		case 'i':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 's':
		case 't':
		case 'u':
		case '|':
		    t[n++] = (char)'{';
		    t[n++] = (char)'"';
		    t[n++] = s[++k];
		    t[n++] = (char)'}';
		    break;
		default:		/* should never happen */
		    t[n++] = (char)'{';
		    t[n++] = (char)'"';
		    t[n++] = (char)'}';
		    warning(
			"Unexpected quote usage in German-style braced string");
		    break;
		}
	    }
	    else			/* German_style == NO */
	    {
		if ((s[k-1] == '\\') &&
		    (s[k+1] == '{') &&
		    ((s[k+2] != '\0') && (s[k+2] != '{') && (s[k+2] != '}')) &&
		    (s[k+3] == '}'))	/* change \"{x}y to {\"{x}}y */
		{ /* BUG FIX for overlooked case: this branch added [08-Mar-1995] */
		    n--;			/* now t[n] == '\\' */
		    k--;			/* now s[k] == '\\' */
		    t[n++] = (char)'{';
		    t[n++] = s[k++];		/* copy 5-char string \"{x} */
		    t[n++] = s[k++];
		    t[n++] = s[k++];
		    t[n++] = s[k++];
		    t[n++] = s[k];
		    t[n++] = (char)'}';
		}
		else if (s[k-1] == '\\')	/* change \"xy to {\"x}y */
		{
		    n--;			/* overwriting t[n] == '\\' */
		    t[n++] = (char)'{';
		    t[n++] = (char)'\\';
		    t[n++] = (char)'"';
		    t[n++] = s[++k];
		    t[n++] = (char)'}';
		}
		else				/* change x" to x{"} */
		{
		    t[n++] = (char)'{';
		    t[n++] = (char)'"';
		    t[n++] = (char)'}';
		}
	    }
	}
	else
	    t[n++] = s[k];
    }
    t[0] = (char)'"';				/* change initial and final */
    APPEND_CHAR(t,n-1,'"');			/* braces to quotes */
    check_length(n);
    if (c == EOF)
	error("End-of-file in braced string");
    in_value = NO;
    return (strcpy(s,t));
}


static char *
get_digit_string(VOID)
{
    int c;			/* current input character */
    size_t k;			/* index into s[] */
    char *s = shared_string;	/* memory-saving device */

    k = 0;
    s[k++] = (char)'"';			/* convert to quoted string */
    for (c = get_char(); (c != EOF) && Isdigit(c); )
    {
	if (k >= MAX_TOKEN)
	{
	    APPEND_CHAR(s,k,c);
	    out_with_error(s, "BibTeX string too long for field ``%f''");
	    return (EMPTY_STRING(s));
	}
	else
	{
	    s[k++] = (char)c;
	    c = get_char();
	}
    }
    put_back(c);			/* we read past end of digit string */
    s[k++] = (char)'"';			/* supply terminating quote */
    s[k] = (char)'\0';
    check_length(k);
    return (s);
}


static char *
get_identifier_string(VOID)
{
    int c;			/* current input character */
    size_t k;			/* index into s[] */
    char *s = shared_string;	/* memory-saving device */

    for (c = get_char(), k = 0; (c != EOF) && is_idchar(c); )
    {
	if (k >= MAX_TOKEN)
	{
	    APPEND_CHAR(s,k,c);
	    out_with_error(s, "BibTeX string too long for field ``%f''");
	    return (EMPTY_STRING(s));
	}
	else
	{
	    s[k++] = (char)c;
	    c = get_char();
	}
    }
    put_back(c);		/* we read past end of identifier string */
    s[k] = (char)'\0';
    check_length(k);
    return (s);
}


static char *
get_inline_comment(VOID)
{
    int c;			/* current input character */
    char *s = shared_string;	/* memory-saving device */

    s[0] = '\0';
    c = get_char();

    if ((Scribe == NO) && (c == BIBTEX_COMMENT_PREFIX))
    {
	size_t k;		/* index into s[] */
	int newlines;

	token_start = the_file;	/* remember location of token start */

	for (s[0] = (char)BIBTEX_COMMENT_PREFIX, c = get_char(),
	     k = 1, newlines = 0; (c != EOF); )
	{	/* collect up to newline, plus following horizontal space  */
	    if ((newlines == 1) && !Isspace(c))
		break;		/* here's a loop exit */

	    if (k >= MAX_TOKEN)
	    {		/* split long comments into multiple ones */
		s[k++] = '\n';
		put_back(c);	/* restore lookahead */
		c = BIBTEX_COMMENT_PREFIX; /* we put this back too later */
		break;		/* here's a loop exit */
	    }

	    if (c == (int)'\n')
		newlines++;

	    if (newlines > 1)
		break;		/* here's a loop exit */

	    s[k++] = (char)c;
	    c = get_char();
	}

	s[k] = '\0';		/* terminate token string */
    }

    put_back(c);		/* restore lookahead */

    return (s);
}


/*@null@*/
#if defined(HAVE_STDC)
char *
get_line(FILE *fp)
#else /* K&R style */
char *
get_line(fp)
FILE	*fp;
#endif
{   /* return a complete line to the caller, discarding backslash-newlines */
    /* on consecutive lines, and discarding the final newline.  At EOF, */
    /* return (char*)NULL instead. */
    static char line[MAX_LINE];
    static char *p;
    static char *more;

    more = &line[0];
    line[0] = (char)'\0';		/* must set in case we hit EOF */
    while (fgets(more,(int)(&line[MAX_LINE] - more),fp) != (char *)NULL)
    {
	p = strchr(more,'\n');
	if (p != (char*)NULL)		/* did we get a complete line? */
	{				/* yes */
	    *p = '\0';			/* zap final newline */
	    if ((p > &line[0]) && (*(p-1) == '\\'))
					/* then have backslash-newline */
		more = p - 1;		/* so get another line */
	    else			/* otherwise have normal newline */
		break;			/* so return the current line */
	}
	else				/* no, return partial line */
	    break;
    }
    return ((line[0] == '\0' && (feof(fp) != 0)) ? (char*)NULL : &line[0]);
}


static char *
get_optional_space(VOID)
{
    int c;			/* current input character */
    char *s = shared_string;	/* memory-saving device */

    /* Space tokens are returned as single-character values, because */
    /* do_optional_space() pushes them back into the input stream before */
    /* calling do_newline() and do_space() for further processing. However, */
    /* inline comments are returned as multiple-character values */

    c = get_char();
    switch (c)
    {
    case '\n':			/* newline token */
    case ' ':			/* horizontal space token */
    case '\f':
    case '\r':
    case '\t':
    case '\v':
	s[0] = (char)c;
	s[1] = '\0';
	break;

    case BIBTEX_COMMENT_PREFIX: /* in-line comment token */
	put_back(c);
	s = get_inline_comment();
	break;

    default:		/* not optional space */
	put_back(c);
	s[0] = '\0';
	break;
    }
    return (s);
}


static int
get_parbreak(VOID)
{
    /* This function is called when the current input character is a
       newline.  If keep_linebreaks is YES, it returns a LINEBREAK.  If
       keep_parbreaks is NO, then it returns a space.  Otherwise, it
       looks ahead to the next non-blank character, and if a paragraph
       break was seen, the return value is a PARBREAK, and otherwise,
       a space, or a newline if keep_linebreaks is YES. */

    if ((in_value == YES) && (keep_linebreaks == YES))
	return (LINEBREAK);
    else if (KEEP_PREAMBLE_SPACES())
	return (LINEBREAK);
    else if (KEEP_STRING_SPACES())
	return (LINEBREAK);
    else if ((in_value == YES) && (keep_parbreaks == YES))
    {
	put_back((int)'\n');	/* so get_next_non_blank() finds it */
	put_back(get_next_non_blank());
	return ((is_parbreak == YES) ? PARBREAK :
		((keep_linebreaks == YES) ? (int)'\n' : (int)' '));
    }
    else
	return ((int)' ');
}


static char *
get_quoted_string(VOID)
{
    int b_level = 0;		/* brace level */
    int c;			/* current input character */
    size_t k;			/* index into s[] */
    char *s = shared_string;	/* memory-saving device */

    in_value = YES;
    for (c = get_char(), k = 0, *s = '\0'; c != EOF; )
    {
	if ((parbreaks == NO) && (is_parbreak == YES))
	{
	    put_back(c);		/* so this comes AFTER error message */
	    out_with_parbreak_error(s);
	    in_value = NO;
	    return (EMPTY_STRING(s));
	}
	else if (k >= MAX_TOKEN)
	{
	    APPEND_CHAR(s,k,c);
	    out_with_error(s, "BibTeX string too long for field ``%f''");
	    in_value = NO;
	    return (EMPTY_STRING(s));
	}
	else
	{
	    if (c == (int)'\n')
		c = get_parbreak();
	    else if (c == (int)'\f')
	    {
		if (keep_linebreaks == NO)
		    c = (keep_parbreaks == YES) ? PARBREAK : (int)' ';
	    }
	    else if (Isspace(c))
		c = (int)' ';		/* change whitespace to real space */
	    else if (c == (int)'{')
		b_level++;
	    else if (c == (int)'}')
		b_level--;
	    else if (c == (int)'\\')		/* \\<NEWLINE> -> \\<LINEBREAK> */
	    {
		s[k++] = (char)c;
		c = get_char();
		if (c == (int)'\\')		/* found \\ */
		{
		    s[k++] = (char)c;
		    c = get_linebreak();
		}
		else			/* \<NON-BACKSLASH> -> \ */
		{
		    put_back(c);
		    c = (int)'\0';
		}
	    }
	    if (c != (int)'\0')
		s[k++] = (char)c;
	    if ((c == (int)'"') && (k > 1) && (b_level == 0))
	    {
		if (s[k-2] == '\\')
		{
		    /* convert \"x inside string at brace-level 0 to {\"x}: */
		    /* illegal, but hand-entered bibliographies have it */
		    c = get_char();
		    if (c != EOF)
		    {
			k = k - 2;
			s[k++] = (char)'{';
			s[k++] = (char)'\\';
			s[k++] = (char)'"';
			s[k++] = (char)c;
			s[k++] = (char)'}';
		    }
		}
		else
		    break;		/* here's the loop exit */

	    }
	    c = Isspace(c) ? get_next_non_blank() : get_char();
	}
    }
    s[k] = (char)'\0';
    check_length(k);
    if (c == EOF)
	error("End-of-file in quoted string");
    in_value = NO;
    return (s);
}


static char *
get_Scribe_delimited_string(VOID)
{
    int c;
    int close_delim;
    size_t k;
    int last_c;
    char *p;
    char *s;

    s = shared_string;		/* memory-saving device */
    last_c = EOF;

    c = get_char();
    p = strchr(Scribe_open_delims,c);	/* maybe delimited string? */

    if (p == (char*)NULL)
    {
	APPEND_CHAR(s,0,c);
	out_with_error(s,"Expected Scribe value string for field ``%f''");
	return (EMPTY_STRING(s));
    }

    /* We have a delimited string */

    close_delim = (int)Scribe_close_delims[(int)(p - Scribe_open_delims)];

    c = get_next_non_blank();		/* get first character in string */
					/* ignoring leading space */
    for (k = 0, s[k++] = (char)'"';
	 (c != EOF) &&
	 !((last_c != (int)'\\') && (c == close_delim)) &&
	 (k < MAX_TOKEN);
	 k++)
    {
	if ((parbreaks == NO) && (is_parbreak == YES))
	{
	    s[k] = '\0';		/* ensure string termination */
	    put_back(c);		/* so this comes AFTER error message */
	    out_with_parbreak_error(s);
	    return (EMPTY_STRING(s));
	}
	if (c == (int)'"')			/* protect quotes inside string */
	{
	    if (s[k-1] == '\\')
	    {				/* then TeX accent in Scribe string */
		last_c = c;
		c = get_char();
		if (c == (int)'{')		/* change \"{ to {\" */
		{
		    s[k-1] = (char)'{';
		    s[k] = (char)'\\';
		    s[++k] = (char)'"';
		}
		else			/* change \". to {\".} (. = any) */
		{
		    s[k-1] = (char)'{';
		    s[k] = (char)'\\';
		    s[++k] = (char)'"';
		    s[++k] = (char)c;
		    s[++k] = (char)'}';
		}
	    }
	    else
	    {
		s[k] = (char)'{';
		s[++k] = (char)'"';
		s[++k] = (char)'}';
	    }
	}
	else if (c == (int)'\n')
	    s[k] = (char)get_parbreak();
	else if (c == (int)'\f')
	    s[k] = (keep_linebreaks == YES) ? (char)'\f' :
		((keep_parbreaks == YES) ? (char)PARBREAK : (char)' ');
	else if (Isspace(c))
	    s[k] = (char)' ';		/* change whitespace to real space */
	else if (c == (int)'\\')		/* \\<NEWLINE> -> \\<LINEBREAK> */
	{
	    s[k] = (char)c;
	    c = get_char();
	    if (c == (int)'\\')		/* found \\ */
	    {
		s[++k] = (char)c;
		c = get_linebreak();
		if (c != (int)'\0')
		    s[++k] = (char)c;
	    }
	    else			/* \<NON-BACKSLASH> -> \ */
		put_back(c);
	}
	else
	    s[k] = (char)c;
	last_c = c;
	c = Isspace(c) ? get_next_non_blank() : get_char();
    }

    APPEND_CHAR(s,k,'"');		/* append close delimiter */
    if (k >= MAX_TOKEN)
    {
	out_with_error(s, "Scribe string too long for field ``%f''");
	return (EMPTY_STRING(s));
    }
    check_length(k);
    return (s);
}


static char *
get_Scribe_identifier_string(VOID)	/* read undelimited identifier */
{					/* and return quoted equivalent */
    int c;
    size_t k;
    char *s = shared_string;		/* memory-saving device */

    c = get_char();
    for (k = 0, s[k++] = (char)'"'; is_idchar(c) && (k < MAX_TOKEN);
	k++, c = get_char())
    {
	s[k] = (char)c;
    }
    put_back(c);			/* put back lookahead */
    APPEND_CHAR(s,k,'"');
    if (k >= MAX_TOKEN)
    {
	out_with_error(s, "Scribe number string too long for field ``%f''");
	return (EMPTY_STRING(s));
    }
    ++k;
    check_length(k);
    return (s);
}


static char *
get_Scribe_string(VOID)			/* read Scribe string */
{
    int c;

    do_optional_space();

    c = get_char();			/* peek ahead one character */
    put_back(c);

    return (is_idchar(c) ?
	    get_Scribe_identifier_string() :
	    get_Scribe_delimited_string());
}


static char *
get_simple_string(VOID)		/* read simple BibTeX string */
{
    int c;			/* current input character */
    char *s = shared_string;	/* memory-saving device */

    *s = '\0';
    c = get_next_non_blank();	/* peek ahead to next non-blank */

    if (c == EOF)
	return (EMPTY_STRING(s));
    else if ((parbreaks == NO) && (is_parbreak == YES))
    {
	put_back(c);		/* so this comes AFTER error message */
	out_with_parbreak_error(s);
	return (EMPTY_STRING(s));
    }

    put_back(c);		/* put back lookahead */

    token_start = the_file;	/* remember location of string start */

    if (c == (int)'{')
	return (get_braced_string());
    else if (Isdigit(c))
	return (get_digit_string());
    else if (c == (int)'"')
	return (get_quoted_string());
    else if (Isalpha(c))
	return (get_identifier_string());
    else
    {
	out_with_error("", "Expected BibTeX value string for field ``%f''");
	return (EMPTY_STRING(s));
    }
}


/*@null@*/
#if defined(HAVE_STDC)
static char *
get_token(/*@null@*/ char *s, char **nextp, const char *terminators)
#else /* K&R style */
static char *
get_token(s,nextp,terminators)
/*@null@*/ char *s;
char **nextp;
const char *terminators;
#endif
{
    char *t;
    char *token;

    t = s;

    /*******************************************************************
      Ignoring leading space, find the next token in s[], stopping at
      end-of-string, or one of the characters in terminators[],
      whichever comes first.  Replace the terminating character in s[]
      by a NUL.  Set *nextp to point to the next character in s[], or to
      (char*)NULL if end-of-string was reached.  Return (char*)NULL if
      no token was found, or else a pointer to its start in s[].  The
      job is terminated with an error message if a syntax error is
      detected.

      Quoted strings are correctly recognized as valid tokens, and
      returned with their surrounding quotes removed, and embedded
      escape sequences expanded.  The comment character is recognized
      outside quoted strings, but not inside.
      *******************************************************************/

    if (t != (char*)NULL)
	SKIP_SPACE(t);

    if ((t == (char*)NULL) || (*t == '\0') || (*t == (char)COMMENT_PREFIX))
    {					/* initial sanity check */
	t = (char*)NULL;		/* save for *nextp later */
	token = (char*)NULL;
    }
    else if (*t == '"')			/* then collect quoted string */
    {
	token = ++t;			/* drop leading quote */
	for ( ; (*t != '\0') && (*t != '"'); ++t)
	{				/* find ending quote */
	    /* step over escape sequences; it doesn't matter if we have */
	    /* \123, since we are only looking for the ending quote */
	    if (*t == '\\')
		++t;
	}
	if (*t == '"')			/* then found valid string */
	{
	    *t++ = '\0';		/* terminate token */
	    do_escapes(token);		/* and expand escape sequences */
	}
	else
	{
	    (void)fprintf(stdlog,
		"%s Bad line [%s] in initialization file [%s]\n",
		ERROR_PREFIX, s, initialization_file_name);
	    finish(EXIT_FAILURE);
	}
    }
    else				/* else collect unquoted string */
    {
	for (token = t; (*t != '\0') && (*t != (char)COMMENT_PREFIX) &&
		 !IN_SET(terminators, *t); ++t)
	    continue;			/* scan over token */

	if ((*t == '\0') || (*t == (char)COMMENT_PREFIX)) /* then hit end of s[] */
	    t = (char*)NULL;		/* save for *nextp later */
	else				/* else still inside s[] */
	    *t++ = '\0';		/* terminate token */
    }
    *nextp = t;				/* set continuation position */
    return (token);
}


void
keyword_initialize(VOID)
{
    int k;

    for (k = 0; (k < MAX_KEYWORD) && (field_pair[k].old_name != (const char *)NULL); ++k)
    {
	field_pair[k].old_name = Strdup(field_pair[k].old_name);
	field_pair[k].new_name = Strdup(field_pair[k].new_name);
    }

    for ( ; k < MAX_KEYWORD; ++k)
    {
	field_pair[k].old_name = (const char *)NULL;
	field_pair[k].new_name = (const char *)NULL;
    }
}


static void
new_entry(VOID)			/* initialize for new BibTeX @name{...} */
{
    at_level = 0;
    brace_level = 0;
    is_parbreak = NO;
    non_white_chars = 0;
    rflag = NO;				/* already synchronized */
    current_entry_name[0] = '\0'; 	/* empty current_xxx[] strings */
    current_field[0] = '\0';
    current_key[0] = '\0';
    current_value[0] = '\0';
}


#if defined(HAVE_STDC)
static void
new_io_pair(IO_PAIR *pair)
#else /* K&R style */
static void
new_io_pair(pair)
IO_PAIR *pair;
#endif
{
    new_position(&pair->input);
    new_position(&pair->output);
}


#if defined(HAVE_STDC)
static void
new_position(POSITION *position)
#else /* K&R style */
static void
new_position(position)
POSITION *position;
#endif
{
    position->byte_position = 0L;
    position->last_column_position = 0L;
    position->column_position = 0L;
    position->line_number = 1L;
}


static void
out_close_brace(VOID)
{
    out_string(TOKEN_RBRACE, "}");
}


static void
out_comma(VOID)
{
    YESorNO save_wrapping;

    save_wrapping = wrapping;
    wrapping = NO;
    out_string(TOKEN_COMMA, ",");
    wrapping = save_wrapping;
}


static void
out_complex_value(VOID)
{
    char *s;
    char *p;

    /* A complex value may contain concatenated simple strings with */
    /* intervening inline comments delimited by BIBTEX_HIDDEN_DELIMITER. */
    /* We split it apart and output separate tokens. */

    for (s = &current_value[0]; (*s != '\0'); )
    {
	p = strchr(s,BIBTEX_HIDDEN_DELIMITER);
	if (p == (char*)NULL)
	{
	    out_string((*s == '"') ? TOKEN_VALUE : TOKEN_ABBREV,s);
	    check_length(strlen(s));
	    return;
	}
	*p = '\0';
	out_string((*s == '"') ? TOKEN_VALUE : TOKEN_ABBREV,s);
	check_length(strlen(s));
	s = p + 1;
	p = strchr(s,BIBTEX_HIDDEN_DELIMITER);
	if (p == (char*)NULL)	/* should never happen, but recover safely */
	    p = strchr(s,'\0');	/* if it does */
	*p = '\0';
	out_string(TOKEN_INLINE,s);
	check_length(strlen(s));
	s = p + 1;
    }
}


static void
out_equals(VOID)
{
    if (prettyprint == YES)
    {
	if (KEEP_PREAMBLE_SPACES())
	    out_c((int)'=');
	else if (KEEP_STRING_SPACES())
	    out_c((int)'=');
	else
	{
	    out_c((int)' ');
	    if (align_equals == YES)
		out_spaces((int)(value_indentation -
				 the_file.output.column_position -  2));
	    out_c((int)'=');	/* standardize to = */
	    out_c((int)' ');	/* always surround = by spaces */
	}
    }
    else
	out_token(TOKEN_EQUALS,"=");
}


static void
out_field(VOID)
{
    if (prettyprint == YES)
    {
	if (in_string == NO)
	    out_spaces(field_indentation);
	out_s(current_field);
    }
    else
	out_token((in_string == YES) ? TOKEN_ABBREV : TOKEN_FIELD,
		  current_field);
}


static void
out_open_brace(VOID)
{
    out_string(TOKEN_LBRACE, "{");
}


#if defined(HAVE_STDC)
static void
out_other(const char *s)	/* output a non-BibTeX string */
#else /* K&R style */
static void
out_other(s)
const char *s;
#endif
{
    if (prettyprint == YES)
	out_s(s);
    else
    {
	if (Isspace(s[0])) /* do_other() guarantees whole token is whitespace */
	    out_token(TOKEN_SPACE, s);
	else if (s[0] == (char)BIBTEX_COMMENT_PREFIX)
	    out_token(TOKEN_INLINE, s);
	else
	    out_token(TOKEN_LITERAL, s);
    }
}


static void
out_value(VOID)
{
    static OPTION_FUNCTION_ENTRY checks[] =
    {
	{"author",		6,	check_other},
	{"book-DOI",		8,	check_DOI},
	{"book-URL",		8,	check_URL},
	{"chapter",		7,	check_chapter},
	{"CODEN",		5,	check_CODEN},
	{"DOI",			3,	check_DOI},
	{"editor",		6,	check_other},
	{"ISBN",		4,	check_ISBN},
	{"ISBN-13",		7,	check_ISBN_13},
	{"ISSN",		4,	check_ISSN},
	{"ISSN-L",		6,	check_ISSN_L},
	{"journal-URL",		11,	check_URL},
	{"month",		5,	check_month},
	{"number",		6,	check_number},
	{"pages",		5,	check_pages},
	{"URL",			3,	check_URL},
	{"volume",		6,	check_volume},
	{"year",		4,	check_year},
	{(const char*)NULL,	0,	(void (*)(VOID))NULL},
    };

    static OPTION_FUNCTION_ENTRY fixes[] =
    {
	{"abstract",		8,	fix_math_spacing},
	{"annote",		6,	fix_math_spacing},
	{"author",		6,	fix_namelist},
	{"booktitle",		9,	fix_title},
	{"editor",		6,	fix_namelist},
	{"month",		5,	fix_month},
	{"note",		4,	fix_math_spacing},
	{"pages",		5,	fix_pages},
	{"remark",		6,	fix_math_spacing},
	{"title",		5,	fix_title},
	{(const char*)NULL,	0,	(void (*)(VOID))NULL},
    };

    trim_value();

    if ((in_preamble == NO) && (in_string == NO))
    {
	YESorNO save_brace_math;

	/*
	** None of the keywords in fixes[] at bibclean-2.16 requires
	** outer brace protection for math text, so we suppress that
	** feature.  If new entries are later added to fixes[], then
	** it may be necessary to revise this code to set brace_math
	** according to the value of current_field.
	*/

	save_brace_math = brace_math;
	brace_math = NO;
	(void)apply_function(current_field,fixes);
	brace_math = save_brace_math;

	if ((check_values == YES) && !STREQUAL(current_value,"\"\""))
	{
	    if (apply_function(current_field,checks) == NO)
		check_other();
	}
	if ((remove_OPT_prefixes == YES) &&
	    (strncmp(current_field,"OPT",3) == 0) &&
	    (strlen(current_field) > (size_t)3) &&
	    (strlen(current_value) > (size_t)2)) /* 2, not 0: quotes are included! */
	{
	    out_c(DELETE_LINE);
	    Memmove(current_field,&current_field[3],
		    (size_t)(strlen(current_field)-3+1));
				/* reduce "OPTname" to "name" */
	    out_field();
	    out_equals();
	    out_spaces((int)(value_indentation -
			     the_file.output.column_position));
	}
	else if ((delete_empty_values == YES) &&
		 STREQUAL(current_value,"\"\""))
	{			/* 2, not 0, because quotes are included! */
	    out_c(DELETE_LINE);
	    discard_next_comma = YES;
	    return;
	}
    }

    out_complex_value();
}


#if defined(HAVE_STDC)
static void
prt_pattern(const char *fieldname, /*@null@*/ const char *pattern, /*@null@*/ const char *message)
#else /* K&R style */
static void
prt_pattern(fieldname,pattern,message)
const char *fieldname;
const char *pattern;
const char *message;
#endif
{
    if (print_patterns == YES)
    {
	if ((pattern == (const char*)NULL) || (*pattern == '\0'))
	    (void)fprintf(stdlog,
		"\nfile=[%s] field=[%-12s] existing patterns discarded\n\n",
		initialization_file_name, fieldname);
	else if (message == (char*)NULL)
	    (void)fprintf(stdlog,
		"file=[%s] field=[%-12s] pattern=[%s]\n",
		initialization_file_name, fieldname, pattern);
	else
	    (void)fprintf(stdlog,
		"file=[%s] field=[%-12s] pattern=[%s] message[%s]\n",
		initialization_file_name, fieldname, pattern, message);
    }
}


#if defined(HAVE_STDC)
static void
put_back_string(const char *s)	/* put string value back onto input stream */
#else /* K&R style */
static void
put_back_string(s)	/* put string value back onto input stream */
const char *s;
#endif
{
    const char *p;

    for (p = strchr(s,'\0') - 1; p >= s; p--)
	put_back((int)*p);
}


static void
trim_value(VOID)
{		/* trim leading and trailing space from current_value[] */
    size_t k;
    size_t n = strlen(current_value);

    if ((current_value[0] == '"') && Isspace(current_value[1]))
    {		/* then quoted string value with leading space*/
	for (k = 1; (k < n) && Isspace(current_value[k]); ++k)
	    continue;
	Memmove(&current_value[1], &current_value[k], (size_t)(n + 1 - k));
				/* copy includes trailing NULL */
	n = strlen(current_value);
    }
    if (current_value[n-1] == '"')
    {
	for (k = n; (k > 1) && Isspace(current_value[k-2]); --k)
	    continue;
	if (current_value[k-2] == '\\')	/* maybe have \<space> or \\<space> */
	{
	    if ((k > 2) && (current_value[k-3] != '\\'))
		--k;			/* discard final \<space> */
	}
	current_value[k-1] = (char)'"';
	current_value[k] = (char)'\0';
    }
}
