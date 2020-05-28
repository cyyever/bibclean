/* -*-C-*- romtol.c */
/*-->romtol*/
/**********************************************************************/
/****************************** romtol ********************************/
/**********************************************************************/

#include <config.h>
#include "xctype.h"
#include "xstring.h"
#include "xstdbool.h"
#include "xstdlib.h"

RCSID("$Id: romtol.c,v 1.5 2003/08/22 23:22:55 beebe Exp beebe $")

/*
 * $Log: romtol.c,v $
 * Revision 1.5  2003/08/22 23:22:55  beebe
 * Add splint annotations.  Include xstdbool.h. Add several (char) and
 * (int) typecasts to reduce instances of char/int type mixing.  Change
 * isxxx() functions to Isxxx().  Change isxxx() functions to is_xxx() to
 * avoid conflict with reserved names in C89 and C99, and make their type
 * bool instead of int.  Change conditionals based on zero/nonzero tests
 * to explicit Boolean relational expressions.
 *
 * Revision 1.4  2003/04/05 21:58:55  beebe
 * Replace NO-OP comments by continue keyword.
 *
 * Revision 1.3  2000/11/25 22:40:59  beebe
 * Add some type casts for portability.
 *
 * Revision 1.2  1999/11/08 14:27:44  beebe
 * Change code to use fgets() instead of gets(), because the
 * latter is unsafe.  It was used only in the test program, so
 * bibclean itself was not made insecure by its use.
 *
 * Revision 1.1  1995/10/07 15:51:44  beebe
 * Initial revision
 *
 * Revision 1.1  1995/10/07 15:50:28  beebe
 * Initial revision
 *
 * Revision 1.2  1992/12/28  22:08:40  beebe
 * Add additional comments about Roman number system.
 *
 * Revision 1.1  1992/12/28  16:32:11  beebe
 * Initial revision
 * */

long		romtol ARGS((/*@null@*/ const char *nptr_, /*@null@*/ char** endptr_));
bool		is_roman ARGS((int c_));
static int	roman_digit_value ARGS((int c_));


/***********************************************************************
This file provides two functions, romtol() (analogous to strtol()) for
converting from Roman numbers to long integers, and is_roman() for
testing whether a character is a valid Roman numeral.

Roman numbers are represented with the symbols i (1), v (5), x (10), l
(50), c (100), d (500), m (1000), v-overbar (5000), x-overbar (10000),
c-overbar (100000), m-overbar (1000000) in a peculiar way.  There is
no representation of zero at all, and letter case is NOT significant.
`Digits' are ordinarily given in order of descending numerical values,
with the number value being the sum of the digit values.  However, if
a larger digit follows a smaller one, then the smaller one is
subtracted from the larger one.  Thus, ix is -1 + 10 = 9, while xi is
10 + 1 = 11.

One source that I consulted claimed that when digits are repeated, the
subtraction rule is suppressed, so iiix would be 3 + 10 = 13, not 2 -
1 + 10 = 11.  The current code does not implement this feature; iiix
will evaluate to 11.

The overbar representation for 5000 and up is not representable in
most computer character sets, so we ignore it here.

The Roman number representation is not unique.  For example, clocks
traditionally use iiii for 4, while iv is used elsewhere.  1990 can be
written as mxm or mcmxc.

***********************************************************************/


#if defined(HAVE_STDC)
long					/* Convert Roman number to long */
romtol(
/*@null@*/ const char  *nptr,		/* pointer to Roman number string */
/*@null@*/ char** endptr		/* if non-NULL, set on return to */
					/* point just beyond last character */
					/* converted */
)
#else /* NOT defined(HAVE_STDC) */
long
romtol(nptr,endptr)
/*@null@*/ const char *nptr;
/*@null@*/ char** endptr;
#endif /* defined(HAVE_STDC) */
{
    long last_value;
    long number;
    const char *p;
    long value;

    if (nptr == (const char*)NULL)
    {
	if (endptr != (char**)NULL)
	    *endptr = (char*)NULL;
	return 0L;
    }

    for (p = nptr; Isspace((int)*p); ++p)	/* ignore leading space */
	continue;

    for (last_value = 0, number = 0L; (*p != '\0'); ++p)
    {
	value = (long)roman_digit_value((int)(*p));
	if (value == 0L)
	    break;
	if (value > last_value)
	    number -= last_value;
	else
	    number += last_value;
	last_value = value;
    }
    number += last_value;
    if (endptr != (char**)NULL)
	*endptr = (char*)p;
    return (number);
}


#if defined(HAVE_STDC)
bool
is_roman(int c)	/* return true if c is Roman digit, otherwise false */
#else /* NOT defined(HAVE_STDC) */
bool
is_roman(c)
int c;
#endif /* defined(HAVE_STDC) */
{
    return ((roman_digit_value(c) == 0) ? false : true);
}


#if defined(HAVE_STDC)
static int
roman_digit_value(int c)	/* return digit value, or 0 if non-digit */
#else /* NOT defined(HAVE_STDC) */
static int
roman_digit_value(c)
int c;
#endif /* defined(HAVE_STDC) */
{
    /*@observer@*/ static const char* roman_digits = "ivxlcdm";
    static const int roman_values[] = { 1, 5, 10, 50, 100, 500, 1000 };
    const char *p = strchr(roman_digits,Isupper(c) ? tolower(c) : c);

    if (p == (const char*)NULL)
	return 0;
    else
	return roman_values[(int)(p - roman_digits)];
}

#ifdef TEST
/***********************************************************************
Simple test program for romtol().   Values are read from stdin, and the
results are echoed to stdout.
***********************************************************************/

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif /* EXIT_SUCCESS */


#if defined(HAVE_STDC)
int
main(
int			argc,
char			*argv[]
)
#else /* NOT defined(HAVE_STDC) */
int
main(argc,argv)
int			argc;
char			*argv[];
#endif /* defined(HAVE_STDC) */
{
    char		s[25];
    char		*endptr;
    long		n;

    while (fgets(s,sizeof(s),stdin) != (char*)NULL)
    {
	endptr = strchr(s,'\n');
	if (endptr != (char*)NULL)	/* then remove final newline */
	    *endptr = '\0';
	n = romtol(s,&endptr);
	(void)printf("romtol():  %s -> %ld\tRemainder = [%s]\n",
	    s,n,(endptr == (char*)NULL) ? "" : endptr);
    }
    exit (EXIT_SUCCESS);
    return (0);
}
#endif /* TEST */
