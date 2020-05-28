#include <config.h>
#include "xlimits.h"
#include "xstdbool.h"
#include "xstdlib.h"
#include "xstring.h"

RCSID("$Id: option.c,v 1.14 2014/04/03 18:07:03 beebe Exp beebe $")

#include "ch.h"
#include "isbn.h"
#include "yesorno.h"
#include "match.h"
#include "typedefs.h"			/* must come AFTER match.h */

extern YESorNO	align_equals;		/* NO: left-adjust equals */
extern YESorNO	brace_protect;		/* YES: brace-protect title words */
extern YESorNO	check_values;		/* NO: suppress value checks */
extern YESorNO	debug_match_failures; 	/* YES: debug value pattern match failures */
extern YESorNO	delete_empty_values; 	/* YES: delete empty values */
extern YESorNO	fix_accents;		/* repair accent bracing? */
extern YESorNO	fix_braces;		/* normalize bracing? */
extern YESorNO	fix_degrees;		/* normalize author academic degrees? */
extern YESorNO	fix_font_changes;	/* brace {\em E. Coli}? */
extern YESorNO	fix_initials;		/* reformat A.U. Thor?	*/
extern YESorNO	fix_math;		/* reformat math mode? */
extern YESorNO	fix_names;		/* reformat Bach, P.D.Q? */

extern YESorNO	German_style;		/* YES: " inside braced string
					   value obeys german.sty style */
extern YESorNO	keep_linebreaks;	/* YES: keep linebreaks in values */
extern YESorNO	keep_parbreaks;		/* YES: keep parbreaks in values */
extern YESorNO	keep_preamble_spaces;	/* YES: keep spaces in @Preamble{} */
extern YESorNO	keep_spaces;		/* YES: keep spaces in values */
extern YESorNO	keep_string_spaces;	/* YES: keep spaces in @String{} */
extern long	max_width;
extern YESorNO	parbreaks;		/* NO: parbreaks forbidden */
					/* in strings and entries */
extern YESorNO	prettyprint;		/* NO: do lexical analysis */
extern YESorNO	print_ISBN_table;	/* YES: print ISBN table */
extern YESorNO	print_keyword_table;	/* YES: print keyword table */
extern YESorNO	print_patterns;		/* YES: print value patterns */
extern char	*program_name;		/* set to argv[0] */
extern YESorNO	read_initialization_files; /* -[no-]read-init-files sets */
extern YESorNO	remove_OPT_prefixes; 	/* YES: remove OPT prefix */
extern int	screen_lines;		/* kbopen() and out_lines() reset */
extern YESorNO	Scribe;			/* Scribe format input */
extern YESorNO	show_file_position; 	/* messages usually brief */
extern FILE	*stdlog;		/* usually stderr */
extern YESorNO	trace_file_opening;	/* -[no-]trace-file-opening sets */
extern YESorNO	warnings;		/* NO: suppress warnings */

static int	current_index;		/* argv[] index in do_args() */
static char	*current_option;	/* set by do_args() */
static char	*next_option;		/* set in do_args() */

void		do_args ARGS((int argc_, char *argv_[]));
void		do_preargs ARGS((int argc_, char *argv_[]));

bool		is_optionprefix ARGS((int c));

extern YESorNO	apply_function ARGS((const char *option_,
		    OPTION_FUNCTION_ENTRY table_[]));
extern void	check_inodes ARGS((void));
extern void	do_initfile ARGS((const char *pathlist_,const char *name_));
extern void	do_keyword_file ARGS((const char *pathlist_,const char *name_));
extern void	finish ARGS((int status));
extern void	free_keyword_table ARGS((void));
extern void	out_lines ARGS((FILE *fpout_,const char *lines_[],
		    YESorNO pausing_));
extern FILE	*tfopen ARGS((const char *filename_, const char *mode_));
extern void	warning ARGS((const char *msg_));

