# $Id: Makefile,v 1.3 1997/03/27 21:04:46 carr Exp $

HERE = libs/fortD/performance/staticInfo

DIRS	= 

PABLO_INC = $(DSYSTEM)/lib/pablo/include

CPPFLAGS := $(CPPFLAGS) -I$(PABLO_INC)

SRCS = \
	ArrayNameInfo.C \
	MesgStaticSDDF.C \
	SDDF_SetupCleanup.C \
	SDDF_Instrumentation.C \
	SDDF_General.C \
	SDDF_IO.C\
	StaticSDDF.C \
	SD_Globals.C \
	SD_Base.C \
	SD_List.C \
	SD_DataInfo.C \
	SD_MsgInfo.C \
	SD_SrcInfo.C \
	SD_Map.C \
	VPDlist.C \
	MkAssert.C \
	utility.C



ARCHIVE	= FortD-$(DSYSTEM_OS)-g.a

include Makeinclude

