/* $Id: pt_util.C,v 1.2 1997/10/30 15:28:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	pt_util.c		Utility routines for pt
					
	History
	~~~~~~~
	09 Sep 90  cwt  Collected from several pt files, mostly by mhb

*/
/*---------------------------------------------------------------------

	This file exports the following utilities:

	void                    pt_get_loop_bounds()
        Boolean                 pt_is_loop_with_inductive()
	Boolean                 pt_loop_is_normal()
	Boolean                 pt_loop_get_count()
        void			pt_flip_nodes()
	void			pt_zero_var()
	void			pt_check_min_max()
	int			pt_convert_to_int()
	void			pt_itoa()
	AST_INDEX		pt_gen_const()
	AST_INDEX		pt_gen_ident()
	AST_INDEX		pt_gen_int()
	AST_INDEX		pt_gen_add()
	AST_INDEX		pt_gen_sub()
	AST_INDEX		pt_gen_mul()
	AST_INDEX		pt_gen_div()
	AST_INDEX		pt_gen_invoke()
	AST_INDEX		pt_gen_call()
	AST_INDEX		pt_gen_comment()
	AST_INDEX		pt_simplify_expr()
	Boolean			pt_comp_inv_name()
	Boolean			pt_expr_equal()
	void			pt_tree_replace()
	AST_INDEX		pt_add_const()
	void			pt_fold_term()
	AST_INDEX		pt_del_unary_minus()
	AST_INDEX		pt_simplify_expr()
	AST_INDEX		pt_find_var_node()
	Boolean			pt_find_var()
	void			pt_sep_linear()
	void			pt_separate_linear()
	void			pt_get_coeff()
	int			pt_perf_nest()
	void			pt_clear_info()
	void			pt_var_replace()
	void			pt_var_add()
	void			pt_var_mul()
	Boolean			pt_eval()
	AST_INDEX		pt_simplify_node()
	Pt_ref_params		*pt_refs()
	char			*ast2str()	

*/

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

#ifdef WAITING_FOR_PACO
#include <libs/moduleAnalysis/valNum/val.h>
#endif

#include <libs/frontEnd/ast/groups.h>
/*-------------*/
/* local defs  */

typedef struct  /* used for walk_expression */
{
	AST_INDEX node;
	char *var;
	int value;
} Pt_walk_parms;

#define MAX_IVS			64		/* max # of loop IVs found		*/
#define MAX_SUBLS		256		/* max # of subscript lists */
#define MAX_STMTS		256		/* max # of statements */
#define MAX_DEFS		256		/* max # of variables defined	*/
#define MAX_USES		1024	/* max # of variables used		*/

/*---------------*/
/* forward decls */

STATIC(int, pt_walk_replace,(AST_INDEX expr, Pt_walk_parms *parms));
STATIC(int, pt_walk_add,(AST_INDEX expr, Pt_walk_parms *parms));
STATIC(int, pt_walk_mul,(AST_INDEX expr, Pt_walk_parms *parms));
STATIC(int, pt_walk_clear_info,(AST_INDEX expr, Generic info_side_array));
STATIC(int, pt_stmt_refs,(AST_INDEX stmt, int lvl, Pt_ref_params *refs));
STATIC(int, pt_expr_uses,(AST_INDEX expr, Pt_ref_params *refs));


/*------------------------------------------------------------------------a
    pt_get_loop_bounds -- return the AST_INDEXes of the loop induction
    var, lower bound, upper bound and stepsize. 
    1/15/93RvH: moved here from ped_cp/perf/PEDInterface.c
  -------------------------------------------------------------------------*/
void
pt_get_loop_bounds(AST_INDEX stmt, AST_INDEX *ivar, AST_INDEX *lbound, 
                   AST_INDEX *ubound, AST_INDEX *stepsize)
{
  AST_INDEX  control;

  *ivar   = AST_NIL;
  *lbound = AST_NIL;
  *ubound = AST_NIL;
  *stepsize = AST_NIL;

  if (!((pt_is_loop_with_inductive(stmt) && is_do(stmt))
	|| (is_parallelloop(stmt))))
    return;
  
  control = gen_get_loop_control(stmt);
  *ivar   = gen_INDUCTIVE_get_name(control);
  *lbound = gen_INDUCTIVE_get_rvalue1(control);
  *ubound = gen_INDUCTIVE_get_rvalue2(control);
  *stepsize = gen_INDUCTIVE_get_rvalue3(control);
  
}


/**********************************************************************
 * pt_is_loop_with_inductive()  Determine whether <loop_node> has an
 *                           inductive subtree (not the case for
 *                           while loops).
 */
Boolean
pt_is_loop_with_inductive(AST_INDEX loop_node)
{
  Boolean flag;

  flag = BOOL(is_loop(loop_node)
	      && is_inductive(gen_get_loop_control(loop_node)));

  return flag;
}


/**********************************************************************
 * pt_loop_is_normal()  Determine whether a loop is in normal form
 *                      (lower bound == 1, stride == 1)
 */
Boolean                              /* Is this loop in normal form ? */
pt_loop_is_normal(AST_INDEX loop_node)         /* The loop to be analyzed */
{
  AST_INDEX var_node, lo_node, hi_node, step_node;
  int       lo, step;                /* Lower bound & step */
  Boolean   lo_flag, step_flag;      /* Non-constant symbolics ? */
  Boolean   loop_normal_flag;

  assert(is_do(loop_node));
  if (pt_is_loop_with_inductive(loop_node))
  {
    pt_get_loop_bounds(loop_node, &var_node, &lo_node, &hi_node, &step_node);

    lo_flag   = pt_eval(lo_node, &lo);
    if (step_node == AST_NIL) {
      step      = 1;
      step_flag = false;
    } else {
      step_flag = pt_eval(step_node, &step);
    }

    loop_normal_flag = BOOL((NOT(lo_flag) && (lo == 1)) &&
			    (NOT(step_flag) && (step == 1)));
  }
  else
  {
    loop_normal_flag = false;
  }

  return loop_normal_flag;
}


/**********************************************************************
 * pt_loop_get_count()  Find # of iterations etc.
 */
Boolean                             /* Constant # of iterations ? */
pt_loop_get_count(AST_INDEX loop_node,  /* The loop to be analyzed */
		  int *iter_cnt,        /* # of iterations (if constant) */
		  AST_INDEX *iter_cnt_node,    /* AST giving # of iterations */
		  Boolean *loop_normal_flag) /* Is this loop in normal form ? */
{
  AST_INDEX var_node, lo_node, hi_node, step_node;
  int       lo, hi, step;                             /* Bounds & step */
  Boolean   lo_flag, hi_flag, step_flag, iter_cnt_flag; /* Symbolics ? */
  assert(is_do(loop_node)); 
  
  if (pt_is_loop_with_inductive(loop_node))
  {
    pt_get_loop_bounds(loop_node, &var_node, &lo_node, &hi_node, &step_node);
    lo_flag   = pt_eval(lo_node, &lo);
    hi_flag   = pt_eval(hi_node, &hi);
    
    /* Construct & evaluate ((hi - lo) / step + 1) */
    /* Note that we have to generate a strict tree */
    *iter_cnt_node = pt_gen_sub(tree_copy(hi_node), tree_copy(lo_node));
    if (step_node == AST_NIL) {
      step      = 1;
      step_flag = false;
    } else {
      step_flag     = pt_eval(step_node, &step);
      *iter_cnt_node = pt_gen_div(*iter_cnt_node, tree_copy(step_node));
    }
    *iter_cnt_node = pt_gen_add(*iter_cnt_node, pt_gen_const("1"));
    
    /* Note that pt_simplify_expr() destroys the argument tree */
    *iter_cnt_node = pt_simplify_expr(*iter_cnt_node);
    iter_cnt_flag = NOT(pt_eval(*iter_cnt_node, iter_cnt));
    
    /* Is loop in normal form ? */
    *loop_normal_flag = BOOL(NOT(lo_flag) && (lo == 1) && 
			     NOT(step_flag) && (step == 1));
  }
  else
  {
    iter_cnt_flag     = false;
    *loop_normal_flag =  false;
  }
  return iter_cnt_flag;
}


