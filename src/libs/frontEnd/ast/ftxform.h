/* $Id: ftxform.h,v 1.7 1997/03/11 14:29:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ftxform_h
#define ftxform_h

/*
 * 			      ftxform.h
 * 									
 *			AST transform utility
 * 									
 *   FUNCTION:
 *	create and modify a transformation object which can then
 *	be applied to a FortTree to transform it
 *
 *   USAGE:
 *
 *	void init_graph_routine(ftx_state s);
 *
 *	ftx_state s = ftx_create(init_graph_routine);
 *	ftx_register_fn(s, xformfn, fn_name, <dependences ...> );
 *	...
 *	ftx_request_fn(s, xformfn, true/false);
 *	...
 *	ftx_transform(s, module_annotations, program_annotations); 
 *		which calls some collection 1..n of the xformfn's:
 *			xformfn_1(module_annotations, program_annotations);
 *			...
 *			xformfn_n(module_annotations, program_annotations);
 *	ftx_destroy(s);
 *
 *   AUTHOR:
 *	Robert Hood
 *	    with design consulting from:
 *		Alan Carle
 *		Ben Chase
 *		John Mellor-Crummey
 *
 *   MODIFICATION HISTORY:
 *      June 1992                             John Mellor-Crummey
 *       -- generalize argument passing to use an annotations hashtable 
 *          associated with the graph
 *       -- add support for FOLLOW ordering constraints   
 */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef struct ftx_state_t *ftx_state; 

#define TRANSFORMATION_FUNCTION(f)  EXTERN(void, f, (ftx_state s))
#define FTX_INIT_FUNCTION(f)        EXTERN(void, f, (ftx_state s))
#define FTX_ENABLE_FUNCTION(f)      EXTERN(void, f, (ftx_state s))

typedef FUNCTION_POINTER(void, XFORM_FUNCTION, (ftx_state s));

typedef FUNCTION_POINTER(void, FTX_INIT_FN, (ftx_state s));

#define FTX_FOLLOW	        ((XFORM_FUNCTION)(-1))
#define FTX_PRECEDE	        ((XFORM_FUNCTION)(-2))
#define FTX_MUST_PRECEDE	((XFORM_FUNCTION)(-3))
#define FTX_MUST_FOLLOW         ((XFORM_FUNCTION)(-4))
#define FTX_MUST_NOT	        ((XFORM_FUNCTION)(-5))
#define FTX_END                 ((XFORM_FUNCTION)(-6))


EXTERN(ftx_state, ftx_create, (FTX_INIT_FN ftx_init_routine));

EXTERN(void, ftx_destroy, (ftx_state s));

EXTERN(void, ftx_register_fn, (ftx_state s, XFORM_FUNCTION xformfn,
                               char* name, ...));

/*
	if done, must be done before me	   	FTX_PRECEDE	    ...
	if done, must be done after me	   	FTX_FOLLOW	    ...
	must not be done if I'm done	   	FTX_MUST_NOT	    ...
	must be done before me, if I'm done	FTX_MUST_PRECEDE    ...
	must be done after me, if I'm done	FTX_MUST_FOLLOW     ...
	end of arguments		   	FTX_END
*/

/* request that a transformation function be done (or not done) */
EXTERN(void, ftx_request_fn, (ftx_state s, PFI xformfn, Boolean doit));


/* return whether xformfn will be done given current requests */
EXTERN(Boolean, ftx_will_do_fn, (ftx_state s, PFI xformfn));

EXTERN(int, ftx_transform, (ftx_state s));

/* access to annotations that are globally accessible. the annotations 
 * provide a generalized mechanism for passing information between
 * operations
 */ 
EXTERN(Boolean, ftx_get_annotation, (ftx_state s, char* aname, void* value));
EXTERN(void, ftx_put_annotation, (ftx_state s, char* aname, void* value));
EXTERN(void, ftx_delete_annotation, (ftx_state s, char* aname));

# endif
