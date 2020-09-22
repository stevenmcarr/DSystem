# $Id: Makefile,v 1.1 1997/03/11 14:30:59 carr Exp $

HERE = libs/graphicInterface/cmdProcs/paraScopeEditor

DIRS = \
	   pt \
	   dp \
	   dg \
	   el \
	   dt \
	   perf \
	   stubs
#	   pedCP \

#	   memory \

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
