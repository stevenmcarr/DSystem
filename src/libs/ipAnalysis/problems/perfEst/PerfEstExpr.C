/* $Id: PerfEstExpr.C,v 1.8 1997/03/11 14:35:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * "Symbolic Expression" abstraction for use with performance estimation.
 * This class inherits from SymExprNode, a simple 'symbolic expression'
 * tree class. It adds additional node types to the tree which are
 * specific to performance estimation.
 */

/******************************************************************
 * Author: N. McIntosh                                            *
 *                                                                *
 * Copyright 1991, Rice University, as part of the ParaScope      *
 * Programming Environment Project                                *
 *                                                                *
 ******************************************************************/

#include <libs/support/misc/general.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/ipAnalysis/problems/perfEst/PerfEstExpr.h>

/* String to pass to get mem
*/
#define GETMEMSTRING "PerfEstExpr.C"

/* Write more verbose form of PerfEstExprNodes to database (debugging)
*/
#define PORT_IO_DEBUG 1

/*-------------------- Methods for class PerfEstExprNode --------------------*/

/*
 * The following set of "New<nodetype>" methods are used to override a set
 * of virtual functions of the same name in class SymExprNode. See the
 * comments for the same routines in that class. For these routines, the
 * basic pattern is to construct a SymExprNode of the correct type, then
 * copy it to a newly createf PerfEstExpr node.
 */

/*
 * Given a SymExprNode 'x', create a new PerfEstExpr node 'n'and copy the
 * contents of 'x' to 'n'. Then return n.
 */

SymExprNode *
PerfEstExprNode::copy_symexprnode_to_perfestexprnode(SymExprNode &x)
{
  PerfEstExprNode *n = new PerfEstExprNode;

  x.copynode((SymExprNode *) n);
  n->typ = PNT_BASE;
  return (SymExprNode *) n;
}

SymExprNode *
PerfEstExprNode::NewIconst(int i_const)
{
  SymExprNode x(i_const);
  return copy_symexprnode_to_perfestexprnode(x);
}

SymExprNode *
PerfEstExprNode::NewRconst(double r_const)
{
  SymExprNode x(r_const);
  return copy_symexprnode_to_perfestexprnode(x);
}

SymExprNode *
PerfEstExprNode::NewArithOp(SymExprNodeType a_type,
			    SymExprNode *left_son,
			    SymExprNode *right_son)
{
  SymExprNode ls(0), rs(0);
  SymExprNode x(a_type, &ls, &rs);
  SymExprNode *r;

  /* the following copies just the contents, not the children */
  r = copy_symexprnode_to_perfestexprnode(x);
  ls.unlink();
  rs.unlink();

  /* now link in the children */
  left_son->link(r);
  right_son->link(r);
  return r;
}

SymExprNode *
PerfEstExprNode::NewVar(char *vname)
{
  SymExprNode x(vname);
  return copy_symexprnode_to_perfestexprnode(x);
}

SymExprNode *
PerfEstExprNode::NewNode(SymExprNodeType a_type)
{
  SymExprNode x(a_type);
  return copy_symexprnode_to_perfestexprnode(x);
}

char *
PerfEstExprNode::GetCallFname()
{
  assert(typ == PNT_CALL);
  return val.call.fname;
}

double 
PerfEstExprNode::get_ifstmt_branchprob()
{
  assert(typ == PNT_IF);
  return val.ifstmt.branch_prob;
}

PerfEstExprNode::PerfEstExprNode(int node_number,
				 PerfEstExprHandle loop_body,
				 PerfEstExprHandle lower_bound,
				 PerfEstExprHandle upper_bound,
				 PerfEstExprHandle loop_step,
				 int iterations_guess,
				 int is_parallelizable,
				 int is_marked_parallel,
				 double parallel_time,
				 int best_num_procs,
				 int astindex,
				 char *labelstring) :
				 SymExprNode(SYMEXPR_ARITH_DERIVED)
{
  typ = PNT_LOOP;
  lower_bound->link(this);
  upper_bound->link(this);
  loop_step->link(this);
  loop_body->link(this);
  val.loop.node_number = node_number;
  val.loop.iterations_guess = iterations_guess;
  val.loop.parallelizable = is_parallelizable;
  val.loop.marked_parallel = is_marked_parallel;
  val.loop.par_time = parallel_time;
  val.loop.best_num_procs = best_num_procs;
  val.loop.debug_astindex = astindex;
  val.loop.debug_lbl = ssave(labelstring);
}

