# $Id: Makefile,v 1.3 1997/03/27 21:07:40 carr Exp $

HERE = libs/support/file

ifeq "$(DSYSTEM_OS)" "OSF1"
SRCS = \
	   Cookie.C \
	   File.C \
	   FileName.ansi.c \
	   FileUtilities.C \
	   FormattedFile.C \
	   UnixFile.ansi.c
else
SRCS = \
	   Cookie.C \
	   File.C \
	   FileName.ansi.c \
	   FileUtilities.C \
	   FormattedFile.C \
	   ScanDir.C \
	   UnixFile.ansi.c

endif

ARCHIVE = ../Support-$(DSYSTEM_OS)-g.a

include Makeinclude
