# $Id: Makefile,v 1.1 1997/03/11 14:35:41 carr Exp $

HERE = libs/moduleAnalysis/dependence

DIRS = \
	   controlDependence \
	   dependenceGraph \
	   dependenceTest \
	   edgeList \
	   interface \
	   loopInfo \
	   utilities

ARCHIVE	= ModuleAnalysis-$(DSYSTEM_OS)-g.a

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