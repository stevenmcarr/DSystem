/* $Id: ip_perfwalk.C,v 1.20 1997/03/11 14:32:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * This module contains routines to compute an expression for the estimated
 * execution time for a module (an AST). The expression generated may
 * contain symbolics. The information computed here will be written to a
 * file to be used in interprocedural analysis. The basic steps taken by
 * the performance estimator are as follows:
 * 
 * 1)   read in performance data for the target architecture
 * 2)   for each subroutine/function/program in the module,
 * 2.1) build a control dependence graph
 * 2.2) walk the control dependence graph, building up estimates for each
 *      statement
 * 2.3) accumulate the estimate into a 'PerfEstExpr' object
 * 2.4) write out the completed 'PerfEstExpr' to the database
 *
 * Main entry point: perf_local_phase_walkcd
 */

/* Revision History:
 *
 * $Log: ip_perfwalk.C,v $
 * Revision 1.20  1997/03/11 14:32:10  carr
 * newly checked in as revision 1.20
 *
 * Revision 1.20  94/01/24  21:47:39  mcintosh
 * Fix a bug and get rid of a compiler warning.
 * 
 * Revision 1.19  93/12/17  14:55:42  rn
 * made include paths relative to the src directory. -KLC
 * 
 * Revision 1.18  93/08/11  16:37:27  mcintosh
 * Change method for mapping AST indices to partial performance
 * estimates; clean up some debugging crud; improve comments.
 * 
 * Revision 1.17  93/08/10  14:17:11  curetonk
 * minor bug fix
 * 
 * Revision 1.16  93/06/30  16:54:47  johnmc
 * use FileContext and FormattedFile now for I/O
 * 
 * Revision 1.15  93/06/11  14:14:55  patton
 * made changes to allow compilation on Solaris' CC compiler
 * 
 * Revision 1.14  93/05/25  15:59:21  curetonk
 * *** empty log message ***
 * 
 * Revision 1.13  93/05/17  11:17:11  mcintosh
 * Various changes: 1) add an option which checks to make sure
 * that a module contains a single entry point with a given name,
 * 2) add support for getting the symbolic expressions for 
 * conditionals/guards
 * 3) add an option for disabling loop-bounds/array-bounds
 * guessing heuristic
 * 
 * Revision 1.12  93/04/28  10:48:49  mcintosh
 * Numerous changes. Move some structure defs, etc. to ip_perfwalk.h
 * to support ip_perfutil.C; add loop upper bound guesses based on
 * array access patterns; do a better job freeing estimates while
 * making sure we don't have problems with unstructured control
 * flow; changes to debugging code.
 * 
 * Revision 1.11  93/03/31  11:05:41  mcintosh
 * Recode to eliminate a warning msg on the RS6000.
 * 
 * Revision 1.10  93/03/19  09:34:01  rn
 * Added include of stdio.h to get definition of sprintf.
 * 
 * Revision 1.9  93/01/21  10:48:31  reinhard
 * Reintroduced perf_pt_get_loop_bounds() to become independent of
 * PED again.  Eventually, this utulity should be moved to a place
 * outside of PED (now it is in pt_util.c).
 * 
 * Revision 1.8  93/01/19  15:43:42  reinhard
 * Included pt_util.h
 * 
 * Revision 1.7  93/01/19  15:27:02  reinhard
 * Removed get_loop_bounds(); call pt_get_loop_bounds() instead.
 * 
 * Revision 1.6  92/12/14  21:57:48  mcintosh
 * Improve comments. Better handling of block data statements. 
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perfwalk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perf.h>

#include <libs/graphicInterface/oldMonitor/OBSOLETE/db/FileContext.h>
#include <libs/support/file/FormattedFile.h>

/*
 * Debugging flag. Values:
 * 
 *	0 - no debugging information
 *	1 - some debugging information
 *	2 - lots of debugging information
 * 
 * The value this variable takes on is set at the beginning of the CDG walk
 * (i.e. the initialized value is ignored).
 */
int pe_debug;

FILE *pe_debugfp;		/* file to write debug output to */

/* Forward declarations
*/
static PerfEstExprHandle add_sum_expr(Treewalk *walk,
				      PerfEstExprHandle s1,
				      PerfEstExprHandle s2);
static void add_expr_to_list(Treewalk *walk, PerfEstExprHandle handle,
			     AST_INDEX ai, cdNode *cdn);
static PerfEstExprHandle add_constant(Treewalk *walk, double dval);
static PerfEstExprHandle add_bottom(Treewalk *walk);
static PerfEstExprHandle simplify_expr(Treewalk *walk,
				       PerfEstExprHandle handle);
static PerfEstExprHandle examine_loop_bound(Treewalk *walk, AST_INDEX astnode);
static void set_parallel_cost(Treewalk *walk, PerfEstExprHandle handle);
static PerfEstExprHandle add_loop(Treewalk *walk, PerfEstExprHandle lb_handle,
		    PerfEstExprHandle ub_handle, PerfEstExprHandle step_handle,
		    PerfEstExprHandle body_cost, int node_number, 
		    int iterations_guess, int is_parallelizable,
		    int is_marked_parallel, double parallel_etime,
		    int nprocs, AST_INDEX astindex);
static int get_stmt_node_number(Treewalk *walk, AST_INDEX stmt);
static int get_loop_is_parallelizable(Treewalk *walk, AST_INDEX stmt);
static PerfEstExprHandle add_if(Treewalk *walk,
				PerfEstExprHandle true_branch_cost,
				PerfEstExprHandle false_branch_cost,
				PerfEstExprHandle cond_expr,
				double branch_prob);
static PerfEstExprHandle add_call(Treewalk *walk, int node_number,
				  char *name, int isinvocation,
				  AST_INDEX actual_list);
// static void print_elist(Treewalk *w);


/*---------------*/

/*
 * Print out information about a node in the control dependence graph, for
 * debugging purposes.
 */

char *pr_cdNode(cdNode *n, char *buf)
{
  char b[50];
  cdEdge *e;

  if (!n) {
    sprintf(buf, "%s", "#");
  } else {
    sprintf(buf, "cdn=0x%x(", n);
    if (!n->pred) {
      strcat(buf, "<no pred>");
    } else {
      sprintf(b, "srcs:0x%x", n->pred->src);
      strcat(buf, b);
      for (e = n->pred->next_pred; e; e = e->next_pred) {
	sprintf(b, ",0x%x", e->src);
	strcat(buf, b);
      }
    }
    strcat(buf, ")");
  }
  return buf;
}

