# $Id: Makefile,v 1.1 1997/03/11 14:33:44 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor/mon/root

VERSIONS = \
	       monN \
	       monX

SRCS = \
	   cp_mgr.ansi.c \
	   menu_mgr.ansi.c \
	   notify_mgr.ansi.c \
	   root.ansi.c \
	   stand_alone.ansi.c

LOCAL_LINT = \
	         llib-lfor_mgrs.ln \
	         llib-ldesk_sm.ln \
	         lint_mgr \
	         lint_desk \
	         lint_dia \
	         lint_sa


# X windows version
SRCS_monX = \
	        cp_mgr.ansi.c \
	        menu_mgr.ansi.c \
	        notify_mgr.ansi.c \
	        root.ansi.c

DIRS_monX = \
	        desk_sm \
	        dialog_mgr \
	        system_mgr

ARCHIVE_monX = ../../monX-$(DSYSTEM_OS)-g.a

# command line version
SRCS_monN = \
	    notify_mgr.ansi.c \
	    stand_alone.ansi.c

DIRS_monN = 

ARCHIVE_monN = ../../monN-$(DSYSTEM_OS)-g.a

include Makeinclude

# Handle lint specially (non-version specific)
lint_mgr: menu_mgr.ansi.c cp_mgr.ansi.c notify_mgr.ansi.c
	$(LINT.c) $^ llib-lfor_mgrs.ln -lc

lint_desk: root.ansi.c
	$(LINT.c) $^ llib-ldesk_sm.ln  -lc

lint_sa: notify_mgr.ansi.c stand_alone.ansi.c
	@echo "Stand alone version..."
	$(LINT.c) $^ -lc

.PHONY: lint_mgr lint_desk lint_dia lint_sa
