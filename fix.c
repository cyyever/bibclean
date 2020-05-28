#include <config.h>
#include <assert.h>		/* FOR DEBUGGING ONLY */
#include "xctype.h"
#include "xstring.h"

RCSID("$Id: fix.c,v 1.12 2017/06/09 14:46:57 beebe Exp beebe $")

#include "yesorno.h"
#include "token.h"
#include "match.h"
#include "typedefs.h"			/* must come AFTER match.h */

#define SKIP_SPACE(p)	while (Isspace((unsigned char)*p)) ++p

extern YESorNO	brace_math;		/* NO: leave mixed-case math text untouched */
extern YESorNO	brace_protect;		/* NO: leave mixed-case title words untouched */
extern char	current_value[];	/* string value */
extern YESorNO	fix_accents;		/* repair accent bracing? */
extern YESorNO	fix_braces;		/* normalize bracing? */
extern YESorNO	fix_degrees;		/* normalize author academic degrees? */
extern YESorNO	fix_font_changes; 	/* brace {\em E. Coli}? */
extern YESorNO	fix_initials;		/* reformat A.U. Thor?	*/
extern YESorNO	fix_math;		/* reformat math mode? */
extern YESorNO	fix_names;		/* reformat Bach, P.D.Q? */
extern NAME_PAIR month_pair[];
extern YESorNO	Scribe;			/* Scribe format input */

extern YESorNO	check_junior ARGS((const char *last_name_));
extern void	check_length ARGS((size_t n_));
extern char	shared_string[MAX_TOKEN_SIZE];

static void	brace_font_changes ARGS((void));
static void	check_math_words ARGS((const char *));
static void	fix_accent_bracing ARGS((void));
static char	*fix_author ARGS((char *author_));
static char	*fix_author_degrees ARGS((char *author_));
static void	fix_bracing ARGS((void));
void		fix_math_spacing ARGS((void));
void		fix_month ARGS((void));
void		fix_namelist ARGS((void));
void		fix_pages ARGS((void));
static char	*fix_periods ARGS((char *author_));
void		fix_title ARGS((void));
/*@null@*/ static const char *month_token ARGS((/*@null@*/ const char *s_, size_t *p_len_));
static void	store_space(char *s_, size_t *pk_);
static size_t	squeeze_space ARGS((char *s_));
extern void	warning ARGS((const char *msg_));

    /* we need some common operations dozens of times, so hide them in clean statement macros */

#define COPY_1()	STORE_CHAR(current_value[t++])

#define COPY_2()	( COPY_1(), COPY_1() )

#define COPY_3()	( COPY_1(), COPY_1(), COPY_1() )

#define DISCARD_OUTPUT_SPACE()		while ((k > 0) && (s[k - 1] == ' ')) k--

#define DISCARD_CURRENT_VALUE_SPACE()	while (Isspace(current_value[t])) t++

#define MAYBE_INSERT_OPEN_BRACE()	if (need_brace == YES) STORE_CHAR('{')

#define MAYBE_INSERT_CLOSE_BRACE()	if (need_brace == YES) STORE_CHAR('}')

#define PREV_CHAR()	((k >= 2) ? s[k - 2] : '\0')

#define PROCESS_2()	do					\
			{					\
			    MAYBE_INSERT_OPEN_BRACE();		\
			    COPY_2();				\
			    MAYBE_INSERT_CLOSE_BRACE();		\
			    DISCARD_CURRENT_VALUE_SPACE();	\
			} while (0)


#define PROCESS_3()	do					\
			{					\
			    MAYBE_INSERT_OPEN_BRACE();		\
			    COPY_3();				\
			    MAYBE_INSERT_CLOSE_BRACE();		\
			    DISCARD_CURRENT_VALUE_SPACE();	\
			} while (0)

#define STORE_CHAR(c)	s[k++] = (char)(c)

#define STORE_NUL()	s[k] = (char)'\0'

#define STORE_SPACE()	store_space(s, &k)

#define THIS_CHAR()	((k >= 1) ? s[k - 1] : '\0')

static void
brace_font_changes(VOID)
{
    /*******************************************************************
    ** If the user has coded a title string like
    **
    ** "Signal-transducing {G} proteins in {\em Dictyostelium Discoideum}"
    **
    ** or
    **
    ** "Signal-transducing {G} proteins in {\em {D}ictyostelium {D}iscoideum}"
    **
    ** BibTeX styles that downcase titles will downcase the name
    ** Dictyostelium Discoideum, even WITH the protecting braces around
    ** the D's.  The solution offered by this function is to rewrite
    ** the title string as
    **
    ** "Signal-transducing {G} proteins in {{\em Dictyostelium Discoideum}}"
    **
    ** This action cannot be taken without forethought, because there
    ** are many cases where the downcasing inside font changes is
    ** consistent, so the default run-time option is -no-fix-font-changes.
    *******************************************************************/

    int b_level;			/* brace level */
    size_t k;				/* index into current_value[] */
    size_t m;				/* index into s[] */
    YESorNO need_close_brace;
    char *p;				/* pointer into current_value[] */
    char *s = shared_string;		/* memory-saving device */

    for (b_level = 0, k = 0, m = 0, need_close_brace = NO;
	 (current_value[k] != '\0') ; ++k, ++m)
    {
	switch (current_value[k])
	{
	case '{':			/* '}' for balance */
	    b_level++;

	    if (b_level == 1)
	    {
		p = &current_value[k+1];
		SKIP_SPACE(p);

		if (*p == '{')		/* '}' for balance */
		    break;		/* already have extra brace level */

		if (
		    (strncmp(p,"\\bf",3) == 0) ||
		    (strncmp(p,"\\em",3) == 0) ||
		    (strncmp(p,"\\it",3) == 0) ||
		    (strncmp(p,"\\rm",3) == 0) ||
		    (strncmp(p,"\\sf",3) == 0) ||
		    (strncmp(p,"\\sl",3) == 0) ||
		    (strncmp(p,"\\tt",3) == 0) )
		{
		    s[m++] = '{';	/* '}' for balance */
		    need_close_brace = YES;
		}
	    }

	    break;
			/* '{' for balance */
	case '}':
	    if ( (b_level == 1) && (need_close_brace == YES) )
	    {				/* '{' for balance */
		s[m++] = '}';
		need_close_brace = NO;
	    }

	    b_level--;
	    break;

	default:
	    break;
	}
	s[m] = current_value[k];
    }
    s[m] = '\0';			/* terminate collected string */
    (void)strcpy(current_value, s);
}


static void
check_math_words(const char * s)
{
    /*
    ** Diagnose omitted backslashes in math mode, such as in "$sin x$,
    ** which should be "$\sin x$", and "$O(n log n)$" which should be
    ** "$O(n \log n)$".
    */

    const char *math_words[] =
    {
	/*
	** Math words (from plain.tex:1049, and LaTeX Users Guide and
	** Reference Manual, 2nd edition, page 44, Tabel 3.9: Log-like
	** Functions) that should only appear as macros (i.e.,
	** prefixed with a backslash).  It is a common error to omit
	** the backslash, especially in data copied from non-TeX files
	** and Web pages, and easy for us to check words against this
	** list.  Of course, there are many other possible words that
	** COULD be similarly in error, but this list at least catches
	** the commonest ones.
	**
	** The insertion of spacing between consecutive letters in
	** math mode in response to the --fix-math option may help to
	** make them more evident to the user.
	*/
	"arccos",
	"arcsin",
	"arctan",
	"arg",
	"cos",
	"cosh",
	"cot",
	"coth",
	"csc",
	"deg",
	"det",
	"dim",
	"exp",
	"gcd",
	"hom",
	"inf",
	"ker",
	"lg",
	"lim",
	"liminf",
	"limsup",
	"ln",
	"log",
	"max",
	"min",
	"Pr",
	"sec",
	"sin",
	"sinh",
	"sup",
	"tan",
	"tanh",
	(const char *)NULL
    };
    size_t k, n;

    for (k = 0; math_words[k] != (const char *)NULL; ++k)
    {
	n = strlen(math_words[k]);

	if ( (!Isalpha(s[n])) &&
	      (s[0] == math_words[k][0]) &&
	      (strncmp(s, math_words[k], n) == 0) )
	{
	    warning("Unbackslashed math word in math mode in value ``%v''");
	    break;
	}
    }
}


