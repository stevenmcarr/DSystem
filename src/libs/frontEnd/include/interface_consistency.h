/* $Id: interface_consistency.h,v 1.3 1997/03/11 14:29:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef interface_consistency_h
#define interface_consistency_h
#include <libs/frontEnd/include/gi.h>

EXTERN(Boolean, entry_is_consistent, (GI_ENT *base_entry,
				       GI_ENT *new_entry,
				       char *base_ident,
				       char *new_ident,
				       char **error_bufptr,
				       char **warning_bufptr,
				       int   *error_kind));

#endif
