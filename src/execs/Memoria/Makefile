# $Id: Makefile,v 1.4 2000/01/11 21:00:02 carr Exp $
HERE = execs/Memoria
SRCS	        = Memoria.C MemoriaOptions.C
ARCHIVE         = Memoria-$(DSYSTEM_OS)-g.a
EXEC	        = Memoria-$(DSYSTEM_OS)-g
DSYSTEM_LIBS    = $(NLIBS) 

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

