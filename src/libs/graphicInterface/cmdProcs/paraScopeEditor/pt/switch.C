/* $Id: switch.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	PEditorCP/pt/switch.c						*/
/*									*/
/*	Parascope Loop Interchange Transformation			*/
/*									*/
/*	These routines routines do the actual loop interchange.		*/
/*									*/
/*	Main entry point: pt_switch_loops()     			*/
/*									*/
/*      Last change: mattb 6/90                                         */
/*                   jass Mar 23, 88                                    */
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

/**************************************************************
 * pt_create_func2 (name,arg1,arg2)
 *
 *   create an AST invocation node of 2 arguments
 *
 * Inputs: name - name of function to create
 *         arg1 - first argument of new function
 *         arg2 - second argument of new function
 *
 * Returns: AST index of new tree
 **************************************************************
 */
AST_INDEX
pt_create_func2(char *name, AST_INDEX arg1, AST_INDEX arg2)
{
    AST_INDEX arg_list,func_node;

    arg_list = list_create(arg1);
    arg_list = list_append(arg_list,list_create(arg2));
    func_node = gen_IDENTIFIER();
    gen_put_text(func_node,name,STR_IDENTIFIER);
    func_node = gen_INVOCATION(func_node,arg_list);

    return func_node;
}



void
pt_get_arg_nodes(AST_INDEX node, char *var, AST_INDEX *arg1, AST_INDEX *arg2)
{
  int arg_cnt,arg_idx,coeff;

  pt_get_arg_num (node,var,&arg_cnt,&arg_idx,&coeff);
  *arg1 = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
  if (arg_idx == 2)
    *arg2 = list_next(*arg1);
  else {
    *arg2 = *arg1;
    *arg1 = list_next(*arg2);
  }
}


/**************************************************************
 * pt_switch_tri (node,tri_type)
 *
 *   This routine finishes processing of triangular loops.
 *   Called from pt_switch_loops() 
 *
 * Inputs: node     - AST node of outer do loop
 *         tri_type - type of triangular loop this is
 **************************************************************
 */
void
pt_switch_tri (AST_INDEX node, int tri_type)
{
  AST_INDEX next_node,inner_control,outer_control;
  AST_INDEX inner1,inner2,outer1,outer2;
  char *innervar,*outervar;

  next_node = list_first(gen_DO_get_stmt_LIST(node));
  outer_control = gen_DO_get_control(node);
  inner_control = gen_DO_get_control(next_node);
  innervar = gen_get_text(gen_INDUCTIVE_get_name(inner_control));
  outervar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
  inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
  inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
  outer1 = gen_INDUCTIVE_get_rvalue1(outer_control);
  outer2 = gen_INDUCTIVE_get_rvalue2(outer_control);
   
  switch (tri_type) {

    case TRI_LL:
      pt_tree_replace(outer2,tree_copy(inner2));
	  pt_tree_replace(inner1,pt_gen_add(pt_gen_ident(outervar),pt_gen_int(1)));
      break;

    case TRI_UR:
	  pt_tree_replace(outer1,tree_copy(inner1));
	  pt_tree_replace(inner2,pt_gen_sub(pt_gen_ident(outervar),pt_gen_int(1)));
      break;

    case TRI_LLD:
	  pt_tree_replace(outer2,tree_copy(inner2));
	  pt_tree_replace(inner1,pt_gen_ident(outervar));
      break;

    case TRI_URD:
	  pt_tree_replace(outer1,tree_copy(inner1));
	  pt_tree_replace(inner2,pt_gen_ident(outervar));
      break;

    case TRI_ULD:
    case TRI_UL:
	  pt_var_replace(outer2,innervar,pt_gen_ident(outervar));
      pt_flip_nodes(inner2,outer2);
      break;

    case TRI_LRD:
    case TRI_LR:
	  pt_var_replace(outer1,innervar,pt_gen_ident(outervar));
      pt_flip_nodes(inner1,outer1);
      break;
  }
}


