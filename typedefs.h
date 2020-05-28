#ifndef TYPEDEFS_H_DEFINED_
#define TYPEDEFS_H_DEFINED_

typedef struct s_name_pair
{
    /*@null@*/ const char *old_name;
    /*@null@*/ const char *new_name;
} NAME_PAIR;

typedef struct s_option_function_entry
{
    /*@null@*/ const char *name;		/* option name */
    size_t min_match;		/* minimum length string match */
    /*@null@*/ void (*function)(VOID);	/* function to call when option matched */
} OPTION_FUNCTION_ENTRY;

typedef struct s_parse_data
{
    YESorNO	(*is_name_char) ARGS((int c_, size_t n_));
    const char	*s;		/* pointer to next char in list */
    const char	*token;		/* pointer to token in list */
    size_t	token_length;	/* number of token characters (== (s - token)) */
} parse_data;

typedef struct s_pattern_table
{
    MATCH_PATTERN *patterns;
    int current_size;
    int maximum_size;
} PATTERN_TABLE;

typedef struct s_pattern_names
{
    const char *name;
    PATTERN_TABLE *table;
} PATTERN_NAMES;

typedef struct s_position
{
    const char *filename;
    long byte_position;
    long last_column_position;
    long column_position;
    long line_number;
} POSITION;

typedef struct s_io_pair
{
    POSITION input;
    POSITION output;
} IO_PAIR;

#endif /* TYPEDEFS_H_DEFINED_ */
