/* $Id: xstack.h,v 1.8 1997/06/25 15:19:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef xstack_h
#define xstack_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#define STACK_TOP 1		/* Depth of top of stack */

struct stack_struct;
typedef struct stack_struct *Stack;

typedef FUNCTION_POINTER (Generic*, stack_on_func, (char*, char*));

EXTERN(Stack, stack_create, (int eltsize));
/*
 * Creates a stack with elements of size eltsize.
 * This stack must be freed with stack_destroy.
 */

EXTERN(void, stack_push, (Stack stack, Generic *elt));
/*
 * Pushes the element pointed to by elt onto stack.
 */

EXTERN(Boolean, stack_pop, (Stack stack, Generic *elt));
/* 
 * Pops the top element of the stack, putting it into elt.
 * Returns false if the stack was empty.
 */

EXTERN(Boolean, stack_get, (Stack stack, Generic *elt, int depth));
/*
 * Copies the element "depth" elements from the top of the stack into
 * elt.  The top of the stack is at depth 1.
 * Returns false if "depth" is too shallow (non-positive) or too deep.
 */

EXTERN(Boolean, stack_set, (Stack stack, Generic *elt, int depth));
/* 
 * Copies the element pointed to by elt onto the stack element which is
 * "depth" elements from the top of the stack.
 * The top of the stack is at depth 1.
 * Returns false if "depth" is too shallow (non-positive) or too deep.
 */

EXTERN(int, stack_depth, (Stack stack));
/*
 * Returns the number of elements on the stack.  Thus,
 *     stack_get(stack,&elt,stack_depth(stack))
 * retrieves the deepest element on the stack.
 */

EXTERN(int *, stack_addr, (Stack stack, int depth));
/*
 * Returns a true pointer to the element on the stack at depth "depth".
 * This pointer may be invalidated by calls to stack_push() and
 * stack_destroy(), and consequently should not be held onto across 
 * calls to these functions.  If depth is invalid, (Generic *)0 is
 * returned.
 */

EXTERN(int *, stack_on,
 (Stack stack, Generic *elt, stack_on_func eq));
/*
 * Returns a true pointer to the first element which successfully
 * compares with the passed in element.  (Generic *) 0 is returned
 * if no match is found.
 */

EXTERN(void, stack_destroy, (Stack stack));
/*
 * Frees memory associated with a stack.
 */

#endif