/**************************************************************
 * pt_switch_trap (node,trap_type)
 *
 *   This routine finishes processing of trapezoidal loops.
 *   Called from pt_switch_loops() 
 *
 * Inputs: node      - AST node of outer do loop
 *         trap_type - type of trapezoidal loop this is
 **************************************************************
 */
void 
pt_switch_trap (AST_INDEX node, int trap_type)
{
  AST_INDEX next_node,inner_control,outer_control,temp_expr;
  AST_INDEX inner1,inner2,outer1,outer2,fac,con;
  char *innervar,*outervar;
  Boolean lin;

  next_node = list_first(gen_DO_get_stmt_LIST(node));
  outer_control = gen_DO_get_control(node);
  inner_control = gen_DO_get_control(next_node);
  innervar = gen_get_text(gen_INDUCTIVE_get_name(inner_control));
  outervar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
  inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
  inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
  outer1 = gen_INDUCTIVE_get_rvalue1(outer_control);
  outer2 = gen_INDUCTIVE_get_rvalue2(outer_control);

  switch (trap_type) {

  case TRAP_UR:
    pt_separate_linear(outer1,innervar,&lin,&fac,&con);
    temp_expr = pt_gen_add(tree_copy(inner1),tree_copy(con));
    tree_replace(outer1,pt_simplify_expr(temp_expr));
    temp_expr = pt_simplify_expr(pt_gen_sub(pt_gen_ident(outervar),con));
    temp_expr = pt_create_func2("min",tree_copy(inner2),temp_expr);
    pt_check_min_max(temp_expr,outervar,outer2);
    pt_tree_replace(inner2,temp_expr);
    tree_free(fac);
    break;

  case TRAP_LR:
    pt_separate_linear(outer1,innervar,&lin,&fac,&con);
    temp_expr = pt_gen_sub(tree_copy(con),tree_copy(inner2));
    tree_replace(outer1,pt_simplify_expr(temp_expr));
    temp_expr = pt_simplify_expr(pt_gen_sub(con,pt_gen_ident(outervar)));
    temp_expr = pt_create_func2("max",tree_copy(inner1),temp_expr);
    pt_check_min_max(temp_expr,outervar,outer2);
    pt_tree_replace(inner1,temp_expr);
    tree_free(fac);
    break;

  case TRAP_LL:
    pt_separate_linear(outer2,innervar,&lin,&fac,&con);
    temp_expr = pt_gen_add(tree_copy(inner2),tree_copy(con));
    tree_replace(outer2,pt_simplify_expr(temp_expr));
    temp_expr = pt_simplify_expr(pt_gen_sub(pt_gen_ident(outervar),con));
    temp_expr = pt_create_func2("max",tree_copy(inner1),temp_expr);
    pt_check_min_max(temp_expr,outervar,outer1);
    pt_tree_replace(inner1,temp_expr);
    tree_free(fac);
    break;

  case TRAP_UL: 
    pt_separate_linear(outer2,innervar,&lin,&fac,&con);
    temp_expr = pt_gen_sub(tree_copy(con),tree_copy(inner1));
    tree_replace(outer2,pt_simplify_expr(temp_expr));
    temp_expr = pt_simplify_expr(pt_gen_sub(con,pt_gen_ident(outervar)));
    temp_expr = pt_create_func2("min",tree_copy(inner2),temp_expr);
    pt_check_min_max(temp_expr,outervar,outer1);
    pt_tree_replace(inner2,temp_expr);
    tree_free(fac);
    break;
  }
}


/**************************************************************
 * pt_switch_pent (node,pent_type)
 *
 *   This routine finishes processing of pentagonal loops.
 *   Called from pt_switch_loops() 
 *
 * Inputs: node      - AST node of outer do loop
 *         pent_type - type of pentagonal loop this is
 **************************************************************
 */
