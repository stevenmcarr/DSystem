/* $Id: sreplace.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************/
/*                                                                         */
/*   scalar_rep.c - code to perform scalar replacement                     */
/*                                                                         */
/*   For now only identifies loop independent reuse opportunities          */
/*                                                                         */
/*   Sep 1990 cwt  created                                                 */
/*                                                                         */
/***************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

#define RSIZE 256		/* max replace opportunities		*/
#define ITER 2			/* 2^2 = expected # loop iterations	*/

typedef struct			/* for walk stmt routines	*/
{
	PedInfo ped;
	AST_INDEX var;
	int refs;				/* # of actual refs		*/
	int read;				/* # of expected reads	*/
	int write;				/* # of expected writes	*/
	int mult;				/* mult factor in loops	*/
	Boolean exposed_use;
	AST_INDEX locs[RSIZE];
	AST_INDEX var_loop;		/* deepest variant loop	*/
} ReplaceS_parm;

typedef struct			/* for walk expr routines	*/
{
	char *ivar[20];
	int level;
	Boolean invariant;
} Invariant_parm;

/*--------*/
/* macros */

#define SYM_EQ2(x,y)      ((*(x) == *(y)) && !strcmp((x), (y)))

static int scalar_id;		/* generate unique id	*/

/* forward defs	*/

STATIC(int, srep_stest_pre,(AST_INDEX stmt, int level, ReplaceS_parm *parm));
STATIC(int, srep_stest_post,(AST_INDEX stmt, int level, ReplaceS_parm *parm,
                             PedInfo ped));
STATIC(int, srep_etest,(AST_INDEX expr, ReplaceS_parm *parm));
STATIC(int, pt_invariant_test,(AST_INDEX expr, Invariant_parm *parm));


/*-------------------------------------------------------------------

	pt_invariant_subs()		Checks whether subscripted variable
							is invariant with respect to the loop 

*/

Boolean
pt_invariant_subs(AST_INDEX node, AST_INDEX subs)
{
	AST_INDEX temp;
	Invariant_parm parm;
	int level = 0;

	parm.invariant = true;
	parm.level = 0;

	/*----------------------------------*/
	/* get index vars of outer loops	*/

	node = tree_out(node);
	while (node != AST_NIL)
	{
		if (is_do(node) || is_parallelloop(node))
		{
			temp = gen_DO_get_control(node);
			temp = gen_INDUCTIVE_get_name(temp);
			parm.ivar[parm.level++] = gen_get_text(temp);
		}
		node = tree_out(node);
	}

	/* look at subscripts	*/

	subs = gen_SUBSCRIPT_get_rvalue_LIST(subs);
	walk_expression(subs, (WK_EXPR_CLBACK)pt_invariant_test, NULL, (Generic)&parm);

	return parm.invariant;
}


/*-------------------------------------------------------------------

	pt_rep_s_estimate()	Determine legality, estimate profitablity

	Returns:	REP_S_BAD
				REP_S_VAR
				REP_S_INLOOP
				REP_S_INNER
				REP_S_NONE
				REP_S_GOOD

*/

int
pt_rep_s_estimate(PedInfo ped, char *msg)
{
	AST_INDEX loop, sel, node;
	ReplaceS_parm parm;

	/*----------------------------------------------*/
	/* first check whether array ref selected		*/

	sel = PED_SELECTION(ped);
	if (is_identifier(sel))		/* in case only name selected	*/
		sel = tree_out(sel);
	if (!is_subscript(sel))
		return REP_S_VAR;

	/*--------------------------------------*/
	/* make sure selection in current loop	*/

	loop = PED_SELECTED_LOOP(ped);

	node = tree_out(sel);
	while ((node != AST_NIL) && (node != loop))
		node = tree_out(node);

	if (node == AST_NIL)
		return REP_S_INLOOP;

	/*----------------------------------------------*/
	/* now check loop for other occurrences of var	*/

	parm.ped = ped;
	parm.var = sel;
	parm.refs = 0;
	parm.read = 0;
	parm.write = 0;
	parm.mult = 0;
	parm.var_loop = AST_NIL;
	parm.exposed_use = true;

	walk_statements(loop, loop_level(loop), 
		(WK_STMT_CLBACK)srep_stest_pre, NULL, (Generic)&parm);

	/*------------------------------*/
	/* decide whether profitable	*/

	/* profitable if 2 or more writes or reads	*/

	if ((parm.write > 1) || (parm.read > 1))
		return REP_S_GOOD;

	/* if either only 1 read or only 1 write, not profitable	*/

	if (!parm.write || !parm.read)
		return REP_S_NONE;

	/* exactly 1 read & 1 write					*/
	/* if exposed use, then must generate load	*/
	/* not profitable to scalar replace 		*/

	return (parm.exposed_use) ? REP_S_NONE : REP_S_GOOD;

}

