# $Id: Makefile,v 1.4 1997/06/25 15:14:22 carr Exp $

HERE = libs/support/file

ifeq "$(DSYSTEM_OS)" "OSF1"
SRCS = \
	   Cookie.C \
	   File.C \
	   FileName.C \
	   FileUtilities.C \
	   FormattedFile.C \
	   UnixFile.C
else
SRCS = \
	   Cookie.C \
	   File.C \
	   FileName.C \
	   FileUtilities.C \
	   FormattedFile.C \
	   ScanDir.C \
	   UnixFile.C

endif

ARCHIVE = ../Support-$(DSYSTEM_OS)-g.a

include Makeinclude
