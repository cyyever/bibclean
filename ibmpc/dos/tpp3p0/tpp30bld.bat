:# Build bibclean with Turbo C/C++ 3.0 in C++ compilation mode on IBM PC DOS.
:#
:# It is assumed that your ???/tc???/bin/turboc.cfg file sets library
:# and link search paths appropriately.  My turboc.cfg file says
:#
:#	-IS:\SYS\TC3P0\INCLUDE
:#	-LS:\SYS\TC3P0\LIB
:#
:# [02-May-1996]

:# You'll need to modify the directory path for wildargs.obj, which
:# provides wild-card expansion of command line arguments.
:# set WILDARGS=s:\sys\tc3p0\lib\wildargs.obj
:#======================================================================
:# Set the desired compiler flags.  These should not require changing.
:# -P (C++ compilation), -A (ANSI keywords), -N (stack overflow check),
:# -mc (compact memory model), -v (source debugging), -y (debug line
:# numbers).
:# set CFLAGS=-P -A -N -mc -v -y
:# The executable produced is considerably smaller for C than for C++,
:# so we remove -P -v -y for production use.
tcc -P -2 -G -Z -O -I. -A -N -mh -c bibclean.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c chek.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c do.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c fix.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c match.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c -DMAXPATHLEN=260 fndfil.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c isbn.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c keybrd.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c match.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c -DHOST="plot79.math.utah.edu" -DUSER="beebe" option.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c romtol.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c strist.c
tcc -P -2 -G -Z -O -I. -A -N -mh -c strtol.c
tcc -P -N -mh bibclean.obj c*.obj d*.obj f*.obj i*.obj k*.obj m*.obj o*.obj r*.obj s*.obj s:\sys\tc3p0\lib\wildargs.obj
