/* $Id: SymExpr.C,v 1.11 1997/03/11 14:35:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * Methods for class SymExpr. See "symexpr.h" for more information.
 *
 * Author: N. McIntosh
 *
 * Copyright 1992, Rice University, as part of the ParaScope
 * Programming Environment Project
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <libs/support/misc/general.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/file/FormattedFile.h>

#include <libs/ipAnalysis/problems/perfEst/SymExpr.h>

#define JUNKVAL 0xfeedface
#define FJUNKVAL 987654321.0

/*
 * If the following macro is defined, we'll a slightly more human-readable
 * intermediate file when saving expressions to the database.
 */
#define VERBOSE_IO_FORMAT 1

/*----------------------------------------------------------------------*/

static Boolean is_cond_op(SymExprNodeType atype)
{
  if (atype == SYMEXPR_COND_EQ || atype == SYMEXPR_COND_GE ||
      atype == SYMEXPR_COND_NE || atype == SYMEXPR_COND_LT ||
      atype == SYMEXPR_COND_GT || atype == SYMEXPR_COND_LE)
    return true;
  else
    return false;
}

static Boolean is_arith_op(SymExprNodeType atype)
{
  if (atype == SYMEXPR_ARITH_ADD || atype == SYMEXPR_ARITH_SUB || 
      atype == SYMEXPR_ARITH_MULT || atype == SYMEXPR_ARITH_DIV)
    return true;
  else return false;
}

static Boolean is_log_op(SymExprNodeType atype)
{
  if (atype == SYMEXPR_LOG_NOT || atype == SYMEXPR_LOG_AND ||
      atype == SYMEXPR_LOG_OR)
    return true;
  else return false;
}

/*
 * This routine is primarily to avoid duplication of code. It is called by
 * the various types of constructors. It also makes a convenient place to
 * set a breakpoint in the debugger if you want to look at all of the
 * objects constructed in this class.
 */

void SymExprNode::Construct(SymExprNodeType a_type,
			 int d_type,
			 int i_const,
			 double r_const,
			 char *vname,
			 SymExprNode *left_son,
			 SymExprNode *right_son,
			 SymExprNode *new_parent)
{
  atype = a_type;
  dtype = TYPE_UNKNOWN;
  switch(a_type) {
    case SYMEXPR_ARITH_CONST:
    {
      dtype = d_type;
      if (dtype == TYPE_INTEGER)
	arith.iconst = i_const;
      else if (dtype == TYPE_REAL)
	arith.rconst = r_const;
      else assert(1 == 0);
      break;
    }
    case SYMEXPR_ARITH_VAR:
    {
      arith.name = ssave(vname);
      break;
    }
    case SYMEXPR_ARITH_ADD:
    case SYMEXPR_ARITH_SUB:
    case SYMEXPR_ARITH_MULT:
    case SYMEXPR_ARITH_DIV:
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
      left_son->link(this);
      right_son->link(this);
      break;
    }
    case SYMEXPR_ARITH_DERIVED:
    case SYMEXPR_ARITH_BOTTOM:
    {
      break;
    }
    default:
    {
      assert(1 == 0);
    }
  }
  if (new_parent)
    link(new_parent);
  shelper = 0;
}

/*
 * Vanilla constructor -- should only be used during tree cloning and
 * other operations where nodes are copied or read in. In other words, a
 * node constructed by this routine should then be initialized in some
 * other way.
 */

SymExprNode::SymExprNode(SymExprNode *new_parent) 
   : NonUniformDegreeTreeNodeWithDBIO (new_parent)
{
  Construct(SYMEXPR_ARITH_BOTTOM);
  SetNodeType(SYMEXPR_ARITH_NONE);
}

/*
 * Constructors for making nodes of various types.
 */

SymExprNode::SymExprNode(SymExprNodeType a_type, SymExprNode *new_parent)
   : NonUniformDegreeTreeNodeWithDBIO (new_parent)
{
  assert(a_type == SYMEXPR_ARITH_BOTTOM ||
	 a_type == SYMEXPR_ARITH_DERIVED);
  Construct(a_type);
}

SymExprNode::SymExprNode(int i_const, SymExprNode *new_parent) 
   : NonUniformDegreeTreeNodeWithDBIO (new_parent)
{
  Construct(SYMEXPR_ARITH_CONST, TYPE_INTEGER, i_const);
}

SymExprNode::SymExprNode(double r_const, SymExprNode *new_parent) 
   : NonUniformDegreeTreeNodeWithDBIO (new_parent)
{
  Construct(SYMEXPR_ARITH_CONST, TYPE_REAL, 0, r_const);
}

