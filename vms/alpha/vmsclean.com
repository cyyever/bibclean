$ !=====================================================================
$ ! Cleanup after a successful build and test, leaving only the
$ ! sources, test files, and DEC Alpha OpenVMS executable.
$ ! [08-Oct-1993]
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
