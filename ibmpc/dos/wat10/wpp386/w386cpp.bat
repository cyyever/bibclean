:# Compile bibclean with the Watcom C/C++ 10.0 compiler
:# [02-May-1996]
wpp386 -I. -zq bibclean.c
wpp386 -I. -zq chek.c
wpp386 -I. -zq do.c
wpp386 -I. -zq fix.c
wpp386 -I. -zq fndfil.c
wpp386 -I. -zq isbn.c
wpp386 -I. -zq keybrd.c
wpp386 -I. -zq match.c
wpp386 -I. -zq -DHOST="plot79.math.utah.edu" -DUSER="beebe" option.c
wpp386 -I. -zq romtol.c
wpp386 -I. -zq strist.c
wpp386 -I. -zq wildargv.c

wcl386 -k131072 bibclean.obj c*.obj d*.obj f*.obj i*.obj k*.obj m*.obj o*.obj r*.obj s*.obj w*.obj