SymExprNode::SymExprNode(char *vname, SymExprNode *new_parent) 
   : NonUniformDegreeTreeNodeWithDBIO (new_parent)
{
  Construct(SYMEXPR_ARITH_VAR, TYPE_UNKNOWN, 0, 0.0, vname);
}
 
SymExprNode::SymExprNode(SymExprNodeType a_type,
			 SymExprNode *leftson, SymExprNode *rightson,
			 SymExprNode *new_parent) 
   : NonUniformDegreeTreeNodeWithDBIO (new_parent)
{
  assert(is_arith_op(a_type) || is_cond_op(a_type) || is_log_op(a_type));
  Construct(a_type, TYPE_UNKNOWN, 0, 0.0, (char *) 0, leftson, rightson);
}

/* Relink a node, giving it a new parent.
*/

void SymExprNode::link(SymExprNode *new_parent)
{
    NonUniformDegreeTreeNode::link((NonUniformDegreeTreeNode *) new_parent);
}

/* Get the left son of a binary operator (like a "+" node).
*/

SymExprNode *
SymExprNode::leftson()
{
  int kids = ChildCount();

  if (!kids)
    return 0;
  else return FirstChild();
}
  
/* Get the right son of a binary operator (like a "+" node).
*/

SymExprNode *
SymExprNode::rightson()
{
  SymExprNode *firstchild;
  int kids = ChildCount();

  if (kids < 2)
    return 0;
  else {
    firstchild = FirstChild();
    return firstchild->NextSibling();
  }
}
  
/*
 * The following set of "New<mumble>" functions are virtual functions
 * which can be overridden in derived classes. They are called during
 * expression simplification. When we simplify an expression like "(+ 1
 * 2)", for example, we have to create a new constant node, "3". Since the
 * tree may be composed of nodes in a derived class, we want to call the
 * derived class constructor, not the SymExprNode constructor. This is
 * accomplished via the "New<mumble>" functions -- a derived class will
 * override these with new methods.
 */

/* Create an integer constant node.
*/

SymExprNode *
SymExprNode::NewIconst(int i_const)
{
  return new SymExprNode(i_const);
}

/* Create a "real" constant node
*/

SymExprNode *
SymExprNode::NewRconst(double r_const)
{
  return new SymExprNode(r_const);
}

/* Create a "bottom" node or DERIVED type node
*/

SymExprNode *
SymExprNode::NewNode(SymExprNodeType a_type)
{
  return new SymExprNode(a_type);
}

/* Create a variable node
*/

SymExprNode *
SymExprNode::NewVar(char *v_name)
{
  return new SymExprNode(v_name);
}

/* Create an arithmetic operator node
*/

SymExprNode *
SymExprNode::NewArithOp(SymExprNodeType a_type,
			SymExprNode *left_son,
			SymExprNode *right_son)
{
  return new SymExprNode(a_type, left_son, right_son);
}

/* Create a conditional/logical operator node
*/

SymExprNode *
SymExprNode::NewCondOp(SymExprNodeType a_type,
		       SymExprNode *left_son,
		       SymExprNode *right_son)
{
  return new SymExprNode(a_type, left_son, right_son);
}

/* Various random methods.
*/

int SymExprNode::GetConstDataType()
{
  assert(atype == SYMEXPR_ARITH_CONST);
  return dtype;
}

int SymExprNode::GetIntegerConstVal()
{
  assert(atype == SYMEXPR_ARITH_CONST);
  assert(dtype == TYPE_INTEGER);
  return arith.iconst;
}

double SymExprNode::GetRealConstVal()
{
  assert(atype == SYMEXPR_ARITH_CONST);
  assert(dtype == TYPE_REAL);
  return arith.rconst;
}

char *SymExprNode::GetVarName()
{
  assert(atype == SYMEXPR_ARITH_VAR);
  return arith.name;
}

/* Use a virtual destructor to support derived classes.
*/

SymExprNode::~SymExprNode()
{
  if (atype == SYMEXPR_ARITH_VAR)
    sfree(arith.name);
}

/*
 * Copy the contents of one node into another, ignoring the node's
 * children. The intent of this routine is that data at the node should be
 * duplicated (i.e. after this routine, the target node will not share any
 * pointers or other data with the old node), but that the children of the
 * node be ignored,
 */

void SymExprNode::copynode(SymExprNode *target)
{
  target->atype = atype;
  target->arith = arith;
  target->dtype = dtype;
  if (atype == SYMEXPR_ARITH_VAR)
    target->arith.name = ssave(arith.name);
}

