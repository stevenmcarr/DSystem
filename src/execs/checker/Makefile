# $Id: Makefile,v 1.2 2000/01/11 21:01:08 carr Exp $

HERE = execs/checker

DSYSTEM_OS = Linux-i686
SRCS = \
	   Checker.C \
	   CheckerOptions.C

ARCHIVE = Checker-$(DSYSTEM_OS)-g.a

EXEC = checker-$(DSYSTEM_OS)-g

DSYSTEM_LIBS = $(NLIBS) 

LOCAL_LIBS      = -lm -lfl

exec: $(ARCHIVE) 
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(ARCHIVE) $(NLIBS) $(LOCAL_LIBS)

include ../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
