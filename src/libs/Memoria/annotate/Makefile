# $Id: Makefile,v 1.9 1999/09/20 14:29:19 carr Exp $
HERE = libs/Memoria/annotate
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= annotate.C CacheAnalysis.C ldst.C lexDirectives.c \
		  yaccDirectives.c DepGraphStat.C AddressEquivalenceClassSet.C \
		  AddressOptimization.C  f2iAnalysis.C
DERIVED		= $(DSYSTEM_PVT)/obj/$(DSYSTEM_OS)/$(HERE)/lexDirectives.c $(DSYSTEM_PVT)/obj/$(DSYSTEM_OS)/$(HERE)/yaccDirectives.c

include Makeinclude

$(DSYSTEM_PVT)/obj/$(DSYSTEM_OS)/$(HERE)/yaccDirectives.c: yaccDirectives.y
	$(BISON) -o yaccDirectives.c -p a2i_ -d $<
	#$(MV) yaccDirectives.tab.h yaccDirectives.h

$(DSYSTEM_PVT)/obj/$(DSYSTEM_OS)/$(HERE)/lexDirectives.c: lexDirectives.l
	$(FLEX) -Pa2i_ -o$@ $< 
	sed -e 's/getc([ ]*yyin[ ]*)/*(a2i_DirectiveString++)/g' -e 's/EOF/EOS/g' $@ > tempflex.c
	$(MV) tempflex.c $@