static void	opt_align_equals ARGS((void));
static void	opt_author ARGS((void));
static void	opt_brace_protect ARGS((void));
static void	opt_check_values ARGS((void));
static void	opt_copyleft ARGS((void));
static void	opt_debug_match_failures ARGS((void));
static void	opt_delete_empty_values ARGS((void));
static void	opt_error_log ARGS((void));
static void	opt_file_position ARGS((void));
static void	opt_fix_accents ARGS((void));
static void	opt_fix_braces ARGS((void));
static void	opt_fix_degrees ARGS((void));
static void	opt_fix_font_changes ARGS((void));
static void	opt_fix_initials ARGS((void));
static void	opt_fix_math ARGS((void));
static void	opt_fix_names ARGS((void));
static void	opt_help ARGS((void));
static void	opt_init_file ARGS((void));
static void	opt_ISBN_file ARGS((void));
static void	opt_German_style ARGS((void));
static void	opt_keep_linebreaks ARGS((void));
static void	opt_keep_parbreaks ARGS((void));
static void	opt_keep_preamble_spaces ARGS((void));
static void	opt_keep_spaces ARGS((void));
static void	opt_keep_string_spaces ARGS((void));
static void	opt_keyword_file ARGS((void));
static void	opt_max_width ARGS((void));
static void	opt_output_file ARGS((void));
static void	opt_parbreaks ARGS((void));
static void	opt_prettyprint ARGS((void));
static void	opt_print_ISBN_table ARGS((void));
static void	opt_print_keyword_table ARGS((void));
static void	opt_print_patterns ARGS((void));
static void	opt_quiet ARGS((void));
static void	opt_read_init_files ARGS((void));
static void	opt_remove_OPT_prefixes ARGS((void));
static void	opt_scribe ARGS((void));
static void	opt_trace_file_opening ARGS((void));
static void	opt_version ARGS((void));
static void	opt_warnings ARGS((void));
static void	usage ARGS((void));
static void	version ARGS((void));
static YESorNO	YESorNOarg ARGS((void));

static CONST char *help_lines[] =
{
    "\nUsage: ",
    (const char*)NULL,
    " [ -author ] [ -copyleft ] [ -copyright ]\n",
    "\t[ -error-log filename ] [ -help ] [ '-?'  ]\n",
    "\t[ -init-file filename ] [ -ISBN-file filename ]\n",
    "\t[ -keyword-file filename ] [ -max-width nnn ]\n",
    "\t[ -[no-]align-equals ] [ -[no-]brace-protect ]\n",
    "\t[ -[no-]check-values ] [ -[no-]debug-match-failures ]\n",
    "\t[ -[no-]delete-empty-values ] [ -[no-]file-position ]\n",
    "\t[ -[no-]fix-accents ] [ -[no-]fix-braces ] [ -[no-]fix-degrees ]\n",
    "\t[ -[no-]fix-font-changes ] [ -[no-]fix-initials ]\n",
    "\t[ -[no-]fix-math ] [ -[no-]fix-names ] [ -[no-]German-style ]\n",
    "\t[ -[no-]keep-linebreaks ] [ -[no-]keep-parbreaks ]\n",
    "\t[ -[no-]keep-preamble-spaces ] [ -[no-]keep-spaces ]\n",
    "\t[ -[no-]keep-string-spaces ] [ -[no-]parbreaks ]\n",
    "\t[ -[no-]prettyprint ] [ -[no-]print-ISBN-table ]\n",
    "\t[ -[no-]print-keyword-table ] [ -[no-]print-patterns ]\n",
    "\t[ -[no-]quiet ] [ -[no-]read-init-files ]\n",
    "\t[ -[no-]remove-OPT-prefixes ] [ -[no-]scribe ]\n",
    "\t[ -[no-]trace-file-opening ] [ -[no-]warnings ]\n",
    "\t[ -output-file filename ] [ -version ]\n",
    "\t[ <infile or bibfile1 bibfile2 bibfile3 ...] >outfile\n",
    "\n",
#include "bibclean.h"
};


