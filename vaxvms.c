/* -*-C-*- vaxvms.c */
/*-->vaxvms*/
/**********************************************************************/
/****************************** vaxvms ********************************/
/**********************************************************************/

/***********************************************************************
This file provides alternative functions for several VMS VMS  C  library
routines which either unacceptable, or incorrect, implementations.  They
have  been developed and  tested under VMS Version  4.4, but indications
are  that they apply  to  earlier versions, back to 3.2  at least.  They
should be retested with each new release of VMS C.

Some of these (memxxx(), strxxx(),  and system()) are available with VMS
C 2.3 or later, but these versions should still work.

Under VAX VMS 5.2, system(), sscanf(), and  strtod() are in the run-time
library.  However, the  run-time  library version of system() returns  a
VMS-style exit code, not a UNIX-style code; for portability, our version
below is preferred.

Contents:
	FSEEK
	FTELL
	GETCHAR
	GETENV
	READ
	UNGETC
	getjpi		-- system-service access
	getlogin
	memchr
	memcmp
	memcpy
	memmove
	memset
	stricmp
	strtok
	strtol
	system
	tell
	unlink
	utime

The VAX VMS  file system record  structure has  unfortunate consequences
for random access files.

By default, text files written  by most system  utilities, and languages
other than  C, have a variable  length record format,  in which a 16-bit
character count is  aligned  on an even-byte  boundary in the disk block
b(always  512 bytes in  VMS, independent  of  record and  file formats),
followed by <count> bytes  of data.  Binary  files,  such as .EXE, .OBJ,
and  TeX .DVI and font files,  all use  a   512-byte fixed record format
which has  no explicit  length field.  No  file  byte count  is  stored;
instead, the block  count, and the offset of  the last data byte in  the
last block are recorded in the  file header (do ``DUMP/HEADER filespec''
to see it).  For binary files with  fixed-length records, the last block
is  normally assumed to be  full,  and  consequently, file transfer   of
binary data from other  machines via Kermit, FTP,  or DCL COPY from ANSI
tapes, generally fails  because the input file  length is not a multiple
of 512.

This record organization may  be contrasted with  the STREAM, STREAM_LF,
and STREAM_CR organizations supported from Version 4.0; in  these,  disk
blocks contain a continuous byte stream in which nothing, or  LF, or CR,
is recognized as a record terminator.  These formats are similar to  the
UNIX  and TOPS-20 file system  formats  which also use continuous   byte
streams.

For C, this  means that a  program operating on a file  in record format
cannot count input characters and expect that count to be the same value
as the  offset parameter passed  to fseek(),  which  numerous C programs
assume to  be the case.  The 15-Dec-1989 C  Standard,  and  Harbison and
Steele's ``C Reference Manual'', emphasize that only  values returned by
ftell() should be used as arguments to fseek(),  allowing the program to
return to  a position previously read or  written.  UNFORTUNATELY, VMS C
ftell()  DOES NOT  RETURN   A CORRECT  OFFSET VALUE FOR   RECORD  FILES.
Instead, for record files, it returns the byte  offset  of the start  of
the current record, no matter where in that  record the current position
may  be.   This  misbehavior  is  completely unnecessary,   since    the
replacements below perform correctly, and are written entirely in C.

Another problem is that ungetc(char  c, FILE  *fp) is unreliable.  VMS C
implements  characters as  signed  8-bit integers (so do  many   other C
implementations).  fgetc(FILE *fp)  returns an int,   not a char,  whose
value is EOF (-1) in the event of end-of-file; however,  this value will
also be  returned for   a character 0xFF,  so  it  is  essential  to use
feof(FILE  *fp)  to test for  a true  end-of-file  condition when EOF is
returned.  ungetc() checks  the sign  of its argument  c,  and if it  is
negative (which it will be for 128 of the 256  signed bytes), REFUSES TO
PUT IT BACK IN THE INPUT STREAM, on the assumption that c is really EOF.
This  too can  be fixed;  ungetc()  should only  do  nothing  if  feof()
indicates  a  true end-of-file   condition.   The overhead of  this   is
trivial, since  feof() is  actually implemented as  a macro   which does
nothing more than a logical AND and compare-with-zero.

getchar()  waits for a <CR> to  be typed when stdin is  a terminal;  the
replacement vms_getchar() remedies this.

Undoubtedly  other  deficiencies  in   VMS  C will   reveal  themselves.

VMS read() returns   only  a  single  disk   block on  each call.    Its
replacement, vms_read(), will  return the  requested number of bytes, if
possible.

[29-Apr-1987] Brendan Mackay (munnari!anucsd.oz!bdm@seismo.CSS.GOV)
This fix has been incorporated in vms_read() below.  Here are  Brendan's
comments:
>> The code for vms_read() has problems.  One is that you don't test for
>> end of file.  The other is that there is a bug in the C library which
>> prevents you  asking for  more than  65535 bytes  at a  time.  It  is
>> documented that no more  than 65535 bytes will  be returned, but  not
>> that you can't ask for more.  If you do, it reduces your request  mod
>> 65536!

There are  also a few  UNIX standard functions  which are unimplemented.
getlogin() and unlink() have VMS  equivalents provided below.  tell() is
considered obsolete, since its functionality is available  from lseek(),
but it is still seen in a few programs, so is  provided below.  getenv()
fails  if the name contains  a colon; its replacement  allows the colon,
and ignores letter case.

In the interest  of  minimal source perturbation,  replacements  for VMS
functions   are  given   the same  names,    but prefixed  "vms_".   For
readability,   the original names  are  preserved,  but are converted to
upper-case:

	#define FTELL vms_ftell
	#define FSEEK vms_fseek
	#define GETCHAR vms_getchar
	#define GETENV vms_getenv
	#define UNGETC vms_ungetc

These  are  only defined to work   correctly for fixed  length  512-byte
records, and no check is made that the file has that organization (it is
possible, but   not without  expensive calls to    fstat(), or access to
internal library structures).

[02-Apr-1987]	-- Nelson H. F. Beebe, University of Utah Center for
		   Scientific Computing
[13-Apr-1988]	-- added memxxx(), strxxx(), fixed return code in system()
***********************************************************************/

