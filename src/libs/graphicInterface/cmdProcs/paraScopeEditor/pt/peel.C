/* $Id: peel.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************/
/*                                                                         */
/*   peel.c - Contains the code to do loop peel of the first or last       */
/*            iterations of a do loop.                                     */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

STATIC(void,     peel_first_iteration,(PedInfo ped, AST_INDEX loop, Boolean iteration,
                                       Boolean *finished_ptr));
STATIC(void,     peel_last_iteration,(PedInfo ped, AST_INDEX loop, Boolean iteration,
                                      Boolean *finished_ptr));
STATIC(Boolean,  add_peel_stmts,(PedInfo ped, char *inductvar, AST_INDEX pld_it,
                                 Boolean iteration, AST_INDEX loop, AST_INDEX grd));
STATIC(Boolean,  check_loop,(AST_INDEX node));
STATIC(Boolean,  grd_simplifier,(AST_INDEX node, Boolean *rslt));
STATIC(Boolean,  compare,(AST_INDEX node, Boolean *bptr));

/*==================================================================================
 *  pt_peel_iteration(ped, loop,iteration)
 *        The calling function for loop peel.
 *
 *  Inputs:  loop - The loop
 *           iteration - true = 1st, false = last
 *  Outputs: none
 */

void
pt_peel_iteration (PedInfo ped, AST_INDEX loop, Boolean iteration, 
                   int number_iterations)
{
    int             i;
    Boolean         finished = false;
    
    if (iteration)
	for (i = 1; i <= number_iterations; i++)
	{
	    if (NOT(finished))
		peel_first_iteration(ped, loop, iteration, &finished);
	}
    else
	for (i = 1; i <= number_iterations; i++)
	{
	    if (NOT(finished))
		peel_last_iteration(ped, loop, iteration, &finished);
	}
}

/*==================================================================================
 * peel_first_iteration (ped, loop, iteration, finished_ptr)
 *	Changes the induction variable lower bound to lower bound
 *	+ 1, and calls the function to add the statements for the
 *      old first iteration.
 *
 *  Inputs:  ped - The dependence abstract
 *           loop - The loop
 *           iteration - true = 1st, false = last
 *           finished_ptr - tells if loop has been deleted
 *
 * Outputs:  none
 *==================================================================================
 */
static void
peel_first_iteration (PedInfo ped, AST_INDEX loop, Boolean iteration, 
                      Boolean *finished_ptr)
{
    AST_INDEX       loopcntrl;	/* loop control structure */
    AST_INDEX       low_bound, lb_step, first_it;	/* old and new bound */
    AST_INDEX       high_bnd;	/* copies of original loop bounds */
    AST_INDEX       grd;	/* gaurd expression for the peeled iteration */
    AST_INDEX       temp1, temp2, temp3, zero;
    AST_INDEX       simplified_node;
    AST_INDEX       step;	/* loop step size (default:1) */
    char           *inductvar;	/* induction variable */
    
    /* get the induction variable */
    loopcntrl = gen_DO_get_control(loop);
    inductvar = gen_get_text(gen_INDUCTIVE_get_name(loopcntrl));
    
    /* get the lower bound */
    low_bound = tree_copy(gen_INDUCTIVE_get_rvalue1(loopcntrl));
    
    /* save copies of the bounds */
    high_bnd = tree_copy(gen_INDUCTIVE_get_rvalue2(loopcntrl));
    
    /* get the step size */
    step = gen_INDUCTIVE_get_rvalue3(loopcntrl);
    
    if (step == AST_NIL)			/* generate the expression 1 */
    {
	step = gen_CONSTANT();
	gen_put_text(step, "1", STR_CONSTANT_INTEGER);
    }
    else			/* generate the expression step */
    {
	step = tree_copy(step);
    }
    
    /* generate the expression up_bound + step */
    lb_step = gen_BINARY_PLUS(low_bound, step);
    
    /* insert the expression into the loop */
    gen_INDUCTIVE_put_rvalue1(loopcntrl, lb_step);
    simplified_node = pt_simplify_node(lb_step);
    if (simplified_node != lb_step)
	tree_replace(lb_step, simplified_node);
    
    /* generate the guard expression for the iteration */
    
    first_it = tree_copy(low_bound);
    zero = gen_CONSTANT();
    gen_put_text(zero, "0", STR_CONSTANT_INTEGER);
    temp1 = gen_BINARY_LE(tree_copy(first_it), tree_copy(high_bnd));
    temp2 = gen_BINARY_GT(tree_copy(step), tree_copy(zero));
    temp3 = gen_BINARY_AND(temp1, temp2);
    temp1 = gen_BINARY_GE(tree_copy(first_it), tree_copy(high_bnd));
    temp2 = gen_BINARY_LT(tree_copy(step), tree_copy(zero));
    temp1 = gen_BINARY_AND(temp1, temp2);
    grd = gen_BINARY_OR(temp1, temp3);
    
    /* insert the statements */
    
    *finished_ptr = add_peel_stmts(ped, inductvar, first_it, iteration, loop, grd);
}

