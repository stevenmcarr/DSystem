# $Id: Makefile,v 1.7 1997/06/30 18:43:01 carr Exp $
HERE = libs/Memoria/annotate
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= annotate.C CacheAnalysis.C ldst.C lexDirectives.c yaccDirectives.c \
		  DepGraphStat.C

%.h %.c: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' < y.tab.c > $*.c; $(RM) y.tab.c
	sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' < y.tab.h > $*.h; $(RM) y.tab.h

%.c: %.l
	$(LEX.l) $< | sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' -e 's/getc(a2i_in)/*(a2i_DirectiveString++)/g' -e 's/EOF/EOS/g' > $@

include Makeinclude
