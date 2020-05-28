/***********************************************************************
  @C-file{
    author              = "Nelson H. F. Beebe",
    version             = "3.03",
    date                = "11 March 2018",
    time                = "12:14:56 MDT",
    filename            = "bibclean.c",
    address             = "University of Utah
			   Department of Mathematics, 110 LCB
			   155 S 1400 E RM 233
			   Salt Lake City, UT 84112-0090
			   USA",
    telephone           = "+1 801 581 5254",
    FAX                 = "+1 801 581 4148",
    URL                 = "http://www.math.utah.edu/~beebe",
    checksum            = "56265 2241 7416 57389",
    email               = "beebe@math.utah.edu, beebe@acm.org,
			   beebe@computer.org (Internet)",
    codetable           = "ISO/ASCII",
    keywords            = "prettyprint, bibliography",
    license             = "GNU General Public License, version 2 or
                           later",
    supported           = "yes",
    docstring           = {Prettyprint one or more BibTeX files on stdin,
			   or specified files, to stdout, and check
			   the brace balance and value strings as well.

			   Text outside @item-type{...} BibTeX entries
			   is passed through verbatim, except that
			   trailing blanks are trimmed.

			   BibTeX items are formatted into a consistent
			   structure with one field = "value" pair per
			   line, and the initial @ and trailing right
			   brace in column 1.  Long values are split at a
			   blank and continued onto the next line with
			   leading indentation.  Tabs are expanded into
			   blank strings; their use is discouraged
			   because they inhibit portability, and can
			   suffer corruption in electronic mail.  Braced
			   strings are converted to quoted strings.

			   This format facilitates the later application
			   of simple filters to process the text for
			   extraction of items, and also is the one
			   expected by the GNU Emacs BibTeX support
			   functions.

			   Usage:

			   bibclean [ -author ]
				    [ -copyleft ]
				    [ -copyright ]
				    [ -error-log filename ]
				    [ -help ]
				    [ '-?' ]
				    [ -init-file filename ]
				    [ -ISBN-file filename ]
				    [ -keyword-file filename ]
				    [ -max-width # ]
				    [ -[no-]align-equals ]
				    [ -[no-]check-values ]
				    [ -[no-]debug-match-failures]
				    [ -[no-]delete-empty-values ]
				    [ -[no-]file-position ]
				    [ -[no-]fix-font-changes ]
				    [ -[no-]fix-initials ]
				    [ -[no-]fix-math ]
				    [ -[no-]fix-names ]
				    [ -[no-]German-style ]
				    [ -[no-]keep-linebreaks]
				    [ -[no-]keep-parbreaks]
				    [ -[no-]keep-preamble-spaces]
				    [ -[no-]keep-spaces]
				    [ -[no-]keep-string-spaces]
				    [ -[no-]parbreaks ]
				    [ -[no-]prettyprint ]
				    [ -[no-]print-ISBN-table ]
				    [ -[no-]print-keyword-table ]
				    [ -[no-]print-patterns ]
				    [ -[no-]read-init-files ]
				    [ -[no-]remove-OPT-prefixes ]
				    [ -[no-]scribe ]
				    [ -[no-]trace-file-opening ]
				    [ -version ]
				    [ -[no-]warnings ]
				    <infile or  bibfile1 bibfile2 bibfile3 ...
				    >outfile

			   Letter case is NOT significant in command-line
			   option names.

			   The checksum field above contains a CRC-16
			   checksum as the first value, followed by the
			   equivalent of the standard UNIX wc (word
			   count) utility output of lines, words, and
			   characters.  This is produced by Robert
			   Solovay's checksum utility.},
       }
***********************************************************************/

/***********************************************************************

The formatting should perhaps be user-customizable; that is left for
future work.

The major goal has been to convert entries to the standard form

@item-type{citation-key,
  field =	  "value",
  field =	  "value",
  ...
}

while applying heuristics to permit early error detection.  If
the input file is syntactically correct for BibTeX and LaTeX,
this is reasonably easy.  If the file has errors, error recovery
is attempted, but cannot be guaranteed to be successful; however,
the output file, and stderr, will contain an error message that
should localize the error to a single entry where a human can
find it more easily than a computer can.  To facilitate error
checking and recovery, the following conditions are used:

	@	starts a BibTeX entry only if it occurs at brace
		level 0 and is not preceded by non-blank text on
		the same line.
	"	is significant only at brace level 1.
	{}	are expected to occur at @-level 1 or higher
	}	at beginning of line ends a BibTeX entry

Backslashes preceding these 4 characters remove their special
significance.

These heuristics are needed to deal with legal value strings like

	{..."...}
	"...{..}..."

and will flag as errors strings like

	"...{..."
	"...}..."

The special treatment of @ and } at beginning of line attempts to
detect errors in entries before the rest of the file is swallowed
up in an attempt to complete an unclosed entry.

The output bibliography file should be processed by BibTeX and
the LaTeX without errors before discarding the original
bibliography file.

We do our own output and line buffering here so as to be able to
trim trailing blanks, and output data in rather large blocks for
efficiency (in filters of this type, I/O accounts for the bulk of
the processing, so large output buffers offer significant
performance gains).

The -scribe option enables recognition of the extended syntax used by
the Scribe document formatting system, originally developed by Brian
Reid at Carnegie-Mellon University, and now marketed by Unilogic, Ltd.
I have followed the syntax description in the Scribe Introductory
User's Manual, 3rd Edition, May 1980.

Scribe extensions include these features:

(1) Letter case is not significant in field names and entry names, but
case is preserved in value strings.

(2) In field/value pairs, the field and value may be separated by one of
three characters: =, /, or space.  Space may optionally surround these
separators.

(3) Value delimiters are any of these seven pairs:
{ }   [ ]   ( )	  < >	' '   " "   ` `

(4) Value delimiters may not be nested, even when with the first four
delimiter pairs, nested balanced delimiters would be unambiguous.

(5) Delimiters can be omitted around values that contain only letters,
digits, sharp (#), ampersand (&), period (.), and percent (%).

(6) A literal at-sign (@) is represented by doubled at-signs (@@).

(7) Bibliography entries begin with @name, as for BibTeX, but any of
the seven Scribe value delimiters may be used to surround the
field/value pairs.  As in (4), nested delimiters are forbidden.

(8) Arbitrary space may separate entry names from the following
delimiters.

(9) @Comment is a special command whose delimited value is discared.
As in (4), nested delimiters are forbidden.

(10) The special form

@Begin{comment}
...
@End{comment}

permits encapsulating arbitrary text containing any characters or
delimiters, other than "@End{comment}".	 Any of the seven delimiters
may be used around the word comment following the @begin or @end.

(11) The "key" field name is required in each bibliography entry.

(12) Semicolons may be used in place of "and" in author lists
(undocumented, but observed in practice).

Because of this loose syntax, error detection heuristics are less
effective, and consequently, Scribe mode input is not the default; it
must be explicitly requested.

***********************************************************************/

/***********************************************************************
We want this code to be compilable with C++ compilers as well as C
compilers, in order to get better compile-time checking.  We therefore
must declare all function headers in both old Kernighan-and-Ritchie
style, as well as in new Standard C and C++ style.  Although Standard C
also allows K&R style, C++ does not.

For functions with no argument, we just use VOID which expands to either
void, or nothing.

Older C++ compilers predefined the symbol c_plusplus, but that was
changed to __cplusplus in 1989 to conform to ISO/ANSI Standard C
conventions; we allow either.

It is regrettable that the C preprocessor language is not powerful
enough to transparently handle the generation of either style of
function declaration.
***********************************************************************/

#include <config.h>
#include "xtypes.h"
#include "xstdbool.h"
#include "xstdlib.h"
#include "xstring.h"
#include "xctype.h"
#include "xlimits.h"
#include "xstat.h"
#include "xunistd.h"

RCSID("$Id: bibclean.c,v 1.21 2014/04/03 18:00:15 beebe Exp beebe $")

#include "ch.h"
#include "delete.h"
#include "isbn.h"
#include "keybrd.h"
#include "yesorno.h"
#include "match.h"			/* must come AFTER yesorno.h */
#include "typedefs.h"			/* must come AFTER match.h */

