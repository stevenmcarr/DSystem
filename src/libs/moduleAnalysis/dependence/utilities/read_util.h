/* $Id: read_util.h,v 1.6 1999/03/31 21:49:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	dep/utilities/read_util.h			  		*/
/*									*/
/*	read_util.h -- functions for reading graph and map file		*/
/*									*/
/************************************************************************/

#ifndef	read_util_h
#define	read_util_h

#ifndef	general_h
#include <libs/support/misc/general.h>
#endif

#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif

#ifndef	dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif

/*----------------------------------------------------------------------*/


EXTERN( char *,  skip_field, (char * start, char ** buf) );
EXTERN( char *,  get_field_b, (char * start, Boolean * b) );
EXTERN( char *,  get_field_c, (char * start, ConsistentType * cons_ptr) );
EXTERN( char *,  get_field_d, (char * start, int * num) );
EXTERN( char *,  get_field_s, (char * start, char ** buf) );


/*----------------------------------------------------------------------*/

#endif
