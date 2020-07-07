# $Id: Makefile,v 1.1 1997/03/11 14:28:49 carr Exp $

HERE = libs/fortD/performance

DIRS	= instr staticInfo stats

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