/*
 * For debugging purposes, we would like to have a mapping between nodes
 * in the AST and partial performance estimates. This routine builds a
 * data structure (walkdata.smap) which accomplishes this mapping.
 */

static void debug_add_est_to_list(Treewalk *walk,
				  AST_INDEX stmt,
				  PerfEstExprHandle cost)
{
  ASTtoPerfMapListEntry *e;
  int isp;
  double nv;

  if (walk->t->is_bottom(cost))
    nv = -99.0;
  else walk->t->get_estimate(cost, nv, isp);
  e = new ASTtoPerfMapListEntry(stmt, nv);
  walk->smap->Append(e);
}

/*
 * Look up an AST node in the STMT->ESTIMATE map, and return the mapping
 * if there is one.
 */

double pe_debug_map_stmt_to_est(ASTtoPerfMapList *l, AST_INDEX ind)
{
  /* Search through the list until we find it.
   */
  ASTtoPerfMapListIterator it(l);
  ASTtoPerfMapListEntry *cur;

  for (; it.Current(); it++) {
    cur = it.Current();
    if (cur->ai() == ind)
      return cur->pe();
  }
  return -1.0;
}

/*
 * The following set of function are callbacks used by the 'jumptrans_...'
 * code for building symbolic expressions.
 * 
 * This is a mess, but it's necessary for software engineering reasons. Sorry
 * for writing such abstruse code. See "jumptrans.C" and "jumptrans.h" for
 * more information (they live with the value numbering code).
 */

static
JumpTransPtr callback_create_iconst(int value)
{
  PerfEstExprNode dummy;

  return (JumpTransPtr) dummy.NewIconst(value);
}

static
JumpTransPtr callback_create_bottom()
{
  PerfEstExprNode dummy;

  return (JumpTransPtr) dummy.NewNode(SYMEXPR_ARITH_BOTTOM);
}

static
JumpTransPtr callback_create_var(char *varname)
{
  PerfEstExprNode dummy;

  return (JumpTransPtr) dummy.NewVar(varname);
}

static
JumpTransPtr callback_create_op(int op, JumpTransPtr left,
				JumpTransPtr right)
{
  PerfEstExprNode *left_e = (PerfEstExprNode *) left;
  PerfEstExprNode *right_e = (PerfEstExprNode *) right;
  PerfEstExprNode dummy;
  SymExprNodeType otype;

  switch (op) {
    case GEN_BINARY_PLUS:
      otype = SYMEXPR_ARITH_ADD;
      break;
    case GEN_BINARY_MINUS:
      otype = SYMEXPR_ARITH_SUB;
      break;
    case GEN_BINARY_TIMES:
      otype = SYMEXPR_ARITH_MULT;
      break;
    case GEN_BINARY_DIVIDE:
      otype = SYMEXPR_ARITH_DIV;
      break;
    case GEN_BINARY_EQ:
      otype = SYMEXPR_COND_EQ;
      break;
    case GEN_BINARY_NE:
      otype = SYMEXPR_COND_NE;
      break;
    case GEN_BINARY_LE:
      otype = SYMEXPR_COND_LE;
      break;
    case GEN_BINARY_GE:
      otype = SYMEXPR_COND_GE;
      break;
    case GEN_BINARY_LT:
      otype = SYMEXPR_COND_LT;
      break;
    case GEN_BINARY_GT:
      otype = SYMEXPR_COND_GT;
      break;
    case GEN_BINARY_AND:
      otype = SYMEXPR_LOG_AND;
      break;
    case GEN_BINARY_OR:
      otype = SYMEXPR_LOG_OR;
      break;
    case GEN_UNARY_NOT:
      otype = SYMEXPR_LOG_NOT;
      break;
    default:
      return 0;
    }
  
  return (JumpTransPtr) dummy.NewArithOp(otype, left_e, right_e);
}

/*
 * The cost of a procedure call can be broken into two parts: the cost to
 * actually perform the control transfer, and the cost to execute the body
 * of the called routine. We know the former, but can't get the latter
 * without interprocedural info.
 * 
 * The parameter 'isinvocation' is set to 1 if this is a function call, as
 * opposed to a procedure call (0).
 */

static PerfEstExprHandle add_call(Treewalk *walk, int node_number,
				  char *fname, int isinvocation,
				  AST_INDEX actual_list)
{
  AST_INDEX alist;
  int actual_count = 0;
  PerfEstExprNode **e_list;
  PerfEstExprHandle control_xfer_cost, body_cost;
  double ccost;

  /* Count the actuals
  */
  alist = list_first(actual_list);
  while (alist != ast_null_node) {
    actual_count++;
    alist = list_next(alist);
  }

  /* First the control transfer cost */
  ccost = (isinvocation ?
	   (PE_COST_FCALL(walk->pdata, actual_count)) :
           (PE_COST_PCALL(walk->pdata, actual_count)));
  control_xfer_cost = add_constant(walk, ccost);

  /*
   * Now a representation of the called routine cost. This involves
   * invoking symbolic analysis to see if the actuals at the call site are
   * well-behaved, and if so, recording them symbolically.
   */
  if (actual_count > 0) {
    e_list = new PerfEstExprNode_ptr[actual_count];
    actual_list_to_jumpfunction_list(fname,
				     actual_list,
				     actual_count,
				     (JumpTransPtr *) e_list,
				     walk->jumptrans);
  } else e_list = 0;
  body_cost = walk->t->c_call(node_number, fname,
			      actual_count, (PerfEstExprNode **) e_list);
  
  /* Sum them together */
  return add_sum_expr(walk, control_xfer_cost, body_cost);
}

/* Branch probabilities are currently guessed.
*/

static PerfEstExprHandle add_if(Treewalk *walk,
				PerfEstExprHandle true_branch_cost,
				PerfEstExprHandle false_branch_cost,
				PerfEstExprHandle cond_expr,
				double branch_prob)
{
  return walk->t->c_ifstmt(branch_prob,
			   cond_expr,
			   true_branch_cost,
			   false_branch_cost);
}

/* Create a symbolic expression for a loop.
*/
		  
