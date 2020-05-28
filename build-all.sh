#! /bin/sh
#=======================================================================
# This script attempts to build and check a program on every UNIX C
# and C++ compiler available to the author.
#
# Because of wide variations in optimizer switches, no attempt is yet
# made to validate the same compiler at multiple optimization levels:
# that job is left for the installer.
#
# Usage:
#	build-all.sh |& tee -a typescript.xxx
#
# The program author can examine the output in the typescript.xxx file
# to determine what, if any, changes are needed in the code to ensure
# that it will build correctly.
#
# [08-Nov-1999] -- update with more compilers, and use separate 
#		   doconfig.sh to work around lack of shell functions on
#		   some systems
# [30-Sep-1995]
#=======================================================================

# Originally, I used a shell function with a workaround on DECstation
# ULTRIX, where /bin/sh does not support shell functions, but
# /usr/bin/ksh and /usr/local/bin/bash do, of feeding the input stream
# inline to an alternate shell. Unfortunately, this failed on Sun SunOS
# 4.x, where the first time that the shell function was executed, it
# exited, rather than returning to the caller.  This version therefore
# moves that shell function into a separate file, doconfig.sh.

# Here is a list of known compilers on DEC (MIPS ULTRIX and Alpha
# OSF/1), HP (PA-RISC HP-UX), IBM (POWER AIX), IBM PC (Intel LINUX),
# NeXT (Motorola 68K Mach) SGI (MIPS), and Sun (SPARC SunOS and SPARC
# Solaris) systems.  We try all that actually exist on this system.

./doconfig.sh /bin/c++
./doconfig.sh /bin/c89
./doconfig.sh /bin/CC
./doconfig.sh /bin/cc
./doconfig.sh /bin/cxx
./doconfig.sh /bin/xlC
./doconfig.sh /opt/SUNWspro/bin/c89
./doconfig.sh /opt/SUNWspro/bin/CC
./doconfig.sh /opt/SUNWspro/bin/cc
./doconfig.sh /usr/bin/c89
./doconfig.sh /usr/bin/CC
./doconfig.sh /usr/bin/cc
./doconfig.sh /usr/bin/g++
./doconfig.sh /usr/bin/gcc
./doconfig.sh /usr/ccs/bin/c89
./doconfig.sh /usr/ccs/bin/cc
./doconfig.sh /usr/lang/acc
./doconfig.sh /usr/lang/CC
./doconfig.sh /usr/local/bin/g++
./doconfig.sh /usr/local/bin/gcc
./doconfig.sh /usr/local/bin/lcc
./doconfig.sh /usr/local/bin/lcc "-A -A"
./doconfig.sh /usr/ucb/cc

echo ================================= Done =================================