/*--------------------------------------------------------------------

	pt_rep_s()		Perform scalar replacement

*/

void
pt_rep_s(PedInfo ped)
{
	AST_INDEX loop, sel, scalar, node, slist;
	int i;
	ReplaceS_parm parm;
	Loop_info *lptr;
	Boolean invariant;
	char buf[80];

	/*--------------------------*/
	/* find occurrences of var	*/

	loop = PED_SELECTED_LOOP(ped);
	lptr = find_loop( PED_LI(ped), loop);

	sel = PED_SELECTION(ped);
	if (is_identifier(sel))		/* in case only name selected	*/
		sel = tree_out(sel);

	PED_SELECTION(ped) = tree_out(sel);		/* change selection	*/

	parm.ped = ped;
	parm.var = sel;
	parm.refs = 0;
	parm.read = 0;
	parm.write = 0;
	parm.mult = 0;
	parm.var_loop = AST_NIL;
	parm.exposed_use = true;
	
	walk_statements(loop, loop_info_loopLevel(lptr), 
			(WK_STMT_CLBACK)srep_stest_pre, NULL, (Generic)&parm);

	/*------------------------------------------*/
	/* now replace them all with a new variable	*/

	loop = PED_SELECTED_LOOP(ped);
	node = gen_SUBSCRIPT_get_name(sel);

	sprintf(buf, "%s$%d", gen_get_text(node), scalar_id++);
	scalar = pt_gen_ident(buf);

	for (i = 0; i < parm.refs; i++)
		tree_replace(parm.locs[i], tree_copy(scalar));

	/*--------------------------------------------------------------*/
	/* no loop variant loop found, must be totally loop invariant	*/
	/* put Load/Store outside of selected loop						*/

	if (parm.var_loop == AST_NIL)
	{
		/*------------------------------------------*/
		/* generate load if not local def			*/
		/* Note: this assumes def was not guarded!	*/

		if (parm.exposed_use)
		{
			node = gen_ASSIGNMENT(AST_NIL, tree_copy(scalar), 
				tree_copy(sel));
			list_insert_before(loop, node);
		}

		/*------------------------------------------*/
		/* generate store if write occurred			*/

		if (parm.write)
		{
			node = gen_ASSIGNMENT(AST_NIL, tree_copy(sel), 
				tree_copy(scalar));
			list_insert_after(loop, node);
		}
	}

	/*--------------------------------------------------------------*/
	/* loop variant loop found, must put Load/Store in that loop	*/

	else
	{
		/*------------------------------------------*/
		/* generate load if not local def			*/
		/* Note: this assumes def was not guarded!	*/

		if (parm.exposed_use)
		{
			node = gen_ASSIGNMENT(AST_NIL, tree_copy(scalar), 
				tree_copy(sel));
			slist = gen_DO_get_stmt_LIST(parm.var_loop);
			slist = list_append(list_create(node), slist);
			gen_DO_put_stmt_LIST(parm.var_loop, slist);
		}

		/*------------------------------------------*/
		/* generate store if write occurred			*/

		if (parm.write)
		{
			node = gen_ASSIGNMENT(AST_NIL, tree_copy(sel), 
				tree_copy(scalar));
			list_append(gen_DO_get_stmt_LIST(parm.var_loop), 
				list_create(node));
		}
	}

	/*------------------------------------------------------------------*/
	/* add new scalar to private var list of all enclosing loops		*/

	el_add_private_up( PED_LI(ped), loop, buf);

}

