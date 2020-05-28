/* -*-C-*- vmswild.c */
/*-->vmswild*/
/**********************************************************************/
/****************************** vmswild *******************************/
/**********************************************************************/

#include <config.h>

#if 0			/* BEGIN COMMENT SECTION */

/* Compile with TEST defined to create a stand-alone test program */


/***********************************************************************
** [09-Mar-1990]
** Edit by  Nelson H. F. Beebe <Beebe@science.utah.edu>
** Overhaul to meet these objectives:
**
**	(1) If a file is in the current directory, the device and
**	directory specification is stripped from the file name.
**
**	(2) Convert all file names in argv[] to lower case.
**
**	(3) If a generation number was specified in the command-line
**	file name, as indicated by the presence of a semicolon in the
**	wildcard specification, preserve generation numbers in the
**	output; otherwise, eliminate them, since the files must
**	correspond to the highest generation.
**
**	(4) If a filename ends in a dot, remove it ("foo." -> "foo").
**
**      (5) Reorder routines alphabetically, and supply Standard C
**      function prototype declarations.
**
**	(6) Make all functions and global variables static (private),
**      except cmd_lin(), so they do not interfere with user-defined
**      values outside this file.
**
**	(7) Remove prompting for missing arguments.
**
** These changes result in a shorter command line, which is important
** when the names are used to build other commands (e.g. in vcc),
** because VAX VMS has a very limited command line buffer size.  They
** also match what UNIX programs get from the shell command line
** expansion; this is important for programs like gawk where the
** filename may be used by the program.  UNIX programs never prompt
** for arguments, and they shouldn't do so either when ported to VAX
** VMS.  New functions added are expand_logical() and
** normalize_filename().  The body of nxt_wld() has been completely
** rewritten.
**
************************************************************************
** PDVI:VMSWILD.C, Sat Oct  1 15:59:31 1988
** Edit by  Nelson H. F. Beebe <Beebe@science.utah.edu>
** Fix erroneous leading comments, change initialization of OPoption
** so that arguments of form "{foo}" (e.g. for awk) are not lost.
**
************************************************************************
** Original version:
**  4-Mar-88 19:08:58-MST,26262;000000000000
** Return-Path: <news@cs.utah.edu>
** Received: from cs.utah.edu by SCIENCE.UTAH.EDU with TCP;
** 	Fri 4 Mar 88 19:08:46-MST
** Received: by cs.utah.edu (5.54/utah-2.0-cs)
** 	id AA20734; Fri, 4 Mar 88 19:08:52 MST
** From: news@cs.utah.edu (Netnews Owner)
** Reply-To: ERICMC%usu.BITNET@CC.UTAH.EDU (Eric McQueen)
** To: comp-os-vms@cs.utah.edu
** Subject: RE: Wild card expansion under VAX11-C: Code Wanted
** Message-Id: <8803041943.AA24229@jade.berkeley.edu>
** Date: 4 Mar 88 04:59:00 GMT
** Sender: daemon@ucbvax.berkeley.edu
**
** (sorry for the previous abbreviated copy of this)
**
** In article <5361@ames.arpa> woo@pioneer.UUCP (Alex Woo) writes:
** > How does one expand wildcards in command line arguments in
** > VAX11-C under VMS?
**
** I have a tool for C programmers using VMS, especially those porting
** facilities from Un*x.  It prompts for command line arguments (unless
** you have already specified some via a "foreign command" or MCR),
** expands file wildcards found on the command line, and redirects
** 'stdin' or 'stdout'.  Here is a sample use:
**
** int
** main( argc, argv )
**   int argc;
**   char **argv;
** {
**   !* local variables *!
** #ifdef VMS     !* Or whatever the Standard C-conforming word will be *!
** 	extern char **cmd_lin();
**
** 	argv = cmd_lin( "", &argc, argv );
** 	!* The first argument provides misc. options (not currently used). *!
** #endif !* VMS *!
** 	!* your routine *!
** }
**
** $ run grep
** _command_line_arguments: -i STOP *.for
**
** The source is 40 blocks (600 lines) so I am not sure that I should
** post the actual source in this group, but I have (see below).  If you
** are interested, please look through the documentation, try it out,
** and send any significant comments, suggestions, useful pieces of
** code, etc. to one of the addresses below.	I will post summaries to
** comp.os.vms/INFO-VAX and comp.lang.c/INFO-C.  Please excuse the
** "lived-in" look of the code but I am still working on this thing so
** this is mearly the last debugged version I have.
**
** ---
** Eric Tye McQueen	  Mathematics Department	Also at (after some
** ericmc@usu.bitnet	  Utah State University	      time in March[June?!]):
** (801) 753-4683	  Logan, Utah  84322-3900	ericmc@usu.usu.edu
**
**    UUCP:  ...{psuvax1,uunet}!usu.bitnet!ericmc	     "Doodle doodle dee"
**    Arpa:  ericmc%usu.bitnet@cunyvm.cuny.edu	      "wubba wubba wubba."
**
** +------------------------------- cmdlin.c -------------------------------+
***********************************************************************/