static PerfEstExprHandle add_loop(Treewalk *walk, PerfEstExprHandle lb_handle,
		    PerfEstExprHandle ub_handle, PerfEstExprHandle step_handle,
		    PerfEstExprHandle body_cost, int node_number, 
		    int iterations_guess, int is_parallelizable,
		    int is_marked_parallel, double parallel_etime,
		    int nprocs, AST_INDEX astindex)
{
  AST_INDEX l = AST_NIL;
  STR_INDEX sym = AST_NIL;
  STR_TEXT  text = "";
  
  /* Get the loop label if possible
   */
  if (is_do(astindex))
    l = gen_DO_get_lbl_ref(astindex);
  else if (is_parallelloop(astindex))
    l = gen_PARALLELLOOP_get_lbl_ref(astindex);
  if (l != AST_NIL) {
    sym = ast_get_symbol(l);
    text = string_table_get_text(sym);
  }
  
  return walk->t->c_loop(node_number, body_cost,
			       lb_handle,  ub_handle,
			       step_handle,  iterations_guess,
			       is_parallelizable,  is_marked_parallel,
			       parallel_etime,  nprocs, (int) astindex,
			       text);
}

static PerfEstExprHandle add_constant(Treewalk *walk, double dval)
{
  return walk->t->c_constant(dval);
}

static PerfEstExprHandle add_bottom(Treewalk *walk)
{
  return walk->t->c_bottom();
}

static PerfEstExprHandle add_sum_expr(Treewalk *walk,
			       PerfEstExprHandle s1,
			       PerfEstExprHandle s2)
{
  PerfEstExprHandle ns1, ns2;

  /*
   * Clone the expressions if they are already part of some existing
   * expression tree. This can happen if the node in the control
   * dependence graph corresponding to s1 (or s2) has more than one
   * predecessor.
   * 
   * Currently we check this by seeing if the expression already has a
   * parent; this is something of a hack since it relies on the implementation
   * of the expression tree. Really what we should do is check the CDG
   * node corresponding to the statement for s1, s2.
   */
  ns1 = s1;
  ns2 = s2;
  if (walk->t->handle_parent(ns1))
    ns1 = walk->t->clone_handle(ns1);
  if (walk->t->handle_parent(ns2))
    ns2 = walk->t->clone_handle(ns2);
  return walk->t->c_sum(ns1, ns2);
}

static int get_loop_is_parallelizable(Treewalk *walk, AST_INDEX stmt)
{
#if 0
  return query_parloop_map(walk, stmt);
#endif
  return 0;
}

static int get_stmt_node_number(Treewalk *walk, AST_INDEX stmt)
{
  return ft_NodeToNumber(walk->ft, stmt);
}

/*
 * Try to determine the cost of executing a loop in parallel. Requires
 * that we have a lot of things known at compile time. 
 */

static void set_parallel_cost(Treewalk * walk, PerfEstExprHandle handle)
{
  int iters, isp, nprocs;
  double bodycost, parallel_etime;
  
  /*
   * Get information about the loop.
   */
  walk->t->get_loop_info_estimates(handle, iters, bodycost, isp);
  
  /*
   * Examine the training set data to get the body cost and the
   * parallel execution time. Then set the values in the table.
   */
  PE_COST_PARLOOP(walk->pdata, bodycost, iters,
		  &parallel_etime, &nprocs);
  walk->t->set_parallel_info(handle, parallel_etime, nprocs);
  return;
}

/* Argument must be an integer constant
 */

static int convert_constant_expr_to_int(AST_INDEX n)
{
  int f;
  char *t1, *t2;
  
  if (n == AST_NIL)
    return 0;
  t1 = gen_get_text(n);
  f = (int) strtol(t1, &t2, 0);
  if (t2 == t1)
    fprintf(stderr, "perf_local_phase: illegal integer constant('%s')\n",
	    t1);
  return f;
}

/*
 * Look at a loop bound. Try to figure out if it has a constant value, or
 * whether a jump function can be built for it.
 */

static PerfEstExprHandle examine_loop_bound(Treewalk * walk, AST_INDEX astnode)
{
  PerfEstExprHandle rval;

  /*
   * Optimize the most common case.
   */
  if (is_constant(astnode) && gen_get_real_type(astnode) == TYPE_INTEGER) {
    int val;
    val = convert_constant_expr_to_int(astnode);
    return walk->t->c_constant((double) val);
  }

  /* Call a more powerful routine.
  */
  if (rval = (PerfEstExprHandle)
      loopbound_expr_to_jumpfunction(astnode, walk->jumptrans))
    return rval;

  /*
   * No information-- set the value to bottom.
   */
  return walk->t->c_bottom();
}

/*
 * The walk of the control dependence graph is done in postorder, which
 * means that if the procedure contains a high-level construct like a DO
 * or IF, we see the body of the DO or the arms of the conditional before
 * we see the statement itself. As a result, we simply store the partial
 * performance estimates in a list and deal with them later.
 */

static void add_expr_to_list(Treewalk *walk, PerfEstExprHandle handle,
			     AST_INDEX ai, cdNode *cdn)
{
  walk->elist->create_and_append_item(ai, cdn, handle);

  if (pe_debug >= 10) {
    char buf[256];
    fprintf(pe_debugfp, "+ add  : entry=0x%x ai=%07d e=0x%x %s\n",
	   walk->elist->Last(), ai, handle,
	   pr_cdNode(cdn, buf));
  }
}

/*
 * When we build a symbolic expression for the cost of executing a
 * statement, it's done in a very simpleminded way. For example, if the
 * performance estimator encounters the statement "a = b + c - max(q,r)",
 * it will most likely generate which is the sum of a bunch of constants,
 * when all we really need is a single constant.
 * 
 * This routine calls another function which simplifies an expression (mainly
 * constant folding). 
 */

static PerfEstExprHandle simplify_expr(Treewalk *walk,
				       PerfEstExprHandle handle)
{
  PerfEstExprHandle h;

  h = walk->t->simplify_expr(handle);

  /* this should no longer apply */
#if 0
  /*
   * HACK CITY -- this is a memory leak (i.e. commenting out the
   * "delete"). Fix this before releasing. Need to have PerfWalkList's
   * *own* the expressions which are put on them, and have them free those
   * expressions when they are done.
   */
#endif

  delete handle;

  return h;
}

static void add_loop_bound_guess(PerfLoopStackEntry *e,
				 int num_dims,
				 int guess,
				 Boolean is_assign)
{
  if (!(e->guess.valid) ||
      (is_assign && !e->guess.assign) ||
      num_dims > e->guess.dims) {
    if (pe_debug >= 2) {
      fprintf(pe_debugfp,
	      "New guess for '%s' loop: %d iters (assign=%d, dims=%d)\n",
	      e->indvar,
	      guess, is_assign, num_dims);
    }
    e->guess.valid = true;
    e->guess.intg = guess;
    e->guess.dims = num_dims;
    e->guess.assign = is_assign;
  }
}