#ifdef VMS			/* so this compiles anywhere */

#undef VOIDP
#if 0				/* prior to VMS C 2.3 */
#define VOIDP	char*		/* char  *prior to Standard C */
#define const			/* const is a type modifier in Standard C */
#else  /* NOT 1 */
#define VOIDP	void*		/* char  *prior to Standard C */
#endif /* 1 */

#define FTELL	vms_ftell
#define FSEEK	vms_fseek
#define GETENV	vms_getenv
#define GETCHAR vms_getchar
#define READ	vms_read
#define UNGETC	vms_ungetc

#include <config.h>
#include <stdio.h>

#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#if defined(HAVE_STRING_H)
#include <string.h>
#endif

#if defined(HAVE_TYPES_H)
#include <types.h>
#endif

#if defined(HAVE_CTYPE_H)
#include <ctype.h>
#endif

#if defined(HAVE_DESCRIP_H)
#include <descrip.h>
#endif

#if defined(HAVE_ERRNO_H)
#include <errno.h>		/* need for utime() */
#endif

#if defined(HAVE_IODEF_H)
#include <iodef.h>		/* need for vms_getchar() */
#endif

#if defined(HAVE_RMS_H)
#include <rms.h>		/* need for utime() */
#endif

#if defined(HAVE_SSDEF_H)
#include <ssdef.h>
#endif

#if defined(HAVE_STAT_H)
#include <stat.h>
#endif

#if defined(HAVE_STRING_H)
#include <string.h>
#endif

#if defined(HAVE_TIME_H)
#include <time.h>
#endif

#if defined(HAVE_UNIXIO_H)
#include <unixio.h>
#endif

