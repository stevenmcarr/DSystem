# $Id: Makefile,v 1.6 1997/03/27 20:22:30 carr Exp $
HERE = libs/Memoria/annotate
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= annotate.C CacheAnalysis.C ldst.C lexDirectives.c yaccDirectives.c

%.h %.c: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' < y.tab.c > $*.c; $(RM) y.tab.c
	sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' < y.tab.h > $*.h; $(RM) y.tab.h

%.c: %.l
	$(LEX.l) $< | sed -e 's/yy/a2i_/g' -e 's/YY/A2I_/g' -e 's/getc(a2i_in)/*(a2i_DirectiveString++)/g' -e 's/EOF/EOS/g' > $@

include Makeinclude
