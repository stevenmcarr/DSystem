# $Id: Makefile,v 1.6 1997/04/04 20:00:11 carr Exp $

HERE = libs/config

SRCS = \
	   arch.C \
	   config.C

ifeq "$(DSYSTEM_OS)" "SunOS-4.x"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH_SUNOS_4x
endif # "$(DSYSTEM_OS)" "SunOS-4.x"

ifeq "$(DSYSTEM_OS)" "SunOS-5.x"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH_SUNOS_5x
endif # "$(DSYSTEM_OS)" "SunOS-5.x"

ifeq "$(DSYSTEM_OS)" "AIX-3.x"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH_AIX_3x
endif # "$(DSYSTEM_OS)" "AIX-3.x"

ifeq "$(DSYSTEM_OS)" "OSF1"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH_OSF1
endif # "$(DSYSTEM_OS)" "OSF1"

ARCHIVE	= Config-$(DSYSTEM_OS)-g.a

include Makeinclude
