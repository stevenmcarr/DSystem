/* $Id: bound.C,v 1.7 1995/08/18 10:29:58 trsuchyt Exp $ */

/****************************************************************************/
/*                                                                          */
/*    File:  bound.C                                                        */
/*                                                                          */
/*    Description:  Contains function for update loop bounds after unrolling*/
/*                                                                          */
/****************************************************************************/
  
#include <general.h>
#include <mh.h>
#include <mh_ast.h>
#include <fort/walk.h>
#include <bound.h>

#include <pt_util.h>


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
	  mod = pt_gen_mod(pt_simplify_expr(pt_gen_add(
                                                pt_gen_sub(tree_copy_with_type(upb),
							   tree_copy_with_type(lwb)),
							 pt_gen_int(1))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(step_v),
						       pt_gen_int(val+1))));
	else if (step_v == -1)
	  mod = pt_gen_mod(pt_simplify_expr(pt_gen_add(
                                                pt_gen_sub(tree_copy_with_type(lwb),
							   tree_copy_with_type(upb)),
							 pt_gen_int(1))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(-step_v),
						       pt_gen_int(val+1))));
	else if (step_v > 0)
	  mod = pt_gen_mod(pt_simplify_expr(pt_gen_div(pt_gen_add(
                                                           pt_gen_sub(
								tree_copy_with_type(upb),
							       tree_copy_with_type(lwb)),
							   pt_gen_int(1)),
							 tree_copy_with_type(step))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(step_v),
						       pt_gen_int(val+1))));
	else 
	  mod = pt_gen_mod(pt_simplify_expr(pt_gen_div(pt_gen_sub(
                                                          pt_gen_sub(
								tree_copy_with_type(upb),
							       tree_copy_with_type(lwb)),
							   pt_gen_int(1)),
							 tree_copy_with_type(step))),
			   pt_simplify_expr(pt_gen_mul(pt_gen_int(-step_v),
						       pt_gen_int(val+1))));
	pt_tree_replace(upb_c,pt_simplify_expr(
                              pt_gen_add(pt_gen_mul(tree_copy_with_type(mod),
						    tree_copy_with_type(step)),
					 pt_gen_sub(tree_copy_with_type(lwb),
						    tree_copy_with_type(step)))));
	pt_tree_replace(ph1,pt_gen_add(
                              pt_simplify_expr(pt_gen_mul(tree_copy_with_type(mod),
							  tree_copy_with_type(step))),
			      lwb));
	pt_tree_replace(ph2,pt_simplify_expr(pt_gen_mul(tree_copy_with_type(step),
							 pt_gen_int(val+1))));
       }
  }

