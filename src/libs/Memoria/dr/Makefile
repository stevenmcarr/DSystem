# $Id: Makefile,v 1.9 1994/01/18 14:23:13 carr Exp $
ENV_FLAG := -g
HERE = libs/mc_lib/dr
INCLUDEDIRS	= libs/mc_lib/include
ARCHIVE		= ../mc-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= mh_config.c mh_walk.C
DERIVED		= mh_config.c

include Makeinclude

# special rule for lex generated file
%.c: %.l
	$(LEX.l) $< | sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' > $@