/*
 * Clone a node. This function creates a clone of a node which has the
 * identical data but does not have any children.
 */

SymExprNode *
SymExprNode::nodeclone()
{
  SymExprNode *n = new SymExprNode;

  copynode(n);
  return n;
}

/* Clone an entire tree. 
*/

SymExprNode *
SymExprNode::clone()
{
  SymExprNode *n = this->nodeclone();
  SymExprNode *childptr, *childclone;
  int kids;

  if (!(kids = ChildCount()))
    return n;

  /* Clone the children.
  */
  for (childptr = FirstChild(); kids; --kids) {
    childclone = childptr->clone();
    childclone->link(n);
    childptr = childptr->NextSibling();
  }

  /* Return the cloned tree.
  */
  return n;
}

void SymExprNode::node_to_cvals(int &constinfo, int &iconst, double &rconst)
{
  if (!(atype == SYMEXPR_ARITH_CONST)) {
    constinfo = TYPE_UNKNOWN;
    return;
  } else {
    constinfo = dtype;
    switch(dtype) {
      case TYPE_REAL:
      {
	rconst = arith.rconst; return;
      }
      case TYPE_INTEGER: 
      {
	iconst = arith.iconst; return;
      }
      default: return;
    }
  }
}

SymExprNode *
SymExprNode::cvals_to_node(int constinfo, int iconst, double rconst)
{
  switch(constinfo) {
    case TYPE_INTEGER:
    {
      return NewIconst(iconst);
    }
    case TYPE_REAL:
    {
      return NewRconst(rconst);
    }
    default:
    {
      return 0;
    }
  }
}

/* Evaluate an integer operator.
*/

static int eval_iop(int l, int r, SymExprNodeType typ)
{
  switch(typ) {
    case SYMEXPR_ARITH_ADD: return l + r;
    case SYMEXPR_ARITH_MULT: return l * r;
    case SYMEXPR_ARITH_SUB: return l - r;
    case SYMEXPR_ARITH_DIV: return (r == 0) ? 0 : (l / r);
    case SYMEXPR_COND_EQ: return (l == r) ? 1 : 0;
    case SYMEXPR_COND_GE: return (l >= r) ? 1 : 0;
    case SYMEXPR_COND_NE: return (l != r) ? 1 : 0;
    case SYMEXPR_COND_LT: return (l < r) ? 1 : 0;
    case SYMEXPR_COND_GT: return (l > r) ? 1 : 0;
    case SYMEXPR_COND_LE: return (l <= r) ? 1 : 0;
    case SYMEXPR_LOG_NOT: return (!l) ? 1 : 0;
    case SYMEXPR_LOG_AND: return (l && r) ? 1 : 0;
    case SYMEXPR_LOG_OR: return (l || r) ? 1 : 0;
    default: assert(1 == 0);			    
  }
  return 0;
}

/* Evaluate a floating point operator.
*/

static double eval_fop(double l, double r, SymExprNodeType typ)
{
  switch(typ) {
    case SYMEXPR_ARITH_ADD: return l + r;
    case SYMEXPR_ARITH_MULT: return l * r;
    case SYMEXPR_ARITH_SUB: return l - r;
    case SYMEXPR_ARITH_DIV: return (r == 0.0) ? 0.0 : (l / r);
    default: assert(1 == 0);			    
  }
  return 0;
}

/*
 * Required preconditions: node is an arithmetic, logical, or conditional
 * operator constant children.
 */

SymExprNode *
SymExprNode::eval_op()
{
  SymExprNode *ls = leftson();
  SymExprNode *rs = rightson();
  
  /* If both args are integers, then return an integer
  */
  if (ls->dtype == TYPE_INTEGER && 
      rs->dtype == TYPE_INTEGER) {
    int ival = eval_iop(ls->arith.iconst, rs->arith.iconst, atype);
    return NewIconst(ival);
  } else {
    if (is_cond_op(atype) || is_log_op(atype)) {
      /*
       * We appear to have a type mismatch here. Print a warning and
       * return bottom.
       */
      fprintf(stderr, "condition/logical op type mismatch.\n");
      return NewNode(SYMEXPR_ARITH_BOTTOM);
    }

    /* Otherwise, coerce everything to double and evaluate.
     */
    double lv = (ls->dtype == TYPE_INTEGER) ?
    ((double) ls->arith.iconst) : ls->arith.rconst;
    double rv = (rs->dtype == TYPE_INTEGER) ?
      ((double) rs->arith.iconst) : rs->arith.rconst;
    double rval = eval_fop(lv, rv, atype);
    return NewRconst(rval);
  }
}