/***********************************************************************
 * cmdlin.c -- Un*x-style command line processing for VMS.  (V2.0,  3-Mar-1988)
 *	Copyright (C) 1988 Eric Tye McQueen
 * Emulates Un*x-style command line options in VAX C programs:
 *    - Allows entry of command line arguments after the program starts:
 *	The easiest way to run a program in VMS (the RUN command) does not
 *	allow for specification of command-line arguments while other methods
 *	that do allow arguments won't interpret the arguments in a very Un*x-
 *	like manner.  If no arguments are specified (other than the program
 *	name which is supplied by VAX C), we prompt for arguments from
 *	SYS$COMMAND and try to interpret them as Un*x would.
 *    - Expands any filename wildcards found in the command line arguments:
 *	C programs that process files are usually written to accepts several
 *	filenames, one name per argument.  Un*x environments expand filename
 *	wildcards specified on the command line to the names of all matching
 *	files and pass these names to the program.  This routine does this
 *	since VMS will not.  VMS wildcards and filenames are supported, NOT
 *	Un*x-style wildcards and filenames (this has its good and bad points).
 *    - Supports simple C-shell-style standard I/O redirection (`<', `>', `>>',
 *	`>&', and `>>&').
 *    - In the future support can be added for strings and quoting of charac-
 *	ters (`\', "'", `"'), symbol substitution (`$'), parallel processing
 *	and pipes (`||', `&&', etc.), etc.
 *
 * Currently supported:
 *	All VMS wildcards (`*', `%', and `...').
 *	Both VMS and Un*x-style file names.
 *	I/O redirection via `<', `>', `>>', `>&', and `>>&'
 *	    (watch out for people who use `<' and `>' for directory names)
 * Reasonable to add support for:
 *	Specification of cmd_lin() options between `{' and `}'.
 *	Continuation lines via \ at end of line.
 *	Inclusion of spaces (and newlines) into arguments via ' and ".
 *	Substitutions from the VMS symbol table using ${name}.
 *	Using Un*x-style directories (/) with wildcards.
 * Pipe dreams:
 *	Pipes via || and &&.
 *	Un*x-style wildcards like [char-set].
 *	    (watch out for people who use `[' and `]' for directory names!!)
 *
 * This files also contains a special demonstration program.  To use it type:
 *	$ cc cmdlin/define:example
 *	$ link cmdlin,sys$input:/opt
 *	sys$share:vaxcrtl/share
 *	$ run cmdlin
 *
 * Sample use:
 *	...
 *	int
 *	main( argc, argv )
 *	  int argc;
 *	  char **argv;
 *	{
 *	  ... (definitions)
 *	#ifdef VMS
 *	  extern char **cmd_lin();
 *		argv = cmd_lin( "", &argc, argv );
 *	#endif
 *		... (other statements)
 *	}
 *	...
 *	$ run grep
 *	_command_line_arguments: -i stop *.for
 *
 *	If you specify something that looks like a VMS wildcard (that contains
 * "*", "%", or "...") but that doesn't match the name of any existing files,
 * cmd_lin() will write "<pattern>: No match." to `stderr'.  If you specify a
 * string containing "*", "%", or "..." that is not a valid VMS file wildcard,
 * cmd_lin() will also write "<pattern>: Invalid file wildcard." to `stderr'.
 * In either case, the original string will be included, unchanged, as a
 * command line argument.
 ***********************************************************************/

#endif			/* END COMMENT SECTION */

#if 0
static char version[] = /* (Not currently) printed inside "debug" option. */
  "cmd_lin() V2.0, Copyright (C) 1988 Eric Tye McQueen (3-Mar).";
#endif /* 0 */

/*@unused@*/ static char rcsid[] = "$Id: vmswild.c,v 1.5 2013/11/30 19:17:27 beebe Exp beebe $";


#ifndef VMS	/* Should be changed to Standard C-conforming name soon. */

/* Dummy version of cmd_lin() for non VAX C (non-VMS) systems: */

char **
#if defined(HAVE_STDC)
cmd_lin(
/*@unused@*/ char	*opt0,		/* Options specified in `main()' */
/*@unused@*/ int	*ainpc,		/* &main\argc (input and output) */
char	**inpv				/* main\argv (input) */
)
#else /* NOT defined(HAVE_STDC) */
cmd_lin( opt0, ainpc, inpv )
/*@unused@*/ char	*opt0;		/* Options specified in `main()' */
/*@unused@*/ int	*ainpc;		/* &main\argc (input and output) */
char	**inpv;				/* main\argv (input) */
#endif /* defined(HAVE_STDC) */
{
    return( inpv );
}

#else  /* VMS */

#include <stdio.h>	/* stdin stdout stderr */

