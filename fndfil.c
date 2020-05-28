/* -*-C-*- fndfil.c */
/*-->findfile*/
/**********************************************************************/
/****************************** findfile ******************************/
/**********************************************************************/

/***********************************************************************
NB:  If  the macro DVI  is defined  when this  file  is compiled, then
additional code will be executed  in is_file() to  optionally trace the
lookup attempts.

If the macro TEST is defined at compile-time, a test main program will
be compiled  which takes paths and  names from stdin,  and echos their
existence and expansion to stdout.
***********************************************************************/

/***********************************************************************

Search for a file  in a list of  directories specified in the  pathlist,
and if found,  return a pointer  to an internal  buffer (overwritten  on
subsequent  calls)  containing  a   full  filename;  otherwise,   return
(char*)NULL.

This version has been extended to use the file cache stored in an
optional hash table pointed to by fsf_table.  If the table exists, and
a match is found there, then it is assumed to be a valid file name and
the operating system is not consulted at all, nor is the path list
examined.

Note that this permits finding files whose directories are not
specified in the path list.  You can view this as both a bug and a
feature.  Since the user can control the contents of the FSMAPFILE
file, and can suppress it entirely, the feature outweighs the bug.

Filename caching is a valuable economization because file system
lookups are relatively time consuming.  Of course, if the cache is
incorrect, a returned filename may not exist, and a later open attempt
will fail; that should alert to user to update the FSMAPFILE file.

The pathlist is expected to contain directories separated by one (or
more) of the characters in the system-dependent SEP_COMP string
constant.  The search is restricted to the directories in the path
list; unlike PC DOS's PATH variable, the current directory is NOT
searched unless it is included in the path list.

For example,

	findfile(".:/usr/local/lib:/tmp", "foo.bar")

will check for the files

	./foo.bar
	/usr/local/lib/foo.bar
	/tmp/foo.bar

in that order.  The character which normally separates a directory  from
a file name will  be supplied automatically  if it is  not given in  the
path list.  Thus, the example above could have been written

	findfile("./:/usr/local/lib/:/tmp/","foo.bar")

Since multiple path separators are ignored, the following are also
acceptable:

	findfile("./ : /usr/local/lib/ : /tmp/","foo.bar")
	findfile("./   /usr/local/lib/   /tmp/","foo.bar")
	findfile("./:::::/usr/local/lib/:::::/tmp/:::::","foo.bar")

For VAX VMS, additional support is  provided for rooted logical names,
and names  that   look like  rooted  names, but  are not.  Normally, a
logical name  in a path  will look name  "LOGNAME:",  and a  file like
"FILE.EXT"; combining them gives "LOGNAME:FILE.EXT".  A rooted logical
name is created by a DCL command like

$ define /translation=(concealed,terminal) LOGNAME DISK:[DIR.]

In such a case, VMS requires that what  follows the logical name begin
with a directory, e.g.  the file is "[SUB]FILE.EXT".  Concatenation of
path with name gives "LOGNAME:[SUB]FILE.EXT", which is acceptable.  To
refer   to  a   file in   the   directory   DISK:[DIR],  one    writes
"LOGNAME:[000000]FILE.EXT", where 000000  is the magic  name of a root
directory.

If, however, the logical  name looks  like a  rooted name,  but wasn't
defined  with  "/translation=(concealed,terminal)", then VMS  will not
recognize it.   [That  is an easy mistake to  make, and  issuing a DCL
"show logical LOGNAME" command will NOT reveal the error.]  Similarly,
if the logical name  looks like a  directory name, and the file begins
with a directory name, VMS will not recognize that either, even though
there is no ambiguity about what is meant.

We therefore make the  following reductions after  an  initial attempt
fails.  First, expand the logical name, skipping further processing if
that fails.  Then, reduce the concatenation as follows:

	[DIR.]FILE.EXT          -->     [DIR]FILE.EXT
	[DIR.][SUB]FILE.EXT     -->     [DIR.SUB]FILE.EXT
	[DIR][SUB]FILE.EXT      -->     [DIR.SUB]FILE.EXT

If  the logical name  points to a  chain of names,  the standard VMS C
run-time library getenv()  will return  only the first  in  the chain.
Chained logical names were added in VMS version  4.0 without operating
system support  to retrieve all members  of the  chain.   The only way
I've been able to  find the subsequent  members is to spawn a  process
that runs a VMS DCL SHOW LOGICAL command, and then parse its output!

Under ATARI TOS, PC DOS, and UNIX,  environment variable  substitution
is supported as follows.  If the initial lookup fails, the filename is
checked for the presence of an initial tilde.   If one is  found, then
if   the  next  character  is   not  alphanumeric or  underscore (e.g.
~/foo.bar),  the  tilde  is  replaced  by ${HOME};     otherwise (e.g.
~user/foo.bar), the tilde is stripped and the following name is looked
up using getpwnam() to find the login directory of that user name, and
that  string is substituted  for the ~name.   Then,  the  filename  is
scanned for  $NAME  or ${NAME} sequences,  where NAME conforms  to the
regular expression  [A-Za-z_]+[A-Za-z0-9_]*, and environment  variable
substitution  is performed for  NAME.  Finally,  the   lookup is tried
again.  If it  is successful,  the  name[] string  is replaced by  the
substituted name, so it can be later used to open the file.

***********************************************************************/

