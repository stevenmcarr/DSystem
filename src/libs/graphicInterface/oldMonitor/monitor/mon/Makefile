# $Id: Makefile,v 1.2 1997/06/25 14:53:48 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor/mon
DSYSTEM_OS = Linux-i686
DIRS = \
	   root

# command line version
SRCS = \
	        Text.C \
	        event.ansi.c \
	        gfx.C \
	        mach_none.C

ARCHIVE = ../monN-$(DSYSTEM_OS)-g.a

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

include ../../../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.