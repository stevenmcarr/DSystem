# $Id: Makefile,v 1.1 1997/03/11 14:34:22 carr Exp $

HERE = libs/graphicInterface/support

DIRS = \
	   graphics \
	   message

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