static void
fix_accent_bracing(VOID)
{
    /*******************************************************************
    ** Normalize accents in current_value[]:
    **
    **	   \'x	 -> {\'x}	{\'x}	-> {\'x}	[unchanged]
    **	   \'{x} -> {\'x}	{\'{x}} -> {\'x}
    **	   \H{x} -> {\H{x}}	{\H{x}} -> {\H{x}}	[unchanged]
    **	   \H x	 -> {\H{x}}	{\H x}	-> {\H{x}}	[unchanged]
    **
    ** where x is any letter, apostrophe stands for all of the standard
    ** TeX accent special characters, and H stands for all of the
    ** alphabetically-named accents.  Here are their expansions (see
    ** Table 3.1, Accents, on page 38 of the LaTeX User's Guide and
    ** Reference Manual, second edition)
    **
    **	   '	 -> ` ' ^ " ~ = .
    **	   H	 -> u v H t c d b [and r, the forgotten ring accent]
    **
    ** We also process these macros that represent letters in extended
    ** Latin alphabets:
    **
    **	   \oe	\OE  \ae  \AE  \aa  \AA	 \o  \O	 \l  \L	 \ss \i
    *******************************************************************/

    char *s = shared_string;		/* memory-saving device */
    size_t k;				/* index into s[] */
    size_t t;				/* index into current_value[] */

    for (k = 0, t = 0; current_value[t] != '\0'; )
    {
	if (current_value[t] == '\\')	/* start of TeX control sequence or macro */
	{
	    YESorNO need_brace;

	    if ( (k > 0) && (THIS_CHAR() != '{') )	/* have, e.g., " \'x " or " \'{x} " */
		need_brace = YES;
	    else
		need_brace = NO;

	    switch (current_value[t + 1])
	    {
	    case '`':
	    case '\'':
	    case '^':
	    case '\"':
	    case '~':
	    case '=':
	    case '.':
		MAYBE_INSERT_OPEN_BRACE();
		COPY_2();				/* copy backslash and accent */

		if ( (       current_value[t + 0] == '{') &&
		     Isalpha(current_value[t + 1]) &&
		     (       current_value[t + 2] == '}') )
		{
		    t++;				/* discard open brace */
		    COPY_1();				/* copy letter */
		    t++;				/* discard close brace */
		}
		else if (current_value[t] == '\\')	/* a TeX macro gets an accent! */
		{
		    COPY_1();		/* copy backslash */

		    if ( (current_value[t] == 'i') || (current_value[t] == 'j') )
		    {
			COPY_1();	/* copy dotless i or j */
			DISCARD_CURRENT_VALUE_SPACE();
		    }
		    else if (Isalpha(current_value[t]))	/* control word, e.g., \alpha */
		    {
			while (Isalpha(current_value[t]))
			    COPY_1();

			DISCARD_CURRENT_VALUE_SPACE();
		    }
		    else				/* single-character control sequence  */
			COPY_1();	/* copy single nonletter */
		}
		else
		    COPY_1();		/* copy argument letter */

		MAYBE_INSERT_CLOSE_BRACE();
		break;

	    case 'H':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'r':
	    case 't':
	    case 'u':
	    case 'v':
		if (!Isalpha(current_value[t + 2]))	/* have valid accent control word */
		{
		    MAYBE_INSERT_OPEN_BRACE();
		    COPY_2();				/* copy backslash and accent */
		    DISCARD_CURRENT_VALUE_SPACE();

		    if ( (       current_value[t + 0] == '{') &&
			 Isalpha(current_value[t + 1]) &&
			 (       current_value[t + 2] == '}') )
			COPY_3();			/* copy open brace, letter, and close brace */
		    else if (current_value[t] == '{')
		    {
			int brace_level;
			
			brace_level = 1;

			while (brace_level > 0)
			{
			    COPY_1();

			    if (current_value[t] == '{')
				brace_level++;
			    else if (current_value[t] == '}')
				brace_level--;
			}
		    }
		    else if (current_value[t] == '\\')	/* a TeX macro gets an accent! */
		    {
			COPY_1();	/* copy backslash */

			if ( (current_value[t] == 'i') && !Isalpha(current_value[t + 1]) ) /* \i */
			{
			    COPY_1();	/* copy i */
			    DISCARD_CURRENT_VALUE_SPACE();
			}
			else if (Isalpha(current_value[t]))	/* control word, e.g., \alpha */
			{
			    while (Isalpha(current_value[t]))
				COPY_1();

			    DISCARD_CURRENT_VALUE_SPACE();
			}
			else				/* single-character control sequence  */
			    COPY_1();	/* copy single nonletter */
		    }
		    else				/* copy braced character */
		    {
			STORE_CHAR('{');
			COPY_1();	/* copy argument character */
			STORE_CHAR('}');
		    }

		    MAYBE_INSERT_CLOSE_BRACE();
		}
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    case 'A':					/* expect \AA or \AE or \Alpha or ... */
		if ( (current_value[t + 2] == 'A') && !Isalpha(current_value[t + 3]) )
		    PROCESS_3();			/* have \AA */
		else if ( (current_value[t + 2] == 'E') && !Isalpha(current_value[t + 3]) )
		    PROCESS_3();			/* have \AE */
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    case 'a':					/* expect \aa or \ae or \alpha or ... */
		if ( (current_value[t + 2] == 'a') && !Isalpha(current_value[t + 3]) )
		    PROCESS_3();			/* have \aa */
		else if ( (current_value[t + 2] == 'e') && !Isalpha(current_value[t + 3]) )
		    PROCESS_3();			/* have \ae */
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    case 'L':					/* expect \L or \Lambda or ... */
		if (!Isalpha(current_value[t + 2]))
		    PROCESS_2();			/* have \L */
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    case 'l':					/* expect \l or \lambda or ... */
		if (!Isalpha(current_value[t + 2]))
		    PROCESS_2();			/* have \l */
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    case 'O':					/* expect \O or \OE or \Omega or ... */
		if (!Isalpha(current_value[t + 2]))
		    PROCESS_2();			/* have \O */
		else if (current_value[t + 2] == 'E')
		    PROCESS_3();			/* have \OE */
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    case 'o':					/* expect \o or \oe or \omega or ... */
		if (!Isalpha(current_value[t + 2]))
		    PROCESS_2();			/* have \o */
		else if (current_value[t + 2] == 'e')
		    PROCESS_3();			/* have \oe */
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    case 's':					/* expect \ss or \sigma or ... */
		if ( (current_value[t + 2] == 's') && !Isalpha(current_value[t + 3]) )
		    PROCESS_3();
		else
		    COPY_1();		/* copy backslash verbatim */

		break;

	    default:
		COPY_1();		/* copy backslash verbatim */
		break;
	    }
	}
	else
	    COPY_1();			/* copy ordinary character */
    }

    STORE_NUL();
    k = squeeze_space(s);
    check_length(k);
    (void)strcpy(current_value,s);
}


#if defined(HAVE_STDC)
static char *			/* normalize author names and return */
fix_author(char *author)	/* new string from static space */
#else /* K&R style */
static char *
fix_author(author)		/* normalize author names and return */
char *author;			/* new string from static space */
#endif
{
    size_t a;			/* index into author[] */
    int b_level;		/* brace level */
    char *p;			/* pointer into author[] */
    char *pcomma;		/* pointer to last unbraced comma in author[] */
    static char s[MAX_TOKEN_SIZE]; /* returned to caller */

    /* Convert "Smith, J.K." to "J. K. Smith" provided "," and "." are */
    /* at brace level 0 */

    if ( (fix_names == NO) || (author == (char *)NULL) )
	return (author);

    /* Leave untouched entries like: */
    /* author = "P. D. Q. Bach (113 Mozart Strasse, Vienna, Austria)" */

    if (IN_SET(author, '('))
	return (author);

    /*******************************************************************
    ** We now have a tricky job.  Some names have additional parts,
    ** which BibTeX calls "von" and "Jr.".  It permits them to be input
    ** as (see L. Lamport, ``LaTeX User's Guide and Reference Manual'',
    ** pp. 141--142)
    **
    **           Brinch Hansen, Per      OR      Per {Brinch Hansen}
    **           Ford, Jr., Henry        OR      Henry {Ford, Jr.}
    **           {Steele Jr.}, Guy L.    OR      Guy L. {Steele Jr.}
    **           von Beethoven, Ludwig   OR      Ludwig von Beethoven
    **           {von Beethoven}, Ludwig OR      Ludwig {von Beethoven}
    **
    ** The last two lines are NOT equivalent; the first will be
    ** alphabetized under Beethoven, and the second under von.
    **
    ** Other examples include names like
    **
    **           Charles XII, King       OR      King Charles XII
    **           Ford, Sr., Henry        OR      Henry {Ford, Sr.}
    **           Vallee Poussin, C. L. X. J. de la       OR
    **                   C. L. X. J. de la Vallee Poussin
    **           van der Waerden, Bartel Leendert        OR
    **                   Bartel Leendert van der Waerden
    **
    ** These transformations conform to the general patterns
    **
    **           B, A            -->     A B
    **           B C, A          -->     A B C           (von case)
    **           B C, A          -->     A {B C}         (Brinch Hansen case)
    **           B, C, A         -->     A {B, C}        (Jr. case)
    **
    ** A, B, and C represent one or more space-separated words, or
    ** brace-delimited strings with arbitrary contents.
    **
    ** Notice the conflict: the von case differs from Brinch Hansen in
    ** that braces may NOT be inserted when the name is reordered,
    ** because this changes the alphabetization.
    **
    ** In order to deal with this ambiguity, we supply braces in the "B
    ** C, A" case ONLY when the C part matches something like Jr (see
    ** the juniors[] table above), or when it looks like a small number
    ** in Roman numerals.  The latter case is uncommon, and we therefore
    ** don't bother to attempt to parse it to determine whether it is a
    ** valid number.
    **
    ** The "B, C, A" case (multiple level-zero commas) is unambiguous,
    ** and can be converted to the form "A {B, C}".
    *******************************************************************/

    for (a = 0, b_level = 0, pcomma = (char*)NULL; author[a] != '\0'; ++a)
    {				/* convert "Smith, John" to "John Smith" */
	switch (author[a])
	{
	case '{':
	    b_level++;
	    break;

	case '}':
	    b_level--;
	    break;

	case ',':
	    if (b_level == 0)
		pcomma = &author[a];	/* remember last unbraced comma */

	    break;

	default:
	    break;
	}
    }

    if (pcomma == (char*)NULL)		/* no commas, so nothing more to do */
	return (author);

    *pcomma = '\0';			/* terminate "Smith" */

    /* have "Smith, J.K." or "Smith, Jr., J.K." */
    p = pcomma + 1;
    SKIP_SPACE(p);
    (void)strcpy(s,p);			/* s <- "J.K." */
    (void)strcat(s," ");		/* s <- "J.K. " */

    if (check_junior(author) == YES)
    {
	(void)strcat(s,"{");
	(void)strcat(s,author);		/* s <- "J.K. {Smith, Jr.}" */
	(void)strcat(s,"}");
    }
    else
	(void)strcat(s,author);		/* s <- "J.K. Smith" */

    return (strcpy(author,s));
}


