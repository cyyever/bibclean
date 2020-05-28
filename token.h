#ifndef TOKEN_H_DEFINED_
#define TOKEN_H_DEFINED_

#if !defined(MAX_TOKEN)
/*
** The value of MAX_TOKEN was originally 4093, but that proved too
** small for many abstract and tableofcontents strings.
**
** A scan of more than 1,500,000 entries in the TeX User Group and
** BibNet Project archives found that the longest complete BibTeX
** entry was less than 43,200 characters, with a longest field value
** of abour 40,600 characters, so a 64KB limit, less a bit
** of overhead, should be ample for future growth.
**
** Modern BibTeX implementations (2007 and later) have larger tables,
** and dynamic array resizing, so the old arbitrary limits on string
** sizes really do need to be relaxed, and reset to reasonable values
** based on real experience.
*/
#define MAX_TOKEN	65525	/* internal buffer size; no BibTeX string
				value may be larger than this. */
#endif /* !defined(MAX_TOKEN) */

#define MAX_TOKEN_SIZE	(MAX_TOKEN + 3)	/* Arrays are always dimensioned
				MAX_TOKEN_SIZE, so as to have space
				for an additional pair of braces and a
				trailing NUL, without tedious
				subscript checking in inner loops. */

#endif /* TOKEN_H_DEFINED_ */
