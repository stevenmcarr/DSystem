/* $Id: ptrans.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************/
/*                                                         	*/
/*	PEditorCP/pt/ptrans.c                                   */
/*                                                         	*/
/*	Parascope Transformations                               */
/*                                                         	*/
/*	Routines to support Serial to Parallel Loop and Loop    */
/*	Interchange transformations in Parascope                */
/*                                                              */
/*	Main entry for loop interchange: pt_set_loop_type()     */
/*                                                         	*/
/*				Last change:  kats 9/91    	*/
/*      Fixed loop walking routines to call walk_statements    	*/
/*      for loops containing conditional control flow.         	*/
/*      Also, now ignoring dependences on private variables.	*/
/*                                                         	*/
/****************************************************************/

#include <libs/frontEnd/ast/groups.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>



typedef struct {
    PedInfo  ped;
    int	     level;
    DG_Edge *edgeptr;
    int	     status;
} InterParm;


/*************************************************************
 * pt_get_tri_type (outer_control,inner_control)
 *
 *   Determine the type of triangular loop this is.
 *
 * Inputs: outer_control - outer DO loop AST control node
 *         inner_control - inner DO loop AST control node
 *
 * Returns: triangular type (1-8) or NOT_TRI
 *************************************************************
 */
static int
pt_get_tri_type (AST_INDEX outer_control, AST_INDEX inner_control)
{
	char *outervar;
	AST_INDEX inner1,inner2,outer1,outer2;
	AST_INDEX fac,con,temp;
	int ifac1,ifac2;
	Boolean lin1,lin2;

        if (gen_INDUCTIVE_get_rvalue3(outer_control) != AST_NIL)
	  return NOT_TRI;
        if (gen_INDUCTIVE_get_rvalue3(inner_control) != AST_NIL)
	  return NOT_TRI;

        outervar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
	inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
	inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
	outer1 = gen_INDUCTIVE_get_rvalue1(outer_control);
	outer2 = gen_INDUCTIVE_get_rvalue2(outer_control);
	pt_get_coeff(inner1,outervar,&lin1,&ifac1);
	pt_get_coeff(inner2,outervar,&lin2,&ifac2);
	if (NOT(lin1) || NOT(lin2))
	  return NOT_TRI;
	if ((ifac1 == 0) && (ifac2 == 0))
	  return NOT_TRI;
	if ((ifac1 != 0) && (ifac2 != 0))
	  return NOT_TRI;

	if (ifac1 == 0) {     /* true -> left side */
	  if (pt_expr_equal(outer1,inner1)) {
	    pt_separate_linear(inner2,outervar,&lin1,&fac,&con);
	    if (ifac2 == 1) {
	      if (pt_expr_equal(con,pt_gen_int(0)))
		return TRI_LLD;
	      if (pt_expr_equal(con,pt_gen_int(-1)))
		return TRI_LL;
	    }
	    else if (ifac2 == -1) {
	      temp = pt_gen_add(tree_copy(outer1),tree_copy(outer2));
	      if (pt_expr_equal(con,temp))
		return TRI_ULD;
	      temp = pt_gen_sub(temp,pt_gen_int(1));
	      if (pt_expr_equal(con,temp))
		return TRI_UL;
	    }
	  }
	}
	else {                /* right side */
	  if (pt_expr_equal(outer2,inner2)) {
	    pt_separate_linear(inner1,outervar,&lin1,&fac,&con);
	    if (ifac1 == 1) {
	      if (pt_expr_equal(con,pt_gen_int(0)))
		return TRI_URD;
	      if (pt_expr_equal(con,pt_gen_int(1)))
		return TRI_UR;
	    }
	    else if (ifac1 == -1) {
	      temp = pt_gen_add(tree_copy(outer1),tree_copy(outer2));
	      if (pt_expr_equal(con,temp))
		return TRI_LRD;
	      temp = pt_gen_add(temp,pt_gen_int(1));
	      if (pt_expr_equal(con,temp))
		return TRI_LR;
	    }
	  }
	}

	return NOT_TRI;
}