/**************************************************************
 * pt_flip_nodes (node1,node2)
 *
 *   swap 2 subtrees in the AST
 *
 * Inputs:  node1 - node to flip
 *          node2 - node to flip
 *
 * Outputs: node1 = old node2
 *          node2 = old node1
 **************************************************************
 */
void
pt_flip_nodes(AST_INDEX node1, AST_INDEX node2)
{
   AST_INDEX     dummy1,dummy2;    
   
   dummy1 = tree_copy_with_type(node1);
   dummy2 = tree_copy_with_type(node2);
   tree_replace(node1,dummy2);
   tree_replace(node2,dummy1);
}


/**************************************************************
 * pt_zero_var (expr,var)
 *
 *   replace all occurrences of var by zero in expr
 *
 * Inputs: expr - expression tree to modify
 *         var  - variable to replace
 *
 * Outputs: updated expr
 **************************************************************
 */
void
pt_zero_var (AST_INDEX expr, char *var)
{
  AST_INDEX temp;

  while ((temp=pt_find_var_node(expr,var)) != AST_NIL)
    pt_tree_replace(temp,pt_gen_int(0));
}


/**************************************************************
 * pt_check_min_max(node,var,expr)
 *
 *   This routine tries to eliminate min and max functions
 *   that are added during loop interchange.  The second arg
 *   of the min/max function is assumed to contain var; the
 *   first arg is assumed to be independent of it.
 *
 * Inputs: node - AST node of min or max function
 *         var  - outer loop induction variable
 *         expr - one of the outer loop bounds
 **************************************************************
 */
void
pt_check_min_max(AST_INDEX node, char *var, AST_INDEX expr)
{
  AST_INDEX temp,arg1,arg2;

  if (!is_invocation(node))
    return;
  arg1 = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
  arg2 = list_next(arg1);
  temp = tree_copy_with_type(arg2);
  pt_var_replace(temp,var,expr);
  if (pt_expr_equal(temp,arg1))
    pt_tree_replace(node,tree_copy_with_type(arg2));
  tree_free(temp);
}



/*************************************************************
 * pt_convert_to_int (str)
 *
 *   Perform robust atoi type conversion.
 *
 * Inputs:  str - string to convert
 * Returns: integer value of str
 *************************************************************
 */
int 
pt_convert_to_int(char *str)
{
	int tempi,temp2;

	if (str[0] == '-')
	  return -pt_convert_to_int(&str[1]);

	for (tempi=0; *str!='\0'; str++) {
	  temp2 = tempi*10 + (*str-'0');
	  if (temp2 >= tempi)
	    tempi = temp2;
	  else {
	    tempi = MAXINT;
	    break;
	  }
	}

	return (tempi);	
}


/*************************************************************
 * pt_itoa (n,a)
 *
 *   Convert int to char array
 *
 * Inputs:  n - number to convert
 * Outputs: a - char array representing n
 *************************************************************
 */
void
pt_itoa(int n, char a[])
{
  int i,j;

  i=0;
  while (n > 0) {
    a[i++] = '0' + n%10;
    n = n / 10;
  }
  if (!i)
    a[i++] = '0';

  /* swap a[] end-for-end */
  for (j=0; j<i/2; j++) {
    a[i] = a[j];
    a[j] = a[i-j-1];
    a[i-j-1] = a[i];
  }

  a[i] = '\0';
}


/*************************************************************
 * pt_gen_const (str) - make integer constant node
 * pt_gen_ident (str) - make identifier node
 * pt_gen_int (num)   - make integer constant node
 * pt_gen_add (node1,node2) - return node1 + node2
 * pt_gen_sub (node1,node2) - return node1 - node2
 * pt_gen_mul (node1,node2) - return node1 * node2
 * pt_gen_div (node1,node2) - return node1 / node2
 * pt_gen_call (node1,node2) - return call node1(node2)
 * pt_gen_comment (str) - return C "str"
 *
 *   These routines create an AST node of the appropriate
 *   type.  In the case of add, sub, and mul, the input
 *   nodes are allowed to be AST_NIL, in which case they
 *   are treated as being zero.  If their result is zero,
 *   they may return AST_NIL.
 *
 *************************************************************
 */
AST_INDEX
pt_gen_const(char *str)
{
    AST_INDEX con;

    con = gen_CONSTANT();
    gen_put_text (con,str,STR_CONSTANT_INTEGER);
    return con;
}


AST_INDEX
pt_gen_ident(char *str)
{
    AST_INDEX temp;

    temp = gen_IDENTIFIER();
    gen_put_text(temp,str,STR_IDENTIFIER);
    return temp;
}


AST_INDEX
pt_gen_label_ref(char *str)
{
    AST_INDEX temp;

    temp = gen_LABEL_REF();
    gen_put_text(temp,str,STR_LABEL_REF);
    return temp;
}


AST_INDEX
pt_gen_label_def(char *str)
{
    AST_INDEX temp;

    temp = gen_LABEL_DEF();
    gen_put_text(temp,str,STR_LABEL_DEF);
    return temp;
}


AST_INDEX
pt_gen_int(int num)
{
  char str[20];
  AST_INDEX temp;

  pt_itoa(abs(num),str);
  temp = pt_gen_const(str);
  if (num < 0)
    temp = gen_UNARY_MINUS(temp);
  return temp;
}


AST_INDEX
pt_gen_add(AST_INDEX node1, AST_INDEX node2)
{
  if (node1 == AST_NIL)
    return node2;
  else if (node2 == AST_NIL)
    return node1;
  else
    return gen_BINARY_PLUS(node1,node2);
}


AST_INDEX
pt_gen_sub(AST_INDEX node1, AST_INDEX node2)
{
  if (node2 == AST_NIL)
    return node1;
  else if (node1 == AST_NIL)
    return gen_UNARY_MINUS(node2);
  else
    return gen_BINARY_MINUS(node1,node2);
}


AST_INDEX
pt_gen_mul(AST_INDEX node1, AST_INDEX node2)
{
	int neg1,neg2;
	AST_INDEX temp,temp2;

	if ((node1 == AST_NIL) || (node2 == AST_NIL))
	  return AST_NIL;
	else {
		neg1 = (gen_get_node_type(node1) == GEN_UNARY_MINUS);
		neg2 = (gen_get_node_type(node2) == GEN_UNARY_MINUS);
		if ((!neg1) && (!neg2))
		  return gen_BINARY_TIMES(node1,node2);
		if (!neg1) {                           /* promote unary minus */
			temp = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node2));
			tree_free(node2);
			temp2 = gen_BINARY_TIMES(node1,temp);
			return gen_UNARY_MINUS(temp2);
		}
		if (!neg2) {
			temp = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node1));
			tree_free(node1);
			temp2 = gen_BINARY_TIMES(temp,node2);
			return gen_UNARY_MINUS(temp2);
		}
		temp = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node1));
		temp2 = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node2));
		tree_free(node1);
		tree_free(node2);
		return gen_BINARY_TIMES(temp,temp2);
	}
}

AST_INDEX
pt_gen_div(AST_INDEX node1, AST_INDEX node2)
{
	int neg1,neg2;
	AST_INDEX temp,temp2;

	if ((node1 == AST_NIL) || (node2 == AST_NIL))
	  return AST_NIL;
	else {
		neg1 = (gen_get_node_type(node1) == GEN_UNARY_MINUS);
		neg2 = (gen_get_node_type(node2) == GEN_UNARY_MINUS);
		if ((!neg1) && (!neg2))
		  return gen_BINARY_DIVIDE(node1,node2);
		if (!neg1) {                          /* promote unary minus */
			temp = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node2));
			tree_free(node2);
			temp2 = gen_BINARY_DIVIDE(node1,temp);
			return gen_UNARY_MINUS(temp2);
		}
		if (!neg2) {
			temp = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node1));
			tree_free(node1);
			temp2 = gen_BINARY_DIVIDE(temp,node2);
			return gen_UNARY_MINUS(temp2);
		}
		temp = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node1));
		temp2 = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(node2));
		tree_free(node1);
		tree_free(node2);
		return gen_BINARY_DIVIDE(temp,temp2);
	}
}

