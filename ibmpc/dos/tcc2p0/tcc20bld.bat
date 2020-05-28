:# Build bibclean with Turbo C 2.0 on IBM PC DOS.
:#
:# It is assumed that your ???/tc???/bin/turboc.cfg file sets library
:# and link search paths appropriately.  My turboc.cfg file says
:#
:#	-IS:\SYS\TC\INCLUDE
:#	-LS:\SYS\TC\LIB
:#
:# [02-May-1996]

:# You'll need to modify the directory path for wildargs.obj, which
:# provides wild-card expansion of command line arguments.
:# set WILDARGS=s:\sys\tc\wildargs.obj

:#======================================================================
:# Set the desired compiler flags.  These should not require changing.
:# -A (ANSI keywords), -N (stack overflow check), -mc (compact memory
:# model), -v (source debugging), -y (debug line numbers)
:# set CFLAGS=-A -N -mc -v -y

tcc -I. -A -N -mh -c bibclean.c
tcc -I. -A -N -mh -c chek.c
tcc -I. -A -N -mh -c do.c
tcc -I. -A -N -mh -c fix.c
tcc -I. -A -N -mh -c match.c
tcc -I. -A -N -mh -c -DMAXPATHLEN=260 fndfil.c
tcc -I. -A -N -mh -c isbn.c
tcc -I. -A -N -mh -c keybrd.c
tcc -I. -A -N -mh -c match.c
tcc -I. -A -N -mh -c -DHOST="plot79.math.utah.edu" -DUSER="beebe" option.c
tcc -I. -A -N -mh -c romtol.c
tcc -I. -A -N -mh -c strist.c
tcc -I. -A -N -mh -c strtol.c
tcc -I. -A -N -mh bibclean.obj c*.obj d*.obj f*.obj i*.obj k*.obj m*.obj o*.obj r*.obj s*.obj s:\sys\tc\wildargs.obj
