# $Id: Makefile,v 1.1 1997/03/11 14:33:52 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor/mon/root/dialog_mgr

VERSIONS = monX

SRCS = group_item.ansi.c expand_item.ansi.c dialog_mgr.ansi.c

LOCAL_LINT = lint_mgr lint_other

# X windows
ARCHIVE_monX = ../../../monX-$(DSYSTEM_OS)-g.a

include Makeinclude

# Handle lint specially
lint_mgr: dialog_mgr.ansi.c
	@$(LINT.c) $^ ../llib-lfor_mgrs.ln -lc

lint_other: group_item.ansi.c expand_item.ansi.c
	@$(LINT.c) $^ -lc

.PHONY: lint_mgr lint_other
