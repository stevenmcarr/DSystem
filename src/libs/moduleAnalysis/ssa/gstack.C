/* $Id: gstack.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
 *  -- gstack.c
 *
 *          This file contains a implementation of general stack.
 *          This is a modification of xstack in Rn.
 *
 *          By: Po-Jen Yang                   June 1991
 ****************************************************************************/

#include <string.h>

#include <libs/moduleAnalysis/ssa/gstack.h>


/*---------------------------------------------------------------------------
 *  -- gstack_create
 * 
 *           Create a Stack structure with "nElt" elements of size "eltSize".
 *     nElt is the initial size of the stack.  The stack will grow if 
 *     more than nElt is pushed onto the stack
 *---------------------------------------------------------------------------*/
GStack gstack_create(int nElt, int eltSize)
{
    GStack newStack;
    int alignEltSize;
    
    /* word align */
    alignEltSize = (eltSize + (sizeof(int) -1)) / sizeof(int) * sizeof(int); 
    
    newStack = (GStack) get_mem(sizeof(struct gstack_struct), "gstack_create");
    newStack->the_stack =  (Generic *) get_mem ( nElt * alignEltSize ,"the_stack");
    newStack->nElt = nElt;
    newStack->eltSize = eltSize;
    newStack->alignEltSize = alignEltSize;
    newStack->top = 0;
    
    return(newStack);
    
} /* end of gstack_create() */





/*---------------------------------------------------------------------------
 *  -- gstack_destroy
 * 
 *           Free all the memory associated with stack
 *---------------------------------------------------------------------------*/
void gstack_destroy(GStack stack)
{
    if (stack)
    {
        free_mem((void*)stack->the_stack);
        stack->the_stack = (Generic *) 0;
        free_mem((void*) stack);
    }
    
    return;
} /* end of gstack_destroy() */





/*---------------------------------------------------------------------------
 *  -- gstack_push
 * 
 *           Push an element onto the stack
 *---------------------------------------------------------------------------*/
void gstack_push(GStack stack, Generic *elt)
{
    char* dest;
    
    if (stack->top == stack->nElt)
    {
        /* need to grow the stack */
        int newSize;
        
        newSize = stack->nElt * 2 + 1;
        stack->the_stack = 
	    (Generic *) reget_mem( (void*) (stack->the_stack),
				   newSize * stack->alignEltSize,
				   "grow_stack");
        stack->nElt = newSize;
    }
    
    dest = ((char*)stack->the_stack) + stack->top * stack->alignEltSize;
    memcpy(dest, (char*)elt, stack->eltSize);
    
    stack->top++;
    return;
    
} /* gstack_push() */





/*---------------------------------------------------------------------------
 *  -- gstack_pop
 * 
 *           Pop an element from the stack
 *---------------------------------------------------------------------------*/
Boolean gstack_pop(GStack stack, Generic *elt)
{
    char* src;
    
    if (stack->top <= 0)
        return(false);
    
    stack->top--;
    
    src = ((char*)stack->the_stack) + stack->top * stack->alignEltSize;
    memcpy( (char*)elt, src, stack->eltSize);
    
    return(true);
    
} /* end of gstack_pop() */





/*---------------------------------------------------------------------------
 *  -- gstack_top
 * 
 *           Get the value of the element on top the stack
 *           Retrun false if stack is empty
 *---------------------------------------------------------------------------*/
Boolean gstack_top(GStack stack, Generic *elt)
{
    char* src;
    
    if (stack->top <= 0)
        return(false);
    
    src = ((char*)stack->the_stack) + (stack->top - 1) * stack->alignEltSize;
    memcpy( (char*)elt, src, stack->eltSize);
    
    return(true);
    
} /* end of gstack_top() */





/*---------------------------------------------------------------------------
 *  -- gstack_set
 * 
 *           Set an element of the stack at depth "depth"
 *---------------------------------------------------------------------------*/
Boolean gstack_set(GStack stack, Generic *elt, int depth)
{
    char* dest;
    
    if ( (depth >= stack->top) || (depth < 0) )
        return(false);          /* out of range */
    
    dest = ((char*)stack->the_stack) + depth * stack->alignEltSize;
    memcpy( dest, (char*)elt, stack->eltSize);
    
    return(true);
    
} /* end of gstack_set() */





/*---------------------------------------------------------------------------
 *  -- gstack_get
 * 
 *           Get an element from the stack at depth "depth"
 *---------------------------------------------------------------------------*/
Boolean gstack_get(GStack stack, Generic *elt, int depth)
{
    char* src;
    
    if ( (depth >= stack->top) || (depth < 0) )
        return(false);          /* out of range */
    
    src = ((char*)stack->the_stack) + depth * stack->alignEltSize;
    memcpy( (char*)elt, src, stack->eltSize);
    
    return(true);
    
} /* end of gstack_get() */





/*---------------------------------------------------------------------------
 *  -- gstack_depth
 * 
 *           Get the depth of the stack.  Returns 0 means empty stack.
 *---------------------------------------------------------------------------*/
int gstack_depth(GStack stack)
{
    return(stack->top);
} /* end of gstack_depth() */





/*---------------------------------------------------------------------------
 *  -- gstack_decrement_depth
 * 
 *           Decrement the depth of the stack.
 *           Return true if successful, otherwise the stack is untouched
 *---------------------------------------------------------------------------*/
Boolean gstack_decrement_depth(GStack stack, int n)
{
    if ( n > stack->top )
	return( false );

    stack->top -= n;
    return(true);
} /* end of gstack_decrement_depth() */






/*---------------------------------------------------------------------------
 *  -- gstack_pop_all
 * 
 *           Reset the stack to be an empty stack
 *---------------------------------------------------------------------------*/
void gstack_pop_all(GStack stack)
{
    stack->top = 0;
    return;
} /* end of gstack_pop_all() */



