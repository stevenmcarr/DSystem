# $Id: Makefile,v 1.2 1997/03/27 20:19:38 carr Exp $

HERE = libs

DIRS = \
	   Memoria \
	   config \
	   f2i \
	   fileAttrMgmt \
	   fortD \
	   frontEnd \
	   graphicInterface \
	   ipAnalysis \
	   moduleAnalysis \
	   perfAnalysis \
	   support

.SILENT:

dirs: 
	for dir in $(DIRS); do \
	  echo "Making directory " $$dir; \
	  make -C $$dir; \
	done

