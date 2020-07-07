# $Id: Makefile,v 1.1 1997/03/11 14:29:15 carr Exp $

HERE = libs/frontEnd

DIRS = \
	   ast \
	   fortTextTree \
	   fortTree \
	   prettyPrinter \
	   textTree

ARCHIVE = FrontEnd-$(DSYSTEM_OS)-g.a

.SILENT:

dirs: 
	for dir in $(DIRS); do \
	  echo "Making directory" $$dir; \
	  make -C $$dir; \
	done

clean: 
	for dir in $(DIRS); do \
	  echo "Making directory" $$dir; \
	  make -C $$dir clean; \
	done
	$(RM) $(ARCHIVE)
