/* $Id: skew.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************/
/*                                                                         */
/*   skew.c - contains code to skew a loop in the positive direction       */
/*                                                                         */
/*   Apr 1990 JPW  Created                                                 */
/*   May 1990 cwt  Update for new DT & simplify exprs where convenient     */
/*   Jul 1990 mhb  Redid pt_skew, got rid of skew_search stuff             */
/*   Aug 1990 tsm  Re-wrote pt_skew_estimate				   */
/*                                                                         */
/***************************************************************************/

#include <stdlib.h>
#include <math.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>

/*******************************************************************
 * pt_skew_estimate -- estimate the degree of skew for the
 * selected loop that would make wavefront execution possible
 * this analysis is only for the case where there is a true
 * dependence on the outer loop and an anti dependence on the
 * inner loop.  If these conditions do not exist it will just 
 * return 1
 *******************************************************************
 */
int 
pt_skew_estimate(PedInfo ped)
  //PedInfo ped;
{
AST_INDEX loop;
AST_INDEX bump;
EDGE_INDEX edge;
int vector, i_dist, j_dist, skew_est, i_level, j_level, level;
DG_Edge *dg;
Boolean need_skew;

/* initialize the variables */
loop = PED_SELECTED_LOOP(ped);
dg = dg_get_edge_structure( PED_DG(ped));
bump = list_first(gen_DO_get_stmt_LIST(loop));

/* the inner loop is the j level while the outer loop is the i level */
j_level = loop_level(loop);
i_level = j_level - 1;

/* need skew is a flag.  It is set to one if a skew is needed. */
need_skew = false;

/* skew est contains the amount of skewing to be done */
skew_est = 0;

/* Go through all the dependences and see if skewing is needed, and if so, how much */
while (bump!=AST_NIL)
  {
    vector = get_info(ped, bump, type_levelv);

	/* first get the vectors for the dependences that are on the i loop,
	   then get the same from the j loop.  (the i_level and the j_level) */
	for (level = i_level; level <= j_level; level++)
	for (edge = dg_first_src_stmt( PED_DG(ped),vector,level); edge !=NIL;
					 edge = dg_next_src_stmt( PED_DG(ped),edge))
	  {
		if (dg[edge].dt_type != DT_UNKNOWN)
		{
			/* get the components of the vector */
			i_dist = gen_get_dt_DIS(dg + edge, i_level);
			j_dist = gen_get_dt_DIS(dg + edge, j_level);

			/* make sure that these are distances */
			if ((!gen_is_dt_DIR(i_dist)) && (!gen_is_dt_DIR(j_dist)))
			{
				/* the presence of this type of vector is needed for
				   skewing to be necessary                     */
				if ((i_dist == 0) && (j_dist != 0))
					need_skew = true;
				else
				{
					/* this type of vector requires a skew of at least 1 */
					if ((i_dist != 0) && (j_dist == 0))
					{
						if (skew_est < 1)
							skew_est = 1;
					}
					else
					{
						/* this type may need a bigger skew */
						if (((i_dist < 0) && (j_dist > 0)) || ((i_dist > 0) && (j_dist < 0)))
						{
							if (skew_est < (1+abs(j_dist/i_dist)))
								skew_est = 1+abs(j_dist/i_dist);
						}
					}
				}
			}
		}
	  }
    bump = list_next(bump);
  }

  return need_skew ? skew_est : 0;
}


/*******************************************************************
 *  pt_skew(skew_degree,loop)
 *
 *        The calling function for loop skew.
 *
 *  Inputs:  skew_degree - amount to skew
 *           loop        - the loop to skew
 *
 *  Outputs: none
 *******************************************************************
 */
void
pt_skew(char *skew_degree,
	AST_INDEX loop)
  //char *skew_degree;
  //AST_INDEX loop;
{
	AST_INDEX low_bound,up_bound;	/* loop bounds */
	AST_INDEX skew_step;			/* skew amount */
	char *outer_var,*inner_var;
	AST_INDEX outer_control,inner_control;
	AST_INDEX temp,fac,con;
	int skew_int;
	Boolean lin;

	outer_control = gen_DO_get_control(out(loop));
	outer_var = gen_get_text(gen_INDUCTIVE_get_name(outer_control));
	
	/* get the induction variable */
	inner_control = gen_DO_get_control(loop);
	inner_var = gen_get_text(gen_INDUCTIVE_get_name(inner_control));

	/* get bounds */
	get_expressions (loop,&low_bound,&up_bound);

	skew_int = pt_convert_to_int(skew_degree);

	/* fix lower bound */
	pt_sep_linear (low_bound,outer_var,&lin,&fac,&con);
	if (lin) {
		temp = pt_gen_add (fac,pt_gen_int(skew_int));
		temp = pt_gen_mul (pt_gen_ident(outer_var),temp);
		temp = pt_gen_add (con,temp);
	}
	else {
		temp = pt_gen_mul (pt_gen_ident(outer_var),pt_gen_int(skew_int));
		temp = pt_gen_add (tree_copy(low_bound),temp);
	}
	pt_tree_replace (low_bound,pt_simplify_expr(temp));

	/* fix upper bound */
	pt_sep_linear (up_bound,outer_var,&lin,&fac,&con);
	if (lin) {
		temp = pt_gen_add (fac,pt_gen_int(skew_int));
		temp = pt_gen_mul (pt_gen_ident(outer_var),temp);
		temp = pt_gen_add (con,temp);
	}
	else {
		temp = pt_gen_mul (pt_gen_ident(outer_var),pt_gen_int(skew_int));
		temp = pt_gen_add (tree_copy(up_bound),temp);
	}
	pt_tree_replace (up_bound,pt_simplify_expr(temp));

	/* replace J with J-I*f  */
	temp = pt_gen_mul(pt_gen_ident(outer_var),pt_gen_int(skew_int));
	temp = pt_gen_sub(pt_gen_ident(inner_var),temp);
	temp = pt_simplify_expr(temp);
	pt_var_replace(gen_DO_get_stmt_LIST(loop),inner_var,temp);
}