#if defined(__ALPHA)
int	lib$spawn(struct dsc$descriptor *t_,int,int,int,int,int,int *stat_);
void	lib$stop(int status_);
int	sys$assign(void *desc_,int *channel_,int n1_,int n2_);
int	sys$bintim(struct dsc$descriptor_s *time_desc_, int xab_element_);
int	sys$close(struct FAB *fab_);
int	sys$qiow(int, int channel_, int flags_, int n1_, int n2_, int n3_,
		int *ret_char_, int n4_,  int n5_, int n6_, int n7_, int n8_);
int	sys$open(struct FAB *fab_);
int	system(const char *s_);
#else
/* VAX VMS 6.1 has these header files */
#include <lib$routines.h>		/* for lib$spawn(), lib$stop() */
#include <starlet.h>			/* for sys$assign(), sys$bintim(), sys$close(),
					sys$exit(), sys$open(), sys$qiow() */
#endif

long	vms_fseek(FILE *fp_, long n_, long dir_);
long	vms_ftell(FILE *fp_);
char	*vms_getenv(char *name_);
int	vms_getchar(void);
int	vms_read(int file_desc_, char *buffer_, int nbytes_);
long	vms_ungetc(char c_, FILE *fp_);

static char rcsid[] = "$Id: vaxvms.c,v 1.3 2003/08/22 23:25:43 beebe Exp beebe $";


/**********************************************************************/
/*-->FSEEK*/

/* VMS fseek() and ftell() on fixed-length record files work correctly
only at block boundaries.  This replacement code patches in the offset
within	the  block.  Directions	 from	current	  position   and  from
end-of-file are converted to absolute positions, and then the code for
that case is invoked. */

long
#if defined(HAVE_STDC)
FSEEK(
FILE *fp,
long n,
long dir
)
#else /* NOT defined(HAVE_STDC) */
FSEEK(fp,n,dir)
FILE *fp;
long n;
long dir;
#endif /* defined(HAVE_STDC) */
{
    long k,m,pos,val,oldpos;
    struct stat buffer;

    for (;;)			/* loops only once or twice */
    {
      switch (dir)
      {
      case 0:			/* from BOF */
	  oldpos = FTELL(fp);	/* get current byte offset in file */
	  k = n & 511;		/* offset in 512-byte block */
	  m = n >> 9;		/* relative block number in file */
	  if (((*fp)->_cnt) && ((oldpos >> 9) == m)) /* still in same block */
	  {
	    val = 0;		/* success */
	    (*fp)->_ptr = ((*fp)->_base) + k;
					/* reset pointers to requested byte */
	    (*fp)->_cnt = 512 - k;
	  }
	  else
	  {
	    val = fseek(fp,m << 9,0);
				/* move to start of requested 512-byte block */
	    if (val == 0)	/* success */
	    {
	      (*fp)->_cnt = 0;	/* indicate empty buffer */
	      (void)fgetc(fp);	/* force refill of buffer */
	      (*fp)->_ptr = ((*fp)->_base) + k;
				/* reset pointers to requested byte */
	      (*fp)->_cnt = 512 - k;
	    }
	  }
	  return (val);

      case 1:			/* from current pos */
	  pos = FTELL(fp);
	  if (pos == EOF)	/* then error */
	    return (EOF);
	  n += pos;
	  dir = 0;
	  break;		/* go do case 0 */

      case 2:			/* from EOF */
	  val = fstat((int)fileno(fp),&buffer);
	  if (val == EOF)	/* then error */
	    return (EOF);
	  n += buffer.st_size - 1; /* convert filesize to offset and */
				   /* add to requested offset */
	  dir = 0;
	  break;		/* go do case 0 */

      default:			/* illegal direction parameter */
	  return (EOF);
      }
    }
}

/**********************************************************************/
/*-->FTELL*/

/* With fixed-length record files, ftell() returns the offset of the
start of block.	 To get the true position, this must be biased by
the offset within the block. */

long
#if defined(HAVE_STDC)
FTELL(
FILE *fp
)
#else /* NOT defined(HAVE_STDC) */
FTELL(fp)
FILE *fp;
#endif /* defined(HAVE_STDC) */
{
    char c;
    long pos;
    long val;
    if ((*fp)->_cnt == 0)	/* buffer empty--force refill */
    {
	c = fgetc(fp);
	val = UNGETC(c,fp);
	if (val != c)
	    return (EOF);	/* should never happen */
    }
    pos = ftell(fp);	/* this returns multiple of 512 (start of block) */
    if (pos >= 0)		/* then success--patch in offset in block */
      pos += ((*fp)->_ptr) - ((*fp)->_base);
    return (pos);
}

