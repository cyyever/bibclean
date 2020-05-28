#ifndef MATCH_H_DEFINED_
#define MATCH_H_DEFINED_
typedef struct s_pattern
{
    /*@null@*/ const char *pattern;
    /*@null@*/ const char *message;
} MATCH_PATTERN;

YESorNO		match_pattern ARGS((const char *s_, const char *pattern_));

#define BIBTEX_HIDDEN_DELIMITER	'\001' /* brackets inline comments in values */
				/* to hide them from pattern matching */
#endif /* MATCH_H_DEFINED_ */
