/* -*-C-*- strist.c */
/*-->strist*/
/**********************************************************************/
/****************************** strist ********************************/
/**********************************************************************/

#include <config.h>
#include "xctype.h"
#include "xstring.h"

RCSID("$Id: strist.c,v 1.2 2003/08/22 23:23:07 beebe Exp beebe $")


/* Return pointer to location of sub[] in s[], ignoring letter case,
else (char*)NULL.  This is a simple implementation; a library version
should use a more sophisticated version (e.g. Boyer-Moore,
Knuth-Morris-Pratt, hardware search). */

char*
#if defined(HAVE_STDC)
stristr(
const char  *s,
const char  *sub
)
#else /* NOT defined(HAVE_STDC) */
stristr(s,sub)
const char  *s;
const char  *sub;
#endif /* defined(HAVE_STDC) */
{
    size_t length;

    length = (size_t)strlen(sub);
    if (s == (const char*)NULL)
	return ((char*)NULL);
    if ((sub == (char*)NULL) || (*sub == '\0'))
	return ((char*)s); /* NULL substring always found at start */
    for (; (*s != '\0');)
    {
	if (strnicmp(s,sub,length) == 0)
	    return ((char*)s);
	++s;
    }
    return ((char*)NULL);
}