static int perf_get_loop_best_guess(Treewalk *walk, AST_INDEX stmt)
{
  char *debugdisable = getenv("PERF_LOOP_BOUND_GUESS_CONTROL");

  if (!debugdisable || !strcmp(debugdisable,"off")) {

    /*
     * The loop we're looking at will be the top one on the loop stack.
     * Check the AST index just to be sure.
     */
    PerfLoopStackEntry *e = walk->loopstack->first_entry();
    
    if (e->node != stmt) {
      fprintf(stderr,
	      "internal error file %s line %d (loop stack node mismatch)\n",
	      __FILE__, __LINE__);
      return 10;
    }
    
    if (e->guess.valid) {
      if (pe_debug >= 2) {
	fprintf(pe_debugfp,
		"Using dimension-based guess of %d iters for '%s' loop\n",
		e->guess.intg, e->indvar);
      }
      return e->guess.intg;
    }
    
    if (pe_debug >= 2)
      fprintf(pe_debugfp,
	      "Using default guess of %d iters for '%s' loop\n",
	      PERF_GUESS_LOOP_ITERATIONS, e->indvar);
  }
  return PERF_GUESS_LOOP_ITERATIONS;
}

static void perf_examine_subscript(Treewalk *walk, AST_INDEX subs)
{
  AST_INDEX elmnt, namenode;
  int numlinear, i;
  char *ivar;
  PerfLoopStackIterator *it;
  PerfLoopStackEntry *e;
  int coeff;
  Boolean lin;
  int num_dims_decl;
  ArrayBound *dimarray;
  char *name;

  if (!walk->loopstack)
    return;

  namenode = gen_SUBSCRIPT_get_name(subs);
  name = string_table_get_text(ast_get_symbol(namenode));

  num_dims_decl = (int) fst_GetField(walk->sd, name, SYMTAB_NUM_DIMS);
  dimarray = (ArrayBound *) fst_GetField(walk->sd, name, SYMTAB_DIM_BOUNDS);

  elmnt = list_first(gen_SUBSCRIPT_get_rvalue_LIST(subs));
  for (i = 0; elmnt != AST_NIL; elmnt = list_next(elmnt), i++) {

    numlinear = 0;
    it = new PerfLoopStackIterator(walk->loopstack);
    for (; it->Current(); (*it)++) {

      /* Get the induction variable for this loop.
      */
      e = it->Current();
      ivar = e->indvar;

      /*
       * Does the induction variable appear in this subscript position? If
       * so, then add a new guess to the loop record in question. If
       * the array is declared as of size 1, this is probably an indication
       * of variable length, so ignore these as well.
       */
      coeff = 0;
      pt_get_coeff(elmnt, ivar, &lin, &coeff);
      if (lin && coeff)
	if (dimarray[i].ub.type == constant &&
	    (dimarray[i].ub.value.const_val != 1)) {
	  AST_INDEX p = tree_out(subs);
	  Boolean is_assign = (is_assignment(p) ? true : false);
	  add_loop_bound_guess(e,
			       num_dims_decl,
			       dimarray[i].ub.value.const_val,
			       is_assign);
	}
    }
  }
}

/*
 * This routine is called by "walk_expression" when the main control
 * dependence walk sees an expression such as the one you might find on
 * the right hand side of an assignment statement. For each component of
 * the expression, we try to determine what the component is, and if we
 * recognize it, we try to look it up in our training set. If it's in the
 * training set, then we add it to the symbolic cost expression which we
 * are accumulating.
 */

static int perf_expr_postwalk(AST_INDEX node, Treewalk *walk)
{
  Perf_data *pdata = walk->pdata;
  int typ = pe_get_type_of_binary_operator(node);

  if (typ == TYPE_LOGICAL) {
    if (is_binary_and(node) || is_binary_or(node) || is_unary_not(node))
      add_expr_to_list(walk, add_constant(walk, PE_COST_LOGOP(pdata)),
		       node, (cdNode *) 0);
    else if (is_binary_lt(node)  || is_binary_gt(node) ||
	     is_binary_eq(node)  || is_binary_ne(node) ||
	     is_binary_le(node)  || is_binary_ge(node) ||
	     is_binary_eqv(node) || is_binary_neqv(node))
    add_expr_to_list(walk, add_constant(walk, PE_COST_CMP(pdata)),
		     node, (cdNode *) 0);
  } else if (is_binary_plus(node))
    add_expr_to_list(walk, add_constant(walk, PE_COST_ADD(pdata, typ)),
		     node, (cdNode *) 0);
  else if (is_binary_minus(node))
    add_expr_to_list(walk, add_constant(walk, PE_COST_SUB(pdata, typ)),
		     node, (cdNode *) 0);
  else if (is_binary_times(node))
    add_expr_to_list(walk, add_constant(walk, PE_COST_MULT(pdata, typ)),
		     node, (cdNode *) 0);
  else if (is_binary_divide(node))
    add_expr_to_list(walk, add_constant(walk, PE_COST_DIV(pdata, typ)),
		     node, (cdNode *) 0);
  else if (is_binary_exponent(node))
    add_expr_to_list(walk, add_constant(walk, PE_COST_EXP(pdata, typ)),
		     node, (cdNode *) 0);
  else if (is_subscript(node)) {
    int num_dims = pe_get_subscript_num_dims(node);

    /* Start by assignng a cost for this array reference. 
    */
    if (typ != TYPE_INTEGER && typ != TYPE_REAL && 
	typ != TYPE_DOUBLE_PRECISION && typ != TYPE_COMPLEX)
      typ = TYPE_REAL;
    add_expr_to_list(walk, add_constant(walk,
					PE_COST_ARR(pdata, num_dims, typ)),
		     node, (cdNode *) 0);

    /*
     * Next try to use it to guess the upper bound of the enclosing loop.
     */
    perf_examine_subscript(walk, node);

  } else if (is_invocation(node)) {
    char *name = gen_get_text(gen_INVOCATION_get_name(node));
    int nargs;
    
    /* Check to see if it is a generic or an intrinsic
     */
    if (builtins_isGenericFunction(name)) {
      pe_get_generic_info(node, &typ, &nargs);
      add_expr_to_list(walk,
		       add_constant(walk,
				    PE_COST_GENERIC(pdata, name, typ, nargs)),
		       node, (cdNode *) 0);
    } else if (builtins_isIntrinsicFunction(name))
      add_expr_to_list(walk, add_constant(walk,
					  PE_COST_INTRINSIC(pdata, name)),
		       node, (cdNode *) 0);
    else {
      
      /*
       * Check to see if it is a statement function invocation. If it is,
       * then just ignore it (this is a temporary solution -- eventually
       * we want to come up with a solution similar to the one used by the
       * mod/ref analysis -- expand the stmt function on the fly).
       */
      int storage_class = fst_GetField(walk->sd, name, SYMTAB_STORAGE_CLASS);
      if (storage_class & SC_STMT_FUNC) {
	/* Throw in a 0 */
	add_expr_to_list(walk, add_constant(walk, 0.0),
			 node, (cdNode *) 0);
      } else {
	PerfEstExprHandle call_cost;
	
	/* Record the call
	 */
	call_cost = add_call(walk, get_stmt_node_number(walk, node),
			     name,
			     1, /* function invokcation, not proc call */
			     gen_INVOCATION_get_actual_arg_LIST(node));
	add_expr_to_list(walk, call_cost,
			 node, (cdNode *) 0);
      }
    }
  } else {
    /*
     * If we haven't recognized the node by now, then it's probably
     * something we're not interested in (example: a constant).
     */
  }
  
  return (WALK_CONTINUE);
}

