/* $Id: loop.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************/
/*                                                                         */
/*   loop.c - code to make some loop transformations loop bounds           */
/*                                                                         */
/*   Includes code for:                                                    */
/*                                                                         */
/*        sequential <-> parallel                                          */
/*        loop adjust                                                      */
/*        loop reversal                                                    */
/*        loop unswitch                                                    */
/*                                                                         */
/***************************************************************************/

#include <stdlib.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>

typedef struct
{
	PedInfo ped;
	int level;
	int ret;
	DG_Edge *DG;
} Reverse_parm;

STATIC(int, pt_reverse_test,(AST_INDEX , int , Reverse_parm *));


/*-------------------------------------------------------------------
/*
 * pt_make_parallel() - replace the DO with a PARALLEL LOOP (a la IBM PF)
 * and a PRIVATE stmt immediately following it. The PRIVATE stmt is generated
 * only if the loop has a non-empty list of private variables.
 *
 */
void
pt_make_parallel(PedInfo ped, AST_INDEX node)
{
    LI_Instance	*LI = PED_LI(ped);
    Loop_info	*loop;
    Slist	*pvar;
    AST_INDEX	id, pvar_list, s1, s2, slist;
    int		len, rc;
        
    loop = find_loop( PED_LI(ped), node);
    
    slist = gen_DO_get_stmt_LIST(node);
    tree_replace(slist, AST_NIL);
    /* make the parallel loop */
    s1 = gen_PARALLELLOOP(tree_copy(gen_DO_get_lbl_def(node)), 
			  tree_copy(gen_DO_get_close_lbl_def(node)),
			  tree_copy(gen_DO_get_lbl_ref(node)), 
			  tree_copy(gen_DO_get_control(node)),
			  slist);

    /* replace the DO with the PARALLEL LOOP */
    tree_replace(node, s1);
    
    /* get the private vars in the form of a list */

    pvar_list = list_create(AST_NIL);
    pvar = el_get_first_private_node(PED_LI(ped), &len, ALL_SHARED);

    if(pvar != NULL) {
    	while(pvar != NULL) {
	   /* kluge: weed out spurious private vars that may be
	      put in by PTOOL */
	   rc = strcmp(pvar->name, "ilwb1");
	   if(rc == 0) {
       	      /* get the next pvar */
       	      pvar = el_get_next_private_node(LI, pvar, &len, 0);
	      continue;
	   }
	   rc = strcmp(pvar->name, "ilwb2");
	   if(rc == 0) {
       	      /* get the next pvar */
       	      pvar = el_get_next_private_node(LI, pvar, &len, 0);
	      continue;
	   }
           /* create an identifier node */
           id = gen_IDENTIFIER();
       	   gen_put_text(id, pvar->name, STR_IDENTIFIER);
       	   /* add pvar to pvar_list */
       	   list_insert_last(pvar_list, id);
       	   /* get the next pvar */
       	   pvar = el_get_next_private_node(LI, pvar, &len, 0);
    	}
    
   	/* create a PRIVATE node, and insert it after the PARALLEL LOOP */
    	s2 = gen_PRIVATE(AST_NIL, pvar_list);    
    	list_insert_first(gen_PARALLELLOOP_get_stmt_LIST(s1), s2);
    }

    loop->loop_hdr_index = s1;
    el_set_parallelized_bit(PED_LI(ped));

}


/*-------------------------------------------------------------------
/*
 * pt_make_sequential() - replace the PARALLEL LOOP with an ordinary DO loop.
 *
 * Vas, Sept. 1988
 */
