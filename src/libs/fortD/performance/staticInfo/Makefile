# $Id: Makefile,v 1.1 1997/03/11 14:28:54 carr Exp $

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
	SDDF_IO.C \
	StaticSDDF.C \
	SD_Globals.C \
	SD_Base.C \
	SD_List.C \
	SD_DataInfo.C \
	SD_MsgInfo.C \
	SD_SrcInfo.C \
	VPDlist.C \
	MkAssert.C \
	SD_Map.C \
	utility.C



ARCHIVE	= FortD-$(DSYSTEM_OS)-g.a

include Makeinclude

