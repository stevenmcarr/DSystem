/* $Id: gstack.h,v 3.3 1997/03/11 14:36:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
 *  -- gstack.h
 *
 *          This file is the header file for stack.c
 *          This is a modification of xstack in Rn.
 *
 *          By: Po-Jen Yang                   June 1991
 ****************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>

typedef struct gstack_struct {
    Generic *the_stack;	/* An extensible array representing the stack */
    int nElt;   	/* # of elements in the array used for this stack */
    int eltSize;	/* Size of each element in the stack */
    int alignEltSize;	/* Word align element size */
    int top;	/* Top of stack, one greater than index of topmost element */
} *GStack;





/*---------------------------------------------------------------------------
 *  -- gstack_create
 * 
 *           Create a Stack structure with "nElt" elements of size "eltSize".
 *     nElt is the initial size of the stack.  The stack will grow if 
 *     more than nElt is pushed onto the stack
 *---------------------------------------------------------------------------*/
EXTERN( GStack, gstack_create, (int nElt, int eltSize) );



/*---------------------------------------------------------------------------
 *  -- gstack_push
 * 
 *           Push an element onto the stack
 *---------------------------------------------------------------------------*/
EXTERN( void, gstack_push, (GStack stack, Generic* elt) );



/*---------------------------------------------------------------------------
 *  -- gstack_pop
 * 
 *           Pop an element from the stack
 *---------------------------------------------------------------------------*/
EXTERN( Boolean, gstack_pop, (GStack stack, Generic* elt) );



/*---------------------------------------------------------------------------
 *  -- gstack_top
 * 
 *           Get the value of the element on top the stack
 *           Retrun false if stack is empty
 *---------------------------------------------------------------------------*/
EXTERN( Boolean, gstack_top, (GStack stack, Generic* elt) );
 
/*---------------------------------------------------------------------------
 *  -- gstack_set
 * 
 *           Set an element of the stack at depth "depth"
 *---------------------------------------------------------------------------*/
EXTERN( Boolean, gstack_set, (GStack stack, Generic* elt, int depth) );



/*---------------------------------------------------------------------------
 *  -- gstack_get
 * 
 *           Get an element from the stack at depth "depth"
 *---------------------------------------------------------------------------*/
EXTERN( Boolean, gstack_get, (GStack stack, Generic* elt, int depth) );




/*---------------------------------------------------------------------------
 *  -- gstack_depth
 * 
 *           Get the depth of the stack.  Returns 0 means empty stack.
 *---------------------------------------------------------------------------*/
EXTERN(int, gstack_depth, (GStack stack));




/*---------------------------------------------------------------------------
 *  -- gstack_decrement_depth
 * 
 *           Decrement the depth of the stack.
 *           Return true if successful, otherwise the stack is untouched
 *---------------------------------------------------------------------------*/
EXTERN( Boolean, gstack_decrement_depth, (GStack stack, int n) );


/*---------------------------------------------------------------------------
 *  -- gstack_pop_all
 * 
 *           Reset the stack to be an empty stack
 *---------------------------------------------------------------------------*/
EXTERN( void, gstack_pop_all, (GStack stack) );




/*---------------------------------------------------------------------------
 *  -- gstack_destroy
 * 
 *           Free all the memory associated with stack
 *---------------------------------------------------------------------------*/
EXTERN( void, gstack_destroy, (GStack stack) );

