# $Id: Makefile,v 1.2 2000/01/11 21:01:08 carr Exp $

HERE = execs/checker

SRCS = \
	   Checker.C \
	   CheckerOptions.C

ARCHIVE = Checker-$(DSYSTEM_OS)-g.a

EXEC = checker-$(DSYSTEM_OS)-g

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
