# $Id: Makefile,v 1.2 1997/06/24 17:45:59 carr Exp $

HERE = libs/frontEnd/fortTextTree/fortParse1

SRCS = \
	   FortParse1.C \
	   gram1.C \
	   lex1.C

DERIVED	= gram1.C gram1.h

ARCHIVE	= ../../FrontEnd-$(DSYSTEM_OS)-g.a

include Makeinclude

# special rules for yacc generated files
$(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.h $(RN_OBJ)/$(DSYSTEM_OS)/$(HERE)/%.C: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.c > $*.C; $(RM) y.tab.c
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.h > $*.h; $(RM) y.tab.h

%.h %.C: %.y
	$(YACC.y) -d $<
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.c > $*.C; $(RM) y.tab.c
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < y.tab.h > $*.h; $(RM) y.tab.h