static void
fix_bracing(VOID)
{
    /*******************************************************************
    ** Here are some cases to consider for developing an algorithm for
    ** removal of unnecessary bracing, and moving closing braces to
    ** end of token:
    **
    ** Strings with protecting outer braces:
    **
    **     "{ ... {\em E. coli} ... }"
    **            |-----------| required for font scope
    **
    **     "{ ... \cite{label} ... }"
    **                 |-----| required for argument delimiting
    **
    **     "{ ... ${\Delta}$ ... }"
    **             |------| removable (at level 1), but look like font change
    **
    **     " ... $x_{1/2}$ ... "
    **              |---| required for argument delimiting
    **
    **     "{ ... {ARITH '20} conference ... }"
    **            |---------| removable: at level 1
    **
    **     "{ ... \twoargs{one}{two} ... }"
    **                    |---||---| required for argument delimiting
    **
    **     "{Books about the {{\TeX}} typesetting system}"
    **                       |------| removable
    **                        |----| required for macro delimiting
    **
    **     "{Breaking the f{}i, f{}l, ff{}i, and ff{}l ligatures}"
    **                     ||    ||     ||         ||  not removable
    **
    **     "{Breaking the {f}i, {f}l, f{f}i, and f{f}l ligatures}"
    **                    |-|   |-|    |-|        |-|  not removable
    **
    **     "{History of the {E}nglish language}"
    **                      |-| removable: at level 1
    **
    **     "{History of the non{E}nglish languages}"
    **                         |-| removable: at level 1
    **
    ** Here are the same examples, but without the outer protecting
    ** braces:
    **
    **     " ... {\em E. coli} ... "
    **           |-----------| required for font scope: another level needed
    **
    **     " ... \cite{label} ... "
    **                |-----| required for argument delimiting
    **
    **     " ... ${\Delta}$ ... "
    **            |------| not removable: at level 0
    **
    **     " ... $x_{1/2}$ ... "
    **              |---| required for argument delimiting
    **
    **     " ... {ARITH '20} conference ... "
    **           |---------| not removable: at level 0
    **
    **     " ... \twoargs{one}{two} ... "
    **                   |---||---| required for argument delimiting
    **
    **     "Books about the {{\TeX}} typesetting system"
    **                      |------| removable
    **                       |----| required for macro delimiting
    **
    **     "Breaking the f{}i, f{}l, ff{}i, and ff{}l ligatures"
    **                    ||    ||     ||         ||  not removable
    **
    **     "Breaking the {f}i, {f}l, f{f}i, and f{f}l ligatures"
    **                    |-|   |-|    |-|        |-|  not removable
    **
    **     "History of the {E}nglish language"
    **                     |-| not removable: at level 0
    **                         but redistributable to {English}
    **
    **     "History of the non{E}nglish languages"
    **                     |-| not removable: at level 0
    **                         but redistributable to {nonEnglish}
    **
    ** The required algorithm seems to be:
    **
    **   ==> braces are removable if at level 1 or higher, and not
    **       inside math mode, and not following a TeX macro name
    **       without intervening space
    **
    **   ==> braces at any level can be moved to the ends of the
    **       current alphabetic token (except for the (rare and
    **       unlikely) ligature-breaking case)
    **
    ** The ligature example shows that we cannot fix old-style
    ** bracing without occasionally breaking the user's intent!
    **
    ** WRONG:  " ... f{f}l ... " -> " ... ffl ... "
    **
    ** WRONG:  "Die {A}xiomatisierung der {W}ahrscheinlichkeitsrechnung" ->
    **         "Die Axiomatisierung der Wahrscheinlichkeitsrechnung"
    **
    ** OKAY:   "{Die {A}xiomatisierung der {W}ahrscheinlichkeitsrechnung}" ->
    **         "{Die Axiomatisierung der Wahrscheinlichkeitsrechnung}"
    **
    ** Our simple parsing breaks with multiargument macros: our code
    ** does these reductions:
    **
    ** OKAY:	"{{\twoargs {One}{Two}}}"	-> "{{\twoargs {One}{Two}}}"
    **
    ** OKAY:	"{{\twoargs {One}{Two}}}"	-> "{{\twoargs {One}{Two}}}"
    ** WRONG:	"{{\twoargs {One} {Two}}}"	-> "{{\twoargs {One} Two}}"
    **
    ** OKAY:	"{\twoargs {One}{Two}}"		-> "{\twoargs {One}{Two}}"
    ** WRONG:	"{\twoargs {One} {Two}}"	-> "{{\twoargs {One} Two}}"
    **
    ** Because TeX macros can consume arbitrary numbers of arguments,
    ** and those arguments do not require braces or separators if they
    ** are single tokens (e.g., a, b, c, \alpha, \beta, \gamma, ...),
    ** we simply CANNOT guarantee always-correct behavior.
    ** Fortunately, most uses of macros in BibTeX titles are just font
    ** changes, or have zero arguments, or have single arguments
    ** (e.g., "{\em E. coli}", "{\TeX}: the program", and
    ** "\booktitle{The Dog}").  In most cases, we do the right thing,
    ** and provide the minimal number of braces.
    *******************************************************************/

    char *s = shared_string;		/* memory-saving device */
    int b_level;			/* brace level */
    int f_level;			/* font level */
    int m_level;			/* math-mode level */
    size_t k;				/* index into s[] */
    size_t m;				/* index into s[] */
    size_t t;				/* index into current_value[] */
    YESorNO discarded_close_open_pair;	/* YES if we just discarded "}{" in s[] */
    YESorNO in_TeX_macro;		/* YES if current nonblank token has backslash */

    (void)memset(s, 0, 255);		/* DEBUGGING ONLY! */

    for (b_level = 0, f_level = 0, k = 0, m_level = 0, in_TeX_macro = NO, t = 0;
	 current_value[t] != '\0'; )
    {
	switch (current_value[t])
	{
	case '\\':			/* macro and following space */
	    COPY_1();
	    in_TeX_macro = YES;

	    if (Isalpha(current_value[t]))
	    {
		while (Isalpha(current_value[t]))
		    COPY_1();
	    }
	    else
		COPY_1();

	    /*
	    ** We need to copy trailing space here to preserve the
	    ** value of in_TeX_macro, which otherwise gets reset to NO
	    ** at a space.
	    */

	    while (Isspace(current_value[t]))
		COPY_1();

	    break;

	case ' ':
	    COPY_1();
	    in_TeX_macro = NO;
	    break;

	case '$':
	    if (current_value[t + 1] == '$')
		COPY_2();      		/* display math mode "...$$...$$..." */
	    else
		COPY_1(); /* normal math mode "...$...$..." */

	    if (m_level == 0)
		m_level = 1;		/* enter normal or display math mode */
	    else
		m_level = 0;		/* leave normal math mode */

	    break;

	case '{':

	    /*
	    ** BibTeX's normal handling of personal names in author/editor
	    ** fields allows rearrangements such as these:
	    **
	    ** "Arthur Baines Conway" -> "A. B. Conway"
	    **                        -> "Arthur B. Conway"
	    **                        -> "Conway, Arthur B."
	    **                        -> "Conway, A. B.",
	    **
	    ** and the family name (here, "Conway") can be used to
	    ** create an alphanumeric citation label, and a bibliography
	    ** sort key.
	    **
	    ** However, in Hungary and many Oriental countries, the
	    ** family name appears first, and it is then desirable to
	    ** prevent name reordering when the title is in the
	    ** corresponding language.  BibTeX's undocumented
	    ** convention is that a braced space prevents name
	    ** reordering, so we have author strings like these:
	    **
	    **     "H{\'a}n{ }Th{\^e}\llap{\raise 0.5ex\hbox{\'{\relax}}}{ }Th{\'a}nh"
	    **	   "Kawasaki{ }Kimio"
	    **     "Mao{ }Tse-Tung"
	    **     "Neumann{ }Jan{\'o}s"
	    **     "Park{ }Chung-hee"
	    **     "{von{ }E{\"o}tv{\"o}s{ }Lor{\'a}nd}"
	    **
	    ** We must therefore check for "{ }" and output it verbatim.
	    */

	    if ( (current_value[t + 1] == ' ') && (current_value[t + 2] == '}') )
	    {
		COPY_3();
		break;
	    }

	    b_level++;
	    COPY_1();

	    /* Leave "{Z}{\"u}rich" untouched, but reduce "{A}{BC}" to "{ABC}" */

	    if ( (current_value[t] == '\\') || (current_value[t + 1] == '\\') )
		in_TeX_macro = YES;

	    if ( (k > 0) && (s[k - 2] == '}') && (in_TeX_macro == NO) && (m_level == 0) )
	    {
		k -= 2;		/* discard (probably) unneeded "}{" */
		s[k] = '\0';	/* FOR DEBUGGING ONLY */
		discarded_close_open_pair = YES;
	    }
	    else
		discarded_close_open_pair = NO;

	    if (current_value[t] == '\\')
	    {
		f_level = b_level;
		in_TeX_macro = YES;
	    }
	    else if ( (discarded_close_open_pair == NO) && (in_TeX_macro == NO) && (m_level == 0) )
	    {
		/*
		** Try to convert, e.g., "non{E}nglish" to "{nonE}nglish" (the
		** close brace is moved later).
		**
		** We also convert "4.4{BSD}" to "{4.4BSD}" by including period
		** (dot or full-stop) inside the braces.  Such characters are
		** common in software version numbers (e.g., "Web-2.0").
		*/

		for (m = k - 2; (m > 0) && (Isalnum(s[m]) || (s[m] == '.')); --m)
		    /* NO-OP */;

		if (m < (k - 2))
		{			/* move open brace from s[k] back to s[m + 1] */
		    (void)memmove(&s[m + 2], &s[m + 1], k - m);
		    s[m + 1] = '{';
		}
	    }

	    break;

	case '}':
	    COPY_1();
	    b_level--;

	    if (b_level < f_level)
		f_level = b_level;

	    /* try to convert, e.g., "{nonE}nglish" to "{nonEnglish}" */

	    m = k - 1;			/* index of final '}' in s[] */

	    if ( (in_TeX_macro == NO) && (m_level == 0) )
	    {
		while (Isalnum(current_value[t]))
		{			/* we might move close brace multiple times */
		    while (Isalnum(current_value[t]))
			COPY_1();

		    if (m < (k - 1)) 	/* move close brace from s[m] to s[k - 1] */
		    {
			(void)memmove(&s[m], &s[m + 1], k - m);
			s[k - 1] = '}';
		    }
		}
	    }

	    assert(s[k - 1] == '}'); /* FOR DEBUGGING ONLY */

	    /*
	    ** Try to reduce text like
	    **
	    **     "{Advances in database technology, {EDBT '88}}"
	    **
	    ** to
	    **
	    **     "{Advances in database technology, EDBT '88}"
	    **
	    ** However, preserve the font-change group in text like this:
	    **
	    **     "{Advances in {\em E. coli\//} research}"
	    **
	    ** and around macro argument like this:
	    **
	    **     "{Einstein}'s {\booktitle{The meaning of relativity}}"
	    **
	    */

	    if ( (in_TeX_macro == NO) && (b_level > 0) && (m_level == 0) )
	    {			/* unbrace last braced group */

		if (k > 2)
		{
		    int last_level;

		    last_level = b_level;

		    for (m = k - 1; m > 0; --m)
		    {
			if (s[m] == '{')
			{
			    last_level--;

			    if (last_level == b_level)
				break;
			}
			else if (s[m] == '}')
			    last_level++;
		    }

		    assert(s[m] == '{'); /* FOR DEBUGGING ONLY */

		    if ( (s[m + 1] != '\\') &&			/* not a font-change group */
			 ((m > 0) && !Isalpha(s[m - 1])) &&	/* probably not macro argument */
			 (strnicmp("{ }", &s[m], 3) != 0)	/* not special one-space group */
		       )
		    {
			(void)memmove(&s[m], &s[m + 1], k - m);

			k -= 2;	 /* because we discarded the surrounding braces */
			s[k] = '\0'; /* FOR DEBUGGING ONLY */
		    }
		}
	    }

	    break;

	default:
	    COPY_1();
	    break;
	}
    }

    STORE_NUL();
    k = squeeze_space(s);
    check_length(k);
    (void)strcpy(current_value,s);

    if (b_level != 0)
	warning("Unbalanced brace(s) in value ``%v''");
}

