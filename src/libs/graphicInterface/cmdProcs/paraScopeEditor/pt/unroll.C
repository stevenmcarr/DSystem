/* $Id: unroll.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************/
/*                                                                         */
/*   unroll.c - code to perform loop unroll & unroll and jam               */
/*                                                                         */
/*   Sep 1990 cwt  created                                                 */
/*                                                                         */
/***************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>

typedef struct
{
	PedInfo ped;
	int level;
	int ret;
	DG_Edge *DG;
} Unroll_parm;

STATIC(int, pt_unroll_jam_test, (AST_INDEX stmt, int level, Unroll_parm *uparm));

static int bound_num;


/*-------------------------------------------------------------------

	pt_gen_mod()	generate MOD ast node

*/

AST_INDEX
pt_gen_mod(AST_INDEX arg1, AST_INDEX arg2)
{
	return gen_INVOCATION(pt_gen_ident("mod"), 
		list_append(list_create(arg1),list_create(arg2)));
}

/*-------------------------------------------------------------------

	pt_insert_before()		Insert list of AST_NODES before node

*/

void
pt_insert_before(AST_INDEX target, AST_INDEX inject)
{
	AST_INDEX node;

	while ((node = list_remove_first(inject)) != AST_NIL)
		list_insert_before(target, node);
}


/*-------------------------------------------------------------------

	pt_insert_after()		Insert list of AST_NODES after node

*/

void
pt_insert_after(AST_INDEX target, AST_INDEX inject)
{
	AST_INDEX node;

	while ((node = list_remove_last(inject)) != AST_NIL)
		list_insert_after(target, node);
}


/*-------------------------------------------------------------------

	pt_unroll_estimate()	Determine legality, estimate profitablity

	Returns:

		UNROLL_SYM_LOBOUND
		UNROLL_SYM_STEP
		UNROLL_IRREG_BOUND
		UNROLL_NEG_STEP
		UNROLL_DEPS
		UNROLL_ONELOOP
		UNROLL_OK
		max unroll value
*/

int
pt_unroll_estimate(PedInfo ped, Boolean jam)
{
	AST_INDEX loop, node;
	AST_INDEX control, lo_bound, up_bound, step;
	int lo, up, st, level;
	Unroll_parm uparm;

	/*--------------------------*/
	/* first check loop bounds	*/

	loop = PED_SELECTED_LOOP(ped);
	control = gen_DO_get_control(loop);
	lo_bound = gen_INDUCTIVE_get_rvalue1(control);
	up_bound = gen_INDUCTIVE_get_rvalue2(control);
	step = gen_INDUCTIVE_get_rvalue3(control);

	/* lower bound must be constant	*/
	if (pt_eval(lo_bound, &lo))
		return UNROLL_SYM_LOBOUND;

	/* upper bound may be symbolic	*/
	if ((NOT(pt_eval(up_bound, &up))) && (lo > up))
		return UNROLL_IRREG_BOUND;

	/* step must be constant	*/
	st = 1;
	if ((step != AST_NIL) && pt_eval(step, &st))
		return UNROLL_SYM_STEP;

	if (!st)
		return UNROLL_IRREG_BOUND;
	else if (st < 0)
		return UNROLL_NEG_STEP;

	if (NOT(jam))				/* that's all to be checked	*/
		return UNROLL_OK;		/* for unrolling loops		*/

	/*------------------------------*/
	/* now check jam possibilities	*/

	/* must have at least 1 other loop for unroll & jam	*/

	node = gen_DO_get_stmt_LIST(loop);
	node = list_first(node);
	while ((node != AST_NIL) && !is_do(node))
		node = list_next(node);
	if (node == AST_NIL)
		return UNROLL_ONELOOP;

	/* check for unroll & jam preventing dependences	*/

	uparm.ped = ped;
	uparm.level = loop_level(loop);
	uparm.DG = dg_get_edge_structure( PED_DG(ped));
	uparm.ret = MAXINT;

	walk_statements(loop, uparm.level, (WK_STMT_CLBACK)pt_unroll_jam_test, 
                        NULL, (Generic)&uparm);

	return (uparm.ret == MAXINT) ? UNROLL_OK : uparm.ret;
}


/*--------------------------------------------------------------------

	pt_unroll_jam_test()		Check whether deps inhibit unroll & jam

*/