#if defined(HAVE_CTYPE_H)
#include <ctype.h>
#endif

#if defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#if defined(HAVE_STRING_H)
#include <string.h>
#endif

typedef	 char  bool;		/* Smallest addressable signed data type. */
typedef	 unsigned char	 uchar;
typedef	 unsigned short	 ushort;
typedef	 unsigned int	 uint;
typedef	 unsigned long	 ulong;

#define	  odd(stat)   ( (stat) & 1 )

/* General VMS descriptor (not as clumsy as $DESCRIPTOR from <descrip.h>): */
struct descr {
	ushort leng;	/* Length of data area (or string). */
	uchar  type;	/* Type of data in area (dsc$k_dtype_t). */
	uchar  class;	/* Class of descriptor (dsc$k_class_s). */
	char  *addr;	/* Address of start of data area. */
};
globalvalue dsc$k_dtype_t; /* Text (data type) */
globalvalue dsc$k_class_s; /* Static (class) descriptor */

/* To allocate a descriptor (dsc) for array of char (arr): */
#define	  desc_arr(dsc,arr)	struct descr dsc = \
  { (sizeof arr)-1, dsc$k_dtype_t, dsc$k_class_s, arr }

/* To allocate a descriptor (dsc) for null-terminated string (str): */
#define	  desc_str(dsc,str)	struct descr dsc = \
  { strlen(str), dsc$k_dtype_t, dsc$k_class_s, str }

#define	  FNAMSIZ  1024 /* size of filename buffers (+1 for '\0') */

/* Options that are set by an argument like "{opt1,!opt2,noopt3,opt4=val}":
 * (none of these are really supported in this release)
 *	help : show possible options then terminate execution.
 *	option : allows options to be specified (default).
 *	expand : expand VMS-style wildcards (default).
 *	prompt [ = string ] : prompt if no arguments are present (default).
 *	append : prompt if some arguments are initially present.
 *	debug : show option changes and final command line arguments.
 *	redirect : allows `<', `>', etc. to be used to redirect `std*'.
 *	unixy : don't return VMS-style device or directory names (default).
 *	lower : convert letters in file specifications to lowercase (default).
 *	device = "never", "different" (default), "always" : show device name.
 *	directory = "never", "different" (default), "always" : show directory.
 *	version = "never", "different" (default), "always" : show version.
 *	`dev':/`path' : Un*x-like alias for device name (ex: "user$disk:/usr").
 * Interactions:
 *	unixy & show device -> show directory
 *	unixy & !device & directory -> directory is relative (../etc)
 *	unixy & !device & directory=always -> default = ./
 */

/* Key values for OTfreq options (device, directory, and version): */
enum { never=0, diff, always };

/* Current values of all options: */
#if 0
char	*OPprompt	= "_command_line_arguments: ";
#else /* 1 */
char	*OPprompt	= (char*)NULL;
#endif /* 0 */

#if 0
/* Disable; otherwise an awk option string like "{print $0}" disappears. */
static char	*OPoption	= "{}";
#else  /* NOT 0 */
static char	*OPoption	= "";
#endif /* 0 */

static bool	 OPexpand	= 1;
static bool	 OPappend	= 0;
static bool	 OPdebug	= 0;
static bool	 OPredir	= 1;
static bool	 OPunixy	= 1;
static bool	 OPlower	= 1;
static bool	 OPdevice	= diff;
static bool	 OPdirect	= diff;
static bool	 OPversion	= diff;

char		**cmd_lin(char *opt0,int *ainpc,char **inpv);

/* Private functions */
static char	**add_arg(char *arg, int *acnt, char **ptrs);
static uint	crelnm(char *table, char *name, char *value);
static void	*emalloc(unsigned siz);
static void	*erealloc(void *ptr, unsigned siz);
static char	*expand_logical(char *logname);
static bool	is_wild(char *name);
static char	*nameonly(char *file);
static char	*normalize_filename(char *filename);
static char	*nxt_wld(char *wild);
static char	**parse_opt(int *ainpc,char **inpv);
static char	**read_cmd_lin(int *ainpc,char **inpv);
static char	*read_w_prompt(char *prompt, char *buff, int size);
static char	**redir(int *ainpc,char **inpv,FILE *fp,char *io,
			char *tok,char *lnm,char *acc);
static char	**redirin(int *ainpc,char **inpv);
static char	**redirout(int *ainpc,char **inpv);
static void	sigvms(uint stat);
static char	*strsub(char *str, char *sub);

#if defined(__ALPHA)
void	lib$stop(int status_);
#endif

#ifdef TEST
static void	dump(char *str);
int		main(int argc,char **argv);
#endif /* TEST */