void
fix_math_spacing(VOID)
{
    /*******************************************************************
    ** TeX ignores whitespace inside math mode, so spacing changes
    ** there are harmless, and we exploit that fact to improve the
    ** readability of math in BibTeX abstract, annote, booktitle,
    ** note, remark, and title fields (the definitive list is in file
    ** do.c in the initialization of the fixes[] table).
    **
    ** Also, we often prefer math mode groups to be surrounded by
    ** space, and if the group contains an uppercase letter, the
    ** entire group must be braced to protect against downcasing by
    ** some BibTeX styles, because that action changes the meaning of
    ** the mathematics, and also could make the TeX math markup
    ** invalid.
    **
    ** There are some common cases, however, where we need to suppress
    ** such outer spacing:
    **
    ** (1) superscripts before names (e.g., chemical isotope $^{13}$C)
    **
    ** (2) superscripts and scripts in chemical compounds (e.g.,
    **     Cr$_2$O$_3$) and reaction names (e.g., S$_N$2)
    **
    ** (3) math at end of clause, phrase, or sentence:
    **     "If $a > b$, then ..."
    **     "Suppose $a > b$; then ..."
    **     "Here is the equation: $a = b$."
    **     "In the equations ``$a = b$'' and ``c = d$'', ..."
    **
    ** The code in the loop body is somewhat simplified if we allow
    ** consecutive spaces; they are removed by squeeze_space() after
    ** the main loop has completed.
    **
    ** In general, we add spaces immediately inside the delimiting
    ** dollar signs, and if they are not braced, around macros and
    ** letters. Here are some typical transformations:
    **
    **	  "${\rm SU}(2)$"		-> "{$ {\rm SU}(2) $}"
    **    "$4d\sigma$"			-> "$ 4 d \sigma $"
    **    "$Ax=b$"			-> "{$ A x = b $}"
    **    "$abc=xyz$"			-> "$ a b c = x y z $"
    **    "$\phi\chi\psi$"		-> "$ \phi \chi \psi $"
    **
    ** However, for single-character math mode, and single-character
    ** subscripts and superscripts, spacing is suppressed:
    **
    **    "CH$_3$OH"			-> unchanged
    **	  "The {S$_N$2} reaction"	-> unchanged
    **
    *******************************************************************/

    char *s = shared_string;		/* memory-saving device */
    int b_level;			/* brace level */
    int b_level_math;			/* brace level at start of math mode */
    int m_level;			/* math level */
    size_t k;				/* index into s[] */
    size_t math_start;			/* index into s[] */
    size_t t;				/* index into current_value[] */
    YESorNO is_unary_operator;		/* unary (not binary) operator */
    YESorNO just_left_math;		/* just left math mode? */
    YESorNO math_ties;		 	/* unexpected ties in math mode? */
    YESorNO suppress_math_space;	/* YES: omit space around math contents */
    YESorNO ucmath;			/* uppercase letters in math mode? */

    if (fix_accents == YES)
	fix_accent_bracing();

    if (fix_braces == YES)
	fix_bracing();

    if (fix_math == NO)
	return;

    just_left_math = NO;
    math_start = 0;
    math_ties = NO;
    suppress_math_space = NO;
    ucmath = NO;

    for (k = 0, b_level = 0, b_level_math = 0, m_level = 0, t = 0;
	 current_value[t] != '\0'; )
    {
	YESorNO need_space;		/* YES if need space before pending token */

	switch (current_value[t])
	{
	case '\\':			/* copy backslash and following character(s) */
	    if ( (m_level > 0) && Isupper(current_value[t + 1]) )
		ucmath = YES;

	    need_space = ((m_level > 0) && (k > 0) && !IN_SET("^_([{", THIS_CHAR())) ? YES : NO;

	    if (m_level > 0)
	    {
		if (need_space == YES)
		    STORE_SPACE();	/* space before backslashed delimiters */
		else if ( (t > 0) && Isalnum(current_value[t - 1]) )
		    STORE_SPACE();	/* space between letter and macro name */
	    }

	    COPY_1();			/* copy the backslash */

	    if (Isalpha(current_value[t]))	/* TeX control word */
	    {
		while (Isalpha(current_value[t]))
		    COPY_1();

		if ( (m_level > 0) && (need_space == NO) )
		    STORE_SPACE();
	    }
	    else			/* TeX control symbol */
	    {
		if (current_value[t] == '$')
		    need_space = NO;

		COPY_1();

		if ( (m_level > 0) && (need_space == NO) )
		    STORE_SPACE();
	    }

	    if ( (m_level > 0) && (need_space == YES) )
		STORE_SPACE();		/* space after backslashed delimiters */

	    break;

	case '$':
	    if (current_value[t + 1] == '$')
	    {   /* display math mode "...$$...$$..." */
		if (m_level > 0)
		{
		    m_level--;		/* leave display-math mode */
		    just_left_math = YES;
		    STORE_SPACE();	/* always space before closing math delimiter */
		}
		else
		{
		    b_level_math = b_level;
		    m_level++;		/* enter display-math mode */
		    ucmath = NO;

		    if ( (k > 0) &&
			 !IN_SET("`',.:;ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", THIS_CHAR()) )
			STORE_SPACE();/* space before math delimiter */

		    math_start = k;	/* remember start of math mode */
		}

		COPY_1(); /* copy first dollar */
	    }
	    else if (m_level > 0)
	    {				/* leave normal math mode */
		m_level--;
		just_left_math = YES;

		if (IN_SET("-)]", current_value[t + 1])) /* leave "$\tau$-particle", */
		    suppress_math_space = YES;		 /* "($x$)", and "[$y$]", unchanged */

		if ((suppress_math_space == NO) && Isspace(s[math_start + 1]))
		    STORE_SPACE();	/* symmetry: two outer spaces, or neither */

		if ( (k >= 4) &&
		     (s[k - 4] == '$') &&
		     (s[k - 3] == ' ') &&
		     (s[k - 1] == ' ') )
		{			/* squeeze single-character math mode */
		    s[k - 3] = s[k - 2];
		    k -= 2;
		}
		else if ( (k >= 4) &&
			  (s[k - 4] == '$') &&
			  IN_SET("^_", s[k - 3]) &&
			  (s[k - 1] == ' ') ) /* squeeze sub/sup + single-character math mode */
		    k--;
		else if ( (k >= 3) &&
		     (s[k - 3] == '$') &&
		     (s[k - 1] == ' ') )/* squeeze single-character math mode */
		    k--;
	    }
	    else
	    {				/* enter normal math mode */
		b_level_math = b_level;
		m_level++;
		math_start = k;		/* remember start of math mode */
		ucmath = NO;
	    }

	    COPY_1(); /* copy final open or close dollar */

	    if (m_level > 0)
	    {
		/* suppress space for "xxx$_n" and "xxx${}_n$ (leading subscripts)
		   and "xxx$^n" and "xxx${}^n$ (leading superscripts) */
		if (IN_SET("^_", current_value[t]))
		    suppress_math_space = YES;
		else if ( (current_value[t + 0] == '{') &&
			  (current_value[t + 1] == '}') &&
			  IN_SET("^_", current_value[t + 2]) )
		    suppress_math_space = YES;
		else if ( Isalnum(current_value[t]) &&
			  (current_value[t + 1] == '$') )
		    suppress_math_space = YES; /* keep $x$ and $3$D without extra spacing */
		else
		    STORE_SPACE();	/* always space after opening math delimiter */
	    }
	    else if (!IN_SET("{}`)]}',.:;-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", current_value[t]))
		STORE_SPACE();		/* space after math mode, unless punctuation
					   or uppercase letter or digit follows */

	    if ( (brace_math == YES) &&
		 (just_left_math == YES) &&
		 (ucmath == YES) &&
		 (k > math_start) &&
		 (b_level == 0) )
	    {
		(void)memmove(&s[math_start + 1], &s[math_start], k - math_start);
		s[math_start] = '{';

		if (!Isspace(s[k]))
		    k++;

		STORE_CHAR('}');
		math_start = 0;
		ucmath = NO;
	    }

	    just_left_math = NO;
	    break;

	case '+':			/* FALL THROUGH */
	case '-':			/* FALL THROUGH */
	case '/':
	    if (m_level > 0)
	    {
		/*
		** In math mode, +, -, and / may be binary operators,
		** which should have surrounding space for better
		** visibility, but + and - may also be unary
		** operators, as in $x^{-n}$ and Cr${2+}$, in which
		** case, surrounding space is undesirable.  We can
		** distinguish the two cases be looking at the
		** characters output before and after the operator.
		*/

		if ( (k > 0) && !IN_SET("\\^_", THIS_CHAR()) )
		{
		    is_unary_operator = ( (THIS_CHAR() == '{') || (current_value[t + 1] == '}') ) ? YES : NO;

		    if (is_unary_operator == NO)
			STORE_SPACE();

		    COPY_1();

		    if ( (is_unary_operator == NO) &&
			 (current_value[t] != '=') ) /* keep "+=", "-=", "/=", "==" together */
			STORE_SPACE();
		}
		else
		    COPY_1();
	    }
	    else
		COPY_1();

	    break;

	case '=':
	    if (m_level > 0)
	    {
		if ( (k > 0) && !IN_SET("\\+-/*:<>", THIS_CHAR()) )
		{
		    STORE_SPACE();
		    COPY_1();

		    if (current_value[t] != '=') /* keep "+=", "-=", "/=", "==" together */
			STORE_SPACE();
		}
		else
		{
		    if (IN_SET("+-/*:<>", THIS_CHAR()))
			warning("Unusual compound assignment or equality-test "
				"operator in math mode value ``%v'' (e.g., perhaps replace <= by \\leq)");

		    COPY_1();

		    if (current_value[t] != '=') /* keep "==" together */
			STORE_SPACE();
		}
	    }
	    else
		COPY_1();

	    break;

	case '&':			/* FALL THROUGH */
	case ':':
	    if (m_level > 0)
	    {
		if ( (k > 0) && (THIS_CHAR() != '\\') )
		{
		    STORE_SPACE();
		    COPY_1();

		    if (current_value[t] != '=') /* keep ":=" together */
			STORE_SPACE();
		}
		else
		    COPY_1();
	    }
	    else
		COPY_1();

	    break;

	case '<':			/* FALL THROUGH */
	case '>':
	    if (m_level > 0)
	    {
		if ( (k > 0) && (THIS_CHAR() != '\\') )
		{
		    STORE_SPACE();
		    COPY_1();

		    if (!IN_SET("=<>", current_value[t]))
			STORE_SPACE();
		}
		else
		    COPY_1();
	    }
	    else
		COPY_1();

	    break;

	case '{':
	    if (PREV_CHAR() != '\\')
		b_level++;

	    COPY_1();

	    if ( (m_level > 0) && (b_level == b_level_math) )
	    {
		/*
		** Restricting the discarding of space to the brace
		** level at entry is essential to preserve significant
		** space in macros, such as $\phi \hbox{ and } \chi$.
		*/

		while (Isspace(current_value[t]))
		    t++;		/* discard space after open delimiter */
	    }

	    break;

	case '}':
	    if ( (m_level > 0) && (b_level == b_level_math) )
	    {
		/*
		** Restricting the discarding of space to the brace
		** level at entry is essential to preserve significant
		** space in macros, such as $\phi \hbox{ and } \chi$.
		*/

		while (Isspace(current_value[t]))
		while ( (k > 0) && Isspace(THIS_CHAR()) )
		    k--;    /* discard space before close delimiter */
	    }

	    COPY_1();

	    if (PREV_CHAR() != '\\')
		b_level--;

	    break;

	case '[':			/* FALL THROUGH */
	case '(':
	    COPY_1();

	    if (m_level > 0)
	    {
		while (Isspace(current_value[t]))
		    t++;		/* discard space after open delimiter */
	    }

	    break;

	case ']':			/* FALL THROUGH */
	case ')':
	    if (m_level > 0)
	    {
		while ( (k > 0) && Isspace(THIS_CHAR()) )
		    k--;    /* discard space before close delimiter */
	    }

	    COPY_1();
	    break;

	case '_':			/* FALL THROUGH */
	case '^':
	    if (m_level > 0)
	    {
		while ((k > 0) && Isspace(THIS_CHAR()))
		    k--;	/* discard space before subscript and superscript */

		COPY_1();	/* copy subscript/superscript operator */

		while (Isspace(current_value[t]))
		    t++;   /* discard space after subscript and superscript */

		if ( (current_value[t    ] == '{') &&
		     (current_value[t + 1] != '\\') &&
		     (current_value[t + 2] == '}') )
		{ /* discard superfluous braces around single-character subscript and superscript */
		    t++;
		    COPY_1();
		    t++;

		    if (Isupper(THIS_CHAR()))
			ucmath = YES;
		}
		else if (IN_SET("+-*<>", current_value[t]))
		    COPY_1();
	    }
	    else if (current_value[t] == '_')
	    {
		COPY_1();
		warning("Subscript operator outside math mode in value ``%v''");
	    }
	    else if (current_value[t] == '^')
	    {
		COPY_1();
		warning("Superscript operator outside math mode in value ``%v''");
	    }
	    else			/* NOT REACHED */
		COPY_1();

	    break;

	case '~':
	    if ( (m_level > 0) && (k > 0) && (THIS_CHAR() != '\\') )
		math_ties = YES;

	    COPY_1();
	    break;

	case ',':			/* FALL THROUGH */
	case ';':
	    if (m_level > 0)
		DISCARD_OUTPUT_SPACE();

	    COPY_1();

	    if (m_level > 0)
		STORE_SPACE();		/* space after list separators */

	    break;

	case ' ':
	    STORE_SPACE();
	    t++;
	    break;

	default:
	    if ( (m_level > 0) && Isupper(current_value[t]) )
		ucmath = YES;

	    if ((m_level > 0) && Isalpha(current_value[t]))
	    {			   /* replace math "xyz" by "x y z" */
		check_math_words(&current_value[t]);

		COPY_1();

		while (Isalpha(current_value[t]))
		{
		    if (b_level == b_level_math)
			STORE_SPACE();
		    COPY_1();
		}
	    }
	    else if ((m_level > 0) && Isdigit(current_value[t]))
	    {			   /* copy numbers intact */
		COPY_1();

		while (Isdigit(current_value[t]) || (current_value[t] == '.'))
		    COPY_1();

		if ((t > 2) && (current_value[t - 2] == '$') && (current_value[t] == '$') )
		    /* NO-OP: e.g. "$3$D */;
		else if ( (current_value[t] != '$') && (b_level == b_level_math) )
		    STORE_SPACE();
	    }
	    else
		COPY_1();

	    break;
	}
    }

    if ( (k > 1) && (THIS_CHAR() == '\"') && Isspace(PREV_CHAR()) )
    {
	k--;

	while ( (k > 0) && Isspace(THIS_CHAR()) ) /* trim trailing space */
	    k--;

	STORE_CHAR('"');
    }

    STORE_NUL();
    k = squeeze_space(s);
    check_length(k);
    (void)strcpy(current_value,s);

    if (m_level != 0)
	warning("Unbalanced math-mode dollar(s) in value ``%v''");

    if (math_ties == YES)
	warning("Unexpected TeX tie[s] (tilde[s]) in math mode ``%v''");
}