/*************************************************************
 * pt_get_trap_type (outer_control,inner_control)
 *
 *   Determine the type of trapezoidal loop this is.
 *
 * Inputs: outer_control - outer DO loop AST control node
 *         inner_control - inner DO loop AST control node
 *
 * Returns: trapezoidal type (1-4) or NOT_TRAP
 *************************************************************
 */
static int
pt_get_trap_type (AST_INDEX outer_control, AST_INDEX inner_control)
{
	char *outer_inductvar;
	int coeff1, coeff2;
	Boolean lin1,lin2;

        outer_inductvar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
        if (gen_INDUCTIVE_get_rvalue3(outer_control) != AST_NIL)
	  return NOT_TRAP;
        if (gen_INDUCTIVE_get_rvalue3(inner_control) != AST_NIL)
	  return NOT_TRAP;

	pt_get_coeff(gen_INDUCTIVE_get_rvalue1(inner_control),outer_inductvar,&lin1,&coeff1);
	pt_get_coeff(gen_INDUCTIVE_get_rvalue2(inner_control),outer_inductvar,&lin2,&coeff2);

	if (!lin1 || !lin2)
	  return NOT_TRAP;

	if ((coeff1 == 1) && (coeff2 == 0))
	  return TRAP_UR;
	else if ((coeff1 == -1) && (coeff2 == 0))
	  return TRAP_LR;
	else if ((coeff1 == 0) && (coeff2 == 1))
	  return TRAP_LL;
	else if ((coeff1 == 0) && (coeff2 == -1))
	  return TRAP_UL;
	else
	  return NOT_TRAP;
}


/*************************************************************
 * pt_get_arg_num (node,inductvar,arg_cnt,arg_idx,coeff)
 *
 *   Parameterize argument list at node
 *
 * Inputs: node - invocation node whose args are to be parameterized
 *         inductvar - variable to search for in args
 *
 * Outputs: arg_cnt - number of args in list
 *          arg_idx - index of only arg containing inductvar.
 *                    if no arg then return 0.
 *                    if more than 1 arg then return -1.
 *          coeff - coeffecient of inductvar in arg[arg_idx]
 *                  (if it's a constant)
 *************************************************************
 */
void
pt_get_arg_num (AST_INDEX node, char *inductvar, int *arg_cnt,
                int *arg_idx, int *coeff)
{
        AST_INDEX stmt_bump;
        int tempi;
	Boolean lin;

	/* get the first arg */
	stmt_bump = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
	*arg_cnt = 0;
	*arg_idx = 0;
	*coeff = 0;

	/* bump through the args */
	while(stmt_bump != AST_NIL) {
	        (*arg_cnt)++;
		if (pt_find_var(stmt_bump,inductvar)) {
		  if (*arg_idx == 0) {
		    *arg_idx = *arg_cnt;
		    pt_get_coeff(stmt_bump,inductvar,&lin,&tempi);
		    if (lin)
		      *coeff = tempi;
		  }
		  else
		    *arg_idx = -1;
		}
		stmt_bump = list_next(stmt_bump);
	}
}


/*************************************************************
 * pt_get_pent_type (outer_control,inner_control)
 *
 *   Determine the type of pentagonal loop this is.
 *
 * Inputs: outer_control - outer DO loop AST control node
 *         inner_control - inner DO loop AST control node
 *
 * Returns: pentagonal type (1-4) or NOT_PENT
 *************************************************************
 */