char **
#if defined(HAVE_STDC)
cmd_lin(
char	*opt0,				/* Options specified in `main()' */
int	*ainpc,				/* &main\argc (input and output) */
char	**inpv				/* main\argv (input) */
)
#else /* NOT defined(HAVE_STDC) */
cmd_lin( opt0, ainpc, inpv )		/* returns new value for main\argv */
char	*opt0;				/* Options specified in `main()' */
int	*ainpc;				/* &main\argc (input and output) */
char	**inpv;				/* main\argv (input) */
#endif /* defined(HAVE_STDC) */
{
    int endc;	/* `endc' and `endv' will be `argc' and `argv' for main() */
    char **endv = add_arg((char*)NULL,&endc,(char**)NULL);
					/* Initialize `add_arg()' structures */
    char **xtra = 0;	/* So we can free() what read_cmd_lin() may allocate */
    char *cp;

	if(  OPoption[0] == *opt0  )
		inpv = parse_opt( ainpc, inpv );
#ifdef DEBUG
	else if(  *opt0	 )
	{
	    globalvalue ss$abort;
	    (void)fprintf( stderr, "%s (%s) %s.\n", "Invalid options string",
		    opt0, "specified in `main()'" );
	    exit( ss$abort );
	}
#endif /* DEBUG */
	if(  *ainpc  )
	{			/* Process `argv[0]', the program name: */
	    if(  OPunixy  ) /* Remove VMS device/dir/version: */
		*inpv = nameonly( *inpv );	/* (and ".ext") */
	    endv = add_arg( *inpv, &endc, endv );
	    --*ainpc;
	    ++inpv;
	}
	else		/* (Impossible?)  No `argv[0]' so make one up: */
	    endv = add_arg( "Me", &endc, endv );
	if (  OPappend  ||  ( !*ainpc && OPprompt )  )
		xtra = inpv = read_cmd_lin( ainpc, inpv );
	while (	*ainpc	)
	{
		if(  OPoption  &&  OPoption[0] == **inpv  )
			inpv = parse_opt( ainpc, inpv );
		else if(  OPredir  &&	 '<' == **inpv	)
			inpv = redirin( ainpc, inpv );
		else if(  OPredir  &&	 '>' == **inpv	)
			inpv = redirout( ainpc, inpv );
		else if(  OPexpand  &&  is_wild( *inpv )  )
		{
			if(  ( cp = nxt_wld(*inpv) )  )
			{
			    do
			    {
				endv = add_arg( cp, &endc, endv );
			    } while(  ( cp = nxt_wld(*inpv) )  );
			}
			else
			{
			    (void)fprintf( stderr, "%s: No match.\n", *inpv );
			    endv = add_arg( *inpv, &endc, endv );
			}
		}
		else
		    endv = add_arg( *inpv, &endc, endv );
		--*ainpc;
		++inpv;
	}
	if (  xtra  )	/* An extra block of pointers was malloc()ed */
	    FREE( xtra );
	*ainpc = endc;

	return( endv );
}

/**********************************************************************/
/*                     Private functions                              */
/**********************************************************************/

/* Builds argc,argv type lists of strings: */
static char **
#if defined(HAVE_STDC)
add_arg(
  char *arg,	/* Argument to be added to the argument list */
  int *acnt,	/* Number of arguments currently in the argument list */
  char **ptrs	/* Array of pointers to the arguments */
)
#else /* NOT defined(HAVE_STDC) */
add_arg( arg, acnt, ptrs )
  char *arg;	/* Argument to be added to the argument list */
  int *acnt;	/* Number of arguments currently in the argument list */
  char **ptrs;	/* Array of pointers to the arguments */
#endif /* defined(HAVE_STDC) */
{
# define ARGRP 64	/* number of string pointers allocated at a time */
	if(  !ptrs  ) { /* Initialize a new argument list: */
	  char **v = emalloc( ARGRP * sizeof(char *) );
		*acnt = 0;
		v[0] = 0;
		return( v );
	}
	if(  0 == (*acnt+1) % ARGRP  ) /* Need more space for pointers */
		ptrs = erealloc( ptrs, (*acnt+1+ARGRP) * sizeof(char *) );
	ptrs[*acnt] = arg;
	ptrs[++*acnt] = (char*)NULL;
	return( ptrs );
}

/* Creates a logical name. */
static uint				/* Returns a VMS condition value. */
#if defined(HAVE_STDC)
crelnm(
  char *table,
  char *name,
  char *value
)
#else /* NOT defined(HAVE_STDC) */
crelnm( table, name, value )
  char *table;
  char *name;
  char *value;
#endif /* defined(HAVE_STDC) */
{
  desc_str( tdsc, table );
  desc_str( ndsc, name );
  struct itemstr {
    ushort buflen, type;
    char *addr;
    ushort *retlen;
    uint end;
  } itemrec;
  /* globalvalue psl$k_user; */ /* Change this to "#include psldef". */
  uchar mode = 3 /* psl$k_user */;
  extern uint sys$crelnm();
  /* globalvalue lnm$_string; */ /* Change this to "#include lnmdef". */
	itemrec.type	= 2 /* lnm$_string */;
	itemrec.buflen	= strlen(value);
	itemrec.addr	= value;
	itemrec.retlen	= (ushort *) 0;
	itemrec.end	= 0;
	return(	 sys$crelnm( 0, &tdsc, &ndsc, &mode, &itemrec )	 );
}

