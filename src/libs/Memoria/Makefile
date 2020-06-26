# $Id: Makefile,v 1.10 1997/03/27 20:21:21 carr Exp $
HERE = libs/Memoria
DIRS		= annotate dr li sp sr uj ut
ARCHIVE		= Memoria-$(DSYSTEM_OS)-g.a

.SILENT:

dirs: 
	for dir in $(DIRS); do \
	  echo "Making directory " $$dir; \
	  make -C $$dir; \
	done