#if defined(HAVE_STDC)
void
do_args(int argc, char *argv[])
#else /* K&R style */
void
do_args(argc,argv)
int argc;
char *argv[];
#endif
{
    int k;				/* index into argv[] */
#define MSG_PREFIX	"Unrecognized option switch: "
#define MAX_OPTION_LENGTH	100
    char msg[sizeof(MSG_PREFIX) + MAX_OPTION_LENGTH + 1];
					/* for error messages */
    int nfiles;				/* number of files found in argv[] */

    static OPTION_FUNCTION_ENTRY options[] =
    {
	{"?",				1,	opt_help},
	{"align-equals",		2,	opt_align_equals},
	{"author",			2,	opt_author},
	{"brace-protect",		1,	opt_brace_protect },
	{"check-values",		1,	opt_check_values},
	{"copyleft",			2,	opt_copyleft },
	{"copyright",			2,	opt_copyleft },
	{"debug-match-failures",	3,	opt_debug_match_failures},
	{"delete-empty-values",		1,	opt_delete_empty_values},
	{"error-log",			1,	opt_error_log},
	{"file-position",		3,	opt_file_position},
	{"fix-accents",			5,	opt_fix_accents},
	{"fix-braces",			5,	opt_fix_braces},
	{"fix-degrees",			5,	opt_fix_degrees},
	{"fix-font-changes",		5,	opt_fix_font_changes},
	{"fix-initials",		5,	opt_fix_initials},
	{"fix-math",			5,	opt_fix_math},
	{"fix-names",			5,	opt_fix_names},
	{"German-style",		1,	opt_German_style},
	{"help",			1,	opt_help},
	{"init-file",			2,	opt_init_file},
	{"ISBN-file",			2,	opt_ISBN_file},
	{"keep-linebreaks",		6,	opt_keep_linebreaks},
	{"keep-parbreaks",		7,	opt_keep_parbreaks},
	{"keep-preamble-spaces",	7,	opt_keep_preamble_spaces},
	{"keep-spaces",			7,	opt_keep_spaces},
	{"keep-string-spaces",		7,	opt_keep_string_spaces},
	{"keyword-file",		3,	opt_keyword_file},
	{"max-width",			1,	opt_max_width},
	{"no-align-equals",		4,	opt_align_equals},
	{"no-brace-protect",		4,	opt_brace_protect },
	{"no-check-values",		4,	opt_check_values},
	{"no-debug-match-failures",	6,	opt_debug_match_failures},
	{"no-delete-empty-values",	6,	opt_delete_empty_values},
	{"no-file-position",		6,	opt_file_position},
	{"no-fix-accents",		8,	opt_fix_accents},
	{"no-fix-braces",		8,	opt_fix_braces},
	{"no-fix-degrees",		8,	opt_fix_degrees},
	{"no-fix-font-changes",		8,	opt_fix_font_changes},
	{"no-fix-initials",		8,	opt_fix_initials},
	{"no-fix-math",			8,	opt_fix_math},
	{"no-fix-names",		8,	opt_fix_names},
	{"no-German-style",		4,	opt_German_style},
	{"no-keep-linebreaks",		9,	opt_keep_linebreaks},
	{"no-keep-parbreaks",		10,	opt_keep_parbreaks},
	{"no-keep-preamble-spaces",	10,	opt_keep_preamble_spaces},
	{"no-keep-spaces",		10,	opt_keep_spaces},
	{"no-keep-string-spaces",	10,	opt_keep_string_spaces},
	{"no-parbreaks",		5,	opt_parbreaks},
	{"no-prettyprint",		6,	opt_prettyprint},
	{"no-print-ISBN-table",		10,	opt_print_ISBN_table},
	{"no-print-keyword-table",	10,	opt_print_keyword_table},
	{"no-print-patterns",		10,	opt_print_patterns},
	{"no-quiet",			4,	opt_quiet},
	{"no-read-init-files",		6,	opt_read_init_files},
	{"no-remove-OPT-prefixes",	6,	opt_remove_OPT_prefixes},
	{"no-scribe",			4,	opt_scribe},
	{"no-trace-file-opening",	4,	opt_trace_file_opening},
	{"no-warnings",			4,	opt_warnings},
	{"output-file",			1,	opt_output_file},
	{"parbreaks",			2,	opt_parbreaks},
	{"prettyprint",			3,	opt_prettyprint},
	{"print-ISBN-table",		7,	opt_print_ISBN_table},
	{"print-keyword-table",		7,	opt_print_keyword_table},
	{"print-patterns",		7,	opt_print_patterns},
	{"quiet",			1,	opt_quiet},
	{"read-init-files",		3,	opt_read_init_files},
	{"remove-OPT-prefixes",		3,	opt_remove_OPT_prefixes},
	{"scribe",			1,	opt_scribe},
	{"trace-file-opening",		1,	opt_trace_file_opening},
	{"version",			1,	opt_version},
	{"warnings",			1,	opt_warnings},
	{(const char*)NULL,		0,	(void (*)(VOID))NULL},
    };

    for (nfiles = 1, k = 1; k < argc; ++k)
    {
	if ( (argv[k][1] != '\0') && is_optionprefix((int)argv[k][0]) )
	{				/* then process command-line switch */
	    current_index = k;		/* needed by opt_init_file() and */
	    next_option = argv[k+1];	/* opt_error_log() */
	    current_option = argv[k];	/* needed by YESorNOarg() */
	    if (is_optionprefix((int)current_option[1]))
		current_option++;	/* allow GNU/POSIX --option */
	    if (apply_function(current_option+1,options) == NO)
	    {
		(void)sprintf(msg, "%s%.*s", MSG_PREFIX, MAX_OPTION_LENGTH,
			      current_option);
		warning(msg);
		usage();
		finish(EXIT_FAILURE);
	    }
	    k = current_index;		/* some opt_xxx() functions update it */
	}
	else				/* save file names */
	    argv[nfiles++] = argv[k];	/* shuffle file names down */
    }
    argv[nfiles] = (char*)NULL;		/* terminate new argument list */
}