static int
pt_get_pent_type (AST_INDEX outer_control, AST_INDEX inner_control)
{
	AST_INDEX inner1,inner2;
	char *outer_inductvar;
	int coeff,arg_cnt,arg_idx;
	Boolean found1,found2;

	outer_inductvar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
	if (gen_INDUCTIVE_get_rvalue3(outer_control) != AST_NIL)
	  return NOT_PENT;
	if (gen_INDUCTIVE_get_rvalue3(inner_control) != AST_NIL)
	  return NOT_PENT;

	inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
	inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
	found1 = pt_find_var(inner1,outer_inductvar);
	found2 = pt_find_var(inner2,outer_inductvar);

	if (pt_comp_inv_name(inner1,"max") && (!found2)) {
	  pt_get_arg_num(inner1,outer_inductvar,&arg_cnt,&arg_idx,&coeff);
	  if ((arg_cnt == 2) && ((arg_idx == 1) || (arg_idx == 2))) {
	    if (coeff == 1)
	      return PENT_UR;
	    else if (coeff == -1)
	      return PENT_LR;
	  }
	}
	else if (pt_comp_inv_name(inner2,"min") && (!found1)) {
	  pt_get_arg_num(inner2,outer_inductvar,&arg_cnt,&arg_idx,&coeff);
	  if ((arg_cnt == 2) && ((arg_idx == 1) || (arg_idx == 2))) {
	    if (coeff == 1)
	      return PENT_LL;
	    else if (coeff == -1)
	      return PENT_UL;
	  }
	}

	return NOT_PENT;
}


/*************************************************************
 * pt_get_skew_type (outer_control,inner_control)
 *
 *   Determine the type of skewed loop this is.
 *
 * Inputs: outer_control - outer DO loop AST control node
 *         inner_control - inner DO loop AST control node
 *
 * Returns: skewed type (1-2) or NOT_SKEW
 *************************************************************
 */
static int
pt_get_skew_type (AST_INDEX outer_control, AST_INDEX inner_control)
{
        AST_INDEX inner1,inner2;
	char *outer_inductvar;
	Boolean lin1,lin2;
	int coeff1,coeff2;

        outer_inductvar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
        if (gen_INDUCTIVE_get_rvalue3(outer_control) != AST_NIL)
	  return NOT_SKEW;
        if (gen_INDUCTIVE_get_rvalue3(inner_control) != AST_NIL)
	  return NOT_SKEW;

	inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
	inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
	pt_get_coeff(inner1,outer_inductvar,&lin1,&coeff1);
	pt_get_coeff(inner2,outer_inductvar,&lin2,&coeff2);

	if (lin1 && lin2)
	  if (coeff1 == coeff2)
	    if (coeff1 > 0)
	      return SKEW_1;
	    else if (coeff1 < 0)
	      return SKEW_2;

	return NOT_SKEW;
}


/*************************************************************
 * pt_get_hex_type (outer_control,inner_control)
 *
 *   Determine the type of hexagonal loop this is.
 *
 * Inputs: outer_control - outer DO loop AST control node
 *         inner_control - inner DO loop AST control node
 *
 * Returns: hexagonal type (1-4) or NOT_HEX
 *************************************************************
 */