static int
pt_unroll_jam_test(AST_INDEX stmt, int level, Unroll_parm *uparm)
{
	int vector;
	int edge_idx;
	DG_Edge *Edge;
	int i;
	int dis;
	Boolean unroll_bound;

    if ((vector = get_info(uparm->ped, stmt, type_levelv)) == UNUSED)
		return WALK_CONTINUE;

	edge_idx = dg_first_src_stmt( PED_DG(uparm->ped), vector, uparm->level);

	/* look at all DG edges for statement	*/

	while (edge_idx != UNUSED)
	{
		Edge = uparm->DG + edge_idx;
		unroll_bound = false;

		/* give up if unknown dependences	*/

		if ((Edge->type == dg_io) ||
			(Edge->type == dg_call) ||
			(Edge->type == dg_exit))
		{
			uparm->ret = UNROLL_DEPS;
			return WALK_ABORT;
		}

		/* check for unroll-and-jam preventing TRUE/ANTI/OUTPUT deps */

		else if ((Edge->type == dg_true) ||
				(Edge->type == dg_anti) ||
				(Edge->type == dg_output))
        {

		if (Edge->dt_type == DT_UNKNOWN)
		{
			uparm->ret = UNROLL_DEPS;
			return WALK_ABORT;
		}

		/* using dir/dis vector, check to see that there are	*/
		/* no vectors of form (<, =, >, ...)					*/

		/* vectors such as (<, =, <, ...) are okay				*/

		/* if illegal vector found, it's still possible to 		*/
		/* unroll less than the distance on the outer loop		*/

		for (i = uparm->level + 1; i <= gen_get_dt_CLVL(Edge); i++)
		{
			dis = gen_get_dt_DIS(Edge, i);

			/*----------------------------------*/
			/* entry corresponds to a direction	*/

			if (gen_is_dt_DIR(dis))
			{
				if (dis == DDATA_LT)	/* all's well, this loop can 	*/
					break;				/* carry the dep after unroll	*/

				if ((dis == DDATA_GT) || (dis == DDATA_GE) || 
					(dis == DDATA_NE) || (dis == DDATA_ANY)) 
				{
					/* trouble, dependence may change direction		*/
					/* if loop is unrolled too much					*/
					unroll_bound = true;
				}
			}

			/*----------------------------------*/
			/* entry corresponds to a distance	*/

			else
			{
				if (dis > 0)		/* all's well, this loop can 	*/
					break;			/* carry the dep after unroll	*/

				if (dis < 0)
				{
					/* trouble, dependence may change direction		*/
					/* if loop is unrolled too much					*/
					unroll_bound = true;
				}
			}

			/*--------------------------------------------------*/
			/* may need to bound amount of unrolling allowed	*/

			if (unroll_bound)
			{
				dis = gen_get_dt_DIS(Edge, uparm->level);

				if (gen_is_dt_DIR(dis) || (dis < 2))
				{
					/* forget it, no unroll possible	*/
					uparm->ret = UNROLL_DEPS;
					return WALK_ABORT;
				}
				else if (--dis < uparm->ret)
				{
					/* it's possible to unroll up to		*/
					/*	this distance on outer loop			*/
					/* pick smallest bound found thus far	*/
					uparm->ret = dis;
				}
			}
		}
		}
		edge_idx = dg_next_src_stmt( PED_DG(uparm->ped), edge_idx);
	}

	return WALK_CONTINUE;
}


/*--------------------------------------------------------------------

	pt_unroll_jam()		Perform unroll and jam


	Strategy 1 for guard creation
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	n$ = mod(up - lo + 1, step * unroll)

	if (n$ == 1)
		body
	else if (n$ == 2) 
		body
		body
	else if (n$ == 3) 
		body
		body
		body
	else if (n$ == 4) 
		...
		...
	else if (n$ == step * unroll - 1)
		body
		...

	do I = lo + n$ * step, up, step * unroll
		body...
	enddo

	Strategy 2 for guard creation
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	n$ = mod(up - lo + 1, step * unroll)

	do I = lo, lo + ((n$ - 1) * step), step
		body...

	do I = lo + n$ * step, up, step * unroll
		body...
	enddo


*/

