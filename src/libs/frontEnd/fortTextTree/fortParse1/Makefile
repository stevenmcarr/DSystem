# $Id: Makefile,v 1.1 1997/03/11 14:29:42 carr Exp $

HERE = libs/frontEnd/fortTextTree/fortParse1

SRCS = \
	   FortParse1.ansi.c \
	   gram1.ansi.c \
	   lex1.ansi.c

DERIVED	= gram1.ansi.c gram1.h

ARCHIVE	= ../../FrontEnd-$(DSYSTEM_OS)-g.a

include Makeinclude

# special rules for yacc generated files
$(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.h $(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.ansi.c: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.c > $*.ansi.c; $(RM) y.tab.c
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.h > $*.h; $(RM) y.tab.h

%.h %.ansi.c: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.c > $*.ansi.c; $(RM) y.tab.c
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.h > $*.h; $(RM) y.tab.h