SymExprNode *
SymExprNode::simplify_node(Boolean &deletethis,
			   SymExprNode_mapnamefunc mapfunc,
			   void *user)
{
  int cinfo = TYPE_UNKNOWN, iconst = JUNKVAL;
  double rconst = FJUNKVAL;
  SymExprNode *rval;

  rval = simplify_node_internal(deletethis, cinfo, iconst, rconst, mapfunc, user);
  if (!rval) {
    return cvals_to_node(cinfo, iconst, rconst);
  } else if (cinfo == TYPE_UNKNOWN) {
    return rval;
  } else {
    SymExprNode *c = cvals_to_node(cinfo, iconst, rconst);
    return NewArithOp(SYMEXPR_ARITH_ADD, rval, c);
  }
}

/*
 * This function is called by SymExprNode::simplify(). Its job is to
 * simplify the node that it is invoked on, if possible. See the
 * declaration for this function in class SymExprNode for a more complete
 * description of it. Also see the definition of the function 'simplify',
 * which calls this routine.
 * 
 * Virtual functions are used in a couple of ways. First, this routine uses
 * the virtual functions "NewIconst()", "NewRconst()", ..., to create new
 * nodes during constant folding. These routines are virtual since if the
 * tree is actually composed of nodes in a derived class, we want to use
 * the derived class's constructor, not the SymExprNode constructor.
 * Secondly, this function itself is virtual, in case the derived class
 * needs to radically change the simplifcation process (for example, a
 * derived class might want to deal with integer/real numbers in a
 * different way).
 */

SymExprNode *
SymExprNode::simplify_node_internal(Boolean &deletethis,
			   int &constinfo,
			   int &iconst, 
			   double &rconst,
			   SymExprNode_mapnamefunc mapfunc,
			   void *map_user)
{
  SymExprNode *n;
  SymExprNode *rval;

  deletethis = false;
  constinfo = TYPE_UNKNOWN;
  switch(atype) {

    case SYMEXPR_ARITH_CONST:
    {
      node_to_cvals(constinfo, iconst, rconst);
      rval = 0;
      deletethis = true;
      break;
    }
    
    case SYMEXPR_ARITH_BOTTOM:
    {
      rval = this;
      break;
    }

    case SYMEXPR_ARITH_VAR:
    {
      if (mapfunc) {
	constinfo = (*mapfunc)(arith.name, iconst, rconst, map_user);
	if (constinfo != TYPE_UNKNOWN) {
	  rval = NULL;
	  deletethis = true;
	  break;
	}
      }
      rval = this;
      break;
    }
	
    case SYMEXPR_ARITH_ADD:
    case SYMEXPR_ARITH_SUB:
    case SYMEXPR_ARITH_MULT:
    case SYMEXPR_ARITH_DIV:
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
      SymExprNode *ls = leftson(), *rs = rightson();
      int lst = ls->atype;
      int rst = rs->atype;
      int bottom_desc = (lst == SYMEXPR_ARITH_BOTTOM ||
			  rst == SYMEXPR_ARITH_BOTTOM);
      int derived_desc = (lst == SYMEXPR_ARITH_DERIVED ||
			 rst == SYMEXPR_ARITH_DERIVED);

      /*
       * If one descendent is a BOTTOM node, and neither is derived, then
       * return BOTTOM.
       */
      if (bottom_desc && !derived_desc) {
	rval = NewNode(SYMEXPR_ARITH_BOTTOM);
	deletethis = true;
	break;
      }

      /* Special case (+ const X), since it happens all the time.
      */
      if (atype == SYMEXPR_ARITH_ADD && 
	  (((rval = ls) && (n = rs) && 
	    (lst != SYMEXPR_ARITH_CONST) &&
	    (rst == SYMEXPR_ARITH_CONST)) ||
	   ((rval = rs) && (n = ls) &&
	    (rst != SYMEXPR_ARITH_CONST) &&
	    (lst == SYMEXPR_ARITH_CONST)))) {
	deletethis = true;
	rval->unlink();
	n->node_to_cvals(constinfo, iconst, rconst);
	break;
      }

      /*
       * At this point, we've taken care of BOTTOM and special cases
       * involving derived nodes. Reject anything that's not 2 constants.
       */
      if (!(ls->atype == SYMEXPR_ARITH_CONST && 
	    rs->atype == SYMEXPR_ARITH_CONST)) {
	rval = this;
	deletethis = false;
	constinfo = TYPE_UNKNOWN;
	break;
      }

      /*
       * Otherwise, evaluate. 
       */
      deletethis = true;
      rval = 0;
      n = eval_op();
      n->node_to_cvals(constinfo, iconst, rconst);
      delete n;
      break;
    }
    
    default: assert(1 == 0);
  }

  return rval;
}

