$ !=====================================================================
$ ! Test bibclean on DEC VAX VMS 6.1.
$ ! [02-May-1996]
$ !=====================================================================
$ !
$ ! Create a foreign command symbol for bibclean.  You must edit 
$ ! this to suit your local configuration, because VMS has no
$ ! way that I know of to get the full path name of the current
$ ! directory.
$ bibclean :== $public$disk:[nbeebe.bibclean]bibclean.exe
$ !
$ ! Some of the tests intentionally generate a failure return code.
$ ! Prevent that from stopping this command file.
$ set noon
$ !
$ write sys$output "================== begin BibTeX test 1 ==================="
$ define /user sys$error testbib1.err
$ bibclean -init bibclean.ini testbib1.org >testbib1.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib1.bok testbib1.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib1.eok testbib1.err
$ write sys$output "=================== end BibTeX test 1 ===================="
$ !
$ write sys$output "================== begin BibTeX test 2 ==================="
$ define /user sys$error testbib2.err
$ bibclean -init bibclean.ini -no-check-values testbib2.org >testbib2.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib2.bok testbib2.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib2.eok testbib2.err
$ write sys$output "=================== end BibTeX test 2 ===================="
$ !
$ write sys$output "================== begin BibTeX test 3 ==================="
$ define /user sys$error testbib3.err
$ bibclean -init bibclean.ini -fix-font-change  testbib3.org >testbib3.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib3.bok testbib3.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib3.eok testbib3.err
$ write sys$output "=================== end BibTeX test 3 ===================="
$ !
$ write sys$output "================== begin BibTeX test 4 ==================="
$ define /user sys$error testbib4.err
$ bibclean -init bibclean.ini -fix-font-change testbib4.org >testbib4.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib4.bok testbib4.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib4.eok testbib4.err
$ write sys$output "=================== end BibTeX test 4 ===================="
$ !
$ write sys$output "================== begin BibTeX test 5 ==================="
$ define /user sys$error testbib5.err
$ bibclean -init bibclean.ini -German-style testbib5.org >testbib5.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib5.bok testbib5.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testbib5.eok testbib5.err
$ write sys$output "=================== end BibTeX test 5 ===================="
$ !
$ write sys$output "================== begin BibTeX test 6 ==================="
$ define /user sys$error testisxn.err
$ bibclean -init bibclean.ini testisxn.org >testisxn.bib
$ write sys$output ""
$ diff testisxn.bok testisxn.bib
$ write sys$output ""
$ diff testisxn.eok testisxn.err
$ write sys$output "=================== end BibTeX test 6 ==================="
$ !
$ write sys$output "================== begin BibTeX test 7 ==================="
$ define /user sys$error testcodn.err
$ bibclean -init bibclean.ini testcodn.org >testcodn.bib
$ write sys$output ""
$ diff testcodn.bok testcodn.bib
$ write sys$output ""
$ diff testcodn.eok testcodn.err
$ write sys$output "=================== end BibTeX test 7 ==================="
$ !
$ write sys$output "================== begin Scribe test 1 ==================="
$ define /user sys$error testscr1.err
$ bibclean -init bibclean.ini -scribe -no-check testscr1.org >testscr1.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testscr1.bok testscr1.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testscr1.eok testscr1.err
$ write sys$output "=================== end Scribe test 1 ===================="
$ !
$ write sys$output "================== begin Scribe test 2 ==================="
$ define /user sys$error testscr2.err
$ bibclean -init bibclean.ini -scribe -no-check testscr2.org >testscr2.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testscr2.bok testscr2.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testscr2.eok testscr2.err
$ write sys$output "=================== end Scribe test 2 ===================="
$ !
$ write sys$output "================== begin Scribe test 2a==================="
$ define /user sys$error testscr2.er2
$ bibclean -init bibclean.ini -scribe -file-pos -no-check -no-par -
	testscr2.org >testscr2.bi2
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testscr2.bo2 testscr2.bi2
$ write sys$output ""
$ diff testscr2.eo2 testscr2.er2
$ write sys$output "=================== end Scribe test 2a===================="
$ !
$ write sys$output "================== begin Scribe test 3 ==================="
$ define /user sys$error testscr3.err
$ bibclean -init bibclean.ini -scribe -no-check testscr3.org >testscr3.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testscr3.bok testscr3.bib
$ write sys$output ""
$ write sys$output "There should be no differences found:"
$ diff testscr3.eok testscr3.err
$ write sys$output "=================== end Scribe test 3 ===================="
$ !