#if defined(sun386)
/* Sun386i run-time library bug in fputs(): only first line in s is written! */
#define fputs(s,fp) fwrite(s,1,strlen(s),fp)
#endif

#if !defined(BIBCLEAN_INI)
#define BIBCLEAN_INI		"BIBCLEANINI"	/* environment variable */
#endif

#if !defined(BIBCLEAN_ISBN)
#define BIBCLEAN_ISBN		"BIBCLEANISBN"	/* environment variable */
#endif

#if !defined(BIBCLEAN_KEY)
#define BIBCLEAN_KEY		"BIBCLEANKEY"	/* environment variable */
#endif

#define EMPTY_STRING(s)	(s[0] = (char)'\0', s)
				/* for return (EMPTY_STRING(foo))*/


#if !defined(EXIT_FAILURE)
#define EXIT_FAILURE 1
#endif

#if !defined(EXIT_SUCCESS)
#define EXIT_SUCCESS 0
#endif

#ifdef FOPEN
#undef FOPEN
#endif

#if defined(__SUNCC__)
#define FOPEN(a,b) fopen((char*)(a),(char*)(b))
	/* bug workaround: wrong type for fopen() args with SunOS 4.1.2 CC */
#else /* NOT defined(__SUNCC__) */
#define FOPEN(a,b) fopen((a),(b))
#endif /* defined(__SUNCC__) */

#define GETDEFAULT(envname,default) \
	((getenv(envname) != (char *)NULL) ? getenv(envname) : default)

#define FIELD_INDENTATION 2	/* how far to indent "field = value," pairs */

int field_indentation = FIELD_INDENTATION;

#define KEEP_PREAMBLE_SPACES()	((in_preamble == YES) && \
				 (keep_preamble_spaces == YES))

#define KEEP_STRING_SPACES()	((in_string == YES) && \
				 (keep_string_spaces == YES))

#define LAST_SCREEN_LINE (-2)	/* used in opt_help() and do_more() */

#if defined(MAX)
#undef MAX
#endif

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))

#if !defined(MAX_BUFFER)
#define MAX_BUFFER	8192	/* output buffer size; this does NOT */
				/* limit lengths of input lines */
#endif /* !defined(MAX_BUFFER) */

#if !defined(MAX_WIDTH)
#define MAX_WIDTH	72L	/* length of longest entry line; */
				/* non-BibTeX entry text is output verbatim */
#endif /* !defined(MAX_WIDTH) */

#if !defined(MAX_FIELD_LENGTH)
#define MAX_FIELD_LENGTH 12	/* "howpublished" */
#endif /* !defined(MAX_FIELD_LENGTH) */

#include "pattern.h"

#include "token.h"

#if !defined(SCREEN_LINES)
#if OS_PCDOS
#define SCREEN_LINES	25	/* set 0 to disable pausing in out_lines() */
#else /* NOT OS_PCDOS */
#define SCREEN_LINES	24	/* set 0 to disable pausing in out_lines() */
#endif /* OS_PCDOS */
#endif /* !defined(SCREEN_LINES) */

#define VALUE_INDENTATION	(FIELD_INDENTATION + MAX_FIELD_LENGTH + 3)
				/* where item values are output; allow space */
				/* for "<field indent><field name>< = >" */

int value_indentation = VALUE_INDENTATION;


/* Operating system-specific customizations. */

#if OS_UNIX

#if !defined(INITDIR)
#define INITDIR		("/usr/local/share/" PACKAGE_NAME "/" PACKAGE_VERSION)
#endif

#if !defined(INITFILE)
#define INITFILE	(PACKAGE_NAME ".ini")
#endif

#if !defined(ISBNFILE)
#define ISBNFILE	(PACKAGE_NAME ".isbn")
#endif

#if !defined(KEYWORDFILE)
#define KEYWORDFILE	(PACKAGE_NAME ".key")
#endif

#if !defined(USERPATH)
#define USERPATH	"BIBINPUTS"
#endif

#endif /* OS_UNIX */

#if OS_VAXVMS
#if !defined(INITFILE)
#define INITFILE	(PACKAGE_NAME ".ini")
#endif

#if !defined(USERPATH)
#define USERPATH	"BIBINPUTS"
#endif

#endif /* OS_VAXVMS */

/* For any that are undefined, default to values suitable for OS_PCDOS. */
#if !defined(INITFILE)
#define INITFILE	(PACKAGE_NAME ".ini")
#endif

#if !defined(ISBNFILE)
#define ISBNFILE	(PACKAGE_NAME ".isb")
#endif

#if !defined(KEYWORDFILE)
#define KEYWORDFILE	(PACKAGE_NAME ".key")
#endif

#if !defined(USERPATH)
#define USERPATH	"BIBINPUTS"
#endif

#include "toklst.h"

static const char *type_name[] =
{				/* must be indexable by TOKEN_xxx */
    "UNKNOWN",
    "ABBREV",			/* alphabetical order, starting at 1 */
    "AT",
    "COMMA",
    "COMMENT",
    "ENTRY",
    "EQUALS",
    "FIELD",
    "INCLUDE",
    "INLINE",
    "KEY",
    "LBRACE",
    "LITERAL",
    "NEWLINE",
    "PREAMBLE",
    "RBRACE",
    "SHARP",
    "SPACE",
    "STRING",
    "VALUE",
};

/* Almost all functions except main() are static to overcome
limitations on external name lengths in ISO/ANSI Standard C.  Please
keep them in ALPHABETICAL order, ignoring letter case. */

void		do_initfile ARGS((/*@null@*/ const char *pathlist_, /*@null@*/ const char *name_));
YESorNO		apply_function ARGS((const char *option_,
		    OPTION_FUNCTION_ENTRY table_[]));
void		check_inodes ARGS((void));
void		error ARGS((const char *msg_));
/*@noreturn@*/ void	fatal ARGS((const char *msg_));
void		free_pattern_table_entries ARGS((PATTERN_TABLE *pt_));
int		get_char ARGS((void));
int		get_linebreak ARGS((void));
int		get_next_non_blank ARGS((void));
bool		is_idchar ARGS((int c_));
int		main ARGS((int argc_, char *argv_[]));
void		out_at ARGS((void));
void		out_c ARGS((int c_));
void		out_flush ARGS((void));
void		out_lines ARGS((FILE *fpout_,const char *lines_[],
		    YESorNO pausing_));
void		out_newline ARGS((void));
void		out_s ARGS((const char *s_));
void		out_spaces ARGS((int n_));
void		out_string ARGS((token_t type_, const char *token_));
void		out_token ARGS((token_t type_, const char *token_));
void		out_with_error ARGS((const char *s_,const char *msg_));
void		out_with_parbreak_error ARGS((char *s_));
void		put_back ARGS((int c_));
/*@null@*/ FILE	*tfopen ARGS((const char *filename_, const char *mode_));
void		warning ARGS((const char *msg_));

extern void	do_args ARGS((int argc_, char *argv_[]));
extern void	do_files ARGS((int argc_, char *argv_[]));
extern void	do_keyword_file ARGS((/*@null@*/ const char *pathlist_, /*@null@*/ const char *name_));
extern void	do_other ARGS((void));
extern void	do_preargs ARGS((int argc_, char *argv_[]));
extern void	do_print_keyword_table ARGS((void));
extern void	finish ARGS((int status));
extern void	free_keyword_table ARGS((void));
extern void	keyword_initialize ARGS((void));

static long	bcolumn ARGS((void));
static void	bdelc ARGS((void));
static void	bdelline ARGS((void));
static void	bflush ARGS((void));
static int	blastc ARGS((void));
static int	bpeekc ARGS((int c_));
static void	bputc ARGS((int c_));
static char	*format ARGS((const char *msg_));
static void	free_match_pattern ARGS((MATCH_PATTERN *mp_));
static void	free_pattern_table ARGS((PATTERN_TABLE *pt_));
static void	free_tables ARGS((void));
static void	init_tables ARGS((void));

#if (defined(HAVE_REGEXP) || defined(HAVE_RECOMP))
static int	match_regexp  ARGS((const char *string_,const char *pattern_));
#endif /* (defined(HAVE_REGEXP) || defined(HAVE_RECOMP)) */

