/* $Id: bound.C,v 1.9 1997/03/27 20:29:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


/****************************************************************************/
/*                                                                          */
/*    File:  bound.C                                                        */
/*                                                                          */
/*    Description:  Contains function for update loop bounds after unrolling*/
/*                                                                          */
/****************************************************************************/
  
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/include/bound.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/ast/forttypes.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>

static int set_type(AST_INDEX node,
		    Generic   dummy)

  {
   gen_put_real_type(node,TYPE_INTEGER);
   gen_put_converted_type(node,TYPE_INTEGER);
   return(WALK_CONTINUE);
  }

static AST_INDEX make_integer_type(AST_INDEX expr)

  {
   walk_expression(expr,(WK_EXPR_CLBACK)set_type,(WK_EXPR_CLBACK)NOFUNC,(Generic)NULL);
   return expr;
  }

/****************************************************************************/
/*                                                                          */
/*    Function:  ut_update_bounds                                           */
/*                                                                          */
/*    Input:     loop - AST index of original loop                          */
/*               copy - AST index of pre_loop                               */
/*               val - unroll amount                                        */
/*                                                                          */
/*    Description:  Update the loop bounds of the original and pre-loop     */
/*                  after unrolling or unroll-and-jam.                      */
/*                                                                          */
/****************************************************************************/
  

void ut_update_bounds(AST_INDEX loop,
		      AST_INDEX copy,
		      int       val)
  {
   AST_INDEX step,control,mod,
             lwb,upb,upb_c,ph1,ph2,pstep;
   int       step_v;

     control = gen_DO_get_control(loop);
     step = gen_INDUCTIVE_get_rvalue3(control);
     if (step == AST_NIL)
       {
        step = pt_gen_int(1);
	gen_INDUCTIVE_put_rvalue3(control,step);
       }
     if (copy == AST_NIL)
       {
	ph1 = gen_PLACE_HOLDER();
	tree_replace(step,ph1);
	pt_tree_replace(ph1,pt_simplify_expr(pt_gen_mul(step,
							pt_gen_int(val+1))));
       }
     else
       {
	 /*  n$ = mod(up - lo + 1, step * (val + 1))
	     
	     do i = lo, lo + ((n$ -1) * step), step

	     do i = lo + n$ * step, up, step * (val + 1)
	 */
	lwb = gen_INDUCTIVE_get_rvalue1(control);
	upb = gen_INDUCTIVE_get_rvalue2(control);

	ph1 = gen_PLACE_HOLDER();
	tree_replace(lwb,ph1);
	ph2 = gen_PLACE_HOLDER();
	tree_replace(step,ph2);

	control = gen_DO_get_control(copy);
	upb_c = gen_INDUCTIVE_get_rvalue2(control);
	if (pt_eval(step,&step_v))
	  die_with_message("ut_update_bounds: symbolic step,shouldn't be here");
	if (step_v == 1)
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_add(
                                                pt_gen_sub(tree_copy_with_type(upb),
							   tree_copy_with_type(lwb)),
							 pt_gen_int(1))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(step_v),
						       pt_gen_int(val+1)))));
	else if (step_v == -1)
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_add(
                                                pt_gen_sub(tree_copy_with_type(lwb),
							   tree_copy_with_type(upb)),
							 pt_gen_int(1))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(-step_v),
						       pt_gen_int(val+1)))));
	else if (step_v > 0)
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_div(pt_gen_add(
                                                           pt_gen_sub(
								tree_copy_with_type(upb),
							       tree_copy_with_type(lwb)),
							   pt_gen_int(1)),
							 tree_copy_with_type(step))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(step_v),
						       pt_gen_int(val+1)))));
	else 
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_div(pt_gen_sub(
                                                          pt_gen_sub(
								tree_copy_with_type(upb),
							       tree_copy_with_type(lwb)),
							   pt_gen_int(1)),
							 tree_copy_with_type(step))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(-step_v),
						       pt_gen_int(val+1)))));
	pt_tree_replace(upb_c,make_integer_type(pt_simplify_expr(
                              pt_gen_add(pt_gen_mul(tree_copy_with_type(mod),
						    tree_copy_with_type(step)),
					 pt_gen_sub(tree_copy_with_type(lwb),
						    tree_copy_with_type(step))))));
	pt_tree_replace(ph1,make_integer_type(pt_gen_add(
                              pt_simplify_expr(pt_gen_mul(tree_copy_with_type(mod),
							  tree_copy_with_type(step))),
			      lwb)));
	pt_tree_replace(ph2,make_integer_type(pt_simplify_expr(pt_gen_mul(tree_copy_with_type(step),
							 pt_gen_int(val+1)))));
       }
  }


