# $Id: Makefile,v 1.2 1997/06/24 17:47:33 carr Exp $

HERE = libs/frontEnd/fortTextTree/fortParse2

YACC_SRCS = gram2.y

SRCS = \
	   FortParse2.C \
	   gram2.C \
	   lex2.C

ARCHIVE	= ../../FrontEnd-$(DSYSTEM_OS)-g.a

%.h %.C: %.y
	$(BISON) -d $<
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < gram2.tab.c > $*.C; $(RM) gram2.tab.c
	sed -e 's/yy/yy2/g' -e 's/YY/YY2/g' < gram2.tab.h > $*.h; $(RM) gram2.tab.h

include ../../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.