/*==================================================================================
 * peel_last_iteration(ped, loop, iteration, finished_ptr)
 *	Changes the induction variable upper bound to upper bound
 *	- 1, and calls the function to add the statements for the
 *      old last iteration.
 *
 *  Inputs:  ped - The dependence abstract
 *           loop - The loop
 *           iteration - true = 1st, false = last
 *           finished_ptr - tells if loop has been deleted
 *
 * Outputs:  none
 *==================================================================================
 */
static void
peel_last_iteration(PedInfo ped, AST_INDEX loop, Boolean iteration, 
                    Boolean *finished_ptr)
{
    AST_INDEX       loopcntrl;	/* control structure */
    AST_INDEX       last_it, ub_step;	/* old bound and new bound */
    AST_INDEX       high_bnd, low_bnd;	/* copies of original loop bounds */
    AST_INDEX       step;	/* step size for the loop or constant one */
    AST_INDEX       grd;	/* guard for peeled iteration */
    AST_INDEX       simplified_node;
    AST_INDEX       temp1, temp2, temp3, zero;
    char           *inductvar;	/* induction variable */
    
    /* get the induction variable */
    loopcntrl = gen_DO_get_control(loop);
    inductvar = gen_get_text(gen_INDUCTIVE_get_name(loopcntrl));
    
    /* save copies of the bounds */
    low_bnd = tree_copy(gen_INDUCTIVE_get_rvalue1(loopcntrl));
    high_bnd = tree_copy(gen_INDUCTIVE_get_rvalue2(loopcntrl));
    
    
    /* get the step size */
    step = gen_INDUCTIVE_get_rvalue3(loopcntrl);
    if (step == AST_NIL)
    {			/* generate the expression 1 */
	step = gen_CONSTANT();
	gen_put_text(step, "1", STR_CONSTANT_INTEGER);
    }
    else
    {			/* generate the expression  step */
	step = tree_copy(step);
    }
    /* generate the expression last_it - step */
    ub_step = gen_BINARY_MINUS(tree_copy(high_bnd), step);
    
    /* insert the expression into the loop */
    gen_INDUCTIVE_put_rvalue2(loopcntrl, ub_step);
    simplified_node = pt_simplify_node(ub_step);
    if (simplified_node != ub_step)
	tree_replace(ub_step, simplified_node);
    
    if (is_constant(step) && (!strcmp(gen_get_text(step), "1")))
    {
	last_it = high_bnd;			/* step size is 1 */
    }
    else						/* step size is not 1 */
    {
	temp1 = gen_BINARY_MINUS(tree_copy(high_bnd), tree_copy(low_bnd));
	temp2 = gen_BINARY_DIVIDE(temp1, tree_copy(step));
	temp3 = gen_BINARY_TIMES(temp2, tree_copy(step));
	last_it = gen_BINARY_PLUS(tree_copy(low_bnd), temp3);
	last_it = pt_simplify_node(last_it);
    }
    
    /* generate the guard expression for the iteration */
    
    zero = gen_CONSTANT();
    gen_put_text(zero, "0", STR_CONSTANT_INTEGER);
    temp1 = gen_BINARY_GE(tree_copy(last_it), tree_copy(low_bnd));
    temp2 = gen_BINARY_GT(tree_copy(step), tree_copy(zero));
    temp3 = gen_BINARY_AND(temp1, temp2);
    temp1 = gen_BINARY_LE(tree_copy(last_it), tree_copy(low_bnd));
    temp2 = gen_BINARY_LT(tree_copy(step), tree_copy(zero));
    temp1 = gen_BINARY_AND(temp1, temp2);
    grd = gen_BINARY_OR(temp1, temp3);
    
    /* insert the statements */
    *finished_ptr = add_peel_stmts(ped, inductvar, last_it, iteration, loop, grd);
}