#include <config.h>
#include "xunistd.h"
#include "xctype.h"
#include "xpwd.h"			/* for ~name lookup in envsub() */
#include "xstdbool.h"
#include "xstdlib.h"
#include "xstring.h"

#include "ch.h"
#include "yesorno.h"

#if defined(HAVE_IO_H)
#include <io.h>				/* for more function declarations */
#endif

RCSID("$Id: fndfil.c,v 1.7 2014/04/03 18:04:55 beebe Exp beebe $")

extern FILE		*stdlog;
extern YESorNO		trace_file_opening;

#ifdef DVI
#include "typedefs.h"
#include "hash.h"

extern UNSIGN32		debug_code;
extern HASH_TABLE      *fsf_table;
extern BOOLEAN		g_dolog;	/* allow log file creation */
extern FILE		*g_logfp;	/* log file pointer (for errors) */
extern FILE		*stddbg;	/* debug output file pointer */
extern char		g_logname[];	 /* name of log file, if created */
#endif /* DVI */

#if !defined(R_OK)
#define R_OK 4
#endif
#define IS_FILE_READABLE(n) is_file_readable(n)

#define ISNAMEPREFIX(c) (Isalpha((int)(c)) || ((int)(c) == (int)'_'))
#define ISNAMESUFFIX(c) (Isalnum((int)(c)) || ((int)(c) == (int)'_'))

#ifdef MIN
#undef MIN
#endif /* MIN */

#define	MIN(a,b)	((a) < (b) ? (a) : (b))

#if    OS_VAXVMS
#undef GETENV
#define GETENV (char*)vms_getenv
char *vms_getenv(char *name_);
#endif /* OS_VAXVMS */

/* VMS 4.4 string.h incorrectly declares strspn as char* instead of
size_t, but the return value is (correctly) an integer.  VMS 5.5 string.h
declares it correctly. */
#define STRSPN(s,t)	(size_t)strspn(s,t)

extern void		dbglookup ARGS((const char*, const char*));
/*@null@*/ extern char	*findfile ARGS((/*@null@*/ const char *pathlist_,
					/*@null@*/ const char *name_));
extern void		warning ARGS((const char *msg_));

static const char	*copyname ARGS((char *target_, const char *source_));
static bool		is_file_readable ARGS((const char *filename));
/*@null@*/ static char	*envsub ARGS((const char *filename_));
static bool		is_file ARGS((char *filename_));


#ifdef TEST
int		main ARGS((void));
#endif /* TEST */

/***********************************************************************
Copy environment variable or username, and return updated pointer past
end of copied name in source[].
***********************************************************************/

static const char*
#if defined(HAVE_STDC)
copyname(
register char			*target, /* destination string */
register const char		*source /* source string */
)
#else /* NOT defined(HAVE_STDC) */
copyname(target,source)
register char			*target; /* destination string */
register const char		*source; /* source string */
#endif /* defined(HAVE_STDC) */
{
    if (ISNAMEPREFIX(*source))
    {
	for (*target++ = *source++; ISNAMESUFFIX(*source); )
	    *target++ = *source++;	/* copy name */
	*target = '\0';			/* terminate name */
    }
    return (source);
}


/***********************************************************************
Do  tilde and environment  variable  in  a private copy   of filename,
return  (char*)NULL if no  changes were made,  and otherwise, return a
pointer to the internal copy of the expanded filename.
***********************************************************************/

