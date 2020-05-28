:# Compile bibclean with the Microsoft C 5.1 compiler
:# [02-May-1996]
cl -c -I. -AH -W3 -Gt512 bibclean.c
cl -c -I. -AH -W3 -Gt512 fndfil.c
cl -c -I. -AH -W3 -Gt512 romtol.c
cl -c -I. -AH -W3 -Gt512 match.c
cl -c -I. -AH -W3 -Gt512 chek.c
cl -c -I. -AH -W3 -Gt512 do.c
cl -c -I. -AH -W3 -Gt512 fix.c
cl -c -I. -AH -W3 -Gt512 fndfil.c
cl -c -I. -AH -W3 -Gt512 isbn.c
cl -c -I. -AH -W3 -Gt512 keybrd.c
cl -c -I. -AH -W3 -Gt512 match.c
cl -c -I. -AH -W3 -Gt512 -DHOST=\"plot79.math.utah.edu\" -DUSER=\"beebe\" option.c
cl -c -I. -AH -W3 -Gt512 romtol.c
cl -c -I. -AH -W3 -Gt512 strist.c
cl -c -I. -AH -W3 -Gt512 strtol.c
cl -AH -Febibclean.exe -F 8000 -Gt512 s:\sys\msc5p1\lib\setargv.obj *.obj -link /noe
:# exemod bibclean.exe /stack 8000