/**********************************************************************/
/*-->GETCHAR*/

static int tt_channel = -1;	/* terminal channel for image QIO's */

#define FAILED(status) (~(status) & 1) /* failure if LSB is 0 */

int
GETCHAR(VOID)
{
    int ret_char;		/* character returned */
    int status;			/* system service status */
    static $DESCRIPTOR(sys_in,"TT:");

    if (tt_channel == -1)	/* then first call--assign channel */
    {
	status = sys$assign(&sys_in,&tt_channel,0,0);
	if (FAILED(status))
	    lib$stop(status);
    }
    ret_char = 0;
    status = sys$qiow(0,tt_channel,IO$_TTYREADALL | IO$M_NOECHO,0,0,0,
	&ret_char,1,0,0,0,0);
    if (FAILED(status))
	lib$stop(status);

    return (ret_char);
}

/**********************************************************************/
/*-->memchr*/

/* This is a simple implementation of memchr(), which searches for the
first occurrence of a byte in the first n bytes of a byte string.  A
library version should use hardware moves, or unrolled loops, or other
tricks for greater efficiency. */

#if !defined(HAVE_MEMCHR)
VOIDP
#if defined(HAVE_STDC)
memchr(
const VOIDP s,
int c,
size_t n
)
#else /* NOT defined(HAVE_STDC) */
memchr(s,c,n)
const VOIDP s;
int c;
size_t n;
#endif /* defined(HAVE_STDC) */
{
    unsigned char *ss = (unsigned char*)s;

    for (; n > 0; ss++,--n)
    {
	if (*ss == (unsigned char)c)
	    return ((VOIDP)ss);
    }
    return ((VOIDP)NULL);
}
#endif

/**********************************************************************/
/*-->memcmp*/

/* This is a simple implementation of memcmp(), which compares two
objects byte by byte, stopping after n bytes.  A library version
should use hardware moves, or unrolled loops, or other tricks for
greater efficiency. */

#if !defined(HAVE_MEMCMP)
int
#if defined(HAVE_STDC)
memcmp(
const VOIDP s1,
const VOIDP s2,
size_t n
)
#else /* NOT defined(HAVE_STDC) */
memcmp(s1,s2,n)
const VOIDP s1;
const VOIDP s2;
size_t n;
#endif /* defined(HAVE_STDC) */
{
	unsigned char *t1;
	unsigned char *t2;

	for (t1 = (unsigned char*)s1, t2 = (unsigned char*)s2; n > 0;
	    --n, t1++, t2++)
	{
		if (*t1 < *t2)
			return (-(int)(t2 - (unsigned char*)s2));
		else if (*t1 > *t2)
			return ((int)(t2 - (unsigned char*)s2));
	}
	return (0);
}
#endif

/**********************************************************************/
/*-->memcpy*/

/* This is a simple implementation of memcpy(), which copies source
to target with undefined behavior in the event of overlap.  This
particular implementation copies from first to last byte, in order. */

#if !defined(HAVE_MEMCPY)
VOIDP
#if defined(HAVE_STDC)
memcpy(
VOIDP t,	/* target */
const VOIDP s,	/* source */
size_t n
)
#else /* NOT defined(HAVE_STDC) */
memcpy(t,s,n)
VOIDP t;	/* target */
const VOIDP s;	/* source */
size_t n;
#endif /* defined(HAVE_STDC) */
{
    unsigned char *ss = (unsigned char*)s;
    unsigned char *tt = (unsigned char*)t;

    for (; n > 0; --n)
	*tt++ = *ss++;			/* always copy in forward order */
    return (t);
}
#endif

/**********************************************************************/
/*-->memmove*/

