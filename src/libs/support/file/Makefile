# $Id: Makefile,v 1.2 1997/03/27 20:49:09 carr Exp $

HERE = libs/support/file

ifeq "$(DSYSTEM_OS)" "OSF1"
	   #File.C 
SRCS = \
	   Cookie.C \
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