void
pt_make_sequential(PedInfo ped, AST_INDEX node)
{
    Loop_info	*loop;
    AST_INDEX	s1, parm1, parm2, parm3, parm4, parm5;
    
    loop = find_loop( PED_LI(ped), node);

    s1 = gen_DO(AST_NIL, AST_NIL, AST_NIL, AST_NIL, AST_NIL);
    parm1 =  gen_PARALLELLOOP_get_lbl_def(node);
    parm2 =  gen_PARALLELLOOP_get_close_lbl_def(node);
    parm3 =  gen_PARALLELLOOP_get_lbl_ref(node);
    parm4 =  gen_PARALLELLOOP_get_control(node);
    parm5 =  gen_PARALLELLOOP_get_stmt_LIST(node);

    /* insert the DO stmt before the PARALLEL LOOP stmt */
    list_insert_before(node, s1);
    
    /* pluck these guys out of the tree */
    tree_replace(parm1, AST_NIL);
    tree_replace(parm2, AST_NIL);
    tree_replace(parm3, AST_NIL);
    tree_replace(parm4, AST_NIL);
    tree_replace(parm5, AST_NIL);
    
    /* remove the PARALLEL LOOP stmt */
    list_remove_node(node);

    /* set up the DO correctly */
    gen_DO_put_lbl_def(s1, parm1);
    gen_DO_put_close_lbl_def(s1, parm2);
    gen_DO_put_lbl_ref(s1, parm3);
    gen_DO_put_control(s1, parm4);
    gen_DO_put_stmt_LIST(s1, parm5);
    
    /* finally remove the PRIVATE stmt if one exists */
    loop->loop_hdr_index = s1;
    loop->parallelized = false;
    if(is_private(list_first(gen_DO_get_stmt_LIST(s1))))
       list_remove_node(list_first(gen_DO_get_stmt_LIST(s1)));

}

/*-------------------------------------------------------------------

	pt_adjust()		Adjust lower bound of loop

*/

void pt_adjust(PedInfo ped, char *adjust)
{
	AST_INDEX loop, control, lo_bound, up_bound, stmts;
	char *ivar;
	int adjust_v, lo, up;

	adjust_v = atoi(adjust);
	loop = PED_SELECTED_LOOP(ped);
	control = gen_DO_get_control(loop);
	lo_bound = gen_INDUCTIVE_get_rvalue1(control);
	up_bound = gen_INDUCTIVE_get_rvalue2(control);
	ivar = gen_get_text(gen_INDUCTIVE_get_name(control));
	stmts = gen_DO_get_stmt_LIST(loop);

	/*------------------*/
	/* new lower bound	*/

	if (pt_eval(lo_bound, &lo))		/* variable value	*/
	{
		tree_replace(lo_bound, adjust_v > 0 ?
			pt_gen_add(tree_copy(lo_bound), pt_gen_int(adjust_v)) :
			pt_gen_sub(tree_copy(lo_bound), pt_gen_int(-adjust_v)));

	}
	else		/* constant value	*/
	{
		tree_replace(lo_bound, pt_gen_int(lo+adjust_v));
	}

	/*------------------*/
	/* new upper bound	*/

	if (pt_eval(up_bound, &up))		/* variable value	*/
	{
		tree_replace(up_bound, adjust_v > 0 ?
			pt_gen_add(tree_copy(up_bound), pt_gen_int(adjust_v)) :
			pt_gen_sub(tree_copy(up_bound), pt_gen_int(-adjust_v)));
	}
	else		/* constant value	*/
	{
		tree_replace(up_bound, pt_gen_int(up+adjust_v));
	}

	/*------------------*/
	/* fix body of loop */

	pt_var_add(stmts, ivar, -adjust_v);

}


/*-------------------------------------------------------------------

	pt_reverse()		Reverse lower bounds

*/

void
pt_reverse(PedInfo ped)
{
	AST_INDEX loop, control, lo_bound, up_bound, step, temp;
	int step_v, lo_v, up_v, newup_v;

	/* assume must be both possible and legal to reverse loop	*/

	loop = PED_SELECTED_LOOP(ped);
	control = gen_DO_get_control(loop);
	lo_bound = gen_INDUCTIVE_get_rvalue1(control);
	up_bound = gen_INDUCTIVE_get_rvalue2(control);
	step = gen_INDUCTIVE_get_rvalue3(control);

	if (step == AST_NIL)
	{
		gen_INDUCTIVE_put_rvalue3(control, pt_gen_int(-1));
	}
	else if (NOT(pt_eval(step, &step_v)))
	{
		/* now reverse step value	*/

		tree_replace(step, pt_gen_int(-step_v));
		tree_free(step);

		if ((step_v != 1) && (step_v != -1))
		{
			/*--------------------------------------------------*/
			/* not unitary step, both bounds must be constant	*/
			/* or else reversal is not possible					*/

			/* if needed, adjust upper bound to actual value	*/

			if (pt_eval(lo_bound, &lo_v) || pt_eval(up_bound, &up_v))
			{
				message("Error in pt_reverse");
				return;
			}

			newup_v = lo_v + (((up_v - lo_v) / step_v ) * step_v);
			if (newup_v != up_v)
			{
				tree_replace(lo_bound, pt_gen_int(newup_v));
				tree_replace(up_bound, lo_bound);
				tree_free(up_bound);
				return;
			}
		}

	}
	else
	{
		message("Error in pt_reverse");
		return;
	}

	/* swap lower bound with upper bound	*/

	temp = pt_gen_int(1);
	tree_replace(lo_bound, temp);
	tree_replace(up_bound, lo_bound);
	tree_replace(temp, up_bound);
	tree_free(temp);
}



