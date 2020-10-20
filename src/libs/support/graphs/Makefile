# $Id: Makefile,v 1.1 1997/03/11 14:36:40 carr Exp $

HERE = libs/support/graphs

DIRS = \
	   directedGraph 

RM = /bin/rm -r -f
ARCHIVE         = Support-$(DSYSTEM_OS)-g.a

.SILENT:

all: dirs $(ARCHIVE)

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