static void	out_error ARGS((FILE *fpout_, const char *s_));
static void	out_input_position ARGS((IO_PAIR *pair_));
static void	out_number ARGS((long n_));
static void	out_position ARGS((FILE *fpout_,const char *msg_,
		    IO_PAIR *the_location_));
static void	out_status ARGS((FILE *fpout_, const char *prefix_));
static void	out_verbatim ARGS((const char *token_));
static void	resync ARGS((void));
extern char	*Strdup ARGS((const char *s_));
static size_t	word_length ARGS((const char *s_));
static void	wrap_line ARGS((void));


/**********************************************************************/

/* Please keep these (mostly) global variables in ALPHABETICAL order,
ignoring letter case. */

YESorNO		align_equals = NO;		/* NO: left-adjust equals */
int		at_level = 0;			/* @ nesting level */
int		brace_level = 0;		/* curly brace nesting level */
YESorNO		brace_math = YES;		/* NO: leave mixed-case math text untouched */
YESorNO		brace_protect = YES;		/* NO: leave mixed-case title words untouched */
static size_t	buf_length = 0;			/* length of buf[] */
static char	buf[MAX_BUFFER+1];		/* 1 extra slot for trailing NUL */
YESorNO		check_values = YES;		/* NO: suppress value checks */
int		close_char = EOF;		/* BibTeX entry closing; may */
						/* be right paren or brace */
char		current_entry_name[MAX_TOKEN_SIZE]; /* entry name */
char		current_field[MAX_TOKEN_SIZE];	/* field name */
char		current_key[MAX_TOKEN_SIZE]; 	/* citation key */
char		current_value[MAX_TOKEN_SIZE];	/* string value */
YESorNO		delete_empty_values = NO; 	/* YES: delete empty values */
YESorNO		discard_next_comma = NO; 	/* YES: deleting field/value */
YESorNO		eofile = NO;			/* set to YES at end-of-file */
static int	error_count = 0;		/* used to decide exit code */
						/* normalizing names */
YESorNO		fix_accents = NO;		/* repair accent bracing? */
YESorNO		fix_braces = NO;		/* normalize bracing? */
YESorNO		fix_degrees = NO;		/* normalize author academic degrees? */
YESorNO		fix_font_changes = NO;		/* brace {\em E. Coli}? */
YESorNO		fix_initials = YES;		/* reformat A.U. Thor?	*/
YESorNO		fix_math = NO;			/* reformat math mode? */
YESorNO		fix_names = YES;		/* reformat Bach, P.D.Q? */

#if defined(DEBUG)
static FILE	*fpdebug;			/* for debugging */
#endif /* defined(DEBUG) */

FILE		*fpin;				/* input file pointer */

char		*initialization_file_name;
YESorNO		in_preamble = NO;		/* YES: in @Preamble{...} */
YESorNO		in_string = NO;			/* YES: in @String{...} */
YESorNO		in_value = NO;			/* YES: in value string */
YESorNO		is_parbreak = NO;		/* get_next_non_blank() sets */
YESorNO		keep_linebreaks = NO; 	/* YES: keep linebreaks in values */
YESorNO		keep_parbreaks = NO;	/* YES: keep parbreaks in values */
YESorNO		keep_preamble_spaces = NO; /* YES: keep spaces in @Preamble{} */
YESorNO		keep_spaces = NO;	/* YES: keep spaces in values */
YESorNO		keep_string_spaces = NO; /* YES: keep spaces in @String{} */

long		max_width;

int		non_white_chars = 0;		/* used to test for legal @ */
static IO_PAIR	original_file;			/* used in error messages */
YESorNO		parbreaks = YES;		/* NO: parbreaks forbidden */
						/* in strings and entries */
YESorNO		prettyprint = YES;		/* NO: do lexical analysis */
YESorNO		print_ISBN_table = NO;		/* YES: print ISBN table */
YESorNO		print_keyword_table = NO;	/* YES: print keyword table */
YESorNO		print_patterns = NO;		/* YES: print value patterns */
char		*program_name;			/* set to argv[0] */


PATTERN_NAMES pattern_names[MAX_PATTERN_NAMES] =
{
    {"chapter",		(PATTERN_TABLE*)NULL},
    {"month",		(PATTERN_TABLE*)NULL},
    {"number",		(PATTERN_TABLE*)NULL},
    {"pages",		(PATTERN_TABLE*)NULL},
    {"volume",		(PATTERN_TABLE*)NULL},
    {"year",		(PATTERN_TABLE*)NULL},

#if _AIX370
    {NULL,		NULL},	/* CC compiler cannot handle correct cast */
#else /* NOT _AIX370 */
    {(CONST char*)NULL,	(PATTERN_TABLE*)NULL}, /* entry terminator */
#endif /* _AIX370 */

    /* remaining slots may be initialized at run time */
};

#define MAX_PUSHBACK	10
static int	n_pushback = 0;
static int	pushback_buffer[MAX_PUSHBACK];
YESorNO		read_initialization_files = YES;/* -[no-]read-init-files sets */
YESorNO		remove_OPT_prefixes = NO; 	/* YES: remove OPT prefix */
YESorNO		rflag = NO;			/* YES: resynchronizing */
int		screen_lines = SCREEN_LINES;/* kbopen() and out_lines() reset */
YESorNO		Scribe = NO;			/* Scribe format input */

/* In all memory models from tiny to huge, Turbo C on IBM PC DOS will
not permit more than 64KB of global constant data.  Therefore, we use a
global scratch array shared between the functions fix_title(),
format(), get_Scribe_identifier_string() and
get_Scribe_delimited_string().  The code has been carefully examined
to make sure that this space is not overwritten while still in use.
Oh, the pain of the Intel segmented memory architecture! */

char		shared_string[MAX_TOKEN_SIZE];

YESorNO		show_file_position = NO; 	/* messages usually brief */
FILE		*stdlog;			/* usually stderr */
YESorNO		stdlog_on_stdout = YES;		/* NO for separate files */

#if OS_PCDOS
unsigned int _stklen = 0xF000;			/* stack size for Turbo C */
#endif /* OS_PCDOS */

IO_PAIR		token_start;			/* used for # line output */
IO_PAIR		the_entry;			/* used in error messages */
IO_PAIR		the_file;			/* used in error messages */
IO_PAIR		the_value;			/* used in error messages */

YESorNO		trace_file_opening = NO; /* -[no-]trace-file-opening sets */
YESorNO		warnings = YES;			/* NO: suppress warnings */
YESorNO		wrapping = YES;			/* NO: verbatim output */

/**********************************************************************/


#if defined(HAVE_STDC)
YESorNO
apply_function(const char *option, OPTION_FUNCTION_ENTRY table[])
#else /* K&R style */
YESorNO
apply_function(option,table)
const char *option;
OPTION_FUNCTION_ENTRY table[];
#endif
{	/* return YES if function matching option was invoked, otherwise NO */
    int k;			/* index into table[] */
    size_t n;

    n = strlen(option);		/* all chars of option[] will be examined */

    for (k = 0; table[k].name != (const char*)NULL; ++k)
    {
	if (strnicmp(option,table[k].name,MAX(n,table[k].min_match)) == 0)
	{
	    table[k].function();
	    return (YES);
	}
    }
    return (NO);
}


/* Version 2.10: the new functions bcolumn(), bdelc(), bdelline(),
bflush(), blastc(), bpeekc(), and bputc() provide a clean, simple, and
sole interface to the output buffer, buf[].  Code evolution prior to
version 2.10 had made out_c() too complex, and these routines
simplified it substantially.  We no longer treat horizontal tab
specially: all characters except newline increment the column position
by 1, because doing otherwise complicates updating the column position
on a DELETE_CHAR operation. */

static long
bcolumn(VOID)				/* return output column position */
{
    return (the_file.output.column_position);
}


static void
bdelc(VOID)				/* delete last character in buf[] */
{
    if (buf_length > 0)
    {
	the_file.output.byte_position--;
	switch (buf[--buf_length])
	{
	case '\n':
	    the_file.output.column_position =
		the_file.output.last_column_position;
	    break;
	default:
	    the_file.output.column_position--;
	    break;
	}
    }
}


