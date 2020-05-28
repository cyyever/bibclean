dnl Process this file with autoconf to produce a configure script.
AC_INIT(bibclean,3.04,[Nelson H. F. Beebe <beebe@math.utah.edu>])
PACKAGE_DATE="21-Nov-2019"

AC_CONFIG_SRCDIR(bibclean.c)

AH_TEMPLATE(PACKAGE_BUGREPORT,	[Package bug-reporting address])
AH_TEMPLATE(PACKAGE_DATE,	[Package date])
AH_TEMPLATE(PACKAGE_NAME,	[Package name])
AH_TEMPLATE(PACKAGE_STRING,	[Package string])
AH_TEMPLATE(PACKAGE_TARNAME,	[Package tarname])
AH_TEMPLATE(PACKAGE_VERSION,	[Package version])
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT,	"$PACKAGE_BUGREPORT")
AC_DEFINE_UNQUOTED(PACKAGE_DATE,	"$PACKAGE_DATE")
AC_DEFINE_UNQUOTED(PACKAGE_NAME,	"$PACKAGE_NAME")
AC_DEFINE_UNQUOTED(PACKAGE_STRING,	"$PACKAGE_STRING")
AC_DEFINE_UNQUOTED(PACKAGE_TARNAME,	"$PACKAGE_TARNAME")
AC_DEFINE_UNQUOTED(PACKAGE_VERSION,	"$PACKAGE_VERSION")

AC_CONFIG_HEADER(config.h:config.hin)

dnl Checks for support programs

AC_CHECK_PROGS(ANTIC,		antic)
AC_CHECK_PROGS(AUTOCONF,	autoconf)
AC_CHECK_PROGS(AUTOHEADER,	autoheader)
AC_CHECK_PROGS(AWK,		mawk nawk gawk bawk awk)
AC_CHECK_PROGS(BIBTEX,		bibtex)
AC_CHECK_PROGS(CAT,		cat)
AC_CHECK_PROGS(CHECKSUM,	checksum)
AC_CHECK_PROGS(CHMOD,		chmod)
AC_CHECK_PROGS(CMP,		cmp)

dnl We want a copy command that has the -p (preserve time stamp) flag,
dnl but older cp implementations lack it.  scp and rcp always have it,
dnl but some sites disable rcp when scp is installed, so we must use
dnl scp if it is available.
AC_CHECK_PROG(CP,		scp,scp -p)
AC_CHECK_PROG(CP,		rcp,rcp -p)
AC_CHECK_PROG(CP,		cp,cp)

AC_CHECK_PROGS(CPPCHECK,	cppcheck)
AC_CHECK_PROGS(CUT,		cut)
AC_CHECK_PROGS(DATE,		date)
AC_CHECK_PROGS(DIFF,		diff)
AC_CHECK_PROGS(DISTILL,		distill ps2pdf)
AC_CHECK_PROGS(DITROFF,		ditroff groff)
AC_CHECK_PROGS(ETAGS,		etags)
AC_CHECK_PROGS(EXPR,		expr)
AC_CHECK_PROGS(FLAWFINDER,	flawfinder)
AC_CHECK_PROGS(GREP,		egrep grep)
AC_CHECK_PROGS(GZIP,		gzip compress)
AC_CHECK_PROGS(HTMLNCHECK,	html-ncheck)
AC_CHECK_PROGS(HTMLPRETTY,	html-pretty)
AC_CHECK_PROGS(ITS4,		its4)
AC_CHECK_PROGS(JAR,		jar tar)
AC_CHECK_PROGS(LATEX,		latex2e latex)
AC_CHECK_PROGS(LINT,		lint)
AC_CHECK_PROGS(LN,		ln)
AC_CHECK_PROGS(LS,		ls)
AC_CHECK_PROGS(MAN2HTML,	man2html)
AC_CHECK_PROGS(MKDIR,		mkdir)
AC_CHECK_PROGS(MV,		mv)
AC_CHECK_PROGS(NROFF,		nroff)
AC_CHECK_PROGS(RATS,		rats)
AC_CHECK_PROGS(RM,		rm)
AC_CHECK_PROGS(SED,		sed)
AC_CHECK_PROGS(SPLINT,		[splint lclint])
AC_PATH_PROG(SHELL,		sh)
AC_CHECK_PROGS(STRIP,		strip)
AC_CHECK_PROGS(TAR,		gtar gnutar tar)
AC_CHECK_PROGS(TEST,		test)
AC_CHECK_PROGS(TOUCH,		touch)
AC_CHECK_PROGS(TR,		tr)
AC_CHECK_PROGS(UNO,		uno)
AC_CHECK_PROGS(UNZIP,		unzip)
AC_CHECK_PROGS(WINE,		wine)
AC_CHECK_PROGS(ZIP,		zip false)
AC_CHECK_PROGS(ZOO,		zoo false)

dnl Checks for NeXT -posix
AC_MSG_CHECKING(whether we have NeXT system, and so need -posix)
if test -d /NextAdmin
then CFLAGS=-posix
AC_MSG_RESULT(yes)
else
AC_MSG_RESULT(no)
fi

dnl Checks for compiler
AC_PROG_CC

dnl Checks for ALL (IBM RS/6000 c89 needs _ALL_SOURCE when ioctl.h is
dnl used with winsize)
AC_MSG_CHECKING(whether we need _ALL_SOURCE)
all=no
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/ioctl.h>
struct winsize w;
], , , all=maybe)
if test $all = maybe ; then
AC_TRY_COMPILE([
#define _ALL_SOURCE
#include <sys/types.h>
#include <sys/ioctl.h>
struct winsize w;
], , AC_DEFINE(_ALL_SOURCE) all=yes)
fi
if test $all = maybe ; then all=no ; fi
AC_MSG_RESULT($all)