PerfEstExprNode::PerfEstExprNode(int node_number,
				 char *f_name,
				 int num_actuals,
				 PerfEstExprNode **actuals_arr) :
				 SymExprNode(SYMEXPR_ARITH_DERIVED)
{
  int i;

  typ = PNT_CALL;
  if (actuals_arr) {
    for (i = 0; i < num_actuals; ++i)
      (actuals_arr[i])->link(this);
    delete actuals_arr; /* just the array of pointers, not the objects 
			   pointed to */
  }
  val.call.node_number = node_number;
  val.call.fname = ssave(f_name);
}

PerfEstExprNode::PerfEstExprNode(double branch_prob,
				 PerfEstExprNode *cond_expr,
				 PerfEstExprNode *true_branch,
				 PerfEstExprNode *false_branch) :
				 SymExprNode(SYMEXPR_ARITH_DERIVED)
{
  typ = PNT_IF;
  val.ifstmt.branch_prob = branch_prob;
  true_branch->link(this);
  false_branch->link(this);
  cond_expr->link(this);
}

PerfEstExprNode::~PerfEstExprNode()
{
  switch(typ) {
    case PNT_LOOP: sfree(val.loop.debug_lbl);
		   break;
    case PNT_CALL: sfree(val.call.fname);
		   break;
    default:       break;
  }
}

PerfEstExprNode *
PerfEstExprNode::get_nth_child(unsigned whichchild)
{
  PerfEstExprNode *n;

  if (whichchild > ChildCount()) {
    fprintf(stderr, "%s(%d): internal error: incorrect # of children\n",
	    __FILE__, __LINE__);
    return PERFEST_EXPR_HANDLE_NULL;
  }

  whichchild--;
  n = (PerfEstExprNode *) FirstChild();
  for (; whichchild; --whichchild)
    n = (PerfEstExprNode *) n->NextSibling();
  return n;
}

/*
 * Copy the contents of one node to another, without affecting the node's
 * children.
 */

void PerfEstExprNode::copynode(SymExprNode *starget)
{
  PerfEstExprNode *target = (PerfEstExprNode *) starget;

  /* Copy data in the base class */
  SymExprNode::copynode(starget);

  /* Now copy data in the derived class */
  target->typ = typ;
  target->val = val;
  switch(typ) {
    case PNT_LOOP:
    {
      if (val.loop.debug_lbl)
	target->val.loop.debug_lbl = ssave(val.loop.debug_lbl);
      break;
    }
    case PNT_CALL:
    {
      if (val.call.fname)
	target->val.call.fname = ssave(val.call.fname);
      break;
    }
    default: break;
  }
}

/*
 * Clone a node without cloning any of its children. Required by the
 * 'simplify()' method in the SymExprNode class.
 */

SymExprNode *
PerfEstExprNode::nodeclone()
{
  PerfEstExprNode *n = new PerfEstExprNode;

  copynode(n);
  return (SymExprNode *) n;
}

PerfEstExprNode *
PerfEstExprNode::clone()
{
  return (PerfEstExprNode *) SymExprNode::clone();
}

double PerfEstExprNode::interpret_bottom()
{
  PerfEstExprNode *par = Parent();

  if (!par)
    return 0.0;

  switch(par->GetNodeType()) {
    case SYMEXPR_ARITH_ADD:
    case SYMEXPR_ARITH_SUB:
    {
      return 0.0;
    }
    case SYMEXPR_ARITH_MULT:
    case SYMEXPR_ARITH_DIV:
    case SYMEXPR_ARITH_DERIVED: 
    case SYMEXPR_LOG_NOT:
    case SYMEXPR_LOG_AND:
    case SYMEXPR_LOG_OR:
    case SYMEXPR_COND_EQ:
    case SYMEXPR_COND_NE:
    case SYMEXPR_COND_LT:
    case SYMEXPR_COND_GT:
    case SYMEXPR_COND_LE:
    case SYMEXPR_COND_GE:
    {
      return 1.0;
    }
    default:
    {
      assert(1 == 0);
    }
  }
  return 0.0; /* not reached */
}      