static void
bdelline(VOID)				/* delete last line in buf[] */
{
    while ((buf_length > 0) && (buf[buf_length-1] != '\n'))
	bdelc();
}


static void
bflush(VOID)				/* output all but last line in buf[] */
{
    size_t k;

    /* Output all but the last line of buf[], unless there is only one
       line, in which case, output all of buf[].  We need to keep a
       complete line available in case of a DELETE_LINE operation. */

    buf[buf_length] = '\0';		/* terminate buffer string */
    for (k = buf_length; (k > 0) && (buf[k-1] != '\n'); --k)
	continue;			/* find preceding newline */
    if (buf[k] == '\n')			/* then found last line */
    {
	buf[k] = (char)'\0';		/* zap the newline */
	(void)fputs(buf,stdout);	/* output up to the zapped newline */
	(void)fputc('\n',stdout);	/* output the zapped newline */
	buf_length = strlen(&buf[k+1]);
	(void)Memmove(buf,&buf[k+1],buf_length);
					/* move last line to start of buf[] */
					/* NB: overlap requires Memmove(), */
					/* NOT strcpy()! */
    }
    else if (buf_length > 0)		/* have only one line in buf[] */
    {
	(void)fputs(buf,stdout);
	buf_length = 0;
    }
    (void)fflush(stdout);		/* synchronize for error messages */
}


static int
blastc(VOID)			/* return last character in buffer, or EOF */
{
    return (buf_length > 0) ? (int)buf[buf_length-1] : EOF;
}


#if defined(HAVE_STDC)
static int				/* return char at (negative) offset */
bpeekc(int offset)			/* from end of buf[], or EOF */
#else /* K&R style */
static int
bpeekc(offset)
int offset;
#endif
{
    int k;

    k = (int)buf_length - 1 + offset;
    return ((0 <= k) && (k < (int)buf_length)) ? (int)buf[k] : EOF;
}


#if defined(HAVE_STDC)
static void
bputc(int c)				/* output c to buf[] */
#else /* K&R style */
static void
bputc(c)
int c;
#endif
{
    if (buf_length >= MAX_BUFFER)
	bflush();
    switch (c)
    {
    case '\n':
	the_file.output.column_position = 0;
	the_file.output.line_number++;
	break;

    default:
	the_file.output.column_position++;
	break;
    }
    the_file.output.byte_position++;
    buf[buf_length++] = (char)c;
}


#if defined(HAVE_STDC)
void			/* issue an error message */
error(const char *msg)		/* default provided if this is NULL */
#else /* K&R style */
void
error(msg)			/* issue an error message */
const char *msg;		/* default provided if this is NULL */
#endif
{
    char *p;

    error_count++;
    out_flush();		/* flush all buffered output */
    at_level = 0;		/* suppress further messages */
				/* until we have resynchronized */
    original_file = the_file;	/* save for error messages */

    p = format(msg);
    (void)fprintf(stdlog,"%s \"%s\", line %ld: %s.\n",
	ERROR_PREFIX,
	the_file.input.filename, the_value.input.line_number, p);
				/* UNIX-style error message for */
				/* GNU Emacs M-x compile to parse */
    out_status(stdlog, ERROR_PREFIX);
    (void)fflush(stdlog);

    out_error(stdout, "\n"); /* make sure we start a newline */
    out_error(stdout, ERROR_PREFIX);
    out_error(stdout, " ");
    out_error(stdout, p);
    out_error(stdout, ".\n");
    out_status(stdout, ERROR_PREFIX);
    out_flush();		/* flush all buffered output */
}


/*@noreturn@*/
#if defined(HAVE_STDC)
void				/* issue an error message and die */
fatal(const char *msg)
#else /* K&R style */
void
fatal(msg)			/* issue an error message and die */
const char *msg;
#endif
{
    (void)fprintf(stdlog,"%s %s\n", ERROR_PREFIX, msg);
    exit(EXIT_FAILURE);
}


#if defined(HAVE_STDC)
void
finish(int status)
#else /* K&R style */
void
finish(status)
int status;
#endif
{
    if (status == EXIT_SUCCESS)
	free_tables();

    exit(status);
}


#if defined(HAVE_STDC)
static char *
format(const char *msg)
#else /* K&R style */
static char *
format(msg)
const char *msg;
#endif
{   /* expand %f, %k, %v, and %% items in msg[], return pointer to new copy */
    size_t k;
    size_t len;
    size_t n;
    static char newmsg[MAX_TOKEN_SIZE];	/* static because we return it */

    /* Shorthand for writable copy of msg[] with guaranteed NUL termination */
#define ORIGINAL_MESSAGE (strncpy(newmsg,msg,MAX_TOKEN_SIZE), \
	newmsg[MAX_TOKEN_SIZE-1] = (char)'\0', newmsg)

    for (k = 0, n = 0; msg[k] != '\0'; ++k)
    {
	switch (msg[k])
	{
	case '%':			/* expect valid format item */
	    switch (msg[++k])
	    {
	    case 'e':			/* %e -> current_entry_name */
		len = strlen(current_entry_name);
		if ((n + len) >= MAX_TOKEN)
		    return (ORIGINAL_MESSAGE); /* no space left*/
		(void)strcpy(&newmsg[n],current_entry_name);
		n += len;
		break;

	    case 'f':			/* %f -> current_field */
		len = strlen(current_field);
		if ((n + len) >= MAX_TOKEN)
		    return (ORIGINAL_MESSAGE); /* no space left*/
		(void)strcpy(&newmsg[n],current_field);
		n += len;
		break;

	    case 'k':			/* %k -> current_key */
		len = strlen(current_key);
		if ((n + len) >= MAX_TOKEN)
		    return (ORIGINAL_MESSAGE); /* no space left*/
		(void)strcpy(&newmsg[n],current_key);
		n += len;
		break;

	    case 'v':			/* %v -> current_value */
		len = strlen(current_value);
		if ((n + len) >= MAX_TOKEN)
		    return (ORIGINAL_MESSAGE); /* no space left*/
		(void)strcpy(&newmsg[n],current_value);
		n += len;
		break;

	    case '%':			/* %% -> % */
		newmsg[n++] = (char)'%';
		break;

	    default:
		return (ORIGINAL_MESSAGE); /* no space left*/
	    }
	    break;

	default:
	    if (n >= MAX_TOKEN)
		return (ORIGINAL_MESSAGE); /* no space left*/
	    newmsg[n++] = msg[k];
	    break;
	}
    }
    newmsg[n] = (char)'\0';			/* terminate string */
    return (newmsg);
}


#if defined(HAVE_STDC)
static void
free_match_pattern(MATCH_PATTERN *mp)
#else /* K&R style */
static void
free_match_pattern(mp)
MATCH_PATTERN *mp;
#endif
{
    if (mp->pattern != (const char*)NULL)
    {					/* NB: (void*) cast fails with Sun C++ */
	FREE(mp->pattern);
	mp->pattern = (const char*)NULL;
    }
    if (mp->message != (const char*)NULL)
    {
	FREE(mp->message);
	mp->message = (const char*)NULL;
    }
}


#if defined(HAVE_STDC)
static void
free_pattern_table(PATTERN_TABLE *pt)
#else /* K&R style */
static void
free_pattern_table(pt)
PATTERN_TABLE *pt;
#endif
{
    if (pt != (PATTERN_TABLE*)NULL)
    {
	free_pattern_table_entries(pt);
	FREE(pt);
    }
}


#if defined(HAVE_STDC)
void
free_pattern_table_entries(PATTERN_TABLE *pt)
#else /* K&R style */
void
free_pattern_table_entries(pt)
PATTERN_TABLE *pt;
#endif
{
    if (pt != (PATTERN_TABLE*)NULL)
    {
	int k;

	for (k = 0; k < pt->current_size; ++k)
	    free_match_pattern(&(pt->patterns[k]));

	if (pt->patterns != (MATCH_PATTERN*)NULL)
	{
	    FREE(pt->patterns);
	    pt->patterns = (MATCH_PATTERN*)NULL;
	}

	pt->current_size = 0;
	pt->maximum_size = 0;
    }
}


