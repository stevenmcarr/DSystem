# $Id: Makefile,v 1.1 1997/03/11 14:29:11 carr Exp $

HERE = libs/fortD/performance/stats

PABLO_INC = $(DSYSTEM)/lib/pablo/include
CPPFLAGS := $(CPPFLAGS) -I$(PABLO_INC)

SRCS	 = \
	   ProcessTrace.C \
	   CallSequence.C \
	   StaticIDTable.C \
	   MakeStaticIDTable.C \
	   Metrics.C \
	   FortDPerfTable.C \
	   PerfInfo.C

ARCHIVE	= FortD-$(DSYSTEM_OS)-g.a

include Makeinclude
