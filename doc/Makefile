#=======================================================================
# Makefile for bibclean TUBboat article and GNU Emacs info documentation
# [23-Aug-2003] -- add rules for bibclean.pdf and bibclean.ps, and 
#		   remove dependencies on .bbl and .ind files to prevent
#		   unnecessary rebuilds
# [03-May-1996] -- update style and macro files with latest versions,
#		   update GNU standard targets, and update spelling
#		   error checks to use style of TUG bibliography 
#		   collection
# [31-Dec-1993] -- final polishing of Makefile for public release
#                  after TUGboat publication
# [04-May-1993] -- add bundle, SHAR, SHARFILES, and add quoting of @
#		   in index
# [14-Jan-1993]
#=======================================================================
BIBTEX		= bibtex
CP		= /bin/cp
DETEX		= detex
DVIPS		= dvips -Pcmr
DVIPS		= dvips-type1
DW		= dw
ISPELL		= ispell
LATEX		= latex
MAKEINDEX 	= makeindex
MV		= /bin/mv
PS2PDF		= ps2pdf
PS2PDF		= distill
RM		= /bin/rm -f
SED		= sed
SHAR		= shar -c -v
SHARFILES       = Makefile bibclean.aux bibclean.bbl bibclean.bib \
		  bibclean.idx bibclean.ind bibclean.lot bibclean.ltx \
		  bibclean.sok bibclean.sty bibclean.toc is-plain.bst
SPELL		= spell
TEXINDEX 	= texindex

ALL:	bibclean.dvi bibclean.ps bibclean.pdf

all:	bibclean.ser dw bibclean.dvi

bibclean.aux:
	if [ ! -f bibclean.aux ] ; then touch bibclean.aux ; fi

bibclean.bbl:	bibclean.bib bibclean.aux
	if [ ! -f bibclean.aux ] ; then touch bibclean.aux ; fi
	-$(BIBTEX) bibclean

bibclean.dvi:	bibclean.ltx bibclean.sty
	$(LATEX) bibclean.ltx
	$(MAKE) bibclean.bbl bibclean.ind
	$(LATEX) bibclean.ltx
	$(LATEX) bibclean.ltx

bibclean.idx:
	if [ ! -f bibclean.idx ] ; then touch bibclean.idx ; fi

bibclean.ind:	bibclean.idx Makefile
	$(MV) bibclean.idx bibclean.idw
	$(SED) -e 's/{\\tt <}@/<@/' \
		-e 's/{\\tt >}@/>@/' \
		-e 's/{\\%\\%@{\\tt \\%\\%}}/{\\%\\%@{\\protect\\tt \\%\\%}}/' \
		-e 's/@@{\\tt @}/"@@{\\tt "@}/' \
		-e 's/{@@{\\tt @}}/{"@@{\\tt "@}}/' \
		-e 's/\\{ */\\{/g' \
		-e 's/\\} */\\}/g' \
		<bibclean.idw >bibclean.idx
	-$(MAKEINDEX) bibclean

bibclean.pdf:	bibclean.ps
	$(MV) bibclean.log bibclean.log.save
	$(PS2PDF) $?
	$(MV) bibclean.log.save bibclean.log 

bibclean.ps:	bibclean.dvi
	$(DVIPS) -o $@ $?

bibclean.ser:	bibclean.ltx bibclean.sok
	if [ ! -f bibclean.sok ] ; then touch bibclean.sok ; fi
	@echo '==================== spelling exceptions ===================='
	$(DETEX) bibclean.ltx | $(SED) -e 's/[0-9,.:;]/ /g' | \
		$(SPELL) +bibclean.sok >bibclean.ser
	$(DETEX) bibclean.ltx | $(SED) -e 's/[0-9,.:;]/ /g' | \
		$(ISPELL) -l -p bibclean.sok | sort -u >>bibclean.ser
	cat bibclean.ser

bundle:	$(SHARFILES)
	$(SHAR) $(SHARFILES) >$@

clean mostlyclean:
	$(RM) *.blg
	$(RM) *.dvi*
	$(RM) *.idw
	$(RM) *.ilg
	$(RM) *.log
	$(RM) *.o
	$(RM) *.ser
	$(RM) *.tmp
	$(RM) *~
	$(RM) \#*
	$(RM) core

clobber distclean:	clean
	$(RM) *.aux
	$(RM) *.bbl
	$(RM) *.idx
	$(RM) *.ind
	$(RM) *.lof
	$(RM) *.lot
	$(RM) *.toc

# We strip \index{} entries first to avoid large numbers of bogus duplicate
# word warnings.
dw:
	@echo '==================== doubled words ===================='
	$(SED) -e 's/^ *\\index{.*$$//' bibclean.ltx | $(DETEX) | $(DW)

maintainer-clean: distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."
	$(RM) bibclean.pdf
	$(RM) bibclean.ps

spell: bibclean.ser