AST_INDEX
pt_gen_invoke(char *name, AST_INDEX args)
{
    if (args == AST_NIL)
      args = list_create(args);

    return gen_INVOCATION(pt_gen_ident(name), args);
}

AST_INDEX
pt_gen_call(char *name, AST_INDEX args)
{
	return gen_CALL(AST_NIL, pt_gen_invoke(name, args));
}

AST_INDEX
pt_gen_comment(char *str)
{
	AST_INDEX node;

	node = gen_TEXT();
	gen_put_text(node, str, STR_COMMENT_TEXT);
	node = gen_COMMENT(node);
	return node;
}


/*************************************************************
 * pt_comp_inv_name (node,name)
 *
 *   Determine whether node is a call to function "name".
 *   Return true if it is.
 *
 * Inputs:  node - an AST node
 *          name - the name of the expected function
 * Returns: true or false
 *************************************************************
 */
Boolean
pt_comp_inv_name(AST_INDEX node, char *name)
{
  if (is_invocation(node))
    return BOOL(!strcmp(gen_get_text(gen_INVOCATION_get_name(node)),name));
  else
    return false;
}


/*************************************************************
 * pt_tree_replace(old_node,new_node)
 *
 *   Do improved tree_replace.  This routine frees whatever
 *   was at old_node.  Also, if old_node is the root, the
 *   routine still works, whereas tree_replace does not.
 *
 * Inputs:  old_node = node of tree to replace
 *          new_node = what to replace old_node with
 *************************************************************
 */
void
pt_tree_replace(AST_INDEX old_node, AST_INDEX new_node)
{
  AST_INDEX father, new_son;
  Generic idx;

  if (old_node == new_node)
    return;
  if (old_node == AST_NIL)
    old_node = gen_PLACE_HOLDER();
  if (in_list(old_node))
    tree_replace(old_node,new_node);
  else {
    father = ast_get_father(old_node);
    if (father != AST_NIL)
      idx = ast_which_son(father,old_node);
    new_son = tree_copy_with_type(new_node);
    if (father != AST_NIL)
      ast_put_son_n(father,idx,new_son);
    tree_free(old_node);
    tree_free(new_node);
  }
}


/*************************************************************
 * pt_add_const (expr,const)
 *
 *   Return an expression that is sum of expr and const
 *
 * Inputs:  expr - an expression AST
 *          const - an integer
 * Returns: an AST equal to expr+const
 *************************************************************
 */
AST_INDEX
pt_add_const (AST_INDEX expr, int constant)
{
  AST_INDEX newconst;

  if (expr == AST_NIL)
    return pt_gen_int(constant);
  else if (!constant)
    return expr;
  else {
    newconst = pt_gen_int(abs(constant));
    if (constant < 0)
      return gen_BINARY_MINUS(expr,newconst);
    else
      return gen_BINARY_PLUS(expr,newconst);
  }
}


/*************************************************************
 * pt_fold_term (exprin,exprout,constout)
 *
 *   Fold all constants in exprin.  Unfoldable terms are
 *   returned in exprout.
 *
 * Inputs:  exprin - expression to evaluate
 * Outputs: exprout - unfoldable terms
 *          constout - sum of constant terms
 *************************************************************
 */
void
pt_fold_term (AST_INDEX exprin, AST_INDEX *exprout, int *constout)
{
int con1,con2;
AST_INDEX expr1,expr2;

*constout = 0;
*exprout = AST_NIL;
if (exprin == AST_NIL)
	return;

/* switch on the node type */
switch (gen_get_node_type(exprin)) {

	case GEN_CONSTANT:
		if (str_get_type(gen_get_symbol(exprin)) == STR_CONSTANT_INTEGER)
		  *constout = pt_convert_to_int(gen_get_text(exprin));
		else
		  *exprout = tree_copy_with_type(exprin);
		break;
	case GEN_UNARY_MINUS:
		pt_fold_term (gen_UNARY_MINUS_get_rvalue(exprin),&expr1,&con1);
		*constout = -con1;
		if (expr1 != AST_NIL)
		   *exprout = gen_UNARY_MINUS(expr1);
		break;
   	case GEN_BINARY_PLUS:
		pt_fold_term(gen_BINARY_PLUS_get_rvalue1(exprin),&expr1,&con1);
		pt_fold_term(gen_BINARY_PLUS_get_rvalue2(exprin),&expr2,&con2);
		*constout = con1 + con2;
		*exprout = pt_gen_add(expr1,expr2);
		break;
   	case GEN_BINARY_MINUS:
		pt_fold_term(gen_BINARY_MINUS_get_rvalue1(exprin),&expr1,&con1);
		pt_fold_term(gen_BINARY_MINUS_get_rvalue2(exprin),&expr2,&con2);
		*constout = con1 - con2;
		*exprout = pt_gen_sub(expr1,expr2);
		break;
   	case GEN_BINARY_TIMES:
		pt_fold_term(gen_BINARY_TIMES_get_rvalue1(exprin),&expr1,&con1);
		pt_fold_term(gen_BINARY_TIMES_get_rvalue2(exprin),&expr2,&con2);
		if ((expr1 != AST_NIL) && (expr2 != AST_NIL)) {
		  expr1 = pt_add_const(expr1,con1);
		  expr2 = pt_add_const(expr2,con2);
		  *exprout = pt_gen_mul(expr1,expr2);
		}
		else if (expr1 != AST_NIL) {
		  expr1 = pt_add_const(expr1,con1);
		  switch (con2) {
		    case -1: *exprout = gen_UNARY_MINUS(expr1); break;
		    case 0 : tree_free(expr1); break;
		    case 1 : *exprout = expr1; break;
		    default:
		      *exprout = pt_gen_mul(expr1,pt_gen_int(con2));
		      break;
		  }
		}
		else if (expr2 != AST_NIL) {
		  expr2 = pt_add_const(expr2,con2);
		  switch (con1) {
		    case -1: *exprout = gen_UNARY_MINUS(expr2); break;
		    case 0 : tree_free(expr2); break;
		    case 1 : *exprout = expr2; break;
		    default:
		      *exprout = pt_gen_mul(expr2,pt_gen_int(con1));
		      break;
		  }
		}
		else
		  *constout = con1 * con2;
		break;
   	case GEN_BINARY_DIVIDE:
		pt_fold_term(gen_BINARY_DIVIDE_get_rvalue1(exprin),&expr1,&con1);
		pt_fold_term(gen_BINARY_DIVIDE_get_rvalue2(exprin),&expr2,&con2);
		if ((expr1 == AST_NIL) && (expr2 == AST_NIL)) {
		  if (con2)
		    *constout = con1/con2;
		  else
		    *exprout = gen_BINARY_DIVIDE(pt_gen_int(con1),pt_gen_int(con2));
		}
		else if (expr2 == AST_NIL) {
		  expr1 = pt_add_const(expr1,con1);
		  switch (con2) {
		    case -1: *exprout = gen_UNARY_MINUS(expr1); break;
		    case 1 : *exprout = expr1; break;
		    default:
		      *exprout = gen_BINARY_DIVIDE(expr1,pt_gen_int(con2));
		      break;
		  }
		}
		else {
		  expr1 = pt_add_const(expr1,con1);
		  expr2 = pt_add_const(expr2,con2);
		  *exprout = gen_BINARY_DIVIDE(expr1,expr2);
		}
		break;
	default:
		*exprout = tree_copy_with_type(exprin);
		break;
	}

}


