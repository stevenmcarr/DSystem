# $Id: Makefile,v 1.1 1997/03/11 14:33:38 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor/mon

SPECIAL_INCLUDE_DIRS = $(RN_X11_INCLUDEDIR)

VERSIONS = \
	       monN \
	       monX

SRCS = \
	   Text.c \
	   event.ansi.c \
	   font.c \
	   gfx.ansi.c \
	   mach_none.ansi.c \
	   mach_x.ansi.c \
	   scr_mod.ansi.c 

DIRS = \
	   root

LOCAL_LINT = $(addprefix lint_,$(VERSIONS))

# X windows version
SRCS_monX = \
	        Text.c \
	        event.ansi.c \
	        font.c \
	        gfx.ansi.c \
	        mach_x.ansi.c \
	        scr_mod.ansi.c

ARCHIVE_monX = ../monX-$(DSYSTEM_OS)-g.a


# command line version
SRCS_monN = \
	        Text.c \
	        event.ansi.c \
	        font.c \
	        gfx.ansi.c \
	        mach_none.ansi.c

ARCHIVE_monN = ../monN-$(DSYSTEM_OS)-g.a


ifeq "$(DSYSTEM_OS)" "SunOS-5.x"

# Handle compilation for event.ansi.c specifically to avoid 
# having to convert it to SVR4, since it will be going away soon.

# build event.ansi.o from event.ansi.c
# specify this as a pattern rule so that implicit chaining doesn't
# keep trying to build the event.ansi.o intermediate
%vent.ansi.o: %vent.ansi.c
	$(CC) -DSOLARIS $(CFLAGS) -I/usr/ucbinclude $(CPPFLAGS) $(TARGET_ARCH) -c $< $(OUTPUT_OPTION)

endif

include Makeinclude

# Handle lint specially
lint_monX: $(filter %.ansi.c,$(SRCS_monX))
	$(LINT.c) $^ -lc

lint_monN: $(filter %.ansi.c,$(SRCS_monN))
	$(LINT.c) $^ -lc

.PHONY: lint_monX lint_monN
