# $Id: Makefile,v 1.2 1997/03/27 20:34:26 carr Exp $

HERE = libs/fortD/performance/stats

PABLO_INC = $(DSYSTEM)/lib/pablo/include
CPPFLAGS := $(CPPFLAGS) -I$(PABLO_INC)

#	   ProcessTrace.C 
	   #CallSequence.C 
	   #MakeStaticIDTable.C 
	   #FortDPerfTable.C 
SRCS	 = \
	   StaticIDTable.C \
	   Metrics.C \
	   PerfInfo.C

ARCHIVE	= FortD-$(DSYSTEM_OS)-g.a

include Makeinclude