static void
free_tables(VOID)
{
    int k;				/* index into pattern_names[] */

    for (k = 0; pattern_names[k].name != (const char*)NULL; ++k)
    {					/* free dynamic part */
	free_pattern_table(pattern_names[k].table);
	FREE(pattern_names[k].name);
	pattern_names[k].name = (const char*)NULL;
	pattern_names[k].table = (PATTERN_TABLE *)NULL;
    }

    free_keyword_table();
    free_ISBN_table();
}


int
get_char(VOID)			/* all input is read through this function */
{
    int c;

    /* NB: this is the ONLY place where the input file is read! */
    c =  (n_pushback > 0) ? pushback_buffer[--n_pushback] : getc(fpin);

    the_file.input.byte_position++;

    /* Adjust global status and position values */

    if (c == EOF)
	eofile = YES;
    else if (c == (int)'\n')
    {
	the_file.input.line_number++;
	the_file.input.last_column_position = the_file.input.column_position;
	the_file.input.column_position = 0L;
	non_white_chars = 0;
    }
    else if (!Isspace(c))
    {
	the_file.input.last_column_position = the_file.input.column_position;
	the_file.input.column_position++;
	non_white_chars++;
    }
    else if (c == (int)'\t')
    {
	the_file.input.last_column_position = the_file.input.column_position;
	the_file.input.column_position =
		(the_file.input.column_position + 8L) & ~07L;
    }
    else
    {
	the_file.input.last_column_position = the_file.input.column_position;
	the_file.input.column_position++;
    }

    if (c == (int)'{')
	brace_level++;
    else if (c == (int)'}')
	brace_level--;

#if defined(DEBUG)
    if (fpdebug)
	(void)fprintf(fpdebug,"[%c] %5ld %4ld %2ld\n",
	    c,
	    the_file.input.byte_position,
	    the_file.input.line_number,
	    the_file.input.column_position);
#endif /* defined(DEBUG) */

    return (c);
}


int
get_linebreak(VOID)
{
    int c;
    int ns = 0;

    while (((c = get_char(), c) != EOF) && Isspace(c))
    {
	switch (c)
	{
	case '\n':
	    return (LINEBREAK);

	case '\f':
	    return (PARBREAK);

	default:
	    ns++;
	    break;
	}
    }
    put_back(c);
    return ((ns > 0) ? (int)' ' : (int)'\0');
}


int
get_next_non_blank(VOID)
{
    int c;

    if (keep_spaces == YES)
	c = get_char();
    else if (KEEP_PREAMBLE_SPACES())
	c = get_char();
    else if (KEEP_STRING_SPACES())
	c = get_char();
    else
    {
	int ff = 0;
	int nl = 0;

	is_parbreak = NO;

	while (((c = get_char(), c) != EOF) && Isspace(c))
	{
	    switch (c)
	    {
	    case '\n':
		if ((in_value == YES) && (keep_linebreaks == YES))
		    return (LINEBREAK);

		nl++;
		break;

	    case '\f':
		if ((in_value == YES) && (keep_linebreaks == YES))
		    return (c);

		ff++;
		break;

	    default:
		break;
	    }
	}

	is_parbreak = ((nl > 1) || (ff > 0)) ? YES : NO;
    }

    return (c);
}


static void
init_tables(VOID)
{
    int k;

    /* Convert statically-allocated table data to dynamic data */
    for (k = 0; (k < MAX_PATTERN_NAMES) &&
		(pattern_names[k].name != (const char *)NULL); ++k)
    {
	pattern_names[k].name = (const char *)Strdup(pattern_names[k].name);
	pattern_names[k].table = (PATTERN_TABLE*)malloc(sizeof(PATTERN_TABLE));
	if (pattern_names[k].table == (PATTERN_TABLE*)NULL)
	    fatal("Out of memory for pattern tables");
	pattern_names[k].table->patterns = (MATCH_PATTERN*)NULL;
	pattern_names[k].table->current_size = 0;
	pattern_names[k].table->maximum_size = 0;
    }
}


#if defined(HAVE_STDC)
bool
is_idchar(int c)
#else /* K&R style */
bool
is_idchar(c)
int c;
#endif
{
    /*
    ** See LaTeX User's Guide and Reference Manual, Section B.1.3, for
    ** the rules of what characters can be used in a BibTeX word
    ** value.  Section 30 of BibTeX initializes id_class[] to match
    ** this, but curiously, allows ASCII DELete (0x3f), as an
    ** identifier character.  That irregularity was reported to Oren
    ** Patashnik on [06-Oct-1990].  We disallow it here.
    **
    ** The Scribe syntax is simpler: letters, digits, ., #, &, and %.
    */

    return ((Scribe == YES) ?
	    ((Isalnum(c) ||
	      (c == (int)'.') ||
	      (c == (int)'#') ||
	      (c == (int)'&') ||
	      (c == (int)'%') ) ? YES : NO) :
	    ((Isgraph(c) &&
	      !IN_SET("\"#%'(),={}", c)) ? YES : NO) );
}


#if defined(HAVE_STDC)
int
main(int argc, char *argv[])
#else /* K&R style */
int
main(argc,argv)
int argc;
char *argv[];
#endif
{
    const char *initfile;
    const char *ISBN_file;
    const char *keyword_file;
    const char *initdir;
    static char alt_initdir[MAXPATHLEN + 1] = { '\0' };

#if defined(vms)
    extern char **cmd_lin();

    argv = cmd_lin( "", &argc, argv );
#endif /* defined(vms) */

#if defined(HAVE_SYS_PLEDGE_H)
    /*
    ** NB: The prototype for pledge() comes from <unistd.h>, but to
    ** avoid possible conflicts with other O/Ses that might have
    ** declare that name for another purpose, the configure script
    ** actually checks for <sys/pledge.h>, which is more likely to be
    ** the characteristic signature that OpenBSD-style pledge() is
    ** supported.
    **
    ** The pledge() call, introduced in OpenBSD 5.9, and available in
    ** later OpenBSD releases, enumerates classes of operations that
    ** the program is permitted to do.  It can be issued more than
    ** once, but only to further reduce privileges.  Its use here is
    ** part of being a good software citizen on OpenBSD, and any other
    ** operating systems that adopt the pledge mechanism for improving
    ** system security.
    **
    ** The privileges required by bibclean are:
    **
    **     cpath	-- create new files
    **
    **     rpath 	-- read files and directories
    **
    **     stdio 	-- read and write stdio channels
    **
    **     tty		-- allow read/write/ioctl on /dev/tty
    **     		   (needed for --help option)
    **
    **     wpath	-- write files
    **
    */

    if (pledge("cpath rpath stdio tty wpath", NULL) != 0)
    {
	perror("pledge system call failed: perror says");
	exit(EXIT_FAILURE);
    }

#endif

    initfile = GETDEFAULT(BIBCLEAN_INI,INITFILE);
    ISBN_file = GETDEFAULT(BIBCLEAN_ISBN,ISBNFILE);
    keyword_file = GETDEFAULT(BIBCLEAN_KEY,KEYWORDFILE);

    max_width = 0L;			/* reset later */

    stdlog = stderr;	/* cannot assign at compile time on some systems */

    program_name = argv[0];

    initdir = INITDIR;

    if (alt_initdir[0] != '\0')		/* have value from previous call */
	initdir = (const char *)&alt_initdir[0];
    else if ( (program_name != (char *)NULL) && (program_name[0] != '\0') )
    {
	/*
	** If INITDIR does not exist, then convert a path to the
	** executable of, say, "/usr/local/bin/bibclean" to a
	** directory name "/usr/local/bin/", then append to it a
	** relative path to find the installation tree location,
	** "/usr/local/share/bibclean/VERSION"
	*/
	struct stat stat_buf;

	if ( (stat(initdir, &stat_buf) == 0) && (S_ISDIR(stat_buf.st_mode)) )
	{	    /* success: initdir exists */
	}
	else		/* replace likely-relocated initdir at run-time  */
	{
	    char * p;

	    (void)strncpy(alt_initdir, program_name, sizeof(alt_initdir));
	    p = strrchr(alt_initdir, '/');

	    if (p == (char *)NULL)
		alt_initdir[0] = '\0';
	    else
	    {
		*p = '\0';		/* terminate string at last directory */
		p = strrchr(alt_initdir, '/');

		if (p != (char *)NULL)
		{
		    p = strrchr(alt_initdir, '/');
		    if (p != (char *)NULL)
		    {
			const char *q;

			p[1] = '\0';	/* terminate string at second last directory */
			q = ("share/" PACKAGE_NAME "/" PACKAGE_VERSION "/");
			(void)strncat(alt_initdir, q, sizeof(alt_initdir) - strlen(q));
			initdir = &alt_initdir[0];

			if ( (stat(initdir, &stat_buf) == 0) && (S_ISDIR(stat_buf.st_mode)) )
			{
			    /* then relocated directory exists*/
			}
			else
			    (void)fprintf(stderr, "WARNING: relocated %s installation directory does not exist\n", initdir);
		    }
		}
	    }
	}
    }

    check_inodes();

#if defined(DEBUG)
    fpdebug = tfopen((PACKAGE_NAME ".dbg"), "w");
#endif /* defined(DEBUG) */

    the_file.input.filename = "";
    the_file.output.filename = "stdout";

    init_tables();

    do_preargs(argc,argv);/* some args must be handled BEFORE initializations */

    if (read_initialization_files == YES)
	do_initfile(initdir,initfile);

    if (read_initialization_files == YES)
	do_initfile(getenv(USERPATH),initfile);

    ISBN_initialize();
    keyword_initialize();

    if (read_initialization_files == YES)
    {
	do_ISBN_file(initdir,ISBN_file);
	do_ISBN_file(getenv(USERPATH),ISBN_file);

	do_keyword_file(initdir,keyword_file);
	do_keyword_file(getenv(USERPATH),keyword_file);
    }

    do_args(argc,argv);

    if (print_ISBN_table == YES)
    {
	do_print_ISBN_table();
	finish(EXIT_SUCCESS);
    }

    if (print_keyword_table == YES)
    {
	do_print_keyword_table();
	finish(EXIT_SUCCESS);
    }

    if (max_width == 0L)		/* set default value */
	max_width = (prettyprint == YES) ? MAX_WIDTH : LONG_MAX;

    do_files(argc,argv);

    finish((error_count > 0) ? EXIT_FAILURE : EXIT_SUCCESS);
}