/* Same as malloc() except never returns NULL. */
static void *
#if defined(HAVE_STDC)
emalloc(
  unsigned siz
)
#else /* NOT defined(HAVE_STDC) */
emalloc( siz )
  unsigned siz;
#endif /* defined(HAVE_STDC) */
{
  globalvalue ss$_insfmem;	/* Insufficient dynamic memory error */
  register void *p = malloc( siz );
	if(  !p	 )	/* Don't try to recover after allocating too much: */
		sys$exit( ss$_insfmem );	/* Don't recover */
	return( p );
}

/* Same as realloc() except never returns NULL. */
static void *
#if defined(HAVE_STDC)
erealloc(
  void *ptr,
  unsigned siz
)
#else /* NOT defined(HAVE_STDC) */
erealloc( ptr, siz )
  void *ptr;
  unsigned siz;
#endif /* defined(HAVE_STDC) */
{
  globalvalue ss$_insfmem;	/* Insufficient dynamic memory error */
  register void *p = realloc( ptr, siz );
	if(  !p	 )	/* Don't try to recover after allocating too much: */
		sys$exit( ss$_insfmem );	/* Don't recover */
	return( p );
}

static char*
#if defined(HAVE_STDC)
expand_logical(
char	*logname
)
#else /* NOT defined(HAVE_STDC) */
expand_logical(logname)		/* return recursively expanded logical name */
char	*logname;
#endif /* defined(HAVE_STDC) */
{
    char	*p;
    char	*colon;

    colon = strchr(logname,':'); /* trim string before colon for getenv() */
    if (colon)
	*colon = '\0';
    p = getenv(logname);
    if (colon)
	*colon = ':';

    return ((p == (char*)NULL) ? logname : expand_logical(p));
}


static bool	/* Returns 1 if `name' looks like a wild-carded file name. */
#if defined(HAVE_STDC)
is_wild(
  char *name
)
#else /* NOT defined(HAVE_STDC) */
is_wild( name )
  char *name;
#endif /* defined(HAVE_STDC) */
{
	return(strchr(name,'*')	 ||  strchr(name,'%')  ||  strsub(name,"..."));
}

/* Usually used to simplify argv[0]. */
static char *
#if defined(HAVE_STDC)
nameonly(
  char *file
)
#else /* NOT defined(HAVE_STDC) */
nameonly( file )
  char *file;
#endif /* defined(HAVE_STDC) */
{
  char *cp = file, *tp;
#ifdef VMS
	tp = strrchr(cp,']');		/* Skip directory */
	tp = tp ? tp+1 : cp;
	cp = strrchr(tp,'>');		/* Skip alternate form of directory */
	cp = cp ? cp+1 : tp;
	tp = strrchr(cp,':');		/* Skip device and/or node */
	tp = tp ? tp+1 : cp;
	cp = tp;
	if(  tp = strchr(cp,';')  )	/* Remove version number */
		*tp = '\0';
	if(  tp = strchr(cp,'.')  )	/* Remove extension and/or... */
		*tp = '\0';		/*  alternate form for version */
#else  /* NOT VMS */
	/* cp = strrchr(cp,'/'); may be useful on other than MS-DOS */
#ifdef MSDOS	/* Like this would ever be run under MS-DOS! */
	tp = strrchr(cp,'/');		/* Skip Un*x path */
	tp = tp ? tp+1 : cp;
	cp = strrchr(tp,'\\');		/* Skip MS-DOS path */
	cp = cp ? cp+1 : tp;
	if(  tp = strchr(cp,'.')  )
		*tp = '\0';		/* Remove extension */
#endif /* MSDOS */
#endif /* VMS */
	return( cp );
}

static char*
#if defined(HAVE_STDC)
normalize_filename(
char	*filename		/* with logical names expanded */
)
#else /* NOT defined(HAVE_STDC) */
normalize_filename(filename)	/* return lower-case copy of normalized name */
char	*filename;		/* with logical names expanded */
#endif /* defined(HAVE_STDC) */
{
    char	*logname;
    char	*p;
    char	*q;

    p = strchr(filename,':');	/* do we have a logical name? */
    if (p != (char*)NULL)
    {				/* yes, have to translate it interactively */
	logname = expand_logical(filename);
	q = emalloc(strlen(p+1) + strlen(logname) + 1);
	(void)strcpy(q,logname);
	(void)strcat(q,p+1);
    }
    else			/* no, just copy bare file name */
	q = strcpy(emalloc(strlen(filename)),filename);

    for (p = q ; *p; ++p)	/* convert filename to lowercase */
	*p = Isupper(*p) ? tolower(*p) : *p;

				/* remove remnants of rooted logical names */
    for (p = strsub(q,".]["); p; p = strsub(q,".]["))
	(void)strcpy(p+1,p+3);

    return (q);
}


