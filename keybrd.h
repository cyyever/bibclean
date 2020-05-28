#ifndef KEYBRD_H_DEFINED_
#define KEYBRD_H_DEFINED_

#if (SCREEN_LINES > 0)
#if defined(HAVE_STDC)
typedef enum keyboard_code {
    KEYBOARD_EOF = EOF,
    KEYBOARD_UNKNOWN = 0,
    KEYBOARD_AGAIN,
    KEYBOARD_DOWN,
    KEYBOARD_END,
    KEYBOARD_HELP,
    KEYBOARD_HOME,
    KEYBOARD_PGDN,
    KEYBOARD_PGUP,
    KEYBOARD_QUIT,
    KEYBOARD_SEARCH_BACKWARD,
    KEYBOARD_SEARCH_FORWARD,
    KEYBOARD_UP,
    KEYBOARD_DOWN_PARAGRAPH,
    KEYBOARD_UP_PARAGRAPH
} keyboard_code_t;
#else /* K&R style */
#define KEYBOARD_EOF		EOF
#define KEYBOARD_UNKNOWN	0
#define KEYBOARD_AGAIN		1
#define KEYBOARD_DOWN		2
#define KEYBOARD_END		3
#define KEYBOARD_HELP		4
#define KEYBOARD_HOME		5
#define KEYBOARD_PGDN		6
#define KEYBOARD_PGUP		7
#define KEYBOARD_QUIT		8
#define KEYBOARD_SEARCH_BACKWARD 9
#define KEYBOARD_SEARCH_FORWARD	10
#define KEYBOARD_UP		11
#define KEYBOARD_DOWN_PARAGRAPH 12
#define KEYBOARD_UP_PARAGRAPH	13
typedef int keyboard_code_t;
#endif

extern int	do_more ARGS((FILE *fpout_, int line_, int pause_after_,
		    const char *lines[]));
extern int	get_screen_lines ARGS((void));
extern void	kbclose ARGS((void));
extern void	kbopen ARGS((void));
#endif /* SCREEN_LINES > 0 */

#endif /* KEYBRD_H_DEFINED_ */
