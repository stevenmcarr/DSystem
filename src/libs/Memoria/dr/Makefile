# $Id: Makefile,v 1.12 1997/03/27 20:23:39 carr Exp $
HERE = libs/Memoria/dr
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= mh_config.c mh_walk.C
DERIVED		= mh_config.c

include Makeinclude

# special rule for lex generated file
%.c: %.l
	$(LEX.l) $< | sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' > $@