void ut_update_bounds_post(AST_INDEX loop,
			   AST_INDEX copy,
			   int       val)
  {
   AST_INDEX step,control,mod,
             lwb,upb,lwb_copy,ph1,ph2,pstep;
   int       step_v;

     control = gen_DO_get_control(loop);
     step = gen_INDUCTIVE_get_rvalue3(control);
     if (step == AST_NIL)
       {
        step = pt_gen_int(1);
	gen_INDUCTIVE_put_rvalue3(control,step);
       }
     if (copy == AST_NIL)
       {
	ph1 = gen_PLACE_HOLDER();
	tree_replace(step,ph1);
	pt_tree_replace(ph1,pt_simplify_expr(pt_gen_mul(step,
							pt_gen_int(val+1))));
       }
     else
       {
	 /*  n$ = mod(up - lo + 1, step * (val + 1))
	     
	     do i = lo, up - (n$ * step),  step * (val + 1)

	     do i = up - ((n$-1) * step, up, step
	 */
	lwb = gen_INDUCTIVE_get_rvalue1(control);
	upb = gen_INDUCTIVE_get_rvalue2(control);

	control = gen_DO_get_control(copy);
	lwb_copy = gen_INDUCTIVE_get_rvalue1(control);
	if (pt_eval(step,&step_v))
	  die_with_message("ut_update_bounds: symbolic step,shouldn't be here");
	if (step_v == 1)
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_add(
                                                pt_gen_sub(tree_copy_with_type(upb),
							   tree_copy_with_type(lwb)),
							 pt_gen_int(1))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(step_v),
						       pt_gen_int(val+1)))));
	else if (step_v == -1)
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_add(
                                                pt_gen_sub(tree_copy_with_type(lwb),
							   tree_copy_with_type(upb)),
							 pt_gen_int(1))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(-step_v),
						       pt_gen_int(val+1)))));
	else if (step_v > 0)
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_div(pt_gen_add(
                                                           pt_gen_sub(
								tree_copy_with_type(upb),
							       tree_copy_with_type(lwb)),
							   pt_gen_int(1)),
							 tree_copy_with_type(step))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(step_v),
						       pt_gen_int(val+1)))));
	else 
	  mod = make_integer_type(pt_gen_mod(pt_simplify_expr(pt_gen_div(pt_gen_sub(
                                                          pt_gen_sub(
								tree_copy_with_type(upb),
							       tree_copy_with_type(lwb)),
							   pt_gen_int(1)),
							 tree_copy_with_type(step))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(-step_v),
						       pt_gen_int(val+1)))));
	pt_tree_replace(lwb_copy,make_integer_type(pt_simplify_expr(
                                 pt_gen_sub(
				    tree_copy_with_type(upb),
                                    pt_gen_mul(pt_gen_sub(tree_copy_with_type(mod),
							  pt_gen_int(1)),
					       tree_copy_with_type(step))))));
	pt_tree_replace(upb,make_integer_type(pt_simplify_expr(
                              pt_gen_sub(tree_copy_with_type(upb),
					 pt_gen_mul(tree_copy_with_type(mod),
						    tree_copy_with_type(step))))));
	pt_tree_replace(step,make_integer_type(pt_simplify_expr(pt_gen_mul
						           (tree_copy_with_type(step),
							 pt_gen_int(val+1)))));
       }
  }

