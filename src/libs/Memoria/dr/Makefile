# $Id: Makefile,v 1.13 1997/10/30 15:10:35 carr Exp $
HERE = libs/Memoria/dr
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= MemoriaConfig.c mh_walk.C
DERIVED		= MemoriaConfig.c

include Makeinclude

# special rule for lex generated file
%.c: %.l
	$(LEX.l) $< | sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' > $@
