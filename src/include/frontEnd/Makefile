# $Id: Makefile,v 1.1 1997/03/11 14:27:33 carr Exp $

#HERE = include/frontEnd

DERIVED	= astcons.h astsel.h astrec.h astnode.h nodeinfo.h astmeta.h

#include Makeinclude

# Tricky implicit rules for making .h files from their awk files.
#$(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.h: %.awk
	#grep -v \\\*\\\* $(word 2,$^) | $(AWK) -f $< > $(notdir $@)

AWK = /usr/bin/awk

target: $(DERIVED)

%.h: %.awk
	grep -v \\\*\\\* $(word 2,$^) | $(AWK) -f $< > $(notdir $@)

# These dependences factor into the previous rule!
astcons.h astsel.h astrec.h astnode.h nodeinfo.h: nodedef.def
astmeta.h: metamap.def
