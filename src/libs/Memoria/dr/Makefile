HERE		= ped_cp/memory/dr
INCLUDEDIRS	= ped_cp/memory/include ped_cp/ped_include ned_cp
ARCHIVE		= ../ped-$(RN_ARCH)$(ENV_FLAG).a
SRCS		= memory_menu.C mh_config.l mh_walk.C

$(ARCHIVE)(mh_config.o): mh_config.l mh_config.sed.cmd ../../ped_include/mh_config.h
	lex mh_config.l
	sed -f mh_config.sed.cmd lex.yy.c > mh_config.c
	rm -f lex.yy.c
	$(CC) $(CFLAGS) -I. -I/rn/src/include -I../include -I../../ped_include -c mh_config.c
	rm -f mh_config.c
	$(AR) $(ARFLAGS) $@ mh_config.o
	rm mh_config.o

include Makeinclude