/*
 * Return an execution time estimate for a particular expression or
 * subexpression (presumably corresponding to a procedure body or loop).
 * The estimate will be returned in the variable "re". The variable "isp"
 * will be set to TRUE (1) if no guessing or was required to come up with
 * the number, or FALSE (0) if the routine had to guess at some point.
 * 
 * Notes:
 * 
 * 1) this routine assumes that recursive calls to get_estimate() don't
 * make ANY changes to the data structures being examined.
 * 
 * 2) an implementation detail: the estimate generated assumes that all loops
 * are executed sequentially.  Even if a loop is marked as being
 * explicitly parallel, we'll still return an estimate of the serial
 * execution time.
 */

void PerfEstExprNode::get_estimate(double& re, int& isp)
{
  double s1, s2, s3;
  int ip1, ip2, ip3;
  
  /* Deal with the base node types.
  */
  switch (GetNodeType()) {

    case SYMEXPR_ARITH_CONST:
    {
      if (GetConstDataType() == TYPE_INTEGER)
	re = GetIntegerConstVal();
      else re = GetRealConstVal();
      isp = 1;
      return;
    }

    case SYMEXPR_ARITH_BOTTOM:
    case SYMEXPR_ARITH_VAR:
    {
      re = interpret_bottom();
      isp = 0;
      return;
    }

    case SYMEXPR_ARITH_ADD:
    case SYMEXPR_ARITH_SUB:
    case SYMEXPR_ARITH_MULT:
    case SYMEXPR_ARITH_DIV:
    {
      PerfEstExprNode *ls = leftson(), *rs = rightson();
      SymExprNode *n, *n2;
      Boolean deleteit;

      /*
       * The following is something of a hack, but it avoids some
       * duplicated code. Rather than explicitly evaluating the node,
       * create a new arithmetic op node which has children that are
       * constants, then call simplify_node() to get the result. The
       * result is then thrown away.
       */
      ls->get_estimate(s1, ip1);
      rs->get_estimate(s2, ip2);
      n = SymExprNode::NewArithOp(GetNodeType(),
				  this->NewRconst(s1),
				  this->NewRconst(s2));
      n2 = n->simplify_node(deleteit);
      if (deleteit) delete n;
      assert(n2->GetNodeType() == SYMEXPR_ARITH_CONST &&
	     n2->GetConstDataType() == TYPE_REAL);
      re = n2->GetRealConstVal();
      isp = ip1 && ip2;
      delete n2;
      return;
    }

    case SYMEXPR_ARITH_DERIVED:
    {
      break;
    }

    default:
    {
      assert(1 == 0);
      return;
    }
  }

  /*
   * Derived class node types...
   */
  switch (typ) {

    /*
     * At this point we can't determine whether one branch of the
     * IF is always taken, so we have to guess.
     */
    case PNT_IF:
    {
      PerfEstExprNode *tchild, *fchild, *condexpr;
      PerfEstExprNode *cresult;
      
      tchild = get_ifstmt_true_branch();
      fchild = get_ifstmt_false_branch();
      tchild->get_estimate(s1, ip1);
      fchild->get_estimate(s2, ip2);

      condexpr = get_ifstmt_cond_expr();
      cresult = condexpr->simplify();
      if (cresult->GetNodeType() == SYMEXPR_ARITH_CONST &&
	  cresult->GetConstDataType() == TYPE_INTEGER)
	val.ifstmt.branch_prob = (double) cresult->GetIntegerConstVal();
      delete cresult;

      re = (val.ifstmt.branch_prob * s1) +
	((1 - val.ifstmt.branch_prob) * s2);
      isp = 0;
      break;
    }
    
    /*
     * For loops, if the bounds and step are constant, then we
     * get a precise estimate, otherwise we need to guess.
     */
    case PNT_LOOP:
    {
      PerfEstExprNode *lb, *ub, *st, *bod;
      int iters = -1, p;
      
      /*
       * Examine the loop bounds and step to get # of
       * iterations if possible
       */
      bod = get_loop_body();
      lb = get_loop_lb();
      ub = get_loop_ub();
      st = get_loop_step();
      lb->get_estimate(s1, ip1);
      ub->get_estimate(s2, ip2);
      st->get_estimate(s3, ip3);
      if (p = (ip1 && ip2 && ip3))
	iters = (int) ((s2 - s1 + 1) / s3);
      
      /*
       * Get the cost of executing the body
       */
      bod->get_estimate(s1, ip1);
      p = p && ip1;
      
      /*
       * Figure out the time
       */
      isp = p;
      if (iters != -1) {	/* # of iterations known */
	re = s1 * iters;
      } else {			/* # of iterations not known */
	re = s1 * val.loop.iterations_guess;
      }
      break;
    }

    /* Can't do much about calls at the moment; just
     * ignore it, but set the "precise" flag to 0.
     */
    case PNT_CALL:
    {
      re = 0.0;
      isp = 0;
      return;
    }
    
    case PNT_NONE:	/* we should never get here. */
    default:
    {
      assert(1 != 0);
      break;
    }
  }
  
  /*
   * Done.
   */
  return;
}