/*-------------------------------------------------------------------

	srep_stest_pre()	Walk routine to collect var occurrences
						For statements (pre_action)

	Also check for deepest loop where var is loop variant

*/

static int
srep_stest_pre(AST_INDEX stmt, int level, ReplaceS_parm *parm)
{
	AST_INDEX  expr1;
	AST_INDEX  expr2;

	/*----------------------------------------------------------*/
	/* special treatment of assign, since we separate def/uses	*/

	if (is_assignment(stmt))
	{
		expr1 = gen_ASSIGNMENT_get_lvalue(stmt);	/* get lhv		*/
		expr2 = gen_ASSIGNMENT_get_rvalue(stmt);	/* check rhv	*/

		walk_expression(expr2, (WK_EXPR_CLBACK)srep_etest, NULL, (Generic)parm);

		if (is_subscript(expr1))
		{
			/* just in case there are refs in the subscript list	*/
			/* note that they count as uses							*/

			walk_expression(gen_SUBSCRIPT_get_rvalue_LIST(expr1), 
				(WK_EXPR_CLBACK)srep_etest, NULL, (Generic)parm);

			/* now check lhs	*/

			if (pt_expr_equal(expr1, parm->var))
			{
				/* if first ref found is a def, remember that	*/
				if (!parm->refs)
					parm->exposed_use = false;

				parm->write += 1 << parm->mult;
				parm->locs[parm->refs++] = expr1;
			}
		}
	}

	/*----------------------------------------------------------*/
	/* special treatment of loop to check for loop invariance	*/

	else if (is_do(stmt))
	{
		if (pt_invariant_subs(stmt, parm->var))
		{
			parm->mult += ITER;		/* increase coeff	*/
		}
		else
		{
			/* use overwrite strategy to locate last loop		*/
			/* Note: fails if multiple loops at desired level!	*/

			parm->var_loop = stmt;
		}
	}

	/*--------------------------------------------------------------*/
	/* skip compound stmts, will be walking over components later	*/

	else if (!is_compound(stmt))	
	{
		/* generic single statement, get & walk over component exprs	*/

		if (get_expressions(stmt, &expr1, &expr2) == UNKNOWN_STATEMENT)
			return WALK_CONTINUE; 

		if (expr1 != AST_NIL)
                  walk_expression(expr1, (WK_EXPR_CLBACK)srep_etest, NULL, (Generic)parm);

		if (expr2 != AST_NIL)
                  walk_expression(expr2, (WK_EXPR_CLBACK)srep_etest, NULL, (Generic)parm);
	}

	return WALK_CONTINUE;
}

/*-------------------------------------------------------------------

	srep_stest_post()	Walk routine to collect var occurrences
						For statements (post_action)

	Restore multiplier loops where var is loop invariant

*/

static int
srep_stest_post(AST_INDEX stmt, int level, ReplaceS_parm *parm, PedInfo ped)
{
	if (is_do(stmt))
	{
		if (pt_invariant_subs(stmt, parm->var))
		{
			parm->mult -= ITER;		/* decrease coeff	*/
		}
	}
	return WALK_CONTINUE;
}


/*-------------------------------------------------------------------

	srep_etest()	Walk routine to collect var occurrences
						For expressions
*/

static int
srep_etest(AST_INDEX expr, ReplaceS_parm *parm)
{

	if (is_subscript(expr) && pt_expr_equal(expr, parm->var))
	{
		parm->read += 1 << parm->mult;
		parm->locs[parm->refs++] = expr;
	}

	return WALK_CONTINUE;
}

/*-------------------------------------------------------------------

	pt_invariant_test()		Walk routine to check for variance
							For expressions
*/

static int
pt_invariant_test(AST_INDEX expr, Invariant_parm *parm)
{
	char *str;
	int i;

	if (is_identifier(expr))
	{
		str = gen_get_text(expr);

		for (i = 0; i < parm->level; i++)
		{
			if (SYM_EQ2(parm->ivar[i], str))
				return WALK_CONTINUE;
		}

		parm->invariant = false;
		return WALK_ABORT;
	}

	return WALK_CONTINUE;
}