SymExprNode *
SymExprNode::test_and_sum(SymExprNode *l, SymExprNode *r) 
{
  if (!l && !r)
    return 0;
  else if (!l)
    return r;
  else if (!r)
    return l;
  else {
    return NewArithOp(SYMEXPR_ARITH_ADD, l, r);
  }
}

void SymExprNode::sum_constant_terms(int &constinfo,
				     int &iconst, 
				     double &rconst,
				     int cil, int icl, double rcl,
				     int cir, int icr, double rcr)
{
  SymExprNode *l, *r, *a, *t;

  if (cil == TYPE_UNKNOWN && cir == TYPE_UNKNOWN) {
    constinfo = TYPE_UNKNOWN;
    return;
  }
  
  if (cil == TYPE_UNKNOWN) {
    l = NewIconst(0);
  } else l = cvals_to_node(cil, icl, rcl);

  if (cir == TYPE_UNKNOWN) {
    r = NewIconst(0);
  } else r = cvals_to_node(cir, icr, rcr);

  a = NewArithOp(SYMEXPR_ARITH_ADD, l, r);
  t = a->eval_op();
  delete a;
  t->node_to_cvals(constinfo, iconst, rconst);
  delete t;
}

SymExprNode *SymExprNode::simplify_add(Boolean &deletethis, 
				       SymExprNode *orig,
				       int &constinfo,
				       int &iconst, 
				       double &rconst,
				       SymExprNode_mapnamefunc mapfunc,
				       void *user)
{
  SymExprNode *sleft, *sright;
  SymExprNode *l = orig->leftson();
  SymExprNode *r = orig->rightson();
  SymExprNode *csum, *ssum, *n;
  int cil = TYPE_UNKNOWN, cir = TYPE_UNKNOWN;
  int icl = JUNKVAL, icr = JUNKVAL;
  double rcl = FJUNKVAL, rcr = FJUNKVAL;
  
  deletethis = true;
  sleft = l->simplify_internal(cil, icl, rcl, mapfunc, user);
  sright = r->simplify_internal(cir, icr, rcr, mapfunc, user);
  ssum = test_and_sum(sleft, sright);
  sum_constant_terms(constinfo, iconst, rconst,
		     cil, icl, rcl,
		     cir, icr, rcr);
  if ((n = orig->Parent()) && (n->atype == SYMEXPR_ARITH_ADD)) {
    return ssum;
  } else {
    csum = cvals_to_node(constinfo, iconst, rconst);
    constinfo = TYPE_UNKNOWN;
    return test_and_sum(ssum, csum);
  }
  // return 0; /* NOT REACHED */
}
    
SymExprNode *
SymExprNode::combine_cvals_and_node(SymExprNode *n, 
				    int constinfo, 
				    int iconst,
				    double rconst)
{
  if (!n)
    return cvals_to_node(constinfo, iconst, rconst);
  else if (constinfo == TYPE_UNKNOWN)
    return n;
  else {
    SymExprNode *c = cvals_to_node(constinfo, iconst, rconst);
    return NewArithOp(SYMEXPR_ARITH_ADD, n, c);
  }
}

SymExprNode *
SymExprNode::simplify(SymExprNode_mapnamefunc mapfunc, void *user)
{
  SymExprNode *n;
  int iconst = JUNKVAL, cinfo = TYPE_UNKNOWN;
  double rconst = FJUNKVAL;

  n = simplify_internal(cinfo, iconst, rconst, mapfunc, user);
  return combine_cvals_and_node(n, cinfo, iconst, rconst);
}

/*
 * Return a simplified version of an expression tree. The original tree
 * remains unmodified. The basic model for simplification is:
 * 
 * 0)   let the original node be called 'O'
 * 1)   create a clone of 'O', call it 'N' (note that the semantics
 *      of node cloning specify that N now has no children)
 * 2)   if the O has children, then for each child C:
 * 2.1)    recursively call simplify() on C; call the resulting tree CS
 * 2.2)    link CS to N
 * 3)   call simplify_node() on N
 * 4)   return N
 * 
 * In order to avoid memory leaks, if the simplify_node routine decides to
 * return a new node rather than returning the node it was invoked on, it
 * must free the node it was invoked on.
 * 
 * This method of constructing the simplify routine is designed to allow
 * new derived classes to have new node types *without* having to rewrite
 * this routine (they will have to rewrite 'simplify_node()', however)
 * Note the use of the virtual method 'clone', which will cause the
 * derived or base class constructor accordingly.
 */