void
pt_unroll_jam(PedInfo ped, char *degree, Boolean jam)
{
	AST_INDEX loop;
	AST_INDEX loop2;
	AST_INDEX lo_bound,up_bound, step;	/* loop bounds */
	int lo_bound_v,up_bound_v, step_v;	/* loop bounds */
	char *ivar;
	int unroll_v;
	int remain_v;
	Boolean exact;
	Boolean unroll_completely;
	AST_INDEX slist;
	AST_INDEX new_step;
	AST_INDEX control;
	AST_INDEX control2;
	AST_INDEX new_ivar;
	AST_INDEX new_list;
	AST_INDEX first_stmt;
	AST_INDEX last_stmt;
	AST_INDEX preloop;
	AST_INDEX postloop;
	AST_INDEX loop_body;
	int i;
	int iters_v;
	AST_INDEX new_up_bound;
	AST_INDEX stmt, stmt2;

	loop = PED_SELECTED_LOOP(ped);	/* get loop to work on	*/
	unroll_completely = false;
	new_up_bound = AST_NIL;
	exact = true;				/* init assume constant bounds	*/

	/*------------------*/
	/* get parameters	*/

	unroll_v = pt_convert_to_int(degree) + 1;
	control = gen_DO_get_control(loop);
	ivar = gen_get_text(gen_INDUCTIVE_get_name(control));
	lo_bound = gen_INDUCTIVE_get_rvalue1(control);
	up_bound = gen_INDUCTIVE_get_rvalue2(control);
	step = gen_INDUCTIVE_get_rvalue3(control);

	/*------------------*/
	/* check out step	*/

	if (step == AST_NIL)			/* default step = 1	*/
	{
		step_v = 1;
		new_step = pt_gen_int(unroll_v);
	}
	else if (pt_eval(step, &step_v))		/* variable loop step	*/
	{
		new_step = pt_gen_mul(pt_gen_int(unroll_v), tree_copy(step));
		exact = false;

		return;			/* should have be caught earlier	*/
	}
	else									/* const step			*/
	{
		new_step = pt_gen_int(step_v * unroll_v);
	}

	/*--------------*/
	/* check bounds	*/

	if (pt_eval(lo_bound, &lo_bound_v))	/* variable lower bound	*/
	{
		return;			/* should have be caught earlier	*/
	}

	if (pt_eval(up_bound, &up_bound_v))	/* variable upper bound	*/
		exact = false;	

	/*------------------------------*/
	/* check whether guard needed	*/

	if (NOT(exact))		/* should only mean var upper bound	*/
	{
		if (NOT(jam))
		{
			/* if unroll only, put guard immediately before loop	*/

		}
		else
		{
			/* if unroll & jam, move guard outside ALL loops	*/
		}
	}
	else
	{
		if (up_bound_v <= lo_bound_v)
		{
			/* illegal bounds should be caught earlier		*/
 
			/* should be here only if bounds are equal		*/
			/* in that case, loop is only executed once,	*/
			/* so don't bother unrolling					*/

			return;	
		}

		/* calc # iterations excuted (trip count of loop)	*/

		iters_v = ((up_bound_v - lo_bound_v) / step_v) + 1;

		/* check whether can unroll by this amount	*/

		if (iters_v <= unroll_v)
		{
			/* unroll outer loop completely!	*/

			unroll_completely = true;
			unroll_v = iters_v;
		}

		/* check whether there are extra iterations remaining */

		else if (remain_v = iters_v % unroll_v)
		{
			/* peel off the required # of iterations to make	*/
			/* the remaining loop unroll exactly right			*/

			if (remain_v == 1)
				pt_peel_iteration(ped, loop, true, remain_v);
			else
			{
				/* split off part of loop	*/

				/* oh well, do that later...	*/
				pt_peel_iteration(ped, loop, true, remain_v);
			}

			/* adjust lower bound value, calc real upper bound	*/

			lo_bound_v += step_v * remain_v;
			if (remain_v = ((up_bound_v - lo_bound_v) % (step_v * unroll_v)))
			{
				up_bound_v -= remain_v;
				new_up_bound = pt_gen_int(up_bound_v);
			}
		}
	}

	new_list = AST_NIL;

	/*------------------------------*/
	/* get code to be duplicated	*/

	/* find loop_body, the unrolled code	*/

	if (NOT(jam))	/* if unroll only, copy body of loop	*/
	{
		if (unroll_completely)
		{
			pt_peel_iteration(ped, loop, true, unroll_v);
			return;
		}

		loop_body = gen_DO_get_stmt_LIST(loop);
	}
	else		/* if unroll and jam, copy body of INNERMOST loop	*/
	{
		loop_body = gen_DO_get_stmt_LIST(loop);
		first_stmt = list_first(loop_body);
		loop2 = first_stmt;

		/* we need to make sure that all imperfectly 	*/
		/* nested statements are also unrolled			*/

		while (true)
		{
			/*------------------------------------------*/
			/* are we at deepest level of nesting?		*/
			
			stmt = first_stmt;
			while ((stmt != AST_NIL) && !is_do(stmt))
				stmt = list_next(stmt);

			if (stmt == AST_NIL)	/* loop2 is deepest loop, go on	*/
				break;

			loop2 = stmt;			/* found deeper loop, save	*/

			/* not yet at deepest level of nesting, so...	*/

			/*--------------------------*/
			/* handle all preloop stmts	*/

			/* first find all preloop stmts	*/

			preloop = AST_NIL;
			stmt = first_stmt;

			while (!is_do(stmt))
			{
				if (!is_comment(stmt))
				{
					/* accumulate noncomment new statements	*/

					preloop = (preloop == AST_NIL) ? 
						list_create(tree_copy(stmt)) : 
						list_append(preloop, list_create(tree_copy(stmt)));
				}

				stmt = list_next(stmt);
			}

			/* now make copies of preloop statements	*/

			pt_clear_info(ped, preloop);

			new_list = AST_NIL;
			for (i = 1; i < unroll_v; i++)
			{
				/* adjust occurences of ivar in new body	*/

				slist = tree_copy(preloop);
				pt_var_add(slist, ivar, i * step_v);
				new_list = (new_list == AST_NIL) ? 
					slist : list_append(new_list, slist);
			}

			/* and insert them into the program	*/

			tree_free(preloop);
			pt_insert_before(loop2, new_list);

			/*--------------------------*/
			/* handle all postloop stmts	*/

			/* first find all postloop stmts	*/

			postloop = AST_NIL;
			stmt = list_next(loop2);

			while (stmt != AST_NIL)
			{
				if (!is_comment(stmt))
				{
					/* accumulate noncomment new statements	*/

					postloop = (postloop == AST_NIL) ? 
						list_create(tree_copy(stmt)) : 
						list_append(postloop, list_create(tree_copy(stmt)));
				}

				last_stmt = stmt;
				stmt = list_next(stmt);
			}

			/* now make copies of postloop statements	*/

			pt_clear_info(ped, postloop);

			new_list = AST_NIL;
			for (i = 1; i < unroll_v; i++)
			{
				/* adjust occurences of ivar in new body	*/

				slist = tree_copy(postloop);
				pt_var_add(slist, ivar, i * step_v);
				new_list = (new_list == AST_NIL) ? 
					slist : list_append(new_list, slist);
			}

			/* and insert them into the program	*/

			tree_free(postloop);
			pt_insert_after(last_stmt, new_list);

			/*------------------------------------------*/
			/* now go look at body of next loop			*/

			loop_body = gen_DO_get_stmt_LIST(loop2);
			first_stmt = list_first(loop_body);
		}
	}

	/* now loop_body is body of innermost loop 	*/

	/*------------------------------------------*/
	/* unroll body of loop # of times requested	*/

	new_list = AST_NIL;

	for (i = 1; i < unroll_v; i++)
	{
		slist = tree_copy(loop_body);

		pt_clear_info(ped, slist);	/* clear DG edges */

		/* adjust occurences of ivar in new body	*/

		if (unroll_completely)
		{
			pt_var_replace(slist, ivar, 
				pt_gen_int((i * step_v) + lo_bound_v));
		}
		else
		{
			pt_var_add(slist, ivar, i * step_v);
		}

		/* accumulate new statements	*/

		new_list = (new_list == AST_NIL) ? slist : 
						list_append(new_list, slist);
	}

	if (unroll_completely)	/* don't forget orig body	*/
	{
		pt_var_replace(loop_body, ivar, lo_bound);
	}
	else
	{
		/* fix new step & upper bound			*/

		if (step == AST_NIL)
			gen_INDUCTIVE_put_rvalue3(control, new_step);
		else
			tree_replace(step, new_step);

		if (new_up_bound != AST_NIL)
			tree_replace(up_bound, new_up_bound);
	}

	list_append(loop_body, new_list);	/* add after orig body	*/
}