/*-------------------------------------------------------------------

	pt_reverse_estimate()		

	Check whether loop reversal is legal and/or possible

*/

int
pt_reverse_estimate(PedInfo ped)
{
	AST_INDEX loop, control, lo_bound, up_bound, step;
	Boolean unitary_step;
	int value;
	Reverse_parm parm;

	unitary_step = false;

	/*------------------------------------------*/
	/* first check whether reversal is possible	*/

	loop = PED_SELECTED_LOOP(ped);
	control = gen_DO_get_control(loop);
	lo_bound = gen_INDUCTIVE_get_rvalue1(control);
	up_bound = gen_INDUCTIVE_get_rvalue2(control);
	step = gen_INDUCTIVE_get_rvalue3(control);

	if (step == AST_NIL)
	{
		unitary_step = true;
	}
	else if (pt_eval(step, &value))		/* symbolic step	*/
	{
		return REV_UNABLE;
	}
	else
	{
		if ((value == 1) || (value == -1))
		{
			unitary_step = true;
		}
	}

	if (pt_eval(lo_bound, &value) || pt_eval(up_bound, &value))
	{
		if (NOT(unitary_step))
			return REV_UNABLE;
	}

	/*--------------------------------------------------*/
	/* now check whether dependences prevent reversal	*/

	parm.ped = ped;
	parm.level = loop_level(loop);
	parm.DG = dg_get_edge_structure( PED_DG(ped));
	parm.ret = 0;

	walk_statements(loop, LEVEL1, (WK_STMT_CLBACK)pt_reverse_test, NULL, (Generic)&parm);

	return parm.ret ? REV_ILLEGAL : REV_OKAY;
}



static int
pt_reverse_test(AST_INDEX stmt, int level, Reverse_parm *parm)
{
	int vector;
	int edge_idx;
	DG_Edge *Edge;

    if ((vector = get_info(parm->ped, stmt, type_levelv)) == UNUSED)
		return WALK_CONTINUE;

	for (edge_idx = dg_first_src_stmt(PED_DG(parm->ped), 
						vector, parm->level);
       edge_idx != UNUSED;
       edge_idx = dg_next_src_stmt(PED_DG(parm->ped), edge_idx))
	{
		Edge = parm->DG + edge_idx;

		if ((Edge->type == dg_true) || (Edge->type == dg_anti) ||
			(Edge->type == dg_output))
		{
			parm->ret++;	/* found loop-carried data dependence	*/
			return WALK_ABORT;
		}
	}

	return WALK_CONTINUE;
}



/*-------------------------------------------------------------------

	pt_unswitch()

*/

void pt_unswitch(PedInfo ped)
{
	AST_INDEX node;

	node = PED_SELECTED_LOOP(ped);
	node = gen_DO_get_stmt_LIST(node);

	node = list_first(node);

	while (is_comment(node))
		node = list_next(node);

	if (!is_if(node))
		;

}


pt_unswitch_test(PedInfo ped)
{
	AST_INDEX node;

	node = PED_SELECTED_LOOP(ped);
	node = gen_DO_get_stmt_LIST(node);

	/*------------------------------*/
	/* check for stmts before IF	*/

	node = list_first(node);

	while (is_comment(node))
		node = list_next(node);

	if (!is_if(node))
	{
		if (list_next(node) == AST_NIL)
			return UNSWITCH_NOIF;

		return UNSWITCH_IFPLUS;
	}

	/*------------------------------------------------------*/
	/* check whether guard expressions are loop invariant	*/


	/*----------------------------------*/
	/* check for other stmts after IF	*/

	node = list_next(node);

	while (is_comment(node))
		node = list_next(node);

	return node == AST_NIL ? UNSWITCH_OKAY : UNSWITCH_IFPLUS;
}









