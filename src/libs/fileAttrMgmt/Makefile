# $Id: Makefile,v 1.1 1997/03/11 14:27:41 carr Exp $

HERE = libs/fileAttrMgmt

DIRS = \
	   composition \
	   attributedFile \
	   fortranModule \
	   module

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

