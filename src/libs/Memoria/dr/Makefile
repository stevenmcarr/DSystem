# $Id: Makefile,v 1.11 1995/08/18 10:09:04 trsuchyt Exp $
HERE = libs/mc_lib/dr
INCLUDEDIRS	= libs/mc_lib/include libs/ped_cp/ped_include libs/ped_cp/pt
ARCHIVE		= ../mc-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= memory_menu.C mh_config.c mh_walk.C
DERIVED		= mh_config.c

include Makeinclude

# special rule for lex generated file
%.c: %.l
	$(LEX.l) $< | sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' > $@
