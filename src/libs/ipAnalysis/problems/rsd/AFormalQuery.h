/* $Id: AFormalQuery.h,v 1.1 1997/03/11 14:35:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AFormalQuery_h
#define AFormalQuery_h

/**************************************************************************
// 
// AFormalQuery.h 
//
// query interface for whether a parameter to a procedure is ever 
// accessed as an array by that procedure or any of its descendants 
// in the callgraph
//
// Author: 
//  John Mellor-Crummey                                      February 1993
//
// Copyright 1992, 1993, Rice University, as part of the ParaScope 
// Programming Environment Project.
//
 ************************************************************************/



#ifndef general_h
#include <libs/support/misc/general.h>
#endif


/**************************************************************************
//
// Boolean IPQuery_ParamIsAFormal
//
//         a query interface for interprocedural analysis of whether a 
//         formal parameter of a procedure is ever indexed as an array by 
//         the procedure itself, or any procedures reachable from it.
//         
//         this capability will eventally be subsumed by regular section
//         analysis
//
//         ARGUMENTS:
//           cg:           callgraph obtained from program compiler iterface
//           procEntry:    name of the routine entry point invoked 
//           param_index:  0-based index of the parameter position being 
//                         queried
//         
/ ************************************************************************/
EXTERN(Boolean, IPQuery_ParamIsAFormal,
       (Generic cg, const char *procEntry, unsigned int param_index));


/**************************************************************************
// 
// Boolean IPQuery_CalleeParamIsAFormal
//         a query interface for interprocedural analysis of whether an 
//         actual parameter to a procedure is ever indexed as an array by 
//         the procedure itself, or any procedures reachable from it.
//         
//         if an actual parameter is only accessed as a scalar and never as
//         an array, then some dependences can be identified as loop
//         independent rather than loop carried. 
//
//         an actual parameter to a procedure is never indexed as an array
//         by a procedure or any of that procedure's descendants in the call 
//         chain only if the corresponding formal parameter of the procedure
//         is never indexed as an array.
//     
//         this capability will eventally be subsumed by regular section
//         analysis
//
//         ARGUMENTS:
//           cg:           callgraph obtained from program compiler iterface
//           caller:       name of calling procedure
//           callsite_id:  ft_NodeToNumber id for the callsite 
//           param_index:  0-based index of the parameter position being 
//                         queried
//         
/ ************************************************************************/
EXTERN(Boolean, IPQuery_CalleeParamIsAFormal, 
       (Generic cg, const char *caller, int callsite_id, 
	unsigned int param_index));


#endif /* AFormalQuery_h */
