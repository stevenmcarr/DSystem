# $Id: Makefile,v 1.4 1999/07/22 18:10:17 carr Exp $

HERE = execs/f2i

SRCS = f2i.C f2i_options.C 

ARCHIVE	= F2i-$(DSYSTEM_OS)-g.a

EXEC	= f2i-$(DSYSTEM_OS)-g

DSYSTEM_LIBS = $(NLIBS)

ifeq ($(DSYSTEM_OS),Linux)
  LOCAL_LIBS = -lm  -fl
else
  LOCAL_LIBS = -lm  -ll
endif

include Makeinclude