void PerfEstExprNode::get_loop_info_estimates(int &niterations,
					      double& bodycost,
					      int& is_precise)
{
  PerfEstExprHandle bod_h, lb_h, ub_h, st_h;
  double bodyval, lb, ub, st;
  int isp, ispsum = 1, bodyprec;

  if (typ != PNT_LOOP) {
    fprintf(stderr,
	    "%s(%d): internal error: loop info requested for non-loop.\n",
	    __FILE__, __LINE__);
    is_precise = 0;
    niterations = 1;
    bodycost = 5.0;
    return;
  }

  /* Estimate the body cost. If 0, make it a small value.
  */
  bod_h = get_loop_body();
  bod_h->get_estimate(bodyval, bodyprec);
  if (bodyval <= 5.0)
    bodyval = 5.0;
  bodycost = bodyval;

  /* Get the number of iterations.
  */
  lb_h = get_loop_lb();
  ub_h = get_loop_ub();
  st_h = get_loop_step();
  lb_h->get_estimate(lb, isp);
  ispsum = ispsum && isp;
  ub_h->get_estimate(ub, isp);
  ispsum = ispsum && isp;
  st_h->get_estimate(st, isp);
  ispsum = ispsum && isp;
  if (!ispsum)
    niterations = val.loop.iterations_guess;
  else
    niterations = (int) ((ub - lb + 1) / st);

  /* Done.
  */
  is_precise = ispsum;
}

void PerfEstExprNode::set_parallel_info(double parallel_etime,
					int best_nprocs)
{
  if (typ != PNT_LOOP) {
    fprintf(stderr, "%s(%d): attempt to set loop info for non-loop.\n",
	    __FILE__, __LINE__);
    return;
  }

  val.loop.best_num_procs = best_nprocs;
  val.loop.par_time = parallel_etime;
  return;
}

void PerfEstExprNode::estimate_best_num_procs(int& returned_estimate,
					      int& is_precise)
{
  double est_val;
  int isp;

  returned_estimate = 1;
  is_precise = 0;

  if (typ != PNT_LOOP) {
    fprintf(stderr, "%s(%d): best_num_procs requested for non-loop.\n",
	    __FILE__, __LINE__);
    return;
  }
  
  /*
   * If the estimate for the loop is precise, then the number of processors
   * choice should be precise as well.
   */
  get_estimate(est_val, isp);
  is_precise = isp;
  returned_estimate = val.loop.best_num_procs;
  return;
}

/*
 * Nodes such as LOOP, IF, and CALL nodes don't simplify -- their children
 * may, if they are arithmetic expressions, but there's not much you can
 * do with loops, etc.
 */

SymExprNode *
PerfEstExprNode::simplify_node(Boolean &deletethis, 
			       SymExprNode_mapnamefunc mapfunc,
			       void *user)
{
  if (typ == PNT_BASE)
    return SymExprNode::simplify_node(deletethis, mapfunc, user);
  else {
    deletethis = false;
    return (SymExprNode *) this;
  }
}