/* This is a simple implementation of memmove(), which copies as if the
source were first completely copied to a temporary area, then that
area were copied to the target.    A library version should
use hardware moves, or unrolled loops, or other tricks for greater
efficiency. */

#if !defined(HAVE_MEMMOVE)
VOIDP
#if defined(HAVE_STDC)
memmove(
VOIDP t,	/* target */
const VOIDP s,	/* source */
size_t n
)
#else /* NOT defined(HAVE_STDC) */
memmove(t,s,n)
VOIDP t;	/* target */
const VOIDP s;	/* source */
size_t n;
#endif /* defined(HAVE_STDC) */
{
    unsigned char *ss = (unsigned char*)s;
    unsigned char *tt = (unsigned char*)t;

    if ((ss < tt) && ((ss + n) > tt))	/* source overlaps target from below */
    	for (ss += n, tt += n; n > 0; --n)
    		*tt-- = *ss--;	/* copy in reverse order */
    else
    	for (; n > 0; --n)
    		*tt++ = *ss++;	/* copy in forward order */
    return (t);
}
#endif

/**********************************************************************/
/*-->memset*/

/* This is a simple implementation of memset().   A library version should
use hardware moves, or unrolled loops, or other tricks for greater
efficiency. */


#if !defined(HAVE_MEMSET)
VOIDP
#if defined(HAVE_STDC)
memset(
VOIDP s,	/* target */
int ch,		/* fill character (treated as unsigned char) */
size_t n	/* fill count */
)
#else /* NOT defined(HAVE_STDC) */
memset(s,ch,n)
VOIDP s;	/* target */
int ch;		/* fill character (treated as unsigned char) */
size_t n;	/* fill count */
#endif /* defined(HAVE_STDC) */
{
    unsigned char *ss = (unsigned char *)s;

    for (; n > 0; --n)
    	*ss++ = (unsigned char)ch;
    return (s);
}
#endif

/**********************************************************************/
/*-->READ*/
int
#if defined(HAVE_STDC)
READ(
register int file_desc,
register char *buffer,
register int nbytes
)
#else /* NOT defined(HAVE_STDC) */
READ(file_desc,buffer,nbytes)
register int file_desc;
register char *buffer;
register int nbytes;
#endif /* defined(HAVE_STDC) */
{
    register int ngot;
    register int left;

    for (left = nbytes; left > 0; /* NOOP */)
    {
	ngot = read(file_desc,buffer,(left > 65024 ? 65024 : left));
	if (ngot < 0)
	    return (-1);        /* error occurred */
	if (ngot == 0)          /* eof occurred */
	    return (nbytes-left);
	buffer += ngot;
	left -= ngot;
    }
    return (nbytes-left);
}

/**********************************************************************/
/*-->UNGETC*/
long
#if defined(HAVE_STDC)
UNGETC(
char c,
FILE *fp
)
#else /* NOT defined(HAVE_STDC) */
UNGETC(c,fp) /* VMS ungetc() is a no-op if c < 0 (which is half the time!) */
char c;
FILE *fp;
#endif /* defined(HAVE_STDC) */
{

    if ((c == EOF) && feof(fp))
	return (EOF);		/* do nothing at true end-of-file */
    else if ((*fp)->_cnt >= 512)/* buffer full--no fgetc() done in this block!*/
	return (EOF);		/* must be user error if this happens */
    else			/* put the character back in the buffer */
    {
      (*fp)->_cnt++;		/* increase count of characters left */
      (*fp)->_ptr--;		/* backup pointer to next available char */
      *((*fp)->_ptr) = c;	/* save the character */
      return (c);		/* and return it */
    }
}

/**********************************************************************/
/*-->getenv*/
char*
#if defined(HAVE_STDC)
GETENV(
char  *name
)
#else /* NOT defined(HAVE_STDC) */
GETENV(name)
char  *name;
#endif /* defined(HAVE_STDC) */
{
    char  *p;
    char  *result;
    char ucname[256];

    p = ucname;
    while (*name)	/* VMS logical names must be upper-case */
    {
      *p++ = Islower(*name) ? toupper(*name) : *name;
      ++name;
    }
    *p = '\0';

    p = strchr(ucname,':');		/* colon in name? */
    if (p == (char *)NULL)		/* no colon in name */
	result = getenv(ucname);
    else				/* try with and without colon */
    {
	result = getenv(ucname);
	if (result == (char *)NULL)
	{
	    *p = '\0';
	    result = getenv(ucname);
	    *p = ':';
	}
    }
    return (result);
}

