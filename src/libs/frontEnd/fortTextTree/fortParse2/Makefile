# $Id: Makefile,v 1.2 1997/06/24 17:47:33 carr Exp $

HERE = libs/frontEnd/fortTextTree/fortParse2

SRCS = \
	   FortParse2.C \
	   gram2.C \
	   lex2.C

DERIVED	= gram2.C gram2.h

ARCHIVE	= ../../FrontEnd-$(DSYSTEM_OS)-g.a

include Makeinclude

# special rules for yacc generated files
$(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.h $(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.C: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.c > $*.C; $(RM) y.tab.c
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.h > $*.h; $(RM) y.tab.h

%.h %.C: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.c > $*.C; $(RM) y.tab.c
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < y.tab.h > $*.h; $(RM) y.tab.h
