:# Compile bibclean with the Microsoft C 6.0 compiler
:# [02-May-1996]
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 bibclean.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 fndfil.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 romtol.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 match.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 chek.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 do.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 fix.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 fndfil.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 isbn.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 keybrd.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 match.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 -DHOST=\"plot79.math.utah.edu\" -DUSER=\"beebe\" option.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 romtol.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 strist.c
cl -c -I. -AC -W4 -D__STDC__=1 -Gt512 strtol.c
copy s:\sys\msc6p0\lib\setargv.obj setargv.obj
cl -AC -Gt512 -W4 -Febibclean.exe -Fmbibclean.map -F 8000 *.obj -link /noe