/*************************************************************
 * pt_del_unary_minus (expr)
 *
 *   Convert instances of:
 *       (--expr) to (expr)
 *       (expr + -expr) to (expr - expr)
 *       (expr - -expr) to (expr + expr)
 *
 * Inputs:  expr - expression to simplify
 * Returns: simplified AST
 *************************************************************
 */
AST_INDEX
pt_del_unary_minus (AST_INDEX expr)
{
AST_INDEX expr1,expr2,temp_expr;

if (expr == AST_NIL)
	return AST_NIL;

/* switch on the node type */
switch( gen_get_node_type(expr)) {

	case GEN_UNARY_MINUS:
		expr1 = gen_UNARY_MINUS_get_rvalue(expr);
		if (gen_get_node_type(expr1) == GEN_UNARY_MINUS)
		  return pt_del_unary_minus(gen_UNARY_MINUS_get_rvalue(expr1));
		else
		  {
		   temp_expr = pt_del_unary_minus(expr1);
		   return gen_UNARY_MINUS(temp_expr);
		  }
     	case GEN_BINARY_PLUS:
		expr1 = pt_del_unary_minus(gen_BINARY_PLUS_get_rvalue1(expr));
		expr2 = pt_del_unary_minus(gen_BINARY_PLUS_get_rvalue2(expr));
		if (gen_get_node_type(expr2) == GEN_UNARY_MINUS) {
		  temp_expr = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(expr2));
		  tree_free(expr2);
		  return gen_BINARY_MINUS(expr1,temp_expr);
		}
		else
		  return gen_BINARY_PLUS(expr1,expr2);
    	case GEN_BINARY_MINUS:
		expr1 = pt_del_unary_minus(gen_BINARY_MINUS_get_rvalue1(expr));
		expr2 = pt_del_unary_minus(gen_BINARY_MINUS_get_rvalue2(expr));
		if (gen_get_node_type(expr2) == GEN_UNARY_MINUS) {
		  temp_expr = tree_copy_with_type(gen_UNARY_MINUS_get_rvalue(expr2));
		  tree_free(expr2);
		  return gen_BINARY_PLUS(expr1,temp_expr);
		}
		else
		  return gen_BINARY_MINUS(expr1,expr2);
	default:
		return tree_copy_with_type(expr);
	}
}


/*************************************************************
 * pt_simplify_expr (expr)
 *
 *   Perform 5 simplifications to an expression:
 *       1 - fold constants
 *       2 - eliminate multiplication by +1,0,-1
 *       3 - eliminate division by +1,-1
 *	 4 - convert (--expr) to expr
 *	 5 - convert (expr + -expr) to -, and (expr - -expr) to +
 *
 *   Note that the AST pointed to by expr is freed by this
 *   routine and a completely new AST is returned.
 *
 * Inputs:  expr - expression to simplify
 * Returns: simplified AST
 *************************************************************
 */
AST_INDEX
pt_simplify_expr (AST_INDEX expr)
{
  AST_INDEX temp1,temp2;
  int constant;

  pt_fold_term(expr,&temp1,&constant);
  temp1 = pt_add_const(temp1,constant);
  tree_free(expr);
  temp2 = pt_del_unary_minus(temp1);
  tree_free(temp1);
  return(temp2);
}


/*************************************************************
 * pt_find_var_node (expr,var)
 *
 *   If the expression contains a reference to the variable
 *   then return node of reference, else return AST_NIL
 *
 * Inputs: expr - The expression
 *         var  - the variable to search for
 *
 * Returns: AST_INDEX of node of reference
 *************************************************************
 */
static int
pt_find_var_node_walk (AST_INDEX expr, Pt_walk_parms *pt_parms)
{
	if ((pt_parms->node == AST_NIL) && (expr != AST_NIL))
	  if (is_identifier(expr))
		if (!strcmp(gen_get_text(expr), pt_parms->var)) {
			pt_parms->node = expr;
			return WALK_ABORT;
		}

	return WALK_CONTINUE;
}


AST_INDEX
pt_find_var_node (AST_INDEX expr, char *var)
{
	Pt_walk_parms pt_parms;
	AST_INDEX temp_expr;

	pt_parms.node = AST_NIL;
	pt_parms.var = var;
	walk_expression (expr, (WK_EXPR_CLBACK)pt_find_var_node_walk, NULL, 
                         (Generic)&pt_parms);
	temp_expr = pt_parms.node;
	return temp_expr;
}


/*************************************************************
 * pt_find_a_var (expr)
 *
 *   If the expression contains a reference to any variable
 *   then return a pointer to the var name, else return NULL.
 *
 * Inputs:  expr - the expression to search
 *
 * Returns: char pointer or NULL.
 *************************************************************
 */
