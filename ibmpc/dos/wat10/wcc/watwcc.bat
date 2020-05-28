:# Compile bibclean with the Watcom C/C++ 10.0 compiler
:# [02-May-1996]
wcc -I. -mc -ox -zq -zt512 bibclean.c
wcc -I. -mc -ox -zq -zt512 chek.c
wcc -I. -mc -ox -zq -zt512 do.c
wcc -I. -mc -ox -zq -zt512 fix.c
wcc -I. -mc -ox -zq -zt512 fndfil.c
wcc -I. -mc -ox -zq -zt512 isbn.c
wcc -I. -mc -ox -zq -zt512 keybrd.c
wcc -I. -mc -ox -zq -zt512 match.c
wcc -I. -mc -ox -zq -zt512 -DHOST="plot79.math.utah.edu" -DUSER="beebe" option.c
wcc -I. -mc -ox -zq -zt512 romtol.c
wcc -I. -mc -ox -zq -zt512 strist.c
wcc -I. -mc -ox -zq -zt512 wildargv.c

wcl -k32768 -mc -ox -fm=bibclean.map bibclean.obj c*.obj d*.obj f*.obj i*.obj k*.obj m*.obj o*.obj r*.obj s*.obj w*.obj
