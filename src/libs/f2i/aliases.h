/* $Id: aliases.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef AliasQuery_h
#define AliasQuery_h

/******************************************************************************
 * AliasQuery.h
 *
 *
 ******************************************************************************/


#ifndef general_h
#include <libs/support/misc/general.h>
#endif


/******************************************************************************
 * Boolean IPQuery_IsAliased
 * 
 * return true if the variable identified by the triple (vname, offset, length)
 * is in the aliased in procedure proc_name
 *
 ******************************************************************************/
EXTERN(Boolean, IPQuery_IsAliased, (Generic callGraph, char *proc_name, 
				    char *vname, int length, int offset,
				    Boolean IsGlobal));


#endif /* AliasQuery_h */