static int
pt_get_hex_type (AST_INDEX outer_control, AST_INDEX inner_control)
{
        AST_INDEX inner1,inner2;
	char *outer_inductvar;
	Boolean found1,found2,lin1,lin2;
	int arg_cnt1,arg_idx1,coeff1;
	int arg_cnt2,arg_idx2,coeff2;
	AST_INDEX node1a,node1b,node2a,node2b,expr1,expr2;
	AST_INDEX fac1,fac2,con1,con2;
	int ifac1,ifac2;

        outer_inductvar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
        if (gen_INDUCTIVE_get_rvalue3(outer_control) != AST_NIL)
	  return NOT_HEX;
        if (gen_INDUCTIVE_get_rvalue3(inner_control) != AST_NIL)
	  return NOT_HEX;

	inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
	inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
	found1 = pt_find_var(inner1,outer_inductvar);
	found2 = pt_find_var(inner2,outer_inductvar);

	if (!found1 || !found2)
	  return NOT_HEX;
	if (!pt_comp_inv_name(inner1,"max") || !pt_comp_inv_name(inner2,"min"))
	  return NOT_HEX;
	pt_get_arg_num(inner1,outer_inductvar,&arg_cnt1,&arg_idx1,&coeff1);
	pt_get_arg_num(inner2,outer_inductvar,&arg_cnt2,&arg_idx2,&coeff2);
	if ((arg_cnt1 != 2) || (arg_cnt2 != 2))
	  return NOT_HEX;
	if ((arg_idx1 < 1) || (arg_idx1 > 2) ||
	    (arg_idx2 < 1) || (arg_idx2 > 2))
	  return NOT_HEX;
	if (coeff1 != coeff2)
	  return NOT_HEX;
	if (coeff1 > 0)
	  return HEX_1_MUL;
	else if (coeff1 < 0)
	  return HEX_2_MUL;

	/* if we get here, then coeff1=coeff2=0 */

	node1a = list_first(gen_INVOCATION_get_actual_arg_LIST(inner1));
	node1b = list_next(node1a);
	node2a = list_first(gen_INVOCATION_get_actual_arg_LIST(inner2));
	node2b = list_next(node2a);
	switch (arg_idx1) {
	    case 1: expr1 = node1a; break;
 	    case 2: expr1 = node1b; break;
	}
	switch (arg_idx2) {
	    case 1: expr2 = node2a; break;
	    case 2: expr2 = node2b; break;
	}
	if (!is_binary_divide(expr1) || !is_binary_divide(expr2))
	  return NOT_HEX;
	pt_fold_term(gen_BINARY_DIVIDE_get_rvalue2(expr1),&node1a,&coeff1);
	pt_fold_term(gen_BINARY_DIVIDE_get_rvalue2(expr2),&node2a,&coeff2);
	if ((node1a != AST_NIL) || (node2a != AST_NIL) || (coeff1 != coeff2))
	  return NOT_HEX;
	pt_separate_linear(gen_BINARY_DIVIDE_get_rvalue1(expr1),
			   outer_inductvar,&lin1,&fac1,&con1);
	pt_separate_linear(gen_BINARY_DIVIDE_get_rvalue1(expr2),
			   outer_inductvar,&lin2,&fac2,&con2);
	if (!lin1 || !lin2)
	  return NOT_HEX;
	pt_fold_term(fac1,&node1a,&ifac1);
	pt_fold_term(fac2,&node2a,&ifac2);
	if ((ifac1 == ifac2) && (node1a == AST_NIL) && (node2a == AST_NIL)) {
	  if (ifac1 == 1)
	    return HEX_1_DIV;
	  else if (ifac1 == -1)
	    return HEX_2_DIV;
	}

	return NOT_HEX;
}


int
pt_process_switch (AST_INDEX node, int nesting_level, Generic *Parm)
{
    InterParm   *parm    = (InterParm *) Parm;
    PedInfo	 ped     = parm->ped;
    DG_Edge	*edgeptr = parm->edgeptr;
    int		 level   = parm->level;
    int newstatus;
    
    if (is_do(node) || is_comment(node) || is_format(node))
    	return (WALK_CONTINUE);

    if ((newstatus = 
	 pt_check_switch (ped, edgeptr, node, level)) 
	== CANNOT_CHANGE)
    {
	parm->status = newstatus;
	return (WALK_ABORT);
    }
    else
	parm->status = pt_get_status(parm->status, newstatus);

    return (WALK_CONTINUE);    
}




int
pt_check_switch (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node, int level)
{
    Boolean	 out_par = true;
    Boolean	 in_par  = true;
    int		 vector, edge;
    Slist	*varinfo;
    
    vector = get_info(ped, node, type_levelv);

    /* If a scalar edge is on a shared variable, then interchange
     * cannot be performed.
     */
    for (edge = dg_first_src_stmt( PED_DG(ped), vector, SCALAR); edge != NIL;
	 edge = dg_next_src_stmt( PED_DG(ped), edge))
    {
	if (check_if_shared( PED_LI(ped), edgeptr[edge].type, 
			     gen_get_text(edgeptr[edge].src), &varinfo))
	    return (CANNOT_CHANGE);
    }

    /* Check the interchange flags for every edge.
     */
    for (edge = dg_first_src_stmt( PED_DG(ped), vector, level); edge != NIL;
	 edge = dg_next_src_stmt( PED_DG(ped),edge))
    {
	if  (check_if_shared( PED_LI(ped), edgeptr[edge].type, 
			     gen_get_text(edgeptr[edge].src), &varinfo))
	{
	    if (edgeptr[edge].ic_preventing)
		return(CANNOT_CHANGE);
	    if (edgeptr[edge].ic_sensitive)
		in_par = false;
	    else out_par = false;
	}
	edgeptr[edge].label = NOT_SET; /* Marking of edges used in switch.c */
    }

    /* Check for edges that move to the outer loop with interchange, 
     * and marking for switching later   
     */
    for (edge = dg_first_src_stmt( PED_DG(ped), vector, level + 1); edge != NIL;
	 edge = dg_next_src_stmt( PED_DG(ped),edge))
    {
	edgeptr[edge].label = NOT_SET;
	if (check_if_shared( PED_LI(ped), edgeptr[edge].type, 
			     gen_get_text(edgeptr[edge].src), &varinfo))
	    out_par = false;
    }
    
    if (in_par)
    	if(out_par)
	    return(BOTH);
	else return(INNER_ONLY);
    else
    	if(out_par)
	    return(OUTER_ONLY);
	else return(NONE);
}