void 
pt_switch_pent (AST_INDEX node, int pent_type)
{
  AST_INDEX next_node,inner_control,outer_control,outer1,outer2;
  AST_INDEX temp_expr,old_expr,expr1,expr2;
  char *innervar,*outervar;

  next_node = list_first(gen_DO_get_stmt_LIST(node));
  outer_control = gen_DO_get_control(node);
  inner_control = gen_DO_get_control(next_node);
  innervar = gen_get_text(gen_INDUCTIVE_get_name(inner_control));
  outervar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
  outer1 = gen_INDUCTIVE_get_rvalue1(outer_control);
  outer2 = gen_INDUCTIVE_get_rvalue2(outer_control);

  switch (pent_type) {

  case PENT_LL:
    pt_get_arg_nodes(outer2,innervar,&expr1,&expr2);
    temp_expr = tree_copy(expr2);
    pt_tree_replace (outer2,tree_copy(expr1));
    pt_zero_var(temp_expr,innervar);
    temp_expr = pt_simplify_expr(pt_gen_sub(pt_gen_ident(outervar),temp_expr));
    old_expr = gen_INDUCTIVE_get_rvalue1(inner_control);
    temp_expr = pt_create_func2("max",tree_copy(old_expr),temp_expr);
    pt_check_min_max(temp_expr,outervar,outer1);
    pt_tree_replace(old_expr,temp_expr);
    break;

  case PENT_UR:
    pt_get_arg_nodes(outer1,innervar,&expr1,&expr2);
    temp_expr = tree_copy(expr2);
    pt_tree_replace (outer1,tree_copy(expr1));
    pt_zero_var(temp_expr,innervar);
    temp_expr = pt_simplify_expr(pt_gen_sub(pt_gen_ident(outervar),temp_expr));
    old_expr = gen_INDUCTIVE_get_rvalue2(inner_control);
    temp_expr = pt_create_func2("min",tree_copy(old_expr),temp_expr);
    pt_check_min_max(temp_expr,outervar,outer2);
    pt_tree_replace(old_expr,temp_expr);
    break;

  case PENT_UL:
    pt_get_arg_nodes(outer2,innervar,&expr1,&expr2);
    pt_var_replace(expr2,innervar,pt_gen_ident(outervar));
    pt_flip_nodes(expr1,gen_INDUCTIVE_get_rvalue2(inner_control));
    pt_flip_nodes(outer2,gen_INDUCTIVE_get_rvalue2(inner_control));
    pt_check_min_max(gen_INDUCTIVE_get_rvalue2(inner_control),outervar,outer1);
    break;

  case PENT_LR:
    pt_get_arg_nodes(outer1,innervar,&expr1,&expr2);
    pt_var_replace(expr2,innervar,pt_gen_ident(outervar));
    pt_flip_nodes(expr1,gen_INDUCTIVE_get_rvalue1(inner_control));
    pt_flip_nodes(outer1,gen_INDUCTIVE_get_rvalue1(inner_control));
    pt_check_min_max(gen_INDUCTIVE_get_rvalue1(inner_control),outervar,outer2);
    break;
  }
}


/**************************************************************
 * pt_switch_skew (node,skew_type)
 *
 *   This routine finishes processing of skewed loops.
 *   Called from pt_switch_loops() 
 *
 * Inputs: node      - AST node of outer do loop
 *         skew_type - type of skewed loop this is
 **************************************************************
 */
