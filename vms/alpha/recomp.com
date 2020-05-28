$ !============================================================
$ ! Compile a bibclean file on DEC Alpha OpenVMS.
$ ! [08-Oct-1993]
$ !============================================================
$ set verify
$ set noon
$ !============================================================
$ show time
$ CFLAGS :== "/DEBUG=ALL /NOOPTIMIZE /WARNING /STANDARD=PORTABLE"
$ show symbol CFLAGS
$ cc 'CFLAGS' 'p1'.c
$ set noverify