#if defined(HAVE_STDC)
void
Memmove(void *target, const void *source, size_t n)
#else /* K&R style */
void
Memmove(target, source, n)
void *target;
const void *source;
size_t n;
#endif
{
    char *t;
    const char *s;

    t = (char *)target;
    s = (const char*)source;
    if ((s <= t) && (t < (s + n))) /* overlap: need right to left copy */
    {
	for (t = ((char *)target) + n - 1, s = ((const char*)source) + n - 1;
	     n > 0; --n)
	    *t-- = *s--;
    }
    else /* left to right copy is okay */
    {
	for ( ; n > 0; --n)
	    *t++ = *s++;
    }
}


#if defined(HAVE_STDC)
void*
Memset(void *target, int value, size_t n)
#else /* K&R style */
void*
Memset(target, value, n)
void *target;
int value;
size_t n;
#endif
{
    unsigned char *t = (unsigned char*)target;

    for ( ; n > 0; --n)
	*t++ = (unsigned char)value;

    return (target);
}


void
out_at(VOID)
{
    out_string(TOKEN_AT, "@");
}


#if defined(HAVE_STDC)
void				/* output c, but trim trailing blanks, */
out_c(int c)			/* and output buffer if c == EOF */
#else /* K&R style */
void
out_c(c)			/* output c, but trim trailing blanks, */
int c;				/* and output buffer if c == EOF */
#endif
{
    int offset;

    switch (c)
    {
    case EOF:
	bflush();			/* output all but last line of buf[], */
	break;				/* or all of it, if on last line */

    case DELETE_CHAR:			/* delete last character from buf[] */
	bdelc();
	break;

    case DELETE_LINE:			/* delete current line in buf[] */
	bdelline();
	break;

    case DELETE_WHITESPACE:		/* discarding trailing whitespace */
	for (c = blastc(); Isspace(c); c = blastc())
	    bdelc();

	/* If the buffer ends with an inline comment, then we just
	   deleted the newline that terminates it, so we must put it back */
	for (offset = 0; ((c = bpeekc(offset)) != EOF) && (c != (int)'\n');
	     offset--)
	{
	    if (c == BIBTEX_COMMENT_PREFIX)
	    {
		bputc((int)'\n');
		break;
	    }
	}
	break;

#if OS_PCDOS
    case '\r':
	/* Because we open the input file in binary mode to prevent
	   Ctl-Z causing an end-of-file, we see Ctl-M characters.  We
	   simply discard them here: they will be supplied by the
	   run-time library when the following newline is written,
	   since the output files are opened in text mode. */
	break;
#endif

    case '\n':
	for (c = blastc(); (c == (int)' ') || (c == (int)'\t'); c = blastc())
	    bdelc();			/* discard trailing horizontal space */
	bputc((int)'\n');
	break;

    default:		/* output ordinary character, with possible line wrap */
	if ((prettyprint == NO) && (c != (int)'\n'))
	{				/* need to line wrap? */
	    if (bcolumn() > (max_width - 2))
	    {				/* yes, output backslash-newline pair */
		the_file.input.last_column_position =
		    the_file.input.column_position;
		bputc((int)'\\');
		bputc((int)'\n');
	    }
	}
	bputc(c);
	break;
    } /* end switch (c) */
}


#if defined(HAVE_STDC)
static void
out_error(FILE *fpout, const char *s)
#else /* K&R style */
static void
out_error(fpout, s)
FILE *fpout;
const char *s;
#endif
{
    if (fpout == stdout)	/* private handling of stdout so we */
	out_s(s);		/* can track positions */
    else
	(void)fputs(s,fpout);
}


void
out_flush(VOID)			/* flush buffered output */
{
    out_c(EOF);			/* magic value to flush buffers */
    out_c(EOF);			/* NB: TWO EOFs needed to really flush: */
				/* see out_c() for the explanation */
}


#if defined(HAVE_STDC)
static void
out_input_position(IO_PAIR *pair)
#else /* K&R style */
static void
out_input_position(pair)
IO_PAIR *pair;
#endif
{
    out_s("# line ");
    out_number(pair->input.line_number);
    out_s(" \"");
    out_s(pair->input.filename);
    out_s("\"\n");
}


#if defined(HAVE_STDC)
void
out_lines(FILE *fpout, const char *lines[], YESorNO pausing)
#else /* K&R style */
void
out_lines(fpout, lines, pausing)
FILE *fpout;
const char *lines[];
YESorNO pausing;
#endif
{
    int k;

#if (SCREEN_LINES > 0)
    if ((pausing == YES) && (screen_lines > 0))
    {
	int lines_on_screen;
	int nlines;

	kbopen();
	for (nlines = 0; lines[nlines] != (const char*)NULL; ++nlines)
	    continue;				/* count number of lines */

	for (k = 0, lines_on_screen = 0; ; )
	{
	    if (lines[k] != (const char*)NULL)
	    {
		(void)fputs(lines[k], fpout);

		if (IN_SET(lines[k], '\n'))
		    lines_on_screen++;	/* some lines[k] are only partial */
	    }

	    if ((lines_on_screen == (screen_lines - 2)) ||
		   (lines[k] == (const char*)NULL))
	    {					/* pause for user action */
		lines_on_screen = 0;
		screen_lines = get_screen_lines(); /* maybe window resize? */
		if (screen_lines == 0)
		{
		    k++;
		    if (k < nlines)
			continue;
		    else
			break;			/* here's the loop exit */
		}

		k = do_more(fpout,k,screen_lines - 2,lines);
		if (k == EOF)
		    break;			/* here's the loop exit */
		else if (k == LAST_SCREEN_LINE)
		    k = nlines - (screen_lines - 2);
		if (k < 0)			/* ensure k stays in range */
		    k = 0;
		else if (k >= nlines)
		    k = nlines - 1;
	    }
	    else			/* still filling current screen */
		k++;
	}					/* end for (k...) */
	kbclose();
    }
    else					/* pausing == NO */
    {
	for (k = 0; lines[k] != (const char*)NULL; k++)
	    (void)fputs(lines[k], fpout);
    }
#else /* NOT (SCREEN_LINES > 0) */
    for (k = 0; lines[k] != (const char*)NULL; k++)
	(void)fputs(lines[k], fpout);
#endif /* (SCREEN_LINES > 0) */

}


