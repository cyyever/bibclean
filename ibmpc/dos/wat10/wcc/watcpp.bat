:# Compile bibclean with the Watcom C/C++ 10.0 compiler
:# 
:# NB: Compilation of do.c fails, even without optimization, because
:# the compiler runs out of memory, and isbn.c requires a code change
:# to get compilation to work (see the ../README file).
:# 
:# [02-May-1996]
wcl -c -cc++ -I. -mc -ox -zq -zt512 bibclean.c
wcl -c -cc++ -I. -mc -zq -zt512 chek.c
wcl -c -cc++ -I. -mc -zq -zt512 do.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 fix.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 fndfil.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 isbn.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 keybrd.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 match.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 -DHOST="plot79.math.utah.edu" -DUSER="beebe" option.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 romtol.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 strist.c
wcl -c -cc++ -I. -mc -ox -zq -zt512 wildargv.c

wcl -cc++ -k32768 -mc -ox -fm=bibclean.map bibclean.obj c*.obj d*.obj f*.obj i*.obj k*.obj m*.obj o*.obj r*.obj s*.obj w*.obj