SymExprNode *
SymExprNode::simplify_internal(int &constinfo,
			       int &iconst, 
			       double &rconst,
			       SymExprNode_mapnamefunc mapfunc,
			       void *user)
{
  SymExprNode *childptr, *simplified_child;
  int kids = ChildCount();
  SymExprNode *n, *rval;
  Boolean deletethis;

  constinfo = TYPE_UNKNOWN;
  n = this->nodeclone();

  /* Treat addition as a special case
  */
  if (atype == SYMEXPR_ARITH_ADD) {
    rval = n->simplify_add(deletethis, this,
			   constinfo, iconst,
			   rconst, mapfunc, user);
  } else {
    for (childptr = FirstChild(); kids; --kids) {
      simplified_child = childptr->simplify(mapfunc, user);
      simplified_child->link(n);
      childptr = childptr->NextSibling();
    }
    if (atype != SYMEXPR_ARITH_DERIVED)
      rval = n->simplify_node_internal(deletethis, constinfo, iconst,
				       rconst, mapfunc, user);
    else
      rval = n->simplify_node(deletethis, mapfunc, user);
  }
  
  if (deletethis)
    delete n;
  return rval;
}

/*
 * Evaluate a tree. This boils down to trying to simplify it to a
 * constant.
 */

int SymExprNode::eval(int &ival, double &fval,
		      SymExprNode_mapnamefunc mapfunc, void *user)
{
  SymExprNode *n = this->simplify(mapfunc, user);

  if (n->atype == SYMEXPR_ARITH_CONST)
    if (n->dtype == TYPE_INTEGER) {
      ival = n->arith.iconst;
      delete n;
      return TYPE_INTEGER;
    } else if (n->dtype == TYPE_REAL) {
      fval = n->arith.rconst;
      delete n;
      return TYPE_REAL;
    }
  
  delete n;
  return TYPE_UNKNOWN;
}

/*
 * Perform substitution on a tree. The intent here is to provide a general
 * mechanism for walking the tree and substituting new expressions for
 * various pieces of it (example: substituting a constant for all
 * occurrences of a particular variable).
 * 
 * Semantics for the substitution function are as follows. It is passed a
 * pointer to the current node in the tree, and a (void *) which
 * corresponds to the value originally passed to "substitute". It then
 * returns either 0, indicating that no substition should take place at
 * that node, or a pointer to a new expression, indicating that the new
 * expression should appear in the new tree instead of this node. The old
 * tree (i.e. the node passed to the routine) is left UNMODIFIED.
 * 
 * See the method SymExprNode::substitute_var for an example of how to use
 * this method.
 */

SymExprNode *
SymExprNode::substitute(SymExprNode_substfunc func, void *user)
{
  SymExprNode *childptr, *childresult;
  SymExprNode *n;
  int kids;

  /*
   * Call the user-supplied substitution function, passing it this node.
   * If the substitution function returns a non-NULL value, then this is a
   * signal to replace the tree at this point with the result. If the
   * value is null, then simplify proceed with the cloning of the tree.
   */
  if ((n = (*func)(this, user)))
    return n;

  /*
   * Otherwise, clone this node and call ourselves recursively on all the
   * children.
   */
  n = this->nodeclone();

  if (!(kids = ChildCount()))
    return n;

  /* Recursively substitute in children.
  */
  for (childptr = FirstChild(); kids; --kids) {
    childresult = childptr->substitute(func, user);
    childresult->link(n);
    childptr = childptr->NextSibling();
  }

  /* Return the result.
  */
  return n;
}

/*
 * Local structure used only by "substitute_var" and it's callback
 * function (used as a place to dump information into a single place).
 */

typedef struct local_substvar_info__ {
  SymExprNode *e;
  char *var;
} local_substvar_info;

/*
 * Callback function called by SymExprNode::substitute, used for
 * subsituting expressions for variables.
 */