/* Returns the next file name matching the specified wildcard. */
static char *	/* Returns a pointer to an malloc()ed expanded filename */
#if defined(HAVE_STDC)
nxt_wld(
  char	*wild	/* Wildcard to be expanded. */
)
#else /* NOT defined(HAVE_STDC) */
nxt_wld( wild ) /* Returns 0 if all matching filenames have been returned. */
  char	*wild;	/* Wildcard to be expanded. */
#endif /* defined(HAVE_STDC) */
{
    desc_str( wlddsc, wild );	/* VMS descriptor of wildcard string. */
    static uint cntxt = 0;	/* Context of wildcard search. */
    uint stat;			/* Status returned by lib$* services. */
    uint two = 2;		/* Flags to be passed by reference. */
    char file[FNAMSIZ];		/* Buffer to hold expanded file name. */
    desc_arr( fildsc, file );	/* VMS descriptor of buffer. */
    extern uint lib$find_file(), lib$find_file_end();	/* Library services. */
    globalvalue rms$_fnf, rms$_nmf, rms$_syn;	/* Possible statuses. */
    static char* cwd = (char*)NULL;
    static int lencwd = 0;
    char	*colon;
    char	*logname;
    char	*newcwd;
    char	*p;
    char	*q;

    if (cwd == (char*)NULL)		/* true only on first call */
    {
	cwd = (char*)getcwd((char*)NULL,FNAMSIZ);
	lencwd = (cwd == (char*)NULL) ? 0 : strlen(cwd);
	if (lencwd > 0)
	{
	    newcwd = normalize_filename(cwd);
	    FREE(cwd);
	    cwd = newcwd;
	    lencwd = strlen(cwd);
	}
    }

    stat = lib$find_file( &wlddsc, &fildsc, &cntxt, 0, 0, 0, &two );
    if (  rms$_syn == stat )
    {				/* File syntax error: */
	(void)fprintf( stderr, "%s: Invalid file wildcard.\n", wild );
    }
    else if (  rms$_fnf != stat  &&  rms$_nmf != stat )
    {
	/* Not "file not found" (1st try)
	 * nor "no more files" (later tries): */
	sigvms( stat ); /* Display non-trivial status messages. */
    }
    if(  !odd(stat)	 )
    {				/* Search didn't work: */
	sigvms( lib$find_file_end( &cntxt ) );
	return( 0 );
    }
    while (' ' == file[fildsc.leng-1]) /* Remove trailing spaces: */
    --fildsc.leng;
    file[fildsc.leng] = '\0';

    p = normalize_filename(file);
    (void)strcpy(file,p);
    FREE(p);
    p = (char*)NULL;
    if (strsub(wild,";") == (char*)NULL) /* strip generation numbers too */
    {
	p = strrchr(file,';');
	if (p != (char*)NULL)
	    *p = '\0';
	else
	    p = strrchr(file,'\0');
	if (p[-1] == '.')	/* trim final dot too */
	    p[-1] = '\0';
    }
    fildsc.leng = strlen(file);

    /* Strip off current working directory, if present. */
    if ((lencwd > 0) && (strncmp(cwd,file,lencwd) == 0))
    {
	(void)strcpy(file,&file[lencwd]);
	fildsc.leng -= lencwd;
    }

    return( strcpy( emalloc((unsigned)fildsc.leng+1), file )  );
}

/* Accepts argument beginning "{" and parses the options listed before the
 * closing "}". */
static char **		/* Returns new value of `inpv'. */
#if defined(HAVE_STDC)
parse_opt(
  int *ainpc,
  char **inpv
)
#else /* NOT defined(HAVE_STDC) */
parse_opt( ainpc, inpv )
  int *ainpc;
  char **inpv;
#endif /* defined(HAVE_STDC) */
{
	/* code to be added later */
	return( inpv );
}

/* Read additional command line arguments: */
static char **
#if defined(HAVE_STDC)
read_cmd_lin(
  int *ainpc,
  char **inpv
)
#else /* NOT defined(HAVE_STDC) */
read_cmd_lin( ainpc, inpv )
  int *ainpc;
  char **inpv;
#endif /* defined(HAVE_STDC) */
{
  int midc;
  char **midv, line[4096], *ap, *cp, c;
	if(  NULL == read_w_prompt( OPprompt, line, sizeof(line) )  )
		lib$stop( vaxc$errno );
	midv = add_arg( (char*)NULL, &midc, (char**)NULL );
	while(	(*ainpc)--  )
		midv = add_arg( *(inpv++), &midc, midv );
	cp = strcpy(  emalloc( strlen(line)+1 ),  line	);
	do {
		while(	Isspace( *cp )	)
			cp++;
		if(  !*( ap = cp )  )
			break;
		while(	*cp  &&	 !Isspace(*cp)	)
			cp++;
		c = *cp;   *cp++ = '\0';
		midv = add_arg( ap, &midc, midv );
	} while(  c  );
	*ainpc = midc;
	return( midv );
}

