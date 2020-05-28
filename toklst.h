#ifndef TOKLST_H_DEFINED_
#define TOKLST_H_DEFINED_

#if defined(__APPLE__)
#define token_t Token_t	/* avoid conflict with type in /usr/include/bsm/audit.h */
#endif

#if defined(HAVE_STDC)
typedef enum token_list {
    TOKEN_UNKNOWN = 0,
    TOKEN_ABBREV = 1,		/* alphabetical order, starting at 1 */
    TOKEN_AT,
    TOKEN_COMMA,
    TOKEN_COMMENT,
    TOKEN_ENTRY,
    TOKEN_EQUALS,
    TOKEN_FIELD,
    TOKEN_INCLUDE,
    TOKEN_INLINE,
    TOKEN_KEY,
    TOKEN_LBRACE,
    TOKEN_LITERAL,
    TOKEN_NEWLINE,
    TOKEN_PREAMBLE,
    TOKEN_RBRACE,
    TOKEN_SHARP,
    TOKEN_SPACE,
    TOKEN_STRING,
    TOKEN_VALUE
} token_t;
#else /* K&R style */
typedef int token_t;
#define	TOKEN_UNKNOWN	0
#define	TOKEN_ABBREV	1		/* alphabetical order, starting at 1 */
#define	TOKEN_AT	2
#define	TOKEN_COMMA	3
#define	TOKEN_COMMENT	4
#define	TOKEN_ENTRY	5
#define	TOKEN_EQUALS	6
#define	TOKEN_FIELD	7
#define	TOKEN_INCLUDE	8
#define	TOKEN_INLINE	9
#define	TOKEN_KEY	10
#define	TOKEN_LBRACE	11
#define	TOKEN_LITERAL	12
#define	TOKEN_NEWLINE	13
#define	TOKEN_PREAMBLE	14
#define	TOKEN_RBRACE	15
#define	TOKEN_SHARP	16
#define	TOKEN_SPACE	17
#define	TOKEN_STRING	18
#define	TOKEN_VALUE	19
#endif

#endif /* TOKLST_H_DEFINED_ */
