# $Id: Makefile,v 1.1 1997/03/11 14:34:27 carr Exp $

HERE = libs/ipAnalysis

DIRS = callGraph interface ipInfo problems

.SILENT:

dirs: 
	for dir in $(DIRS); do \
		echo "Making directory " $$dir; \
		make -C $$dir; \
	done