static SymExprNode *local_substvar_func(SymExprNode *e, void *info)
{
  local_substvar_info *i = (local_substvar_info *) info;
  SymExprNode *subst = i->e;
  char *var = i->var;

  /*
   * If this is a variable, then return a copy of the expression.
   * Otherwise return 0 to indicate that no substitution should take place
   * at this node.
   */
  if (e->GetNodeType() == SYMEXPR_ARITH_VAR &&
      !strcmp(e->GetVarName(), var))
    return subst->clone();
  else return 0;
}

/*
 * Substitute an expression for all occurences of a variable in a given
 * tree. Current tree is left unmodified; a new tree is built and
 * returned. See the class definition of SymExprNode for more info.
 */

SymExprNode *
SymExprNode::substitute_var(char *var, SymExprNode *subst_expr)
{
  local_substvar_info i;
  i.var = var;
  i.e = subst_expr;
  return this->substitute(local_substvar_func, (void *) &i);
}

/*
 * Write out an expression tree to a database file. Returns 0 if write was
 * successful, or EOF otherwise (ex: we ran out of disk space).
 */

int SymExprNode::WriteUpCall(FormattedFile& port)
{

#ifdef VERBOSE_IO_FORMAT
  // if (port.p_putc(IO_CHAR_START_SYMEXPRNODE) == EOF) return EOF;
#endif

  if (port.Write((int) atype) == EOF ||
      port.Write((int) dtype) == EOF) return EOF;

  switch(atype) {
    case SYMEXPR_ARITH_VAR:
    {
      if (port.Write(arith.name) == EOF) return EOF;
      break;
    }
    case SYMEXPR_ARITH_CONST:
    {
      if (dtype == TYPE_REAL) {
	if (port.Write(arith.rconst) == EOF) return EOF;
      } else {
	if (port.Write(arith.iconst) == EOF) return EOF;
      }
      break;
    }
    default: break;
  }

#ifdef VERBOSE_IO_FORMAT
  // if (port.p_putc(IO_CHAR_END_SYMEXPRNODE) == EOF) return EOF;
#endif

  return 0; /* success */
}

/* Read an expression tree from the database.
*/
  
int SymExprNode::ReadUpCall(FormattedFile& port)
{
  char buf[1024];
  int i;

#ifdef VERBOSE_IO_FORMAT
  // if ((i = port.p_getc()) == EOF) return EOF;
  // if (i != IO_CHAR_START_SYMEXPRNODE)
  //   fprintf(stderr, "SymExprNode::read -- no magic char\n");
#endif

  if (port.Read(i) == EOF) return EOF;
  atype = (SymExprNodeType) i;
  if (port.Read(dtype) == EOF) return EOF;

  switch(atype) {
    case SYMEXPR_ARITH_VAR:
    {
      if (port.Read(buf, 1023) == EOF) return EOF;
      arith.name = ssave(buf);
      break;
    }
    case SYMEXPR_ARITH_CONST:
    {
      if (dtype == TYPE_REAL) {
	if (port.Read(arith.rconst) == EOF) return EOF; 
      } else {
	if (port.Read(arith.iconst) == EOF) return EOF;
      }
      break;
    }
    default: break;
  }

#ifdef VERBOSE_IO_FORMAT
  // if (port.p_getc() == EOF) return EOF;
#endif

  return 0; /* success */
}

#define INDENTIT(filep,xx) { int _ilc; for (_ilc = xx; _ilc; --_ilc) \
			     fprintf(filep, " "); }


void SymExprNode::print(FILE *fp, int indent_level, Boolean indentnode)
{
  int i, nchildren = ChildCount();
  SymExprNode *n;
  char **child_strings;

  if (indentnode)
    INDENTIT(fp, indent_level);
  child_strings = print_node(fp);

  for (i = 0, n = FirstChild(); nchildren; --nchildren, i++) {
    if (child_strings) {
      INDENTIT(fp, indent_level+2);
      fprintf(fp, "%s", child_strings[i]);
      n->print(fp, indent_level+2, false);
    } else
      n->print(fp, indent_level+2, true);
    n = n->NextSibling();
  }

  if (child_strings) {
    for (i = 0; child_strings[i]; i++)
      free_mem((void*) child_strings[i]);
    free_mem((void*) child_strings);
  }
}
  
