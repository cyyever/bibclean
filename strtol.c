/* -*-C-*- strtol.c */
/*-->strtol*/
/**********************************************************************/
/****************************** strtol ********************************/
/**********************************************************************/

#include <config.h>
#include "xctype.h"
#include "xstdbool.h"
#include "xstring.h"
#include "xstdlib.h"

RCSID("$Id: strtol.c,v 1.3 2003/08/22 23:24:55 beebe Exp beebe $")


#define IN(l,a,r) (((l) <= (a)) && ((a) <= (r)))

/* This is a simple implementation of Standard C strtol().  A library
version should be programmed with more care. */

long
#if defined(HAVE_STDC)
strtol(
const char  *nptr,
/*@null@*/ char** endptr,
int base
)
#else /* NOT defined(HAVE_STDC) */
strtol(nptr,endptr,base)
const char  *nptr;
/*@null@*/ char** endptr;
int base;
#endif /* defined(HAVE_STDC) */
{
    int			c;		/* current character value */
    int			digit;		/* digit value */
    /*@observer@*/ static const char *digits = "0123456789abcdefghijklmnopqrstuvxwyz";
    bool		is_negative;	/* false for positive, true for negative */
    long		number;		/* the accumulating number */
    const char		*pos;		/* pointer into digit list */
    const char		*q;		/* pointer past end of digits */

    if (!(IN(2,base,36) || (base == 0) || (nptr != (const char*)NULL)))
    {
	if (endptr != (char**)NULL)
	    *endptr = (char*)nptr;
	return (0L);
    }

    while (Isspace((int)*nptr))
	nptr++;				/* ignore leading whitespace */

    switch (*nptr)			/* set number sign */
    {
    case '-':
	is_negative = true;
	nptr++;
	break;

    case '+':
	is_negative = false;
	nptr++;
	break;

    default:
	is_negative = false;
	break;
    }

    q = nptr;
    if (base == 0)			/* variable base; set by lookahead */
    {
	if (*q == '0')
	    base = ((*(q+1) == 'x') || (*(q+1) == 'X')) ? 16 : 8;
	else
	    base = 10;
    }

    /* eliminate optional "0x" or "0X" prefix */

    if (    (base == 16) &&
	(*q == '0') &&
	((*(q+1) == 'x') || (*(q+1) == 'X')) )
	q += 2;

    number = 0L;

    /* Number conversion is done by shifting rather than multiplication
       when the base is a power of 2, in order that the results not be
       impacted by integer overflow. */
    switch (base)
    {
    case 2:
	while (IN('0',*q,'1'))
	{
	    number <<= 1;
	    number |= *q - '0';
	    q++;
	}
	break;

    case 4:
	while (IN('0',*q,'3'))
	{
	    number <<= 2;
	    number |= *q - '0';
	    q++;
	}
	break;


    case 8:
	while (IN('0',*q,'7'))
	{
	    number <<= 3;
	    number |= *q - '0';
	    q++;
	}
	break;


    case 16:
	for (;;)
	{
	    if (*q == '\0')
		break;
	    c = (int)(unsigned)*q;
	    if (Isupper(c))
		c = tolower(c);
	    pos = strchr(digits,c);
	    if (pos == (char*)NULL)
		break;
	    digit = (int)(pos - digits);
	    if (!IN(0,digit,15))
		break;
	    number <<= 4;
	    number |= digit;
	    q++;
	}
	break;


    case 32:
	for (;;)
	{
	    if (*q == '\0')
		break;
	    c = (int)(unsigned)*q;
	    if (Isupper(c))
		c = tolower(c);
	    pos = strchr(digits,c);
	    if (pos == (char*)NULL)
		break;
	    digit = (int)(pos - digits);
	    if (!IN(0,digit,31))
		break;
	    number <<= 5;
	    number |= digit;
	    q++;
	}
	break;

    default:		/* all other bases done by multiplication */
	for (;;)	/* accumulate negative so most negative */
	{		/* number on two's-complement is handled */
	    if (*q == '\0')
		break;
	    c = (int)(unsigned)*q;
	    if (Isupper(c))
		c = tolower(c);
	    pos = strchr(digits,c);
	    if (pos == (char*)NULL)
		break;
	    digit = (int)(pos - digits);
	    if (!IN(0,digit,base-1))
		break;
	    number *= base;
	    number -= digit;
	    q++;
	}
	if (endptr != (char**)NULL)
	    *endptr = (char*)q;
	if (is_negative)
	    return(number);
	number = -number;
	break;
    }
    if (is_negative)
	number = -number;
    if (endptr != (char**)NULL)
	*endptr = (char*)q;
    return (number);
}

#ifdef TEST
/***********************************************************************
Simple test program for strtol().   Values are read from stdin, and the
results in different bases are echoed to stdout.
***********************************************************************/

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif /* EXIT_SUCCESS */


int
#if defined(HAVE_STDC)
main(
int			argc,
char			*argv[]
)
#else /* NOT defined(HAVE_STDC) */
main(argc,argv)
int			argc;
char			*argv[];
#endif /* defined(HAVE_STDC) */
{
    char		s[25];
    char		*endptr;
    long		n;
    int			k;
    static int		base[] =
    {
	0, 2, 4, 8, 10, 16, 32, 36, 5,
    };


    while (gets(s) != (char*)NULL)
    {
	for (k = 0; k < sizeof(base)/sizeof(base[0]); ++k)
	{
		n = strtol(s,&endptr,base[k]);
		(void)printf(
		    "strtol(,,%d):  %s -> 16#%lx  8#%lo  10#%ld  Rem = [%s]\n",
		    base[k],s,n,n,n,endptr);
	}
    }
    exit (EXIT_SUCCESS);
    return (0);
}
#endif /* TEST */