static char *
pt_find_a_var (AST_INDEX expr)
{
	char *temp;

	if (expr == AST_NIL)
	  return NULL;

	switch( gen_get_node_type(expr)) {
	  case GEN_IDENTIFIER:
		return gen_get_text(expr);
	  case GEN_UNARY_MINUS:
		return pt_find_a_var(gen_UNARY_MINUS_get_rvalue(expr));
	  case GEN_BINARY_EXPONENT:
		temp = pt_find_a_var(gen_BINARY_EXPONENT_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_EXPONENT_get_rvalue2(expr));
	  case GEN_BINARY_TIMES:
		temp = pt_find_a_var(gen_BINARY_TIMES_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_TIMES_get_rvalue2(expr));
	  case GEN_BINARY_DIVIDE:
		temp = pt_find_a_var(gen_BINARY_DIVIDE_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_DIVIDE_get_rvalue2(expr));
	  case GEN_BINARY_PLUS:
		temp = pt_find_a_var(gen_BINARY_PLUS_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_PLUS_get_rvalue2(expr));
	  case GEN_BINARY_MINUS:
		temp = pt_find_a_var(gen_BINARY_MINUS_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_MINUS_get_rvalue2(expr));
	  case GEN_UNARY_NOT:
		return (pt_find_a_var(gen_UNARY_NOT_get_rvalue(expr)));
	  case GEN_BINARY_AND:
		temp = pt_find_a_var(gen_BINARY_AND_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_AND_get_rvalue2(expr));
	  case GEN_BINARY_OR:
		temp = pt_find_a_var(gen_BINARY_OR_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_OR_get_rvalue2(expr));
	  case GEN_BINARY_EQ:
		temp = pt_find_a_var(gen_BINARY_EQ_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_EQ_get_rvalue2(expr));
	  case GEN_BINARY_NE:
		temp = pt_find_a_var(gen_BINARY_NE_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_NE_get_rvalue2(expr));
	  case GEN_BINARY_GE:
		temp = pt_find_a_var(gen_BINARY_GE_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_GE_get_rvalue2(expr));
	  case GEN_BINARY_GT:
		temp = pt_find_a_var(gen_BINARY_GT_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_GT_get_rvalue2(expr));
	  case GEN_BINARY_LE:
		temp = pt_find_a_var(gen_BINARY_LE_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_LE_get_rvalue2(expr));
	  case GEN_BINARY_LT:
		temp = pt_find_a_var(gen_BINARY_LT_get_rvalue1(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_BINARY_LT_get_rvalue2(expr));
	  case GEN_ASSIGNMENT:
		temp = pt_find_a_var(gen_ASSIGNMENT_get_lvalue(expr));
		if (temp != NULL)
		  return temp;
		else
		  return pt_find_a_var(gen_ASSIGNMENT_get_rvalue(expr));
	  default:
		return NULL;
	}
}


/*************************************************************
 * pt_find_var (expr,var)
 *
 *   If the expression contains a reference to var
 *   then return true, else return false.
 *
 * Inputs:  expr - The expression
 *          var  - the variable to search for
 *
 * Returns: true or false
 *************************************************************
 */
Boolean
pt_find_var (AST_INDEX expr, char *var)
{
  return BOOL(pt_find_var_node(expr,var) != AST_NIL);
}


/*************************************************************
 * pt_sep_linear (expr,var,lin,factor,const)
 *
 *   If expr is linear in var, then separate expr into
 *   its component parts:
 *      return lin=true,
 *      expr = factor*var + const
 *   else:
 *      return lin=false
 *      factor = const = AST_NIL
 *
 * Inputs:  expr - The expression to separate
 *          var  - the variable to search for
 *
 * Outputs: lin    - true if expr is linear in var
 *          factor - AST of factor of var in expr
 *          const  - AST of constant terms in expr
 *************************************************************
 */
void
pt_sep_linear (AST_INDEX expr, char *var, Boolean *lin,
               AST_INDEX *factor, AST_INDEX *constant)
{
  char *string;
  Boolean lin1,lin2;
  AST_INDEX fac1,fac2,con1,con2;
  AST_INDEX tcon1,tcon2;

  *lin = true;
  *factor = *constant = AST_NIL;

  if (expr == AST_NIL)
	return;

  switch (gen_get_node_type(expr)) {

    	case GEN_IDENTIFIER:
		string = gen_get_text(expr);
		if (!strcmp(string, var))
		  *factor = pt_gen_const("1");
		else
		  *constant = tree_copy_with_type(expr);
		break;
	case GEN_CONSTANT:
		*constant = tree_copy_with_type(expr);
		break;
	case GEN_UNARY_MINUS:
		pt_sep_linear(gen_UNARY_MINUS_get_rvalue(expr),var,&lin1,&fac1,&con1);
		*lin = lin1;
		if (*lin) {
		  if (fac1 != AST_NIL)
		    *factor = gen_UNARY_MINUS(fac1);
		  if (con1 != AST_NIL)
		    *constant = gen_UNARY_MINUS(con1);
		}
		break;
     	case GEN_BINARY_PLUS:
		pt_sep_linear(gen_BINARY_PLUS_get_rvalue1(expr),var,&lin1,&fac1,&con1);
		pt_sep_linear(gen_BINARY_PLUS_get_rvalue2(expr),var,&lin2,&fac2,&con2);
		*lin = BOOL(lin1 && lin2);
		if (*lin) {
		  *factor = pt_gen_add(fac1,fac2);
		  *constant = pt_gen_add(con1,con2);
		}
		break;
    	case GEN_BINARY_MINUS:
		pt_sep_linear(gen_BINARY_MINUS_get_rvalue1(expr),var,&lin1,&fac1,&con1);
		pt_sep_linear(gen_BINARY_MINUS_get_rvalue2(expr),var,&lin2,&fac2,&con2);
		*lin = BOOL(lin1 && lin2);
		if (*lin) {
		  *factor = pt_gen_sub(fac1,fac2);
		  *constant = pt_gen_sub(con1,con2);
		}
		break;
	case GEN_BINARY_TIMES:
		pt_sep_linear(gen_BINARY_TIMES_get_rvalue1(expr),var,&lin1,&fac1,&con1);
		pt_sep_linear(gen_BINARY_TIMES_get_rvalue2(expr),var,&lin2,&fac2,&con2);
		*lin = BOOL(lin1 && lin2);
		if (*lin)
		  if ((fac1 != AST_NIL) && (fac2 != AST_NIL))
		    *lin = false;
		if (*lin) {
		  AST_INDEX tcon1 = tree_copy_with_type(con1);
		  AST_INDEX tcon2 = tree_copy_with_type(con2);
		  fac1 = pt_gen_mul(fac1,con2);
		  fac2 = pt_gen_mul(fac2,con1);
		  *factor = pt_gen_add(fac1,fac2);
		  *constant = pt_gen_mul(tcon1,tcon2); 
		}
		break;
	default:
		if (pt_find_var(expr,var))
		  *lin = false;
		else
 	          *constant = tree_copy_with_type(expr);
		break;
  }
}


/*************************************************************
 * pt_separate_linear (expr,var,lin,factor,const)
 *
 *   Separate an expression that is linear in var into
 *   2 AST's so that expr = factor*const.
 *   Do this via pt_sep_linear, then call pt_simplify_expr
 *   on factor and const.
 *
 * Inputs:  expr - the expression to separate
 *          var  - the variable to search for
 *
 * Outputs: lin    - true if expr is linear in var
 *          factor - AST of factor of var in expr
 *          const  - AST of constant terms in expr
 *************************************************************
 */
void
pt_separate_linear (AST_INDEX expr, char *var, Boolean *lin,
                    AST_INDEX *factor, AST_INDEX *constant)
{
  pt_sep_linear(expr,var,lin,factor,constant);
  *factor = pt_simplify_expr(*factor);
  *constant = pt_simplify_expr(*constant);
}


/*************************************************************
 * pt_get_coeff (expr,var,lin,coeff)
 *
 *   If the coefficient of var in expr evaluates to a constant,
 *   then return lin=true and coeff = coefficient of var.
 *
 * Inputs:  expr - the expression to evaluate
 *          var  - the variable to search for
 *
 * Outputs: lin    - true if expr is linear in var
 *          coeff  - the coefficient of var
 *************************************************************
 */
void
pt_get_coeff (AST_INDEX expr, char *var, Boolean *lin, int *coeff)
{
  AST_INDEX fac,con,temp;

  pt_separate_linear(expr,var,lin,&fac,&con);
  if (*lin) {
    pt_fold_term(fac,&temp,coeff);
    if (temp != AST_NIL)
      *lin = false;
  }
  if (NOT(*lin))
    *coeff = 0;

  tree_free(fac);
  tree_free(con);
}

/*************************************************************
 * pt_equal_stmt_list (slist1,slist2)
 *
 *   Compare 2 lists of statements.  If the lists are
 *   symbolically equal then return true, else false.
 *
 * Inputs: slist1 - statement list to compare
 *         slist2 - statement list to compare
 *
 * Returns: true or false
 *************************************************************
 */
static Boolean
pt_equal_stmt_list(AST_INDEX slist1, AST_INDEX slist2)
{
        AST_INDEX stmt1,stmt2;

	/* get the first statement */
	stmt1 = list_first(slist1);
	stmt2 = list_first(slist2);

	/* bump through the statements */
	while((stmt1 != AST_NIL) && (stmt2 != AST_NIL)) {

	        if (!pt_expr_equal(stmt1,stmt2))
		  return false;
		stmt1 = list_next(stmt1);
		stmt2 = list_next(stmt2);
	}

        if ((stmt1 == AST_NIL) && (stmt2 == AST_NIL))
	  return true;
	else
	  return false;
}


/*************************************************************
 * pt_expr_equal (expr1,expr2)
 *
 *   Compare expr1 to expr2 symbolically and return
 *   true if they are equivalent.
 *
 * Inputs:  expr1 - AST to compare
 *          expr2 - AST to compare
 *
 * Returns: true or false
 *************************************************************
 */
Boolean
pt_expr_equal (AST_INDEX expr1, AST_INDEX expr2)
{
  int icon1,icon2;
  AST_INDEX new1,new2,fac1,fac2,con1,con2;
  Boolean lin1,lin2;
  char *varname;

  if ((expr1 == AST_NIL) && (expr2 == AST_NIL))
    return true;
  if ((expr1 == AST_NIL) || (expr2 == AST_NIL))
    return false;

  if (is_identifier(expr1))
  {
      return BOOL(is_identifier(expr2) && 
                  !strcmp(gen_get_text(expr1),gen_get_text(expr2)));
  }

  pt_fold_term (expr1,&new1,&icon1);
  pt_fold_term (expr2,&new2,&icon2);
  if (icon1 != icon2)
    return false;
  if ((new1 == AST_NIL) && (new2 == AST_NIL))
    return true;
  if ((new1 == AST_NIL) || (new2 == AST_NIL))
    return false;

  if (is_invocation(new1) && is_invocation(new2))
    return pt_equal_stmt_list(gen_INVOCATION_get_actual_arg_LIST(new1),
			      gen_INVOCATION_get_actual_arg_LIST(new2));
  else if (is_subscript(new1) && is_subscript(new2))
  {
    return BOOL(pt_expr_equal(gen_SUBSCRIPT_get_name(new1), 
                              gen_SUBSCRIPT_get_name(new2))  &&
               pt_equal_stmt_list(gen_SUBSCRIPT_get_rvalue_LIST(new1),
			                      gen_SUBSCRIPT_get_rvalue_LIST(new2)));
  }
  else {
    varname = pt_find_a_var(new1);
    if (varname == NULL)
      return false;
  
    pt_separate_linear(new1,varname,&lin1,&fac1,&con1);
    pt_separate_linear(new2,varname,&lin2,&fac2,&con2);
    if (NOT(lin1) || NOT(lin2))
      return false;
    return BOOL(pt_expr_equal(fac1,fac2) && pt_expr_equal(con1,con2));
  }
}


/*************************/
/* Routines to check AST */
/*************************/

/*-----------------------------------------------------------------------

	pt_perf_nest()		Checks whether perfectly nested loop

	We are interested in all levels of perfect nesting, i.e.
	we check i to see whether it is perfectly nested:

	do i ...
		do j ...
			do ...
				<stmts>
			enddo
		enddo
	enddo

	Returns:	Level of perfect nesting found
				0 means not perfect nest
*/

int
pt_perf_nest(AST_INDEX node)
{
   int level = 0;

   while (is_comment(node) && (node != AST_NIL)) node = list_next(node);

   if (!is_do(node)) return 0;   /* need to start with a loop */

   node = gen_DO_get_stmt_LIST(node);
   node = list_first(node);

   while (is_comment(node) && (node != AST_NIL)) node = list_next(node);

   level++;

   if (node == AST_NIL) return level;  /* at the end of the list */

   if (is_do(node)) { /* perfectly nested so far */ 
      int temp = 0;

      /* check to see if this loop is a perfect nest */
      if ((temp = pt_perf_nest(node)) == 0) return 0;

      node = list_next(node);

      while (node != AST_NIL) {
	 /* check for a statement other than a comment.  If one */
	 /* is found, it is not a perfect nest loop.            */
	 if (!is_comment(node)) return 0;
	 node = list_next(node);
      }

      level += temp;

   }
   else { /* found an actual statement */
      node = list_next(node);
      while (node != AST_NIL) {
	 if (is_do(node)) return 0;  /* too bad, found a do statement */
	 node = list_next(node);
      }
   }
   return level;
}


/*************************************/
/* Routines to clear info side array */
/*************************************/

/*-----------------------------------------------------------------------

	pt_clear_info()		Clear bogus info array handles
						from the AST tree

	Used to clean up after tree_copy_with_type(), which 
	copies all side array values to the new tree.

*/


void
pt_clear_info(PedInfo ped, AST_INDEX tree)
{
    walk_expression(tree, pt_walk_clear_info, NULL, PED_INFO_SIDE_ARRAY(ped));
}

static int
pt_walk_clear_info(AST_INDEX expr, Generic info_side_array)
{
    ast_put_side_array (info_side_array, expr, 0, UNUSED);
    return WALK_CONTINUE;
}


/***********************************/
/* Routines to replace variables   */
/***********************************/

/*-----------------------------------------------------------------------

	pt_var_replace (expr,var,new_expr)
	
	Replace each occurrence of var in tree with new_expr
	
	Inputs: tree - The subtree to be searched
	        var - the variable
	        new_expr - the replacement expression
	
	Outputs: none

*/

void
pt_var_replace(AST_INDEX tree, char *var, AST_INDEX new_expr)
{
	Pt_walk_parms parms;

	parms.node = new_expr;
	parms.var = var;
	walk_expression(tree, NULL, (WK_EXPR_CLBACK)pt_walk_replace, (Generic)&parms);
}

static int
pt_walk_replace (AST_INDEX expr, Pt_walk_parms *parms)
{
	if ((expr == AST_NIL) || !is_identifier(expr) ||
			strcmp(gen_get_text(expr), parms->var))
		return WALK_CONTINUE;

	pt_tree_replace(expr,tree_copy_with_type(parms->node));
	return WALK_CONTINUE;
}


/*-----------------------------------------------------------------------

	pt_var_add (tree,var,value)
	
	Add value to each occurrence of var in tree
	Fold simple additive terms if found
	
	Inputs: tree - The subtree to be searched
	        var - the variable
	        value - the adjustment value
	
	Outputs: none

*/

void
pt_var_add(AST_INDEX tree, char *var, int value)
{
	Pt_walk_parms parms;

	if (value)
	{
		parms.value = value;
		parms.var = var;
		walk_expression(tree, NULL, (WK_EXPR_CLBACK)pt_walk_add, (Generic)&parms);
	}
}

static int
pt_walk_add (AST_INDEX expr, Pt_walk_parms *parms)
{
	AST_INDEX parent;
	AST_INDEX term;
	int value;
	int v;

	/*--------------------------------------------------*/
	/* check whether variable to adjust value for		*/

	if ((expr == AST_NIL) || !is_identifier(expr) ||
			strcmp(gen_get_text(expr), parms->var))
		return WALK_CONTINUE;

	parent = tree_out(expr);
	value = parms->value;

	/*--------------------------------------------------*/
	/* check whether already part of an add expr		*/

	if (is_binary_plus(parent))
	{
		/*----------------------*/
		/* get the other term	*/

		term = gen_BINARY_PLUS_get_rvalue1(parent);
		if (term == expr)
			term = gen_BINARY_PLUS_get_rvalue2(parent);

		/*----------------------------------------------*/
		/* now see whether we can simply merge terms	*/

		if (NOT(pt_eval(term, &v)))		/* constant found	*/
		{
			v += value;

			if (!v)		/* additive terms disappear	*/
			{
				tree_replace(parent, tree_copy_with_type(expr));
			}

			else if (v < 0)	/* change to minus node	*/
			{
				tree_replace(parent, 
					pt_gen_sub(tree_copy_with_type(expr), pt_gen_int(-v)));
			}

			else	 /* if (v > 0)	modify additive term	*/
			{
				tree_replace(term, pt_gen_int(v));
			}

			return WALK_CONTINUE;
		}
	}

	/*--------------------------------------------------*/
	/* check whether already part of an sub expr		*/

	else if (is_binary_minus(parent))
	{
		/*----------------------*/
		/* get the other term	*/

		term = gen_BINARY_MINUS_get_rvalue2(parent);

		/*----------------------------------------------*/
		/* now see whether we can simply merge terms	*/

		if (NOT(pt_eval(term, &v)))		/* constant found	*/
		{
			v = value - v;

			if (!v)		/* additive terms disappear	*/
			{
				tree_replace(parent, tree_copy_with_type(expr));
			}

			else if (v > 0)	/* change to plus node	*/
			{
				tree_replace(parent, 
					pt_gen_add(tree_copy_with_type(expr), pt_gen_int(v)));
			}

			else	 /* if (v < 0)	modify subtract term	*/
			{
				tree_replace(term, pt_gen_int(-v));
			}

			return WALK_CONTINUE;
		}
	}

	/*----------------------------------------------------------*/
	/* unable to simplify/combine with existing expressions		*/
	/* just do a brute force replace with the appropriate expr	*/

	if (value > 0)
	{
		pt_tree_replace(expr,
			pt_gen_add(tree_copy_with_type(expr), pt_gen_int(value)));
	}
	else
	{
		pt_tree_replace(expr,
			pt_gen_sub(tree_copy_with_type(expr), pt_gen_int(-value)));
	}


	return WALK_CONTINUE;
}


/*-----------------------------------------------------------------------

	pt_var_mul (tree,var,value)
	
	Add value to each occurrence of var in tree
	Fold simple multiply terms if found
	
	Inputs: tree - The subtree to be searched
	        var - the variable
	        value - the adjustment value
	
	Outputs: none

*/

void
pt_var_mul(AST_INDEX tree, char *var, int value)
{
	Pt_walk_parms parms;

	if (value != 1)
	{
		parms.value = value;
		parms.var = var;
		walk_expression(tree, NULL, (WK_EXPR_CLBACK)pt_walk_mul, (Generic)&parms);
	}
}

static int
pt_walk_mul (AST_INDEX expr, Pt_walk_parms *parms)
{
	AST_INDEX parent;
	AST_INDEX term;
	int value;
	int v;

	/*--------------------------------------------------*/
	/* check whether variable to adjust value for		*/

	if ((expr == AST_NIL) || !is_identifier(expr) ||
			strcmp(gen_get_text(expr), parms->var))
		return WALK_CONTINUE;

	parent = tree_out(expr);
	value = parms->value;

	/*--------------------------------------------------*/
	/* check whether already part of an times expr		*/

	if (is_binary_times(parent))
	{
		/*----------------------*/
		/* get the other term	*/

		term = gen_BINARY_TIMES_get_rvalue1(parent);
		if (term == expr)
			term = gen_BINARY_TIMES_get_rvalue2(parent);

		/*----------------------------------------------*/
		/* now see whether we can simply merge terms	*/

		if (NOT(pt_eval(term, &v)))		/* constant found	*/
		{
			v *= value;
			tree_replace(term, pt_gen_int(v));
			return WALK_CONTINUE;
		}
	}

	/*----------------------------------------------------------*/
	/* unable to simplify/combine with existing expressions		*/
	/* just do a brute force replace with the appropriate expr	*/

	pt_tree_replace(expr, pt_gen_mul(pt_gen_int(value), tree_copy_with_type(expr)));
	return WALK_CONTINUE;
}


/************************************/
/* Routines to evaluate expressions */
/************************************/

/*-----------------------------------------------------------------------

	pt_eval()	eval function for simple integer expressions 

	Returns:	true		if symbolics/complex exprs found
				false		otherwise (constant expression)
*/

Boolean
pt_eval(AST_INDEX node, int *iptr)
{
	int val1, val2;					/* intermediate results */

	/*---------------------------------------------------*/
	/* unary minus */

	if (is_unary_minus(node))
	{
		if (pt_eval(gen_UNARY_MINUS_get_rvalue(node), &val1))
			return true;

		*iptr = -val1;
		return false;			/* found constant expression	*/
	}

	/*---------------------------------------------------*/
	/* constants */

	if (is_constant(node))
	{
		*iptr = atoi(gen_get_text(node));
		return false;			/* found constant expression	*/
	}

	/*---------------------------------------------------*/
	/* identifier */

	if (is_identifier(node))
	{
		return true;

#ifdef WAITING_FOR_PACO
		if (NOT(val_exp_is_const(node)))
			return true;

		*iptr = (int) val_exp_int(node);
		return false;			/* found constant expression	*/
#endif

	}

	/*---------------------------------------------------*/
	/* plus	*/

	if (is_binary_plus(node))
	{
		if (pt_eval(gen_BINARY_PLUS_get_rvalue1(node), &val1))
			return true;

		if (pt_eval(gen_BINARY_PLUS_get_rvalue2(node), &val2))
			return true;

		*iptr = val1 + val2;
		return false;			/* found constant expression	*/
	}

	/*---------------------------------------------------*/
	/* minus */

	if (is_binary_minus(node))
	{
		if (pt_eval(gen_BINARY_MINUS_get_rvalue1(node), &val1))
			return true;

		if (pt_eval(gen_BINARY_MINUS_get_rvalue2(node), &val2))
			return true;

		*iptr = val1 - val2;
		return false;			/* found constant expression	*/
	}

	/*---------------------------------------------------*/
	/* multiply	*/

	if (is_binary_times(node))
	{
		if (pt_eval(gen_BINARY_TIMES_get_rvalue1(node), &val1))
			return true;

		if (pt_eval(gen_BINARY_TIMES_get_rvalue2(node), &val2))
			return true;

		*iptr = val1 * val2;
		return false;			/* found constant expression	*/
	}

	/*---------------------------------------------------*/
	/* divide	*/

	if (is_binary_divide(node))
	{
		if (pt_eval(gen_BINARY_DIVIDE_get_rvalue1(node), &val1))
			return true;

		if (pt_eval(gen_BINARY_DIVIDE_get_rvalue2(node), &val2))
			return true;

		if (!val2)
		{
			printf("pt_eval(): division by zero found\n");
			return true;
		}

		*iptr = val1 / val2;
		return false;			/* found constant expression	*/

	}

	/*---------------------------------------------------*/
	/* not in proper normalized form	*/

	return true;
}


/*---------------------------------------------------------------

  pt_simplify_node()  

  Replaces AST node with constant AST node if possible

*/

AST_INDEX
pt_simplify_node(AST_INDEX node)
{
	int             value;	/* ptr to result & result   */
	AST_INDEX       new_node;
	char            char_buf[100];

	if (NOT(pt_eval(node, &value)))	/* expression does simplify */
	{
		new_node = gen_CONSTANT();
		(void) sprintf(char_buf, "%d", value);
		gen_put_text(new_node, char_buf, STR_CONSTANT_INTEGER);
		return new_node;
	}
	return node;
}


/**********************************/
/* Routines to build list of refs */
/**********************************/

/*-----------------------------------------------------------------------

	pt_refs()		Find all variables referenced in AST
					The list includes any induction variables used 
					Separate into def & uses lists

	Returns:	Pointer to Pt_ref_params structure holding results

*/


Pt_ref_params *
pt_refs(AST_INDEX tree, PedInfo ped)
{
	Pt_ref_params *params;

	/*--------------------------*/
	/* initialize parameters	*/

	params = (Pt_ref_params *) get_mem(sizeof(Pt_ref_params), "pt_refs");

	if (!params)
	{
		die_with_message("pt_refs(): out of memory\n");
	}

	params->defs_num = 0;
	params->defs_size = MAX_DEFS;
	params->subls_num = 0;
	params->subls_size = MAX_SUBLS;
	params->stmts_num = 0;
	params->stmts_size = MAX_STMTS;
	params->uses_num = 0;
	params->uses_size = MAX_USES;
	params->iv_num = 0;
	params->iv_size = MAX_USES;

	params->defs = (AST_INDEX *) get_mem(sizeof(AST_INDEX) * MAX_DEFS, "pt_refs");
	params->subls = (AST_INDEX *) get_mem(sizeof(AST_INDEX) * MAX_SUBLS, "pt_refs");
	params->stmts = (AST_INDEX *) get_mem(sizeof(AST_INDEX) * MAX_STMTS, "pt_refs");
	params->uses = (AST_INDEX *) get_mem(sizeof(AST_INDEX) * MAX_USES, "pt_refs");
	params->iv   = (AST_INDEX *) get_mem(sizeof(AST_INDEX) * MAX_IVS, "pt_refs");

	if (!params->defs || !params->uses || !params->iv || !params->stmts || !params->subls)
	{
		die_with_message("pt_refs(): out of memory\n");
	}

	/*--------------*/
	/* go on walk	*/

	walk_statements(tree, LEVEL1, (WK_STMT_CLBACK)pt_stmt_refs, (WK_STMT_CLBACK)NOFUNC,
		        (Generic)params);

	return params;

}

/*-----------------------------------------------------------------------

	pt_stmt_refs()		Find all variables referenced in AST

	Returns:		Code for walk_statements()

*/

static int
pt_stmt_refs(AST_INDEX stmt, int lvl, Pt_ref_params *refs)
{
	AST_INDEX  expr1;
	AST_INDEX  expr2;

	/* store the statement index */
	if (refs->stmts_num == refs->stmts_size)
	{
		refs->stmts_size <<= 1;
		refs->stmts = (AST_INDEX *) reget_mem(refs->stmts, refs->stmts_size, "pt_stmt_refs");
	}
	refs->stmts[refs->stmts_num++] = stmt;	

	/*----------------------------------------------------------*/
	/* special treatment of do, since we want induction vars	*/

	if (is_do(stmt) || is_parallelloop(stmt))
	{
		expr1 = is_do(stmt) ?	gen_DO_get_control(stmt) :
								gen_PARALLELLOOP_get_control(stmt);

		/*------------------------------------------*/
		/* store away inductive variable			*/
		/* grow iv list if needed					*/
		/* mark variable being defined in def list	*/

		expr2 = gen_INDUCTIVE_get_name(expr1);

		if (refs->iv_num == refs->iv_size)
		{
			refs->iv_size <<= 1;
			refs->iv = (AST_INDEX *) reget_mem(refs->iv, refs->iv_size, "pt_stmt_refs");
		}

		refs->iv[refs->iv_num++] = expr2;	

		/*----------------------------------------------*/
		/* walk over lower, upper, and step expressions	*/

		if ((expr2 = gen_INDUCTIVE_get_rvalue1(expr1)) != AST_NIL)
                  walk_expression(expr2, (WK_EXPR_CLBACK)pt_expr_uses, NULL, (Generic)refs);

		if ((expr2 = gen_INDUCTIVE_get_rvalue2(expr1)) != AST_NIL)
                  walk_expression(expr2, (WK_EXPR_CLBACK)pt_expr_uses, NULL, (Generic)refs);

		if ((expr2 = gen_INDUCTIVE_get_rvalue3(expr1)) != AST_NIL)
                  walk_expression(expr2, (WK_EXPR_CLBACK)pt_expr_uses, NULL, (Generic)refs);
	}

	/*----------------------------------------------------------*/
	/* special treatment of assign, since we separate defs/uses	*/

	else if (is_assignment(stmt))
	{
		/*--------------------------*/
		/* grow def list if needed	*/

		if (refs->defs_num == refs->defs_size)
		{
			refs->defs_size <<= 1;
			refs->defs = (AST_INDEX *) reget_mem(refs->defs, refs->defs_size, "pt_stmt_refs");
		}

		/*------------------------------------------*/
		/* mark variable being defined in def list	*/

		expr1 = gen_ASSIGNMENT_get_lvalue(stmt);
		expr2 = gen_ASSIGNMENT_get_rvalue(stmt);

		if (is_subscript(expr1))
		{
			/* store the subscript index */
			if (refs->subls_num == refs->subls_size)
			{
				refs->subls_size <<= 1;
				refs->subls = (AST_INDEX *) reget_mem(refs->subls, refs->subls_size, "pt_stmt_refs");
			}
			refs->subls[refs->subls_num++] = expr1;	


			/* store lvalue	& incr # of defs found	*/
			refs->defs[refs->defs_num++] = gen_SUBSCRIPT_get_name(expr1);	

			/*------------------------------------------------------*/
			/* array variables may include uses in subscripts		*/
			/* so if array def, get uses in subscript expression(s)	*/

			expr1 = gen_SUBSCRIPT_get_rvalue_LIST(expr1);
			walk_expression(expr1, (WK_EXPR_CLBACK)pt_expr_uses, 
                                        NULL, (Generic)refs);
		}
		else
		{
			/* store lvalue	& incr # of defs found	*/

			refs->defs[refs->defs_num++] = expr1;	
		}

		/*------------------------------------------*/
		/* mark variable(s) being used in uses list	*/

		walk_expression(expr2, (WK_EXPR_CLBACK)pt_expr_uses, NULL, (Generic)refs);
	}

	/*------------------------------------------------------*/
	/* generic statement, get & walk over component exprs	*/

	else
	{
		if (get_expressions(stmt, &expr1, &expr2) == UNKNOWN_STATEMENT)
                  return WALK_CONTINUE; 

		if (expr1 != AST_NIL)
                  walk_expression(expr1, (WK_EXPR_CLBACK)pt_expr_uses, NULL, (Generic)refs);

		if (expr2 != AST_NIL)
                  walk_expression(expr2, (WK_EXPR_CLBACK)pt_expr_uses, NULL, (Generic)refs);
	}

	return WALK_CONTINUE;

}

/*-----------------------------------------------------------------------

	pt_expr_uses()		Find all variables used in expression

	Returns:		Code for walk_expression()

*/

static int
pt_expr_uses(AST_INDEX expr, Pt_ref_params *refs)
{
	if (is_identifier(expr))	/* find identifiers used in expression	*/
	{
		/*--------------------------*/
		/* grow uses list if needed	*/

		if (refs->uses_num == refs->uses_size)
		{
			refs->uses_size <<= 1;
			refs->uses = (AST_INDEX *) reget_mem(refs->uses, refs->uses_size, "pt_expr_uses");
		}

		/*------------------------------------------*/
		/* mark variable being used in uses list	*/

		refs->uses[refs->uses_num++] = expr;	

	}

	return WALK_CONTINUE;
}

/*---------------------------------------------------------------------

*/

char*
pt_get_stmt_text(PedInfo ped, AST_INDEX num)
{
	static char     text[512];
	char           *p;

	/* return the text of the statement corresponding to this ast index */
	if (num == 0)
		return ("");
	strcpy(text, (ped->GetLine) (PED_ED_HANDLE(ped), num));
	/* eat leading blanks */
	for (p = text; (*p == ' ') || (*p == '\t'); p++);
	return (p);
}


/*-------------------------------------------------------------------

	pt_gen_func1()	generate single-parameter-function ast node

*/

AST_INDEX
pt_gen_func1(char *fn, AST_INDEX arg1)
{
	return gen_INVOCATION(pt_gen_ident(fn),list_create(arg1));
}


void
pt_get_constant_walk(AST_INDEX expr, AST_INDEX *constant)
{
  char *string;
  AST_INDEX con1,con2;
  AST_INDEX tcon1,tcon2;

  *constant = AST_NIL;

  if (expr == AST_NIL)
	return;

  switch (gen_get_node_type(expr)) {

    	case GEN_IDENTIFIER:
		break;
	case GEN_CONSTANT:
		*constant = tree_copy_with_type(expr);
		break;
	case GEN_UNARY_MINUS:
		pt_get_constant_walk(gen_UNARY_MINUS_get_rvalue(expr),&con1);
		if (con1 != AST_NIL)
		  *constant = gen_UNARY_MINUS(con1);
		break;
     	case GEN_BINARY_PLUS:
		pt_get_constant_walk(gen_BINARY_PLUS_get_rvalue1(expr),&con1);
		pt_get_constant_walk(gen_BINARY_PLUS_get_rvalue2(expr),&con2);
		*constant = pt_gen_add(con1,con2);
		break;
    	case GEN_BINARY_MINUS:
		pt_get_constant_walk(gen_BINARY_MINUS_get_rvalue1(expr),&con1);
		pt_get_constant_walk(gen_BINARY_MINUS_get_rvalue2(expr),&con2);
		*constant = pt_gen_sub(con1,con2);
		break;
	case GEN_BINARY_TIMES:
		pt_get_constant_walk(gen_BINARY_TIMES_get_rvalue1(expr),&con1);
		pt_get_constant_walk(gen_BINARY_TIMES_get_rvalue2(expr),&con2);
		{
		 AST_INDEX tcon1 = tree_copy_with_type(con1);
		 AST_INDEX tcon2 = tree_copy_with_type(con2);
		 *constant = pt_gen_mul(tcon1,tcon2); 
		}
		break;
	default:
		break;
  }
}

/*************************************************************
 * pt_get_constant (expr,const)
 *
 *************************************************************
 */
void
pt_get_constant(AST_INDEX expr, int *constant)
  {
   AST_INDEX node;

     pt_get_constant_walk(expr,&node);
     if (node == AST_NIL)
       *constant = 0;
     else
       (void)pt_eval(node,constant);
  }