/*==================================================================================
 * add_peel_stmts (ped, inductvar, pld_it, iteration, loop, grd)
 *	steps through the loop's statement list and calls search statement.
 *	Then inserts the new statement in the appropriate place
 *
 * Inputs: inductvar - name of the induction variable
 *	   pld_it - the expression to replace the induction variable with
 *	   iteration - true = 1st, false = last
 *	   loop - the loop
 *    	    grd - the gaurd for the peeled iteration
 *
 * Outputs: tells if loop deleted (true if so else false )
 *==================================================================================
 */
static Boolean
add_peel_stmts (PedInfo ped, char *inductvar, AST_INDEX pld_it, 
                Boolean iteration, AST_INDEX loop, AST_INDEX grd)
{
    AST_INDEX       oldnode, curnode;	/* nodes to add after */
    AST_INDEX       curstmt, stmtcopy;	/* current statement and copy */
    AST_INDEX       ifstmt, lst;	/* if statement variables to 
					   protect peeled iteration */
    AST_INDEX       slist;
    Boolean         grd_true, grd_loc;
    Boolean         contnue = false;	/* true if there is a labeled continue 
					 * for the DO rather than an ENDDO */
    
    slist = gen_DO_get_stmt_LIST(loop);
    
    /* ifstmt is just a place holder for grd at this point */
    ifstmt = gen_GUARD(AST_NIL, grd, list_create(AST_NIL));
    grd_true = grd_simplifier(grd, &grd_loc);
    if (grd_loc)
    {			/* peeled statements & loop can not be executed
			 * so delete loop & don't add statements */
	list_remove_node(loop);
	return true;
    }
    grd = gen_GUARD_get_rvalue(ifstmt);
    tree_replace(grd, AST_NIL);
    tree_free(ifstmt);
    if (NOT(grd_true))
    {
	/* create the protecting if */
	ifstmt = gen_GUARD(AST_NIL, grd, list_create(AST_NIL));
	lst = gen_GUARD_get_stmt_LIST(ifstmt);
	ifstmt = gen_IF(AST_NIL, AST_NIL, list_create(ifstmt));
	
	if (iteration)
	    list_insert_before(loop, ifstmt);
	else
	    list_insert_after(loop, ifstmt);
    }
    
    if (gen_DO_get_lbl_ref(loop) != AST_NIL)
	contnue = true;

    /* set the old node to the loop index */
    oldnode = loop;
    
    /* loop through the statements */
    for (curstmt = list_first(slist); curstmt != AST_NIL; 
	   curstmt = list_next(curstmt))
    {
	stmtcopy = tree_copy(curstmt);
	pt_clear_info (ped, stmtcopy);
	
	/* replace the inductive var by its value in the statement */
	pt_var_replace (stmtcopy, inductvar, pld_it);
	
	if (grd_true)
	{
	    if (iteration)
		list_insert_before(loop, stmtcopy);
	    else
	    {
		curnode = list_insert_after(oldnode, stmtcopy);
		oldnode = curnode;
	    }
	}
	else
	    list_insert_last(lst, stmtcopy);
    }
    /* delete the continue from the statement list */
    if (contnue)
    {
	if (grd_true)
	{
	    if (iteration)
		list_remove_node(list_prev(loop));
	    else
		list_remove_node(oldnode);
	}
	else
	    list_remove_last(lst);
    }
    return check_loop(loop);
}