void 
pt_switch_skew (AST_INDEX node, int skew_type)
{
  AST_INDEX next_node,inner_control,outer_control;
  char *innervar,*outervar;
  AST_INDEX fac1,fac2,con1,con2,inner1,inner2,outer1,outer2,temp;
  Boolean lin1,lin2;

  next_node = list_first(gen_DO_get_stmt_LIST(node));
  outer_control = gen_DO_get_control(node);
  inner_control = gen_DO_get_control(next_node);
  innervar = gen_get_text(gen_INDUCTIVE_get_name(inner_control));
  outervar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
  inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
  inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
  outer1 = gen_INDUCTIVE_get_rvalue1(outer_control);
  outer2 = gen_INDUCTIVE_get_rvalue2(outer_control);

  switch (skew_type) {

    case SKEW_1:
      pt_separate_linear(outer1,innervar,&lin1,&fac1,&con1);
      pt_separate_linear(outer2,innervar,&lin2,&fac2,&con2);

      /* fix up outer loop bounds */
      temp = pt_gen_mul(tree_copy(fac1),tree_copy(inner1));
      temp = pt_gen_add(tree_copy(con1),temp);
      pt_tree_replace(outer1,pt_simplify_expr(temp));
      temp = pt_gen_mul(tree_copy(fac2),tree_copy(inner2));
      temp = pt_gen_add(tree_copy(con2),temp);
      pt_tree_replace(outer2,pt_simplify_expr(temp));

      /* fix up inner loop bounds */
      temp = pt_gen_sub(pt_gen_ident(outervar),con2);
      temp = pt_gen_add(temp,tree_copy(fac2));
      temp = pt_gen_sub(temp,pt_gen_int(1));
      temp = pt_simplify_expr(gen_BINARY_DIVIDE(temp,fac2));
      temp = pt_create_func2("max",tree_copy(inner1),temp);
      pt_check_min_max(temp,outervar,gen_INDUCTIVE_get_rvalue1(outer_control));
      pt_tree_replace(inner1,temp);
      temp = pt_gen_sub(pt_gen_ident(outervar),con1);
      temp = pt_simplify_expr(gen_BINARY_DIVIDE(temp,fac1));
      temp = pt_create_func2("min",tree_copy(inner2),temp);
      pt_check_min_max(temp,outervar,gen_INDUCTIVE_get_rvalue2(outer_control));
      pt_tree_replace(inner2,temp);
      break;

    case SKEW_2:
      pt_separate_linear(outer1,innervar,&lin1,&fac1,&con1);
      pt_separate_linear(outer2,innervar,&lin2,&fac2,&con2);
      fac1 = pt_simplify_expr(gen_UNARY_MINUS(fac1));
      fac2 = pt_simplify_expr(gen_UNARY_MINUS(fac2));

      /* fix up outer loop bounds */
      temp = pt_gen_mul(tree_copy(fac1),tree_copy(inner2));
      temp = pt_gen_sub(tree_copy(con1),temp);
      pt_tree_replace(outer1,pt_simplify_expr(temp));
      temp = pt_gen_mul(tree_copy(fac2),tree_copy(inner1));
      temp = pt_gen_sub(tree_copy(con2),temp);
      pt_tree_replace(outer2,pt_simplify_expr(temp));

      /* fix up inner loop bounds */
      temp = pt_gen_sub(con1,pt_gen_ident(outervar));
      temp = pt_gen_add(temp,tree_copy(fac1));
      temp = pt_gen_sub(temp,pt_gen_int(1));
      temp = pt_simplify_expr(gen_BINARY_DIVIDE(temp,fac1));
      temp = pt_create_func2("max",tree_copy(inner1),temp);
      pt_check_min_max(temp,outervar,gen_INDUCTIVE_get_rvalue2(outer_control));
      pt_tree_replace(inner1,temp);
      temp = pt_gen_sub(con2,pt_gen_ident(outervar));
      temp = pt_simplify_expr(gen_BINARY_DIVIDE(temp,fac2));
      temp = pt_create_func2("min",tree_copy(inner2),temp);
      pt_check_min_max(temp,outervar,gen_INDUCTIVE_get_rvalue1(outer_control));
      pt_tree_replace(inner2,temp);
      break;
  }
}


/**************************************************************
 * pt_switch_hex (node,hex_type)
 *
 *   This routine finishes processing of hexagonal loops.
 *   Called from pt_switch_loops() 
 *
 * Inputs: node     - AST node of outer do loop
 *         hex_type - type of hexagonal loop this is
 **************************************************************
 */