int PerfEstExprNode::ReadUpCall(FormattedFile& port)
{
  char buf[1024];
  int t;
  
#ifdef PORT_IO_DEBUG
  // if ((t = port.p_getc()) == EOF) return EOF;
  // if (t != IO_CHAR_START_PERFESTEXPRNODE)
    // fprintf(stderr, "PerfEstExprNode::read -- no magic char\n");
#endif PORT_IO_DEBUG

  if (port.Read(t) == EOF) return EOF;
  typ = (PerfEstExprNodeType) t;
  if (typ == PNT_BASE) {
    if (SymExprNode::ReadUpCall(port) == EOF) return EOF;
  } else {

    switch(typ) {
      
      case PNT_IF:
      {
	port.Read(val.ifstmt.branch_prob);
	break;
      }
      
      case PNT_LOOP:
      {
	if (port.Read(val.loop.node_number) == EOF ||
	    port.Read(val.loop.iterations_guess) == EOF ||
	    port.Read(val.loop.parallelizable) == EOF ||
	    port.Read(val.loop.marked_parallel) == EOF ||
	    port.Read(val.loop.par_time) == EOF ||
	    port.Read(val.loop.best_num_procs) == EOF ||
	    port.Read(buf, 1023) == EOF)
	  return EOF;
	val.loop.debug_lbl = ssave(buf);
	break;
      }
      
      case PNT_CALL:
      {
	if (port.Read(buf, 1023) == EOF) return EOF;
	val.call.fname = ssave(buf);
	if (port.Read(val.call.node_number) == EOF) return EOF;
	break;
      }
      
      default: break;
    }
    
  }

#ifdef PORT_IO_DEBUG
  // if (port.p_getc() == EOF) return EOF;
#endif PORT_IO_DEBUG

  return 0;
}

int PerfEstExprNode::WriteUpCall(FormattedFile& port)
{
#ifdef PORT_IO_DEBUG
  // if (port.p_putc(IO_CHAR_START_PERFESTEXPRNODE) == EOF) return EOF;
#endif PORT_IO_DEBUG

  if (port.Write((int) typ) == EOF) return EOF;

  if (typ == PNT_BASE) {
    if (SymExprNode::WriteUpCall(port) == EOF) return EOF;
  } else {

    switch(typ) {
      
      case PNT_IF:
      {
	if (port.Write(val.ifstmt.branch_prob) == EOF) return EOF;
	break;
      }
      
      case PNT_LOOP:
      {
	if (port.Write(val.loop.node_number) == EOF ||
	    port.Write(val.loop.iterations_guess) == EOF ||
	    port.Write(val.loop.parallelizable) == EOF ||
	    port.Write(val.loop.marked_parallel) == EOF ||
	    port.Write(val.loop.par_time) == EOF ||
	    port.Write(val.loop.best_num_procs) == EOF ||
	    port.Write(val.loop.debug_lbl) == EOF)
	  return EOF;
	break;
      }
      
      case PNT_CALL:
      {
	if (port.Write(val.call.fname) == EOF) return EOF;
	if (port.Write(val.call.node_number) == EOF) return EOF;
	break;
      }
      
      default: break;
    }
  }

#ifdef PORT_IO_DEBUG
  // if (port.p_putc(IO_CHAR_END_PERFESTEXPRNODE) == EOF) return EOF;
#endif PORT_IO_DEBUG

  return 0;
}

void print_PerfEstExprNode(PerfEstExprHandle h)
{
  h->print(stdout);
}

static char **stringlist_copy(char **from)
{
  int i, n, sl;
  char **r;
  
  if (!from)
    return 0;
  for (n = 0; from[n]; n++);
  r = (char **) get_mem(sizeof(char **) * (n + 1), GETMEMSTRING);
  for (i = 0; from[i]; i++) {
    sl = strlen(from[i]) + 1;
    r[i] = (char *) get_mem(sl, GETMEMSTRING);
    strcpy(r[i], from[i]);
  }
  r[n] = 0;
  return r;
}

