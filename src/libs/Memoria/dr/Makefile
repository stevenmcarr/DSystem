HERE		= ped_cp/memory/dr
INCLUDEDIRS	= ped_cp/memory/include ped_cp/ped_include
ARCHIVE		= ../ped-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= memory_menu.C mh_config.c mh_walk.C
DERIVED		= mh_config.c

include Makeinclude

# special rule for lex generated file
%.c: %.l
	$(LEX.l) $< | sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' > $@