void
fix_month(VOID)			/* convert full month names to macros*/
{				/* for better style-file customization */
    size_t k;				/* index into month_pair[] and s[] */
    size_t token_length;		/* token length */
    const char *p;			/* pointer to current_value[] */
    char *s = shared_string;		/* memory-saving device */
    const char *token;			/* pointer into current_value[] */

    for (p = current_value;
	 (token = month_token(p,&token_length), token) != (const char*)NULL;
	 p = (const char*)NULL)
    {
	if (token_length == 1)		/* just copy single-char tokens	 */
	   *s++ = *token;
	else
	{
	    for (k = 0; month_pair[k].old_name != (const char*)NULL; ++k)
	    {
		if ( (strlen(month_pair[k].old_name) == token_length) &&
		     (strnicmp(month_pair[k].old_name,token,token_length) == 0) )
		{			/* change "January" to jan etc. */
		    (void)strcpy(s,"\" # ");

		    if ( (s >= (shared_string + 1)) &&
			 (strncmp(s-1,"\"\" # ",5) == 0) )
		    {	/* eliminate any concatenation with an empty string */
			s--;
			*s = '\0';	/* need string terminator for strcat() */
		    }

		    (void)strcat(s,month_pair[k].new_name);
		    (void)strcat(s," # \"");
		    s = strchr(s,'\0');
		    token_length = 0;	/* so we don't copy twice at loop end */
		    break;		/* exit loop after first substitution */
		}
	    }				/* end for (k = 0, ...) */
	    (void)strncpy(s,token,token_length); /* no definition, just copy */
	    s += token_length;
	}

	if ( (s >= (shared_string + 5)) && (strncmp(s-5," # \"\"",5) == 0) )
	    s -= 5;	/* eliminate any concatenation with an empty string */
    }
    *s = '\0';				/* supply string terminator */
    s = shared_string;

    if (strncmp(s,"\"\" # ",5) == 0)
	(void)strcpy(current_value,&s[5]); /* discard initial empty string */
    else
	(void)strcpy(current_value,s);
}


