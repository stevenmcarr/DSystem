# $Id: Makefile,v 1.2 1997/06/24 17:51:10 carr Exp $

HERE = libs/frontEnd/fortTextTree

DIRS = fortParse1 fortParse2

SRCS = \
	   FortTextTree.C \
	   FortUnparse1.C \
	   FortUnparse2.C \
	   MapInfo.C \
	   MapInfo_c.C

ARCHIVE	= ../FrontEnd-$(DSYSTEM_OS)-g.a

include Makeinclude

# make the derived source and include files from the subdirectories (early)
derived::
	-@$(SUB_SANITIZE);                                       \
	for dir in $(DIRS); do                                   \
	    $(MAKE) -C $$dir $(LOCAL_SUBMAKE_FLAGS) derived;     \
	done
