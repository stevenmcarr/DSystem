# $Id: Makefile,v 1.14 1999/06/11 17:49:05 carr Exp $
HERE = libs/Memoria/dr
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= MemoriaConfig.c mh_walk.C
DERIVED		= MemoriaConfig.c

include Makeinclude

# special rule for lex generated file
MemoricaConfig.c: MemoriaConfig.l
	$(LEX.l) $< | sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' > $@
