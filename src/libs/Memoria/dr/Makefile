HERE		= ped_cp/memory/dr
INCLUDEDIRS	= ped_cp/memory/include ped_cp/ped_include ned_cp
ARCHIVE		= ../ped-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= memory_menu.C mh_config.c mh_walk.C

include Makeinclude

mh_config.c: mh_config.l
	lex mh_config.l
	sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' lex.yy.c > mh_config.c
	rm -f lex.yy.c