dnl Checks for POSIX (HP-UX c89 needs _POSIX_SOURCE when stat.h is used)
AC_MSG_CHECKING(whether we need _POSIX_SOURCE)
posix=no
AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/stat.h>
struct stat buffer;
], , , posix=maybe)
if test $posix = maybe ; then
AC_TRY_COMPILE([
#define _POSIX_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
struct stat buffer;
], , AC_DEFINE(_POSIX_SOURCE) posix=yes)
fi
if test $posix = maybe ; then posix=no ; fi
AC_MSG_RESULT($posix)

dnl Checks for Standard C and C++ function declarations.
AC_MSG_CHECKING(for Standard C and C++ function declarations)
AC_TRY_COMPILE([void foo(void){}
int bar(int x, float y, char z){return 1;}], ,
  [AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_STDC)],
    AC_MSG_RESULT(no))

dnl Checks for SunOS C++ free() argument type bug
AC_MSG_CHECKING(for SunOS C++ free() argument type bug)
freebug=no
AC_TRY_COMPILE([#include <stdlib.h>],
[void *p; p = (void*)0; free(p)],
, freebug=maybe)

if test $freebug = maybe ; then
AC_TRY_COMPILE([#include <stdlib.h>],
[void *p; p = (void*)0; free((char*)p)],
AC_DEFINE(FREE_CAST_IS_CHAR_STAR) freebug=yes)
fi
if test $freebug = maybe ; then freebug=no ; fi
AC_MSG_RESULT($freebug)

dnl Checks for Standard C alert character
AC_MSG_CHECKING(for Standard C alert character)
havealert=yes
AC_TRY_RUN(int main(){return((int)'\a'-007);},
    [havealert=yes
     AC_DEFINE(HAVE_ALERT_CHAR)],
    havealert=no,
    havealert=no)
AC_MSG_RESULT($havealert)

dnl Checks for programs.

dnl Checks for libraries.

dnl Checks for header file conflicts.  For the lcc compiler on at
dnl least Compaq/DEC Alpha OSF/1 4.0, <sys/param.h> conflicts with
dnl <limits.h>, sigh...  NB: This test must come BEFORE the
dnl AC_CHECK_HEADERS() test below!
AC_MSG_CHECKING(for <limits.h> <sys/param.h> compatibility)
AC_TRY_COMPILE(
    [#include <sys/param.h>
    #include <limits.h>],
    [],
    [AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)
     ac_cv_header_sys_param_h=no}])


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(conio.h ctype.h descrip.h errno.h fcntl.h io.h \
iodef.h jpidef.h libc.h limits.h osfcn.h pwd.h regex.h regexp.h \
rms.h sgtty.h ssdef.h stat.h stdbool.h stdio.h stdlib.h string.h sys/ioctl.h \
sys/param.h sys/stat.h sys/types.h termio.h termios.h time.h \
tt2def.h ttdef.h types.h unistd.h unixio.h )

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for bool and/or _Bool
AC_CHECK_TYPES(_Bool,,,[#include <stdbool.h>])

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_UTIME_NULL
AC_REPLACE_FUNCS(strtol)
AC_CHECK_FUNCS(getcwd getpwnam getwd re_comp strcspn strdup strspn strstr strtod strtol)
AC_CHECK_DECLS([fileno])

dnl This does not work properly for C++:
dnl AC_CHECK_FUNCS(isatty)

AC_MSG_CHECKING(for isatty)
AC_TRY_LINK(
	[
#if defined(HAVE_STDIO_H)
#include <stdio.h>
#endif
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
	],
	[(void)isatty(1)],
	[AC_MSG_RESULT(yes)
	 AC_DEFINE(HAVE_ISATTY)],
	[AC_MSG_RESULT(no)])

AC_MSG_CHECKING(for ioctl prototype)
AC_EGREP_CPP([ioctl[ 	]*[(]],
[
#include <sys/ioctl.h>
#include <unistd.h>
],
[
AC_DEFINE(HAVE_IOCTL_PROTOTYPE)
AC_MSG_RESULT(yes)
],
[
AC_MSG_RESULT(no)
])

dnl Remove -g from CFLAGS if we are compiling with lcc because it
dnl produces bad debug symbol tables on Sun Solaris 2.x.
if test "$CC" = "lcc"
then
    CFLAGS="`echo $CFLAGS | sed -e 's/-g //g' -e 's/ -g$//g' -e 's/^-g$//'`"
fi

AC_SUBST(VM)

dnl Support for OpenBSD 5.9 and later pledge() call to guarantee
dnl minimal privileges needed to run a program
AC_CHECK_HEADERS(sys/pledge.h)

test "x$ac_cv_exeext" = "x.exe" && VM='$(WINE)'

AH_TEMPLATE(FREE_CAST_IS_CHAR_STAR,[Define if free() is of type char*.])
AH_TEMPLATE(HAVE_ALERT_CHAR,[Define if \a is recognized in C strings.])
AH_TEMPLATE(HAVE_IOCTL_PROTOTYPE,[Define if the compiler can see an ioctl() prototype.])
AH_TEMPLATE(HAVE_ISATTY,[Define if isatty() is available.])
AH_TEMPLATE(HAVE_STDC,[Define if Standard C and C++ function declarations are supported.])
AH_TEMPLATE(_ALL_SOURCE,[Define if needed to expose winsize in ioctl.h.])
AH_TEMPLATE(_POSIX_SOURCE,[Define if needed with stat.h.])
AH_BOTTOM([
#define SCREEN_LINES        24
#include <custom.h>])
AC_OUTPUT(Makefile)
