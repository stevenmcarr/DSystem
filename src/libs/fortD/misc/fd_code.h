/* $Id: fd_code.h,v 1.6 1997/03/11 14:28:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef fd_code_h
#define fd_code_h

#ifndef cNameValueTable_h
#include <libs/support/tables/cNameValueTable.h>
#endif

#ifndef fortranD_types_h
#include <libs/fortD/misc/fd_types.h>
#endif

EXTERN(void, fortd_compiler, (cNameValueTable analyses, Fd_opts *fd_opts));

#endif