/*@null@*/ static char*
#if defined(HAVE_STDC)
envsub(
const char			*filename
)
#else /* NOT defined(HAVE_STDC) */
envsub(filename)
const char			*filename;
#endif /* defined(HAVE_STDC) */
{

#if    (OS_ATARI || OS_PCDOS || OS_UNIX)
    static char			altname[MAXPATHLEN+1]; /* result storage */
    register const char		*p;
    register char		*r;
    register const char		*s;
    char			tmpfname[MAXPATHLEN+1];

#if    OS_UNIX && defined(HAVE_GETPWNAM)
    struct passwd		*pw;	/* handle tilde processing */

    tmpfname[0] = '\0';			/* initialize tmpfname[] */
    p = strchr(filename,'~');
    if (p != (char*)NULL)		/* handle tilde (once per filename) */
    {
	(void)strncpy(tmpfname,filename,(size_t)(p - filename));
	tmpfname[(size_t)(p - filename)] = '\0'; /* terminate copied string */
	r = strchr(tmpfname,'\0');	/* remember start of name */
	++p;				/* point past tilde */
	if (ISNAMEPREFIX(*p))		/* expect ~name */
	{
	    p = copyname(r,p);		/* p now points past ~name */
	    pw = getpwnam(r);		/* get password structure */
	    if (pw == (struct passwd *)NULL)
		p--;			/* restore p to point to tilde */
	    else
		(void)strcpy(r,pw->pw_dir);	/* replace name by login directory */
	}
	else				/* expect ~/name */
	    (void)strcat(tmpfname,"${HOME}"); /* change to ${HOME}/name */
	(void)strcat(tmpfname,p);	/* copy rest of filename */
    }
    else
	(void)strcpy(tmpfname,filename);	/* copy of filename */
#else /* NOT OS_UNIX */
    (void)strcpy(tmpfname,filename);	/* copy of filename */
#endif /* OS_UNIX */

    for (r = altname, *r = '\0', p = tmpfname; *p != '\0'; )
    {					/* handle environment variables */
	if (*p == '$')			/* expect $NAME or ${NAME} */
	{
	    p++;			/* point past $ */

	    if (*p == '{')		/* have ${NAME} */
	    {
		p = copyname(r,p+1);

		if (*p++ != '}')
		    return ((char*)NULL); /* bail out -- no closing brace */

		for (s = (const char *)getenv(r); (s != (char*)NULL) && (*s != '\0');)
		    *r++ = *s++;	/* copy environment variable value */

		*r = '\0';		/* terminate altname[] */
	    }
	    else if (ISNAMEPREFIX(*p))	/* have $NAME */
	    {
		p = copyname(r,p);

		for (s = (const char *)getenv(r); (s != (char*)NULL) && (*s != '\0');)
		    *r++ = *s++;	/* copy environment variable value */

		*r = '\0';		/* terminate altname[] */
	    }
	    else			/* invalid $NAME */
		*r++ = '$';		/* so just copy bare $ */
	}
	else				/* just copy character */
	    *r++ = *p++;
    }

    *r = '\0';				/* terminate altname[] */

#ifdef TEST
    (void)printf("envsub: [");
    for (p = filename; *p; ++p)
    {
	putchar(*p);

	if (IN_SET(SEP_COMP,*p))
	    (void)printf("\n\t");
    }

    (void)printf("] ->\n\t[");

    for (p = altname; *p; ++p)
    {
	putchar(*p);

	if (IN_SET(SEP_COMP,*p))
	    (void)printf("\n\t");
    }

    (void)printf("]\n");
#endif /* TEST */

    return ((altname[0] != '\0') ? altname : (char*)NULL);
#else  /* NOT (OS_ATARI || OS_PCDOS || OS_UNIX) */
    return ((char*)NULL);		/* no processing to be done */
#endif /* (OS_ATARI || OS_PCDOS || OS_UNIX) */

}


static bool
#if defined(HAVE_STDC)
is_file_readable(
const char  *filename
)
#else /* NOT defined(HAVE_STDC) */
is_file_readable(filename)
const char  *filename;
#endif /* defined(HAVE_STDC) */
{
    /* return true if filename is readable, and false otherwise */
    bool is_readable;

    is_readable = ((int)access((char*)filename,R_OK) == 0) ? true : false;

    if (trace_file_opening == YES)
    {
	(void)fprintf(stdlog,"%s lookup file [%s]%s\n",
	    WARNING_PREFIX, filename, is_readable ? "" : ": FAILED");
    }

    return (is_readable);
}


/***********************************************************************
Given a directory search path in pathlist[] and a file name in name[],
search the path  for an  existing file.  If  one  is  found, return  a
pointer  to  an  internal copy   of its  full  name; otherwise, return
(char*)NULL.
***********************************************************************/

