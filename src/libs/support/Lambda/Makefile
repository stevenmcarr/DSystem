
HERE = libs/support/Lambda
ARCHIVE	= ../Support-$(DSYSTEM_OS)-g.a

SYSNAME=`uname|sed -e 's/-/_/'`

CFLAGS := $(CFLAGS) -DLA_$(SYSNAME) -DLA_$(RN_ARCH)

SRCS	= la-bounds.C la-code.C la-dep1.C la-dep2.C la-hermite.C la-integers.C\
	la-lattice.C la-matrix.C la-print1.C la-print2.C la-trans.C la-vec.C \
	la-vecMat.C

include ../../../make.config

include $(DEPENDS)

# DO NOT DELETE THIS LINE -- make depend depends on it.