# $Id: Makefile,v 1.15 1999/09/10 14:42:44 carr Exp $
HERE = libs/Memoria/dr
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= MemoriaConfig.c mh_walk.C
DERIVED		= $(DSYSTEM_PVT)/obj/$(DSYSTEM_OS)/$(HERE)/MemoriaConfig.c

include Makeinclude

# special rule for lex generated file
$(DSYSTEM_PVT)/obj/$(DSYSTEM_OS)/$(HERE)/MemoriaConfig.c: MemoriaConfig.l
	$(FLEX) -PMemoria -o$@ $< 
