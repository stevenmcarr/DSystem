# $Id: Makefile,v 1.1 1997/03/11 14:33:00 carr Exp $

HERE = libs/graphicInterface/oldMonitor/dialogs

DIRS = filer

# opt_dialog.c needs rewrite to C++ -- JMC
#SRCS	= opt_dialog.c -- 

ARCHIVE = dialogs-$(DSYSTEM_OS)-g.a


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