/*
 * This routine is called during the walk of the control dependence graph,
 * when we encounter some statement in the program which has other
 * statements control dependent on it.
 *
 * Suppose we have the following IF statement in our program:
 * 
 * 1   IF (x .lt. y) then
 * 2      a = a + b
 * 3      b(1,2) = 3
 * 4   ELSE
 * 5      a = a  + c ** r
 * 6   ENDIF
 * 
 * When we walk the CDG for this program, the walk will encounter the
 * statements in the order (4,3,2,1). When we see statements 2,3, and 4,
 * we don't yet know how to interpret them, so we generate estimates for
 * them and push the estimates onto a list. When we see statement 1, then
 * we need to figure out which statements on the list correspond to the
 * TRUE branch of the conditional and which correspond to the FALSE
 * branch. This routine deals with the situation. It looks through the
 * list of estimates and picks out and sums up selected estimates,
 * depending on the value of the 'flag' varaible passed to this routine.
 * For example, if I only want to sum up the estimates for the TRUE arm of
 * the condition, I pass the value "ONLY_TRUE_CONTROLDEP_LIST_ELEMENTS", then
 * the routine will sum up the estimates on the list corresponding only to
 * the statements which have 'TRUE' control dependences to them.
 */

static
PerfEstExprHandle sum_list(cdNode *cdn, Treewalk *walk,
			   CDGWalkList_control flag)
{
  PerfWalkListIterator i(walk->elist, cdn, flag);
  PerfWalkListEntry *cur;
  PerfEstExprHandle cur_e, sum;
  
  if (pe_debug >= 10) {
    fprintf(pe_debugfp, "sum_list(cdn=0x%x,flag=%s):\n",
	   cdn, (flag == ALL_LIST_ELEMENTS ? "ALL" :
		 (flag == ALL_CONTROLDEP_LIST_ELEMENTS ? "CDEP" :
		  (flag == ONLY_TRUE_CONTROLDEP_LIST_ELEMENTS ? 
		   "TRUE" : "FALSE"))));
  }

  sum = walk->t->c_constant(0.0);
  for (; cur = i.current(); cur = i.next_entry()) {
    cur_e = cur->expr->clone();

    if (pe_debug >= 10) {
      char buf[256];
      fprintf(pe_debugfp, "+ sum  : entry=0x%x ai=%07d e=0x%x %s\n",
	     cur, cur->ai, cur->expr, pr_cdNode(cur->cdn, buf));
    }
    sum = add_sum_expr(walk, cur_e, sum);
  }
  return sum;
}

/*
 * Prune certain elements from a list of partial estimates. Depending on
 * the value of 'flag', remove all elements or only those elements
 * corresponding to a particular type of control dependence. [see also the
 * comment for the routine "sum_list".
 */

static void prune_list(cdNode *cdn, Treewalk *walk,
		       CDGWalkList_control flag)
{
  if (pe_debug >= 10) {
    char buf[256];
    PerfWalkListIterator i(walk->elist, cdn, flag);
    PerfWalkListEntry *cur;
    
    fprintf(pe_debugfp, "prune_list(cdn=0x%x,flag=%s):\n",
	   cdn, (flag == ALL_LIST_ELEMENTS ? "ALL" :
		 (flag == ALL_CONTROLDEP_LIST_ELEMENTS ? "CDEP" :
		  (flag == ONLY_TRUE_CONTROLDEP_LIST_ELEMENTS ? 
		   "TRUE" : "FALSE"))));
    for (; cur = i.current(); cur = i.next_entry())
      fprintf(pe_debugfp, "- prune: entry=0x%x ai=%07d e=0x%x %s\n",
	     cur, cur->ai, cur->expr, pr_cdNode(cur->cdn, buf));
  }

  walk->elist->prune_list(cdn,
			 flag,
			 true /* refcount the nodes */
			 );

  if (pe_debug >= 10) {
    char buf[256];
    PerfWalkListIterator i(walk->elist, cdn, ALL_LIST_ELEMENTS);
    PerfWalkListEntry *cur;
    
    fprintf(pe_debugfp, "list after pruning:\n");
    for (; cur = i.current(); cur = i.next_entry())
      fprintf(pe_debugfp, "+      : entry=0x%x ai=%07d e=0x%x %s\n",
	     cur, cur->ai, cur->expr, pr_cdNode(cur->cdn, buf));
  }
}

/*
 * When copying the treewalk structures during the control dependence walk,
 * we have to be careful not to copy the `elist' field, so as to achieve the
 * correct result when dealing with things like assignments.
 */

static Treewalk *Treewalk_clone(Treewalk *w)
{
  Treewalk *n = new Treewalk;
  *n = *w;
  n->elist = new PerfWalkList;
  return n;
}

/*
 * This should only be used with structures created by 'Treewalk_clone'.
 */

static void Treewalk_delete(Treewalk *w)
{
  delete w->elist;
  delete w;
}