void
fix_namelist(VOID)		/* normalize list of personal names */
{				/* leaving it in global current_value[] */
    size_t m;			/* index of start of author in current_value[]*/
    size_t n;			/* length of current_value[], less 1 */
    char namelist[MAX_TOKEN_SIZE];	/* working copy of current_value[] */
    size_t v;			/* loop index into current_value[] */

    /* Convert "Smith, J.K. and Brown, P.M." to */
    /* "J. K. Smith and P. M. Brown" */
    /* We loop over names separated by " and ", and hand each off */
    /* to fix_author() */

    n = strlen(current_value) - 1;	/* namelist = "\"...\"" */

    if ( (current_value[0] != '"') ||
	 (current_value[n] != '"') )	/* sanity check */
	return;				/* not quoted string, may be macro */

    if (fix_accents == YES)
    {
	fix_accent_bracing();
	n = strlen(current_value) - 1;	/* namelist = "\"...\"" */
    }

    if (fix_braces == YES)
    {
	fix_bracing();
	n = strlen(current_value) - 1;	/* namelist = "\"...\"" */
    }

    (void)strcpy(namelist,"\"");/* supply initial quotation mark */
    current_value[n] = (char)'\0';	/* clobber final quotation mark */

    for (v = 1, m = 1; v < n; ++v) /* start past initial quotation mark */
    {
	if (strncmp(" and ",&current_value[v],5) == 0)
	{
	    current_value[v] = (char)'\0';
	    (void)strcat(namelist,fix_periods(fix_author(&current_value[m])));
	    (void)strcat(namelist," and ");
	    current_value[v] = (char)' ';
	    v += 4;
	    m = v + 1;
	}
	else if ( (Scribe == YES) && (current_value[v] == ';') )
	{				/* expand semicolons to " and " */
	    current_value[v] = (char)'\0';
	    (void)strcat(namelist,fix_periods(fix_author(&current_value[m])));
	    (void)strcat(namelist," and ");
	    current_value[v] = (char)' ';
	    m = v + 1;
	}
    }
    (void)strcat(namelist,fix_periods(fix_author(&current_value[m])));
					/* handle last author */
    (void)strcat(namelist,"\"");	/* supply final quotation mark */
    (void)strcpy(current_value,namelist);
}


void
fix_pages(VOID)
{
    size_t k;			/* index into current_value[] */
    size_t m;			/* index into new_value[] */
    char new_value[MAX_TOKEN_SIZE]; /* working copy of new_value[] */
    char last_char;

    last_char = ' ';
    new_value[0] = '\0';		/* initialize to remove lint warning */

    for (m = 0, k = 0; current_value[k] != '\0'; ++k)
    {				/* squeeze out spaces around hyphens */
				/* and convert hyphen runs to en-dashes */
	if (current_value[k] == '-')
	{			/* convert hyphens to en-dash */

	    for ( ; (m > 0) && Isspace(new_value[m-1]) ; )
		--m;		/* discard preceding spaces */

	    for ( ; current_value[k+1] == '-'; )
		++k;

	    for ( ; Isspace(current_value[k+1]); )
		++k;		/* discard following spaces */

	    new_value[m++] = (char)'-'; /* save an en-dash */

	    /* [04-Mar-1996] force en-dash between digit pairs,
	       alpha pairs, and digit-alpha pairs, but not otherwise,
	       so ``pages = "A-3-A-7"'' is converted to
	       ``pages = "A-3--A-7"''. */

	    if ( (Isdigit(last_char) && Isdigit(current_value[k+1])) ||
		 (Isalpha(last_char) && Isalpha(current_value[k+1])) ||
		 (Isdigit(last_char) && Isalpha(current_value[k+1])) ||
		 (current_value[k+1] == '?') )
		new_value[m++] = (char)'-';
	}
	else
	{
	    new_value[m++] = current_value[k];

	    if (!Isspace(current_value[k])) /* remember last non-blank non-hyphen char */
		last_char = current_value[k];
	}
    }
    new_value[m] = (char)'\0';
    (void)strcpy(current_value,new_value);
}


#if defined(HAVE_STDC)
static char *
fix_author_degrees(char *author)
#else /* K&R style */
static char *
fix_author_degrees(author)
char *author;
#endif
{
    int b_level;			/* brace level */
    size_t a;				/* index in author[] */
    size_t n;				/* index in name[] */
    char *name;				/* synonym for output author, overwriting input */
					/* with possibly shorter string */
    if (fix_degrees == NO)
	return (author);

    /*
    ** Convert "J. J. {Thomson, M. A., F. R. S.}" to "J. J. {Thomson, M.A., F.R.S.}",
    ** "Frederick {Soddy, B. A. (Oxon.)}" to "Frederick {Soddy, B.A. (Oxon.)}",
    ** and "Otto {Hahn, Ph. D.}" to "Otto {Hahn, Ph.D.}".
    ** That is, at nonzero brace level only, discard spaces after
    ** a period following a letter, but preserve one space before an
    ** open parenthesis.  The output overwrites the input, so the
    ** string length is never permitted to grow.
    */
    for (b_level = 0, a = 0, n = 0, name = author; /* NO-OP (exit below) */ ; ++a, ++n)
    {
	name[n] = author[a];		/* copy character */

	if (author[a] == '\0')
	    break;			/* here's the loop exit */

	switch (author[a])
	{
	case '{':
	    b_level++;
	    break;

	case '}':
	    b_level--;
	    break;

	case '.':
	    if ( (b_level > 0) && (n > 0) && Isalpha(name[n-1]) )
	    {
		int c;

		c = 0;
		while (Isspace(author[a + 1])) /* flush space after "[A-Z][.]" */
		{
		    a++;
		    c++;
		}

		if ( (c > 0) && (author[a + 1] == '(') )
		    name[++n] = ' ';
	    }
	    break;

	default:
	    break;
	}
    }

    return (name);
}


#if defined(HAVE_STDC)
static char *
fix_periods(char *author)
#else /* K&R style */
static char *
fix_periods(author)
char *author;
#endif
{
    int b_level;			/* brace level */
    size_t a;				/* index in author[] */
    size_t n;				/* index in name[] */
    char *name = shared_string;		/* memory-saving device */

    if (fix_initials == NO)
	return (fix_author_degrees(author));

    /* Convert "J.K. Smith" to "J. K. Smith" if "." at brace level 0 */

    for (b_level = 0, a = 0, n = 0; /* NO-OP (exit below) */ ; ++a, ++n)
    {
	name[n] = author[a];		/* copy character */

	if (author[a] == '\0')
	    break;			/* here's the loop exit */

	switch (author[a])
	{
	case '{':
	    b_level++;
	    break;

	case '}':
	    b_level--;
	    break;

	case '.':
	    if (b_level == 0)
	    {
		if ( (a > 0) && Isupper(author[a-1]) && Isupper(author[a+1]) )
		    name[++n] = (char)' '; /* supply space between initials */
	    }
	    break;

	default:
	    break;
	}
    }

    return (fix_author_degrees(name));
}


