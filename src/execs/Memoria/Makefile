# $Id: Makefile,v 1.1 1997/03/26 18:18:32 carr Exp $
HERE = execs/mc_ex
INCLUDEDIRS = libs/pcomp_lib libs/mc_lib/include libs/mc_lib/dr
SRCS	   = mc.C mc_options.C
ARCHIVE    = mc-$(RN_ARCH)$(ENV_FLAG).a
EXEC	   = mc-$(RN_ARCH)$(ENV_FLAG)
RN_LIBS    = $(NLIBS) $(CONFIG) $(DMALLOC)
LOCAL_LIBS = -lm $(X11_LIB) -ll

include Makeinclude

