# $Id: Makefile,v 1.2 1997/03/27 20:29:39 carr Exp $

HERE = libs/config

SRCS = \
	   arch.C \
	   config.C

ifeq "$(DSYSTEM_OS)" "OSF1"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH=\\\"$(DSYSTEM_OS)\\\"
endif # "$(DSYSTEM_OS)" "OSF1"

ifeq "$(DSYSTEM_OS)" "SunOS-4.x"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH=\\\"$(DSYSTEM_OS)\\\"
endif # "$(DSYSTEM_OS)" "SunOS-4.x"

ifeq "$(DSYSTEM_OS)" "SunOS-5.x"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH=\\\"$(DSYSTEM_OS)\\\"
endif # "$(DSYSTEM_OS)" "SunOS-5.x"

ifeq "$(DSYSTEM_OS)" "AIX-3.x"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH=\"$(DSYSTEM_OS)\"
endif # "$(DSYSTEM_OS)" "AIX-3.x"

ARCHIVE	= Config-$(DSYSTEM_OS)-g.a

include Makeinclude