void
fix_title(VOID)				/* protect upper-case acronyms */
{
    YESorNO brace_token;		/* YES: brace current token (uppercase content) */
    int b_level;			/* brace level */
    int ignore_level;			/* brace level whose protection is ignored */
    size_t k;				/* index into s[] */
    size_t nc;				/* number of printable characters in current token */
    YESorNO need_brace;			/* YES: need wrapping braces */
    YESorNO font_like;			/* YES: in {\em font-like group} */
    YESorNO in_macro_macro;		/* YES: in macro name */
    char *s = shared_string;		/* memory-saving device */
    size_t t;				/* index into current_value[] */
    size_t u;				/* index into current_value[] */

    static const char * accent_chars = "Hbcdrtuv`'^\"~=";

    if (current_value[0] != '\"')
	return;				/* leave macros alone */

    if (fix_accents == YES)
	fix_accent_bracing();

    if (fix_braces == YES)
	fix_bracing();

    if (fix_math == YES)
	fix_math_spacing();

    (void)memset(s, 0, 255);		/* DEBUGGING ONLY! */

    for (b_level = 0, brace_protect = NO, brace_token = NO, ignore_level = 0, k = 0, nc = 0, t = 0;
	 current_value[t] != '\0'; )
    {
	size_t i;			/* offset in current_value[] */
	int start_level;		/* starting brace level */

	switch (current_value[t])
	{
	case ' ':			/* FALL THROUGH */
	case ',':			/* FALL THROUGH */
	case ':':			/* FALL THROUGH */
	case ';':
	    COPY_1();
	    brace_token = NO;
	    nc = 0;
	    break;

	case '{':
	    if ((t == 0) || (current_value[t - 1] != '\\'))
		b_level++;

	    if ( (current_value[t + 1] == '\\') && (b_level == 1) )
	    {
		ignore_level = b_level;
		font_like = YES;
	    }
	    else
		font_like = NO;

	    /*
	    ** BibTeX uses braces to protect words from downcasing.
	    ** However, it views a backslashed word beginning a braced
	    ** group as a font change, and the braces then do NOT
	    ** protect against downcasing, EXCEPT that the initial
	    ** backslashed word IS protected.  Thus, "{\TeX Book}"
	    ** gets converted to "{\TeX book}", but "{{\TeX Book}}" is
	    ** left intact.
	    **
	    ** Scan over braced group, and supply extra surrounding
	    ** braces if the group contains uppercase letters at
	    ** brace-level 1.  However, additional braces are not
	    ** needed if the braced group contains only a single macro
	    ** name, or there are no uppercase letters after the macro
	    ** name, because BibTeX preserves letter case in the
	    ** font-change-like macro.  However, further bracing
	    ** inside a font-change-like group does NOT protect
	    ** against downcasing: outer braces are needed.
	    **
	    ** Here are some examples where correction is not needed:
	    **
	    **    "{\TeX}"    			-> unchanged
	    **    "{\TeX book}"			-> unchanged
	    **    "{\H{o} is a long accent}"	-> unchanged
	    **
	    ** These examples need additional outer braces:
	    **
	    **    "\TeX"			-> "{\TeX}"
	    **    "{\em Bose--Einstein}"	-> "{{\em Bose--Einstein}}"
	    **    "{\H{O} is a long accent}"	-> "{{\H{O} is a long accent}"
	    */

	    for (start_level = b_level, u = t + 1; current_value[u] != '\0'; ++u)
	    {
		if ( (current_value[u - 1] != '\\') &&
		     (current_value[u] == '{') )
		    b_level++;
		else if ( (current_value[u - 1] != '\\') &&
			  (current_value[u] == '}') )
		{
		    b_level--;

		    if (b_level < start_level)
			break;
		}
		else if (Isupper(current_value[u]) && (b_level == ignore_level) )
		    brace_token = YES;
	    }

	    if (current_value[u] == '\0')
	    {
		u--;

		if (current_value[u] == '\"')
		    u--;

		if (b_level != 0)
		    warning("Unbalanced brace(s) in value ``%v''");
	    }

	    if ( (b_level == 0) && (brace_token == YES) )
	    {
		size_t v;			/* index into current_value[] */

		/* Rescan the group to see whether extra braces are needed */

		for (font_like = NO, in_macro_macro = YES, v = t + 2; v <= u; ++v)
		{
		    if (!Isalpha(current_value[v]))
			in_macro_macro = NO;

		    if ( (in_macro_macro == NO) && Isupper(current_value[v]) )
			font_like = YES;
		}
	    }

	    if ( (b_level == 0) && (brace_token == YES) && (font_like == YES) )
		STORE_CHAR('{');

	    while (t <= u)
		COPY_1();

	    if ( (b_level == 0) && (brace_token == YES)  && (font_like == YES) )
		STORE_CHAR('}');

	    if (b_level < ignore_level)
		ignore_level = 0;

	    brace_token = NO;
	    nc = 0;
	    break;

	case '}':
	    if ((t == 0) || (current_value[t - 1] != '\\'))
	    {
		b_level--;
		ignore_level = 0;
	    }

	    COPY_1();
	    break;

	case '\\':
	    if ( ( (b_level == 0) || (b_level == ignore_level) ) &&
		 Isupper(current_value[t + 1]) )
		need_brace = YES;
	    else
		need_brace = NO;

	    if ( IN_SET(accent_chars, current_value[t + 1]) && !Isalpha(current_value[t + 2]) )
	    {
		if (need_brace == YES)
		    STORE_CHAR('{');

		COPY_2();			/* e.g., "\H" followed by "{o}" */

		while ( Isspace(current_value[t]) && Isspace(current_value[t + 1]) )
		    t++;			/* discard superfluous space */

		if ( Isspace(current_value[t]) && Isalpha(current_value[t + 1]) )
		{
		    t++;			/* discard space after accent macro */
		    STORE_CHAR('{');		/* and brace single letter to be accented */
		    COPY_1();
		    STORE_CHAR('}');
		}
		else if (Isalpha(current_value[t - 1]))
		{
		    if ( (current_value[t + 0] == '{') &&
			 Isalpha(current_value[t + 1]) &&
			 (current_value[t + 2] == '}') )
			COPY_3();
		}
		else			/* non-letter accent */
		{
		    if ( (current_value[t + 0] == '{') &&
			 Isalpha(current_value[t + 1]) &&
			 (current_value[t + 2] == '}') &&
			 (need_brace == YES) )
		    {			/* discard superfluous braces */
			t++;
			COPY_1();
			t++;
		    }
		}

		if (need_brace == YES)
		    STORE_CHAR('}');
	    }
	    else if (current_value[t + 1] == '$')	/* literal dollar */
		COPY_2();
	    else if (Isupper(current_value[t + 1]))	/* e.g., "\O", "\OE", "\AA", ... */
	    {
		COPY_1();
		brace_token = YES;
	    }
	    else
		COPY_1();

	    break;

	case '$':
	    /*
	    ** Isolate the math mode text, checking it for uppercase
	    ** letters, and then output the entire math text
	    ** immediately, with outer braces if needed.
	    **
	    ** As part of our cleanup operations, we collapse
	    ** adjacent normal math modes into a single one:
	    **     "$R$$^3$" -> "$R^3$"
	    */

	    i = (current_value[t + 1] == '$') ? 1 : 0;
	    ignore_level = 0;

	    for (u = t + 1 + i; current_value[u] != '\0'; ++u)
	    {
		if ( (current_value[u + 0] == '$') &&
		     (current_value[u + i] == '$') )
		{
		    u += i;			/* point to final dollar */

		    if ( (i == 0) && (current_value[u + 1] == '$') )
			u++;			/* scan over adjacent math modes */
		    else
			break;
		}

		if ( (current_value[u - 1] != '\\') &&
		     (current_value[u] == '{') )
		{
		    b_level++;

		    if ( (current_value[u + 1] == '\\') && (b_level == 1) )
			ignore_level = b_level;
		}
		else if ( (current_value[u - 1] != '\\') &&
			  (current_value[u] == '}') )
		{
		    b_level--;
		    ignore_level = 0;
		}

		if ( ((b_level == 0) || (b_level == ignore_level) ) &&
		     Isupper(current_value[u]) )
		    brace_token = YES;
	    }

	    if (current_value[u] == '$')
	    {
		size_t k_start;

		k_start = k;

		if (brace_token == YES)
		    STORE_CHAR('{');

		COPY_1();		/* copy dollar */

		if ( ( k >= 5) &&
		     (s[k - 5] != '$') &&
		     (s[k - 4] == '$') &&
		     (s[k - 3] == '}') &&
		     (s[k - 2] == '{') &&
		     (s[k - 1] == '$') &&
		     (s[k + 0] != '$') )
		    k -= 4;		/* discard "$}{$" to join adjacent braced math groups */

		while (t <= u)
		{
		    if ( (i == 0) &&
			 ((t + 1) < u) &&
			 (current_value[t] == '$') &&
			 (current_value[t + 1] == '$') )
			t += 2;		/* discard embedded "$$" in adjacent normal math mode */

		    COPY_1();
		}

		if (brace_token == YES)
		    STORE_CHAR('}');

		nc += (k - k_start);

	        /* Do NOT reset brace_token = NO: see next line */
		/* Do NOT reset nc = 0: it fouls up bracing in "Cr$_2$O$_3$" */
	    }
	    else	     /* error: output remainder of field unprocessed */
	    {
		warning("Unclosed math mode in value ``%v''");

		while (current_value[t] != '\0')
		    COPY_1();
	    }

	    break;

	default:
	    if ( ( (b_level == 0) || (b_level == ignore_level) ) &&
		 Isupper(current_value[t]))
	    {
		/*
		** No bracing needed:
		**     "Three Men In A Boat"
		**     "A Treatise on Gnats"
		**
		** Bracing needed:
		**     "C Programming"		-> "{C} Programming"
		**     "OaSiS: Oat-rich {Sicilian} Sandwiches" -> "{OaSis}: ..."
		**     "X11 Window System"	-> "{X11} Window System"
		**     "XY Plotter Manual"	-> "{XY} Plotter Manual"
		**
		** Bracing that must be supplied by a sensible human,
		** because it cannot be done by a mindless computer
		** program:
		**
		**     "Bose--Einstein statistics"  -> "{Bose--Einstein} statistics"
		**     "Spin-Polarized Photons"	    -> unchanged
		**     "Type A People"              -> "{Type A} People"
		*/

		if ((t > 0) && (current_value[t - 1] == '-'))
		    /* NO-OP */;		/* ignore, e.g., "P" in "Spin-Polarized Photons" */
		else if (nc > 1)		/* uppercase inside word */
		    brace_token = YES;		/* e.g, "aBc" */
		else if (Isupper(current_value[t + 1]))
		    brace_token = YES;		/* "XY" -> "{XY}" */
		else if (Isdigit(current_value[t + 1]))
		    brace_token = YES;		/* "X11" -> "{X11}" */
		else if ( Isspace(current_value[t + 1]) ||
			  IN_SET("}\"", current_value[t + 1]) )	/* end of 1-char token */
		{
		    if (current_value[t] != 'A')
			brace_token = YES; /* "C Programming" -> "{C} Programming" */
		}
	    }

	    COPY_1();
	    nc++;

	    if (current_value[t] == '\0')
		brace_protect = NO;

	    if ( (brace_token == YES) && (brace_protect == YES) && (b_level == 0) )
	    {			/* Convert XWS to {XWS} and X11 to {X11} */
		/*
		** bibclean versions 2.15 and earlier braced partial words, e.g.,
		**
		**	"FORTRAN-to-C"	-> "{FORTRAN}-to-{C}"
		**	"e-CON 2011"	-> "e-{CON} 2011"
		**	"EXcelENt"	-> "{EX}cel{EN}t"
		**	"ExCeLeNt"	-> "ExCeLeNt"		<-- unbraced
		**
		** with code like this:
		**
		**     while (Isupper(current_value[t]) || Isdigit(current_value[t]))
		**         COPY_1();
		**
		** bibclean versions 2.16 and later brace complete
		** blank-separated tokens, e.g.,
		**
		**	"FORTRAN-to-C"	-> "{FORTRAN-to-C}"
		**	"e-CON 2011"	-> "{e-CON 2011}"
		**	"EXcelENt"	-> "{EXcelENt}"
		**	"ExCeLeNt"	-> "{ExCeLeNt}"
		**
		** but it takes considerably more code to supply that
		** feature.
		*/

		if (k > 1)		/* token starts inside string */
		{
		    size_t m;		/* index into s[] */

		    for (m = k - 1; (m > 0) &&
			     !Isspace(s[m]) &&
			     !IN_SET("${})>]", s[m]); --m)
			/* NO-OP */;

		    if (Isspace(s[m]) ||
			(s[m] == '\"') ||
			(m == 0) ||
			IN_SET("${})>]", s[m]))
		    {
			(void)memmove(&s[m + 2], &s[m + 1], k - m);
			s[m + 1] = '{';
			k++;			/* because we inserted a brace in s[] */
		    }
		    else
			STORE_CHAR('{');
		}
		else
		    STORE_CHAR('{');

		while (current_value[t] != '\0')
		{   /* copy remaining token to next blank or end of math or
		       end of string */
		    if (Isspace(current_value[t]))
			break;
		    else if ( (current_value[t + 0] == '$') &&
			      (t > 0) &&
			      (current_value[t - 1] != '\\') )
			break;
		    else if ((current_value[t + 0] == '\"') &&
			     (current_value[t + 1] == '\0') )
			break;
		    else
			COPY_1();
		}

		/*
		** Conference title acronyms with a following optional
		** apostrophe and year are common ("e-CON 2011" and
		** "ARITH '20"), as are computer standards ("IEEE 754"),
		** and programs and their versions ("GCC 4.8-20120401").
		** Thus, we include any following token of that form
		** inside the braces.  Token collection stops before
		** selected punctuation, whitespace, or end of string.
		*/

		if ((current_value[t] != '\0') &&
		    Isspace(current_value[t]) &&
		    (Isdigit(current_value[t + 1]) || (current_value[t + 1] == '\'')))
		{
		    COPY_1();

		    while ( (current_value[t] != '\0') &&
			    (current_value[t] != '$') &&
			    !Isspace(current_value[t]) &&
			    !IN_SET("\",;", current_value[t]) )
		    {
			if ( (current_value[t + 0] == '\\') &&
			     (current_value[t + 1] != '$') )
			    COPY_2();
			else
			    COPY_1();
		    }
		}

		if ( (k > 0) && IN_SET(".,:;", s[k - 1]) )
		{   /* move close brace before final punctuation */
		    char c;

		    c = s[--k];
		    STORE_CHAR('}');
		    STORE_CHAR(c);
		}
		else
		    STORE_CHAR('}');

		brace_token = NO;
		nc = 0;
	    }

	    break;
	}
    }

    STORE_NUL();
    check_length(k);
    (void)strcpy(current_value,s);

    if (STREQUAL(&current_value[k - 2], ".\""))
	warning("Final period/dot/fullstop is unexpected, and often wrong, in BibTeX title ``%v''");

    if (stristr(current_value, "...") != (char *)NULL)
	warning("Literal ellipsis ... should probably be \\ldots{} in BibTeX title ``%v''");

    if (fix_font_changes == YES)
	brace_font_changes();

}


