# $Id: Makefile,v 1.4 1995/08/31 12:47:11 carr Exp $
HERE = libs/mc_lib/annotate
INCLUDEDIRS	= libs/mc_lib/include 
ARCHIVE		= ../mc-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= annotate.C CacheAnalysis.C lexDirectives.c yaccDirectives.c

%.h %.c: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' < y.tab.c > $*.c; $(RM) y.tab.c
	sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' < y.tab.h > $*.h; $(RM) y.tab.h

%.c: %.l
	$(LEX.l) $< | sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' -e 's/getc(a2i_in)/*(a2i_DirectiveString++)/g' -e 's/EOF/EOS/g' > $@

include Makeinclude