#if defined(HAVE_STDC)
void
do_preargs(int argc, char *argv[])
#else /* K&R style */
void
do_preargs(argc,argv)
int argc;
char *argv[];
#endif
{
    int k;

    static OPTION_FUNCTION_ENTRY options[] =
    {
	{"no-print-ISBN-table",		10,	opt_print_ISBN_table},
	{"no-print-patterns",		10,	opt_print_patterns},
	{"no-read-init-files",		6,	opt_read_init_files},
	{"no-trace-file-opening",	4,	opt_trace_file_opening},
	{"print-ISBN-table",		7,	opt_print_ISBN_table},
	{"print-patterns",		7,	opt_print_patterns},
	{"read-init-files",		3,	opt_read_init_files},
	{"trace-file-opening",		1,	opt_trace_file_opening},
	{(const char*)NULL,		0,	(void (*)(VOID))NULL},
    };

    for (k = 1; k < argc; ++k)
    {
	/* Do argument scan for options that must be known BEFORE
	initializations are attempted. */

	if ( (argv[k][1] != '\0') && is_optionprefix((int)argv[k][0]) )
	{				/* then process command-line switch */
	    current_index = k;
	    current_option = argv[k];
	    if (is_optionprefix((int)current_option[1]))
		current_option++;	/* allow GNU/POSIX --option */
	    next_option = argv[k+1];
	    (void)apply_function(current_option+1,options);
	}
    }
}


#if defined(HAVE_STDC)
bool
is_optionprefix(int c)
#else /* K&R style */
bool
is_optionprefix(c)
int c;
#endif
{
    bool result;

    result = false;

#if OS_PCDOS
    result = (((c) == (int)'-') || ((c) == (int)'/')) ? true : false;
#endif /* OS_PCDOS */

#if OS_UNIX
    result = ((c) == (int)'-') ? true : false;
#endif /* OS_UNIX */

#if OS_VAXVMS
    result = (((c) == (int)'-') || ((c) == (int)'/')) ? true : false;
#endif /* OS_VAXVMS */

    return (result);
}


