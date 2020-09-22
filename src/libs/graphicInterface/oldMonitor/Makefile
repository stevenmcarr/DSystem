# $Id: Makefile,v 1.1 1997/03/11 14:32:59 carr Exp $

HERE = libs/graphicInterface/oldMonitor

DIRS = \
	   dialogs \
	   monitor

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