/*************************************************************
 *  pt_canswitch (ped,node,level)
 *
 *    Determines if two nested loops can be interchanged or
 *    not based on dependence graph information.  The return
 *    code determines if interchange is possible and
 *    parallelism status of resulting inner and outer loops.
 *    These are defined in pt.h
 *
 * Inputs: ped   - ped data
 *         node  - the selected do loop AST node
 *         level - loop level
 *************************************************************
 */ 
int
pt_canswitch (PedInfo ped, AST_INDEX node, int level)
{
    InterParm    parm;
    
    parm.ped     = ped;
    parm.level   = level;
    parm.edgeptr = dg_get_edge_structure( PED_DG(ped));
    parm.status  = BOTH;

    walk_statements (node, level, (WK_STMT_CLBACK)pt_process_switch, 
                     (WK_STMT_CLBACK)NOFUNC, (Generic)&parm);
    return (parm.status);
}


int
pt_get_status(int status1, int status2)
{

    if (status1 == BOTH)
    	return(status2);
    if (status2 == BOTH)
    	return(status1);
    if (status1 == status2)
    	return(status1);
    return(NONE);

}


int
pt_can_reorder_stmts (PedInfo ped, AST_INDEX index1, int level1)
{
   AST_INDEX		 sink;
   DG_Edge		*edgeptr;
   int		        edge,vector;
   int                  level;
   int                  index2;
   
   edgeptr = dg_get_edge_structure( PED_DG(ped));
   
   index2 = list_next(index1);

   if ((index1 == AST_NIL) || (index2 == AST_NIL))
      	return(ERROR);
	
   if ((gen_DO_get_lbl_ref(index1) != AST_NIL) ||
       (gen_DO_get_lbl_ref(index2) != AST_NIL))
      	return(ERROR);

   if ((is_assignment(index1) || is_arithmetic_if(index1) || is_call(index1)) &&
       (is_assignment(index2) || is_arithmetic_if(index2) || is_call(index2)))
   {
   	vector = get_info(ped, index1, type_levelv);
   	for (level = LOOP_INDEPENDENT; level <= level1 ; level++)
   	{
      		edge = dg_first_src_stmt( PED_DG(ped),vector,level);
      		while(edge != NIL)
      		{
         		if (edgeptr[edge].type == dg_exit) 
              			return(CANNOT_REORDER);
	 		sink = edgeptr[edge].sink;
	 		if (!is_statement(sink))
	 			sink = out(sink);
         		if ((sink == index2) && (level== LOOP_INDEPENDENT))
              		return(CANNOT_REORDER);
                        edge = dg_next_src_stmt( PED_DG(ped),edge);
      		}

      		edge = dg_first_sink_stmt( PED_DG(ped),vector,level);
      		if (edgeptr[edge].src == index2)
              	return(CANNOT_REORDER);
		 edge = dg_next_sink_stmt( PED_DG(ped),edge);
 
   	}
   	return(CAN_REORDER);
   }
   else
      	return(ERROR);
}
  