/**********************************************************************/
/*-->getjpi*/

/***********************************************************************
Return  a system job/process value  obtained   from the VMS  system call
LIB$GETJPI.   This call   can return  either 32-bit   integer values, or
strings.  The  obtained value is stored in  an  internal  static  buffer
which is overwritten on subsequent calls.

The function return is a (char*) pointer to  that buffer, which must  be
coerced to  (long*) if an integer value is  obtained.  String values are
guaranteed to be NUL terminated, with no trailing blanks.

The argument, jpi_code, is one of the values defined in <jpidef.h>.

In the event of an error return from LIB$GETJPI, (char*)NULL is returned
instead.

[30-Oct-1987]
***********************************************************************/

#define LIB$_INVARG 0x158234		/* not defined in standard .h files */

char*
#if defined(HAVE_STDC)
getjpi(
int jpi_code				/* values defined in <jpidef.h> */
)
#else /* NOT defined(HAVE_STDC) */
getjpi(jpi_code)
int jpi_code;				/* values defined in <jpidef.h> */
#endif /* defined(HAVE_STDC) */
{
    short retlen = 0;
    long retval;
    static char buffer[256];		/* space for up to 255-char results */
    static $DESCRIPTOR(strdes,buffer);

    strdes.dsc$w_length = sizeof(buffer)-1; /* $DESCRIPTOR doesn't set this */

    /* lib$getjpi() will normally return a string representation.
       Try first to get the integer representation, then if an invalid
       argument is signalled, get the string representation. */

    retval = lib$getjpi(&jpi_code,0L,0L,&buffer[0]);
    if (retval == LIB$_INVARG)
    {
	retval = lib$getjpi(&jpi_code,0L,0L,&buffer[0],&strdes,&retlen);
	buffer[retlen] = '\0';		/* terminate any string value */
	while ((retlen > 0) && (buffer[--retlen] == ' '))
	    buffer[retlen] = '\0';
    }

    return ((retval == SS$_NORMAL) ? (char*)(&buffer[0]) : (char*)NULL);
}

/**********************************************************************/
/*-->getlogin*/
char*
getlogin(VOID)
{
    return ((char *)getenv("USER")); /* use equivalent VMS routine */
}

/**********************************************************************/
/*-->system*/
int
#if defined(HAVE_STDC)
system(
const char *s
)
#else /* NOT defined(HAVE_STDC) */
system(s)
const char *s;
#endif /* defined(HAVE_STDC) */
{
    struct    dsc$descriptor t;
    int stat;

    t.dsc$w_length = strlen(s);
    t.dsc$a_pointer = s;
    t.dsc$b_class = DSC$K_CLASS_S;
    t.dsc$b_dtype = DSC$K_DTYPE_T;

    /*******************************************************************
    UNIX system() always returns 0 on success; interpretation of
    non-zero returns varies with the particular implementation of UNIX,
    but always means some kind of failure.  BSD UNIX returns 127 if the
    shell, sh, cannot be executed, and otherwise returns 256*(program
    exit code) + (wait() return value).

    The 3 low-order bits of stat return by LIB$SPAWN are:

	0	warning
	1	success
	2	error
	3	information
	4	severe or fatal error

    DCL returns 0 in the low-order bits for undefined commands, and CC
    returns 0 for correctable syntax errors (it issues warnings for
    them).

    We therefore consider values of 1 or 3 to be success.  LIB$SPAWN
    will usually return SS$_NORMAL, independent of the value of stat,
    but if it fails, we follow BSD UNIX and return 127.
    *******************************************************************/

    if (lib$spawn(&t,0,0,0,0,0,&stat) != SS$_NORMAL)
	return (127);
    switch (stat & 7)
    {
    case 0:
	return (256);
    case 1:
    case 3:
	return (0);
    default:
	return (stat << 8);
    }
}