/*======================================================================
 *
 *  grd_simplifier()
 *
 *  Just needs to be smart enough to handle peel guards
 *  of the form:" a .and. b or c .and. d "
 *  Input: 	node  - the ast for a logical node of form shown above
 *  Output: 	*rslt = true when entire node simplifies to false
 *=======================================================================
 */

static Boolean
grd_simplifier(AST_INDEX node, Boolean *rslt)
    /* node: constant expr   */
    /* rslt: result of evaluation   */
{
    Boolean         c_left, c_right, b1, b2;
    Boolean         and_1_false, and_2_false;
    AST_INDEX       and_node, left, right;
    
    /*---------------------------------------------------*/
    
    *rslt = false;
    if (GEN_BINARY_OR == gen_get_node_type(node))
    {
	and_node = gen_BINARY_OR_get_rvalue1(node);
	if (GEN_BINARY_AND == gen_get_node_type(and_node))
	{
	    left = gen_BINARY_AND_get_rvalue1(and_node);
	    right = gen_BINARY_AND_get_rvalue2(and_node);
	    c_left = compare(left, &b1);
	    c_right = compare(right, &b2);
	    and_1_false = BOOL(b1 && b2);	/* one of the two was false */
	    if (c_left && c_right)
	    {
		return true;
	    }
	    if (c_left)
	    {
		tree_replace(right, AST_NIL);
		tree_replace(and_node, right);
		tree_free(and_node);
	    }
	    else if (c_right)
	    {
		tree_replace(left, AST_NIL);
		tree_replace(and_node, left);
		tree_free(and_node);
	    }
	    and_node = gen_BINARY_OR_get_rvalue2(node);
	    if (gen_get_node_type(and_node) != GEN_BINARY_AND)
		return false;
	    left = gen_BINARY_AND_get_rvalue1(and_node);
	    right = gen_BINARY_AND_get_rvalue2(and_node);
	    c_left = compare(left, &b1);
	    c_right = compare(right, &b2);
	    and_2_false = BOOL(b1 && b2);	/* one of the two was false */
	    if (c_left && c_right)
	    {
		return true;
	    }
	    if (c_left)
	    {
		tree_replace(right, AST_NIL);
		tree_replace(right, AST_NIL);
		tree_replace(and_node, right);
		tree_free(and_node);
	    }
	    else if (c_right)
	    {
		tree_replace(left, AST_NIL);
		tree_replace(and_node, left);
		tree_free(and_node);
	    }
	    left = gen_BINARY_OR_get_rvalue1(node);
	    right = gen_BINARY_OR_get_rvalue2(node);
	    if (NOT(and_1_false) && NOT(and_2_false))
	    {
		*rslt = true;	/* both false */
		tree_free(right);
		tree_free(left);
		tree_free(node);
	    }
	    else if (NOT(and_1_false))
	    {
		tree_replace(right, AST_NIL);
		tree_replace(node, right);
		tree_free(node);
	    }
	    else if (NOT(and_2_false))
	    {
		tree_replace(left, AST_NIL);
		tree_replace(node, left);
		tree_free(node);
	    }
	}
    }
    return false;
}

/*=====================================================================
 *  compare()   returns true if entire expression evaluates to true; 
 *              false otherwise
 *		*bptr is value of expression when false returned
 *
 *
 *   Just needs to be smart enough to handle peel guards
 *		form: x .le. y
 *		      x .ge. y
 *		      x .lt. y
 *		      x .gt. y
 *=======================================================================
*/

