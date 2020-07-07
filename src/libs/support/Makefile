# $Id: Makefile,v 1.2 1997/03/27 20:48:39 carr Exp $

HERE = libs/support

DIRS = \
	   Lambda \
	   annotation \
	   arrays \
	   database \
	   file \
	   graphs \
	   iterators \
	   lists \
	   memMgmt \
	   misc \
	   msgHandlers \
	   numerical \
	   optParsing \
	   patternMatch \
	   queues \
	   registry \
	   sets \
	   sorts \
	   stacks \
	   strings \
	   tables \
	   time \
	   trees \
	   vectors

ARCHIVE = Support-$(DSYSTEM_OS)-g.a

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