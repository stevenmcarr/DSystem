/* $Id: xstack.C,v 1.1 1997/06/25 15:19:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/ExtensibleArray.h>

#include <libs/support/stacks/xstack.h>


struct stack_struct 
{
  int *the_stack;	/* An extensible array representing the stack */
  int tos;		/* Top of stack, one greater than index of topmost element */
};


Stack	stack_create(int eltsize)
{
	Stack newStack = (Stack) get_mem( sizeof(struct stack_struct), "stack_create" );

	/* We initially allocate room for 1 thing on the stack. */
	newStack->the_stack = xalloc(1,eltsize);
	newStack->tos = 0;
	return newStack;
}

void	stack_push(Stack stack, Generic *elt)
{
	stack->the_stack = xput(stack->the_stack, stack->tos++, (char*) elt);
}

Boolean	stack_pop(Stack stack, Generic *elt)
{
	/* Is there anything on the stack? */
	if (stack->tos <= 0)
		return false;

	stack->the_stack = xget( stack->the_stack, --stack->tos, (char*) elt );
	return true;
}

Boolean	stack_get(Stack stack, Generic *elt, int depth)
{
	if (0 < depth && depth <= stack->tos)
	{
		stack->the_stack = xget(stack->the_stack,
					stack->tos - (depth),
					(char *) elt );
		return true;
	}
	else
		return false;
}

Boolean	stack_set(Stack stack, Generic *elt, int depth)
{
	if (0 < depth && depth <= stack->tos)
	{
		stack->the_stack = xput(stack->the_stack,
					stack->tos - (depth),
					(char *) elt );
		return true;
	}
	else
		return false;
}

int stack_depth(Stack stack)
{
	return stack->tos;
}

int *stack_addr(Stack stack, int depth)
{
	if (0 < depth && depth <= stack->tos)
		return &(stack->the_stack[stack->tos - depth]);
	else
		return (int *)0;
}

int *stack_on(Stack stack, Generic *elt, stack_on_func cmp)
{
	int i;
	int *tmp;
	int *rtn = (int *)0;

	for (i = 1; i <= stack->tos; i++)
	{
		tmp = stack_addr (stack, i);
		if (cmp((char *)elt, (char *)tmp))
			return tmp;
	}
	return rtn;
}

void	stack_destroy(Stack stack)
{
	xfree(stack->the_stack);
	free_mem((void*) stack);
}
