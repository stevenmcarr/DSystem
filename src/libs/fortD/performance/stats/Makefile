# $Id: Makefile,v 1.3 1997/03/27 21:05:23 carr Exp $

HERE = libs/fortD/performance/stats

PABLO_INC = $(DSYSTEM)/lib/pablo/include
CPPFLAGS := $(CPPFLAGS) -I$(PABLO_INC)

SRCS	 = \
	   ProcessTrace.C \
	   CallSequence.C \
	   MakeStaticIDTable.C \
	   FortDPerfTable.C \
	   StaticIDTable.C \
	   Metrics.C \
	   PerfInfo.C

ARCHIVE	= ../../FortD-$(DSYSTEM_OS)-g.a

include ../../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.