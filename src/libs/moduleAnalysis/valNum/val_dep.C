/* $Id: val_dep.C,v 1.4 1997/03/11 14:36:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <string.h>

#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/moduleAnalysis/dependence/dependenceTest/dep_dt.h>

static void collect(ValTable &V, ValNumber term, CoVarPair *vec);
static void collect_sub(ValTable &V, ValNumber term, Subs_data *sub);
static void collect_loop_bound(ValTable &V,ValNumber term,Expr *expr,int **vec);

CoVarPair * val_dep_parse(ValTable *Vp, ValNumber v, int level)
{
    //  Find closest surrounding loop, get its level, and allocate
    //  that many elements for the returned vector
    //
    int depth = max(level, ve_level((*Vp)[v]));
    CoVarPair *rv = new CoVarPair[depth +1];

    //  Initialize each vector element to [(int) 0, VAL_ZERO]
    //
    int i;
    for (i = 0; i <= depth; i ++)
    {
	rv[i].coeff = 0;
	rv[i].sym   = VAL_ZERO;
    }

    //  Assume simplified right-leaning sums -- otherwise, this is 
    //  safe but pathologically stupid.
    //
    for (;
	 ((ve_type((*Vp)[v]) == VAL_OP) &&
	  (ve_opType((*Vp)[v]) == VAL_OP_PLUS));
	 v = ve_right((*Vp)[v]))
    {
	collect((*Vp), ve_left((*Vp)[v]), rv);
    }
    collect((*Vp), v, rv);

    return rv;
}

static void collect(ValTable &V, ValNumber term, CoVarPair *vec)
{
    if (ve_type(V[term]) == VAL_CONST)
    {
	vec[0].coeff += ve_const(V[term]);
    }
    else if (ve_type(V[term]) == VAL_IVAR)
    {
	//  Assume all ivars for a level in the same subscript are the same
	//
	vec[ve_level(V[term])].coeff ++;
    }
    else if ((ve_type(V[term]) == VAL_OP) && 
	     (ve_opType(V[term]) == VAL_OP_TIMES) &&
	     (ve_type(V[ve_left(V[term])]) == VAL_CONST) &&
	     (ve_type(V[ve_right(V[term])]) == VAL_IVAR))
    {
	//  Assume all ivars for a level in the same subscript are the same
	//
	vec[ve_level(V[term])].coeff += ve_const(V[ve_left(V[term])]);
    }
    else
    {
	//  Non-inductive symbolic
	//  Double-check against simplifier -- are we guaranteeing 
	//	quadratic list-insertion behavior here?  Not to mention
	//	creating a lot of wasted value numbers?
	//
	vec[ve_level(V[term])].sym = val_binary(&V, VAL_OP_PLUS,
						vec[ve_level(V[term])].sym,
						term);
    }
}

void val_dep_free(CoVarPair *cv)
{
    delete cv;
}

void val_dep_parse_sub(ValTable *Vp, ValNumber v, int depth, Subs_data *sub)
{
  if (depth < ve_level((*Vp)[v]))
    {
      // printf("WARNING: Non-DO loop found! Analysis may be incorrect!\n");
      sub->stype = SUBS_SYM;
      return;
    }

  // Reinitialize Subs_data structure
  //      
  sub->constant = 0;
  sub->stype = SUBS_ZIV;
  sub->symbolic_constant = VAL_ZERO;
  memset(sub->coeffs, 0, sizeof(sub->coeffs));

  //  Assume simplified right-leaning sums -- otherwise, this is 
  //  safe but pathologically stupid.
  //
  for (;
       ((ve_type((*Vp)[v]) == VAL_OP) &&
	(ve_opType((*Vp)[v]) == VAL_OP_PLUS));
       v = ve_right((*Vp)[v]))
    {
      collect_sub((*Vp), ve_left((*Vp)[v]), sub);
      if (sub->stype == SUBS_SYM)
	return;
    }
  collect_sub((*Vp), v, sub);
}

static void collect_sub(ValTable &V, ValNumber term, Subs_data *sub)
{
  if (ve_type(V[term]) == VAL_CONST)
    sub->constant += ve_const(V[term]);

  else if (ve_type(V[term]) == VAL_IVAR ||
	   ((ve_type(V[term]) == VAL_OP) && 
	    (ve_opType(V[term]) == VAL_OP_TIMES) &&
	    (ve_type(V[ve_left(V[term])]) == VAL_CONST) &&
	    (ve_type(V[ve_right(V[term])]) == VAL_IVAR)))
    {
      int loop_level = ve_level(V[term]) - 1;

      if (sub->stype == SUBS_ZIV)
	sub->stype = loop_level;

      else if (sub->stype <= SUBS_SIV && sub->stype != loop_level)
	sub->stype = SUBS_MIV;

      else if (sub->stype == SUBS_SYM_ZIV)
	sub->stype = SUBS_SYM_SIV_FIRST + loop_level;

      else if (sub->stype >= SUBS_SYM_SIV_FIRST && 
	       sub->stype <= SUBS_SYM_SIV_LAST  &&
	       sub->stype - SUBS_SYM_SIV_FIRST != loop_level)
	sub->stype = SUBS_SYM_MIV;
		 
      if (ve_type(V[term]) == VAL_IVAR)
	sub->coeffs[loop_level] ++;
      else 
	sub->coeffs[loop_level] += ve_const(V[ve_left(V[term])]);
    }

  else
    {
      //  Non-inductive symbolic
      //
      if (ve_level(V[term]) == 0)
	{
	  if (sub->stype == SUBS_ZIV)
	    sub->stype = SUBS_SYM_ZIV;
	      
	  else if (sub->stype <= SUBS_SIV)
	    sub->stype += SUBS_SYM_SIV_FIRST;

	  else if (sub->stype == SUBS_MIV)
	    sub->stype = SUBS_SYM_MIV;;
	    
	  sub->symbolic_constant = val_binary(&V, VAL_OP_PLUS,
					      sub->symbolic_constant, term);
	}
      else 
	{
	  sub->stype = SUBS_SYM;
	  return;
	}
    }
}


void val_dep_parse_loop_bound(ValTable *Vp, ValNumber v, int depth,
			      AST_INDEX node, Expr *expr, int **vec)
{
  int level = ve_level((*Vp)[v]);
	
  // Initialize Loop_data structure
  //      
  expr->ast = node;
  expr->str = (char *) NULL;

  if (depth <= level)
    {
      // printf("WARNING: Non-DO loop found! Analysis may be incorrect!\n");
      expr->type = Expr_complex;
      *vec = (int *) NULL;
    }

  else if (ve_type((*Vp)[v]) == VAL_CONST)
    {
      *vec = new int[depth];
      memset(*vec, 0, depth * sizeof(int));
      (*vec)[0] = expr->val = ve_const((*Vp)[v]);
      expr->type = Expr_constant;
    }

  else if (is_invocation(node))
    {
      expr->type = Expr_invocation;
      *vec = (int *) NULL;
    }

  else if (ve_type((*Vp)[v]) == VAL_IVAR)
    {
      *vec = new int[depth];
      memset(*vec, 0, depth * sizeof(int));
      (*vec)[level] = 1;
      expr->type = Expr_index_var;
    }

  else 
    {
      *vec = new int[depth];
      memset(*vec, 0, depth * sizeof(int));
      expr->type = Expr_constant;
      
      //  Assume simplified right-leaning sums 
      //
	for (;
	     ((ve_type((*Vp)[v]) == VAL_OP) &&
	      (ve_opType((*Vp)[v]) == VAL_OP_PLUS));
	     v = ve_right((*Vp)[v]))
	  {
	    collect_loop_bound((*Vp), ve_left((*Vp)[v]), expr, vec);
	    if (expr->type == Expr_complex)
	      {	
		if (is_identifier(node))
		  expr->type = Expr_simple_sym;
		return;
	      }
	  }
      collect_loop_bound((*Vp), v, expr, vec);
      if (expr->type == Expr_complex && is_identifier(node))
	expr->type = Expr_simple_sym;
    }
}

static void collect_loop_bound(ValTable &V, ValNumber term,Expr *expr,int **vec)
{
  if (ve_type(V[term]) == VAL_CONST)
    {
      if (*vec != NULL)
	(*vec)[0] += ve_const(V[term]);
    }

  else if (ve_type(V[term]) == VAL_IVAR)
    {
      if (*vec != NULL)
	{
	  (*vec)[ve_level(V[term])] ++;
	  expr->type = Expr_linear_ivar_only;
	}
    }

  else if ((ve_type(V[term]) == VAL_OP) && 
	   (ve_opType(V[term]) == VAL_OP_TIMES))
    {
      if ((ve_type(V[ve_left(V[term])]) == VAL_CONST) &&
	  (ve_type(V[ve_right(V[term])]) == VAL_IVAR))
	{
	  if (*vec != NULL)
	    {
	      (*vec)[ve_level(V[term])] += ve_const(V[ve_left(V[term])]);
	      expr->type = Expr_linear_ivar_only;	      
	    }
	}
      else if ((ve_type(V[ve_right(V[term])]) == VAL_CONST) &&
	       (ve_type(V[ve_left(V[term])]) == VAL_IVAR))
	{
	  if (*vec != NULL)
	    {
	      (*vec)[ve_level(V[term])] += ve_const(V[ve_right(V[term])]);
	      expr->type = Expr_linear_ivar_only;	      
	    }
	}
      else if ((ve_type(V[ve_right(V[term])]) == VAL_CONST) ||
	       (ve_type(V[ve_left(V[term])])  == VAL_CONST))
	{
	  expr->type = Expr_linear;
	  delete *vec;
	  *vec = (int *) NULL;
	  // This corresponds to CONST * VARIABLE (non-inductive)
	  // We set vec to NULL, but do not abort parsing because we 
	  // have to distinguish  between Expr_linear and Expr_complex
	}
      else 
	{
	  expr->type = Expr_complex;
	  delete *vec;
	  *vec = (int *) NULL;
	  return;
	}
    }

  else
    {
      //  Non-inductive symbolic
      //
      expr->type = Expr_complex;
      delete *vec;
      *vec = (int *) NULL;
      return;
    }
}