void 
pt_switch_hex (AST_INDEX node, int hex_type)
{
  AST_INDEX next_node,inner_control,outer_control;
  AST_INDEX inner1,inner2,outer1,outer2,temp1,temp2,temp_node;
  AST_INDEX node1a,node1b,node1c,node2a,node2b,node2c;
  AST_INDEX fac1,con1,fac2,con2;
  char *innervar,*outervar;
  Boolean lin1,lin2;

  next_node = list_first(gen_DO_get_stmt_LIST(node));
  outer_control = gen_DO_get_control(node);
  inner_control = gen_DO_get_control(next_node);
  innervar = gen_get_text(gen_INDUCTIVE_get_name(inner_control));
  outervar = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
  inner1 = gen_INDUCTIVE_get_rvalue1(inner_control);
  inner2 = gen_INDUCTIVE_get_rvalue2(inner_control);
  outer1 = gen_INDUCTIVE_get_rvalue1(outer_control);
  outer2 = gen_INDUCTIVE_get_rvalue2(outer_control);

  pt_get_arg_nodes(outer1,innervar,&node1a,&node1b);
  pt_get_arg_nodes(outer2,innervar,&node2a,&node2b);
  temp1 = tree_copy(inner1);
  temp2 = tree_copy(inner2);

  switch (hex_type) {

    case HEX_1_MUL:
      pt_separate_linear(node1b,innervar,&lin1,&fac1,&con1);
      pt_separate_linear(node2b,innervar,&lin2,&fac2,&con2);

      /* fix inner bounds */
      temp_node = pt_gen_sub(pt_gen_ident(outervar),con2);
      temp_node = pt_gen_add(temp_node,tree_copy(fac2));
      temp_node = pt_gen_sub(temp_node,pt_gen_int(1));
      temp_node = pt_simplify_expr(gen_BINARY_DIVIDE(temp_node,fac2));
      temp_node = pt_create_func2("max",temp1,temp_node);
      pt_check_min_max(temp_node,outervar,node1a);
      pt_tree_replace(inner1,temp_node);
      temp_node = pt_gen_sub(pt_gen_ident(outervar),con1);
      temp_node = pt_simplify_expr(gen_BINARY_DIVIDE(temp_node,fac1));
      temp_node = pt_create_func2("min",temp2,temp_node);
      pt_check_min_max(temp_node,outervar,node2a);
      pt_tree_replace(inner2,temp_node);

      /* fix outer bounds */
      pt_tree_replace(outer1,tree_copy(node1a));
      pt_tree_replace(outer2,tree_copy(node2a));
      break;

    case HEX_1_DIV:
      node1c = gen_BINARY_DIVIDE_get_rvalue2(node1b);
      node1b = gen_BINARY_DIVIDE_get_rvalue1(node1b);
      node2c = gen_BINARY_DIVIDE_get_rvalue2(node2b);
      node2b = gen_BINARY_DIVIDE_get_rvalue1(node2b);
      pt_separate_linear(node1b,innervar,&lin1,&fac1,&con1);
      pt_separate_linear(node2b,innervar,&lin2,&fac2,&con2);

      /* fix inner bounds */
      temp_node = pt_gen_mul(pt_gen_ident(outervar),tree_copy(node1c));
      temp_node = pt_simplify_expr(pt_gen_sub(temp_node,con2));
      temp_node = pt_create_func2("max",temp1,temp_node);
      pt_check_min_max(temp_node,outervar,node1a);
      pt_tree_replace(inner1,temp_node);
      temp_node = pt_gen_mul(pt_gen_ident(outervar),tree_copy(node2c));
      temp_node = pt_gen_sub(temp_node,con1);
      temp_node = pt_gen_add(temp_node,tree_copy(node2c));
      temp_node = pt_simplify_expr(pt_gen_sub(temp_node,pt_gen_int(1)));
      temp_node = pt_create_func2("min",temp2,temp_node);
      pt_check_min_max(temp_node,outervar,node2a);
      pt_tree_replace(inner2,temp_node);

      /* Fix outer bounds */
      pt_tree_replace(outer1,tree_copy(node1a));
      pt_tree_replace(outer2,tree_copy(node2a));

      tree_free(fac1);
      tree_free(fac2);
      break;

    case HEX_2_MUL:
      pt_separate_linear(node1b,innervar,&lin1,&fac1,&con1);
      pt_separate_linear(node2b,innervar,&lin2,&fac2,&con2);
      fac1 = pt_simplify_expr(gen_UNARY_MINUS(fac1));
      fac2 = pt_simplify_expr(gen_UNARY_MINUS(fac2));

      /* fix inner bounds */
      temp_node = pt_gen_sub(con1,pt_gen_ident(outervar));
      temp_node = pt_gen_add(temp_node,tree_copy(fac1));
      temp_node = pt_gen_sub(temp_node,pt_gen_int(1));
      temp_node = pt_simplify_expr(gen_BINARY_DIVIDE(temp_node,fac1));
      temp_node = pt_create_func2("max",temp1,temp_node);
      pt_check_min_max(temp_node,outervar,node2a);
      pt_tree_replace(inner1,temp_node);
      temp_node = pt_gen_sub(con2,pt_gen_ident(outervar));
      temp_node = pt_simplify_expr(gen_BINARY_DIVIDE(temp_node,fac2));
      temp_node = pt_create_func2("min",temp2,temp_node);
      pt_check_min_max(temp_node,outervar,node1a);
      pt_tree_replace(inner2,temp_node);

      /* fix outer bounds */
      pt_tree_replace(outer1,tree_copy(node1a));
      pt_tree_replace(outer2,tree_copy(node2a));
      break;

    case HEX_2_DIV:
      node1c = gen_BINARY_DIVIDE_get_rvalue2(node1b);
      node1b = gen_BINARY_DIVIDE_get_rvalue1(node1b);
      node2c = gen_BINARY_DIVIDE_get_rvalue2(node2b);
      node2b = gen_BINARY_DIVIDE_get_rvalue1(node2b);
      pt_separate_linear(node1b,innervar,&lin1,&fac1,&con1);
      pt_separate_linear(node2b,innervar,&lin2,&fac2,&con2);

      /* fix inner bounds */
      temp_node = pt_gen_mul(pt_gen_ident(outervar),tree_copy(node1c));
      temp_node = pt_gen_sub(con1,temp_node);
      temp_node = pt_gen_sub(temp_node,tree_copy(node1c));
      temp_node = pt_simplify_expr(pt_gen_add(temp_node,pt_gen_int(1)));
      temp_node = pt_create_func2("max",temp1,temp_node);
      pt_check_min_max(temp_node,outervar,node2a);
      pt_tree_replace(inner1,temp_node);
      temp_node = pt_gen_mul(pt_gen_ident(outervar),tree_copy(node2c));
      temp_node = pt_simplify_expr(pt_gen_sub(con2,temp_node));
      temp_node = pt_create_func2("min",temp2,temp_node);
      pt_check_min_max(temp_node,outervar,node1a);
      pt_tree_replace(inner2,temp_node);

      /* Fix outer bounds */
      pt_tree_replace(outer1,tree_copy(node1a));
      pt_tree_replace(outer2,tree_copy(node2a));

      tree_free(fac1);
      tree_free(fac2);
      break;
  }
}


