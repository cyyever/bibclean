$ !=====================================================================
$ ! Cleanup after a successful build and test, leaving only the
$ ! sources, test files, and VAX VMS executable.
$ ! [15-Nov-1992]
$ !=====================================================================
$ purge *.*
$ delete *.i.*
$ delete *.obj.*
$ delete test*.aux.*
$ delete test*.bi2.*
$ delete test*.bib.*
$ delete test*.blg.*
$ delete test*.dvi.*
$ delete test*.er2.*
$ delete test*.err.*
$ delete test*.log.*