static void
opt_align_equals(VOID)
{
    align_equals = YESorNOarg();
}


static void
opt_author(VOID)
{
    static CONST char *author[] =
    {
	"Author:\n",
	"\tNelson H. F. Beebe\n",
	"\tUniversity of Utah\n",
	"\tDepartment of Mathematics, 110 LCB\n",
	"\t155 S 1400 E RM 233\n",
	"\tSalt Lake City, UT 84112-0090\n",
	"\tUSA\n",
	"\tTel: +1 801 581 5254\n",
	"\tFAX: +1 801 581 4148\n",
	"\tEmail: beebe@math.utah.edu, beebe@acm.org, beebe@computer.org (Internet)\n",
	(const char*)NULL,
    };

    out_lines(stdlog, author, NO);
    finish(EXIT_SUCCESS);
}

static void
opt_brace_protect(VOID)
{
    brace_protect = YESorNOarg();
}

static void
opt_check_values(VOID)
{
    check_values = YESorNOarg();
}


static void
opt_copyleft(VOID)
{
    static CONST char *copyleft[] =
    {
	"Copyright (C) 1990--2018 by Nelson H. F. Beebe and the Free Software Foundation, Inc.\n",
	"This is free software; see the source for copying conditions.  There is NO\n",
	"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",
	(const char*)NULL,
    };

    out_lines(stdlog, copyleft, NO);
    finish(EXIT_SUCCESS);
}


static void
opt_debug_match_failures(VOID)
{
    debug_match_failures = YESorNOarg();
}


static void
opt_delete_empty_values(VOID)
{
    delete_empty_values = YESorNOarg();
}


static void
opt_error_log(VOID)
{
    current_index++;

    if ((stdlog = tfopen(next_option,"w"), stdlog) == (FILE*)NULL)
    {
	(void)fprintf(stderr, "%s cannot open error log file [%s]",
		      WARNING_PREFIX, next_option);
	(void)fprintf(stderr, " -- using stderr instead\n");
	perror("perror() says");
	stdlog = stderr;
    }
    else
	check_inodes();				/* stdlog changed */
}


static void
opt_file_position(VOID)
{
    show_file_position = YESorNOarg();
}


static void
opt_fix_accents(VOID)
{
    fix_accents = YESorNOarg();
}


static void
opt_fix_braces(VOID)
{
    fix_braces = YESorNOarg();
}


static void
opt_fix_degrees(VOID)
{
    fix_degrees = YESorNOarg();
}


static void
opt_fix_font_changes(VOID)
{
    fix_font_changes = YESorNOarg();
}


static void
opt_fix_initials(VOID)
{
    fix_initials = YESorNOarg();
}


static void
opt_fix_math(VOID)
{
    fix_math = YESorNOarg();
}


static void
opt_fix_names(VOID)
{
    fix_names = YESorNOarg();
}


static void
opt_German_style(VOID)
{
    German_style = YESorNOarg();
}


static void
opt_help(VOID)
{
    help_lines[1] = program_name;	/* cannot have this in initializer */
    out_lines(stdlog, help_lines, (screen_lines > 0) ? YES : NO);
    finish(EXIT_SUCCESS);
}


static void
opt_init_file(VOID)
{
    current_index++;
    do_initfile((const char*)NULL,next_option);
}


static void
opt_ISBN_file(VOID)
{
    current_index++;
    do_ISBN_file((const char*)NULL,next_option);
}


static void
opt_keep_linebreaks(VOID)
{
    keep_linebreaks = YESorNOarg();
}


static void
opt_keep_parbreaks(VOID)
{
    keep_parbreaks = YESorNOarg();
}


static void
opt_keep_preamble_spaces(VOID)
{
    keep_preamble_spaces = YESorNOarg();
}


static void
opt_keep_spaces(VOID)
{
    keep_spaces = YESorNOarg();
}


