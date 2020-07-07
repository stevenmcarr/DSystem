# $Id: Makefile,v 1.1 1997/03/11 14:35:24 carr Exp $

HERE = libs/moduleAnalysis

DIRS = \
	   cfg \
	   cfgValNum \
	   dependence \
	   expnSimplifier \
	   ssa \
	   valNum

.SILENT:

dirs: 
	for dir in $(DIRS); do \
	  echo "Making directory" $$dir; \
	  make -C $$dir; \
	done

clean: 
	for dir in $(DIRS); do \
	  echo "Making directory" $$dir; \
	  make -C $$dir clean; \
	done
	$(RM) $(ARCHIVE)