/* Prompt for a string from SYS$INPUT: */
static char *
#if defined(HAVE_STDC)
read_w_prompt(
  char *prompt, /* String to prompt user with */
  char *buff,	/* Pointer to buffer to store the string that is read */
  int size	/* Size of *buff */
)
#else /* NOT defined(HAVE_STDC) */
read_w_prompt( prompt, buff, size )
  char *prompt; /* String to prompt user with */
  char *buff;	/* Pointer to buffer to store the string that is read */
  int size;	/* Size of *buff */
#endif /* defined(HAVE_STDC) */
{
  desc_str( pdsc, prompt );
  struct descr bdsc = { size-1, dsc$k_dtype_t, dsc$k_class_s, buff };
  extern uint lib$get_command();
  ushort len;
  uint stat = lib$get_command( &bdsc, &pdsc, &len );
	if(  !odd( stat )  ) {
		errno = EIO; /* or EVMSERR? */
		vaxc$errno = stat;
		return( 0 );
	}
	buff[len] = '\0';
	return( buff );
}

/* Aspects of redirection common to both ">" (and variants) and "<": */
static char **				/* Returns new value for `inpv'. */
#if defined(HAVE_STDC)
redir(
  int *ainpc,
  char **inpv,
  FILE *fp,	/* `stdin' or `stdout' */
  char *io,	/* "in" or "out" */
  char *tok,	/* ", `<'" or " (`>', `>>', `>&', or `>>&')" */
  char *lnm,	/* logical name:  "SYS$INPUT:" or "SYS$OUTPUT:" */
  char *acc	/* file access:	 "r", "w", or "a" */
)
#else /* NOT defined(HAVE_STDC) */
redir( ainpc, inpv, fp, io, tok, lnm, acc )
  int *ainpc;
  char **inpv;
  FILE *fp;	/* `stdin' or `stdout' */
  char *io;	/* "in" or "out" */
  char *tok;	/* ", `<'" or " (`>', `>>', `>&', or `>>&')" */
  char *lnm;	/* logical name:  "SYS$INPUT:" or "SYS$OUTPUT:" */
  char *acc;	/* file access:	 "r", "w", or "a" */
#endif /* defined(HAVE_STDC) */
{
  uint stat = 0;	/* Assume we wouldn't reassign `lnm'. */
	if(  !**inpv  )		/* Not ">file" / "<file", must be... */
		--*ainpc,  ++inpv;		/* ... "> file" / "< file". */
	if(  !*inpv  ||	 !**inpv  ) {
		(void)fprintf( stderr,
		  "Invalid null %sput redirection%s.\n", io, tok );
		if(  *ainpc == 0  )	/* Fell off end of argument list: */
			++*ainpc,  --inpv;
		return( inpv );
	}
	if(  strchr( *inpv, '*' )  ||  strchr( *inpv, '%' )
	 ||  strsub( *inpv, "..." )  ) {
		(void)fprintf( stderr,
		    "Wildcards (%s) illegal in redirection%s.\n", *inpv, tok );
		return( inpv );
	}
	*strchr( lnm, ':' ) = '\0';	/* Remove trailing colon. */
	if(  strchr( *inpv, '/' )  ||  strsub( *inpv, ".." )  ) {
		if(  OPdebug  )
			(void)fprintf( stderr, "%s %s %s (%s).\n",
			 "Warning:  Can't reassign", lnm,
			 "to Un*x-style file name", *inpv );
	} else {
		stat = crelnm( "LNM$FILE_DEV", lnm, *inpv );
		if(  OPdebug  )
			sigvms( stat );
	}
	lnm[strlen(lnm)] = ':';		/* Restore trailing colon. */
	if(  !freopen( odd(stat) ? lnm : *inpv, acc, fp )  ) {
	/* if(	!( odd(stat) && freopen(lnm,acc,fp) )
	 &&  !freopen(*inpv,acc,fp)  ) { */
		(void)fprintf( stderr, "Can't open \"%s\" as `std%s'.\n",
		    *inpv, io );
		/* perror( *inpv ); */
		exit( vaxc$errno );
	}
	return( inpv );
}

/* Redirects `stdin': */
static char **
#if defined(HAVE_STDC)
redirin(
  int *ainpc,
  char **inpv
)
#else /* NOT defined(HAVE_STDC) */
redirin( ainpc, inpv )
  int *ainpc;
  char **inpv;
#endif /* defined(HAVE_STDC) */
{
	static char sysinput[] = "SYS$INPUT:";	/* must be modifiable */
	++*inpv;	/* Skip over the '<'. */
	return(redir( ainpc, inpv, stdin, "in", ", `<'", sysinput, "r" ));
}

