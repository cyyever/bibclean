:# Compile bibclean with the Watcom C/C++ 10.0 compiler
:# [02-May-1996]
wcc386 -I. -zq bibclean.c
wcc386 -I. -zq chek.c
wcc386 -I. -zq do.c
wcc386 -I. -zq fix.c
wcc386 -I. -zq fndfil.c
wcc386 -I. -zq isbn.c
wcc386 -I. -zq keybrd.c
wcc386 -I. -zq match.c
wcc386 -I. -zq -DHOST="plot79.math.utah.edu" -DUSER="beebe" option.c
wcc386 -I. -zq romtol.c
wcc386 -I. -zq strist.c
wcc386 -I. -zq wildargv.c

wcl386 -k131072 bibclean.obj c*.obj d*.obj f*.obj i*.obj k*.obj m*.obj o*.obj r*.obj s*.obj w*.obj
