
HERE = libs/support/Lambda
ARCHIVE	= ../Support-$(DSYSTEM_OS)-g.a

SYSNAME=`uname|sed -e 's/-/_/'`

CFLAGS := $(CFLAGS) -DLA_$(SYSNAME) -DLA_$(RN_ARCH)

SRCS	= la-bounds.c la-code.c la-dep1.c la-dep2.c la-hermite.c la-integers.c\
	la-lattice.c la-matrix.c la-print1.c la-print2.c la-trans.c la-vec.c \
	la-vecMat.c

include Makeinclude