static Treewalk *Treewalk_new()
{
  Treewalk *n = new Treewalk;
  n->elist = new PerfWalkList;
  n->loopstack = new PerfLoopStack;
  return n;
}

/*
 * The following function gets called after we finish with a single
 * function. It cleans up the 'treewalk' structure in preparation for the
 * next function.
 */

static void Treewalk_endfunction(Treewalk *w)
{
  /*
   * Delete the contents of the mapping lists, then delete the list
   * elements themselves.
   */
  delete w->smap;
  delete w->elist;
  delete w->loopstack;
  w->loopstack = new PerfLoopStack;
  w->elist = new PerfWalkList;

  /* Delete the performance estimate.
  */
  delete w->t;
  w->t = 0;
}

/*
 * Most of the code which calculates estimates of expressions, etc is done
 * in a bottom up (in the forward CDG) fashion. However we would like to
 * be able to look at the enclosing loops for an expression, and this has
 * to be calculated top-down, so as a result this routine is necessary. 
 */

static int perf_cd_walk_pre_action(cdNode *cdn, Treewalk *walk)
{
  AST_INDEX stmt = cdn->stmt;
  AST_INDEX lb, ub, st, ivar;
  char *indvar;
  Boolean istriang;
  PerfLoopGuess ges;
  PerfLoopStackEntry *entry;

  if (is_do(stmt) || is_parallelloop(stmt)) {
    pt_get_loop_bounds(stmt, &ivar, &lb, &ub, &st);
    indvar = string_table_get_text(ast_get_symbol(ivar));
    istriang = false;
    ges.valid = false;
    entry = new PerfLoopStackEntry(stmt, indvar, istriang, ges);
    if (!walk->loopstack)
      walk->loopstack = new PerfLoopStack;
    walk->loopstack->push_entry(entry);
  }
  return WALK_CONTINUE;
}

/*
 * This routine is the guts of the control-dependence walk; it is called
 * by "cdg_walk_nodes". Parameters: 'cdn' is the node in the control
 * dependence graph which we are visiting, and 'walk' is the treewalk data
 * structure which is passed around during the walk (this is just a
 * dumping ground for information and partial results).
 * 
 * The basic goal of this function is to build up a performance estimate for
 * the statement being visited and add it to the list "elist" in the
 * "walk" structure. When this function is called for a given statement S,
 * all of the statements which are control-dependent on S have already
 * been visited, so the list "elist" will already contain estimates for
 * those statements.
 */