/*@null@*/ char*
#if defined(HAVE_STDC)
findfile(
/*@null@*/ const char  *pathlist,
/*@null@*/ const char  *name
)
#else /* NOT defined(HAVE_STDC) */
findfile(pathlist,name)
/*@null@*/ const char  *pathlist;
/*@null@*/ const char  *name;
#endif /* defined(HAVE_STDC) */
{
#define BADCACHEFMT	"Invalid font cache entry: font %s is not accessible"
    static char			fullname[MAXPATHLEN+1];
    static char			fullpath[MAXPATHLEN+1];
    size_t			k;
    size_t			len;
    register const char		*p;

#ifdef DVI
    HASH_ENTRY			*he;
    static char			message[MAXPATHLEN+1+sizeof(BADCACHEFMT)];

    if ((name == (const char*)NULL) || (*name == '\0'))
	return ((char*)NULL);

    if (fsf_table != (HASH_TABLE*)NULL)
    {
	he = hash_lookup(name, fsf_table); /* name found in filename cache? */

	if ( (he != (HASH_ENTRY*)NULL) && (he->hash_key != (const char*)NULL) )
	{				/* yes, reconstruct the full filename */
	    (void)strcpy(fullname,(const char*)he->hash_data);
	    k = strlen(fullname);

	    if ( (k > 0) && !IN_SET(SEP_PATH, fullname[k-1]) )
		fullname[k++] = SEP_PATH[0];	/* supply directory separator */

	    (void)strncpy(&fullname[k],name,(size_t)(MAXPATHLEN-k));
					/* append name */
	    fullname[MAXPATHLEN] = '\0'; /* strncpy may not supply this */
	    dbglookup(fullname, "CACHED");

	    /***********************************************************
	      Originally, we returned the filename without an access
	      check, but occasionally, this caused problems if the cache
	      was out-of-date, so we now make the check, and issue a
	      warning if the file is inaccessible, either because it
	      does not exist at all, or lacks read permission.
	    ***********************************************************/

	    if (IS_FILE_READABLE(fullname))
		return ((char*)&fullname[0]);

	    (void)sprintf(message,BADCACHEFMT,fullname);
	    warning(message);
	}
	else
	    dbglookup(name, "NOT CACHED");
    }
#endif /* DVI */

    if ((name == (const char*)NULL) || (*name == '\0'))
	return ((char*)NULL);

    /*******************************************************************
      For user convenience, allow path lists to contain nested
      references to environment variables.  We repeatedly expand the
      path list until there are no more changes in it, or a reasonable
      limit is reached (to prevent infinite recursion when a path
      mistakenly contains itself).
    *******************************************************************/

    p = pathlist;
    if (pathlist != (char*)NULL)
    {
	/* Copy pathlist[] into fullpath[] cautiously.  On the IBM PC,
	Turbo C 2.0 and 3.0 produce a MAXPATHLEN of 80, which is
	shorter than the longest search path that can be set at DOS
	level in an environment variable.  If environment variables
	are set via initialization files, search paths may be even
	longer.  We intentionally do NOT raise an error if path
	truncation occurs. */

	(void)strncpy(fullpath,pathlist,sizeof(fullpath));
	fullpath[sizeof(fullpath)-1] = '\0';
	for (k = 0; k < 10; ++k)
	{
	    p = envsub(fullpath);
	    if ( (p == (char*)NULL) || STREQUAL(p,fullpath) )
		break;			/* no new substitutions were made */
	    else			/* save new expansion */
		(void)strcpy(fullpath,p);
	}
	p = fullpath;			/* remember last expansion */
    }

    if ((p == (char*)NULL) || (*p == '\0')) /* no path, try bare filename */
    {
	(void)strncpy(fullname,name,MAXPATHLEN);
	fullname[MAXPATHLEN] = '\0';	/* in case strncpy() doesn't do it */
	return (is_file(fullname) ? (char*)&fullname[0] : (char*)NULL);
    }
    while (*p != '\0')				/* process pathlist */
    {
	len = strcspn(p,SEP_COMP);	/* get length before separator */
	len = MIN(MAXPATHLEN-1,len);	/* leave space for possible SEP_PATH */
	(void)strncpy(fullname,p,len);	/* set directory name */
	k = len;

	if ( (k > 0) && !IN_SET(SEP_PATH, fullname[k - 1]) )
	    fullname[k++] = SEP_PATH[0];	/* supply directory separator */

	(void)strncpy(&fullname[k],name,(size_t)(MAXPATHLEN-k));
					/* append name */
	fullname[MAXPATHLEN] = '\0';	/* strncpy may not supply this */

	if (is_file(fullname))
	    return ((char*)&fullname[0]);

#if    OS_VAXVMS
	do				/* single trip loop */
	{
	    char		*logname;
	    int			n;

	    if ((k > 0) && (fullname[k-1] == ']')) /* then not logical name */
		break;			/* here's a loop exit */
	    fullname[k] = '\0';		/* terminate logical name */
	    logname = GETENV(fullname);
	    if (logname == (char*)NULL)	/* then unknown logical name */
		break;			/* here's a loop exit */
	    (void)strcpy(fullname,logname);
	    n = strlen(fullname);
	    if ( (n >= 2) && (fullname[n-2] == '.') && (fullname[n-1] == ']') )
	    {				/* have rooted name [dir.] */
		if (name[0] == '[')	/* have [dir.][sub]name */
		    (void)strcpy(&fullname[n-1],&name[1]); /* [dir.sub]name */
		else			/* have [dir.]name */
		{
		    fullname[n-2] = ']';
		    (void)strcpy(&fullname[n-1],&name[0]); /* [dir]name */
		}
	    }
	    else if (fullname[n-1] == ']')
	    {				/* have normal name [dir] */
		if (name[0] == '[')	/* have [dir][sub]name */
		{
		    fullname[n-1] = '.';
		    (void)strcpy(&fullname[n],&name[1]); /* [dir.sub]name */
		}
		else			/* have [dir]name */
		    (void)strcpy(&fullname[n],&name[0]); /* [dir]name */
	    }
	    else			/* must be logical name component */
	    {
		if (fullname[n-1] != ':')
		    fullname[n++] = ':';
		(void)strcpy(&fullname[n],&name[0]); /* logname:name */
	    }
	    if (is_file(fullname))
		return ((char*)&fullname[0]);
	}
	while (0);
#endif /* OS_VAXVMS */

	p += len;			/* point past first path */
	if (*p != '\0')			/* then not at end of path list */
	{
	    p++;			/* point past separator */
	    p += STRSPN(p,SEP_COMP);	/* skip over any extra separators */
	}
    }
    return ((char*)NULL);		/* no file found */
}