/**********************************************************************/
/*-->tell*/
long
#if defined(HAVE_STDC)
tell(
int handle
)
#else /* NOT defined(HAVE_STDC) */
tell(handle)
int handle;
#endif /* defined(HAVE_STDC) */
{
    return (lseek(handle,0L,1));
}

/**********************************************************************/
/*-->unlink*/
int
#if defined(HAVE_STDC)
unlink(
char *filename
)
#else /* NOT defined(HAVE_STDC) */
unlink(filename)
char *filename;
#endif /* defined(HAVE_STDC) */
{
	return (delete(filename)); /* use equivalent VMS routine */
}

/**********************************************************************/
/*-->utime*/

/* utime(path,times) sets the access and modification times of the
   file 'path' to the UNIX binary time values, 'times'.  Return 0
   on success, and -1 on error (setting errno as well). */

#if defined(HAVE_STDC)
utime(
char  *path,
time_t times[2]
)
#else /* NOT defined(HAVE_STDC) */
utime(path,times)		/* VAX VMS C version */
char  *path;
time_t times[2];
#endif /* defined(HAVE_STDC) */
{
    int status;
    struct dsc$descriptor_s time_desc;
    char *ftime = "23-OCT-1907 12:34:56";
    struct tm *the_timeval;
    static char  *months[] = {"JAN","FEB","MAR","APR","MAY","JUN",
			     "JUL","AUG","SEP","OCT","NOV","DEC"};
    struct FAB fab1;
    struct XABRDT xab1;

    /* Zero FAB and XAB structures */
    (void)memset(&fab1,'\0',sizeof(fab1));
    (void)memset(&xab1,'\0',sizeof(xab1));

    /* Convert UNIX binary time to ASCII string for
       sys$bintime().  We use localtime() instead of ctime(),
       because although ctime() is simpler, it drops the seconds
       field, which we would rather preserve.  */

    the_timeval = (struct tm*)localtime(&times[0]);
    sprintf(ftime,"%02d-%3s-19%02d %02d:%02d:%02d",
	the_timeval->tm_mday,
	months[the_timeval->tm_mon],
	the_timeval->tm_year,
	the_timeval->tm_hour,
	the_timeval->tm_min,
	the_timeval->tm_sec);

    /* Setup fab1 and rab fields. */
    fab1.fab$b_bid = FAB$C_BID;
    fab1.fab$b_bln = FAB$C_BLN;
    fab1.fab$l_fop = FAB$V_UFO;
    fab1.fab$b_fac = FAB$V_GET;
    fab1.fab$l_fna = path;
    fab1.fab$b_fns = strlen(path);
    fab1.fab$l_xab = (char*)&xab1;

    xab1.xab$b_bln = XAB$C_RDTLEN;
    xab1.xab$b_cod = XAB$C_RDT;
    xab1.xab$l_nxt = (char*)NULL;

    /* Open the file */
    status = sys$open(&fab1);
    if (status != RMS$_NORMAL)
    {
      errno = ENOENT;
      return (-1);
    }

    /* Convert the time string to a VMS binary time value in the XAB */
    time_desc.dsc$w_length = strlen(ftime);
    time_desc.dsc$a_pointer = ftime;
    time_desc.dsc$b_class = DSC$K_CLASS_S;
    time_desc.dsc$b_dtype = DSC$K_DTYPE_T;
    status = sys$bintim(&time_desc,&xab1.xab$q_rdt);
    if (status != SS$_NORMAL)
    {
      status = sys$close(&fab1);
      errno = EFAULT;
      return (-1);
    }

    /* Close the file, updating the revision date/time value */
    status = sys$close(&fab1);
    if (status != RMS$_NORMAL)
    {
      errno = EACCES;
      return (-1);
    }
    return (0);
}
#else
/*@unused@*/ static int this_is_not_vms = 0;		/* to provide SOMETHING to compile */
#endif /* VMS */