static int perf_cd_walk_post_action(cdNode *cdn, Treewalk *walk)
{
  AST_INDEX stmt = cdn->stmt;
  Perf_data *pdata = walk->pdata;
  PerfEstExprHandle total_cost = PERFEST_EXPR_HANDLE_NULL;

  if (is_assignment(stmt)) {
    Treewalk *lhs_walk, *rhs_walk;
    AST_INDEX lhs, rhs;
    PerfEstExprHandle result, rhs_cost, lhs_cost;
    PerfEstExprHandle assign_cost;

    /* Get the left and right hand sides.
     */
    lhs = gen_ASSIGNMENT_get_lvalue(stmt);
    rhs = gen_ASSIGNMENT_get_rvalue(stmt);

    /* Walk the right hand side of the assignment.
    */
    rhs_walk = Treewalk_clone(walk);
    walk_expression(rhs, NULL, ( WK_EXPR_CLBACK) perf_expr_postwalk, (Generic) rhs_walk);
    rhs_cost = sum_list(cdn, rhs_walk, ALL_LIST_ELEMENTS);
    result = rhs_cost;
    Treewalk_delete(rhs_walk);

    /* Walk the left hand side as well, if it's an array reference
    */
    if (is_subscript(lhs)) {
      lhs_walk = Treewalk_clone(walk);
      walk_expression(lhs, NULL, ( WK_EXPR_CLBACK) perf_expr_postwalk, 
                       (Generic) lhs_walk);
      lhs_cost = sum_list(cdn, lhs_walk, ALL_LIST_ELEMENTS);
      result = add_sum_expr(walk, result, lhs_cost);
      Treewalk_delete(lhs_walk);
    }

    /* Store away the resulting information.
    */
    assign_cost = add_constant(walk, PE_COST_ASSIGN(pdata));
    total_cost = add_sum_expr(walk, result, assign_cost);
    total_cost = simplify_expr(walk, total_cost);
    add_expr_to_list(walk, total_cost, stmt, cdn);

    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);

  } else if (is_goto(stmt)) {

    total_cost = add_constant(walk, PE_COST_GOTO(pdata));
    add_expr_to_list(walk, total_cost, stmt, cdn);

    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);

  } else if (is_do(stmt) || is_parallelloop(stmt)) {
    PerfEstExprHandle body_cost, lb_handle, ub_handle, step_handle;
    AST_INDEX lb, ub, st, ivar;
    double parallel_etime = 0;
    int nprocs = 0, iterations_guess;
    
    /* Summarize the cost of the body of the loop
     */
    body_cost = sum_list(cdn, walk, ALL_CONTROLDEP_LIST_ELEMENTS);
    prune_list(cdn, walk, ALL_CONTROLDEP_LIST_ELEMENTS);
    body_cost = simplify_expr(walk, body_cost);

    /* Walk the expressions for the bounds and the step.
    */
    pt_get_loop_bounds(stmt, &ivar, &lb, &ub, &st);
    lb_handle = examine_loop_bound(walk, lb);
    ub_handle = examine_loop_bound(walk, ub);
    if (st == AST_NIL)
      step_handle = add_constant(walk, (double) 1);
    else step_handle = examine_loop_bound(walk, st);

    /* Make a new loop node and fill in the parallel info.
     */
    iterations_guess = perf_get_loop_best_guess(walk, stmt);
    walk->loopstack->pop_entry();
    total_cost = add_loop(walk, lb_handle, ub_handle, step_handle, body_cost,
			 get_stmt_node_number(walk, stmt),
			 iterations_guess,
			 get_loop_is_parallelizable(walk, stmt),
			 is_parallelloop(stmt) ? 1 : 0,
			 parallel_etime, nprocs, stmt);
    set_parallel_cost(walk, total_cost);
    add_expr_to_list(walk, total_cost, stmt, cdn);

    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);

  } else if (is_if(stmt) || is_guard(stmt) || is_logical_if(stmt)) {
    PerfEstExprHandle true_branch_cost, false_branch_cost;
    PerfEstExprHandle gcost;
    AST_INDEX cexpr, guardlist, guard;
    Treewalk *gwalk;
    PerfEstExprHandle condval;
    
    /* Get the conditional expression for this particular conditional.
     */
    if (is_guard(stmt)) {
      cexpr = gen_GUARD_get_rvalue(stmt);
    } else if (is_if(stmt)) {
      guardlist = gen_IF_get_guard_LIST(stmt);
      guard = list_first(guardlist);
      cexpr = gen_GUARD_get_rvalue(guard);
    } else if (is_logical_if(stmt)) {
      cexpr = gen_LOGICAL_IF_get_rvalue(stmt);
    } else assert(1 == 0);
    
    /* Walk the conditional expression to get its cost
     */
    gwalk = Treewalk_clone(walk);
    walk_expression(cexpr, NULL, ( WK_EXPR_CLBACK) perf_expr_postwalk, (Generic) gwalk);
    gcost = sum_list(cdn, gwalk, ALL_LIST_ELEMENTS);
    Treewalk_delete(gwalk);
    
    /*
     * Apply symbolic analysis to see if the conditional expression is a
     * function of constants and/or unmodified formal parameters. If so,
     * then save it to see if we can evaluate it later after
     * interprocedural constant propagation takes place.
     */
    condval =(PerfEstExprHandle)
      conditional_expr_to_jumpfunction(cexpr, walk->jumptrans);
    
    /*
     * Sum up the costs of the statements which are conditionally
     * dependent on this guard/if.
     */
    true_branch_cost = sum_list(cdn, walk,
				ONLY_TRUE_CONTROLDEP_LIST_ELEMENTS);
    true_branch_cost = simplify_expr(walk, true_branch_cost);
    false_branch_cost = sum_list(cdn, walk,
				 ONLY_FALSE_CONTROLDEP_LIST_ELEMENTS);
    false_branch_cost = simplify_expr(walk, false_branch_cost);
    
    prune_list(cdn, walk, ALL_CONTROLDEP_LIST_ELEMENTS);
    total_cost = add_if(walk, true_branch_cost, false_branch_cost,
			condval, PERF_GUESS_BRANCH_PROB);
    add_expr_to_list(walk, total_cost, stmt, cdn);
    
    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);

  } else if (is_call(stmt)) {
    AST_INDEX invoc = gen_CALL_get_invocation(stmt);
    AST_INDEX invoc_name = gen_INVOCATION_get_name(invoc);

    /* Record the call
     */
    total_cost = add_call(walk, get_stmt_node_number(walk, invoc),
			  gen_get_text(invoc_name),
			  0, /* function invokcation, not proc call */
			  gen_INVOCATION_get_actual_arg_LIST(invoc));
    add_expr_to_list(walk, total_cost, stmt, cdn);

    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);

  } else if (is_program(stmt) || is_function(stmt) || is_subroutine(stmt)) {
    PerfEstExprHandle body_cost;

    /* Get all control-dependent elements
    */
    body_cost = sum_list(cdn, walk, ALL_CONTROLDEP_LIST_ELEMENTS);
    prune_list(cdn, walk, ALL_CONTROLDEP_LIST_ELEMENTS);
    body_cost = simplify_expr(walk, body_cost);
    walk->t->set_root(body_cost);
    total_cost = body_cost;

    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);

  } else if (is_continue(stmt) || is_return(stmt) || is_private(stmt) ||
	     is_read_short(stmt) || is_read_long(stmt) ||
	     is_write(stmt) || is_print(stmt) ||
	     is_close(stmt) || is_open(stmt) ||
	     is_stop(stmt) || is_stmt_function(stmt)) {

    /* Ignore these.
     */
    total_cost = walk->t->c_constant(0.0);
    add_expr_to_list(walk, total_cost, stmt, cdn);

    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);

  } else {

    if (pe_debug)
      fprintf(stderr,
	      "PE: internal error: %s(%d): unknown node (%d)\n",
	      __FILE__, __LINE__, stmt);

    /*
     * Even though we don't know what type of node this is,
     * we still have to push
     * it onto our list, in order for our algorithm to work.
     */
    total_cost = add_bottom(walk);
    add_expr_to_list(walk, total_cost, stmt, cdn);

    /* DEBUGGING */
    debug_add_est_to_list(walk, stmt, total_cost);
  }

#if 0
  /* DEBUGGING */
  if (pe_debug)
    fprintf(pe_debugfp, "cost for node %d: 0x%x\n", stmt, total_cost);
#endif

  return (WALK_CONTINUE);
}

/* Get the textual name of a procedure, function, or program.
*/

static char *get_proc_name(AST_INDEX curr)
{
  if (is_program(curr))
    return gen_get_text(gen_PROGRAM_get_name(curr));
  else if (is_function(curr))
    return gen_get_text(gen_FUNCTION_get_name(curr));
  else if (is_subroutine(curr))
    return gen_get_text(gen_SUBROUTINE_get_name(curr));
  else return "<unknown>";
}

/*
 * Count block data statements too, since the program compiler treats them
 * as functions in some respects.
 */

static int count_functions(AST_INDEX root_ast)
{
  AST_INDEX curr;
  int i = 0;

  curr = gen_GLOBAL_get_subprogram_scope_LIST(root_ast);
  for (curr = list_first(curr); curr != AST_NIL; curr = list_next(curr))
    if (is_program(curr) || is_function(curr) || is_subroutine(curr) ||
	is_block_data(curr))
      i++;
  return i;
}

/*
 * This routine runs the performance estimator on each function in the
 * module. This means building a control dependence graph for each
 * routine, walking it, and writing the results out to the database.
 */