char **PerfEstExprNode::print_node(FILE *fp)
{
  char **r;
  static char *loop_child_strings[] = {
    "LB: ", "UB: ", "STEP: ", "BODY: ", 0 };
  static char *if_child_strings[] = {
    "TRUE: ", "FALSE: ", "CONDEXPR: ", 0, };

  if (typ == PNT_BASE)
    return SymExprNode::print_node(fp);

  switch(typ) {

    case PNT_NONE:
    {
      fprintf(fp, "NONE\n");
      break;
    }

    case PNT_LOOP:
    {
      int nchildren = ChildCount();
      SymExprNode *n;

      fprintf(fp,
	      "LOOP (nn=%d ai=%d lbl=%s mp=%d ip=%d bnp=%d pt=%lf ig=%d)\n",
	      val.loop.node_number, val.loop.debug_astindex,
	      val.loop.debug_lbl,
	      val.loop.marked_parallel, val.loop.parallelizable,
	      val.loop.best_num_procs, val.loop.par_time,
	      val.loop.iterations_guess);

      return stringlist_copy(loop_child_strings);
    }

    case PNT_IF:
    {
      fprintf(fp, "IF (bp=%lf)\n", val.ifstmt.branch_prob);
      return stringlist_copy(if_child_strings);
    }

    case PNT_CALL:
    {
      fprintf(fp, "CALL (nn=%d, name=%s)\n",
	      val.call.node_number, val.call.fname);
      break;
    }
    default: break;
  }
  return 0;
}

/*
 * The following two substitution functions we can get 'for free' by using
 * the same methods in the parent class. These functions are defined just
 * so that the we don't have to add a lot of typecasts when using them.
 */

PerfEstExprNode *
PerfEstExprNode::substitute(SymExprNode_substfunc substfunc, void *user)
{
  return (PerfEstExprNode *)
    SymExprNode::substitute(substfunc, user);
}

PerfEstExprNode *
PerfEstExprNode::substitute_var(char *var, PerfEstExprNode *subst_expr)
{
  return (PerfEstExprNode *)
    SymExprNode::substitute_var(var, (SymExprNode *) subst_expr);
}

/*
 * Local structure used only by PerfEstExprNode::substitute_call, it's
 * callback function, and another callback function for substituting
 * actuals for formals in the called expression. This provides a single
 * place to dump info that the callback functions need.
 */

typedef struct substitute_call_info__ {
  char *called_fname;
  char **formals_list;
  int nformals;
  PerfEstExprNode *callee_expr;
  PerfEstExprNode *callsite;
} substitute_call_info;

static SymExprNode *local_actual_formal_subst(SymExprNode *node,
					      void *user)
{
  PerfEstExprNode *n = (PerfEstExprNode *) node;
  substitute_call_info *info = (substitute_call_info*)user;
  char *varname;

  /*
   * Is this a variable? If so, is it in our list of formals? If so, then
   * return a clone of the appropriate actual.
   */
  if (n->GetType() == PNT_BASE && 
      n->GetNodeType() == SYMEXPR_ARITH_VAR) {
    char *varname = n->GetVarName();
    char **ff = info->formals_list;
    int nformals = info->nformals;
    int i, found = 0;
    for (i = 0; !found && i < nformals; i++)
      found = !(strcmp(varname, ff[i]));
    if (found) {
      PerfEstExprNode *actual = info->callsite->get_call_actual(i);
      return actual->clone();
    } else {
      /*
       * Currently, if a variable is not a formal, then it must be a
       * common block variable. If it's not constant at this point, then
       * we need to turn it into "bottom", since its value may be
       * different in the context of the callee.
       */
      return n->NewNode(SYMEXPR_ARITH_BOTTOM);
    }
  } else return 0;
}

/*
 * This function is called by the "substitute" function during
 * substitution of an expression for a call.
 * 
 * Parameters (passed via 'info') are as follows:
 * 
 * called_fname -- the name of the called function. The goal is to
 *                 find all CALL nodes which are calls to this function
 *                 and replace them.
 * 
 * formal_list  -- array of strings corresponding to the names of the
 *                 formals of the function "called_fname"
 * 
 * nformals     -- number of formals for function "called_fname"
 * 
 * callee_expr  -- PerfEstExprNode tree corresponding to the cost of
 *                 executing the function "called_fname". This expression
 *                 may contain variable nodes which correspond to the
 *                 formals of this routine. When the substitution is
 *                 performed, we want to translate the formals in
 *                 "called_expr" with the actuals given in the callsite "n".
 *
 * callsite	-- Call note in caller's expression (i.e. this is 
 *		   where we get our actual expressions for substitution).
 */

