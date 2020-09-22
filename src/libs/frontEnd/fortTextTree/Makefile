# $Id: Makefile,v 1.2 1997/06/24 17:51:10 carr Exp $

HERE = libs/frontEnd/fortTextTree

DIRS = fortParse1 fortParse2
DSYSTEM_OS = Linux-i686
SRCS = \
	   FortTextTree.C \
	   FortUnparse1.C \
	   FortUnparse2.C \
	   MapInfo.C \
	   MapInfo_c.C

ARCHIVE	= ../FrontEnd-$(DSYSTEM_OS)-g.a


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

include ../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.