char **SymExprNode::print_node(FILE *fp)
{
  switch(atype) {

    case SYMEXPR_ARITH_CONST:
    {
      if (dtype == TYPE_INTEGER)
	fprintf(fp, "%d\n", arith.iconst);
      else if (dtype == TYPE_REAL)
	fprintf(fp, "%f\n", arith.rconst);
      break;
    }

    case SYMEXPR_ARITH_VAR: {
      fprintf(fp, "VAR '%s'\n", arith.name);
      break;
    }

    case SYMEXPR_ARITH_BOTTOM: {
      fprintf(fp, "BOTTOM\n");
      break;
    }

    case SYMEXPR_ARITH_ADD: 
    case SYMEXPR_ARITH_SUB: 
    case SYMEXPR_ARITH_MULT: 
    case SYMEXPR_ARITH_DIV:
    case SYMEXPR_LOG_NOT:
    case SYMEXPR_LOG_AND:
    case SYMEXPR_LOG_OR:
    case SYMEXPR_COND_EQ:
    case SYMEXPR_COND_NE:
    case SYMEXPR_COND_LT:
    case SYMEXPR_COND_GT:
    case SYMEXPR_COND_LE:
    case SYMEXPR_COND_GE: {
      fprintf(fp, "%s\n",
	      atype == SYMEXPR_ARITH_ADD ? "ADD" :
	      atype == SYMEXPR_ARITH_SUB ? "SUBTRACT" :
	      atype == SYMEXPR_ARITH_MULT ? "MULTIPLY" :
	      atype == SYMEXPR_ARITH_DIV ? "DIVIDE" : 
	      atype == SYMEXPR_COND_GE ? ".GE." :
	      atype == SYMEXPR_COND_LE ? ".LE." :
	      atype == SYMEXPR_COND_GT ? ".GT." :
	      atype == SYMEXPR_COND_LT ? ".LT." :
	      atype == SYMEXPR_COND_EQ ? ".EQ." :
	      atype == SYMEXPR_COND_NE ? ".NE." :
	      atype == SYMEXPR_LOG_AND ? ".AND." :
	      atype == SYMEXPR_LOG_OR ? ".OR." :
	      atype == SYMEXPR_LOG_NOT ? ".NOT." : "??");
      break;
    }

    default: assert(1 == 0);
  }
  return 0;
}

int SymExprNode::operator==(SymExprNode &tmp)
{
  if (compare(*this, tmp) == false)
    return(0);
  if ( *(this->leftson()) != *(tmp.leftson()) )
      return(0);
  if ( *(this->rightson()) != *(tmp.rightson()) )
      return(0);
  return 1;
}

int SymExprNode::operator!=(SymExprNode &tmp)
{
return (! (*this == tmp);
}

SymExprNode & SymExprNode::operator=(SymExprNode &tmp)
{
  // First the function completely destroys the left and right
  // child of *this node using the destructor
  if(this->leftson())
    delete this->leftson();
  if(this->rightson())
    delete this->rightson();

  // Now shallow copy tmp into *this node, first making sure the name
  // field is freed
  if (this->atype == SYMEXPR_ARITH_VAR) //this should be done in the
    if(this->arith.name)		// copy routine
      sfree(this->arith.name);
  this->copynode(&tmp);

  // Now deep copy its children
  SymExprNode *left_son = tmp.leftson()->clone();
  SymExprNode *right_son = tmp.rightson()->clone(); 
  // link the two children into the parent node
  left_son->link(this);
  right_son->link(this);  

  return( (SymExprNode &)(*this));
}

Boolean compare(SymExprNode &tmp1, SymExprNode &tmp2)
{

  if (tmp1.atype != tmp2.atype)
    return false;
  
  switch(tmp1.atype)
  {
    case SYMEXPR_ARITH_CONST:
      {
	if (tmp1.dtype != tmp2.dtype) 
	  return false;
	if(tmp1.dtype == TYPE_INTEGER)
	  if ( tmp1.arith.iconst != tmp2.arith.iconst )
	    return false;
	if(tmp1.dtype == TYPE_REAL)
	  if ( tmp1.arith.rconst != tmp2.arith.rconst )
	    return false;

	assert(1==0);
	return true;
      }
    case SYMEXPR_ARITH_VAR:
      {
	if (strcmp(tmp1.arith.name, tmp2.arith.name))\
	  return false;
	return true;
      }
    case SYMEXPR_ARITH_ADD:
    case SYMEXPR_ARITH_SUB:
    case SYMEXPR_ARITH_MULT:
    case SYMEXPR_ARITH_DIV:
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
	if (tmp1.ChildCount() != tmp2.ChildCount() )
	  return false;
	return true;
      }
    case SYMEXPR_ARITH_DERIVED:
    case SYMEXPR_ARITH_BOTTOM:
      {
	return true;  //don't know what this means
      }
    default:
      {
	assert(1==0);
      }
    }
}

void print_SymExprNode(SymExprNode *p)
{
  p->print(stdout);
}