static SymExprNode *local_substitute_call(SymExprNode *node, void *user)
{
  PerfEstExprNode *n = (PerfEstExprNode *) node;
  substitute_call_info *info = (substitute_call_info*)user;
  char *called_fname = info->called_fname;

  /*
   * If this node corresponds to a call to the specified function, then
   * perform the substitution and return the resulting expression.
   * Otherwise, return 0 (see SymExprNode::substitute). 
   */
  if (n->GetType() == PNT_CALL && !(strcmp(n->GetCallFname(), called_fname))) {
    char **formals_list = info->formals_list;
    int nformals = info->nformals;
    PerfEstExprNode *callee_expr = info->callee_expr;
    return (SymExprNode *)
      callee_expr->substitute_actuals_for_formals(formals_list,
						  nformals,
						  n);
  } else
    return 0;
}

/*
 * The assumption is that 'this' is an expression E for a called routine
 * X, and that 'callnode' is a node corresponding to a call to X. This
 * routine creates a new version of E in which the formals of X have been
 * replaced by the actuals at the call site corresponding to 'callnode'.
 */

PerfEstExprNode *
PerfEstExprNode::substitute_actuals_for_formals(char **formals_list,
						int nformals,
						PerfEstExprNode *callnode)
{
  int num_actuals = callnode->ChildCount();
  substitute_call_info info;

  /* Check that formal and actual counts match up
  */
  if (num_actuals != nformals) {
    fprintf(stderr, "PerfEstExpr::substitute_actuals_for_formals -- ");
    fprintf(stderr, "formals count != actuals count\n");
  }

  /*
   * Do simultaneous substitution of formals to actuals. We reuse the info
   * structure, ignoring some of the fields.
   */
  info.called_fname = 0;
  info.formals_list = formals_list;
  info.nformals = nformals;
  info.callee_expr = 0;
  info.callsite = callnode;
  return substitute(local_actual_formal_subst, (void *) &info);
}

/*
 * The following subsitution function is a little complicated. Given an
 * expression containing function calls, we want to substitute an
 * expression E for a particular call to X, while substituting the actuals
 * at the call site for the formals in E. The actual work is done
 * in the callback function 'local_substitute_call'.
 */

PerfEstExprNode *
PerfEstExprNode::substitute_call(char *called_fname,
				 char **formals_list,
				 int nformals,
				 PerfEstExprNode *callee_expr)
{
  substitute_call_info info;
  
  info.called_fname = called_fname;
  info.formals_list = formals_list;
  info.nformals = nformals;
  info.callee_expr = callee_expr;
  return substitute(local_substitute_call, (void *) &info);
}

			      


/*---------------------- Methods for class PerfEstExpr ----------------------*/

PerfEstExpr::PerfEstExpr()
{
  roothandle = PERFEST_EXPR_HANDLE_NULL;
}

PerfEstExpr::~PerfEstExpr()
{
  delete roothandle;
}

PerfEstExprHandle
PerfEstExpr::handle_parent(PerfEstExprHandle h)
{
  return h->Parent();
}

PerfEstExpr *PerfEstExpr::clone()
{
  PerfEstExpr *n = new PerfEstExpr;

  n->roothandle = roothandle->clone();
  return n;
}

PerfEstExprHandle PerfEstExpr::clone_handle(PerfEstExprHandle h)
{
  return h->clone();
}


int PerfEstExpr::write(FormattedFile& port)
{
  return roothandle->Write(port);
}

int PerfEstExpr::read(FormattedFile& port)
{
  if (roothandle)
    delete roothandle;
  roothandle = new PerfEstExprNode;
  return roothandle->Read(port);
}

void PerfEstExpr::print(FILE *fp)
{
  roothandle->print(fp);
}

void PerfEstExpr::set_root(PerfEstExprHandle rhandle)
{
  roothandle = rhandle;
}

PerfEstExprHandle PerfEstExpr::get_root()
{
  return roothandle;
}

PerfEstExprHandle PerfEstExpr::c_constant(double const_value)
{
  PerfEstExprNode n;

  return (PerfEstExprHandle) n.NewRconst((double) const_value);
}

