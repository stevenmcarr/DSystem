# $Id: Makefile,v 1.2 1997/06/25 14:50:40 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor/mon/root/dialog_mgr

VERSIONS = monX

SRCS = group_item.C expand_item.C dialog_mgr.C

LOCAL_LINT = lint_mgr lint_other

# X windows
ARCHIVE_monX = ../../../monX-$(DSYSTEM_OS)-g.a

include Makeinclude

# Handle lint specially
lint_mgr: dialog_mgr.C
	@$(LINT.c) $^ ../llib-lfor_mgrs.ln -lc

lint_other: group_item.C expand_item.C
	@$(LINT.c) $^ -lc

.PHONY: lint_mgr lint_other
