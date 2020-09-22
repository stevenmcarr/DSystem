# $Id: Makefile,v 1.4 2000/01/11 21:00:02 carr Exp $
HERE = execs/Memoria
DSYSTEM_OS = Linux-i686
SRCS	        = Memoria.C MemoriaOptions.C
ARCHIVE         = Memoria-$(DSYSTEM_OS)-g.a
EXEC	        = Memoria-$(DSYSTEM_OS)-g
DSYSTEM_LIBS    = $(NLIBS) 

LOCAL_LIBS      = -lm -lfl

exec: $(ARCHIVE) 
	echo "Generating " $(EXEC)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(ARCHIVE) $(NLIBS) $(LOCAL_LIBS)

include ../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
