/* $Id: ScalarModRefQuery.h,v 1.4 1997/03/11 14:35:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ScalarModRefQuery_h
#define ScalarModRefQuery_h

/******************************************************************************
 * ScalarModRefQuery.h
 *
 * interface to query interprocedural solutions for scalar mod and scalar ref.
 *
 * Author:
 *   John Mellor-Crummey                                       February 1993
 *
 * Copyright 1993, Rice University, as part of the ParaScope Programming
 * Environment Project.
 ******************************************************************************/


#ifndef general_h
#include <libs/support/misc/general.h>
#endif


/******************************************************************************
 * IPQuery_IsScalarRef
 * 
 * return true if the variable identified by the triple (vname, offset, length)
 * is in the referenced set of the callee specified by the (caller, callsite)
 * pair.
 *
 * the return code is gracefully conservative if callGraph is NULL, or the
 * callsite is not found in the (possibly stale) call graph. 
 *
 ******************************************************************************/
EXTERN(Boolean, IPQuery_IsScalarRef, (Generic callGraph, char *caller, 
				      int callsite, char *vname, int offset, 
				      int length));

/*  return true if variable is in the referenced set of the procedure 
 *  by its name "proc"
 */
EXTERN(Boolean, IPQuery_IsScalarRefNode, (Generic callGraph, char *proc, 
					  char *vname, int offset, int length));

/*  return true if the actual parameter at actual_pos has its corresponding
 *  formal parameter in the referenced set of the callee.
 */
EXTERN(Boolean, IPQuery_IsScalarRefArg, (Generic callGraph, char *caller, 
					 int callsite, int actual_pos));

/******************************************************************************
 * Boolean IPQuery_IsScalarMod
 * 
 * return true if the variable identified by the triple (vname, offset, length)
 * is in the modified set of the callee specified by the (caller, callsite)
 * pair.
 *
 * the return code is gracefully conservative if callGraph is NULL, or the
 * callsite is not found in the (possibly stale) call graph. 
 *
 ******************************************************************************/
EXTERN(Boolean, IPQuery_IsScalarMod, (Generic callGraph, char *caller, 
				      int callsite, char *vname, int offset, 
				      int length));

/*  return true if variable is in the modified set of the procedure 
 *  by its name "proc"
 */
EXTERN(Boolean, IPQuery_IsScalarModNode, (Generic callGraph, char *proc,
					  char *vname, int offset, int length));

/*  return true if the actual parameter at actual_pos has its corresponding
 *  formal parameter in the modified set of the callee.
 */
EXTERN(Boolean, IPQuery_IsScalarModArg, (Generic callGraph, char *caller, 
					 int callsite, int actual_pos));

#endif /* ScalarModRefQuery_h */
