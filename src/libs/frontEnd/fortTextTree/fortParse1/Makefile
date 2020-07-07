# $Id: Makefile,v 1.2 1997/06/24 17:45:59 carr Exp $

HERE = libs/frontEnd/fortTextTree/fortParse1

SRCS = \
	   FortParse1.C \
	   gram1.C \
	   lex1.C

YACC_SRCS = gram1.y

ARCHIVE	= ../../FrontEnd-$(DSYSTEM_OS)-g.a

%.h %.C: %.y
	$(YACC) -d $<
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < gram1.tab.c > $*.C; $(RM) gram1.tab.c
	sed -e 's/yy/yy1/g' -e 's/YY/YY1/g' < gram1.tab.h > $*.h; $(RM) gram1.tab.h

include ../../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.