/*@null@*/
#if defined(HAVE_STDC)
static const char *
month_token(/*@null@*/ const char *s, size_t *p_len)
#else /* K&R style */
static const char *
month_token(s, p_len)
/*@null@*/ const char *s;
size_t *p_len;
#endif
{
   /*******************************************************************
    ** Return pointer to next token in s[], with its length in *p_len if
    ** s is NULL, the parsing continues from where it was last.  A token
    ** is either a sequence of letters, possibly with a terminal period,
    ** or else a single character.  Outside quoted strings, all
    ** characters are considered non-letters. This code is vaguely
    ** modelled on Standard C's strtok() function.
    *******************************************************************/

    /*******************************************************************
    ** Bug fix for version 2.11.4 [09-May-1998]:  Prior to this version,
    ** a value
    **
    **      mar # "\slash" # apr
    **
    ** would be incorrectly transformed
    **
    **      month = mar # "\slash" # " # apr # "
    **
    ** because in_quoted_string was incorrect for the remainder of
    ** the value.
    **
    ** If the input value was changed to
    **
    **      mar # "\slash " # apr
    **
    ** then that space inside the quoted string preserved the
    ** correctness of in_quoted_string, and the output was correct.
    **
    ** For version 2.11.4, the body of this function has been entirely
    ** rewritten to simplify the logic, and ensure its correctness.
    *******************************************************************/

    static int b_level = 0;		/* remembered across calls */
    static YESorNO in_quoted_string = NO; /* remembered across calls */
    static const char *next = (const char *)NULL; /* remembered across calls */
    const char *token;			/* pointer to returned token */

    if (s != (const char*)NULL)		/* do we have a new s[]? */
    {
	next = s;			/* yes, remember it */
	b_level = 0;			/* and reinitialize state */
	in_quoted_string = NO;		/* variables */
    }

    *p_len = 0;

    if (next == (const char*)NULL)	/* then improper initialization */
	return (next);

    token = next;

    switch (*next)
    {
    case '"':
	if (b_level == 0)
	    in_quoted_string = (in_quoted_string == YES) ? NO : YES;

	break;

    case '{':				/* '}' for brace balance */
	b_level++;
	break;
					/* '{' for brace balance */
    case '}':
	b_level--;
	break;

    default:
	break;
    }

#define ADVANCE ((*p_len)++, next++)

    if ( (in_quoted_string == YES) && Isalpha(*next) )
    {			/* collect possibly-dot-terminated alphabetic token */

	while (Isalpha(*next))
	    ADVANCE;

	if (*next == '.')
	    ADVANCE;
    }
    else if (*next != '\0')		/* collect single-character token */
	ADVANCE;

#undef ADVANCE

    return ((*p_len == 0) ? (const char*)NULL : token);
}

#if defined(HAVE_STDC)
static size_t
squeeze_space(/*@null@*/ char *s)
#else /* K&R style */
static size_t
squeeze_space(s)
/*@null@*/ char *s;
#endif
{   /* squeeze superfluous space from s[], in place, and return new length */
    size_t m;

    if (s == (char *)NULL)
	m = (size_t)0;
    else
    {
	size_t k;

	for (k = 0, m = 0; s[k] != '\0'; m++)
	{
	    s[m] = s[k++];

	    if (Isspace(s[k - 1]))
	    {
		while (Isspace(s[k]))	/* discard consecutive spaces */
		    k++;
	    }
	}

	s[m] = '\0';
    }

    return (m);
}

#if defined(HAVE_STDC)
void
store_space(char *s, size_t *pk)
#else  /* K&R style */
void
store_space(s, pk)
char *s
size_t *pk;
#endif
{   /* remove any trailing space, then store a space */
    while ( ((*pk) > 0) && Isspace(s[(*pk) - 1]) )
	(*pk)--;

    s[(*pk)++] = ' ';
}
