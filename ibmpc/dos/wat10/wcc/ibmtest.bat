echo off
break on
REM =====================================================================
REM  Test bibclean on IBM PC DOS.
REM  The funny backspace in echo commands is just to get a blank
REM  line, rather than the current echo status.
REM  [01-May-1996]
REM =====================================================================

echo ================== begin BibTeX test 1 ===================
echo bibclean -init bibclean.ini -err testbib1.err  testbib1.org
bibclean -init bibclean.ini -err testbib1.err  testbib1.org >testbib1.bib
echo 
echo There should be no differences found:
echo diff testbib1.bok testbib1.bib
diff testbib1.bok testbib1.bib
echo 
echo There should be no differences found:
echo diff testbib1.eok testbib1.err
diff testbib1.eok testbib1.err
echo =================== end BibTeX test 1 ====================

echo ================== begin BibTeX test 2 ===================
echo bibclean -init bibclean.ini -no-check-values -err testbib2.err testbib2.org 
bibclean -init bibclean.ini -no-check-values -err testbib2.err testbib2.org >testbib2.bib
echo 
echo There should be no differences found:
echo diff testbib2.bok testbib2.bib
diff testbib2.bok testbib2.bib
echo 
echo There should be no differences found:
echo diff testbib2.eok testbib2.err
diff testbib2.eok testbib2.err
echo =================== end BibTeX test 2 ====================

echo ================== begin BibTeX test 3 ===================
echo bibclean -init bibclean.ini -fix-font-change -err testbib3.err testbib3.org 
bibclean -init bibclean.ini -fix-font-change -err testbib3.err testbib3.org >testbib3.bib
echo 
echo There should be no differences found:
echo diff testbib3.bok testbib3.bib
diff testbib3.bok testbib3.bib
echo 
echo There should be no differences found:
echo diff testbib3.eok testbib3.err
diff testbib3.eok testbib3.err
echo =================== end BibTeX test 3 ====================

echo ================== begin BibTeX test 4 ===================
echo bibclean -init bibclean.ini -fix-font-change -err testbib4.err testbib4.org 
bibclean -init bibclean.ini -fix-font-change -err testbib4.err testbib4.org >testbib4.bib
echo 
echo There should be no differences found:
echo diff testbib4.bok testbib4.bib
diff testbib4.bok testbib4.bib
echo 
echo There should be no differences found:
echo diff testbib4.eok testbib4.err
diff testbib4.eok testbib4.err
echo =================== end BibTeX test 4 ====================

echo ================== begin BibTeX test 5 ===================
echo bibclean -init bibclean.ini -German-style -err testbib5.err testbib5.org 
bibclean -init bibclean.ini -German-style -err testbib5.err testbib5.org >testbib5.bib
echo 
echo There should be no differences found:
echo diff testbib5.bok testbib5.bib
diff testbib5.bok testbib5.bib
echo 
echo There should be no differences found:
echo diff testbib5.eok testbib5.err
diff testbib5.eok testbib5.err
echo =================== end BibTeX test 5 ====================

echo ================== begin BibTeX test 6 ===================
echo bibclean -init bibclean.ini -err testisxn.err testisxn.org
bibclean -init bibclean.ini -err testisxn.err testisxn.org >testisxn.bib
echo 
echo There should be no differences found:
echo diff testisxn.bok testisxn.bib
diff testisxn.bok testisxn.bib
echo 
echo There should be no differences found:
echo diff testisxn.eok testisxn.err
diff testisxn.eok testisxn.err
echo =================== end BibTeX test 6 ===================

echo ================== begin BibTeX test 7 ===================
echo bibclean -init bibclean.ini -err testcodn.err testcodn.org
bibclean  -init bibclean.ini -err testcodn.err testcodn.org >testcodn.bib
echo 
echo There should be no differences found:
echo diff testcodn.bok testcodn.bib
diff testcodn.bok testcodn.bib
echo 
echo There should be no differences found:
echo diff testcodn.eok testcodn.err
diff testcodn.eok testcodn.err
echo =================== end BibTeX test 7 ===================

echo ================== begin Scribe test 1 ===================
echo bibclean -init bibclean.ini -err testscr1.err -scribe -no-check testscr1.org
bibclean -init bibclean.ini -err testscr1.err -scribe -no-check testscr1.org >testscr1.bib
echo 
echo There should be no differences found:
echo diff testscr1.bok testscr1.bib
diff testscr1.bok testscr1.bib
echo 
echo There should be no differences found:
echo diff testscr1.eok testscr1.err
diff testscr1.eok testscr1.err
echo =================== end Scribe test 1 ====================

echo ==================== begin Scribe test 2 =====================
echo 
bibclean -err testscr2.err -init-file bibclean.ini -scribe -file -no-check testscr2.org >testscr2.bib
echo 
echo There should be no differences found:
echo diff testscr2.bok testscr2.bib
echo 
echo There should be no differences found:
echo diff testscr2.eok testscr2.err
echo 
echo 
bibclean -err testscr2.er2 -init-file bibclean.ini -scribe -file -no-check -no-par testscr2.org >testscr2.bi2
echo 
echo There should be no differences found:
echo diff testscr2.bo2 testscr2.bi2
echo 
echo There should be no differences found:
echo diff testscr2.eo2 testscr2.er2
echo 
echo ===================== end Scribe test 2 ======================

echo ================== begin Scribe test 3 ===================
echo bibclean -init bibclean.ini -err testscr3.err -scribe -no-check testscr3.org
bibclean -init bibclean.ini -err testscr3.err -scribe -no-check testscr3.org >testscr3.bib
echo 
echo There should be no differences found:
echo diff testscr3.bok testscr3.bib
diff testscr3.bok testscr3.bib
echo 
echo There should be no differences found:
echo diff testscr3.eok testscr3.err
diff testscr3.eok testscr3.err
echo =================== end Scribe test 3 ====================