/**************************************************************
 * pt_adjust_deps (ped, node, level)
 *
 *   Routines to update the dependence graph to reflect 
 *   interchanged loops.
 *
 * Inputs: ped   - pointer to dependence structure
 *         node  - AST node of outer loop
 *         level -
 **************************************************************
 */  
void
pt_adjust_deps (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node, int level)
{
   int 		vector;
   int		edge;
   int		next_edge;
   
    vector = get_info(ped, node, type_levelv);

    if (level > 1)
    {
    	level --;
	/*
	 *   Mark all dependences at this level preventing
	 */
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,level); edge != NIL;
	     edge = dg_next_src_stmt( PED_DG(ped),edge))
	{
	    edgeptr[edge].ic_preventing = true;
	}

	level++;
    }
    /* Do nothing to insensitive edges. Delete and reinsert the sensitive
       edges at level level +1. Mark them as preventing. 
    */
     
    for (edge = dg_first_src_stmt( PED_DG(ped), vector,level); edge != NIL;
	 edge = next_edge)
    {
       next_edge = dg_next_src_stmt( PED_DG(ped),edge);   /*  Fix jss May, 89 */
       if (edgeptr[edge].ic_sensitive)
       {
          edgeptr[edge].ic_preventing = true;
	  dg_delete_edge( PED_DG(ped), edge);
	  edgeptr[edge].level = level + 1;
	  edgeptr[edge].label = SET;
          dg_add_edge( PED_DG(ped), edge);
	}
    }
    level++;
    
    /* Move all edges to level - 1. Mark them as sensitive and not
     *  preventing
     */
    for (edge = dg_first_src_stmt( PED_DG(ped), vector, level); edge != NIL;
	 edge = next_edge)
    {
       next_edge = dg_next_src_stmt( PED_DG(ped),edge);
       if (edgeptr[edge].label == NOT_SET) 
       /* We dont want to consider edges that we just added in last step*/
       {
          dg_delete_edge( PED_DG(ped), edge);
          edgeptr[edge].level = level -1;    
          edgeptr[edge].ic_preventing = false;
          edgeptr[edge].ic_sensitive  = true;
          dg_add_edge( PED_DG(ped), edge);
        }
	else  
	    edgeptr[edge].label = NOT_SET;    
     }
}


