# $Id: Makefile,v 1.1 1997/03/11 14:30:05 carr Exp $

HERE = libs/graphicInterface

DIRS = cmdProcs framework include oldMonitor support

.SILENT:

dirs: 
	for dir in $(DIRS); do \
	  echo "Making directory " $$dir; \
	  make -C $$dir; \
	done