static Boolean
compare(AST_INDEX node, Boolean *bptr)
{
    int             value_left, value_right, i;
    Boolean         pt_rtn;
    AST_INDEX       left, right;
    
    *bptr = true;		/* default could not simplify */
    
    switch (gen_get_node_type(node))
    {
	
    case GEN_BINARY_GE:
	left = gen_BINARY_GE_get_rvalue1(node);
	right = gen_BINARY_GE_get_rvalue2(node);
	pt_rtn = pt_eval(left, &value_left);
	if (pt_rtn)
	    return false;	/* not a constant */
	pt_rtn = pt_eval(right, &value_right);
	if (pt_rtn)
	    return false;	/* not a constant */
	*bptr = BOOL(value_left >= value_right);
	return *bptr;
    case GEN_BINARY_GT:
	left = gen_BINARY_GT_get_rvalue1(node);
	right = gen_BINARY_GT_get_rvalue2(node);
	pt_rtn = pt_eval(left, &value_left);
	if (pt_rtn)
	    return false;	/* not a constant */
	pt_rtn = pt_eval(right, &value_right);
	if (pt_rtn)
	    return false;	/* not a constant */
	*bptr = BOOL(value_left > value_right);
	return *bptr;
    case GEN_BINARY_LE:
	left = gen_BINARY_LE_get_rvalue1(node);
	right = gen_BINARY_LE_get_rvalue2(node);
	pt_rtn = pt_eval(left, &value_left);
	if (pt_rtn)
	    return false;	/* not a constant */
	pt_rtn = pt_eval(right, &value_right);
	if (pt_rtn)
	    return false;	/* not a constant */
	*bptr = BOOL(value_left <= value_right);
	return *bptr;
    case GEN_BINARY_LT:
	left = gen_BINARY_LT_get_rvalue1(node);
	right = gen_BINARY_LT_get_rvalue2(node);
	pt_rtn = pt_eval(left, &value_left);
	if (pt_rtn)
	    return false;	/* not a constant */
	pt_rtn = pt_eval(right, &value_right);
	if (pt_rtn)
	    return false;	/* not a constant */
	*bptr = BOOL(value_left < value_right);
	return *bptr;
    }
    return false;
}


/*=================================================================
 *
 *  check_loop()
 *
 *  Deletes loop if it can not be executed
 *  Input: node		- ast index of loop to be checked
 *  Output: tells if loop was deleted (true is deleted)
 *=================================================================
*/

static Boolean
check_loop(AST_INDEX node)
    /* node: constant expr   */
{
    int             value[3];	/* ptr to result & result   */
    AST_INDEX       loopcntrl;	/* loop control structure */
    AST_INDEX       low_bound, high_bound;	/* old and new bound */
    AST_INDEX       step;	/* loop step size (default:1) */
    
    /* get the induction variable */
    loopcntrl = gen_DO_get_control(node);
    
    /* get the lower bound */
    low_bound = gen_INDUCTIVE_get_rvalue1(loopcntrl);
    
    /* save copies of the bounds */
    high_bound = gen_INDUCTIVE_get_rvalue2(loopcntrl);
    
    /* get the step size */
    step = gen_INDUCTIVE_get_rvalue3(loopcntrl);
    if (step != AST_NIL)
    {
	if (pt_eval(step, value))
	    return false;	/* expression does not simplify */
    }
    else
	value[0] = 1;
    
    if (NOT(pt_eval(low_bound, value + 1)))
    {
	if (NOT(pt_eval(high_bound, value + 2)))
	{
	    if ((value[0] < 0) && (value[2] > value[1]))
	    {
		list_remove_node(node);
		return true;
	    }
	    if ((value[0] > 0) && (value[2] < value[1]))
	    {
		list_remove_node(node);
		return true;
	    }
	}
    }
    return false;
}

