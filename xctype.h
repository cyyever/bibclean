#ifndef XCTYPE_H_DEFINED_
#define XCTYPE_H_DEFINED_

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#else
#endif

/* We need the isxxx() functions/macros from <ctype.h> to work
correctly for 8-bit characters, but regrettably, those in many C
implementations fail to do so if char is a signed data type, and the
character is out of the range 0..127.  If your compiler lacks an
unsigned char data type, then you will have to change (unsigned
char)(c) to (int)(0xff & (unsigned int)(c)).  With this change, it is
important that none of these be invoked with c == EOF. */

#define Isalnum(c)	(isalnum((int)(unsigned char)(c)) != 0)
#define Isalpha(c)	(isalpha((int)(unsigned char)(c)) != 0)
#define Isdigit(c)	(isdigit((int)(unsigned char)(c)) != 0)
#define Isgraph(c)	(isgraph((int)(unsigned char)(c)) != 0)
#define Islower(c)	(islower((int)(unsigned char)(c)) != 0)
#define Isprint(c)	(isprint((int)(unsigned char)(c)) != 0)
#define Isspace(c)	(isspace((int)(unsigned char)(c)) != 0)
#define Isupper(c)	(isupper((int)(unsigned char)(c)) != 0)

#endif /* XCTYPE_H_DEFINED_ */
