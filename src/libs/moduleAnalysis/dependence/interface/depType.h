/* $Id: depType.h,v 1.3 1997/03/11 14:35:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Header */

#ifndef depType_h
#define depType_h


/* Enumerated type for the type of a dependence edge.
 */
typedef enum DependenceType {
	dg_true,
	dg_anti,
	dg_output,
	dg_input,
	dg_inductive,	
	dg_exit,
	dg_io,
	dg_call,
	dg_control,
	dg_unknown
} DepType;



#endif /* depType_h */
