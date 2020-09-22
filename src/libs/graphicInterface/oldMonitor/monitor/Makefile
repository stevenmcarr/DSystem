# $Id: Makefile,v 1.1 1997/03/11 14:33:26 carr Exp $

HERE = libs/graphicInterface/oldMonitor/monitor

# command line version
DIRS = \
			keyboard \
	        mon

ARCHIVE = monN-$(DSYSTEM_OS)-g.a


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
