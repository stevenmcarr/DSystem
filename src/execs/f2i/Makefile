# $Id: Makefile,v 1.5 2000/01/11 21:01:34 carr Exp $

HERE = execs/f2i

SRCS = f2i.C f2i_options.C 

ARCHIVE	= F2i-$(DSYSTEM_OS)-g.a

EXEC	= f2i-$(DSYSTEM_OS)-g

DSYSTEM_LIBS = $(NLIBS)

ifeq "$(DSYSTEM_OS)" "Linux-i686"
  LOCAL_LIBS      = -lm -lfl
else
  ifeq "$(DSYSTEM_OS)" "Linux-alpha"
    LOCAL_LIBS      = -lm -lfl
  else
    LOCAL_LIBS      = -lm -ll
  endif
endif

include Makeinclude
