# $Id: Makefile,v 1.2 1997/06/25 14:53:48 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor/mon

SPECIAL_INCLUDE_DIRS = $(X11_INCLUDE_DIR) 

VERSIONS = \
	       monN \
	       monX

SRCS = \
	   Text.C \
	   event.ansi.c \
	   font.C \
	   gfx.C \
	   mach_none.C \
	   mach_x.ansi.c \
	   scr_mod.C 

DIRS = \
	   root

LOCAL_LINT = $(addprefix lint_,$(VERSIONS))

# X windows version
SRCS_monX = \
	        event.ansi.c \
	        font.C \
	        gfx.C \
	        mach_x.ansi.c \
	        scr_mod.C

ARCHIVE_monX = ../monX-$(DSYSTEM_OS)-g.a


# command line version
SRCS_monN = \
	        Text.C \
	        event.ansi.c \
	        gfx.C \
	        mach_none.C

ARCHIVE_monN = ../monN-$(DSYSTEM_OS)-g.a


ifeq "$(DSYSTEM_OS)" "SunOS-5.x"

# Handle compilation for event.C specifically to avoid 
# having to convert it to SVR4, since it will be going away soon.

# build event.ansi.o from event.C
# specify this as a pattern rule so that implicit chaining doesn't
# keep trying to build the event.ansi.o intermediate
event.ansi.o: event.ansi.c
	$(CC) -Xa -DSOLARIS $(CFLAGS) -I/usr/ucbinclude $(CPPFLAGS) $(TARGET_ARCH) -c $< 

endif

include Makeinclude

# Handle lint specially
lint_monX: $(filter %.C,$(SRCS_monX))
	$(LINT.c) $^ -lc

lint_monN: $(filter %.C,$(SRCS_monN))
	$(LINT.c) $^ -lc

.PHONY: lint_monX lint_monN
