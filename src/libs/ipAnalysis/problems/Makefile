# $Id: Makefile,v 1.1 1997/03/11 14:34:52 carr Exp $

HERE = libs/ipAnalysis/problems

DIRS = \
	   alias \
	   fortD \
	   modRef \
	   rsd \

#  Currently under redesign so do not bother compiling these directories.
#	   perfEst \
#	   symbolic

ARCHIVE = IPAnalysis-$(DSYSTEM_OS)-g.a

.SILENT:

dirs: 
	for dir in $(DIRS); do \
		echo "Making directory " $$dir; \
		make -C $$dir; \
	done

