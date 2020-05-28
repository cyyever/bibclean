s/^char \$ac_func();/#if defined(__cplusplus) || defined(c_plusplus)\
#include <stdlib.h>\
#include <string.h>\
#else\
char \$ac_func();\
#endif/
s/^\$ac_func();/#if defined(__cplusplus) || defined(c_plusplus)\
void (*pfun)(void) = (void (*)(void))\$ac_func;\
#else\
\$ac_func();\
#endif/
s/exit *(/return (/
s/CFLAGS=" *-g */CFLAGS="/
s/CFLAGS=" *-O2 */CFLAGS="/
s/CXXFLAGS=" *-g */CXXFLAGS="/
s/CXXFLAGS=" *-O2 */CXXFLAGS="/