void perf_local_phase_walkcd(Perf_data *pdata,
			     FortTree ft,
			     Context     context,
			     DG_Instance *dg,
			     SideInfo *infoPtr,
			     FILE *debug_fp,
			     Boolean *single,
			     char *trunc_modulename)
{
  AST_INDEX root_ast = ft_Root(ft);
  AST_INDEX curr;
  Treewalk *walkdata = Treewalk_new();
  ControlDep *cd;
  cdNode *cn;
  int i, prec;
  double ret_et;
  char *proc_name;
  jumptransinfo_callbacks callbacks;
  char buf[128];
  int func_count;
  
  /*
   * Record things like the FortTree for the module, get a pointer to
   * the symbol table, etc.
   */
  walkdata->pdata = pdata;
  walkdata->ft = ft;
  
  /* Set up for debugging printouts, if enabled.
   */
  if (debug_fp) {
    pe_debug = 2;
    pe_debugfp = debug_fp;
  }
  
  /*
   * Open the file in the database to which we're going to save the
   * symbolic expression.
   */ 

  FileContext fileContext;
  int code = fileContext.Open(context);
  if (code < 0) {
    fprintf(stderr, "Internal error opening context, file %s line %d\n",
	    __FILE__, __LINE__);
  }

  File *peAttrFile = fileContext.NewAttribute(PE_IPFILE_NAME);
  if (peAttrFile == 0) {
    fprintf(stderr, "Cannot write performance estimate to database\n");
  }

  FormattedFile port(peAttrFile);

  /*
   * Count the functions in this module and write out the count to the
   * initial information file.
   */
  func_count = count_functions(root_ast);
  port.Write(func_count);
  *single = true;
  if (func_count != 1)
    *single = false;
  
  /*
   * Initialize the datastructures, etc. for the "jump_trans_..."
   * routines. This requires that we call a corresponding 'free'
   * function later on.
   */
  callbacks.create_constant = callback_create_iconst;
  callbacks.create_bottom = callback_create_bottom;
  callbacks.create_var = callback_create_var;
  callbacks.create_op = (jumptrans_createop_func) callback_create_op;
  walkdata->jumptrans = jump_trans_init(walkdata->ft,    
                          &callbacks);
  
  curr = gen_GLOBAL_get_subprogram_scope_LIST(root_ast);
  for (curr = list_first(curr); curr != AST_NIL; curr = list_next(curr))
    
    /*
     * Special case block data statements
     */
    if (is_block_data(curr)) {
      PerfEstExpr e;
      e.set_root(e.c_constant(0.0));
      port.Write("DATA");
      e.write(port);
    } else 
      
      if (is_program(curr) || is_subroutine(curr) || is_function(curr)) {
	proc_name = get_proc_name(curr);
	
	if (strcmp(trunc_modulename, proc_name))
	  *single = false;
	
	/* Get the symbol table for the particular procedure
	 */
	walkdata->sd = ft_SymGetTable(walkdata->ft, proc_name);
	
	/*
	 * Build the graph for the procedure.
	 */
	cd = dg_build_cdg(dg, infoPtr, curr, 2.0);
	if (cd == NULL) {
	  fprintf(stderr,
		  "performance estimator: dg_build_cdg failed.\n");
	  return;
	}
	
	/*
	 * Print out some vital statistics.
	 */
	if (pe_debug)
	  fprintf(pe_debugfp,
		  "\n\nPE: subprog %s: cd root = %d, nodes = %d edges = %d\n",
		  proc_name, cd->top, cd->ntotal, cd->etotal);
	
	/*
	 * Find the CD node which corresponds to the root of
	 * the graph.
	 */
	for (cn = cd->nodes, i = 0; i < cd->ntotal; i++, cn++) {
	  if (cn->stmt == cd->top)
	    break;
	}
	if (cn->stmt != cd->top) {
	  fprintf(stderr, "performance estimator: rootless CDG (?)\n");
	  continue;
	}
	
	/* Set up the "jump_trans..." routines for this module.
	 */
	jump_trans_newfunction(walkdata->jumptrans, curr);

#if 0
	/* Dump out debugging information for the CFG/VAL stuff
	*/
	if (pe_debug) {
	  fprintf(pe_debugfp, "Dumping CFG/VAL info to stdout\n");
	  jump_trans_dump_debug_info(ft);
	}
#endif
	
	/*
	 * Make a prepass over the procedure to mark loops as
	 * parallelizable or unparallelizable.
	 */
#if 0
	perf_build_parloop_map(pedp, add_to_parloop_map, curr,
			       (void *) &(walkdata->parmap));
#endif
	
	/* Print some debugging information.
	 */
	if (pe_debug) {
	  sprintf(buf, "\n\n** CDG adjacency list for '%s'", proc_name);
	  pe_dump_cdg(cd, cn, walkdata->smap, buf);
	}
	
	/*
	 * Now do the real work. Make a new "PerfEstExpr" object, pass it to
	 * the CDG walk routine. Then extract the actual number from the
	 * object we get back.
	 */
	walkdata->t = new PerfEstExpr;
	cdg_walk_nodes(cd, cn,
		       ( cdg_action_callback) perf_cd_walk_pre_action,
		       ( cdg_action_callback) perf_cd_walk_post_action,
		       (Generic) walkdata);
	
	/*
	 * For debugging purposes, print out the control
	 * dependence graph, the estimate table, and the
	 * final estimate.
	 */
	if (pe_debug) {
	  sprintf(buf, "\n\n** Post-estimate CDG walk for '%s'", proc_name);
	  pe_dump_cdg(cd, cn, walkdata->smap, buf);
	  fprintf(pe_debugfp, "** Expression tree for '%s':\n",
		  proc_name);
	  walkdata->t->print(pe_debugfp);
	  fflush(pe_debugfp);
	  walkdata->t->get_estimate(walkdata->t->get_root(),
				    ret_et, prec);
	  (void) fprintf(pe_debugfp, "** Numerical estimate for '%s': ",
			 proc_name);
	  fprintf(pe_debugfp, "%lf (prec=%d)\n", ret_et, prec);
	  fflush(pe_debugfp);
	}
	
	/*
	 * Write out the name of the function to the database, followed by
	 * the symbolic expression for it.
	 */
	port.Write(proc_name);
	walkdata->t->write(port);
	
	/*
	 * Clean up. This means freeing the various lists that we allocated,
	 * as well as the perf est "expression".
	 */
	Treewalk_endfunction(walkdata);
	cdg_free(cd);
      }
  
  jump_trans_free(walkdata->jumptrans);
  delete walkdata->elist;
  
  /* Close the database port. 
   */
  port.Close();
  
  /*
   * Done.
   */
  if (pe_debug)
    fprintf(pe_debugfp, "Done walking CD graphs.\n");
}

// /* debugging */
// static void print_elist(Treewalk *walk)
// {
//   PerfWalkListIterator i(walk->elist, 0, ALL_LIST_ELEMENTS);
//   PerfWalkListEntry *cur;
//   char buf[256];
//   
//   for (; cur = i.current(); cur = i.next_entry()) {
//     fprintf(pe_debugfp, "=      : entry=0x%x ai=%07d e=0x%x %s\n",
// 	   cur, cur->ai, cur->expr, pr_cdNode(cur->cdn, buf));
//   }
// }
