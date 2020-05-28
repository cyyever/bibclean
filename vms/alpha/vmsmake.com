$ !=====================================================================
$ ! Command file to build bibclean on DEC Alpha OpenVMS 1.5
$ ! [07-Oct-1993]
$ !=====================================================================
$ CFLAGS :== "/DEBUG=ALL /NOOPTIMIZE /WARNING /STANDARD=PORTABLE"
$ CFLAGS :== "           /OPTIMIZE   /WARNING /STANDARD=PORTABLE"
$ ! We use /noshareable so the .exe file is independent of VMS release 
$ ! numbers
$ LFLAGS := "/debug /noshareable"
$ LFLAGS := "/noshareable"
$ write sys$error "============================================================"
$ write sys$error "Compiling bibclean.c: Expect 0 messages"
$ cc 'CFLAGS' bibclean.c
$ write sys$error "============================================================"
$ write sys$error "Compiling fndfil.c: Expect 0 messages"
$ cc 'CFLAGS' fndfil.c
$ write sys$error "============================================================"
$ write sys$error "Compiling match.c: Expect 0 messages"
$ cc 'CFLAGS' match.c
$ write sys$error "============================================================"
$ write sys$error "Compiling romtol.c: Expect 0 messages"
$ cc 'CFLAGS' romtol.c
$ write sys$error "============================================================"
$ write sys$error "Compiling vaxvms.c: Expect 0 messages"
$ cc 'CFLAGS' vaxvms.c
$ write sys$error "============================================================"
$ write sys$error "Compiling vmswild.c: Expect 0 messages"
$ ! NB: It is ESSENTIAL to have /standard=vaxc here, overriding 
$ ! /standard=portable
$ cc 'CFLAGS' /standard=vaxc vmswild.c
$ write sys$error "============================================================"
$ write sys$error "Linking bibclean: Expect 3 %W messages about memchr(), memcmp(), and system()--ignore them"
$ link /exe=bibclean.exe 'LFLAGS' -
	bibclean.obj, fndfil.obj, match.obj, romtol.obj, -
	vaxvms.obj, vmswild.obj, -
	sys$library:vaxcrtl.olb/lib