AST_INDEX  
pt_reorder_stmts(AST_INDEX index1)
{
    AST_INDEX	index2;

    index2 = list_next(index1);
    index1 = list_remove_node(index1);      
    list_insert_after(index2,index1);
    return index2;
}


/*************************************************************
 * pt_check_inner_control (inner_control,outer_inductvar)
 *
 *   If the outer loop induction variable appears in the any
 *   of the inner loop's control expressions, then return
 *   true, else false.
 *
 * Inputs: inner_control   - AST node of inner control loop
 *         outer_inductvar - variable name to search for
 *
 * Returns: true or false
 *************************************************************
 */
Boolean
pt_check_inner_control(AST_INDEX inner_control, char *outer_inductvar)
{
  return BOOL(pt_find_var(gen_INDUCTIVE_get_rvalue1(inner_control),outer_inductvar) ||
	  pt_find_var(gen_INDUCTIVE_get_rvalue2(inner_control),outer_inductvar) ||
	  pt_find_var(gen_INDUCTIVE_get_rvalue3(inner_control),outer_inductvar));
}


/*************************************************************
 * pt_set_loop_type (ped,loop,loop_type,par_status,sub_type)
 *
 *   Detect the type of loop this is for the purposes
 *   of loop interchange.
 *
 * Inputs:  ped - dependence structure for loop that we
 *                are trying to interchange
 *          node - loop being examined
 *
 * Outputs: loop_type  - structure of loop iteration space
 *          par_status - parallelism status if loops are
 *                       interchanged
 *          sub_type   - subtype of loop structure
 *************************************************************
 */
void
pt_set_loop_type(PedInfo ped, AST_INDEX node, int *loop_type,
                 int *par_status, int *sub_type)
{
	AST_INDEX   temp,next_node,inner_control,outer_control;
	char *outer_inductvar;

	*loop_type = COMPLEX_LOOP;
	*par_status = CANNOT_CHANGE;
	*sub_type = NOT_TRI;

	/* make sure we have perfect nesting */

	if (!is_loop(node))  {
	  *loop_type = BAD_OUTER_LOOP;
	  return;
	}
	
	next_node = list_first(gen_DO_get_stmt_LIST(node));
	
	/* Ignore Comments and format statements, mpal:910720 kats 9/91 
	 */	
	for(  ; is_comment(next_node) || is_format(next_node); 
	    next_node = list_next(next_node) )  ;
	if (!is_do(next_node)) {
	  *loop_type = BAD_INNER_LOOP;
	  return;
	}

	/* Is the end of the loop perfectly nested also?
	 */
	temp = list_next(next_node);
	for(  ;is_comment(temp); temp = list_next(temp) );
	if (temp != AST_NIL)			/* enddo */
	  if (!is_continue(temp)) {
		  *loop_type = BAD_INNER_LOOP;
		  return;
	  }


	/* get control nodes */

	outer_control = gen_DO_get_control(node);
	inner_control = gen_DO_get_control(next_node);
	outer_inductvar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));

	/* check dependences and get parallel status */

	*par_status = pt_canswitch (ped, node, loop_level(node));

	if (*par_status == CANNOT_CHANGE) {
	  *loop_type = BAD_DEP_LOOP;
	  return;
	}

	/* check loop structure */

	if (!pt_check_inner_control(inner_control,outer_inductvar)) 
	  *loop_type = RECT_LOOP;
	else if ((*sub_type=pt_get_tri_type(outer_control,inner_control)) != NOT_TRI)
	  *loop_type = TRI_LOOP;
	else if ((*sub_type=pt_get_trap_type(outer_control,inner_control)) != NOT_TRAP)
	  *loop_type = TRAP_LOOP;
	else if ((*sub_type=pt_get_pent_type(outer_control,inner_control)) != NOT_PENT)
	  *loop_type = PENT_LOOP;
	else if ((*sub_type=pt_get_skew_type(outer_control,inner_control)) != NOT_SKEW)
	  *loop_type = SKEW_LOOP;
	else if ((*sub_type=pt_get_hex_type(outer_control,inner_control)) != NOT_HEX)
	  *loop_type = HEX_LOOP;
}