void
out_newline(VOID)
{
    out_string(TOKEN_NEWLINE, "\n");
}


#if defined(HAVE_STDC)
static void
out_number(long n)
#else /* K&R style */
static void
out_number(n)
long n;
#endif
{
    char number[22];			/* ceil(log10(2^64-1))+1, big enough */
					/* for even 64-bit machines */
    (void)sprintf(number,"%ld",n);
    out_s(number);
}


#if defined(HAVE_STDC)
static void
out_position(FILE* fpout, const char *msg, IO_PAIR *the_location)
#else /* K&R style */
static void
out_position(fpout,msg,the_location)
FILE* fpout;
const char *msg;
IO_PAIR *the_location;
#endif
{
    char s[sizeof(
	" output byte=XXXXXXXXXX line=XXXXXXXXXX column=XXXXXXXXXX")+1];

    out_error(fpout, msg);
    (void)sprintf(s," input byte=%ld line=%ld column=%2ld",
		  the_location->input.byte_position,
		  the_location->input.line_number,
		  the_location->input.column_position);
    out_error(fpout, s);

    (void)sprintf(s, " output byte=%ld line=%ld column=%2ld\n",
		  the_location->output.byte_position,
		  the_location->output.line_number,
		  the_location->output.column_position);
    out_error(fpout, s);
}


#if defined(HAVE_STDC)
void
out_s(const char *s)		/* output a string, wrapping long lines */
#else /* K&R style */
void
out_s(s)			/* output a string, wrapping long lines */
const char *s;
#endif
{
    /* The strings s[] has already had runs of whitespace of all kinds
       collapsed to single spaces.  The word_length() function returns 1
       more than the actual non-blank word length at end of string, so
       that we can automatically account for the comma that will be
       supplied after the string. */

    for (; (*s != '\0'); ++s)
    {
	switch (*s)
	{
	case ' ':		/* may change space to new line and indent */
	    if ((wrapping == YES) &&
		(the_file.output.column_position + 1 + word_length(s+1))
		    > max_width)
		wrap_line();
	    else
		out_c((int)(unsigned char)*s);
	    break;

	case PARBREAK:		/* Possible after first character only if */
				/* keep_parbreaks == YES.  Cannot use */
				/* wrap_line(), because that is infinitely */
				/* recursive to this statement! */
	    out_c((int)(unsigned char)'\n');
	    out_c((int)(unsigned char)'\n'); /* two newlines marks a parbreak */
	    if (keep_spaces == NO)
		out_spaces(VALUE_INDENTATION); /* supply leading indentation */
	    break;

	case LINEBREAK:
	    out_c((int)(unsigned char)'\n'); /* change LINEBREAK to newline */
	    if (keep_spaces == NO)
	    {
		out_spaces(VALUE_INDENTATION); /* supply leading indentation */
		while (s[1] == ' ') /* and ignore following explicit space */
		    ++s;
	    }
	    break;

	default:		/* everything else is output verbatim */
	    out_c((int)(unsigned char)*s);
	}
    }
}


#if defined(HAVE_STDC)
void
out_spaces(int n)
#else /* K&R style */
void
out_spaces(n)
int n;
#endif
{
    if (prettyprint == YES)
    {
	for (; n > 0; --n)
	    out_c((int)' ');
    }
    /* If we are not prettyprinting, but lexically analyzing, we */
    /* cannot use n as a reliable count of spaces, because it is */
    /* based on column positions in prettyprinted output.  We must */
    /* therefore simply discard TOKEN_SPACE from the output stream. */
}


#if defined(HAVE_STDC)
static void
out_status (FILE* fpout,const char *prefix)
#else /* K&R style */
static void
out_status(fpout,prefix)
FILE* fpout;
const char *prefix;
#endif
{
    if (show_file_position == YES)
    {
	out_error(fpout, prefix);
	out_error(fpout, "  File positions:  input [");
	out_error(fpout, original_file.input.filename);
	out_error(fpout, "]  output [");
	out_error(fpout, original_file.output.filename);
	out_error(fpout, "]\n");

	out_error(fpout, prefix);
	out_position(fpout, "  Entry  ", &the_entry);

	out_error(fpout, prefix);
	out_position(fpout, "  Value  ", &the_value);

	out_error(fpout, prefix);
	out_position(fpout, "  Current", &original_file);
    }
}


#if defined(HAVE_STDC)
void
out_string(token_t type, const char *token)
#else /* K&R style */
void
out_string(type,token)
token_t type;
const char *token;
#endif
{
    if (KEEP_PREAMBLE_SPACES())
	out_verbatim(token);
    else if (KEEP_STRING_SPACES())
	out_verbatim(token);
    else if (prettyprint == YES)
	out_s(token);			/* prettyprinted output */
    else
	out_token(type,token);		/* lexical analysis output */
}


#if defined(HAVE_STDC)
void
out_token(token_t type, const char *token) /* lexical analysis output */
#else /* K&R style */
void
out_token(type,token)
token_t type;
const char *token;
#endif
{
    char octal[4 + 1];
    static long last_line_number = 0L;

    if (*token == (char)'\0')	/* ignore empty tokens */
	return;
    if (last_line_number < token_start.input.line_number)
    {
	out_input_position(&token_start);
	last_line_number = token_start.input.line_number;
    }
    out_number((long)type);
    out_c((int)'\t');
    out_s(type_name[(int)type]);
    out_c((int)'\t');
    out_c((int)'"');
    for (; (*token != '\0'); ++token)
    {
	switch (*token)
	{
	case '"':
	case '\\':
	    out_c((int)'\\');
	    out_c((int)*token);
	    break;
	case '\b':
	    out_c((int)'\\');
	    out_c((int)'b');
	    break;
	case '\f':
	    out_c((int)'\\');
	    out_c((int)'f');
	    break;
	case LINEBREAK:
	case '\n':
	    out_c((int)'\\');
	    out_c((int)'n');
	    break;
	case '\r':
	    out_c((int)'\\');
	    out_c((int)'r');
	    break;
	case '\t':
	    out_c((int)'\\');
	    out_c((int)'t');
	    break;
	case '\v':
	    out_c((int)'\\');
	    out_c((int)'v');
	    break;
	case PARBREAK:		/* two newlines mark a parbreak */
	    out_c((int)'\\');
	    out_c((int)'n');
	    out_c((int)'\\');
	    out_c((int)'n');
	    break;
	default:
	    if (Isprint(*token))
		out_c((int)(unsigned char)*token);
	    else
	    {
		(void)sprintf(octal,"\\%03o",BYTE_VAL(*token));
		out_s(octal);
	    }
	    break;
	}
    }
    out_c((int)'"');
    out_c((int)'\n');
}


#if defined(HAVE_STDC)
static void
out_verbatim(const char *token)
#else /* K&R style */
static void
out_verbatim(token)
const char *token;
#endif
{
    for (; (*token != '\0'); ++token)
    {
	switch (*token)
	{
	case LINEBREAK:
	    out_c((int)(unsigned char)'\n');
	    break;
	case PARBREAK:
	    out_c((int)(unsigned char)'\n');
	    out_c((int)(unsigned char)'\n');
	    break;
	default:
	    out_c((int)(unsigned char)*token);
	    break;
	}
    }
}


#if defined(HAVE_STDC)
void
out_with_error(const char *s, const char *msg)
#else /* K&R style */
void
out_with_error(s,msg)	/* output string s, error message, and resynchronize */
const char *s;
const char *msg;
#endif
{
    out_s(s);
    error(msg);
    resync();
}


