# $Id: Makefile,v 1.7 2000/01/27 20:21:09 carr Exp $

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

ifeq "$(DSYSTEM_OS)" "Linux-i686"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH_LINUX_I686
endif # "$(DSYSTEM_OS)" "Linux-i686"

ifeq "$(DSYSTEM_OS)" "Linux-alpha"
CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH_LINUX_ALPHA
endif # "$(DSYSTEM_OS)" "Linux-alpha"

ARCHIVE	= Config-$(DSYSTEM_OS)-g.a

include Makeinclude
