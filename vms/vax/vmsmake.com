$ !=====================================================================
$ ! Command file to build bibclean on VAX VMS (6.1)
$ ! [02-May-1996]
$ !=====================================================================
$ ! This should be set system-wide, but is not on some systems;
$ ! without it, the compiler cannot find header files.
$ define /log DECC$LIBRARY_INCLUDE SYS$LIBRARY
$ !
$ CFLAGS :== "/DEBUG=ALL /INCLUDE_DIRECTORY=([],sys$library:) /NOOPTIMIZE /WARNING /STANDARD=PORTABLE"
$ ! We use /noshareable so the .exe file is independent of VMS release 
$ ! numbers
$ LFLAGS := "/debug /noshareable"
$ LFLAGS := "/noshareable"
$ write sys$error "============================================================"
$ write sys$error "Compiling bibclean.c"
$ cc 'CFLAGS' bibclean.c
$ write sys$error "============================================================"
$ write sys$error "Compiling chek.c"
$ cc 'CFLAGS' chek.c
$ write sys$error "============================================================"
$ write sys$error "Compiling do.c"
$ cc 'CFLAGS' do.c
$ write sys$error "============================================================"
$ write sys$error "Compiling fix.c"
$ cc 'CFLAGS' fix.c
$ write sys$error "============================================================"
$ write sys$error "Compiling fndfil.c"
$ cc 'CFLAGS' fndfil.c
$ write sys$error "============================================================"
$ write sys$error "Compiling isbn.c"
$ cc 'CFLAGS' isbn.c
$ write sys$error "============================================================"
$ write sys$error "Compiling keybrd.c"
$ cc 'CFLAGS' keybrd.c
$ write sys$error "============================================================"
$ write sys$error "Compiling match.c"
$ cc 'CFLAGS' match.c
$ write sys$error "============================================================"
$ write sys$error "Compiling option.c"
$ cc 'CFLAGS' option.c
$ write sys$error "============================================================"
$ write sys$error "Compiling romtol.c"
$ cc 'CFLAGS' romtol.c
$ write sys$error "============================================================"
$ write sys$error "Compiling strist.c"
$ cc 'CFLAGS' strist.c
$ write sys$error "============================================================"
$ write sys$error "Compiling vaxvms.c"
$ cc 'CFLAGS' vaxvms.c
$ write sys$error "============================================================"
$ write sys$error "Compiling vmswild.c: Expect 16 warnings and 4 informational messages"
$ cc 'CFLAGS' vmswild.c
$ write sys$error "============================================================"
$ write sys$error "Linking bibclean: Expect 1 %W message about system()--ignore it"
$ link /exe=bibclean.exe 'LFLAGS' -
	bibclean.obj, chek.obj, do.obj, fix.obj, fndfil.obj, -
	isbn.obj, keybrd.obj, match.obj, option.obj, romtol.obj, -
	strist.obj, vaxvms.obj, vmswild.obj, -
	sys$library:vaxcrtl.olb/lib
