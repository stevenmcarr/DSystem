HERE		= ped_cp/memory/dr
INCLUDEDIRS	= ped_cp/memory/include ped_cp/ped_include ned_cp
ARCHIVE		= ../ped-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= memory_menu.C mh_config.l mh_walk.C

# change RN_SRC to MY_SRC when compiling your own version

$(ARCHIVE)(mh_config.o): $(RN_SRC)/ped_cp/memory/dr/mh_config.l $(RN_SRC)/ped_cp/ped_include/mh_config.h
	lex $(RN_SRC)/ped_cp/memory/dr/mh_config.l
	sed -e 's/yylex()/static int mh_yylex(config) config_type *config;/' lex.yy.c > mh_config.c
	rm -f lex.yy.c
	$(CC) $(CFLAGS) -I. -I$(RN_SRC)/include -I$(RN_SRC)/ped_cp/memory/include -I$(RN_SRC)/ped_cp/ped_include -c mh_config.c
	rm -f mh_config.c
	$(AR) $(ARFLAGS) $@ mh_config.o
	rm mh_config.o

include Makeinclude
