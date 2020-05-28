#ifndef ISBN_H_DEFINED_
#define ISBN_H_DEFINED_

extern void		do_ISBN_file ARGS((/*@null@*/ const char *pathlist_, /*@null@*/ const char *name_));
extern void		do_print_ISBN_table ARGS((void));
extern void		free_ISBN_table ARGS((void));
extern void		ISBN_initialize ARGS((void));
#endif /* ISBN_H_DEFINED_ */