PerfEstExprHandle PerfEstExpr::c_bottom()
{
  PerfEstExprNode n;

  return (PerfEstExprHandle) n.NewNode(SYMEXPR_ARITH_BOTTOM);
}

PerfEstExprHandle PerfEstExpr::c_variable(char *variable_name)
{
  PerfEstExprNode n;
  
  return (PerfEstExprHandle) n.NewVar(variable_name);
}

PerfEstExprHandle PerfEstExpr::c_sum(PerfEstExprHandle s1,
				     PerfEstExprHandle s2)
{
  PerfEstExprNode n;
 
  return (PerfEstExprHandle) n.NewArithOp(SYMEXPR_ARITH_ADD, s1, s2);
}

PerfEstExprHandle PerfEstExpr::c_loop(int node_number,
			 PerfEstExprHandle loop_body,
			 PerfEstExprHandle lower_bound,
			 PerfEstExprHandle upper_bound,
			 PerfEstExprHandle loop_step,
			 int iterations_guess,
			 int is_parallelizable, int is_marked_parallel,
			 double parallel_time, int best_num_procs,
			 int astindex, char *labelstring)
{
  return new PerfEstExprNode(node_number, loop_body, lower_bound,
			     upper_bound, loop_step, iterations_guess,
			     is_parallelizable, is_marked_parallel,
			     parallel_time, best_num_procs,
			     astindex, labelstring);
}

PerfEstExprHandle PerfEstExpr::c_call(int node_number,
				      char *f_name,
				      int num_actuals,
				      PerfEstExprNode **actuals_arr)
{
  return new PerfEstExprNode(node_number, f_name, num_actuals, actuals_arr);
}

PerfEstExprHandle PerfEstExpr::c_ifstmt(double branch_prob,
					PerfEstExprHandle cond_expr,
					PerfEstExprHandle true_branch,
					PerfEstExprHandle false_branch)
{
  return new PerfEstExprNode(branch_prob, cond_expr,
			     true_branch, false_branch);
}

void PerfEstExpr::get_estimate(PerfEstExprHandle handle,
			       double& re, int& isp)
{
  handle->get_estimate(re, isp);
  return;
}

void PerfEstExpr::get_loop_info_estimates(PerfEstExprHandle h,
					  int &niterations,
					  double& bodycost,
					  int& is_precise)
{
  h->get_loop_info_estimates(niterations, bodycost, is_precise);
}
  
void PerfEstExpr::set_parallel_info(PerfEstExprHandle handle, 
				    double parallel_etime,
				    int best_nprocs)
{
  handle->set_parallel_info(parallel_etime, best_nprocs);
}

void PerfEstExpr::estimate_best_num_procs(PerfEstExprHandle handle,
					  int& returned_estimate,
					  int& is_precise)
{
  if (handle == PERFEST_EXPR_HANDLE_NULL) {
    fprintf(stderr, "%s(%d): bad handle passed to `estimate_num_procs'\n",
	    __FILE__, __LINE__);
    return;
  }

  handle->estimate_best_num_procs(returned_estimate, is_precise);
}

PerfEstExprHandle PerfEstExpr::simplify_expr(PerfEstExprHandle handle,
				      SymExprNode_mapnamefunc mapfunc,
				      void *user)
{
  return handle->simplify(mapfunc, user);
}

void PerfEstExpr::simplify_root(SymExprNode_mapnamefunc mapfunc,
			       void *user)
{
  PerfEstExprHandle r = roothandle->simplify(mapfunc, user);
  delete roothandle;
  roothandle = r;
}

void PerfEstExpr::substitute_call(char *called_fname,
				  char **formals_list,
				  int nformals,
				  PerfEstExpr *callee_expr)
{
  PerfEstExprNode *new_subs = 
    roothandle->substitute_call(called_fname, formals_list,
				nformals, callee_expr->roothandle);
  delete roothandle;
  roothandle = new_subs;
}

void 
PerfEstExpr::substitute_actuals_for_formals(char **formals_list,
					    int nformals,
					    PerfEstExprNode *actualnode)
{
  PerfEstExprNode *new_subs = 
    roothandle->substitute_actuals_for_formals(formals_list,
					       nformals,
					       actualnode);

  delete roothandle;
  roothandle = new_subs;
}

