/***********************************************************************
Test the <ctype.h> functions for characters in 0..255.
[23-Aug-2003]
***********************************************************************/

#include <stdio.h>
#include <ctype.h>

#if defined(__cplusplus) || defined(__STDC__)
void
test_isxxx(const char *name, int (*fcn)(int))
#else
void
test_isxxx(name, fcn)
char *name;
int (*fcn)();
#endif
{
    int k;

    (void)printf("\n");

    (void)printf("Test of %s: ", name);
    for (k = 0; k <= 255; ++k)
    {
	if ((*fcn)(k))
	    (void)printf(" %d", k);
    }
    (void)printf("\n");

    (void)printf("Test of %s: ", name);
    for (k = 0; k <= 255; ++k)
    {
	if ((*fcn)(k))
	    (void)printf("%c", k);
    }
    (void)printf("\n");
}

#if defined(__cplusplus) || defined(__STDC__)
void
test_to(const char *name, int (*fcn)(int))
#else
void
test_to(name, fcn)
char *name;
int (*fcn)();
#endif
{
    int k;

    (void)printf("\n");

    (void)printf("Test of %s: ", name);
    for (k = 0; k <= 255; ++k)
    {
	if ((*fcn)(k) != k)
	    (void)printf(" %d->%d", k, (*fcn)(k));
    }
    (void)printf("\n");

    (void)printf("Test of %s: ", name);
    for (k = 0; k <= 255; ++k)
    {
	if ((*fcn)(k) != k)
	    (void)printf(" %c->%c", k, (*fcn)(k));
    }
    (void)printf("\n");
}

#if defined(__cplusplus) || defined(__STDC__)
int
main(int argc, char* argv[])
#else
int
main(argc, argv)
int argc;
char* argv[];
#endif
{

    test_isxxx("isalnum", isalnum);
    test_isxxx("isalpha", isalpha);
    test_isxxx("iscntrl", iscntrl);
    test_isxxx("isdigit", isdigit);
    test_isxxx("isgraph", isgraph);
    test_isxxx("islower", islower);
    test_isxxx("isprint", isprint);
    test_isxxx("ispunct", ispunct);
    test_isxxx("isspace", isspace);
    test_isxxx("isupper", isupper);
    test_isxxx("isxdigit", isxdigit);
    test_to("tolower", tolower);
    test_to("toupper", toupper);

    return (0);
}