/***********************************************************************
Return  a non-zero   result if a  file  named   filename  exists,  and
otherwise, return zero.  If the first existence check  fails,  then we
try tilde and  environment  variable substitutions, and  if there were
any, a second existence check  is  made.  If this second one succeeds,
we  replace filename with the  substituted  name,  since  that will be
needed later to open the file.
***********************************************************************/

static bool
#if defined(HAVE_STDC)
is_file(
char				*filename
)
#else /* NOT defined(HAVE_STDC) */
is_file(filename)
char				*filename;
#endif /* defined(HAVE_STDC) */
{
    bool				exists;
    char			*p;

    exists = IS_FILE_READABLE(filename);

    if (!exists)		/* try environment variable substitution */
    {
	p = envsub(filename);
	if (p != (char*)NULL)
	{
	    exists = IS_FILE_READABLE(p);
	    if (exists)
		(void)strcpy(filename,p);
	}
    }

#ifdef DVI
    if (exists)
	dbglookup(filename, "OK");
    else
	dbglookup(filename, "FAILED");
#endif /* DVI */

    return (exists);
}

#ifdef TEST
/***********************************************************************
Simple test program for findfile().  It prompts for a single directory
search path, then  loops reading test filenames,  uses findfile to see
if they exist in the search path, and  prints the name of the matching
file,  if  any.  If tilde  or environment  variable  substitutions are
made, they are printed as well.
***********************************************************************/

int
main(VOID_ARG)
{
    char			pathlist[MAXPATHLEN+1];
    char			name[MAXPATHLEN+1];
    char			*p;

    (void)printf("Enter file search path: ");
    (void)fgets(pathlist,MAXPATHLEN,stdin);

    p = strchr(pathlist,'\n');
    if (p != (char*)NULL)
	*p = '\0';			/* kill terminal newline */

    for (;;)
    {
	(void)printf("Enter file name: ");
	if (fgets(name,MAXPATHLEN,stdin) == (char*)NULL)
	    break;			/* here's the loop exit */
	p = strchr(name,'\n');
	if (p != (char*)NULL)
	    *p = '\0';			/* kill terminal newline */
	p = findfile(pathlist,name);
	if (p == (char*)NULL)
	    (void)printf("\tNo such file\n");
	else
	    (void)printf("\tFile is [%s]\n",p);
    }
    exit(EXIT_SUCCESS);
    return (0);
}
#endif /* TEST */
