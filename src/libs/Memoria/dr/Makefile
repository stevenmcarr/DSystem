# $Id: Makefile,v 1.8 1992/11/20 13:50:21 joel Exp $
HERE = libs/ped_cp/memory/dr
INCLUDEDIRS	= libs/ped_cp/memory/include libs/ped_cp/ped_include
ARCHIVE		= ../ped-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= memory_menu.C mh_config.c mh_walk.C
DERIVED		= mh_config.c

include Makeinclude

# special rule for lex generated file
%.c: %.l
	$(LEX.l) $< | sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' > $@