/**************************************************************
 * pt_adjust_stmt (ped, edgeptr, node, level)
 * pt_adjust_loop (ped, edgeptr, node, level)
 * pt_adjust_graph (ped, node, level)
 *
 *   The next 3 routines step through all statements
 *   contained within the selected do loop, calling
 *   pt_adjust_deps() for each statement.
 *
 * Inputs: ped     - pointer to dependence structure
 *         edgeptr -
 *         node    - AST node of outer loop
 *         level   -
 **************************************************************
 */  

void pt_adjust_loop();

void
pt_adjust_stmt (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node, int level)
{
    if (is_do(node) || is_parallelloop(node))
    {
    	pt_adjust_loop(ped, edgeptr, node, level);
    }
    else
    	pt_adjust_deps (ped, edgeptr, node, level);
}


void
pt_adjust_loop (PedInfo ped, DG_Edge *edgeptr, AST_INDEX node, int level)
{
    node = list_first(gen_DO_get_stmt_LIST(node));
    while (node!= AST_NIL)
    {
    	pt_adjust_stmt(ped,edgeptr,node,level);
	node = list_next(node);
    }
}


void
pt_adjust_graph (PedInfo ped, AST_INDEX node, int level)
{
        DG_Edge	     *edgeptr;
	
	edgeptr = dg_get_edge_structure( PED_DG(ped));

	/* Correct data dependences, control deps are correct */
        pt_adjust_loop(ped, edgeptr, node, level);
}


/**************************************************************
 * pt_switch_loops (ped,level,loop_type,tri_type)
 *
 *   This is the general purpose loop-interchange routine.
 *
 * Inputs: ped       - pointer to ped structure
 *         node      - loop being interchanged
 *         loop_type - the type of loop this is
 *         tri_type  - the subtype of loop this is
 **************************************************************
 */
void
pt_switch_loops (PedInfo ped, AST_INDEX node, int loop_type, int tri_type)
{
   int		level; /* level of loop */

   AST_INDEX       outer_do;
   AST_INDEX       inner_do;

   AST_INDEX       outer_son;
   AST_INDEX       inner_son;
   
   outer_do = node;
   level = loop_level(node);

   for( inner_do = list_first(gen_DO_get_stmt_LIST(node));
       is_comment(inner_do);		/* Ignore Comments, mpal:910720 */
       inner_do = list_next(inner_do) ) ;
   /*
    *  Switch some fields of the two nodes.
    */    

    outer_son = gen_DO_get_control(outer_do);
    inner_son = gen_DO_get_control(inner_do);
    pt_flip_nodes(outer_son, inner_son);
    
    el_flip_private( PED_LI(ped), outer_do, inner_do);
    
    pt_adjust_graph(ped, node, level); 

   switch (loop_type) {
   case RECT_LOOP:
     break;
   case TRI_LOOP:
     pt_switch_tri(node,tri_type);
     break;
   case TRAP_LOOP:
     pt_switch_trap(node,tri_type);
     break;
   case PENT_LOOP:
     pt_switch_pent(node,tri_type);
     break;
   case SKEW_LOOP:
     pt_switch_skew(node,tri_type);
     break;
   case HEX_LOOP:
     pt_switch_hex(node,tri_type);
     break;
   default:
     break;
   }
}
