# $Id: Makefile,v 1.10 2000/01/27 19:20:47 carr Exp $
HERE = libs/Memoria/annotate
PREFIX = a2i_
FLEX_FLAGS = --bison-bridge -P a2i_
BISON_FLAGS = -p a2i_
ARCHIVE		= ../Memoria-$(DSYSTEM_OS)-g.a
SRCS		= annotate.C CacheAnalysis.C ldst.C lexDirectives.c \
		  yaccDirectives.c DepGraphStat.C AddressEquivalenceClassSet.C \
		  AddressOptimization.C  f2iAnalysis.C

include ../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