static void
opt_keep_string_spaces(VOID)
{
    keep_string_spaces = YESorNOarg();
}


static void
opt_keyword_file(VOID)
{
    current_index++;
    do_keyword_file((const char*)NULL,next_option);
}


static void
opt_max_width(VOID)
{
    current_index++;
    max_width = strtol(next_option,(char**)NULL,0);
    if (max_width <= 0L)		/* width <= 0 means unlimited width */
	max_width = LONG_MAX;
}


static void
opt_output_file(VOID)
{
    current_index++;

    if (freopen(next_option, "w", stdout) != stdout)
    {
	(void)fprintf(stderr, "Cannot open file [%s] for output: job terminated!\n", next_option);
	finish(EXIT_FAILURE);
    }
}


static void
opt_parbreaks(VOID)
{
    parbreaks = YESorNOarg();
}


static void
opt_prettyprint(VOID)
{
    prettyprint = YESorNOarg();
}


static void
opt_print_ISBN_table(VOID)
{
    print_ISBN_table = YESorNOarg();
}


static void
opt_print_keyword_table(VOID)
{
    print_keyword_table = YESorNOarg();
}


static void
opt_print_patterns(VOID)
{
    print_patterns = YESorNOarg();
}


static void
opt_quiet(VOID)
{
    warnings = (YESorNOarg() == YES) ? NO : YES;
}


static void
opt_read_init_files(VOID)
{
    read_initialization_files = YESorNOarg();
}


static void
opt_remove_OPT_prefixes(VOID)
{
    remove_OPT_prefixes = YESorNOarg();
}


static void
opt_scribe(VOID)
{
    Scribe = YESorNOarg();
}


static void
opt_trace_file_opening(VOID)
{
    trace_file_opening = YESorNOarg();
}


static void
opt_version(VOID)
{
    version();
    finish(EXIT_SUCCESS);
}


static void
opt_warnings(VOID)
{
    warnings = YESorNOarg();
}


static void
usage(VOID)
{
    version();
    help_lines[1] = program_name;	/* cannot have this in initializer */
    out_lines(stdlog, help_lines, NO);
}


static void
version(VOID)
{
    static CONST char *version_string[] =
    {
	PACKAGE_NAME, " Version ", PACKAGE_VERSION, " [", PACKAGE_DATE, "]",
	"\n",
	"\n",

#if defined(HOST) || defined(USER) || defined(__DATE__) || defined(__TIME__)
	"Compiled",

#if defined(USER) && defined(HOST)	/* suppressable for privacy reasons */
	" by <", USER, "@", HOST, ">",
#endif /* defined(USER) && defined(HOST) */

#if defined(__DATE__)
	" on ", __DATE__,
#endif /* defined(__DATE__) */

#if defined(__TIME__)
	" ", __TIME__,
#endif /* defined(__TIME__) */

#if defined(HAVE_PATTERNS)
	"\nwith native pattern matching",
#endif /* defined(HAVE_PATTERNS) */

#if defined(HAVE_RECOMP) || defined(HAVE_REGEXP)
	"\nwith regular-expression pattern matching",
#endif /* defined(HAVE_RECOMP) || defined(HAVE_REGEXP) */

#if defined(HAVE_OLDCODE)
	"\nwith old matching code",
#endif /* defined(HAVE_OLDCODE) */

	"\n",
#endif /* defined(HOST)||defined(USER)||defined(__DATE__)||defined(__TIME__) */
	"E-mail bug reports to ", PACKAGE_BUGREPORT, "\n",

	"\n",
	"Copyright (C) 1990--2018 by Nelson H. F. Beebe and the Free Software Foundation, Inc.\n",
	"This is free software; see the source for copying conditions.  There is NO\n",
	"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n",

	(const char*)NULL,
    };

    out_lines(stdlog, version_string, NO);
}


static YESorNO
YESorNOarg(VOID)
{
    return ((strnicmp(current_option+1,"no-",3) == 0) ? NO : YES);
}
