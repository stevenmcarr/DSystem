# $Id: Makefile,v 1.4 1997/06/25 15:20:35 carr Exp $

HERE = libs/support/tables

DSYSTEM_OS = Linux-i686

DIRS = \
	   namedObject \
	   wordObject

ARCHIVE = ../Support-$(DSYSTEM_OS)-g.a

all: dirs $(ARCHIVE)

dirs: 
	for dir in $(DIRS); do \
	  echo "Making directory" $$dir; \
	  make -C $$dir; \
	done

dirclean: 
	for dir in $(DIRS); do \
	  echo "Making directory" $$dir; \
	  make -C $$dir clean; \
	done
	$(RM) $(ARCHIVE)

SRCS = \
	   HashTable.C \
	   cNameValueTable.C \
	   IntegerHashTable.C \
	   NameValueTable.C \
	   PointerMap.C \
	   StringHashTable.C \
	   symtable.C

include ../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
