# $Id: Makefile,v 1.1 1997/03/11 14:33:54 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor/mon/root/system_mgr

VERSIONS = monX

SRCS = system_mgr.ansi.c

LOCAL_LINT=lint_mgr lint_other

# X windows
ARCHIVE_monX = ../../../monX-$(DSYSTEM_OS)-g.a

include Makeinclude

# Handle lint specially
lint_mgr: system_mgr.c
	@$(LINT.c) $^ ../llib-lfor_mgrs.ln -lc

.PHONY: lint_mgr