/* Redirects `stdout': */
static char **
#if defined(HAVE_STDC)
redirout(
  int *ainpc,
  char **inpv
)
#else /* NOT defined(HAVE_STDC) */
redirout( ainpc, inpv )
  int *ainpc;
  char **inpv;
#endif /* defined(HAVE_STDC) */
{
  static char sysoutput[] = "SYS$OUTPUT:";	/* must be modifiable */
  bool err = 0;		/* Assume not ">&" nor ">>&". */
  char *acc = "w";	/* Assume ">" (write) not ">>" (append). */
	if(  '>' == *++*inpv  )		/* Append to output file: */
		++*inpv,  *acc = 'a';
	if(  '&' == **inpv  )		/* Redirect `stderr' as well: */
		++*inpv,  err = 1;
	inpv = redir( ainpc, inpv, stdout, "out",
	  " (`>', `>>', `>&', or `>>&')", sysoutput, acc );
	/* The following is harmless if redir() failed: */
	if(  err  ) {
		if(  !strchr( *inpv, '/' )  &&	!strsub( *inpv, ".." )	) {
		  uint stat = crelnm( "LNM$FILE_DEV", "SYS$ERROR", *inpv );
			if(  OPdebug  )
				sigvms( stat );
		}
		stderr = stdout;	/* VAX-C specific method. */
	}
	return( inpv );
}

/* Signals any non-boring condition values so that error/warning messages
 * will be displayed. */
static void
#if defined(HAVE_STDC)
sigvms(
  uint stat
)
#else /* NOT defined(HAVE_STDC) */
sigvms( stat )
  uint stat;
#endif /* defined(HAVE_STDC) */
{
  extern void lib$signal();
	if(  ( stat & 0xffff ) == 1 )
		return;		/* Boring status message; skip it. */
	lib$signal( stat );
}

/* Returns the position of a substring within a string.	 Should be replaced by
 * strstr() when DEC provides one. */
static char *
#if defined(HAVE_STDC)
strsub(
  char *str,
  char *sub
)
#else /* NOT defined(HAVE_STDC) */
strsub( str, sub )
  char *str;
  char *sub;
#endif /* defined(HAVE_STDC) */
{
  register char *fp = sub;	/* Points to character we hope to Find next */
  char *sp = strchr(str,*fp);	/* Points to current Starting character */
  register char *cp = sp;	/* Points to character currently Comparing */
	while(	sp  ) {
		while(	*fp  &&	 *fp == *cp  )
			++fp, ++cp;
		if(  !*fp  )
			return( sp );	/* We found it */
		fp = sub;		/* Start the search over again */
		cp = sp = strchr(sp+1,*fp);	/* were we left off last time */
	}
	return( 0 );
}
#ifdef TEST

/* Displays a string to `stdout', making all control and meta characters
 * printable unambiguously. */
static void
#if defined(HAVE_STDC)
dump(
  char *str
)
#else /* NOT defined(HAVE_STDC) */
dump( str )
  char *str;
#endif /* defined(HAVE_STDC) */
{
# define META '`'	/* Show 'a'+'\200' as "`a" */
# define CTRL '^'	/* Show '\001' as "^A" */
# define BOTH '~'	/* Show '\201' as "~A" */
# define QOTE '~'	/* Show '`' as "~`", '^' as "~^", and '~' as "~~" */
# define DEL '\177'
# define uncntrl(c)	( DEL==c ? '?' : c + '@' )
  static char spec[] = { META, CTRL, BOTH, QOTE, 0 };
	while(	*str  ) {
		switch(	 (!!iscntrl(*str)) + 2 * (!isascii(*str))  ) {
		  case 0: /* Normal */
			if(  strchr( spec, *str )  )
				putchar( QOTE );
			putchar( *str );
			break;
		  case 1: /* Control */
			putchar( CTRL );
			putchar( uncntrl(*str) );
			break;
		  case 2: /* Meta */
			putchar( META );
			putchar( toascii(*str) );
			break;
		  case 3: /* Both */
			putchar( BOTH );
			putchar( uncntrl(toascii(*str)) );
			break;
		}
		++str;
	}
}

int
#if defined(HAVE_STDC)
main(
  int argc,
  char **argv
)
#else /* NOT defined(HAVE_STDC) */
main( argc, argv )
  int argc;
  char **argv;
#endif /* defined(HAVE_STDC) */
{
    int cnt;
    extern char **cmd_lin();

    argv = cmd_lin( "", &argc, argv );
    (void)printf( "%d argument%s%s\n",
	argc, 1==argc ? "" : "s", argc ? ":" : "." );
    for(  cnt = 0; *argv; ++argv )
    {
	(void)printf( "%5d: `", ++cnt );
	dump( *argv );
	(void)printf( "'\n" );
    }
    exit(EXIT_SUCCESS);
    return(0);
}
#endif /* TEST */

#endif /* VMS */