#if defined(HAVE_STDC)
void
out_with_parbreak_error(char *s)
#else /* K&R style */
void
out_with_parbreak_error(s)
char *s;
#endif
{
    out_with_error(s, "Unexpected paragraph break for field ``%f''");
}


#if defined(HAVE_STDC)
void
put_back(int c)		/* put last get_char() value back onto input stream */
#else /* K&R style */
void
put_back(c)		/* put last get_char() value back onto input stream */
int c;
#endif
{
    if (n_pushback >= MAX_PUSHBACK)
    {
	warning("Pushback buffer overflow: characters lost");
	return;
    }
    pushback_buffer[n_pushback++] = c;

    the_file.input.byte_position--;

    /* Adjust status values that are set in get_char() */

    if (!Isspace(c))
	non_white_chars--;

    if (c == EOF)
	eofile = NO;
    else if (c == (int)'\n')
    {
	the_file.input.column_position = the_file.input.last_column_position;
	the_file.input.line_number--;
    }
    else if (c == (int)'\t')
	the_file.input.column_position = the_file.input.last_column_position;
    else
	the_file.input.column_position--;
    if (c == (int)'{')
	brace_level--;
    else if (c == (int)'}')
	brace_level++;
}


static void
resync(VOID)			/* copy input to output until new entry met */
{				/* and set resynchronization flag */
    rflag = YES;
    do_other();			/* copy text until new entry found */
    brace_level = 0;		/* might have been non-zero because of errors */
}


#if defined(HAVE_STDC)
char*
Strdup(const char *s)
#else /* K&R style */
char*
Strdup(s)
const char *s;
#endif
{
    char *p;
    p = (char*)malloc(strlen(s)+1);
    if (p == (char*)NULL)
	fatal("Out of string memory");
    return (strcpy(p,s));
}


#if defined(HAVE_STDC)
int
stricmp(const char *s1,const char *s2)
#else /* K&R style */
int
stricmp(s1, s2)
const char *s1;
const char *s2;
#endif
{

#define TOUPPER(c) (Islower(c) ? toupper((int)(c)) : (int)(c))

    while ((*s1 != '\0') && (TOUPPER(*s1) == TOUPPER(*s2)))
    {
	s1++;
	s2++;
    }
    return((int)(TOUPPER(*s1) - TOUPPER(*s2)));

#undef TOUPPER

}


#if defined(HAVE_STDC)
int
strnicmp(const char *s1, const char *s2, size_t n)
#else /* K&R style */
int
strnicmp(s1,s2,n)
const char	*s1;
const char	*s2;
size_t		n;
#endif
{
    int	   c1;
    int	   c2;

    /*******************************************************************
      Compare strings ignoring case, stopping after n characters, or at
      end-of-string, whichever comes first.
    *******************************************************************/

    for (; (n > 0) && (*s1 != '\0') && (*s2 != '\0'); ++s1, ++s2, --n)
    {
	c1 = 0xff & (int)(Islower(*s1) ? (int)*s1 : tolower((int)(*s1)));
	c2 = 0xff & (int)(Islower(*s2) ? (int)*s2 : tolower((int)(*s2)));
	if (c1 < c2)
	    return (-1);
	else if (c1 > c2)
	    return (1);
    }
    if (n == 0)		   /* first n characters match */
	return (0);
    else if (*s1 == '\0')
	return ((*s2 == '\0') ? 0 : -1);
    else /* (*s2 == '\0') */
	return (1);
}


#if defined(HAVE_STDC)
/*@null@*/
FILE*
tfopen(const char *filename, const char *mode) /* traced file opening */
#else /* K&R style */
/*@null@*/
FILE*
tfopen(filename,mode)
const char *filename;
const char *mode;
#endif
{
    FILE *fp;

    fp = FOPEN(filename,mode);
    if (trace_file_opening == YES)
	(void)fprintf(stdlog,"%s open file   [%s]%s\n",	/* NB: lineup with lookup in fndfil.c:file_is_readable() */
	    WARNING_PREFIX, filename, (fp == (FILE*)NULL) ? ": FAILED" : "");
    return (fp);
}


#if defined(HAVE_STDC)
void
warning(const char *msg)	/* issue a warning message to stdlog */
#else /* K&R style */
void
warning(msg)			/* issue a warning message to stdlog */
const char *msg;
#endif
{
    if (warnings == YES)
    {
	out_flush();		/* flush all buffered output */

	/* Because warnings are often issued in the middle of lines, we
	   start a new line if stdlog and stdout are the same file. */

	original_file = the_file;	/* save for error messages */

	(void)fprintf(stdlog,"%s%s %s:%ld:%s.\n",
	    (stdlog_on_stdout == YES) ? "\n" : "",
	    WARNING_PREFIX, the_file.input.filename,
	    the_value.input.line_number, format(msg));

	out_status(stdlog, WARNING_PREFIX);
	(void)fflush(stdlog);
    }
}


#if defined(HAVE_STDC)
static size_t
word_length(const char *s)	/* return length of leading non-blank prefix */
#else /* K&R style */
static size_t
word_length(s)			/* return length of leading non-blank prefix */
const char *s;
#endif
{
    size_t n;

    for (n = 0; (s[n] != '\0'); ++n)
    {
	if (Isspace(s[n]))
	    break;
    }
    return ((s[n] == '\0') ? n + 1 : n);
				/* at end of string, return one more than */
				/* true length to simplify line wrapping */
}


static void
wrap_line(VOID)			/* insert a new line and leading indentation */
{
    out_newline();
    out_spaces(VALUE_INDENTATION); /* supply leading indentation */
}


/***********************************************************************
 We put this regular expression matching code last because
   (a) it is not universally available,
   (b) the 6 macros in the HAVE_REGEXP section can only be defined
       once, and
   (c) there are three variants: the old ugly regexp.h interface (HAVE_REGEXP),
       the new clean regex.h interface (HAVE_RECOMP), and the GNU version
       (not yet supported here)
***********************************************************************/

/**********************************************************************/

#if defined(HAVE_RECOMP)
#if (_AIX || ultrix)
/* AIX 370, AIX PS/2, and ULTRIX have these, but no regex.h, sigh... */
#if __cplusplus
extern "C" {
#endif /* __cplusplus */

char		*re_comp ARGS((const char *s_));
int		re_exec ARGS((const char *s_));

#if __cplusplus
};
#endif /* __cplusplus */

#else /* NOT (_AIX || ultrix) */
#if defined(HAVE_REGEX_H)
#include <regex.h>
#endif
#endif /* (_AIX || ultrix) */

#if defined(HAVE_STDC)
static int
match_regexp(const char *string,const char *pattern)
#else /* K&R style */
static int
match_regexp(string,pattern)
const char *string;
const char *pattern;
#endif
{
    if (re_comp(pattern) != (char*)NULL)
	fatal("Internal error: bad regular expression");
    switch (re_exec(string))
    {
    case 1:
	return (YES);
    case 0:
	return (NO);
    default:
	fatal("Internal error: bad regular expression");
    }
    return (YES);		/* keep optimizers happy */
}
#endif /* defined(HAVE_RECOMP) */

/**********************************************************************/

#if defined(HAVE_REGEXP)
const char		*sp_global;
#define ERROR(c)	regerr()
#define GETC()		(*sp++)
#define INIT   		const char *sp = sp_global;
#define PEEKC()		(*sp)
#define RETURN(c)	return(c)
#define UNGETC(c)	(--sp)

void
regerr(VOID)
{
    fatal("Internal error: bad regular expression");
}

#if defined(HAVE_REGEXP_H)
#include <regexp.h>
#endif


#if defined(HAVE_STDC)
static int
match_regexp(const char *string,const char *pattern)
#else /* K&R style */
static int
match_regexp(string,pattern)
const char *string;
const char *pattern;
#endif
{
    char	expbuf[MAX_TOKEN_SIZE];

    sp_global = string;
    (void)compile((char*)pattern, (char*)expbuf,
	(char*)(expbuf + sizeof(expbuf)), '\0');
    return (step((char*)string,(char*)expbuf) ? YES : NO);
}
#endif /* defined(HAVE_REGEXP) */
/**********************************************************************/
