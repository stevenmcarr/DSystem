/* $Id: strip.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************************************************/
/*                                                                 */
/* strip.c - Code to do strip mining                                 */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h> 

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>


Boolean
pt_can_strip(PedInfo ped)
  //PedInfo ped;
{
  return true;
}



/*******************************************************
 * pt_strip_mine(ped,loop,stepstr)
 *
 *  Adjusts the step size of the selected loop, loop, to be step.
 *  Insert an inner loop to iterate over the portions of the iteration
 *  space.
 *
 *  Converts:     do i = lo_b, up_b
 *                  ...body...
 *
 *  To:           do i$ = lo_b, up_b, step
 *                  do i = i$, min(i$+step-1, up_b)
 *                     ...body...
 *
 *
 * Inputs:ped - The dependence abstraction
 *        loop- the loop
 *        step - the step size of the loop
 *
 * Outputs: none
 *******************************************************
 */
void
pt_strip_mine(PedInfo ped, AST_INDEX loop, int step, char *stepstr)
//   PedInfo ped;
//   AST_INDEX loop;
//   int step;           /* integer value of step, 0 if step passed as str */
//   char *stepstr;     /* step passed as string */
{

  AST_INDEX slist; /* statement list */
  AST_INDEX control;    /* loop control */
  AST_INDEX up_bound;   /* upper bound */
  AST_INDEX newloop;    /* new loop */
  AST_INDEX newup; /* new bounds */
  AST_INDEX newivar;    /* new induction variable node */
  AST_INDEX cont;  /* continue */
  AST_INDEX stepast;
  AST_INDEX oldivar;
  AST_INDEX oldstep_ast;
  int oldstep;

  char *ivar;      /* induction variable */
  char buf[80];
  int level;
  int lo_b, up_b;

  EDGE_INDEX edge; /* access to dependence edges */
  DG_Edge *edgeptr;
  Stack estack;

/* if the DO is matched with a labeled continue, rather than a ENDDO */
  Boolean contnue = false;

  /*----------------------------------------------------------*/
  /* get original loop information [do i = lo_b, up_b, s] */

  slist = gen_DO_get_stmt_LIST(loop);
  control = gen_DO_get_control(loop);
  oldivar = gen_INDUCTIVE_get_name(control);
  ivar = gen_get_text(oldivar);
  up_bound = gen_INDUCTIVE_get_rvalue2(control);
  oldstep_ast = gen_INDUCTIVE_get_rvalue3(control);
  if (gen_DO_get_lbl_ref(loop) != AST_NIL)
    contnue = true;
  tree_replace(slist, AST_NIL);

  if (oldstep_ast == AST_NIL)
    oldstep = 1;
  else if (ast_eval(oldstep_ast, &oldstep))
    oldstep = -1;

  /*-------------------------------------------------------------------*/
  /* create new loop header [do i = i$, min(i$+(step-1)*s,up_b), s]    */

  strcpy(buf, ivar);
  strcat(buf, "$");
  newivar = pt_gen_ident(buf);

  if (!step)
  {
    step = isdigit(*stepstr) ? atoi(stepstr) : 0;
    stepast = step ? pt_gen_const(stepstr) : pt_gen_ident(stepstr);
  }
  else
  {
    stepast = pt_gen_int(step);
  }

  /* if cannot determine constant upper bound at compile time,   */
  /* make the upper bound = min (ivar$ + step - 1, up_bound) */

  if (!step || (oldstep != 1) || ast_eval(up_bound, &up_b) ||
      ast_eval(gen_INDUCTIVE_get_rvalue1(control), &lo_b) ||
      ((1 + up_b - lo_b) % step))
  {
    newup = pt_gen_sub(tree_copy(stepast), pt_gen_int(1));
    if (oldstep != 1)
    {
      newup = pt_gen_mul(newup, tree_copy(oldstep_ast));
      gen_put_parens(newup, 1);
    }
    newup = pt_gen_add(tree_copy(newivar), newup);
    newup = pt_simplify_expr(newup);
    newup = pt_create_func2("min", newup, tree_copy(up_bound));
  }
  else
  {
    newup = pt_gen_add(tree_copy(newivar),
                       pt_gen_int((step - 1) * oldstep));
  }

  /* slam them all in */
  newloop = gen_INDUCTIVE(tree_copy(oldivar),
                          tree_copy(newivar), newup, tree_copy(oldstep_ast));
  newloop = gen_DO(AST_NIL, AST_NIL, AST_NIL, newloop, slist);
  gen_DO_put_stmt_LIST(loop, list_create(newloop));

  /*----------------------------------------------------------------*/
  /* update original loop header [do i$ = lo_b, up_b, s * step]     */

  if (oldstep != 1)
  {
    if (step && (oldstep != -1))  /* both step & old step are constants */
      stepast = pt_gen_int(step * oldstep);
    else
      stepast = pt_gen_mul(tree_copy(oldstep_ast), stepast);
  }
  gen_INDUCTIVE_put_rvalue3(control, stepast);
  gen_INDUCTIVE_put_name(control, newivar);

  /* correct the statement lists for loops with labeled continues */
  if (contnue)
  {
    cont = list_last(slist);
    tree_replace(cont, AST_NIL);
    list_insert_after(newloop, cont);
  }

  /* update the meta_type */
  ast_put_meta_type(newloop, ast_get_meta_type(loop));

  /* realign the loops */
  level = loop_level(loop);
  el_add_loop(PED_LI(ped), loop, newloop, level + 1);

  /* correct the shared and private variable lists */
  el_copy_shared_list(PED_LI(ped), loop, newloop);
  el_copy_private_list(PED_LI(ped), loop, newloop);
  el_change_private_var_name(PED_LI(ped), newloop, ivar, ssave(buf));
  el_add_private_up(PED_LI(ped), loop, ssave(buf));

  /* move the dependences to a deeper level */
  estack = stack_create(sizeof(EDGE_INDEX));
  edgeptr = dg_get_edge_structure(PED_DG(ped));
  pt_copy_dep_deeper(ped, edgeptr, estack, newloop, level);
  while (stack_pop(estack, &edge))
  {
    dg_add_edge(PED_DG(ped), edge);
  }
  stack_destroy(estack);
}


