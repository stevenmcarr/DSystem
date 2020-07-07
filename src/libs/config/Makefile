# $Id: Makefile,v 1.7 2000/01/27 20:21:09 carr Exp $

HERE = libs/config

SRCS = \
	   arch.C \
	   config.C

CPPFLAGS := $(CPPFLAGS) -DDSYSTEM_ARCH_LINUX_I686

ARCHIVE	= Config-$(DSYSTEM_OS)-g.a

include ../../make.config

include $(DEPENDS)
