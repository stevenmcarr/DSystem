/* $Id: el_instance.h,v 1.5 1997/03/11 14:35:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************

   NEW: Views on top of the dependence filter facility (leads to a pretty
   sorry user interface, but at least a decent functionality).
   kats 9/90

   Data structures to implement the Edge List abstraction. This is used by
   the editor to browse through the list of dependences in the source file, 
   and by the abstractions that implement parallel transformations.
   - Vas, Sept 1987.
 
   NOTE: As long as the dependence info used by ParaScope is that generated 
   by PSERVE, remember to convert variable names into upper case before
   making any comparisons using strcmp(). This will be unnecessary when
   ParaScope becomes completely independent of PSERVE. 
   -Vas, May 1988.
 ****************************************************************************/

#ifndef el_instance_h
#define el_instance_h

#define  MAX_QUERY_STACK_DEPTH		10

typedef struct edge_list	Edge_List;

typedef struct dep_query	Query;

typedef struct el_instance	EL_Instance;

/* dependence filter choices */
#define DF_LC 		0		/* loop carried */
#define DF_CONTROL  	1		/* control  */
#define DF_LI 		2		/* loop independent */
#define DF_PRIVATE 	3		/* dependences on private variables */

#endif	/* el_instance_h */