void
pt_copy_dep_deeper(PedInfo ped, DG_Edge *edgeptr, Stack estack, AST_INDEX newloop, 
		   int level)
//   PedInfo ped;
//   DG_Edge *edgeptr;
//   Stack estack;
//   AST_INDEX newloop;
//   int level;
{
  AST_INDEX node;
  EDGE_INDEX edge;
  EDGE_INDEX newedge;
  int vector;
  int max;
  int l;

  for (node = list_first(gen_DO_get_stmt_LIST(newloop));
       node != AST_NIL;
       node = list_next(node))
  {
    if (is_do(node) || is_parallelloop(node))
      pt_copy_dep_deeper(ped, edgeptr, estack, node, level);
    else
    {
      vector = get_info(ped, node, type_levelv);
      max = dg_length_level_vector(PED_DG(ped), vector);
      for (l = max; l > level; l--)
      {
        for (edge = dg_first_src_stmt(PED_DG(ped), vector, l);
             edge != NIL;
             edge = dg_first_src_stmt(PED_DG(ped), vector, l))
        {
          dg_delete_edge(PED_DG(ped), edge);
          edgeptr[edge].level = edgeptr[edge].level + 1;
          stack_push(estack, &edge);
        }
      }
      l = level;
      for (edge = dg_first_src_stmt(PED_DG(ped), vector, l);
           edge != NIL;
           edge = dg_next_src_stmt(PED_DG(ped), edge))
      {
        newedge = dg_alloc_edge(PED_DG(ped), &edgeptr);
        el_copy_edge(PED_DG(ped), PED_DT_INFO(ped), edgeptr, edge, newedge);
        edgeptr[newedge].level = edgeptr[edge].level + 1;
        stack_push(estack, &newedge);
      }
    }
  }
}
