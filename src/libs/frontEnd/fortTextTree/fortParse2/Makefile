# $Id: Makefile,v 1.1 1997/03/11 14:29:45 carr Exp $

HERE = libs/frontEnd/fortTextTree/fortParse2

SRCS = \
	   FortParse2.c \
	   gram2.ansi.c \
	   lex2.ansi.c

DERIVED	= gram2.ansi.c gram2.h

ARCHIVE	= ../../FrontEnd-$(DSYSTEM_OS)-g.a

include Makeinclude

# special rules for yacc generated files
$(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.h $(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.ansi.c: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.c > $*.ansi.c; $(RM) y.tab.c
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.h > $*.h; $(RM) y.tab.h

%.h %.ansi.c: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.c > $*.ansi.c; $(RM) y.tab.c
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.h > $*.h; $(RM) y.tab.h
