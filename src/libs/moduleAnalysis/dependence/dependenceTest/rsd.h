/* $Id: rsd.h,v 1.11 1997/03/11 14:35:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef	rsd_h
#define	rsd_h
/*----------------------------------------------------------------

	rsd.h	Definitions for Regular Section Descriptors

    Though starting to look like Data Access Descriptors...

*/

#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef	dep_dt_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>
#endif
#ifndef	dt_info_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#endif
#ifndef	side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif

#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif

/*-------------------*/
/* list of rsd types */

typedef enum rsd_type 
{
	RSD_NO_DATA,		/* unknown RSD      */
	RSD_CONSTANT,		/* single constant  */
	RSD_RANGE,			/* constant range   */
	RSD_EXPR,			/* single index var */
	RSD_EXPR_RANGE,		/* index var range  */
	RSD_BOTTOM			/* bottom           */
} Rsd_type;

/*---------------------------------------------------------*/
/* description of range/expr in one dimension of subscript */

typedef struct 
{
  Rsd_type     type;          /*type of the RSD: expr,range,bottom...*/

  /* range info */

  int          lo_b;          /*lower bound of range*/
  int          up_b;          /*upper bound of range*/
  int          step;          /*stride of range*/
  int          begin;         /*first data point of range*/

  /* expression info */

  int          coeff;         /*ind var coefficient*/
  int          constant;      /*constant term */
  int          ivar;          /*level of ivar in expr*/
                              /*lo_b & up_b are ranges for const */
} Rsd_data;

/*-----------------------------------------------*/
/* description of range/expr of entire subscript */

struct Rsd_section_struct
{
	Loop_list *loop_nest;        /* ptr to enclosing Loop_list */
	int        dims;             /* # of dimensions*/
	Rsd_data   subs[MAXDIM];     /*data for each subscript*/

};

/*---------------------------------------------------*/
/* vector to hang RSD information at all loop levels */

struct Rsd_vector_struct
{
	Rsd_section *lhs[MAXLOOP];	/* just save lhs RSDs for now */

};



EXTERN(Rsd_section *, dt_alloc_rsd, (DT_info *dt));
/* allocate new RSD section */

EXTERN(Rsd_vector *, dt_alloc_rsd_vector, (DT_info *dt));
/* allocate new RSD vector */

EXTERN(void, rsd_build_section, 
		( Rsd_section * r_section, Subs_list * subscript,
		 Loop_list * loop_nest, int level ) );
/* Creates RSD from pre-parsed loop & subscript information */

EXTERN(Rsd_section *,  rsd_ref_section,
		( DT_info * dt, SideInfo * infoPtr,
		 AST_INDEX  ref, AST_INDEX  loop ) );
/*     Given a reference and a loop, create the RSD for the reference. */

EXTERN(Rsd_section *,  rsd_merge,
		( Rsd_section * result, Rsd_section * rsd1,
		 Rsd_section * rsd2 ) );
/*    Given two section handles, this function returns a handle
      for a section containing all the elements in either input
      section (the UNION of the two). 
      The returned value is a pointer to the first parameter
 */

EXTERN(Boolean, rsd_intersecting,
		( Rsd_section* rsd1, Rsd_section* rsd2 ) );
/*    Conservative estimate of whether RSDs intersect.	*/

EXTERN(void, rsd_build_vector,
		( DT_info * dt, SideInfo * infoPtr, AST_INDEX  stmt ) );
/*    Given a statement, build RSDs describing its references for
      all enclosing loops, then store in the type_ref field of the
      side array.  Just builds lhs RSDs for now.
 */

EXTERN(void, rsd_vector_init,
		( DT_info * dt_info, SideInfo * infoPtr, AST_INDEX root) );
/*	Builds Rsd_vectors for all statements in tree. */


#endif
